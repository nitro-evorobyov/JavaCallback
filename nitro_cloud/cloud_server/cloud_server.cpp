// cloud_server.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"


#include "sync_out_puter.h"
#include "server_main_test.hpp"

int _tmain(int argc, wchar_t* argv[])
{
    std::wstring          pipeName = L"\\\\.\\pipe\\iohelper_testPipe";

    boost::asio::io_service ioService;
    boost::asio::add_service(ioService, new boost::asio::windows::stream_handle_service(ioService));

    return ServerMainTest<wchar_t>(pipeName, ioService, [&pipeName]() -> bool
    {
        STARTUPINFO si{};
        PROCESS_INFORMATION pi{};

        si.cb = sizeof(si);
        si.wShowWindow = SW_SHOW;

        // Start the child process. 
        if (!CreateProcess(L"cloud_client.exe",   // No module name (use command line)
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
            return false;
        }

        Sleep(1000);

        return true;
    });
}

