#include <net/EventLoopThreadPool.h>
#include <net/EventLoop.h>
#include <net/EventLoopThread.h>

#include <boost/bind.hpp>

using namespace leaf;
using namespace leaf::net;

EventLoopThreadPool::EventLoopThreadPool(EventLoop* baseLoop)
  : baseLoop_(baseLoop),
   started_(false),
   numThreads_(0),
   next_(0)
{
}

EventLoopThreadPool::~EventLoopThreadPool() {
  // Don't delete loop, it's stack variable
}

void EventLoopThreadPool::start(const ThreadInitCallback& cb) {

  assert(!started_);
  baseLoop_->assertInLoopThread();

  started_ = true;

  for (int i = 0; i < numThreads_; ++i) {
    EventLoopThread* t = new EventLoopThread(cb);
    threads_.push_back(t);
    loops_.push_back(t->startLoop());
  }
  if (numThreads_ == 0 && cb) {
    cb(baseLoop_);
  }  
}

EventLoop* EventLoopThreadPool::getNextLoop() {
  baseLoop_->assertInLoopThread();
  EventLoop* loop = baseLoop_;

  if (!loops_.empty()) {
    // round-robin
    loop = loops_[next_];
    ++next_; // why not use next_ = next_ % loops_.size()
    if (implicit_cast<size_t>(next_) >= loops_.size()) {
      next_ = 0;
    }
  }
  return loop;
}
