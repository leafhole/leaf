#ifndef BASE_DATE_H
#define BASE_DATE_H

#include <base/copyable.h>
#include <base/Types.h>

struct tm;

namespace leaf {

class Date :  public leaf::copyable {
  public:
    struct YearMonthDay {
      int year; // [1900..2500]
      int month; // [1..12]
      int day; // [1..31]
    };

    static const int kDaysPerWeek = 7;
    static const int kJulianDayOf1970_01_01;

    ///
    /// Constructs an invalid Date.
    ///
    Date()
      : julianDayNumber_(0) {
    }

    ///
    /// Constructs a yyyy-mm-dd Date
    ///
    /// 1 <= month <= 12
    Date(int year, int month, int day);

    ///
    /// Constructs a Date from Julian Day Number.
    ///
    explicit Date(int julianDayNumber)
      : julianDayNumber_(julianDayNumber)
    {}

    ///
    /// Constructs a Date from struct tm
    ///
    explicit Date(const struct tm&);

    // default copy/assignment/dtor are Okay

    void swap(Date& that) {
      std::swap(julianDayNumber_, that.julianDayNumber_);
    }

    bool valid() const { return julianDayNumber_ > 0; }

    ///
    /// Convert to yyyy-mm-dd format
    ///
    string toIsoString() const;

    struct YearMonthDay yearMonthDay() const;

    int year() const {
      return yearMonthDay().year;
    }
    
    int month() const {
      return yearMonthDay().month;
    }
    
    int day() const {
      return yearMonthDay().day;
    }

    // [0, 1, ..., 6] => [Sunday, Monday, ..., Saturday]
    int weekDay() const {
      return (julianDayNumber_ + 1) % kDaysPerWeek;
    }

    int julianDayNum() const { return julianDayNumber_; }    

  private:
    int julianDayNumber_;
}; // end of class Date

inline bool operator < (Date x, Date y) {
  return x.julianDayNum() < y.julianDayNum();
}

inline bool operator == (Date x, Date y) {
  return x.julianDayNum() == y.julianDayNum();
}
 
} // end of namespace leaf


#endif // end of #ifndef BASE_DATE_H
