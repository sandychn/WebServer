/*
 * File: Channel.cpp
 * Project: WebServer
 * Author: sandy
 * Last Modified: 2020-10-28 20:12:53
 */

#include "Channel.h"

/*
 * "Linux多线程服务端编程:使用muduo C++网络库" p.281
 * 每个Channel对象自始至终只负责一个文件描述符fd的IO时间分发,
 * 但它并不拥有这个fd,也不会在析构的时候关闭这个fd
 */

// Channel::Channel(EventLoop* loop) : loop_(loop), fd_(0), events_(0), lastEvents_(0) {}

Channel::Channel(EventLoop* loop, int fd) : loop_(loop), fd_(fd), events_(0), lastEvents_(0) {}

Channel::~Channel() {}

int Channel::getFd() const { return fd_; }

// void Channel::setFd(int fd) { fd_ = fd; }

void Channel::setHolderHttpData(std::shared_ptr<HttpData> holder) { holderHttpData_ = holder; }
std::shared_ptr<HttpData> Channel::getHolderHttpData() const { return holderHttpData_.lock(); }

void Channel::setReadHandler(CallBack&& readHandler) { readHandler_ = readHandler; }
void Channel::setWriteHandler(CallBack&& writeHandler) { writeHandler_ = writeHandler; }
void Channel::setErrorHandler(CallBack&& errorHandler) { errorHandler_ = errorHandler; }
void Channel::setConnHandler(CallBack&& connHandler) { connHandler_ = connHandler; }

void Channel::handleEvents() {
    events_ = 0;
    if ((revents_ & EPOLLHUP) && !(revents_ & EPOLLIN)) {
        events_ = 0;
        return;
    }
    if (revents_ & EPOLLERR) {
        handleError();
        events_ = 0;
        return;
    }
    if (revents_ & (EPOLLIN | EPOLLPRI | EPOLLRDHUP)) {
        handleRead();
    }
    if (revents_ & EPOLLOUT) {
        handleWrite();
    }
    handleConn();
}

void Channel::handleRead()  const { if (readHandler_)  readHandler_();  }
void Channel::handleWrite() const { if (writeHandler_) writeHandler_(); }
void Channel::handleConn()  const { if (connHandler_)  connHandler_();  }
void Channel::handleError() const { if (errorHandler_) errorHandler_(); }

void Channel::setRevents(__uint32_t ev) { revents_ = ev; }
void Channel::setEvents(__uint32_t ev)  { events_ = ev;  }

__uint32_t Channel::getEvents() const { return events_; }
__uint32_t Channel::getLastEvents() const { return lastEvents_; }

bool Channel::equalAndUpdateLastEvents() {
    bool ret = (lastEvents_ == events_);
    lastEvents_ = events_;
    return ret;
}
