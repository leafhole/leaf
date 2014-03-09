#include "echo.h"

#include <base/Logging.h>

#include <boost/bind.hpp>

// using namespace leaf;
// using namespace leaf::net;

EchoServer::EchoServer(leaf::net::EventLoop* loop,
                       const leaf::net::InetAddress& listenAddr)
  : server_(loop, listenAddr, "EchoServer")
{
  server_.setConnectionCallback(
      boost::bind(&EchoServer::onConnection, this, _1));
  server_.setMessageCallback(
      boost::bind(&EchoServer::onMessage, this, _1, _2, _3));
}

void EchoServer::start()
{
  server_.start();
}

void EchoServer::onConnection(const leaf::net::TcpConnectionPtr& conn)
{
  LOG_INFO << "EchoServer - " << conn->peerAddress().toIpPort() << " -> "
           << conn->localAddress().toIpPort() << " is "
           << (conn->connected() ? "UP" : "DOWN");
}

void EchoServer::onMessage(const leaf::net::TcpConnectionPtr& conn,
                           leaf::net::Buffer* buf,
                           leaf::Timestamp time)
{
  leaf::string msg(buf->retrieveAllAsString());
  LOG_INFO << conn->name() << " echo " << msg.size() << " bytes, "
           << "data received at " << time.toString();
  conn->send(msg);
}

