#pragma once

#include <memory>
#include <vector>

#include "Epoll.h"
#include "Util.h"
#include "base/CurrentThread.h"
#include "base/Thread.h"
#include "base/ErrorHandle.h"

class Channel;

class EventLoop {
   public:
    typedef std::function<void()> Functor;
    EventLoop();
    ~EventLoop();
    void loop();
    void quit();
    void runInLoop(Functor&& cb);
    void queueInLoop(Functor&& cb);

    void assertInLoopThread() { if (!isInLoopThread()) errorAbort("assertion failed: assertInLoopThread"); }

    void addToPoller(std::shared_ptr<Channel> channel, int timeout = 0) { poller_->epollAdd(channel, timeout); }
    void removeFromPoller(std::shared_ptr<Channel> channel) { poller_->epollDel(channel); }
    void updatePoller(std::shared_ptr<Channel> channel, int timeout = 0) { poller_->epollMod(channel, timeout); }

   private:
    // `wakeupFd_` must be declared before `pwakeupChannel_`

    int wakeupFd_;
    bool looping_;
    bool quit_;
    bool eventHandling_;
    bool callingPendingFunctors_;
    const pid_t threadId_;
    
    mutable MutexLock mutex_; // pendingFunctors_

    std::shared_ptr<Epoll> poller_;
    std::shared_ptr<Channel> pwakeupChannel_;
    std::vector<Functor> pendingFunctors_;

    void wakeup();
    void handleRead();
    void handleConn();
    void doPendingFunctors();
    bool isInLoopThread() const { return threadId_ == CurrentThread::tid(); }

    static int createEventfd();
};
