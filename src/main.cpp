// g++ src/main.cpp src/CpuCollector.cpp src/MemoryCollector.cpp -o sysmon

#include <iostream>
#include <unistd.h>
#include <regex>
#include <string>
#include <chrono>
#include <thread>

#include "CpuCollector.hpp"
#include "MemoryCollector.hpp"
#include "DiskCollector.hpp"

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

    CpuCollector cpu(true);
    MemoryCollector memory;
    DiskCollector disk(period);

    std::string cpu_info;
    std::string memory_info;
    std::string disk_info; 

    while (true) {
        // Update
        cpu.collect();
        memory.collect();
        disk.collect();

        cpu_info = cpu.getFormattedData();
        memory_info = memory.getFormattedData();
        disk_info = disk.getFormattedData();

        std::system("clear");
        std::cout << "SysMon - press ctrl + C for exit.\n";
        std::cout << cpu_info << std::endl; 
        std::cout << memory_info << std::endl;
        std::cout << disk_info << std::endl;

        std::this_thread::sleep_for(period);
    }

    // sysmon help
    // sysmon 
    // sysmon -p=1s
    // sysmon -p=200ms
    // sysmon --period=2s
    // sysmon --period=300ms


    // for (int i = 0; i < 5; ++i) {
    //     cpu.collect();
    //     std::cout << cpu.getFormattedData() << "\n";
    //     std::this_thread::sleep_for(std::chrono::milliseconds(200));
    // }


    // for (int i = 0; i < 5; ++i) {
    //     memory.collect();
    //     std::cout << memory.getFormattedData() << "\n";
    //     std::this_thread::sleep_for(std::chrono::milliseconds(200));
    // }

    return 0;
}