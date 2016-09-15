
#pragma once

#include <sstream>
#include <assert.h>
#include <thread>
#include <chrono>

#include <src/nn.h>
#include <src/survey.h>

#include "common_socket.h"

#include "serializer.h"
#include "command.h"


namespace nitro
{
namespace socket_prototype
{

int server(command::ICommandGenerator& commandGenerator)
{

    SYNC_OUTPUT("-SERVER-") << "nn_socket - Create socket";
    int sock = nn_socket(AF_SP, NN_SURVEYOR);

    assert(sock >= 0);

    SYNC_OUTPUT("-SERVER-") << "nn_bind - Start bind connection";
    if (nn_bind(sock, socketUrl) >= 0)
    {
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));

            PerformanceProvider performance;

            unsigned int taskIndex = 0;

            while (true)
            {
                auto command = commandGenerator.NextCommand();

                if (command != nullptr)
                {
                    std::stringstream   serializeStream;
                    nitro::command::Base::Serialize(serializeStream, *command.get());

                    auto serializeBuffer = serializeStream.str();

                    //SYNC_OUTPUT("-SERVER-") << "nn_send - Send command data";
                    int bytes = nn_send(sock, serializeBuffer.c_str(), serializeBuffer.size(), 0);
                    
                    std::this_thread::yield();

                    //std::this_thread::sleep_for(std::chrono::milliseconds(1));

                    if (bytes >= 0)
                    {
                        ++ taskIndex;
                        continue;
                    }

                    SYNC_OUTPUT("-SERVER-") << "nn_send - error";
                    break;
                }

                break;
            }

            SYNC_OUTPUT("-SERVER-") << "End processing commands. Commands count = " << taskIndex;
        }


        //nn_pollfd pfd{};
        //pfd.fd = sock;
        //pfd.events = NN_POLLOUT;

        //for (int count = 0; count < 10; ++count )
        //{
        //    
        //    std::this_thread::sleep_for(std::chrono::milliseconds(100));
        //}



        system("pause");

        SYNC_OUTPUT("-SERVER-") << "Shut down server.";

        return nn_shutdown(sock, 0);
    }

    return -1;
}

int client(command::ICommandGenerator& commandGenerator)
{
    SYNC_OUTPUT("*CLIENT*") << "nn_socket - Create socket";

    int sock = nn_socket(AF_SP, NN_RESPONDENT);

    assert(sock >= 0);

    SYNC_OUTPUT("*CLIENT*") << "nn_socket - connect to url";
    if (nn_connect(sock, socketUrl) >= 0)
    {
        PerformanceProvider performance;

        unsigned int taskNumber = 0;

        while (true)
        {
            char *buf = NULL;
            //SYNC_OUTPUT("*CLIENT*") << "nn_socket - receive some bytes";
            int bytes = nn_recv(sock, &buf, NN_MSG, 0);

            if (bytes >= 0)
            {
                //SYNC_OUTPUT("*CLIENT*") << "Try to deserialize received buffer. Task Numbrer = " << taskNumber;
                std::stringstream serializeStream;

                BufferToStream(buf, bytes, serializeStream);

                auto command = commandGenerator.FromStream(serializeStream);

                std::string runResult;
                if (command != nullptr)
                {
                    runResult = command->Run();
                    SYNC_OUTPUT("*CLIENT*") << "Task Numbrer = " << taskNumber << ". Result: " << runResult;

                }

                nn_freemsg(buf);

                //std::this_thread::yield();

                if (command != nullptr)
                {
                    ++ taskNumber; 
                    continue;
                }

                SYNC_OUTPUT("*CLIENT*") << "error client.";

                break;
            }
        }

        SYNC_OUTPUT("-Client-") << "Shut down client.";

        return nn_shutdown(sock, 0);
    }

    return -1;
}

}
}