#pragma once

#include <mutex>
#include <iostream>
#include <ctime>
#include <iomanip>
#include <algorithm>
#include <sstream>
#include <set> 


class SyncOutPuter
{
public:
    // Init this variable at one of the cpp where you will include it
    static std::mutex   globalStreamLock;
    static std::stringstream fakeStream;
    static std::ostream*   outputStream;
    static std::set<std::string>  exclude;

    SyncOutPuter(const char* prefix = nullptr)
    {
        globalStreamLock.lock();

        outputStream = &fakeStream;

        if (prefix)
        {
            std::string filterValue(prefix);

            if (std::find_if(exclude.begin(), exclude.end(), [&filterValue](const std::string& etalone)
            {
                return filterValue.find(etalone) != std::string::npos; 
            }) != exclude.end())
            {
                return;
            }
        }

        outputStream = &std::cout;

        auto t = std::time(nullptr);
        auto tm = *std::localtime(&t);

        if (prefix)
        {
            *outputStream << "(" << prefix << ")";
        }

        // cause put_time is not implemented at gcc, I use strftime
        char    timeStampBufer[128] {};

        if (std::strftime(timeStampBufer,sizeof(timeStampBufer),"[%H-%M-%S] : ", &tm) > 0) 
        {
            *outputStream << timeStampBufer;
        }
    }

    ~SyncOutPuter()
    {
        GetSrtream() << std::endl;
        globalStreamLock.unlock();
    }

    std::ostream&  GetSrtream()
    {
        assert(outputStream); 
        return outputStream != nullptr ? *outputStream : std::cout;
    }
};

#define INIT_SYNCOUTPUTER() std::mutex SyncOutPuter::globalStreamLock; std::set<std::string>  SyncOutPuter::exclude; std::ostream*   SyncOutPuter::outputStream = 0; std::stringstream SyncOutPuter::fakeStream;


#define SYNC_OUTPUT() SyncOutPuter(nullptr).GetSrtream()

#define SYNC_OUTPUT(name) SyncOutPuter(name).GetSrtream()