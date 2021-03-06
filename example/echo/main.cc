#include "echo.h"

#include <base/Logging.h>
#include <net/EventLoop.h>

int main()
{
  LOG_INFO << "pid = " << getpid();
  leaf::net::EventLoop loop;
  leaf::net::InetAddress listenAddr(2007);
  EchoServer server(&loop, listenAddr);
  server.start();
  loop.loop();
}

