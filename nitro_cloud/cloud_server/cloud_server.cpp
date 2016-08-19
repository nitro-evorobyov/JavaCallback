// cloud_server.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"


#include "sync_out_puter.h"

#include "server.h"


std::mutex SyncOutPuter::globalStreamLock;

int _tmain(int argc, wchar_t* argv[])
{
    const std::wstring          pipeName = L"testPipe";
    
    boost::asio::io_service     ioService;
    boost::asio::add_service(ioService, new boost::asio::windows::stream_handle_service(ioService));

    boost::asio::strand         strand(ioService);
    
    Nitro::Community::Server<wchar_t>   server(pipeName, ioService, strand);


    std::thread woker1([&](){ioService.run(); });
    std::thread woker2([&](){ioService.run(); });
    std::thread woker3([&](){ioService.run(); });
    std::thread woker4([&](){ioService.run(); });
    std::thread woker5([&](){ioService.run(); });

    std::thread woker6([&]() { server.Run(); });

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
                STARTUPINFO si{};
                PROCESS_INFORMATION pi{};

                si.cb = sizeof(si);
                si.wShowWindow = SW_SHOW;

                std::basic_string<wchar_t> pipeName = server.GetPipeName();

                // Start the child process. 
                if (!CreateProcess(_T("cloud_client.exe"),   // No module name (use command line)
                    &pipeName[0],        // Command line
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
                    SYNC_OUTPUT() << "[Main:]" << "CreateProcess failed. Last error = " << GetLastError() << ".";
                    return -1;
                }
            }

            processNumber = 0;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(10500));
    }

    woker1.join();
    woker2.join();
    woker3.join();
    woker4.join(); 
    woker5.join();
    woker6.join();

	return 0;
}

