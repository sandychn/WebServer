#pragma once

#include <sys/epoll.h>

#include <functional>
#include <memory>

class EventLoop;
class HttpData;

/*
 * 每个Channel对象自始至终只负责一个文件描述符fd的IO事件分发，但它并不拥有
 * 这个fd，也不会在析构的时候关闭这个fd。Channel会把不同的IO事件分发为不同
 * 的回调，如ReadCallback、WriteCallback等。
 */
class Channel {
   public:
    typedef std::function<void()> CallBack;

    // Channel(EventLoop *loop);
    Channel(EventLoop *loop, int fd);
    ~Channel();

    int getFd() const;
    // void setFd(int fd);

    void setHolderHttpData(std::shared_ptr<HttpData> holder);
    std::shared_ptr<HttpData> getHolderHttpData() const;

    void setReadHandler(CallBack&& readHandler);
    void setWriteHandler(CallBack&& writeHandler);
    void setErrorHandler(CallBack&& errorHandler);
    void setConnHandler(CallBack&& connHandler);

    void handleEvents();

    void setRevents(__uint32_t ev);
    void setEvents(__uint32_t ev);

    __uint32_t getEvents() const;
    __uint32_t getLastEvents() const;

    bool equalAndUpdateLastEvents();

   private:
    void handleRead() const;
    void handleWrite() const;
    void handleConn() const;
    void handleError() const;

    CallBack readHandler_;
    CallBack writeHandler_;
    CallBack errorHandler_;
    CallBack connHandler_;
    EventLoop *loop_;
    int fd_;
    __uint32_t events_;      // 关心的IO事件,由用户设置
    __uint32_t revents_;     // 目前活动的事件,由EventLoop/Poller设置
    __uint32_t lastEvents_;

    std::weak_ptr<HttpData> holderHttpData_; // 方便找到上层持有该Channel的对象
};
