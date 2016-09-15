// nitro_nano_client.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

// #define SURVEY
// #define ENABLE_LOG

#ifdef SURVEY
#include "survey_socket.h"
#else
#include "pair_socket.h"
#endif


#include "command_impl.h"
#include "../../../nitro_cloud/include/sync_out_puter.h"


#pragma comment(lib, "nanomsg.lib")

INIT_SYNCOUTPUTER();

const char fileName[] = "../test.txt";

int main(int argc, char* argv[])
{
    //nitro::AddExcludeLogWord("CLIENT");
    //nitro::AddExcludeLogWord("CommandGenerator");
    //nitro::AddExcludeLogWord("CountImpl");

    std::string     socketName;

    socketName = argv[0]; 

    nitro::command::CommandGenerator    commandGenerator;
    nitro::socket_prototype::client(commandGenerator, socketName);
    
    system("pause");

	return 0;
}

