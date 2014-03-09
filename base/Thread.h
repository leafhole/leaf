#ifndef BASE_THREAD_H
#define BASE_THREAD_H

#include <base/Atomic.h>
#include <base/Types.h>

#include <boost/function.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <pthread.h>

namespace leaf {
class Thread : boost::noncopyable {
 public:
  typedef boost::function<void ()> ThreadFunc;

  explicit Thread(const ThreadFunc&, const string& name = string());
#ifdef __GXX_EXPERIMENTAL_CXX0X__
  explicit Thread(ThreadFunc&&, const string& name = string());
#endif
  ~Thread();

  void start();
  int join(); // return pthread_join();

  bool started() const { return started_; }
  // phtread_t pthreadId() const { return pthreadId_; }
  pid_t tid() const { return *tid_; }
  const string& name() const { return name_; }

  static int numCreated() { return numCreated_.get(); }

 private:
  bool started_;
  bool joined_;
  pthread_t pthreadId_;
  boost::shared_ptr<pid_t> tid_;
  ThreadFunc func_;
  string name_;

  static AtomicInt32 numCreated_;  
}; // end of class Thread

} // end of namespace leaf

#endif // BASE_THREAD_H
