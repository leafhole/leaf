#include "LogStream.h"
#include "gtest/gtest.h"
#include <iostream>
#include <base/LogFile.h>
#include <base/Logging.h>
using std::cout;
using std::endl;
using leaf::LogStream;
using leaf::LogFile;
//using leaf::LogStream::Buffer;

int g_total;
FILE* g_file;
boost::scoped_ptr<LogFile> g_logFile;

void dumpOutput(const char* msg, int len) {
  g_total += len;
  if (g_file) {
    fwrite(msg, 1, len, g_file);
  } else if(g_logFile) {
    g_logFile->append(msg, len);
  }
}

void bench(const char* type) {
  leaf::Logger::setOutput(dumpOutput);
  leaf::Timestamp start(leaf::Timestamp::now());
  g_total = 0;

  int n = 1000 * 1000;
  const bool kLongLog = false;
  leaf::string empty = " ";
  leaf::string longStr(3000, 'X');
  longStr += " ";
  for (int i = 0; i < n; i ++) {
    LOG_INFO << "Hello 0123456789" << " abcdefghijklmnopqrstuvwxyz"
	     << (kLongLog ? longStr : empty)
	     << i;
  }
  leaf::Timestamp end(leaf::Timestamp::now());
  double seconds = timeDifference(end, start);
  printf("%12s: %f seconds, %d bytes, %10.2f msg/s, %.2f MiB/s\n",
	 type, seconds, g_total, n / seconds, g_total / seconds / (1024 * 1024));    
}

void logInThread() {
  LOG_INFO << "logInThread";
  usleep(1000);
}
  


void breakhere(){};

TEST(LOGINGTEST, output) {
  char buffer[64 * 1024];

  g_file = fopen("/dev/null", "w");
  setbuffer(g_file, buffer, sizeof buffer);
  bench("/dev/dev");
  fclose(g_file);  

  g_file = fopen("/tmp/log", "w");
  setbuffer(g_file, buffer, sizeof buffer);
  bench("/tmp/log");
  fclose(g_file);
  
  g_file = NULL;
  //setbuffer(g_, buffer, sizeof buffer);
  g_logFile.reset(new leaf::LogFile("test_log_stat", 500 * 1000 * 1000, LogFile::SINGLE_THREAD));
  bench("test_log_stat");


  g_file = NULL;
  //setbuffer(g_, buffer, sizeof buffer);
  g_logFile.reset(new leaf::LogFile("test_log_stat_mt", 500 * 1000 * 1000, LogFile::MULTI_THREAD));
  bench("test_log_stat");
  
}

