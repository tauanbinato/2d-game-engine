#include "Logger.h"
#include <iostream>
#include <ctime>
#include <chrono>
#include <string>

std::vector<LogEntry> Logger::messages;

std::string Logger::CurrentDateTimeToString() {
    std::time_t now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    std::string output(30, '\0');
    std::strftime(&output[0], output.size(), "%d-%b-%Y %H:%M:%S", std::localtime(&now));
    return output;
}

void Logger::Log(const std::string& message) {
    LogEntry logEntry;
    logEntry.type = LOG_INFO;

    logEntry.msg = "LOG: [" + Logger::CurrentDateTimeToString() + "]: " + message;
    std::cout << "\x1B[32m" << logEntry.msg << "\033[0m" << std::endl;

    Logger::messages.push_back(logEntry);
};

void Logger::Error(const std::string& message) {
    LogEntry logEntry;
    logEntry.type = LOG_ERROR;

    logEntry.msg = "ERROR: [" + Logger::CurrentDateTimeToString() + "]: " + message;
    std::cout << "\x1B[91m" << logEntry.msg << "\033[0m" << std::endl;

    Logger::messages.push_back(logEntry);
};