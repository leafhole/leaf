#ifndef BASE_COUNTDOWNLATCH_H
#define BASE_COUNTDOWNLATCH_H

#include <base/Condition.h>
#include <base/Mutex.h>

#include <boost/noncopyable.hpp>

namespace leaf {
class CountDownLatch : boost::noncopyable {
 public:
  explicit CountDownLatch(int count);
  void wait();
  void countDown();
  int getCount() const;
 private:
  mutable MutexLock mutex_;
  Condition condition_;
  int count_;
}; // end of class CountDownLatch
} // end of namespace leaf 
  
  
    


#endif // BASE_COUNTDOWNLATCH_H
