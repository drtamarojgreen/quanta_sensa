#include "../include/logger.h"
#include <iostream>
#include <chrono>
#include <ctime>
#include <iomanip>

Logger::Logger(const std::string& filename, LogLevel lvl) : level(lvl) {
    logFile.open(filename, std::ios::app);
}

Logger::~Logger() {
    if (logFile.is_open()) {
        logFile.close();
    }
}

void Logger::log(LogLevel msgLevel, const std::string& msg) {
    if (msgLevel <= level) {
        std::lock_guard<std::mutex> lock(mtx);
        auto now_c = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        logFile << "[" << std::put_time(std::localtime(&now_c), "%F %T") << "] ";
        switch (msgLevel) {
            case ERROR:
                logFile << "[ERROR] ";
                break;
            case INFO:
                logFile << "[INFO] ";
                break;
            case DEBUG:
                logFile << "[DEBUG] ";
                break;
        }
        logFile << msg << std::endl;
    }
}
