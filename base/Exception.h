#ifndef BASE_EXCEPTION_H
#define BASE_EXCEPTION_H

#include <base/Types.h>
#include <exception>

namespace leaf {

class Exception : public std::exception {
 public:
  explicit Exception(const char* what);
  explicit Exception(const string& what);
  virtual ~Exception() throw();
  virtual const char* what() const throw();
  const char* stackTrace() const throw();

 private:
  void fillStackTrace();

  string message_;
  string stack_;
};

} // end of namespace leaf



#endif // end of BASE_EXCEPTION_H
