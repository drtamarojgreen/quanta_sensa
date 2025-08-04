#include <iostream>
#include <fstream>
#include <chrono>
#include <ctime>
#include <mutex>

enum LogLevel { ERROR=0, INFO=1, DEBUG=2 };

class Logger {
private:
    std::ofstream logFile;
    LogLevel level;
    std::mutex mtx;
public:
    Logger(const std::string& filename, LogLevel lvl = INFO) : level(lvl) {
        logFile.open(filename, std::ios::app);
    }
    ~Logger() {
        if (logFile.is_open()) logFile.close();
    }

    void log(LogLevel msgLevel, const std::string& msg) {
        if (msgLevel <= level) {
            std::lock_guard<std::mutex> lock(mtx);
            auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
            logFile << "[" << std::put_time(std::localtime(&now), "%F %T") << "] ";
            switch(msgLevel) {
                case ERROR: logFile << "[ERROR] "; break;
                case INFO:  logFile << "[INFO] "; break;
                case DEBUG: logFile << "[DEBUG] "; break;
            }
            logFile << msg << std::endl;
        }
    }
};