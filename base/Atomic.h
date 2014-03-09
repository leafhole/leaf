#ifndef BASE_ATOMIC_H
#define BASE_ATOMIC_H

#include <boost/noncopyable.hpp>
#include <stdint.h>

namespace leaf {
namespace detail{
template<typename T>
  class AtomicIntegerT : boost::noncopyable {
 public:
  explicit AtomicIntegerT()
    : value_(0) {
  }

  explicit AtomicIntegerT(T value)
    : value_(value) {
  }

  AtomicIntegerT(const AtomicIntegerT& that)
    : value_(that.get()) {
  }

  AtomicIntegerT& operator=(const AtomicIntegerT& that) {
    getAndSet(that.get());
    return *this;
  }

  T get() {
    return __sync_val_compare_and_swap(&value_, 0, 0);
  }

  T getAndAdd(T x) {
    return __sync_fetch_and_add(&value_, x);
  }

  T addAndGet(T x) {
    return getAndAdd(x) + x;
  }

  T incrementAndGet() {
    return addAndGet(1);
  }

  T decrementAndGet() {
    return addAndGet(-1);
  }

  void add(T x) {
    getAndAdd(x);
  }

  void increment() {
    incrementAndGet();
  }

  void decrement() {
    decrementAndGet();
  }

  T getAndSet(T newValue) {
    return __sync_lock_test_and_set(&value_, newValue);
  }

 private:
  volatile T value_;
 };
} // end of namespace detail

 typedef detail::AtomicIntegerT<int32_t> AtomicInt32;
 typedef detail::AtomicIntegerT<int64_t> AtomicInt64;
} // end of namespace leaf
    

#endif
