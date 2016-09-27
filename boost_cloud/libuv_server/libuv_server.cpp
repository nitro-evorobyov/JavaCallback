// libuv_server.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "sync_out_puter.h"
#include "cs_libuv/server_main_test.hpp"

#pragma comment (lib , "Ws2_32")
#pragma comment (lib , "Iphlpapi")
#pragma comment (lib , "Userenv")
#pragma comment (lib , "Psapi")
#pragma comment (lib , "libuv")

INIT_SYNCOUTPUTER()

#pragma

int main(int argc, char* argv[])
{
    return ServerMainTest(SERVER_URI);/*, [&pipeName]() -> bool*/
}

