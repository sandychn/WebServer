/*
 * File: Timer.cpp
 * Project: WebServer
 * Author: sandy
 * Last Modified: 2020-10-30 20:30:59
 */

#include "Timer.h"
#include "Logger.h"

#include <sys/time.h>
#include <unistd.h>

#include <queue>


TimerNode::TimerNode(std::shared_ptr<HttpData> requestData, int timeout) : deleted_(false), SPHttpData(requestData) {
    update(timeout);
}

TimerNode::TimerNode(TimerNode &tn) : expiredTime_(0), SPHttpData(tn.SPHttpData) {}

TimerNode::~TimerNode() {
    if (SPHttpData) {
        Logger::getLogger().info("closing HttpData in ~TimerNode() (fd={0})", SPHttpData->getChannel()->getFd());
        SPHttpData->handleClose();
    }
}

void TimerNode::update(int timeout) {
    struct timeval now;
    gettimeofday(&now, NULL);
    // 以毫秒计
    expiredTime_ = (((now.tv_sec % 10000) * 1000) + (now.tv_usec / 1000)) + timeout;
}

bool TimerNode::isValid() {
    struct timeval now;
    gettimeofday(&now, NULL);
    size_t temp = (((now.tv_sec % 10000) * 1000) + (now.tv_usec / 1000));
    if (temp < expiredTime_) {
        return true;
    }
    this->setDeleted();
    return false;
}

void TimerNode::clearReq() {
    SPHttpData.reset();
    this->setDeleted();
}

TimerManager::TimerManager() {}

TimerManager::~TimerManager() {}

void TimerManager::addTimer(std::shared_ptr<HttpData> SPHttpData, int timeout) {
    std::shared_ptr<TimerNode> timerNode(new TimerNode(SPHttpData, timeout));
    timerNodeQueue.push(timerNode);
    SPHttpData->linkTimer(timerNode);
}

/*
因为:
(1) 优先队列不支持随机访问
(2) 即使支持，随机删除某节点后破坏了堆的结构，需要重新更新堆结构
所以:
对于被置为deleted的时间节点，会延迟到它
(1) 超时
(2) 它前面的节点都被删除
它才会被删除。一个点被置为deleted，它最迟会在TIMER_TIME_OUT时间后被删除

这样做有两个好处：
(1) 不需要遍历优先队列，省时。
(2) 给超时时间一个容忍的时间，就是设定的超时时间是删除的下限(并不是一到超时时间就立即删除)，
    如果监听的请求在超时后的下一次请求中又一次出现了，就不用再重新申请RequestData节点了，
    这样可以继续重复利用前面的RequestData，减少了一次delete和一次new的时间。
*/

void TimerManager::handleExpiredEvent() {
    // MutexLockGuard locker(lock);
    while (!timerNodeQueue.empty()) {
        std::shared_ptr<TimerNode> ptimer_now = timerNodeQueue.top();
        if (ptimer_now->isDeleted() || !ptimer_now->isValid()) {
            timerNodeQueue.pop();
        } else {
            break;
        }
    }
}
