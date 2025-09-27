#pragma once

#include <cstdint>
#include <vector>
#include "IMetricCollector.hpp"

struct CpuTimes {
    std::uint64_t user = 0;
    std::uint64_t nice = 0;
    std::uint64_t system = 0;
    std::uint64_t idle = 0;
    std::uint64_t iowait = 0;
    std::uint64_t irq = 0;
    std::uint64_t softirq = 0;
    std::uint64_t steal = 0;
    std::uint64_t guest = 0;
    std::uint64_t guest_nice = 0;
};

struct CpuStats {
    CpuTimes total;
    std::vector<CpuTimes> per_core;
    bool has_total = false;
};

class CpuCollector : public IMetricCollector {
public:
    explicit CpuCollector(bool collect_per_core = false);
    void collect() override;
    std::string getFormattedData() override;
private:
    double calculateCpuUsage(const CpuTimes& current, const CpuTimes& previous);
    std::vector<double> calculatePerCoreUsage(
        const std::vector<CpuTimes>& current_cores,
        const std::vector<CpuTimes>& previous_cores
    );

    bool collect_per_core_;
    bool first_run_ = true;
    
    CpuTimes prev_total_;
    double cpu_usage_percent_ = 0.0;

    std::vector<CpuTimes> prev_cores_;
    std::vector<double> core_usage_percents_;

};