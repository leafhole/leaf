#ifndef BASE_ASYNCLOGGING_H
#define BASE_ASYNCLOGGING_H

#include <base/BlockingQueue.h>
#include <base/BoundedBlockingQueue.h>
#include <base/CountDownLatch.h>
#include <base/Mutex.h>
#include <base/Thread.h>

#include <base/LogStream.h>
//#include <base/Logging.h>

#include <boost/bind.hpp>
#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/ptr_container/ptr_vector.hpp>

namespace leaf {
class AsyncLogging : boost::noncopyable {
 public:
  AsyncLogging(const string& basename,
	       size_t rollSize,
	       int flushInterval = 3) ;
  ~AsyncLogging() {
    if (running_) {
      stop();
    }
  }

  void append(const char* logline, int len);

  void start() {
    running_ = true;
    thread_.start();
    sleep(1);
    latch_.wait();
  }

  void stop() {
    running_ = false;
    cond_.notify();
    thread_.join();
  }

 private:
  AsyncLogging(const AsyncLogging&);
  void operator=(const AsyncLogging&);

  void threadFunc();

  typedef muduo::detail::FixedBuffer<muduo::detail::kLargeBuffer> Buffer;
  typedef boost::ptr_vector<Buffer> BufferVector;
  typedef BufferVector::auto_type BufferPtr;

  const int flushInterval_;
  bool running_;
  string basename_;
  size_t rollSize_;
  muduo::Thread thread_;
  muduo::CountDownLatch latch_;
  muduo::MutexLock mutex_;
  muduo::Condition cond_;
  BufferPtr currentBuffer_; // buffer in use, writting...
  BufferPtr nextBuffer_;    // next buffer waitting written now
  BufferVector buffers_;   // buffers with data. next step will be swap(buffersToWrite)
  
}; // end of class AsyncLogging

/*
  
#define ALOG_TRACE if (muduo::Logger::logLevel() <= muduo::Logger::TRACE) \
   muduo::Logger(__FILE__, __LINE__, muduo::Logger::TRACE, __FUNCTION__).stream()
#define ALOG_DEBUG if (muduo::Logger::logLevel() <= muduo::Logger::DEBUG) \
   muduo::Logger(__FILE__, __LINE__, muduo::Logger::DEBUG, __FUNCTION__).stream()
#define ALOG_INFO if (muduo::Logger::logLevel() <= muduo::Logger::INFO) \
   muduo::Logger(__FILE__, __LINE__, muduo::Logger::INFO, __FUNCTION__).stream()
#define ALOG_WARN if (muduo::Logger::logLevel() <= muduo::Logger::INFO) \
   muduo::Logger(__FILE__, __LINE__, muduo::Logger::WARN, __FUNCTION__).stream()
#define ALOG_ERROR if (muduo::Logger::logLevel() <= muduo::Logger::INFO) \
   muduo::Logger(__FILE__, __LINE__, muduo::Logger::ERROR, __FUNCTION__).stream()
#define ALOG_FATAL if (muduo::Logger::logLevel() <= muduo::Logger::INFO) \
   muduo::Logger(__FILE__, __LINE__, muduo::Logger::FATAL, __FUNCTION__).stream()
// fixme bool should not be parameter 
#define ALOG_SYSERR if (muduo::Logger::logLevel() <= muduo::Logger::INFO) \
   muduo::Logger(__FILE__, __LINE__, false).stream()  
#define ALOG_SYSFATAL if (muduo::Logger::logLevel() <= muduo::Logger::INFO) \
   muduo::Logger(__FILE__, __LINE__, true).stream()

*/
 
} // end of namespace leaf


#endif // BASE_ASYNCLOGGING_H
