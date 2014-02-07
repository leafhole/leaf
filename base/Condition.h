#ifndef BASE_CONDITION_H
#define BASE_CONDITION_H

#include <boost/noncopyable.hpp>
#include <base/Mutex.h>


namespace muduo {

class Condition : boost::noncopyable {
 public:
  explicit Condition(MutexLock& mutex)
    : mutex_(mutex) {    
    MCHECK(pthread_cond_init(&pcond_, NULL));
  }

  ~Condition() {
    MCHECK(pthread_cond_destroy(&pcond_));
  }

  void wait() {
    MutexLock::UnassignGuard ug(mutex_);
    // Haha, if need, just give a interface to the API
    MCHECK(pthread_cond_wait(&pcond_, mutex_.getPthreadMutex()));
  }

  // return true if time out, false otherwise
  bool waitForSeconds(int seconds);

  void notify() {
    MCHECK(pthread_cond_signal(&pcond_));
  }

  void notifyAll() {
    MCHECK(pthread_cond_broadcast(&pcond_));
  }  
            
 private:
  MutexLock& mutex_;
  pthread_cond_t pcond_;
}; // end of class Condition

}  // end of namespace muduo





#endif // BASE_CONDITION_H
