#pragma once

#include "IMetricCollector.hpp"
#include <cstdint>
#include <chrono>
#include <vector>

struct DiskStats {
    std::string name;
    std::uint64_t reads = 0;
    std::uint64_t reads_merged = 0;
    std::uint64_t sectors_read = 0;
    std::uint64_t read_time_ms = 0;
    std::uint64_t writes = 0;
    std::uint64_t writes_merged = 0;
    std::uint64_t sectors_written = 0;
    std::uint64_t write_time_ms = 0;
    std::uint64_t io_time_ms = 0;      // поле 12 — время, когда диск был занят
    std::uint64_t weighted_time_ms = 0; // поле 13 — для расчёта очереди (опционально)
};

struct DiskMetrics {
    std::string name;
    float read_iops = 0.0;
    float write_iops = 0.0;
    float read_mib_s = 0.0;
    float write_mib_s = 0.0;
    float utilization_percent = 0.0; // io_time_diff / interval_ms * 100%
};

class DiskCollector : public IMetricCollector {
public:
    explicit DiskCollector(std::chrono::milliseconds interval);
    void collect() override;
    std::string getFormattedData() override;
private:
    std::chrono::milliseconds interval_;
    bool first_run_;
    std::vector<DiskStats> prev_stats_;
    std::vector<DiskMetrics> current_metrics_;
};