#include "discard.h"

#include <base/Logging.h>
#include <net/EventLoop.h>

int main()
{
  LOG_INFO << "pid = " << getpid();
  leaf::net::EventLoop loop;
  leaf::net::InetAddress listenAddr(2006);
  DiscardServer server(&loop, listenAddr);
  //server.setThreadNum(10);
  server.start();
  loop.loop();
}

