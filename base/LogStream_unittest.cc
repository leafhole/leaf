#include "LogStream.h"
#include "gtest/gtest.h"
#include <iostream>
using std::cout;
using std::endl;
using leaf::LogStream;
//using leaf::LogStream::Buffer;
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

