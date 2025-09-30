#pragma once

#include <cstdint>
#include "Logger.hpp"
#include "IMetricCollector.hpp"

class MemoryCollector : public IMetricCollector {
public:
    explicit MemoryCollector(Logger& logger);
    void collect() override;
    std::string getFormattedData() override;
private:
    std::uint64_t total_kb_ = 0;
    std::uint64_t available_kb_ = 0;
    std::uint64_t free_kb_ = 0;
    std::uint64_t swap_total_kb_ = 0;
    std::uint64_t swap_free_kb_ = 0;

    Logger& logger_;
};