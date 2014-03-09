#include <net/TcpConnection.h>

#include <base/Logging.h>
#include <net/Channel.h>
#include <net/EventLoop.h>
#include <net/Socket.h>
#include <net/SocketsOps.h>

#include <boost/bind.hpp>

#include <errno.h>
#include <stdio.h>

namespace leaf {
template<typename CLASS>
class WeakCallback {
public:
  WeakCallback(const boost::weak_ptr<CLASS>& object,
	       const boost::function<void (CLASS*)>& function)
    : object_(object), function_(function)
  {
  }

  // default dtor, copy ctor and assignment are okay

  void operator()() const {
    boost::shared_ptr<CLASS> ptr(object_.lock());
    if (ptr) {
      function_(ptr.get());
    }
  }
private:
  boost::weak_ptr<CLASS> object_;
  boost::function<void (CLASS*)> function_;
}; // end of class WeakCallback

template<typename CLASS>
WeakCallback<CLASS> makeWeakCallback(const boost::shared_ptr<CLASS>& object,
				     void (CLASS::*function)()) {
  return WeakCallback<CLASS>(object, function);
}
  /*
template<typename CLASS>
WeakCallback<CLASS> makeWeakCallback(const boost::shared_ptr<CLASS>& object,
				     void (CLASS::*function)()) {
  return WeakCallback<CLASS>(object, function);
}

tempalte<typename CLASS>
WeakCallback<CLASS> makeWeakCallback(const boost::shared_ptr<CLASS>& object,
				     void (CLASS::*function)() const) {
  return WeakCallback<CLASS>(object, function);
}
  */

} // end of namespace
  
using namespace leaf;
using namespace leaf::net;

void leaf::net::defaultConnectionCallback(const TcpConnectionPtr& conn) {
  LOG_TRACE << conn->localAddress().toIpPort() << " -> "
	    << conn->peerAddress().toIpPort() << " is "
	    << (conn->connected() ? "UP" : "DOWN");
}

void leaf::net::defaultMessageCallback(const TcpConnectionPtr&,
				       Buffer* buf,
				       Timestamp) {
  buf->retrieveAll();
}

TcpConnection::TcpConnection(EventLoop* loop,
			     const string& nameArg,
			     int sockfd,
			     const InetAddress& localAddr,
			     const InetAddress& peerAddr)
  : loop_(CHECK_NOTNULL(loop)),
    name_(nameArg),
    state_(kConnecting),
    socket_(new Socket(sockfd)),
    channel_(new Channel(loop, sockfd)),
    localAddr_(localAddr),
    peerAddr_(peerAddr),
    highWaterMark_( 64 * 1024 * 1024 ) {
  channel_->setReadCallback(boost::bind(&TcpConnection::handleRead, this, _1));
  channel_->setWriteCallback(boost::bind(&TcpConnection::handleWrite, this));
  channel_->setCloseCallback(boost::bind(&TcpConnection::handleClose, this));
  channel_->setErrorCallback(boost::bind(&TcpConnection::handleError, this));
  LOG_DEBUG << "TcpConnection::ctor[" << name_ << "] at " << this
	    << " fd=" << sockfd;
  socket_->setKeepAlive(true);			    
}

TcpConnection::~TcpConnection() {
  LOG_DEBUG << "TcpConnection::dtor[" << name_ << "] at " << this
	    << " fd= " << channel_->fd();
}

void TcpConnection::send(const void* data, int len) {
  send(StringPiece(static_cast<const char*>(data), len));
}

void TcpConnection::send(const StringPiece& message) {
  if (state_ == kConnected) {
    if (loop_->isInLoopThread()) {
      sendInLoop(message);
    } else {
      loop_->runInLoop(boost::bind(&TcpConnection::sendInLoop,
				   this,
				   message.as_string()));
		       
    }
  }
}
  

// FIXME efficiency!!!
void TcpConnection::send(Buffer* buf) {
  if (state_ == kConnected) {
    if (loop_->isInLoopThread()) {
      sendInLoop(buf->peek(), buf->readableBytes());
      buf->retrieveAll();
    }
    else {
      loop_->runInLoop(boost::bind(&TcpConnection::sendInLoop,
				   this,
				   buf->retrieveAllAsString()));
    }
  }
}

void TcpConnection::sendInLoop(const StringPiece& message) {
  sendInLoop(message.data(), message.size());
}

