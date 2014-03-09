#ifndef NET_CALLBACKS_H
#define NET_CALLBACKS_H

#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>

#include <base/Timestamp.h>

namespace leaf {

// Adaped from google-protobuf stubs/common.h
// see in base/Types.h  
template<typename To, typename From>
inline ::boost::shared_ptr<To> down_pointer_cast(const ::boost::shared_ptr<From>& f) {
  if (false) {
    implicit_cast<From*, To*>(0);
  }

#ifndef NDEBUG
  assert( f == NULL || dynamic_cast<To*>(get_pointer(f)) != NULL);
#endif
  return ::boost::static_pointer_cast<To>(f);
  
}

namespace net {

// All client visiable callbacks go here
class Buffer;
class TcpConnection;
typedef boost::shared_ptr<TcpConnection> TcpConnectionPtr;
typedef boost::function<void()> TimerCallback;
typedef boost::function<void (const TcpConnectionPtr&)> ConnectionCallback;
typedef boost::function<void (const TcpConnectionPtr&)> CloseCallback;
typedef boost::function<void (const TcpConnectionPtr&)> WriteCompleteCallback;
typedef boost::function<void (const TcpConnectionPtr&, size_t)> HighWaterMarkCallback;

// the dta has been read to (buf, len)
typedef boost::function<void (const TcpConnectionPtr&,
			      Buffer*,
			      Timestamp)> MessageCallback;

void defaultConnectionCallback(const TcpConnectionPtr& conn);
void defaultMessageCallback(const TcpConnectionPtr& conn,
			    Buffer* buffer,
			    Timestamp receiveTime);


  
} // end of namespace net
  
} // end of namespace leaf

  

#endif // LEAF_NET_CALLBACKS_H
