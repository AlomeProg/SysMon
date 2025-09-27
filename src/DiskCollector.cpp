#include "DiskCollector.hpp"
#include <algorithm>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <sstream>


DiskCollector::DiskCollector(std::chrono::milliseconds interval) :
first_run_(true), interval_(interval) {}

std::vector<DiskStats> readDiskStats() {
    std::ifstream file("/proc/diskstats");
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open /proc/diskstats");
    }
    std::vector<DiskStats> disks;
    std::string line;

    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::vector<std::string> tokens;
        std::string token;
        while (iss >> token) {
            tokens.push_back(token);
        }

        if (tokens.size() < 14) {
            continue;
        }

        const std::string& name = tokens[2];
        if (std::isdigit(name.back())) {
            if (name.find("nvme") == std::string::npos || name.find('p') != std::string::npos) {
                continue;
            }
        }

        try {
            DiskStats ds;
            ds.name = name;
            ds.reads = std::stoull(tokens[3]);
            ds.reads_merged = std::stoull(tokens[4]);
            ds.sectors_read = std::stoull(tokens[5]);
            ds.read_time_ms = std::stoull(tokens[6]);
            ds.writes = std::stoull(tokens[7]);
            ds.writes_merged = std::stoull(tokens[8]);
            ds.sectors_written = std::stoull(tokens[9]);
            ds.write_time_ms = std::stoull(tokens[10]);
            ds.io_time_ms = std::stoull(tokens[12]);      // поле 12 (индекс 12)
            ds.weighted_time_ms = std::stoull(tokens[13]); // поле 13

            disks.push_back(ds);
        } catch (const std::exception&) {
            continue; // некорректные данные — пропускаем
        }
    }

    if (disks.empty()) {
        throw std::runtime_error("No valid disk devices found in /proc/diskstats");
    }

    return disks;
}

void DiskCollector::collect() {
try {
        std::vector<DiskStats> current = readDiskStats();

        if (first_run_) {
            prev_stats_ = current;
            first_run_ = false;
            current_metrics_.clear();
            return;
        }

        current_metrics_.clear();
        double interval_sec = interval_.count() / 1000.0;
        uint64_t interval_ms = interval_.count();

        for (const auto& curr : current) {
            // Найти prev с тем же именем
            auto it = std::find_if(prev_stats_.begin(), prev_stats_.end(),
                [&curr](const DiskStats& d) { return d.name == curr.name; });

            if (it == prev_stats_.end()) {
                continue; // новое устройство — пропускаем в этот раз
            }

            const DiskStats& prev = *it;

            uint64_t read_diff = (curr.reads > prev.reads) ? (curr.reads - prev.reads) : 0;
            uint64_t write_diff = (curr.writes > prev.writes) ? (curr.writes - prev.writes) : 0;
            uint64_t sectors_read_diff = (curr.sectors_read > prev.sectors_read) ? (curr.sectors_read - prev.sectors_read) : 0;
            uint64_t sectors_written_diff = (curr.sectors_written > prev.sectors_written) ? (curr.sectors_written - prev.sectors_written) : 0;
            uint64_t io_time_diff = (curr.io_time_ms > prev.io_time_ms) ? (curr.io_time_ms - prev.io_time_ms) : 0;

            DiskMetrics m;
            m.name = curr.name;
            m.read_iops = (interval_sec > 0) ? (read_diff / interval_sec) : 0.0;
            m.write_iops = (interval_sec > 0) ? (write_diff / interval_sec) : 0.0;
            m.read_mib_s = (interval_sec > 0) ? (sectors_read_diff * 512.0 / (1024*1024) / interval_sec) : 0.0;
            m.write_mib_s = (interval_sec > 0) ? (sectors_written_diff * 512.0 / (1024*1024) / interval_sec) : 0.0;
            m.utilization_percent = (interval_ms > 0) ? (static_cast<double>(io_time_diff) / interval_ms * 100.0) : 0.0;

            // Ограничиваем utilization 100%
            if (m.utilization_percent > 100.0) m.utilization_percent = 100.0;

            current_metrics_.push_back(m);
        }

        prev_stats_ = std::move(current);

    } catch (const std::exception& e) {
        current_metrics_.clear();
        prev_stats_.clear();
        first_run_ = true;
    }
}

std::string DiskCollector::getFormattedData() {
    if (current_metrics_.empty()) {
        return "Disk: N/A";
    }

    std::ostringstream oss;
    oss << "Disk IO:\n";
    for (const auto& m : current_metrics_) {
        oss << "  " << m.name << ": "
            << std::fixed << std::setprecision(1)
            << "R " << m.read_mib_s << " MiB/s, "
            << "W " << m.write_mib_s << " MiB/s, "
            << "Util " << m.utilization_percent << "%\n";
    }
    return oss.str();
}
