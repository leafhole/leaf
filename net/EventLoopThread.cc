#include <net/EventLoopThread.h>
#include <net/EventLoop.h>
#include <boost/bind.hpp>

using namespace leaf;
using namespace leaf::net;

EventLoopThread::EventLoopThread(const ThreadInitCallback& cb)
  : loop_(NULL),
    exiting_(false),
    thread_(boost::bind(&EventLoopThread::threadFunc, this)),
    mutex_(),
    cond_(mutex_),
    callback_(cb)
{
}

EventLoopThread::~EventLoopThread() {
  exiting_ = true;
  if (loop_ != NULL) {
    // not 100% race-free, eg. threadFunc could be running callback_;
    // still a tiny chance to call destructed object, if threadFunc exists just now
    // but when EventLoopThread destructs, usually programming is exiting anyway
    loop_->quit();
    thread_.join();
  }
}

EventLoop* EventLoopThread::startLoop() {
  assert(!thread_.started());
  thread_.start();

  {
    MutexLockGuard lock(mutex_);
    while (loop_ == NULL) {
      cond_.wait();
    }
  }
  return loop_;
}

void EventLoopThread::threadFunc() {
  EventLoop loop;

  if (callback_) {
    callback_(&loop);
  }

  {
    MutexLockGuard lock(mutex_);
    loop_ = &loop; // what's this? FIXME
    cond_.notify();
  }

  loop.loop();

  loop_ = NULL;
}
