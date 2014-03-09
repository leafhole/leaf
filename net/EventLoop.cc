#include <net/EventLoop.h>
#include <base/Logging.h>
#include <net/Poller.h>
#include <signal.h>
#include <base/Logging.h>
#include <base/Mutex.h>
#include <net/Channel.h>
#include <net/Poller.h>
#include <net/SocketsOps.h>
#include <net/TimerQueue.h>
#include <net/Callbacks.h>

#include <boost/bind.hpp>

#include <signal.h>

using namespace leaf;
using namespace leaf::net;


namespace {
__thread EventLoop* t_loopInThisThread = 0;

const int kPollTimeMs = 10 * 1000;

int createEventfd() {
  //  int eventfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
  int eventfd = 0;
  if (eventfd < 0) {
    LOG_SYSERR << "Failed in eventfd";
    abort();
  }
  return eventfd;
}

#pragma GCC diagnostic ignored "-Wold-style-cast"
class IgnoreSigPipe {
public:
  IgnoreSigPipe() {
    ::signal(SIGPIPE, SIG_IGN);
    LOG_TRACE << "Ignore SIGPIPE";
  }
};
#pragma GCC diagnostic error "-Wold-style-cast"

IgnoreSigPipe initObj;
} // end of unnamed namespace

EventLoop* EventLoop::getEventLoopOfCurrentThread() {
  return t_loopInThisThread;
}


EventLoop::EventLoop()
  : looping_(false),
    quit_(false),
    eventHandling_(false),
    callingPendingFunctors_(false),
    iteration_(0),
    threadId_(CurrentThread::tid()),
    poller_(Poller::newDefaultPoller(this)),
    timerQueue_(new TimerQueue(this)),
    wakeupFd_(createEventfd()),
    wakeupChannel_(new Channel(this, wakeupFd_)),
    currentActiveChannel_(NULL) {

  LOG_DEBUG << "EventLoop create " << this << " in thread " << threadId_;
  /*
  if (::socketpair(AF_UNIX, SOCK_STREAM, 0, wakeupFd_) < 0) {
    LOG_SYSFATAL << "Failed in socketpair";
  }
  wakeupChannel_.reset(new Channel(this, wakeupFd_));
  */
  if (t_loopInThisThread) {
    LOG_FATAL << "Another EventLoop " << t_loopInThisThread
	      << " exists in this thread " << threadId_;
  } else {
    t_loopInThisThread = this;    
  }
  //别人写进来一个东西，这里直接给读走就好了
  wakeupChannel_->setReadCallback(
				  boost::bind(&EventLoop::handleRead, this));
  // 仅仅关注读事件，因为仅仅需要唤醒即可，不需要写
  wakeupChannel_->enableReading();  
}

EventLoop::~EventLoop() {
  LOG_DEBUG << "EventLoop " << this << " of thread " << threadId_
	    << " destructs in thread " << CurrentThread::tid();
  ::close(wakeupFd_);
  //::close(wakeupFd_[1]);
  t_loopInThisThread = NULL;  
}

void EventLoop::loop() {
  assert(!looping_);
  assertInLoopThread();
  looping_ = true;
  quit_ = false; // FIXME: what if someone call quit() before loop() ?
  LOG_TRACE << "EventLoop " << this << "start looping";

  while (not quit_) {
    activeChannels_.clear();
    pollReturnTime_ = poller_->poll(kPollTimeMs,
				   &activeChannels_);
    ++ iteration_;
    if (Logger::logLevel() <= Logger::TRACE) {
      printActiveChannels();
    }
    //timerQueue_->processTimers();
    // TODO sort channel by priority
    eventHandling_ = true;
    for (ChannelList::iterator it = activeChannels_.begin();
	 it != activeChannels_.end(); ++ it) {
      currentActiveChannel_ = *it;
      currentActiveChannel_->handleEvent(pollReturnTime_);
    }
    currentActiveChannel_ = NULL;
    eventHandling_ = false;
    doPendingFunctors();
  }

  LOG_TRACE << "Eventloop " << this << " stop looping";
  looping_ = false;
}

void EventLoop::quit() {
  quit_ = true;
  // There is a chance that loop() just executes while(!quit_) and exists,
  // then EventLoop destructs, hen we are accessing an invalid object.
  // Can be fixed using mutex_ in both places. But not necessary
  if (!isInLoopThread()) {
    wakeup();
  }
}

