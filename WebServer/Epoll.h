#pragma once
#include <sys/epoll.h>

#include <memory>
#include <vector>

#include "Channel.h"
#include "HttpData.h"
#include "Timer.h"

class Epoll {
   public:
    Epoll();
    ~Epoll();
    void epollAdd(std::shared_ptr<Channel> request, int timeout);
    void epollMod(std::shared_ptr<Channel> request, int timeout);
    void epollDel(std::shared_ptr<Channel> request);
    std::vector<std::shared_ptr<Channel>> poll();
    void addTimer(std::shared_ptr<Channel> request_data, int timeout);
    int getEpollFd() const { return epollFd_; }
    void handleExpired() { timerManager_.handleExpiredEvent(); }

   private:
    std::vector<std::shared_ptr<Channel>> getEventsRequest(int events_count);
    
    static const int MAXFDS = 100000;
    int epollFd_;
    std::vector<struct epoll_event> events_;
    std::shared_ptr<Channel> fd2chan_[MAXFDS];   // fd -> Channel
    std::shared_ptr<HttpData> fd2http_[MAXFDS];  // fd -> HttpData
    TimerManager timerManager_;
};
