#include "MemoryCollector.hpp"
#include <iomanip>
#include <iostream>
#include <fstream>
#include <sstream>

MemoryCollector::MemoryCollector(Logger& logger):
logger_(logger){
    logger_.info("MemoryCollector start.");
}

void MemoryCollector::collect() {
    std::ifstream file("/proc/meminfo");
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open \"/proc/meminfo\"");
    }

    std::string line;

    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string key, value_str;
        if (!(iss >> key >> value_str)) {
            continue; // пропускаем пустые/битые строки
        }

        try {
            uint64_t value = std::stoull(value_str);

            if (key == "MemTotal:") {
                total_kb_ = value;
            } else if (key == "MemAvailable:") {
                available_kb_ = value;
            } else if (key == "MemFree:") {
                free_kb_ = value;
            } else if (key == "SwapTotal:") {
                swap_total_kb_ = value;
            } else if (key == "SwapFree:") {
                swap_free_kb_ = value;
            }
        } catch (const std::exception&) {
            // Пропускаем строки с некорректными числами
            continue;
        }
    }

    if (total_kb_ == 0) {
        logger_.error("MemTotal not found in /proc/meminfo");
        throw std::runtime_error("MemTotal not found in /proc/meminfo");
    }
}

std::string MemoryCollector::getFormattedData() {
    if (total_kb_ == 0) {
        return "Memory: N/A";
    }

    double used_kb = total_kb_ - available_kb_;
    double used_percent = (total_kb_ > 0)
        ? (used_kb / total_kb_) * 100.0
        : 0.0;

    // Переводим в GiB для удобства
    double total_gb = total_kb_ / (1024.0 * 1024.0);
    double used_gb = used_kb / (1024.0 * 1024.0);

    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2);
    oss << "Memory:\n" << "  " << used_percent << "% ("
        << used_gb << " GiB / " << total_gb << " GiB)";

    // Опционально: swap
    if (swap_total_kb_ > 0) {
        double swap_used = swap_total_kb_ - swap_free_kb_;
        double swap_percent = (swap_used / swap_total_kb_) * 100.0;
        oss << " | Swap: " << swap_percent << "%\n";
    }

    return oss.str();
}
