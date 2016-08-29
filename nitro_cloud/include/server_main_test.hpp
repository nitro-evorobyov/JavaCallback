

#pragma once

#include <functional>
#include <list>

#include "sync_out_puter.h"
#include "helper.h"

std::mutex   SyncOutPuter::globalStreamLock;

INIT_TASK_COUNTER()

template<class TChar>
int ServerMainTest(const std::basic_string<TChar>&  pipeName, 
                   boost::asio::io_service&     ioService,
                   std::function<bool()> createProcessFunction)
{
    Nitro::Community::Server<TChar>   server(pipeName, ioService);

    

    std::list<std::thread>    threadWorkerPull;

    const int countServiceThreads = 5;
    for (int threadIndex = 0; threadIndex < countServiceThreads; ++threadIndex)
    {
        threadWorkerPull.push_back(std::thread([&](){ioService.run(); }));
    }

    threadWorkerPull.push_back(std::thread([&]() { server.Run(); }));

    int processNumber = 5;

    for (int processIndex = 0; processIndex < processNumber; ++processIndex)
    {
        if (!createProcessFunction())
        { 
            return -1;
        }
    }

    while (Nitro::Community::TaskCounter::GetTaskCount() > 0)
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    server.Stop();

    for (auto& thread : threadWorkerPull) {
        if (thread.joinable())
        {
            thread.join();
        }
    };

    return 0;
}

