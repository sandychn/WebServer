/*
 * File: Server.h
 * Project: WebServer
 * Author: Sandy
 * Last Modified: 2020-10-30 19:58:44
 */

#pragma once
#include <memory>

#include "Channel.h"
#include "EventLoop.h"
#include "EventLoopThreadPool.h"

class Server {
   public:
    Server(EventLoop *loop, int threadNum, int port);
    ~Server() {}
    EventLoop *getLoop() const { return loop_; }
    void start();
    void handleNewConnection();
    void handleThisConnection();

   private:
    static const int MAXFDS = 100000;
    
    bool started_;
    int threadNum_;
    int port_;
    int listenFd_;
    EventLoop *loop_;
    std::unique_ptr<EventLoopThreadPool> eventLoopThreadPool_;
    std::shared_ptr<Channel> acceptChannel_;
};
