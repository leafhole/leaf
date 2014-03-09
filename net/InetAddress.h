#ifndef INET_ADDRESS_H
#define INET_ADDRESS_H
#include <base/copyable.h>
#include <base/StringPiece.h>

#include <netinet/in.h>

namespace leaf {
namespace net {

///
/// Wrapper of sockaddr_in
///
/// This is an POD interface class.
class InetAddress : public copyable {
 public:
  /// Constructs an endpoint with given port number.
  /// Mostly used in TcpServer listening.
  explicit InetAddress(uint16_t port, bool loopbackOnly = false);

  /// Constructs an endpoint with given ip and port
  /// @c ip should be "1.2.3.4"
  InetAddress(const StringPiece& ip, uint16_t port);

  /// Constructs on endpoint with given struct @c sockaddr_in
  /// Mostly used when accepting new connections
  InetAddress(const struct sockaddr_in& addr)
    : addr_(addr)
  {}

  string toIp() const;
  string toIpPort() const;
  string toHostPort() const __attribute__ ((deprecated))
  { return toIpPort(); }

  // default copy/assignment are Okay

  const struct sockaddr_in& getSockAddrInet() const { return addr_; }
  void setSockAddrInet(const struct sockaddr_in& addr) { addr_ = addr; }

  uint32_t ipNetEndian() const { return addr_.sin_addr.s_addr; }
  uint16_t portNetEndian() const { return addr_.sin_port; }

  // resolve hostname to IP address, not changing port or sin_family
  // return true on success.
  // thread safe
  static bool resolve(const char* hostname, InetAddress* result);

 private:
  struct sockaddr_in addr_;
  
}; // end of class InetAddress
} // end of namespace net
} // end of namespace leaf

#endif