void TcpConnection::sendInLoop(const void* data, size_t len) {
  loop_->assertInLoopThread();
  ssize_t nwrote = 0;
  size_t remaining = len;
  bool faultError = false;
  if (state_ == kDisconnected) {
    LOG_WARN << "disconnected, give up writing";
    return;
  }

  if (!channel_->isWriting() && outputBuffer_.readableBytes() == 0) {
    nwrote = sockets::write(channel_->fd(), data, len);
    if (nwrote > 0) {
      remaining = len - nwrote;
      if (remaining == 0 && writeCompleteCallback_) {
	loop_->queueInLoop(boost::bind(writeCompleteCallback_, shared_from_this()));
      }
    } else { // write error  nwrote < 0
      nwrote = 0;
      if (errno != EWOULDBLOCK) {
	LOG_SYSERR << "TcpConnection::sendInLoop";
	if (errno == EPIPE || errno == ECONNRESET) { // FIXME: any others?
	  faultError = true;
	}
      }
    }
  }

  assert(remaining <= len);
  if (!faultError && remaining > 0) {
    size_t oldLen = outputBuffer_.readableBytes(); // already written
    if (oldLen + remaining >= highWaterMark_
	and oldLen < highWaterMark_
	&& highWaterMarkCallback_) {
      loop_->queueInLoop(boost::bind(highWaterMarkCallback_, shared_from_this(), oldLen + remaining));
    }
    
    outputBuffer_.append(static_cast<const char*>(data) + nwrote, remaining);
    if (!channel_->isWriting()) {
      channel_->enableWriting();
    }
  }
}

void TcpConnection::shutdown() {
  if (state_ == kConnected) {
    setState(kDisconnecting);
    loop_->runInLoop(boost::bind(&TcpConnection::shutdownInLoop, shared_from_this())); //不过反正也不要了，shutdown之后，也不再要了;错了，仅仅关闭write ,仍然可读，汗
  }
}

void TcpConnection::shutdownInLoop() {
  loop_->assertInLoopThread();
  if (!channel_->isWriting()) {
    socket_->shutdownWrite();
  }
}

void TcpConnection::forceClose() {
  // FIXME: use compare and swap
  if (state_ == kConnected || state_ == kDisconnecting) {
    setState(kDisconnecting);
    loop_->queueInLoop(boost::bind(&TcpConnection::forceCloseInLoop, shared_from_this()));
  }
}

void TcpConnection::forceCloseWithDelay(double seconds) {
  if (state_ == kConnected || state_ == kDisconnecting) {
    setState(kDisconnecting);
    loop_->runAfter(seconds,
		    makeWeakCallback(shared_from_this(),
				     &TcpConnection::forceCloseInLoop));
  }
}

void TcpConnection::forceCloseInLoop() {
  loop_->assertInLoopThread();
  if (state_ == kConnected || state_ == kDisconnecting) {
    // as if we received 0 byte in handleRead();
    handleClose();
  }
}

void TcpConnection::setTcpNoDelay(bool on) {
  socket_->setTcpNoDelay(on);
}

void TcpConnection::connectEstablished() {
  loop_->assertInLoopThread();
  assert(state_ == kConnecting);
  setState(kConnected);
  channel_->tie(shared_from_this());
  channel_->enableReading();

  connectionCallback_(shared_from_this());
}

void TcpConnection::connectDestroyed() {
  loop_->assertInLoopThread();
  if (state_ == kConnected) {
    setState(kDisconnected);
    channel_->disableAll();

    connectionCallback_(shared_from_this());
  }
  channel_->remove();
}

void TcpConnection::handleRead(Timestamp receiveTime) {
  loop_->assertInLoopThread();
  int savedErrno = 0;
  ssize_t n = inputBuffer_.readFd(channel_->fd(), &savedErrno);
  if (n > 0) {
    messageCallback_(shared_from_this(), &inputBuffer_, receiveTime);
  } else if (n == 0) {
    handleClose();
  } else {
    errno = savedErrno;
    LOG_SYSERR << "TcpConnection::handleRead";
    handleError();
  }
}

void TcpConnection::handleWrite() {
  loop_->assertInLoopThread();
  if (channel_->isWriting()) {
    ssize_t n = sockets::write(channel_->fd(),
			       outputBuffer_.peek(),
			       outputBuffer_.readableBytes());
    if (n > 0) {
      outputBuffer_.retrieve(n);
      if (outputBuffer_.readableBytes() == 0) {
	channel_->disableWriting();
	if (outputBuffer_.readableBytes() == 0) {
	  channel_->disableWriting();
	  if (writeCompleteCallback_) {
	    loop_->queueInLoop(boost::bind(writeCompleteCallback_, shared_from_this()));
	  }
	  if (state_ == kDisconnecting) {
	    shutdownInLoop();
	  }
	}
	
      }
      
    } else {
      LOG_SYSERR << "TcpConnection::handleWrite";
    }
  } else {
    LOG_TRACE << "Connection fd = " << channel_->fd()
	      << " is down, no more writing";
  }
}

void TcpConnection::handleClose() {
  loop_->assertInLoopThread();
  LOG_TRACE << "fd = " << channel_->fd() << " state = " << state_;
  assert(state_ == kConnected || state_ == kDisconnecting);
  // we don't close fd, leave it to dtor, so we can find leaks easily
  setState(kDisconnected);
  channel_->disableAll();

  TcpConnectionPtr guardThis(shared_from_this());
  connectionCallback_(guardThis);
  // must be the last line
  closeCallback_(guardThis);
}

void TcpConnection::handleError() {
  int err = sockets::getSocketError(channel_->fd());
  LOG_ERROR << "TcpConnection::handleError [" << name_
	    << "] - SO_ERROR = " << err << " " << strerror_tl(err);
}