void EventLoop::runInLoop(const Functor& cb) {
  if (isInLoopThread()) {
    cb();
  } else {
    queueInLoop(cb);
  }
}

void EventLoop::queueInLoop(const Functor& cb) {

  {
    MutexLockGuard lock(mutex_);
    pendingFunctors_.push_back(cb);
  }

  if (!isInLoopThread() || callingPendingFunctors_) {
    wakeup();
  }  
}

TimerId EventLoop::runAt(const Timestamp& time,
			 const TimerCallback& cb) {
  return timerQueue_->addTimer(cb, time, 0.0);
}

TimerId EventLoop::runAfter(double delay, const TimerCallback& cb) {
  Timestamp time(addTime(Timestamp::now(), delay));
  return runAt(time, cb);
}

TimerId EventLoop::runEvery(double interval, const TimerCallback& cb) {
  Timestamp time(addTime(Timestamp::now(), interval));
  return timerQueue_->addTimer(cb, time, interval);		 
}

#ifdef __GXX_EXPERIMENTAL_CXX0X__
// FIXME: remove duplication
void EventLoop::runInLoop(Functor&& cb) {
  if (isInLoopThread()) {
    cb();
  } else {
    queueInLoop(std::move(cb));
  }
}

void EventLoop::queueInLoop(Functor&& cb) {
  {
    MutexLockGuard lock(mutex_);
    pendingFunctors_.push_back(std::move(cb));
  }

  if (!isInLoopThread() || callingPendingFunctors_) {
    wakeup();
  }
}

TimerId EventLoop::runAt(const Timestamp& time, const TimerCallback&& cb) {
  return timerQueue_->addTimer(std::move(cb), time, 0.0);
}

TimerId EventLoop::runAfter(const Timestamp& time, const TimerCallback&& cb) {
  Timerstamp time(addTime(Timestamp::now(), delay));
  return runAt(time, cb);		  
}

TimerId EventLoop::runEvery(double interval, TimerCallback&& cb) {
  Timestamp time(addTime(Timestamp::now(), interval));
  return timerQueue_->addTimer(std::move(cb), time, interval));
}
#endif

void EventLoop::cancel(TimerId timerId) {
  return timerQueue_->cancel(timerId);
}

void EventLoop::updateChannel(Channel* channel) {
  assert(channel->ownerLoop() == this);
  assertInLoopThread();
  poller_->updateChannel(channel);
}

void EventLoop::removeChannel(Channel* channel) {
  assert(channel->ownerLoop() == this) ;
  assertInLoopThread();
  if (eventHandling_) {
    assert(currentActiveChannel_ == channel ||
	   std::find(activeChannels_.begin(), activeChannels_.end(), channel) == activeChannels_.end());
  }
  poller_->removeChannel(channel);
}

void EventLoop::abortNotInLoopThread() {
  LOG_FATAL << "EventLoop::abortNotInLoopThread - EventLoop " << this
	    << " was created in  threadId_ = " << threadId_
	    << ", current thread id = " << CurrentThread::tid();
}

void EventLoop::wakeup() {
  uint64_t one = 1;
  ssize_t n = sockets::write(wakeupFd_, &one, sizeof one);
  if (n != sizeof one) {
    LOG_ERROR << "EventLoop::wakeup() writes " << n << "btyes instead of 8";
  }
}

void EventLoop::handleRead() {
  uint64_t one = 1;
  ssize_t n = sockets::read(wakeupFd_, &one, sizeof one);
  if (n != sizeof one) {
    LOG_ERROR << "EventLoop::handleRead() reads " << n << " btyes instead of 8";
  }
}

void EventLoop::doPendingFunctors() {
  std::vector<Functor> functors;
  callingPendingFunctors_ = true;

  {
    MutexLockGuard lock(mutex_);
    functors.swap(pendingFunctors_);    
  }

  for (size_t i = 0; i < functors.size(); ++i) {
    functors[i]();
  }
  callingPendingFunctors_ = false;
}

void EventLoop::printActiveChannels() const {
  for (ChannelList::const_iterator it = activeChannels_.begin();
       it != activeChannels_.end(); ++it) {
    const Channel* ch = *it;
    LOG_TRACE << "{" << ch->reventsToString() << "} ";    
  }
}
