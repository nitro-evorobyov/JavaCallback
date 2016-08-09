#pragma once


#include <iostream>
#include <mutex>
#include <fstream>
#include <ctime>    


namespace nitro_unused
{
class Logger
{
public:
    Logger()
      : coreStream(Core::Instance().Lock())
    {
    }

    ~Logger()
    {
        Core::Instance().Unlock();
    }

    std::ostream& GetLoggerStream()
    {
        auto    timeNow = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

        coreStream << "[" << std::ctime(&timeNow) << "]";
        coreStream << "[pid=" << std::this_thread::get_id() << "]";
        coreStream << "\n";

        return coreStream;
    }

private:
    std::ostream&   coreStream;

    class Core
    {
    public:
        std::ostream& Lock()
        {
            mutex.lock();
            return loggerStream;
        }

        void Unlock()
        {
            mutex.unlock();
        }

        static  Core& Instance()
        {
            static Core instance; 

            return instance;
        }

    private:
        std::ofstream   loggerStream;
        std::mutex      mutex;

        Core()
          : loggerStream("dll_side_log.txt")
        {}

        ~Core()
        {
            loggerStream.flush();
            loggerStream.close();
        }
    };
};
}


#define NITRO_TRACE() nitro::Logger().GetLoggerStream()