#include "gtest/gtest.h"
#include <iostream>
#include <base/CurrentThread.h>
#include <base/Thread.h>

#include <string>
#include <boost/bind.hpp>
#include <stdio.h>


void mysleep(int seconds) {
  timespec t = {seconds, 0};
  nanosleep(&t, NULL);
}

void threadFunc() {
  printf("tid=%d\n", leaf::CurrentThread::tid());
}

void threadFunc2(int x) {
  printf("tid=%d,  x=%d\n", leaf::CurrentThread::tid(), x);
}

void threadFunc3() {
  printf("tid=%d\n", leaf::CurrentThread::tid());
  mysleep(1);
}

class Foo {
public:
  explicit Foo(double x)
    : x_(x) {
  }

  void memberFunc() {
    printf("tid=%d, Foo::x_=%f\n", leaf::CurrentThread::tid(), x_);
  }

  void memberFunc2(const std::string& text) {
    printf("tid=%d, Foo::x_=%f, text=%s\n", leaf::CurrentThread::tid(), x_, text.c_str());
  }

private:
  double x_;
};


TEST(THREAD, output) {
  printf("pid=%d, tid=%d\n", ::getpid(), leaf::CurrentThread::tid());

  leaf::Thread t1(threadFunc);
  t1.start();
  t1.join();

  leaf::Thread t2(boost::bind(threadFunc2, 42),
		   "thread for free function without argument");
  t2.start();
  t2.join();

  Foo foo(87.53);
  leaf::Thread t3(boost::bind(&Foo::memberFunc, &foo),
		   "thread for member function without argument");
  t3.start();
  t3.join();

  leaf::Thread t4(boost::bind(&Foo::memberFunc2, boost::ref(foo), std::string("Shuo Chen")));
  t4.start();
  t4.join();

  {
    leaf::Thread t5(threadFunc3);
    t5.start();
    // t5 may destruct earlier than thread creation
  }

  mysleep(2);
  {
    leaf::Thread t6(threadFunc3);
    t6.start();
    mysleep(2);
    // t6 destruct later than thread createion. ?? why
  }
  sleep(2);
  printf("number of created threads %d\n", leaf::Thread::numCreated());

}



