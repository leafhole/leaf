#ifndef BASE_THREADPOOL_H
#define BASE_THREADPOOL_H

#include <base/Condition.h>
#include <base/Mutex.h>
#include <base/Thread.h>
#include <base/Types.h>

#include <boost/function.hpp>
#include <boost/noncopyable.hpp>
#include <boost/ptr_container/ptr_vector.hpp>

#include <deque>

namespace muduo {
class ThreadPool : boost::noncopyable {
 public:
  typedef boost::function<void ()> Task;

  explicit ThreadPool(const string& name = string());
  ~ThreadPool();

  // must be called before start()
  void setMaxQueueSize(int maxSize) {
    maxQueueSize_ =maxSize;
  }

  void start(int numThreads);
  void stop();

  void run(const Task& f);
  

 private:
  bool isFull() const;
  void runInThread();
  Task take();

  MutexLock mutex_;
  Condition notEmpty_;
  Condition notFull_;
  string name_;
  boost::ptr_vector<muduo::Thread> threads_;
  std::deque<Task> queue_;
  size_t maxQueueSize_;
  bool running_;
}; // end of class ThreadPool
 
} // end of namespace muduo


#endif //  BASE_THREADPOOL_H
