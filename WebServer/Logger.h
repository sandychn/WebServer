/*
 * File: Logger.h
 * Project: WebServer
 * Author: Sandy
 * Last Modified: 2020-10-30 22:45:51
 */

#pragma once

#include <iostream>
#include <memory>

#include "spdlog/async.h"
#include "spdlog/sinks/basic_file_sink.h"

class Logger {
public:
    Logger() = delete;

    static spdlog::logger& getLogger();
    static bool isValid();
    static void initLogger(const std::string& logPath);

private:
    static std::shared_ptr<spdlog::logger> asyncLogger;

};
