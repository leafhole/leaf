#ifndef LEAF_EXAMPLES_DISCARD_H
#define LEAF_EXAMPLES_DISCARD_H

#include <net/TcpServer.h>
#include <boost/bind.hpp>
#include <base/Logging.h>

class DiscardServer {

 public:
  DiscardServer(leaf::net::EventLoop* loop,
		const leaf::net::InetAddress& listenAddr);
  void setThreadNum(int threadNum);  
  void start(); // call server_.start();

 private:

  void onConnection(const leaf::net::TcpConnectionPtr& conn);
  void onMessage(const leaf::net::TcpConnectionPtr& conn,
		 leaf::net::Buffer* buf,
		 leaf::Timestamp time);  
  
  leaf::net::TcpServer server_;
}; // end of discard server


#endif // LEAF_EXAMPLE_DISCARD_H
