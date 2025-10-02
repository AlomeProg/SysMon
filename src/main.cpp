// g++ src/main.cpp src/CpuCollector.cpp src/MemoryCollector.cpp src/DiskCollector.cpp -o sysmon

#include <iostream>
#include <unistd.h>
#include <regex>
#include <string>
#include <chrono>
#include <thread>

#include "Logger.hpp"
#include "CpuCollector.hpp"
#include "MemoryCollector.hpp"
#include "DiskCollector.hpp"
#include "NetCollector.hpp"

// Функция для вывода справки
void printHelp() {
    std::cout << "Usage: sysmon [OPTIONS]\n"
              << "Options:\n"
              << "  help                Show this help message\n"
              << "  -p=<duration>       Set update period (e.g., -p=1s, -p=200ms)\n"
              << "  --period=<duration> Set update period (e.g., --period=2s, --period=300ms)\n"
              << "\n"
              << "Duration format:\n"
              << "  <number>s   - seconds (e.g., 1s, 5s)\n"
              << "  <number>ms  - milliseconds (e.g., 200ms, 300ms)\n";
}

void printVersion() {
    std::cout << "SysMon Version 0.1\n";
}

// Функция для парсинга периода из строки вида "1s" или "200ms"
std::chrono::milliseconds parsePeriod(const std::string& periodStr) {
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
    std::chrono::milliseconds period = std::chrono::seconds(1);
    if (argc == 1) {
        std::cout << "Running SysMon with default settings...\n";        
    } else {
        for (int i = 1; i < argc; i++) {
            std::string arg = argv[i];
            if (arg == "help") {
                printHelp();
                return 0;
            } else if (arg == "version") {
                printVersion();
                return 0;
            }

            if (arg.substr(0, 3) == "-p=") {
                period = parsePeriod(arg.substr(3));
            } else if (arg.substr(0,9) == "--period=") {
                period = parsePeriod(arg.substr(9));
            } else {
                std::cerr << "Unknown argument: " << arg << "\n";
                std::cerr << "Use 'sysmon help' for usage information.\n";
                return 1;
            }
        }
    }

    Logger logger("log.txt");
    
    logger.info("Start with interval:" + std::to_string(period.count()) + "ms");
    
    CpuCollector cpu(true, logger);
    MemoryCollector memory(logger);
    DiskCollector disk(period, logger);
    NetCollector net(period, logger);

    std::string cpu_info;
    std::string memory_info;
    std::string disk_info; 
    std::string net_info;
    
    while (true) {
        // Update
        cpu.collect();
        memory.collect();
        disk.collect();
        net.collect();

        cpu_info = cpu.getFormattedData();
        memory_info = memory.getFormattedData();
        disk_info = disk.getFormattedData();
        net_info = net.getFormattedData();

        std::system("clear");
        std::cout << "SysMon - press ctrl + C for exit.\n\n";
        std::cout << cpu_info << std::endl; 
        std::cout << memory_info << std::endl;
        std::cout << disk_info << std::endl;
        std::cout << net_info << std::endl;

        std::this_thread::sleep_for(period);
    }
    return 0;
}