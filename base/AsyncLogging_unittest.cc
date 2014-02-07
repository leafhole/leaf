#include "gtest/gtest.h"
#include <iostream>
#include <base/LogFile.h>
#include <base/AsyncLogging.h>
#include <sys/time.h>
//using std::cout;
//using std::endl;

using namespace muduo;
using leaf::AsyncLogging;

void breakhere(){};

using namespace std;
TEST(ASYNCLOG, output) {
  //AsyncLogging logger;
  AsyncLogging aLogger("logtest", 1024*1024);
  aLogger.start();

  std::string line = "async log test\n";
  aLogger.append(line.c_str(), line.size());
  //sleep(5);
  //struct timeval tstop;
  //gettimeofday(&tstop, NULL);
  aLogger.stop();
  //cout << "tstop " << tstop.tv_sec << "\t" << tstop.tv_usec << endl;
}

