/*
 * File: Main.cpp
 * Project: WebServer
 * Author: sandy
 * Last Modified: 2020-10-30 20:34:21
 */

#include <cstdio>

#include "EventLoop.h"
#include "Parameters.h"
#include "Server.h"
#include "Logger.h"

#include <spdlog/spdlog.h>

int main(int argc, char *argv[]) {
    Parameters parameters = Parameters::getParameters(argc, argv);

    spdlog::set_level(spdlog::level::debug);
    spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%f] [thread %t] [%^%l%$] %v");
    Logger::initLogger(parameters.logPath);

    EventLoop mainLoop;
    Server myHTTPServer(&mainLoop, parameters.threadNum, parameters.port);
    myHTTPServer.start();
    spdlog::info("Running WebServer on port {0} using {1} threads", parameters.port, parameters.threadNum);
    mainLoop.loop();
    return 0;
}
