

#pragma once

#include <functional>
#include <list>

#include "server.hpp"
#include "../performance_provider.h"
#include "../command/command_impl.h"


int ServerMainTest(const std::string&  socketAddr)
{
    boost::asio::io_service     ioService;
    nitro::community::Server    server(socketAddr, ioService);

    std::list<std::thread>    threadWorkerPull;

    const int countServiceThreads = 5;
    for (int threadIndex = 0; threadIndex < countServiceThreads; ++threadIndex)
    {
        threadWorkerPull.push_back(std::thread([&](){ioService.run(); }));
    }

    threadWorkerPull.push_back(std::thread([&]() { server.Run(); }));


    std::shared_ptr<nitro::command::ICommandGenerator>  commandGenerator = std::make_shared<nitro::command::CommandGenerator>();


    {
        nitro::PerformanceProvider performance;
        while (commandGenerator->HasCommands())
        {
            server.SendCommands(commandGenerator);
            std::this_thread::yield();
        }
    }

    system("pause");

    server.Stop();

    for (auto& thread : threadWorkerPull) {
        if (thread.joinable())
        {
            thread.join();
        }
    };

    return 0;
}

