
#pragma once

#include <chrono>
#include <vector>

#include "../../nitro_cloud/include/sync_out_puter.h"

//#define SURVEY

#ifndef SURVEY
const char socketUrl[] = "tcp://127.0.0.1:100";
#else
const char socketUrl[] = "ipc:///tmp/survey.ipc";
#endif

namespace nitro
{

namespace
{
void BufferToStream(char *buf, int bufferLength, std::stringstream& serializeStream)
{
    std::vector<char>   buffer(bufferLength + 1);

    memcpy(&buffer[0], buf, bufferLength);
    buffer[bufferLength] = 0;

    serializeStream.str(&buffer[0]);
}
}


void AddExcludeLogWord(const char* excludeWord)
{
    SyncOutPuter::exclude.insert(excludeWord);
}


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