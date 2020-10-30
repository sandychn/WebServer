/*
 * File: EventLoop.cpp
 * Project: WebServer
 * Author: sandy
 * Last Modified: 2020-10-29 01:21:32
 */

#include "EventLoop.h"

#include <sys/epoll.h>
#include <sys/eventfd.h>

#include "Util.h"
#include "base/ErrorHandle.h"

#include <spdlog/spdlog.h>

/*
 * __thread是GCC内置的线程局部存储设施，存取效率可以和全局变量相比。
 * __thread变量每一个线程有一份独立实体，各个线程的值互不干扰。
 * 它还可以用来修饰那些值可能变、带有全局性，但是又不值得用全局变量保护的变量。
 * __thread使用规则：
 * 只能修饰POD类型 (类似整型指针的标量，不带自定义的构造、拷贝、赋值、析构的类型，
 * 二进制内容可以任意复制memset,memcpy,且内容可以复原)，
 * 不能修饰class类型，因为无法自动调用构造函数和析构函数，可以用于修饰全局变量，函数内的静态变量，
 * 不能修饰函数的局部变量或者class的普通成员变量，
 * 且__thread变量值只能初始化为编译器常量，即值在编译器就可以确定例如const int i = 5,
 * 运行期常量是运行初始化后不再改变例如const int i = rand()
 * 
 */

__thread EventLoop* t_loopInThisThread = nullptr;

int EventLoop::createEventfd() {
    // EFD_CLOEXEC: closed on exec
    int evtfd = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if (evtfd < 0) {
        errorAbort("eventfd error");
    }
    return evtfd;
}

EventLoop::EventLoop()
    : wakeupFd_(EventLoop::createEventfd()),
      looping_(false),
      quit_(false),
      eventHandling_(false),
      callingPendingFunctors_(false),
      threadId_(CurrentThread::tid()),
      poller_(new Epoll()),
      pwakeupChannel_(new Channel(this, wakeupFd_)) {
    if (t_loopInThisThread != nullptr) {
        errorAbort("another EventLoop exists in this thread");
    } else {
        t_loopInThisThread = this;
    }
    // pwakeupChannel_->setEvents(EPOLLIN | EPOLLET | EPOLLONESHOT);
    pwakeupChannel_->setEvents(EPOLLIN | EPOLLET);
    pwakeupChannel_->setReadHandler(std::bind(&EventLoop::handleRead, this));
    pwakeupChannel_->setConnHandler(std::bind(&EventLoop::handleConn, this));
    poller_->epollAdd(pwakeupChannel_, 0);
}

EventLoop::~EventLoop() {
    close(wakeupFd_);
    t_loopInThisThread = nullptr;
}

void EventLoop::loop() {
    assert(!looping_);
    assertInLoopThread();
    looping_ = true;
    quit_ = false;
    std::vector<std::shared_ptr<Channel>> ret;
    while (!quit_) {
        ret.clear();
        ret = poller_->poll();
        eventHandling_ = true;
        for (std::shared_ptr<Channel>& spChannel : ret) spChannel->handleEvents();
        eventHandling_ = false;
        doPendingFunctors();
        poller_->handleExpired();
    }
    looping_ = false;
}

void EventLoop::quit() {
    quit_ = true;
    if (!isInLoopThread()) {
        wakeup();
    }
}

void EventLoop::runInLoop(Functor&& cb) {
    if (isInLoopThread()) {
        cb();
    } else {
        queueInLoop(std::move(cb));
    }
}

void EventLoop::queueInLoop(Functor&& cb) {
    {
        MutexLockGuard lock(mutex_);
        pendingFunctors_.emplace_back(std::move(cb));
    }

    if (!isInLoopThread() || callingPendingFunctors_) wakeup();
}

void EventLoop::wakeup() {
    spdlog::debug("wakeup");
    uint64_t one = 1;
    ssize_t n = Util::writen(wakeupFd_, &one, sizeof(one));
    if (n != sizeof(one)) {
        spdlog::warn("EventLoop::wakeup() writes {0} bytes instead of {1}", n, sizeof(one));
    }
}

void EventLoop::handleRead() {
    spdlog::debug("handleRead");
    uint64_t one = 1;
    ssize_t n = Util::readn(wakeupFd_, &one, sizeof(one));
    if (n != sizeof(one)) {
        spdlog::warn("EventLoop::wakeup() reads {0} bytes instead of {1}", n, sizeof(one));
    }
    spdlog::debug("pwakeupChannel->getEvents() = {0}", pwakeupChannel_->getEvents());
    pwakeupChannel_->setEvents(EPOLLIN | EPOLLET);
}

// called in EventLoop::loop()
void EventLoop::doPendingFunctors() {
    spdlog::debug("doPendingFunctors");
    std::vector<Functor> functors;
    callingPendingFunctors_ = true;

    {
        MutexLockGuard lock(mutex_);
        functors.swap(pendingFunctors_);
    }

    for (size_t i = 0; i < functors.size(); ++i) functors[i]();
    callingPendingFunctors_ = false;
}


/*
 * EventLoop::runInLoop() / EventLoop::queueInLoop()
 *   -> EventLoop()::wakeup()
 *   -> EventLoop()::loop()
 *   -> Channel::handleEvents()
 *   -> Channel::connHandler_
 *   -> EventLoop::handleConn()
 */

void EventLoop::handleConn() {
    spdlog::debug("handleConn");
    updatePoller(pwakeupChannel_, 0);
}
