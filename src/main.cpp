// g++ src/main.cpp src/CpuCollector.cpp src/MemoryCollector.cpp src/DiskCollector.cpp -o sysmon

#include <iostream>
#include <unistd.h>
#include <regex>
#include <string>
#include <chrono>
#include <thread>
#include <vector>

#include "ThreadPool.hpp"
#include "Logger.hpp"
#include "CpuCollector.hpp"
#include "MemoryCollector.hpp"
#include "DiskCollector.hpp"
#include "NetCollector.hpp"

// Функция для вывода справки
void printHelp() {
  std::cout
      << "Usage: sysmon [OPTIONS]\n"
      << "Options:\n"
      << "  help                Show this help message\n"
      << "  version             Show version\n"
      << "  -i=<duration>       Set update interval (e.g., -i=1s, -i=200ms)\n"
      << "  --interval=<duration> Same as -i\n"
      << "  -l=<file>           Set log file path (default: log.txt)\n"
      << "  --log-file=<file>   Same as -l\n"
      << "  --per-core          Enable per-CPU-core statistics\n"
      << "\n"
      << "Duration format:\n"
      << "  <number>s   - seconds (e.g., 1s, 5s)\n"
      << "  <number>ms  - milliseconds (e.g., 200ms, 300ms)\n";
}

void printVersion() {
    std::cout << "SysMon Version 0.1\n";
}

// Функция для парсинга периода из строки вида "1s" или "200ms"
std::chrono::milliseconds parseInterval(const std::string& periodStr) {
    std::regex pattern(R"(^(\d+)(s|ms)$)");
    std::smatch match;

    if (!std::regex_match(periodStr, match, pattern)) {
        throw std::invalid_argument("Invalid period format: " + periodStr);
    }

    long long value = std::stoll(match[1].str());
    std::string unit = match[2].str();

    if (unit == "s") {
        return std::chrono::seconds(value);
    } else if (unit == "ms") {
        return std::chrono::milliseconds(value);
    } else {
        throw std::invalid_argument("Unknown time unit: " + unit);
    }
}

int main(int argc, char* argv[]) {
    std::string log_filename = "log.txt";
    bool per_core = false;
    std::chrono::milliseconds interval = std::chrono::seconds(1);
    std::chrono::time_point last_log_time = std::chrono::steady_clock::now();
    std::chrono::milliseconds log_interval = std::chrono::seconds(120);
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];

        if (arg == "help") {
            printHelp();
            return 0;
        } else if (arg == "version") {
            printVersion();
            return 0;
        }
        else if (arg.size() >= 3 && arg.substr(0, 3) == "-i=") {
            interval = parseInterval(arg.substr(3));
        }
        else if (arg.size() >= 12 && arg.substr(0, 12) == "--interval=") {
            interval = parseInterval(arg.substr(12));
        }
        else if (arg.size() >= 3 && arg.substr(0, 3) == "-l=") {
            log_filename = arg.substr(3);
        }
        else if (arg.size() >= 11 && arg.substr(0, 11) == "--log-file=") {
            log_filename = arg.substr(11);
        }
        else if (arg.size() >= 15 && arg.substr(0, 15) == "--log-interval=") {
            log_interval = parseInterval(arg.substr(15));
        }
        else if (arg == "--per-core") {
            per_core = true;
        }
        else {
            std::cerr << "Unknown argument: " << arg << "\n";
            std::cerr << "Use 'sysmon help' for usage information.\n";
            return 1;
        }
    }

    Logger logger(log_filename);
    logger.info("Start with interval: " + std::to_string(interval.count()) + "ms");
    if (per_core) {
        logger.info("Per-core CPU stats enabled");
    }

    std::unique_ptr<CpuCollector> cpu = std::make_unique<CpuCollector>(per_core, logger);
    std::unique_ptr<MemoryCollector> memory = std::make_unique<MemoryCollector>(logger);
    std::unique_ptr<DiskCollector> disk = std::make_unique<DiskCollector>(interval, logger);
    std::unique_ptr<NetCollector> net = std::make_unique<NetCollector>(interval, logger);

    std::vector<std::unique_ptr<IMetricCollector>> collectors;
    collectors.push_back(std::move(cpu));
    collectors.push_back(std::move(memory));
    collectors.push_back(std::move(disk));
    collectors.push_back(std::move(net));

    ThreadPool pool(collectors.size());

    while (true) {
        std::vector<std::future<void>> futures;
        for (std::unique_ptr<IMetricCollector>& collector : collectors) {
            futures.emplace_back(pool.enqueue([&collector]() {
                collector->collect();
            }));
        }
        for (std::future<void> &f : futures) {
          f.get();
        }

        std::system("clear");
        std::cout << "SysMon - press ctrl + C for exit.\n";
        for (std::unique_ptr<IMetricCollector>& collector : collectors) {
            std::cout << collector->getFormattedData() << std::endl;
        }
        
        std::chrono::time_point now = std::chrono::steady_clock::now();
        if (now - last_log_time >= log_interval) {
            logger.info("=== System Summary start ===");
            for (std::unique_ptr<IMetricCollector>& collector : collectors) {
                std::string data = collector->getFormattedData();
                std::istringstream iss(data);
                std::string line;
                while (std::getline(iss, line)) {
                    if (!line.empty()) {
                        logger.info(line);
                    }
                }
            }
            last_log_time = now;
            logger.info("=== System Summary end ===");
        }

        std::this_thread::sleep_for(interval);
    }

    return 0;
}