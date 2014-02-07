#include "gtest/gtest.h"
#include <iostream>
#include <base/ThreadPool.h>
#include <base/Logging.h>
#include <base/CountDownLatch.h>

#include <boost/bind.hpp>
#include <stdio.h>

void print() {
  //fprintf(stderr, "%s : %d\n", __FUNCTION__, __LINE__);  
  printf("tid=%d\n", muduo::CurrentThread::tid());
}

void printString(const std::string& str) {
  //fprintf(stderr, "%s : %d\n", __FUNCTION__, __LINE__);
  LOG_INFO << str;
  //usleep(10 * 1000);
}

void test(int maxSize) {
  LOG_WARN << "Test ThreadPool with max queue size = " << maxSize;
  muduo::ThreadPool pool("MainThreadPool");
  pool.setMaxQueueSize(maxSize);
  pool.start(5);

  LOG_WARN << "Adding";
  pool.run(print); // can call directly?
  pool.run(print);
  for (int i = 0; i < 100; i++) {
    char buf[32];
    snprintf(buf, sizeof buf, "task %d", i);
    pool.run(boost::bind(printString, std::string(buf)));
  }
  LOG_WARN << "Done";


  int countDownNum = 1000;
  muduo::CountDownLatch latch(countDownNum);
  for (int i = 0; i < countDownNum; i ++) {
    pool.run(boost::bind(&muduo::CountDownLatch::countDown,
			 &latch));
  }
  latch.wait();
  
  pool.stop();
}

TEST(THREADPOOL, output) {
  test(0);
  test(1);
  test(5);
  test(10);
  test(50);
}



