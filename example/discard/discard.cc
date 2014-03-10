#include "discard.h"
#include <net/TcpServer.h>

//class DiscardServer {

// public:
DiscardServer::DiscardServer(leaf::net::EventLoop* loop,
			     const leaf::net::InetAddress& listenAddr)
  : server_(loop, listenAddr, "DiscardServer")
{
  server_.setConnectionCallback(boost::bind(&DiscardServer::onConnection,
					     this, _1));
  server_.setMessageCallback(boost::bind(&DiscardServer::onMessage,
					  this, _1, _2, _3));
}

void DiscardServer::start() {
  server_.start();
}

void DiscardServer::setThreadNum(int threadNum) {
  server_.setThreadNum(threadNum);
  
}

void DiscardServer::onConnection(const leaf::net::TcpConnectionPtr& conn) {
  LOG_INFO << "DiscardServer - " << conn->peerAddress().toIpPort() << " -> "
           << conn->localAddress().toIpPort() << " is "
           << (conn->connected() ? "UP" : "DOWN");  
}

void DiscardServer::onMessage(const leaf::net::TcpConnectionPtr& conn,
			      leaf::net::Buffer* buf,
			      leaf::Timestamp time){
  //buf->retrieveAll();
  leaf::string msg(buf->retrieveAllAsString());
  LOG_INFO << conn->name() << " discards " << msg.size()
           << " bytes received at " << time.toString();  

}  
  
