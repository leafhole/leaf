#include <example/curl/Curl.h>
#include <net/EventLoop.h>
#include <boost/bind.hpp>
#include <stdio.h>

using namespace leaf::net;

EventLoop* g_loop = NULL;

void onData(const char* data, int len) {
  printf("len %d\n", len);
}

void done(curl::Request* c, int code) {
  printf("done %p %s %d\n", c, c->getEffectiveUrl(), code);
}

void done2(curl::Request* c ,int code) {
  printf("done2 %p %s %d %d\n", c, c->getRedirectUrl(),
         c->getResponseCode(), code);
}

int main(int argc, char* argv[]) {
  EventLoop loop;
  g_loop = &loop;
  loop.runAfter(10.0, boost::bind(&EventLoop::quit, &loop));
  curl::Curl::initialize(curl::Curl::kCURLssl);
  curl::Curl curl(&loop);

  curl::RequestPtr req = curl.getUrl("http://chenshuo.com");
  req->setDataCallback(onData);
  req->setDoneCallback(done);

  curl::RequestPtr req2 = curl.getUrl("https://github.com");
  req2->setDataCallback(onData);
  req2->setDoneCallback(done);

  curl::RequestPtr req3 = curl.getUrl("http://192.168.135.135:8000/?q=a");
  req3->setDataCallback(onData);
  req3->setDoneCallback(done);

  loop.loop();
}
