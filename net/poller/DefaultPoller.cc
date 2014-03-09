#include <net/Poller.h>
#include <net/poller/PollPoller.h>
#include <stdlib.h>

using namespace leaf::net;

Poller* Poller::newDefaultPoller(EventLoop* loop) {
  #ifdef __MACH__
  return new PollPoller(loop);
  #else
  return new PollPoller(loop);
  #endif 
}
