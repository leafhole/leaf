#ifndef BASE_CURRENT_CURRENTTHREAD_H
#define BASE_CURRENT_CURRENTTHREAD_H

#include <stdint.h>
#include <pthread.h>

namespace leaf {
namespace CurrentThread {
  // interval
  extern __thread int t_cachedTid;
  extern __thread char t_tidString[32];
  extern __thread const char* t_threadName;
  void cacheTid();

  inline int tid() {
    if (__builtin_expect(t_cachedTid == 0, 0)) {
      cacheTid();
    }
    return t_cachedTid;
  }

  inline const char* tidString() { // for logging
    return t_tidString;
  }

  inline const char* name() {
    return t_threadName;
  }

  bool isMainThread();

  void sleepUsec(int64_t usec);
    
};

};

namespace detail {
#ifdef __MACH__
  mach_port_t gettid();
#else
  pid_t gettid();
#endif
};



#endif  //BASE_CURRENT_CURRENTTHREAD_H
