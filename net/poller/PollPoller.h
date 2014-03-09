#ifndef NET_POLLER_POLLPOLLER_H
#define NET_POLLER_POLLPOLLER_H



#include <net/Poller.h>
#include <map>
#include <vector>

struct pollfd;

namespace leaf {
namespace net {

class PollPoller : public Poller {
 public:
  PollPoller(EventLoop* loop);
  virtual ~PollPoller();

  virtual Timestamp poll(int timeoutMs, ChannelList* activeChannels);
  virtual void updateChannel(Channel* channel);
  virtual void removeChannel(Channel* channel);

 private:
  void fillActiveChannels(int numEvents,
			  ChannelList* activeChannels) const ;
  typedef std::vector<struct pollfd> PollFdList;
  typedef std::map<int, Channel*> ChannelMap;
  PollFdList pollfds_;
  ChannelMap channels_;
};

}

}

#endif // NET_POLLER_POLLPOLLER_H
