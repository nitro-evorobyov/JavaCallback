// libuv_client.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "sync_out_puter.h"
#include "cs_libuv/client_main_test.hpp"

#pragma comment (lib , "Ws2_32")
#pragma comment (lib , "Iphlpapi")
#pragma comment (lib , "Userenv")
#pragma comment (lib , "Psapi")
#pragma comment (lib , "libuv")

INIT_SYNCOUTPUTER()



int main(int argc, char* argv[])
{
    ClientMainTest(SERVER_URI);
	return 0;
}

