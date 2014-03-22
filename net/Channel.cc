#include <base/Logging.h>
#include <net/Channel.h>
#include <net/EventLoop.h>

#include <sstream>
#include <poll.h>

using namespace leaf;
using namespace leaf::net;

const int Channel::kNoneEvent = 0;
const int Channel::kReadEvent = POLLIN | POLLPRI; 
const int Channel::kWriteEvent = POLLOUT;

Channel::Channel(EventLoop* loop, int fd)
  : loop_(loop),
    fd_(fd),
    events_(0),
    revents_(0),
    index_(-1),
    logHup_(true),
    tied_(false),
    eventHandling_(false) {
}

Channel::~Channel() {
  assert(!eventHandling_);
}

void Channel::tie(const boost::shared_ptr<void>& obj) {
  tie_ = obj;
  tied_ = true;
}

void Channel::update() {
  loop_->updateChannel(this);
}

void Channel::remove() {
  assert(isNoneEvent());
  loop_->removeChannel(this);
}

void Channel::handleEvent(Timestamp receiveTime) {
  boost::shared_ptr<void> guard;
  if (tied_) {
    guard = tie_.lock();
    if (guard) {
      handleEventWithGuard(receiveTime);
    }
  } else {
    handleEventWithGuard(receiveTime);
  }
}
  
void Channel::handleEventWithGuard(Timestamp receiveTime) {
  eventHandling_ = true;
  LOG_TRACE << reventsToString();
  if ((revents_ & POLLHUP) && !(revents_ & POLLIN)) {
    if (logHup_) {
      LOG_WARN << "Channel::handle_event() POLLHUP";
    }
    if (closeCallback_) closeCallback_();    
  }

  if (revents_ & POLLNVAL) {
    LOG_WARN << "Channel::handle_event() POLLNVAL";
  }

  if (revents_ & (POLLERR | POLLNVAL)) {
    if (errorCallback_) errorCallback_();
  }

#ifndef POLLRDHUP //这里是指如果remote peer进行了close,那么就会捕捉到POLLRDHUP事件，不过一半来说,没有这个.都是使用的POLLIN,然后read的时候，返回0
  const int POLLRDHUP = 0;
#endif
  if (revents_ & (POLLIN | POLLPRI | POLLRDHUP)) {
    if (readEventCallback_) readEventCallback_(receiveTime);
  }
  if (revents_ & POLLOUT) {
    if (writeCallback_) writeCallback_();
  }
  eventHandling_ = false;
}

leaf::string Channel::reventsToString() const {
  std::ostringstream oss;
  oss << fd_ << " : " << revents_ << " : ";
  if (revents_ & POLLIN) {
    oss << "IN ";
  }
  if (revents_ & POLLPRI) {
    oss << "PRI ";
  }
  if (revents_ & POLLHUP) {
    oss << "HUP ";
  }
#ifdef POLLRDHUP
  if (revents_ & POLLRDHUP) {
    oss << "RDHUP";
  }
#endif
  if (revents_ & POLLERR) {
    oss << "ERR ";
  }
  if (revents_ & POLLNVAL) {
    oss << "NVAL ";
  }

  return oss.str().c_str();
}


/*
POLLPRI
POLLIN There is data to read. POLLPRI There is urgent data to read.

If you use POLLIN only, poll() will return if there is data or urgent data to read. If you use POLLPRI only, poll() will return only if there is urgent data to read, but ignore normal data.

What's urgent data? Like tcp's out-of-band data. In TCP frame header, there is a flag named urg_data. Urg_data means this frame has higher priority to delivery. Once kernel received a urg_data maked frame, it set a POLLPRI flag!




 */
