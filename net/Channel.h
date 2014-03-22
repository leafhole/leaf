#ifndef NET_CHANNEL_H
#define NET_CHANNEL_H

#include <boost/function.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

#include <base/Timestamp.h>

namespace leaf {
namespace net {

class EventLoop;

///
/// A selectable I/O channel.
///
/// This class doesn't own the file descriptor.
/// The file descriptor could be a socket,
/// an eventfd, a timerfd, or a singnal fd

class Channel : boost::noncopyable {
 public:
  typedef boost::function<void()> EventCallback;
  typedef boost::function<void(Timestamp)> ReadEventCallback;

  Channel(EventLoop* loop, int fd);
  ~Channel();
  void setReadCallback(const ReadEventCallback& cb)
  { readEventCallback_ = cb; }

  void setWriteCallback(const EventCallback& cb)
  { writeCallback_ = cb; }

  void setCloseCallback(const EventCallback& cb)
  { closeCallback_ = cb; }

  void setErrorCallback(const EventCallback& cb)
  { errorCallback_ = cb; }

  /// Tie this channel to the owner object managed by shared_ptr,
  /// prevent the owner object being destroyed in handleEvent.
  void tie(const boost::shared_ptr<void>&);

  int fd() const { return fd_; }
  int events() const { return events_; }
  void set_revents(int revt) { revents_ = revt; }; // used by pollers
  bool isNoneEvent() const { return events_ == kNoneEvent; }

  void enableReading() { events_ |= kReadEvent; update(); }
  void disableReading() { events_ &= ~kReadEvent; update(); }
  void enableWriting() { events_ |= kWriteEvent; update(); }// faint
  void disableWriting() { events_ &= ~kWriteEvent; update(); }
  void disableAll() { events_ = kNoneEvent; update(); }
  bool isWriting() const { return events_ & kWriteEvent; }

  // for Poller
  int index() { return index_; }
  void set_index(int idx) { index_ = idx; }

  //for debug
  string reventsToString() const;

  void doNotLogHup() { logHup_ = false; }

  EventLoop* ownerLoop() { return loop_; };
  void remove();
  void handleEvent(Timestamp receiveTime);
 private:
  void update();

  void handleEventWithGuard(Timestamp receiveTime);  

  static const int kNoneEvent;
  static const int kReadEvent;
  static const int kWriteEvent;

  EventLoop* loop_;
  const int fd_;
  int events_;
  int revents_;
  int index_; // used by Poller.
  bool logHup_;

  boost::weak_ptr<void> tie_;
  bool tied_;
  bool eventHandling_;
  ReadEventCallback readEventCallback_; //read的时候,为什么需要加上一个Timestamp呢?也是,临时变量来记录时间,不能够放到成员变量里面
  EventCallback writeCallback_;
  EventCallback closeCallback_;
  EventCallback errorCallback_;

}; // end of class channel
 
} // end of namespace net
} // end of namespace leaf 


#endif
