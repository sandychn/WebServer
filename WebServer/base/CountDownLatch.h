/*
 * File: CountDownLatch.h
 * Project: base
 * Author: Sandy
 * Last Modified: 2020-10-30 19:56:13
 */

#pragma once

#include "Condition.h"
#include "MutexLock.h"
#include "noncopyable.h"

// "Linux多线程服务端编程:使用muduo C++网络库" p.42
// CountDownLatch的主要作用是确保Thread中传进去的func真的启动了以后
// 外层的start才返回
class CountDownLatch : noncopyable {
   public:
    explicit CountDownLatch(int count);
    void wait();
    void countDown();

   private:
    mutable MutexLock mutex_;
    Condition condition_;
    int count_;
};
