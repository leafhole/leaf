#ifndef NET_EVENTLOOP_H
#define NET_EVENTLOOP_H

#include <vector>

#include <boost/function.hpp>
#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>

#include <base/Mutex.h>
#include <base/Thread.h>
#include <base/Timestamp.h>
#include <net/Callbacks.h>
#include <net/TimerId.h>

namespace leaf {
namespace net {
  
class Channel;
class Poller;
class TimerQueue;

///
/// Reactor, at most one per thread.
///
/// This is an interface class, so don't expose too much details.
class EventLoop : boost::noncopyable {
 public:
  typedef boost::function<void()> Functor;

  EventLoop();
  ~EventLoop();

  ///
  /// Loops forever
  ///
  /// Must be called in the same thread as creation of the object.
  ///
  void loop();

  /// Quits loop.
  ///
  /// This is not 100% thread save, if you call through a raw pointer,
  /// better to call through shared_ptr<EventLoop> for 100% safety.
  void quit();

  ///
  /// Time when poll returns, usually means data arrival.
  ///
  Timestamp pollReturnTime() const { return pollReturnTime_; }

  int64_t iteration() const {return iteration_; }

  /// Runs callback immediately in the loop thread.
  /// It wakes up the loop, and run the cb.
  /// If in the same loop thread, cb is run within the function
  /// Safe to call from others.
  /// 在其他线程里面获得这个eventLoop,然后调用他的方法,只不过不可以直接在其他线程里面执行
  /// 需要给queue到eventLoop的持有线程,具体的io相关的工作，必须那个线程自己干，其他线程
  /// 只能分配jobs给他，不能替他干活
  /// 放到pendingJobs里面去,然后唤醒这个持有eventLoop的线程。
  void runInLoop(const Functor& cb);

  /// Queue callback in loop thread.
  /// Runs after finish pooling
  /// Safe to call from other threads.
  /// 同上吧
  void queueInLoop(const Functor& cb);

#ifdef __GXX_EXPERIMENTAL_CXX0X__
  void runInLoop(Functor&& cb);
  void queueInLoop(Functor&& cb);
#endif

  // timers

  ///
  /// Runs callback at 'time'
  /// Safe to call from other threads.
  ///
  TimerId runAt(const Timestamp& time, const TimerCallback& cb);

  ///
  /// Runs callback after @c delay seconds.
  /// Safe to call from other threads.
  ///
  TimerId runAfter(double delay, const TimerCallback& cb);

  ///
  /// Runs call back every @c delay seconds.
  /// Safe to call from other threads.
  ///
  TimerId runEvery(double interval, const TimerCallback& cb);

  ///
  /// Cancels the timer.
  /// Safe to call from other threads.
  ///
  void cancel(TimerId timerId);
#ifdef __GXX_EXPERIMENTAL_CXX0X__

  TimerId runAt(const Timestamp& time, TimeCallback& cb);
  TimerId runAfter(double delay, TimeCallback& cb);
  TimerId runEvery(double interval, TimeCallback& cb);  
  #endif

  // internal usage
  void wakeup();
  void updateChannel(Channel* channel);
  void removeChannel(Channel* channel);

  // pid_t threadId() const { return threadId_; }
  void assertInLoopThread() {
    if (!isInLoopThread()) {
      abortNotInLoopThread();
    }
  }

  bool isInLoopThread() const { return threadId_ == CurrentThread::tid(); }
  bool callingPendingFunctors() const { return callingPendingFunctors_; }
  bool eventHandling() const { return eventHandling_; }

  static EventLoop* getEventLoopOfCurrentThread();

 private:
  void abortNotInLoopThread();
  void handleRead(); // wake up ??
  void doPendingFunctors();

  void printActiveChannels() const; // DEBUG

  typedef std::vector<Channel*> ChannelList;
  
 private:
  bool looping_; /* atomic */
  bool quit_; /* atomic and shared between threads. */
  bool eventHandling_; /* atomic */
  bool callingPendingFunctors_; /* atomic */
  int64_t iteration_;
  const pid_t threadId_;
  Timestamp pollReturnTime_;
  boost::scoped_ptr<Poller> poller_;
  boost::scoped_ptr<TimerQueue> timerQueue_;
  int wakeupFd_;
  // unlike in TimerQueue, which is an internal class,
  // we don't expose Channel to client
  boost::scoped_ptr<Channel> wakeupChannel_;
  ChannelList activeChannels_;
  Channel* currentActiveChannel_;
  MutexLock mutex_;
  ///
  /// pendingFunctors_是用来做queue in job的. 如果要做某种IO的job, TODO
  /// 直接把响应的job给放到pendingFunctors_里面即可.但是问题是
  /// 为咋只有这一个呢?
  /// 其实就是对于这个EventLoop所控制的多个channel,都处理完毕相应的I/O之后
  /// 调用doPendingFunctors()
  ///
  std::vector<Functor> pendingFunctors_; // @BuardedBy mutex_

}; // end of class EventLoop
}
}


#endif // NET_EVENTLOOP_H
