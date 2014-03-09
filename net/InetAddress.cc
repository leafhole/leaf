#include <net/InetAddress.h>

#include <base/Logging.h>
#include <net/Endian.h>
#include <net/SocketsOps.h>

#include <netdb.h>
#include <string.h> // bzero
#include <netinet/in.h>

#include <boost/static_assert.hpp>

// INADDR_ANY use (type)value casting.
#pragma GCC diagnostic ignored "-Wold-style-cast"
static const in_addr_t kInaddrAny = INADDR_ANY;
static const in_addr_t kInaddrLoopback = INADDR_LOOPBACK; // only 127.0.0.1
#pragma GCC diagnostic error "-Wold-style-cast"



//      /* Structure describing an Internet socket address. */
//      struct sockaddr_in {
//         sa_family_t sin_family; /* address family: AF_INET */
//         uint16_t      sin_port;  /* port in network byte order */
//         struct in_addr sin_addr; /* internet address */
//      }

//      /* Internet address. */
//      typedef uint32_t int_addr_t;
//      struct in_addr {
//           in_addr_t s_addr; /* address in network byte order */
//      }

using namespace leaf;
using namespace leaf::net;

BOOST_STATIC_ASSERT(sizeof(InetAddress) == sizeof(struct sockaddr_in));


InetAddress::InetAddress(uint16_t port, bool lookbackOnly)
{
  bzero(&addr_, sizeof addr_);
  addr_.sin_family = AF_INET;
  in_addr_t ip = lookbackOnly ? kInaddrLoopback : kInaddrAny;
  addr_.sin_addr.s_addr = sockets::hostToNetwork32(ip);
  addr_.sin_port = sockets::hostToNetwork16(port);
}

InetAddress::InetAddress(const StringPiece& ip, uint16_t port)
{
  bzero(&addr_, sizeof addr_);
  sockets::fromIpPort(ip.data(), port, &addr_);
}

leaf::string InetAddress::toIpPort() const
{
  char buf[32];
  sockets::toIpPort(buf, sizeof buf, addr_);
  return buf;
}

leaf::string InetAddress::toIp() const
{
  char buf[32];
  sockets::toIp(buf, sizeof buf, addr_);
  return buf;
}

static __thread char t_resolveBuffer[64 * 1024];

bool InetAddress::resolve(const char* hostname, InetAddress* out)
{
  assert(out != NULL);
  struct hostent hent;
  struct hostent* he = NULL;
  int herrno = 0;
  bzero(&hent, sizeof(hent));

  int ret = gethostbyname_r(hostname, &hent, t_resolveBuffer, sizeof t_resolveBuffer, &he, &herrno);
  if (ret == 0 && he != NULL)
  {
    assert(he->h_addrtype == AF_INET && he->h_length == sizeof(uint32_t));
// use h_addr 取了第一个解析的地址
    out->addr_.sin_addr = *reinterpret_cast<struct in_addr*>(he->h_addr);
    return true;
  }
  else
  {
    if (ret)
    {
      LOG_SYSERR << "InetAddress::resolve";
    }
    return false;
  }
}

/*
int gethostbyname_r(const char *name,
    struct hostent *ret, char *buf, size_t buflen,
    struct hostent **result, int *h_errnop);
关于最后取得的result的内容，是不是需要释放，最后发现result存的东西，就是存在了buf里面。所以必定不能单独释放result所指向的内容。
    
  You should print out the values of the pointers in that struct to find out the answer to your question. You'll discover that they all point to data inside the buffer you allocated.

So a single free is all you need to free up all the memory.

But this also means that you must not free that allocation until you've finished using or copying whatever data you're interested in. You're freeing too early in your code.

 */
