// cloud_server.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"


#include "sync_out_puter.h"
#include "server_main_test.hpp"

std::mutex SyncOutPuter::globalStreamLock;


int _tmain(int argc, wchar_t* argv[])
{
    const std::wstring          pipeName = L"testPipe";
    return ServerMainTest<wchar_t>(pipeName, [](std::basic_string<wchar_t>& pipeName) -> bool
    {
        STARTUPINFO si{};
        PROCESS_INFORMATION pi{};

        si.cb = sizeof(si);
        si.wShowWindow = SW_SHOW;

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
            return false;
        }

        return true;
    });
}

