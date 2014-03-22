#include <example/curl/Curl.h>
#include <base/Logging.h>
#include <net/Channel.h>
#include <net/EventLoop.h>
#include <boost/bind.hpp>

#include <curl/curl.h>
#include <assert.h>

using namespace curl;
using namespace leaf;
using namespace leaf::net;

static void dummy(const boost::shared_ptr<Channel>&) {
}

Request::Request(Curl* owner, StringPiece url)
    : owner_(owner),
      curl_(CHECK_NOTNULL(curl_easy_init())) {
  setopt(CURLOPT_URL, url.data());
  setopt(CURLOPT_WRITEFUNCTION, &Request::writeData);
  setopt(CURLOPT_WRITEDATA, this);
  setopt(CURLOPT_HEADERFUNCTION, &Request::headerData);
  setopt(CURLOPT_HEADERDATA, this);
  setopt(CURLOPT_PRIVATE, this);
  setopt(CURLOPT_USERAGENT, "curl");

  LOG_DEBUG << curl_ << " " << url;
  curl_multi_add_handle(owner_->getCurlm(), curl_);
}

Request::~Request() {
  LOG_TRACE;
  assert(!channel_ || channel_->isNoneEvent());
  curl_multi_remove_handle(owner_->getCurlm(), curl_);
  curl_easy_cleanup(curl_);
}

void Request::headerOnly() {
  setopt(CURLOPT_NOBODY, 1);
}

void Request::setRange(const StringPiece rangeOpt) {
  setopt(CURLOPT_RANGE, rangeOpt.data());
}

const char* Request::getEffectiveUrl() {
  const char* p = NULL;
  curl_easy_getinfo(curl_, CURLINFO_EFFECTIVE_URL, &p);
  return p;
}

const char* Request::getRedirectUrl() {
  const char* p = NULL;
  curl_easy_getinfo(curl_, CURLINFO_REDIRECT_URL, &p);
  return p;
}

int Request::getResponseCode() {
  long code = 0;
  curl_easy_getinfo(curl_, CURLINFO_RESPONSE_CODE, &code);
  return static_cast<int>(code);
}

Channel* Request::setChannel(int fd) {
  assert(channel_.get() == NULL);
  LOG_TRACE << "new channel with fd=" << fd << " " << curl_;
  channel_.reset(new Channel(owner_->getLoop(), fd ));
  channel_->tie(shared_from_this());
  return get_pointer(channel_);
}

void Request::removeChannel() {
  LOG_TRACE;
  channel_->disableAll();
  channel_->remove();
  owner_->getLoop()->queueInLoop(boost::bind(dummy, channel_));
  channel_.reset();
}

void Request::done(int code) {
  if (doneCb_) {
    doneCb_(this, code);
  }
}

void Request::dataCallback(const char* buffer, int len) {
  if (dataCb_) {
    dataCb_(buffer, len);
  }
}

void Request::headerCallback(const char* buffer, int len) {
  if (headerCb_) {
    headerCb_(buffer, len);
  }
}

size_t Request::writeData(char* buffer, size_t size, size_t nmemb, void* userp) {
  assert(size == 1);
  Request* req = static_cast<Request*>(userp);
  req->dataCallback(buffer, static_cast<int>(nmemb));
  return nmemb;
}

size_t Request::headerData(char* buffer, size_t size, size_t nmemb, void* userp) {
  assert(size == 1);
  Request* req = static_cast<Request*>(userp);
  req->headerCallback(buffer, static_cast<int>(nmemb));
  return nmemb;
}

void Curl::initialize(Option opt) {
  curl_global_init(opt == kCURLnossl ? CURL_GLOBAL_NOTHING : CURL_GLOBAL_SSL);
}

int Curl::socketCallback(CURL* c, int fd, int what, void* userp, void* socketp) {
  Curl* curl = static_cast<Curl*>(userp);
  const char* whatstr[] = { "none", "IN", "OUT", "INOUT", "REMOVE" };
  LOG_DEBUG << "Curl::socketCallback [" << curl << "] - fd = " << fd
            << " what = " << whatstr[what];
  Request* req = NULL;
  curl_easy_getinfo(c, CURLINFO_PRIVATE, &req);
  assert(req->getCurl() == c);
  if (what == CURL_POLL_REMOVE) {
    leaf::net::Channel* ch = static_cast<Channel*>(socketp);
    assert(req->getChannel() == ch);
    LOG_TRACE << "remove channel for fd=" << fd;
    req->removeChannel();
    ch = NULL;
    curl_multi_assign(curl->curlm_, fd, ch);
  } else {
    leaf::net::Channel* ch = static_cast<Channel*>(socketp);
    if (!ch) {
      ch = req->setChannel(fd);
      ch->setReadCallback(boost::bind(&Curl::onRead, curl, fd));
      ch->setWriteCallback(boost::bind(&Curl::onWrite, curl, fd));
      ch->enableReading(); // 新建的都读，读完了就不能读了，不过下次再可读，怎么触发呢？
      curl_multi_assign(curl->curlm_, fd, ch);
      LOG_TRACE << "new channel for fd=" << fd;
    }
    assert(req->getChannel() == ch);
    // update,奇怪的是，为什么要写呢？有写的话就要往外发
    if (what & CURL_POLL_OUT) {
      ch->enableWriting();
    } else {
      ch->disableWriting();
    }
  }
  return 0;
}

