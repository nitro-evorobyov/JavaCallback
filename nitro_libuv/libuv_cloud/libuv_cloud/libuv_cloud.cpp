// libuv_cloud.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <iostream>
#include <thread>
#include <assert.h>
#include <fcntl.h>

#include "server.h"
#include "client.h"

#pragma comment (lib , "Ws2_32")
#pragma comment (lib , "Iphlpapi")
#pragma comment (lib , "Userenv")
#pragma comment (lib , "Psapi")
#pragma comment (lib , "libuv")

INIT_SYNCOUTPUTER()
INIT_TASK_COUNTER()

#if 1



int main(int c, _TCHAR* argv[]) {
    
#if 0
    SyncOutPuter::exclude.insert("SERVER"); 
    SyncOutPuter::exclude.insert("TASK");
#else
    SyncOutPuter::exclude.insert("Client");
#endif

    std::thread server([]()
    {
        nitro_server::RunServer();
    });

    std::this_thread::sleep_for(std::chrono::seconds(1));

    for (int stepIndex = 0; stepIndex < 5; ++stepIndex)
    {
        std::thread([]()
        {
            nitro_client::RunClient();
        }).detach();
    }

    server.join();
}

#else

#include "test_work.h"

int main(int c, _TCHAR* argv[]) 
{
    work_test::RunWorks();

}

#endif