#pragma once

#include <string>
#include <fstream>
#include <mutex>

class Logger {
public:
    enum class Level {DEBUG, INFO, WARNING, ERROR};

    Logger(const std::string& filename, Level min_level = Level::INFO);

    void log(Level level, const std::string& message);
    void debug(const std::string& message);
    void info(const std::string& message);
    void warning(const std::string& message);
    void error(const std::string& message);
private:
    std::ofstream file_;
    Level min_level_;
    std::mutex mutex_;
};