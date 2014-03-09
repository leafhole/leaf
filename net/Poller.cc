#include <net/Poller.h>

using namespace leaf;
using namespace leaf::net;

Poller::Poller(EventLoop* loop)
  : ownerLoop_(loop) {
}

Poller::~Poller() {
}
