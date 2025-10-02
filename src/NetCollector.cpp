#include "NetCollector.hpp"
#include <algorithm>
#include <istream>
#include <fstream>
#include <iomanip>
#include <iostream>

NetCollector::NetCollector(std::chrono::milliseconds interval, Logger& logger) :
interval_(interval), 
first_run_(true), 
logger_(logger){
    logger_.info("NetCollector start.");
}

std::vector<NetInterface> readNetDev() {
    std::ifstream file("/proc/net/dev");
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open /proc/net/dev");
    }

    std::vector<NetInterface> interfaces;
    std::string line;

    std::getline(file, line); // "Inter-|   Receive..."
    std::getline(file, line); // " face |bytes    packets..."

    while (std::getline(file, line)) {
        size_t pos = line.find(':');
        if (pos == std::string::npos) {
            continue;
        }
        std::string name = line.substr(0, pos);
        // Убираем начальные пробелы
        name.erase(name.begin(), std::find_if(name.begin(), name.end(), [](unsigned char ch) {
            return !std::isspace(ch);
        }));

        std::istringstream iss(line.substr(pos + 1));
        std::vector<uint64_t> stats;
        uint64_t val;
        while (stats.size() < 16 && iss >> val) {
            stats.push_back(val);
        }

        if (stats.size() < 16) {
            continue;
        }

        NetInterface iface;
        iface.name = name;
        iface.rx_bytes = stats[0];  // received bytes
        iface.tx_bytes = stats[8];  // transmitted bytes

        interfaces.push_back(iface);
    }

    if (interfaces.empty()) {
        throw std::runtime_error("No network interfaces found in /proc/net/dev");
    }

    return interfaces;
}

void NetCollector::collect() {
    try {
        std::vector<NetInterface> current = readNetDev();

        if (first_run_) {
            prev_stats_ = current;
            first_run_ = false;
            current_metrics_.clear();
            return;
        }

        current_metrics_.clear();
        double interval_sec = interval_.count() / 1000.0;

        for (const auto& curr : current) {
            auto it = std::find_if(prev_stats_.begin(), prev_stats_.end(),
                [&curr](const NetInterface& i) { return i.name == curr.name; });

            if (it == prev_stats_.end()) {
                continue; // новый интерфейс
            }

            const NetInterface& prev = *it;

            std::uint64_t rx_diff = (curr.rx_bytes > prev.rx_bytes) ? (curr.rx_bytes - prev.rx_bytes) : 0;
            std::uint64_t tx_diff = (curr.tx_bytes > prev.tx_bytes) ? (curr.tx_bytes - prev.tx_bytes) : 0;

            NetMetrics m;
            m.name = curr.name;
            m.rx_mib_s = (interval_sec > 0) ? (rx_diff / (1024.0 * 1024.0) / interval_sec) : 0.0;
            m.tx_mib_s = (interval_sec > 0) ? (tx_diff / (1024.0 * 1024.0) / interval_sec) : 0.0;

            current_metrics_.push_back(m);
        }

        prev_stats_ = std::move(current);

    } catch (const std::exception& e) {
        current_metrics_.clear();
        prev_stats_.clear();
        first_run_ = true;
    }
}

std::string NetCollector::getFormattedData() {
    if (current_metrics_.empty()) {
        return "Network: N/A";
    }

    std::ostringstream oss;
    oss << "Network:\n";
    for (const auto& m : current_metrics_) {
        // Пропускаем loopback, если не отлаживаем
        if (m.name == "lo") continue;

        oss << "  " << m.name << ": "
            << std::fixed << std::setprecision(2)
            << "↓ " << m.rx_mib_s << " MiB/s, "
            << "↑ " << m.tx_mib_s << " MiB/s\n";
    }
    return oss.str();
}