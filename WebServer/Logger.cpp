/*
 * File: Logger.cpp
 * Project: WebServer
 * Author: Sandy
 * Last Modified: 2020-10-30 22:46:35
 */

#include <memory>

#include "Logger.h"

#include "spdlog/async.h"
#include "spdlog/sinks/basic_file_sink.h"

std::shared_ptr<spdlog::logger> Logger::asyncLogger;

spdlog::logger& Logger::getLogger() {
    return *asyncLogger.get();
}

bool Logger::isValid() {
    return static_cast<bool>(asyncLogger);
}

void Logger::initLogger(const std::string& logPath) {
    asyncLogger = spdlog::basic_logger_mt<spdlog::async_factory>("async_file_logger", logPath);
}
