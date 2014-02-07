#include <base/AsyncLogging.h>
#include <base/LogFile.h>
#include <base/Timestamp.h>

#include <stdio.h>
#include <iostream>
#include <sys/time.h>
using namespace std;

using namespace muduo;
using leaf::AsyncLogging;

AsyncLogging::AsyncLogging(const string& basename,
			   size_t rollSize,
			   int flushInterval)
  : flushInterval_(flushInterval),
    running_(false),
    basename_(basename),
    rollSize_(rollSize),
    thread_(boost::bind(&AsyncLogging::threadFunc, this), "Logging"),
    latch_(1),
    mutex_(),
    cond_(mutex_),
    currentBuffer_(new Buffer),
    nextBuffer_(new Buffer),
    buffers_()
{
  currentBuffer_->bzero();
  nextBuffer_->bzero();
  buffers_.reserve(16);  
}

void AsyncLogging::append(const char* logline, int len) {
  muduo::MutexLockGuard lock(mutex_);
  if (currentBuffer_->avail() > len) {
    currentBuffer_->append(logline, len);
  } else {
    buffers_.push_back(currentBuffer_.release());

    if (nextBuffer_) {
      currentBuffer_ = boost::ptr_container::move(nextBuffer_); // nextBuffer_ be empty?
    } else {
      currentBuffer_.reset(new Buffer); // rarely happen, nextBuffer is unavailable
    }
    currentBuffer_->append(logline, len);
    cond_.notify();
  }    
}

void AsyncLogging::threadFunc() {
  assert(running_ == true);
  // here, after count down, the AsyncLogging::start can return,
  // Is countDown after buffer ready necessary? TODO, FIXME
  latch_.countDown();  
  LogFile output(basename_, rollSize_);

  BufferPtr newBuffer1(new Buffer);
  BufferPtr newBuffer2(new Buffer);
  newBuffer1->bzero();
  newBuffer2->bzero();
  
  BufferVector buffersToWrite;
  buffersToWrite.reserve(16);

  while (running_) {
    // writing writing writing
    { // collect buffer with data to write
      MutexLockGuard lock(mutex_);
      if (buffers_.empty()) {
	cond_.waitForSeconds(flushInterval_); // max wait time flushInterval_
      }
      // disable write of currentBuffer_,
      buffers_.push_back(currentBuffer_.release()); 
      currentBuffer_ = boost::ptr_container::move(newBuffer1);
      buffersToWrite.swap(buffers_);
      if (not nextBuffer_) {
	nextBuffer_ = boost::ptr_container::move(newBuffer2);
      }
    }

    assert( not buffersToWrite.empty());

    if (buffersToWrite.size() > 25) {
      // too many buffers, should shrink, why not output first and drop again?
      char buf[256]; // too many buffer to write?? just throw?
      snprintf(buf, sizeof buf, "Dropped log message at %s, %zd larger buffers\n",
	       Timestamp::now().toFormattedString().c_str(),
	       buffersToWrite.size() - 2);
      fputs(buf, stderr);
      output.append(buf, static_cast<int>(strlen(buf)));
      // only 2 left after shrink
      buffersToWrite.erase(buffersToWrite.begin() + 2, buffersToWrite.end());      
    }

    for (size_t i = 0; i < buffersToWrite.size(); i++) {
      output.append(buffersToWrite[i].data(), buffersToWrite[i].length());
    }

    if (buffersToWrite.size() > 2) {
      buffersToWrite.resize(2);
    } 

    if (not newBuffer1) {
      assert(not buffersToWrite.empty());
      newBuffer1 = buffersToWrite.pop_back();
      newBuffer1->reset();
    }

    if (not newBuffer2) {
      assert(not buffersToWrite.empty());
      newBuffer2 = buffersToWrite.pop_back();
      newBuffer2->reset();
    }

    buffersToWrite.clear();
    output.flush();
  } // end of while (running_)

  output.flush();
}
