#ifndef LOGGER_H
#define LOGGER_H

#include <string>
#include <fstream>
#include <mutex>

enum LogLevel { ERROR = 0, INFO = 1, DEBUG = 2 };

class Logger {
public:
    Logger(const std::string& filename, LogLevel lvl = INFO);
    ~Logger();
    void log(LogLevel msgLevel, const std::string& msg);

private:
    std::ofstream logFile;
    LogLevel level;
    std::mutex mtx;
};

#endif // LOGGER_H
