/*
 * File: Condition.h
 * Project: base
 * Author: sandy
 * Last Modified: 2020-10-29 09:00:55
 */

#pragma once
#include <pthread.h>

#include <cerrno>  // ETIMEDOUT
#include <cstdio>
#include <ctime>

#include "MutexLock.h"
#include "noncopyable.h"

#include <spdlog/spdlog.h>

/*
 * "Linux多线程服务端编程:使用muduo C++网络库" p.40
 * 条件变量，其学名为管程(monitor)。
 * 但它并不拥有这个fd,也不会在析构的时候关闭这个fd
 */

class Condition : noncopyable {
   public:
    explicit Condition(MutexLock &_mutex) : mutex(_mutex) { pthread_cond_init(&cond, NULL); }

    ~Condition() {
        int ret = pthread_cond_destroy(&cond);
        spdlog::info("~Condition()");
        if (EBUSY == ret) {
            spdlog::warn("pthread_cond_destroy returns EBUSY");
        }
    }

    void wait() { pthread_cond_wait(&cond, mutex.get()); }

    void notify() { pthread_cond_signal(&cond); }

    void notifyAll() { pthread_cond_broadcast(&cond); }

    bool waitForSeconds(int seconds) {
        struct timespec abstime;
        clock_gettime(CLOCK_REALTIME, &abstime);
        abstime.tv_sec += static_cast<time_t>(seconds);
        return ETIMEDOUT == pthread_cond_timedwait(&cond, mutex.get(), &abstime);
    }

   private:
    MutexLock &mutex;
    pthread_cond_t cond;
};
