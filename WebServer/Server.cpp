/*
 * File: Server.cpp
 * Project: WebServer
 * Author: Sandy
 * Last Modified: 2020-10-30 20:30:32
 */

#include "Server.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <functional>

#include "Util.h"
#include "Logger.h"
#include "base/ErrorHandle.h"

Server::Server(EventLoop *loop, int threadNum, int port)
    : started_(false),
      threadNum_(threadNum),
      port_(port),
      listenFd_(Util::socketBindListen(port_)),
      loop_(loop),
      eventLoopThreadPool_(new EventLoopThreadPool(loop_, threadNum)),
      acceptChannel_(new Channel(loop_, listenFd_)) {
    Util::handleForSigpipe();
    if (Util::setSocketNonBlocking(listenFd_) < 0) {
        errorAbort("set socket non block failed");
    }
}

void Server::start() {
    eventLoopThreadPool_->start();
    acceptChannel_->setEvents(EPOLLIN | EPOLLET);
    acceptChannel_->setReadHandler(std::bind(&Server::handleNewConnection, this));
    acceptChannel_->setConnHandler(std::bind(&Server::handleThisConnection, this));
    loop_->addToPoller(acceptChannel_, 0);
    started_ = true;
}

void Server::handleNewConnection() {
    struct sockaddr_in client_addr;
    memset(&client_addr, 0, sizeof(sockaddr_in));
    socklen_t client_addr_len = sizeof(client_addr);
    int accept_fd = 0;
    while ((accept_fd = accept(listenFd_, (struct sockaddr *)&client_addr, &client_addr_len)) > 0) {
        // "Linux多线程服务端编程:使用muduo C++网络库" p.237
        // 限制服务器的最大并发连接数
        if (accept_fd >= MAXFDS) {
            close(accept_fd);
            continue;
        }

        EventLoop* loop = eventLoopThreadPool_->getNextLoop();
        Logger::getLogger().info("New connection from {0}:{1} (fd={2})",
                     inet_ntoa(client_addr.sin_addr),
                     ntohs(client_addr.sin_port),
                     accept_fd);

        // TCP的保活机制默认是关闭的
        // int optval = 0;
        // socklen_t len_optval = 4;
        // getsockopt(accept_fd, SOL_SOCKET,  SO_KEEPALIVE, &optval, &len_optval);
        // cout << "optval ==" << optval << endl;

        // 设为非阻塞模式
        if (Util::setSocketNonBlocking(accept_fd) < 0) {
            errorAbort("set non block failed");
        }

        Util::setSocketNoDelay(accept_fd);
        // Util::setSocketNoLinger(accept_fd);

        std::shared_ptr<HttpData> httpData(new HttpData(loop, accept_fd));
        httpData->getChannel()->setHolderHttpData(httpData);
        loop->queueInLoop(std::bind(&HttpData::newEvent, httpData));
    }

    acceptChannel_->setEvents(EPOLLIN | EPOLLET);
}

void Server::handleThisConnection() {
    loop_->updatePoller(acceptChannel_);
}
