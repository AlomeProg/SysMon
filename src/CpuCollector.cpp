#include "CpuCollector.hpp"
#include <iomanip>
#include <iostream>
#include <fstream>
#include <sstream>

CpuCollector::CpuCollector(bool collect_per_core) : 
    collect_per_core_(collect_per_core){}

CpuTimes parseCpuLine(const std::string& line) {
    std::istringstream iss(line);
    std::string token;
    iss >> token; 

    std::vector<std::uint64_t> values;
    while (iss >> token) {
        std::size_t pos;
        std::uint64_t val = std::stoull(token, &pos);
        if (pos != token.size()) {
            throw std::runtime_error("Invalid number in CPU line");
        }
        values.push_back(val);
    }

    if (values.size() < 10) {
        throw std::runtime_error("Not enough fields in CPU line");
    }

    return CpuTimes{values[0], values[1], values[2], values[3],
                    values[4], values[5], values[6], values[7],
                    values[8], values[9]};
}

CpuStats readAllCpuCores() {
    std::ifstream file("/proc/stat");
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open \"/proc/stat\"");
    }

    CpuStats stats;
    std::string line;
    while (std::getline(file, line)) {
        if (line.substr(0, 4) == "cpu ") {
            // Общая статистика
            stats.total = parseCpuLine(line);
            stats.has_total = true;
        } else if ( line.size() >= 4 && line.substr(0,3) == "cpu" &&
        std::isdigit(static_cast<unsigned char>(line[3]))) {
            // По ядрам
            stats.per_core.push_back(parseCpuLine(line));
        }
    }
    if (!stats.has_total) {
        throw std::runtime_error("Missing total CPU line in /proc/stat");
    }

    return stats;
}

void CpuCollector::collect() {
    try {
        CpuStats current = readAllCpuCores();
        if (first_run_ == true) {
            prev_total_ = current.total;
            if (collect_per_core_ == true) {
                prev_cores_ = std::move(current.per_core);
                core_usage_percents_.assign(prev_cores_.size(), 0.0);
            }
            first_run_ = false;
            cpu_usage_percent_ = 0.0;
            return;
        }

        cpu_usage_percent_ = calculateCpuUsage(current.total, prev_total_);
        prev_total_ = current.total;

        if (collect_per_core_ && !current.per_core.empty()) {
            core_usage_percents_ = calculatePerCoreUsage(current.per_core, prev_cores_);
            prev_cores_ = std::move(current.per_core);
        }
    } catch (const std::exception& e) {
        std::cout << "Error:" << std::string(e.what());
    }
}

double CpuCollector::calculateCpuUsage(const CpuTimes& current, const CpuTimes& previous) {
    auto active = [](const CpuTimes& t) {
        return t.user + t.nice + t.system + t.irq + t.softirq + t.steal;
    };
    auto idle = [](const CpuTimes& t) {
        return t.idle + t.iowait;
    };

    std::uint64_t active_curr = active(current);
    std::uint64_t idle_curr = idle(current);
    std::uint64_t total_curr = active_curr + idle_curr;

    std::uint64_t active_prev = active(previous);
    std::uint64_t idle_prev = idle(previous);
    std::uint64_t total_prev = active_prev + idle_prev;

    std::uint64_t active_diff = (active_curr > active_prev) ? (active_curr - active_prev) : 0;
    std::uint64_t total_diff = (total_curr > total_prev) ? (total_curr - total_prev) : 0;
    
    if (total_diff == 0) return 0.0;
    return (static_cast<double>(active_diff) / total_diff) * 100.0;
}

std::vector<double> CpuCollector::calculatePerCoreUsage(
    const std::vector<CpuTimes>& current_cores,
    const std::vector<CpuTimes>& previous_cores)
{
    auto active = [](const CpuTimes& t) {
        return t.user + t.nice + t.system + t.irq + t.softirq + t.steal;
    };
    auto idle = [](const CpuTimes& t) {
        return t.idle + t.iowait;
    };

    std::vector<double> usages;
    usages.reserve(current_cores.size());

    for (size_t i = 0; i < current_cores.size(); ++i) {
        const CpuTimes& curr = current_cores[i];
        const CpuTimes& prev = (i < previous_cores.size()) ? previous_cores[i] : curr;

        uint64_t active_curr = active(curr);
        uint64_t idle_curr = idle(curr);
        uint64_t total_curr = active_curr + idle_curr;

        uint64_t active_prev = active(prev);
        uint64_t idle_prev = idle(prev);
        uint64_t total_prev = active_prev + idle_prev;

        uint64_t active_diff = (active_curr > active_prev) ? (active_curr - active_prev) : 0;
        uint64_t total_diff = (total_curr > total_prev) ? (total_curr - total_prev) : 0;

        double usage = (total_diff > 0) ? (static_cast<double>(active_diff) / total_diff) * 100.0 : 0.0;
        usages.push_back(usage);
    }

    return usages;
}

std::string CpuCollector::getFormattedData() {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(1);

    if (cpu_usage_percent_ < 0) {
        oss << "CPU: N/A";
    } else {
        oss << "CPU: " << cpu_usage_percent_ << "%";
    }

    if (collect_per_core_ && !core_usage_percents_.empty()) {
        oss << " [";
        for (size_t i = 0; i < core_usage_percents_.size(); ++i) {
            if (i > 0) oss << ", ";
            if (core_usage_percents_[i] < 0) {
                oss << "N/A";
            } else {
                oss << "C" << i << ":" << core_usage_percents_[i];
            }
        }
        oss << "]";
    }

    return oss.str();
}
