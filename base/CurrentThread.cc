#include <base/CurrentThread.h>
#include <cstdio>
#include <assert.h>
#include <unistd.h>
#include <base/Timestamp.h>

//using namespace muduo;


namespace detail
{

#ifdef __MACH__
mach_port_t gettid() {
  return pthread_mach_thread_np(pthread_self());
}
#else
pid_t gettid() {
  return static_cast<pid_t>(::syscall(SYS_gettid));
}
#endif
};





namespace muduo {
  namespace CurrentThread {
    __thread int t_cachedTid;
    __thread char t_tidString[32];
    __thread const char* t_threadName;
}
}


void muduo::CurrentThread::cacheTid()
{
  if (t_cachedTid == 0)
  {
    t_cachedTid = detail::gettid();
    int n = snprintf(t_tidString, sizeof t_tidString, "%5d ", t_cachedTid);
    assert(n == 6); (void) n;
  }
}

bool muduo::CurrentThread::isMainThread()
{
  return tid() == ::getpid();
}

void muduo::CurrentThread::sleepUsec(int64_t usec)
{
  struct timespec ts = { 0, 0 };
  ts.tv_sec = static_cast<time_t>(usec / Timestamp::kMicroSecondsPerSecond);
  ts.tv_nsec = static_cast<long>(usec % Timestamp::kMicroSecondsPerSecond * 1000);
  ::nanosleep(&ts, NULL);
}
