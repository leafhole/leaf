#ifndef NET_SOCKET_H
#define NET_SOCKET_H

#include <boost/noncopyable.hpp>

namespace leaf {
///
/// TCP networking
///
namespace net {
class InetAddress;

///
/// Wrapper of socket file descriptor
///
/// It closes the sockfd when desctructs.
/// Tt's thread safe, all operations are delagated to OS.
class Socket : boost::noncopyable {
 public:
  explicit Socket(int sockfd)
    : sockfd_(sockfd)
  {}

  // Socket(Socket&&) // move constructor in C++11
  ~Socket();

  int fd() const { return sockfd_; }

  // abort if address in use
  void bindAddress(const InetAddress& localaddr);
  // abort if address in use
  void listen();

  /// On success, return a non-negative integer that is
  /// a descriptor for the accepted socket, which has been
  /// set to non-blocking and close-on-exec. *peeraddr is assigned.
  /// On error, -1 is returned, and *peeraddr is untouched.
  int accept(InetAddress* peeraddr);

  void shutdownWrite();

  ///
  /// Enable/disable TCP_NODELAY (disable/enable Nagle's algorithm).
  ///
  void setTcpNoDelay(bool on);

  ///
  /// Enable/disable SO_REUSEADDR
  ///
  void setReuseAddr(bool on);

  ///
  /// Enable/disable SO_REUSEPORT
  ///
  void setReusePort(bool on);

  ///
  /// Enable/disable SO_KEEPALIVE
  ///
  void setKeepAlive(bool on);
  
 private:
  const int sockfd_;
}; // end of class Socket




} // end of namespace net
} // end of namespace leaf
#endif //NET_SOCKET_H
