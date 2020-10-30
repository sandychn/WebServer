/*
 * File: Main.cpp
 * Project: WebServer
 * Author: sandy
 * Last Modified: 2020-10-30 22:53:41
 */

#include <cstdio>
#include <cstdlib>
#include <signal.h>

#include "EventLoop.h"
#include "Parameters.h"
#include "Server.h"
#include "Logger.h"
#include "spdlog/spdlog.h"

void loggerSave(int signalID) {
    if (Logger::isValid()) {
        Logger::getLogger().info("SIGINT recieved, exiting");
        Logger::getLogger().flush();
    }
    exit(0);
}

int main(int argc, char *argv[]) {
    signal(SIGINT, loggerSave);

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
