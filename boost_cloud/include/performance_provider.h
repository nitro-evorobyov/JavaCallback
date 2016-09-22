
namespace nitro
{

#pragma once

#include <chrono>
#include <vector>

#include "sync_out_puter.h"


class PerformanceProvider
{
public:
    PerformanceProvider()
    {
        SYNC_OUTPUT("!LOG PERFORMANCE!") << "start performance timer." << std::endl;
        m_startTime = std::chrono::high_resolution_clock::now();
    }

    ~PerformanceProvider()
    {
        auto endTime = std::chrono::high_resolution_clock::now();
        SYNC_OUTPUT("!LOG PERFORMANCE!")
            << " perfomance results in seconds -  "
            << std::chrono::duration_cast<std::chrono::seconds>(endTime - m_startTime).count()
            << ","
            << std::chrono::duration_cast<std::chrono::microseconds>(endTime - m_startTime).count() << std::endl;
    }
private:

    std::chrono::system_clock::time_point    m_startTime;
};
}