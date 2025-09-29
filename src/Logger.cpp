#include "Logger.hpp"

#include <chrono>
#include <iomanip>
#include <sstream>
#include <ctime>

Logger::Logger(const std::string& filename, Level min_level) :
min_level_(Level::INFO) {
    file_.open(filename);
}

std::string levelToString(Logger::Level level) {
    switch (level) {
    case Logger::Level::DEBUG:      
        return "DEBUG";
        break;
    case Logger::Level::INFO:       
        return "INFO";
        break;
    case Logger::Level::WARNING:    
        return "WARNING";
        break;
    case Logger::Level::ERROR:      
        return "ERROR";
        break;
    default:
        return "Unidentified meaning";
        break;
    }
}

std::string getCurrentTimestamp() {
    const auto now = std::chrono::system_clock::now();
    const auto now_time_t = std::chrono::system_clock::to_time_t(now);
    
    std::tm tm{};
    localtime_r(&now_time_t, &tm);

    const auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()
    ) % 1000;

    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S")
        << '.' << std::setfill('0') << std::setw(3) << ms.count();
    return oss.str();
}

void Logger::log(Level level, const std::string& message) {
    if (!file_.is_open()) return;

    file_ << "[" << getCurrentTimestamp() << "] "
          << levelToString(level) << ": "
          << message << "\n";
    file_.flush();
}

void Logger::debug(const std::string& message) {
    if (min_level_ <= Level::DEBUG) {
        log(Level::DEBUG, message);
    } 
}

void Logger::info(const std::string& message) {
    if (min_level_ <= Level::INFO) {
        log(Level::INFO, message);
    }
}

void Logger::warning(const std::string& message) {
    if (min_level_ <= Level::WARNING) {
        log(Level::WARNING, message);
    }
}

void Logger::error(const std::string& message) {
    if (min_level_ <= Level::ERROR) {
        log(Level::ERROR, message);
    }
}

