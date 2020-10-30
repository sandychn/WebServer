/*
 * File: Epoll.cpp
 * Project: WebServer
 * Author: sandy
 * Last Modified: 2020-10-29 10:36:01
 */

#include "Epoll.h"

#include <arpa/inet.h>
#include <assert.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/epoll.h>
#include <sys/socket.h>

#include "Util.h"
#include "base/ErrorHandle.h"

#include <spdlog/spdlog.h>

const int EVENTSNUM = 4096;
const int EPOLLWAIT_TIME = 10000;  // ms

/*
int epoll_ctl(int epfd, int op, int fd, struct epoll_event *event);
    epfd:  the return value of epoll_create()
    op   : EPOLL_CTL_ADD / EPOLL_CTL_MOD / EPOLL_CTL_DEL
    fd   : fd to listen
    event: what events to listen

struct epoll_event {
    __uint32_t   events;  // epoll events
    epoll_data_t data;    // user data variable
};

events 可以是以下几个宏的集合：
1) EPOLLIN, 表示对应的文件描述符可以读（包括对端 socket 正常关闭）
2) EPOLLOUT, 表示对应的文件描述符可以写
3) EPOLLPRI，表示对应的文件描述符有紧急的数据可读（这里应该表示有带外数据到来）
4) EPOLLERR, 表示对应的文件描述符发生错误
5) EPOLLHUP, 表示对应的文件描述符被挂断
6) EPOLLLET, 将EPOLL设为边缘触发 (Edge Triggered) 模式, 这是相对水平触发 (Level Triggered) 来说的
7) EPOLLONESHOT, 只监听一次事件, 当监听完这次事件后, 若还需要监听这个socket, 需要再把这个socket加到EPOLL队列里
8) EPOLLRDHUP, 读关闭
*/

Epoll::Epoll() : epollFd_(epoll_create1(EPOLL_CLOEXEC)), events_(EVENTSNUM) {
    if (epollFd_ <= 0) {
        errorAbort("epoll_create return value <= 0");
    }
}

Epoll::~Epoll() {}

// 注册新描述符
void Epoll::epollAdd(std::shared_ptr<Channel> request, int timeout) {
    int fd = request->getFd();
    if (timeout > 0) {
        addTimer(request, timeout);
        fd2http_[fd] = request->getHolderHttpData();
    }
    struct epoll_event event;
    event.data.fd = fd;
    event.events = request->getEvents();

    request->equalAndUpdateLastEvents();
    fd2chan_[fd] = request;

    if (epoll_ctl(epollFd_, EPOLL_CTL_ADD, fd, &event) < 0) {
        spdlog::error("epollAdd error");
        fd2chan_[fd].reset();
    }
}

// 修改描述符状态
void Epoll::epollMod(std::shared_ptr<Channel> request, int timeout) {
    if (timeout > 0) {
        addTimer(request, timeout);
    }
    int fd = request->getFd();
    if (!request->equalAndUpdateLastEvents()) {
        struct epoll_event event;
        event.data.fd = fd;
        event.events = request->getEvents();
        if (epoll_ctl(epollFd_, EPOLL_CTL_MOD, fd, &event) < 0) {
            spdlog::error("epollMod error");
            fd2chan_[fd].reset();
        }
    }
}

// 从epoll中删除描述符
void Epoll::epollDel(std::shared_ptr<Channel> request) {
    int fd = request->getFd();
    struct epoll_event event;
    event.data.fd = fd;
    event.events = request->getLastEvents();

    if (epoll_ctl(epollFd_, EPOLL_CTL_DEL, fd, &event) < 0) {
        spdlog::error("epollDel error");
    }
    fd2chan_[fd].reset();
    fd2http_[fd].reset();
}

// 返回活跃事件
std::vector<std::shared_ptr<Channel>> Epoll::poll() {
    std::vector<std::shared_ptr<Channel>> requestData;

    while (requestData.empty()) {
        // The epoll_wait() system call waits for events on the epoll(7)
        // instance referred to by the file descriptor epfd.  The buffer pointed
        // to by events is used to return information from the ready list about
        // file descriptors in the interest list that have some events
        // available.  Up to maxevents are returned by epoll_wait().  The
        // maxevents argument must be greater than zero.

        int event_count = epoll_wait(epollFd_, &(*events_.begin()), events_.size(), EPOLLWAIT_TIME);
        if (event_count < 0) {
            spdlog::error("epoll_wait return value < 0");
        }
        requestData = getEventsRequest(event_count);
    }
    return requestData;
}

void Epoll::addTimer(std::shared_ptr<Channel> requestData, int timeout) {
    std::shared_ptr<HttpData> t = requestData->getHolderHttpData();
    if (t) {
        timerManager_.addTimer(t, timeout);
    } else {
        spdlog::warn("timer add failed");
    }
}

/** private **/

// 分发处理函数
std::vector<std::shared_ptr<Channel>> Epoll::getEventsRequest(int events_count) {
    std::vector<std::shared_ptr<Channel>> requestData;
    for (int i = 0; i < events_count; ++i) {
        // 获取有事件产生的描述符
        int fd = events_[i].data.fd;

        std::shared_ptr<Channel> currentRequestChannel = fd2chan_[fd];

        if (currentRequestChannel) {
            currentRequestChannel->setRevents(events_[i].events);
            currentRequestChannel->setEvents(0);
            requestData.push_back(currentRequestChannel);
        } else {
            spdlog::warn("currentRequestChannel is invalid");
        }
    }
    return requestData;
}
