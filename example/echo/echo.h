#ifndef MUDUO_EXAMPLES_SIMPLE_ECHO_ECHO_H
#define MUDUO_EXAMPLES_SIMPLE_ECHO_ECHO_H

#include <net/TcpServer.h>

// RFC 862
class EchoServer
{
 public:
  EchoServer(leaf::net::EventLoop* loop,
             const leaf::net::InetAddress& listenAddr);

  void start();  // calls server_.start();

 private:
  void onConnection(const leaf::net::TcpConnectionPtr& conn);

  void onMessage(const leaf::net::TcpConnectionPtr& conn,
                 leaf::net::Buffer* buf,
                 leaf::Timestamp time);

  leaf::net::TcpServer server_;
};

#endif  // MUDUO_EXAMPLES_SIMPLE_ECHO_ECHO_H
