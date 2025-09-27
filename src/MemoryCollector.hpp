#pragma once

#include <cstdint>
#include "IMetricCollector.hpp"

class MemoryCollector : public IMetricCollector {
public:
    explicit MemoryCollector();
    void collect() override;
    std::string getFormattedData() override;
private:
    std::uint64_t total_kb = 0;
    std::uint64_t available_kb = 0;
    std::uint64_t free_kb = 0;
    std::uint64_t swap_total_kb = 0;
    std::uint64_t swap_free_kb = 0;
};