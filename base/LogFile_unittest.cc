#include "LogStream.h"
#include "gtest/gtest.h"
#include <iostream>
#include <base/LogFile.h>
using std::cout;
using std::endl;
using muduo::LogStream;
using muduo::LogFile;
//using muduo::LogStream::Buffer;

boost::scoped_ptr<LogFile> g_logFile;

void outputFunc(const char* msg, int len) {
  g_logFile->append(msg, len);
}

void flushFunc() {
  g_logFile->flush();
}

void breakhere(){};

TEST(LOGTEST, output) {
  const char* filename = "logtest";
  g_logFile.reset(new LogFile(filename, 200 * 1000));
  //Logger::setOutput(outputFunc);
  //Logger::setFlush(flushFunc);

  breakhere();
  
  string line = "1234567890 abcdefghijklmnopqrstuvwxyz ABCDEFGHIJKLMNOPQRSTUVWXYZ \n";
  for (int i = 0; i < 10000; i ++ ) {
    g_logFile->append(line.c_str(), line.size());
  }
}




int Factorial(int n) {
  int result = 1;
  for (int i = 1; i <= n; i++) {
    result *= i;
  }

  return result;
}


TEST(FactorialTest, Zero) {
  EXPECT_EQ(1, Factorial(0));
}



TEST(LogStreamTest, output) {
  LogStream logStream;
  logStream << 1;
  logStream << "abcdefg";
  logStream << 'n';
  //const char* z = "12";
  //const void * zp = z;
  //logStream << zp;

  const LogStream::Buffer& buffer = logStream.buffer();
  cout << buffer.asString() << endl;  
  EXPECT_EQ("1abcdefgn", buffer.asString());
}

