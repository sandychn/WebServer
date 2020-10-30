/*
 * File: EventLoopThreadPool.cpp
 * Project: WebServer
 * Author: Sandy
 * Last Modified: 2020-10-30 19:58:12
 */

#include "EventLoopThreadPool.h"
#include "base/ErrorHandle.h"

EventLoopThreadPool::EventLoopThreadPool(EventLoop *baseLoop, int numThreads)
    : baseLoop_(baseLoop), started_(false), numThreads_(numThreads), next_(0) {
    if (numThreads_ <= 0) {
        errorAbort("EventLoopThreadPool constructor: numThreads_ must > 0");
    }
}

void EventLoopThreadPool::start() {
    baseLoop_->assertInLoopThread();
    started_ = true;
    for (int i = 0; i < numThreads_; ++i) {
        std::shared_ptr<EventLoopThread> t(new EventLoopThread());
        threads_.push_back(t);
        loops_.push_back(t->startLoop());
    }
}

EventLoop *EventLoopThreadPool::getNextLoop() {
    baseLoop_->assertInLoopThread();
    assert(started_);
    EventLoop *loop = baseLoop_;
    if (!loops_.empty()) {
        loop = loops_[next_];
        next_ = (next_ + 1) % numThreads_;
    }
    return loop;
}
