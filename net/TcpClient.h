#ifndef NET_TCPCLIENT_H
#define NET_TCPCLIENT_H

#include <boost/noncopyable.hpp>
#include <base/Mutex.h>
#include <net/TcpConnection.h>

namespace leaf {
namespace net {

class Connector;
typedef boost::shared_ptr<Connector> ConnectorPtr;

class TcpClient : boost::noncopyable {
  TcpClient(EventLoop* loop,
	    const InetAddress& serverAddr,
	    const string& name);
  ~TcpClient(); // force out-line dtor, for scpoed_ptr members.

  void connect();
  void disconnect();
  void stop();

  TcpConnectionPtr connection() const {
    MutexLockGuard lock(mutex_);
    return connection_;
  }

  EventLoop* getLoop() const { return loop_; }
  bool retry() const;
  void enableRetry() { retry_ = true; }

  /// Set connection callback.
  /// Not thread safe
  void setConnectionCallback(const ConnectionCallback& cb)
  { connectionCallback_ = cb; }

  /// Set message callback
  /// Not thread safe
  void setMessageCallback(const MessageCallback& cb) {
    messageCallback_ = cb;
  }

  /// Set write complete callback
  /// Not thread safe
  void setWriteCompleteCallback(const WriteCompleteCallback& cb)
  { writeCompleteCallback_ = cb; }


 private:
  /// not thread safe, but in loop
  void newConnection(int sockfd);
  /// not thread safe, but in loop
  void removeConnection(const TcpConnectionPtr& conn);

  EventLoop* loop_;
  ConnectorPtr connector_; // avoid revealing Connector
  const string name_;
  ConnectionCallback connectionCallback_;
  MessageCallback messageCallback_;
  WriteCompleteCallback writeCompleteCallback_;
  bool retry_; // atomic
  bool connect_; // atomic
  // always in loop thread
  int nextConnId_;
  mutable MutexLock mutex_;
  TcpConnectionPtr connection_; // @Be guarded By mutex_
}; // end of class TcpClient

} // end of namespace net
} // end of namespace leaf

#endif // NET_TCPCLIENT_H
