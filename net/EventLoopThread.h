#ifndef NET_EVENTLOOPTHREAD_H
#define NET_EVENTLOOPTHREAD_H

#include <base/Condition.h>
#include <base/Mutex.h>
#include <base/Thread.h>

#include <boost/noncopyable.hpp>

namespace leaf {

namespace net {
class EventLoop;

class EventLoopThread : boost::noncopyable {
 public:
  typedef boost::function<void(EventLoop*)> ThreadInitCallback;

  EventLoopThread(const ThreadInitCallback& cb = ThreadInitCallback());
  ~EventLoopThread();
  EventLoop* startLoop();

 private:
  void threadFunc();

  EventLoop* loop_;
  bool exiting_;
  Thread thread_;
  MutexLock mutex_;
  Condition cond_;
  ThreadInitCallback callback_;
};
  
  
}// end of namespace net
} // end of namespace leaf



#endif // NET_EVENTLOOPTHREAD_H
