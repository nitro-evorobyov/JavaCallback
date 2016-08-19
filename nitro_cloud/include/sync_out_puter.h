#pragma once

#include <mutex>
#include <iostream>
#include <ctime>
#include <iomanip>

class SyncOutPuter
{
public:
    // Init this variable at one of the cpp where you will include it
    static std::mutex   globalStreamLock;
    

    SyncOutPuter()
    {
        globalStreamLock.lock();
        auto t = std::time(nullptr);
        auto tm = *std::localtime(&t);
        std::cout << std::put_time(&tm, "[%H:%M:%S] - ");
    }

    ~SyncOutPuter()
    {
        globalStreamLock.unlock();
        GetSrtream() << std::endl;
    }

    std::ostream&  GetSrtream()
    {
        return std::cout;
    }
};

#define SYNC_OUTPUT() SyncOutPuter().GetSrtream()