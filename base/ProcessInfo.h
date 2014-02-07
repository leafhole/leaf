#ifndef BASE_PROCESSINFO_H
#define BASE_PROCESSINFO_H
#include <base/Types.h>
#include <base/Timestamp.h>
#include <vector>

namespace muduo {

namespace ProcessInfo{
  pid_t pid();
  string pidString();
  uid_t uid();
  string username();
  uid_t euid();
  Timestamp startTime();

  string hostname();
  string procname();

  /// read /proc/self/status
  string procStatus();

  /// read /proc/self/stat
  string procStat();

  /// readlink /proc/self/exe
  string exePath();

  int openedFiles();
  int maxOpenFiles();

  int numThreads();
  std::vector<pid_t> threads();
}

}
    
#endif
