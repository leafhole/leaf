#ifndef BASE_BOUNDEDBLOCKINGQUEUE_H
#define BASE_BOUNDEDBLOCKINGQUEUE_H

#include <base/Condition.h>
#include <base/Mutex.h>

#include <boost/circular_buffer.hpp>
#include <boost/noncopyable.hpp>
#include <assert.h>

using muduo::MutexLockGuard;

namespace leaf {
template <typename T>
class BoundedBlockingQueue : boost::noncopyable {
 public:
  explicit BoundedBlockingQueue(int maxSize)
    :mutex_(),
    notEmpty_(mutex_),
    notFull_(mutex_),
    queue_(maxSize) {
    }

  void put(const T& x) {
    MutexLockGuard lock(mutex_);
    while (queue_.full()) {
	notFull_.wait();
    }

    assert(not queue_.full());
    queue_.push_back(x);
    notEmpty_.notify();
  }

  T take() {
    MutexLockGuard lock(mutex_);
    while (queue_.empty()) {
      notEmpty_.wait();
    }

    assert(not queue_.empty());
    T front(queue_.front());
    queue_.pop_front();
    notFull_.notify();
    return front;
  }

  bool empty() const {
    MutexLockGuard lock(mutex_);
    return queue_.empty();
  }
  
  bool full() const {
    MutexLockGuard lock(mutex_);
    return queue_.full();
  }

  size_t size() const {
    MutexLockGuard lock(mutex_);
    return queue_.size();
  }

  size_t capacity() const {
    MutexLockGuard lock(mutex_);
    return queue_.capacity();
  }

 private:
  mutable muduo::MutexLock mutex_;
  muduo::Condition notEmpty_;
  muduo::Condition notFull_;
  boost::circular_buffer<T> queue_;

}; // end of class BoundedBlockingQueue
  
} // end of namespace leaf

#endif // BASE_BOUNDEDBLOCKINGQUEUE_H