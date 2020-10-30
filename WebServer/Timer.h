/*
 * File: Timer.h
 * Project: WebServer
 * Author: Sandy
 * Last Modified: 2020-10-30 19:58:54
 */

#pragma once

#include <cstddef>
#include <memory>
#include <queue>

#include "HttpData.h"
#include "base/MutexLock.h"
#include "base/noncopyable.h"

class HttpData;

class TimerNode {
   public:
    TimerNode(std::shared_ptr<HttpData> requestData, int timeout);
    ~TimerNode();
    TimerNode(TimerNode &tn);
    void update(int timeout);
    bool isValid();
    void clearReq();
    void setDeleted() { deleted_ = true; }
    bool isDeleted() const { return deleted_; }
    size_t getExpTime() const { return expiredTime_; }

   private:
    bool deleted_;
    size_t expiredTime_;
    std::shared_ptr<HttpData> SPHttpData;
};

struct TimerCmp {
    bool operator()(std::shared_ptr<TimerNode> &lhs, std::shared_ptr<TimerNode> &rhs) const {
        return lhs->getExpTime() > rhs->getExpTime();
    }
};

class TimerManager {
   public:
    TimerManager();
    ~TimerManager();
    void addTimer(std::shared_ptr<HttpData> SPHttpData, int timeout);
    void handleExpiredEvent();

   private:
    std::priority_queue<std::shared_ptr<TimerNode>, std::deque<std::shared_ptr<TimerNode>>, TimerCmp> timerNodeQueue;
    // MutexLock lock;
};
