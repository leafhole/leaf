#include <base/Condition.h>
#include <errno.h>
#include <time.h>
#include <sys/time.h>

bool muduo::Condition::waitForSeconds(int seconds) {

  struct timespec abstime;

  //clock_gettime(CLOCK_REALTIME, &abstime);
  //abstime.tv_sec += seconds;

  struct timeval tv;
  //struct timespec ts;
  gettimeofday(&tv, NULL);
  abstime.tv_sec = tv.tv_sec + seconds;
  abstime.tv_nsec = tv.tv_usec * 1000;

  
  MutexLock::UnassignGuard ug(mutex_);
  return ETIMEDOUT == pthread_cond_timedwait(&pcond_, mutex_.getPthreadMutex(), &abstime);
}
