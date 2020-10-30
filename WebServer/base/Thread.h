/*
 * File: Thread.h
 * Project: base
 * Author: Sandy
 * Last Modified: 2020-10-30 19:56:52
 */

#pragma once

#include <bits/types.h>
#include <pthread.h>

#include <functional>
#include <memory>
#include <string>

#include "CountDownLatch.h"
#include "noncopyable.h"

class Thread : noncopyable {
   public:
    typedef std::function<void()> ThreadFunc;
    explicit Thread(const ThreadFunc&, const std::string& name = std::string());
    ~Thread();
    void start();
    int join();
    bool started() const { return started_; }
    pid_t tid() const { return tid_; }
    const std::string& name() const { return name_; }

   private:
    void setDefaultName();
    bool started_;
    bool joined_;
    pthread_t pthreadId_;
    pid_t tid_;
    ThreadFunc func_;
    std::string name_;
    CountDownLatch latch_;
};
