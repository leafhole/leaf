#ifndef BASE_TIMESTAMP_H
#define BASE_TIMESTAMP_H

#include <base/copyable.h>
#include <base/Types.h>

#include <boost/operators.hpp>

namespace muduo {

class Timestamp : public muduo::copyable,
  public boost::less_than_comparable<Timestamp> {
 public:

    Timestamp()
      : microSecondsSinceEpoch_(0) {    
    }
    
    explicit Timestamp(int64_t microSecondsSinceEpoch);

    void swap(Timestamp& that) {
      std::swap(microSecondsSinceEpoch_, that.microSecondsSinceEpoch_);
    }

    // default copy/assignment/dtor are Okay

    string toString() const;
    string toFormattedString() const;

    bool valid() const { return microSecondsSinceEpoch_; }

    // for interval usage.
    int64_t microSecondsSinceEpoch() const { return microSecondsSinceEpoch_; }
    time_t secondsSinceEpoch() const
    { return static_cast<time_t>(microSecondsSinceEpoch_ / kMicroSecondsPerSecond); }

    ///
    /// get time of now.
    ///
    static Timestamp now();
    static Timestamp invalid();

    static const int kMicroSecondsPerSecond = 1000 * 1000;

 private:
    int64_t microSecondsSinceEpoch_;  // only one member
  };

 inline bool operator < (Timestamp lhs, Timestamp rhs) {
   return lhs.microSecondsSinceEpoch() < rhs.microSecondsSinceEpoch();
 }

 inline bool operator == (Timestamp lhs, Timestamp rhs) {
   return lhs.microSecondsSinceEpoch() < rhs.microSecondsSinceEpoch();
 }

 /// Get time difference of two timestamps, result in seconds.
 ///
 /// @param high, low
 /// @return (high - low) in seconds
 /// @c double has 52-bit precision, enough for one-microsecond
 /// resolution for next 100 years.
 inline double timeDifference(Timestamp high, Timestamp low) {
   int64_t diff = high.microSecondsSinceEpoch() - low.microSecondsSinceEpoch();
   return static_cast<double>(diff) / Timestamp::kMicroSecondsPerSecond;
 }

 ///
 /// Add @c seconds to given timestamp
 ///
 /// @return timestamp + seconds as Timestamp
 ///
 inline Timestamp addTime(Timestamp timestamp, double seconds) {
   int64_t delta = static_cast<int64_t>(seconds * Timestamp::kMicroSecondsPerSecond);
   return Timestamp(timestamp.microSecondsSinceEpoch() + delta);
 }
 

 
} // end of muduo



#endif // BASE_TIMESTAMP_H
