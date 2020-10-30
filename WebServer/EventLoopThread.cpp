#include "EventLoopThread.h"

#include <functional>

EventLoopThread::EventLoopThread()
    : loop_(nullptr),
      exiting_(false),
      thread_(std::bind(&EventLoopThread::threadFunc, this), "EventLoopThread"),
      mutex_(),
      cond_(mutex_) {}

EventLoopThread::~EventLoopThread() {
    exiting_ = true;
    if (loop_ != nullptr) {
        loop_->quit();
        thread_.join();
    }
}

// "Linux多线程服务端编程:使用muduo C++网络库" p.298
EventLoop* EventLoopThread::startLoop() {
    assert(!thread_.started());
    thread_.start();
    {
        MutexLockGuard lock(mutex_);

        // 一直等到threadFunc在Thread里真正跑起来
        // using `while` instead of `if` ("Spurious wakeup")
        // https://en.wikipedia.org/wiki/Spurious_wakeup
        while (loop_ == nullptr) {
            cond_.wait();
        }
    }
    return loop_;
}

void EventLoopThread::threadFunc() {
    EventLoop loop;

    {
        MutexLockGuard lock(mutex_);
        loop_ = &loop;
        cond_.notify();
    }

    loop.loop();
    loop_ = nullptr;
}
