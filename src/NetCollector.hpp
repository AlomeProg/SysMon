#pragma once

#include <chrono>
#include <vector>
#include "IMetricCollector.hpp"

struct NetInterface {
    std::string name;
    std::uint64_t rx_bytes = 0;
    std::uint64_t tx_bytes = 0;
};

struct NetMetrics {
    std::string name;
    float rx_mib_s = 0.0;
    float tx_mib_s = 0.0;
};

class NetCollector : public IMetricCollector {
public:
    explicit NetCollector(std::chrono::milliseconds interval);
    void collect() override;
    std::string getFormattedData() override;
private:
    std::chrono::milliseconds interval_;
    bool first_run_;
    std::vector<NetInterface> prev_stats_;
    std::vector<NetMetrics> current_metrics_;
};