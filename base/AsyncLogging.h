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

  typedef leaf::detail::FixedBuffer<leaf::detail::kLargeBuffer> Buffer;
  typedef boost::ptr_vector<Buffer> BufferVector;
  typedef BufferVector::auto_type BufferPtr;

  const int flushInterval_;
  bool running_;
  string basename_;
  size_t rollSize_;
  leaf::Thread thread_;
  leaf::CountDownLatch latch_;
  leaf::MutexLock mutex_;
  leaf::Condition cond_;
  BufferPtr currentBuffer_; // buffer in use, writting...
  BufferPtr nextBuffer_;    // next buffer waitting written now
  BufferVector buffers_;   // buffers with data. next step will be swap(buffersToWrite)
  
}; // end of class AsyncLogging

/*
  
#define ALOG_TRACE if (leaf::Logger::logLevel() <= leaf::Logger::TRACE) \
   leaf::Logger(__FILE__, __LINE__, leaf::Logger::TRACE, __FUNCTION__).stream()
#define ALOG_DEBUG if (leaf::Logger::logLevel() <= leaf::Logger::DEBUG) \
   leaf::Logger(__FILE__, __LINE__, leaf::Logger::DEBUG, __FUNCTION__).stream()
#define ALOG_INFO if (leaf::Logger::logLevel() <= leaf::Logger::INFO) \
   leaf::Logger(__FILE__, __LINE__, leaf::Logger::INFO, __FUNCTION__).stream()
#define ALOG_WARN if (leaf::Logger::logLevel() <= leaf::Logger::INFO) \
   leaf::Logger(__FILE__, __LINE__, leaf::Logger::WARN, __FUNCTION__).stream()
#define ALOG_ERROR if (leaf::Logger::logLevel() <= leaf::Logger::INFO) \
   leaf::Logger(__FILE__, __LINE__, leaf::Logger::ERROR, __FUNCTION__).stream()
#define ALOG_FATAL if (leaf::Logger::logLevel() <= leaf::Logger::INFO) \
   leaf::Logger(__FILE__, __LINE__, leaf::Logger::FATAL, __FUNCTION__).stream()
// fixme bool should not be parameter 
#define ALOG_SYSERR if (leaf::Logger::logLevel() <= leaf::Logger::INFO) \
   leaf::Logger(__FILE__, __LINE__, false).stream()  
#define ALOG_SYSFATAL if (leaf::Logger::logLevel() <= leaf::Logger::INFO) \
   leaf::Logger(__FILE__, __LINE__, true).stream()

*/
 
} // end of namespace leaf


#endif // BASE_ASYNCLOGGING_H
