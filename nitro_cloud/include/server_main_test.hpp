

#pragma once

#include <functional>


#include "sync_out_puter.h"
#include "server.h"


template<class TChar>
int ServerMainTest(const std::basic_string<TChar>&  pipeName, 
                   std::function<bool(std::basic_string<TChar>&)> createProcessFunction)
{
    boost::asio::io_service     ioService;
    boost::asio::add_service(ioService, new boost::asio::windows::stream_handle_service(ioService));

    boost::asio::strand         strand(ioService);

    Nitro::Community::Server<TChar>   server(pipeName, ioService, strand);


    std::vector<std::thread>    threadWorkerPull;

    const int countServiceThreads = 5;
    for (int threadIndex = 0; threadIndex < countServiceThreads; ++threadIndex)
    {
        threadWorkerPull.push_back(std::thread([&](){ioService.run(); }));
    }

    threadWorkerPull.push_back(std::thread([&](){server.Run(); }));

    int processNumber = 5;

    while (true)
    {
        if (server.OnIdle() == 0)
        {

            //SYNC_OUTPUT() << "[Main:]" << "Enter number process: ";
            //std::cin >> processNumber;

            if (processNumber == 0)
            {
                server.Stop();
                break;
            }

            for (int processIndex = 0; processIndex < processNumber; ++processIndex)
            {
                std::basic_string<TChar>    pipeNameCopy(server.GetPipeName());
                if (!createProcessFunction(pipeNameCopy))
                {
                    return -1;
                }
            }

            processNumber = 0;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }


    for (auto& workerThread : threadWorkerPull)
    {
        if (workerThread.joinable())
        {
            workerThread.join();
        }
    }

    return 0;
}