int Curl::timerCallback(CURLM* curlm, long ms, void* userp) {
  Curl* curl = static_cast<Curl*>(userp);
  LOG_DEBUG << curl << " " << ms << " ms";
  curl->loop_->runAfter(static_cast<int>(ms)/1000.0, boost::bind(&Curl::onTimer, curl));
  return 0;
}

Curl::Curl(EventLoop* loop)
    : loop_(loop),
      curlm_(CHECK_NOTNULL(curl_multi_init())),
      runningHandles_(0),
      prevRunningHandles_(0) {
  curl_multi_setopt(curlm_, CURLMOPT_SOCKETFUNCTION, &Curl::socketCallback);
  curl_multi_setopt(curlm_, CURLMOPT_SOCKETDATA, this);
  curl_multi_setopt(curlm_, CURLMOPT_TIMERFUNCTION, &Curl::timerCallback);
  curl_multi_setopt(curlm_, CURLMOPT_TIMERDATA, this);

  //Pass a pointer to a function matching the curl_multi_timer_callback prototype:
  //int curl_multi_timer_callback(CURLM *multi /* multi handle */ ,
  //                              long timeout_ms /* timeout in milliseconds */ ,
  //                              void *userp /* TIMERDATA */ )
  // CURLMOPT_TIMERDATA 修改的是 void* userp指定了传递什么值
}
/*
  虽然文档这么说,不过我还是不明白.tnnd
  http://curl.haxx.se/libcurl/c/curl_multi_socket_action.html
1. Create a multi handle

2. Set the socket callback with CURLMOPT_SOCKETFUNCTION

3. Set the timeout callback with CURLMOPT_TIMERFUNCTION, to get to know what timeout value to use when waiting for socket activities.

4. Add easy handles with curl_multi_add_handle()

5. Provide some means to manage the sockets libcurl is using, so you can check them for activity. This can be done through your application code, or by way of an external library such as libevent or glib.

6. Call curl_multi_socket_action(..., CURL_SOCKET_TIMEOUT, 0, ...) to kickstart everything. To get one or more callbacks called.

7. Wait for activity on any of libcurl's sockets, use the timeout value your callback has been told.

8, When activity is detected, call curl_multi_socket_action() for the socket(s) that got action. If no activity is detected and the timeout expires, call curl_multi_socket_action(3) with CURL_SOCKET_TIMEOUT.  
 */

Curl::~Curl() {
  LOG_TRACE;
  curl_multi_cleanup(curlm_);
}

RequestPtr Curl::getUrl(StringPiece url) {
  RequestPtr req(new Request(this, url.data()));
  return req;
}

void Curl::onTimer() {
  CURLMcode rc = CURLM_OK;
  do {
    LOG_TRACE;
    rc = curl_multi_socket_action(curlm_, CURL_SOCKET_TIMEOUT, 0, &runningHandles_);
    LOG_TRACE << rc << " " << runningHandles_;
  } while (rc == CURLM_CALL_MULTI_PERFORM);
  checkFinish(); // 每次有curl_multi_socket_action的时候,都需要检查一下是否finish
}

void Curl::onRead(int fd) {
  CURLMcode rc = CURLM_OK;
  do {
    LOG_TRACE;
    // 这里就是让curl进行接收,进行read,一直读啊读啊读
    rc = curl_multi_socket_action(curlm_, fd, CURL_POLL_IN, &runningHandles_);
    LOG_TRACE << fd << " " << rc << " " << runningHandles_;
  } while (rc == CURLM_CALL_MULTI_PERFORM);
  checkFinish();
}

void Curl::onWrite(int fd) {
  CURLMcode rc = CURLM_OK;
  do {
    LOG_TRACE;
    // 这里就是让curl进行发送,进行write    
    rc = curl_multi_socket_action(curlm_, fd, CURL_POLL_OUT, &runningHandles_);
    LOG_TRACE << fd << " " << rc << " " << runningHandles_;
  } while (rc == CURLM_CALL_MULTI_PERFORM);
  checkFinish();
}

void Curl::checkFinish() {
  if (prevRunningHandles_ > runningHandles_ || runningHandles_ == 0) {
    CURLMsg* msg = NULL;
    int left = 0;
    while ( (msg = curl_multi_info_read(curlm_, &left)) != NULL) {
      if (msg->msg == CURLMSG_DONE) {
        CURL* c = msg->easy_handle;
        CURLcode res = msg->data.result;
        Request* req = NULL;
        curl_easy_getinfo(c, CURLINFO_PRIVATE, &req);
        assert(req->getCurl() == c);
        LOG_TRACE << req << " done";
        req->done(res);
      }
      if (runningHandles_ == 0) {
        //loop_->quit();
      }
    }
  }
  prevRunningHandles_ = runningHandles_;
}
