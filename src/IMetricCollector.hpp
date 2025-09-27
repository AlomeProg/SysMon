#pragma once

#include <string>

class IMetricCollector {
public:
    virtual void collect() = 0;
    virtual std::string getFormattedData() = 0;
protected:
    virtual ~IMetricCollector() = default;
};