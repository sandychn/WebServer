/*
 * File: EventLoopThread.h
 * Project: WebServer
 * Author: Sandy
 * Last Modified: 2020-10-30 19:58:00
 */

#pragma once

#include "EventLoop.h"
#include "base/Condition.h"
#include "base/MutexLock.h"
#include "base/Thread.h"
#include "base/noncopyable.h"

class EventLoopThread : noncopyable {
   public:
    EventLoopThread();
    ~EventLoopThread();
    EventLoop *startLoop();

   private:
    void threadFunc();
    EventLoop *loop_;
    bool exiting_;
    Thread thread_;
    MutexLock mutex_;
    Condition cond_;
};
