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

        // cause put_time is not implemented at gcc, I use strftime
        char    timeStampBufer[128] {};
        if (std::strftime(timeStampBufer,sizeof(timeStampBufer),"[%H:%M:%S] - ", &tm) > 0) {
            std::cout << timeStampBufer;
        }
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