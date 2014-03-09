#ifndef NET_POLLER_H
#define NET_POLLER_H

#include <vector>
#include <boost/noncopyable.hpp>

#include <base/Timestamp.h>
#include <net/EventLoop.h>

namespace leaf {
namespace net {
class Channel;

///
/// Base class for IO Multiplexing
///
/// This class doesn't own the Channel objects.
class Poller : boost::noncopyable {
 public:
  typedef std::vector<Channel*> ChannelList;

  Poller(EventLoop* loop);
  virtual ~Poller();

  /// Polls the I/O events
  /// Must be called in the loop thread
  virtual Timestamp poll(int timeoutMs, ChannelList* aceiveChannels) = 0;


  virtual void updateChannel(Channel* channel) = 0;

  
  /// Remove the channel, when it destructs
  /// Must be called in loop thread
  virtual void removeChannel(Channel* channel) = 0;

  static Poller* newDefaultPoller(EventLoop* loop);

  void assertInLoopThread() {
    ownerLoop_->assertInLoopThread();
  }
  
 private:
  //EventLoop_->assertInLoopThread();
  EventLoop* ownerLoop_;
};

} // end of namespace net
} // end of namespace leaf

#endif //NET_POLLER_H
