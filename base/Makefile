all: logstream_unittest logfile_unittest logging_unittest atomic_unittest backtrace_unittest thread_unittest threadpool_unittest asynclogging_main asynclogging_unittest

clean:
	rm -f *.o
	rm -f logstream_unittest logfile_unittest logging_unittest backtrace_unittest thread_unittest atomic_unittest  threadpool_unittest

CC = gcc

INCLUDES = -I ../ -I ../../googletest-read-only/include/ -I ../../

LIBFLAG = -L ../../googletest-read-only/ -lgtest -lgtest_main -lstdc++



CFLAGS = $(INCLUDES) -DMUDUO_STD_STRING -D__MACH__ -g -O0

atomic_unittest: atomic_unittest.o
	$(CC) -o $@ $^ $(LIBFLAG)



logstream_unittest: LogStream.o LogStream_unittest.o
	$(CC) -o $@ $^ $(LIBFLAG)

logfile_unittest: CurrentThread.o LogFile.o LogFile_unittest.o FileUtil.o LogStream.o ProcessInfo.o Timestamp.o Exception.o
	$(CC) -v -o $@ $^ $(LIBFLAG) -g


logging_unittest: CurrentThread.o LogFile.o Logging.o Logging_unittest.o FileUtil.o LogStream.o ProcessInfo.o Timestamp.o Exception.o
	$(CC) -v -o $@ $^ $(LIBFLAG) -g

backtrace_unittest: backtrace_unittest.o 
	$(CC) -v -o $@ $^ $(LIBFLAG) -g

thread_unittest: Thread_unittest.o Thread.o Exception.o Logging.o LogStream.o LogFile.o FileUtil.o Timestamp.o ProcessInfo.o Exception.o
	$(CC) -v -o $@ $^ $(LIBFLAG) -g

threadpool_unittest: ThreadPool_unittest.o ThreadPool.o CountDownLatch.o Thread.o Exception.o Logging.o LogStream.o LogFile.o FileUtil.o Timestamp.o ProcessInfo.o Exception.o
	$(CC) -v -o $@ $^ $(LIBFLAG) -g

asynclogging_unittest: AsyncLogging_unittest.o AsyncLogging.o Condition.o CountDownLatch.o Thread.o Exception.o Logging.o LogStream.o LogFile.o FileUtil.o Timestamp.o ProcessInfo.o Exception.o
	$(CC) -v -o $@ $^ $(LIBFLAG) -g
asynclogging_main: AsyncLogging_main.o AsyncLogging.o Condition.o CountDownLatch.o Thread.o Exception.o Logging.o LogStream.o LogFile.o FileUtil.o Timestamp.o ProcessInfo.o Exception.o
	$(CC) -v -o $@ $^ $(LIBFLAG) -g

%.o:%.cc
	$(CC) $(CFLAGS) -c -o $@ $<
