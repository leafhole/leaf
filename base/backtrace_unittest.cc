#include "gtest/gtest.h"
#include <iostream>

#include <execinfo.h>
#include <stdio.h>


//using std::cout;
//using std::endl;

void breakhere(){};

/*
TEST(ATOMICTEST, output) {
  int testNum = 5;
  AtomicInt32 i(testNum);
  EXPECT_EQ(testNum, i.get());
  EXPECT_EQ(testNum + 1, i.incrementAndGet());  
}
*/

TEST(BACKTRACE, output) {

  void* callstack[128];
  int i, frames = backtrace(callstack, 128);
  char** strs = backtrace_symbols(callstack, frames);
  for (i = 0; i < frames; ++i) {
    printf("%s\n", strs[i]);
  }
  free(strs);
}



