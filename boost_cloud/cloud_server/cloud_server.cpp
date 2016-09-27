// cloud_server.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"


#include "sync_out_puter.h"
#include "cs_boost/server_main_test.hpp"

INIT_SYNCOUTPUTER()


int _tmain(int argc, char* argv[])
{
    return ServerMainTest(SERVER_URI);/*, [&pipeName]() -> bool*/
}

