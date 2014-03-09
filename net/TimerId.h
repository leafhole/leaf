#ifndef NET_TIMERID_H
#define NET_TIMERID_H
#include <base/copyable.h>

namespace leaf {
  
namespace net {

class Timer;

///
/// An opaque identifier, for candeling Timer.
///
class TimerId : public copyable {
 public:
  TimerId()
    : timer_(NULL),
    sequence_(0) {
  }

  TimerId(Timer* timer, int64_t seq)
    : timer_(timer),
    sequence_(seq) {
    }

  // default copy-ctor, dtor and assignment are okay

  friend class TimerQueue;

 private:
  Timer* timer_;
  int64_t sequence_;
};

} // end of namespace net

} // end of namespace leaf
#endif
