set(CMAKE_BUILD_TYPE Debug)

set(base_SRCS
  AsyncLogging.cc
  Condition.cc
  CountDownLatch.cc
  Date.cc
  Exception.cc
  FileUtil.cc
  LogFile.cc
  Logging.cc
  LogStream.cc
  ProcessInfo.cc
  Timestamp.cc
  Thread.cc
  ThreadPool.cc
  )

add_library(leaf_base ${base_SRCS})
target_link_libraries(leaf_base pthread rt)


install(TARGETS leaf_base DESTINATION lib)

INCLUDE_DIRECTORIES("../", "../base/")
INCLUDE_DIRECTORIES("../../googletest-read-only/include/")

file(GLOB HEADERS "*.h")
install(FILES ${HEADERS} DESTINATION include/leaf/base)

add_subdirectory(tests)
