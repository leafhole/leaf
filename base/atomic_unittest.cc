#include "gtest/gtest.h"
#include <iostream>
#include <base/LogFile.h>
#include <base/Atomic.h>
//using std::cout;
//using std::endl;
using leaf::AtomicInt32;

void breakhere(){};

TEST(ATOMICTEST, output) {
  int testNum = 5;
  AtomicInt32 i(testNum);
  EXPECT_EQ(testNum, i.get());
  EXPECT_EQ(testNum + 1, i.incrementAndGet());  
}

