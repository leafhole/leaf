
#include "gtest/gtest.h"
#include <iostream>
#include <base/LogFile.h>
#include <base/AsyncLogging.h>
#include <sys/time.h>
//using std::cout;
//using std::endl;

using namespace leaf;
using leaf::AsyncLogging;

void breakhere(){};

using namespace std;
int main(int argc, char* argv[]) {
  //AsyncLogging logger;
  AsyncLogging aLogger("logtest", 10);
  aLogger.start();

  std::string line = "async log test\n";
  for (int i = 0 ;i < 100000; i ++) 
    aLogger.append(line.c_str(), line.size());
  //sleep(5);
  //struct timeval tstop;
  //gettimeofday(&tstop, NULL);

  //sleep(5);
  aLogger.stop();  
  //cout << "tstop " << tstop.tv_sec << "\t" << tstop.tv_usec << endl;
  return 0;
}

