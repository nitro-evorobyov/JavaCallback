// one_process.cpp : Defines the entry point for the console application.
//


#include "stdafx.h"


#include <vector>
#include <thread>

#include <boost/asio/io_service.hpp>
#include <boost/bind.hpp>

#include "command/statistic.h"
#include "command/command.h"

#include "sample_prepare.h"
#include "performance_provider.h"

#include "define.h"

INIT_SYNCOUTPUTER()

#ifdef ASYNC_COMMUNCIATION

#include "cs_boost/server_async.hpp"



struct SendWorkerData
{
    std::shared_ptr<nitro::PerformanceProvider>         performance;
    std::shared_ptr<nitro::command::CommandGenerator>   commandGenereator; 
};


void SendCommandSession(boost::asio::io_service& ioService,
                        nitro::community::ServerAsync&   server,
                        std::shared_ptr<SendWorkerData> workerData)
{

    if (workerData->commandGenereator->HasCommands())
    {
        server.SendCommands(workerData->commandGenereator);
        ioService.post(boost::bind(SendCommandSession, std::ref(ioService), std::ref(server), workerData));
    }
    else
    {
        workerData->performance.reset();
        server.Stop();
    }
}

int main(int argc, char* argv[])
{
    nitro::PrepareSample(fileName, "Main");

    nitro::command::statistic::ResultCollection    resultCollection("../boost_result.txt");

    boost::asio::io_service         ioService;    
    nitro::community::ServerAsync   server(SERVER_URI, ioService, resultCollection);
    std::thread                     serviceThread([&]() { ioService.run(); });


    //ioService.

    server.StartListen();

    STARTUPINFO si{};
    PROCESS_INFORMATION pi{};

    si.cb = sizeof(si);
    si.wShowWindow = SW_SHOW;

    if (!CreateProcess(L"pdf_box.exe",   // No module name (use command line)
        0,        // Command line
        NULL,           // Process handle not inheritable
        NULL,           // Thread handle not inheritable
        FALSE,          // Set handle inheritance to FALSE
        CREATE_NEW_CONSOLE,              // No creation flags
        NULL,           // Use parent's environment block
        NULL,           // Use parent's starting directory 
        &si,            // Pointer to STARTUPINFO structure
        &pi)           // Pointer to PROCESS_INFORMATION structure
        )
    {
        // // // SYNC_OUTPUT() << "[Main:]" << "CreateProcess failed. Last error = " << GetLastError() << ".";
        return false;
    }

    auto workerData = std::make_shared<SendWorkerData>();

    workerData->commandGenereator = std::make_shared<nitro::command::CommandGenerator>();
    workerData->performance = std::make_shared<nitro::PerformanceProvider>();

    ioService.post(boost::bind(SendCommandSession, std::ref(ioService), std::ref(server), workerData));

    serviceThread.join();

    resultCollection.Dump();

	return 0;
}

#else

#include "cs_boost/server.hpp"





int main(int argc, char* argv[])
{
    nitro::PrepareSample(fileName, "Main");

    nitro::command::statistic::ResultCollection    resultCollection("../boost_result.txt");

    boost::asio::io_service     ioService;
    nitro::community::Server    server(SERVER_URI, ioService, resultCollection);
    std::thread                 serviceThread([&]() { ioService.run(); });


    STARTUPINFO si{};
    PROCESS_INFORMATION pi{};

    si.cb = sizeof(si);
    si.wShowWindow = SW_SHOW;

    if (!CreateProcess(L"pdf_box.exe",   // No module name (use command line)
        0,        // Command line
        NULL,           // Process handle not inheritable
        NULL,           // Thread handle not inheritable
        FALSE,          // Set handle inheritance to FALSE
        CREATE_NEW_CONSOLE,              // No creation flags
        NULL,           // Use parent's environment block
        NULL,           // Use parent's starting directory 
        &si,            // Pointer to STARTUPINFO structure
        &pi)           // Pointer to PROCESS_INFORMATION structure
        )
    {
        // // // SYNC_OUTPUT() << "[Main:]" << "CreateProcess failed. Last error = " << GetLastError() << ".";
        return false;
    }

    {
        nitro::PerformanceProvider performance;
        server.Listen();
        server.SendCommands(std::make_shared<nitro::command::CommandGenerator>());
    }

    server.Close();

    serviceThread.join();

    system("pause");

    resultCollection.Dump();

    return 0;
}


#endif