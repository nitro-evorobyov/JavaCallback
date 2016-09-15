
#pragma once

#include <sstream>
#include <assert.h>
#include <windows.h>
#include <algorithm>

#include <src/nn.h>
#include <src/pair.h>

#include "common_socket.h"

#include "serializer.h"
#include "command.h"


namespace nitro
{
namespace socket_prototype
{

class  ServerItem
{
public:

    ServerItem(int tcpPort)
      : m_commandsAtPull(0)
    {
        std::stringstream itemNameStream;
        itemNameStream << "Job " << tcpPort;
        m_itemName = itemNameStream.str();

        m_socket = nn_socket(AF_SP, NN_PAIR);

        if (m_socket < 0)
        {
            throw std::exception("Could not create socket.");
        }


        char naemBuffer[MAX_PATH] = { 0 };
        sprintf(naemBuffer, "%s%d", socketUrl, tcpPort);

        std::string     ipcName(naemBuffer);

        if (nn_bind(m_socket, ipcName.c_str()) < 0)
        {
            throw std::exception("Could not bind socket.");
        }

        STARTUPINFOA si{};
        PROCESS_INFORMATION pi{};

        si.cb = sizeof(si);
        si.wShowWindow = SW_SHOW;

        // Start the child process. 
        if (!CreateProcessA("nitro_nano_client.exe",   // No module name (use command line)
            (char*)&ipcName[0],        // Command line
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
            SYNC_OUTPUT(m_itemName.c_str()) << "CreateProcess failed. Last error = " << GetLastError() << ".";
            throw std::exception("Could not start process.");
        }

        Sleep(500);
    }

    bool SendNextCommand(command::ICommandGenerator& commandGenerator)
    {
        auto command = commandGenerator.NextCommand();

        if (command != nullptr)
        {
            std::stringstream   serializeStream;
            nitro::command::Base::Serialize(serializeStream, *command.get());

            auto serializeBuffer = serializeStream.str();

            //SYNC_OUTPUT("-SERVER-") << "nn_send - Send command data";
            while (true)
            {
                Sleep(10);
                int bytes = nn_send(m_socket, serializeBuffer.c_str(), serializeBuffer.size(), NN_DONTWAIT);

                if (bytes >= 0)
                {
                    ++m_commandsAtPull;

                    return true;
                }

                if (nn_errno() == EAGAIN)
                {
                    continue;
                }

                break;
            }
        }
        return false;
    }

    bool ReceiveCommandResponse()
    {
        char* receiveBuffer = NULL;

        int   countRecvBytes = nn_recv(m_socket, &receiveBuffer, NN_MSG, 0);

        if (countRecvBytes >= 0)
        {
            --m_commandsAtPull;

            std::vector<char>   bufferRecvVec(countRecvBytes + 1);

            memcpy(&bufferRecvVec[0], receiveBuffer, countRecvBytes);
            bufferRecvVec[countRecvBytes] = 0;

#ifdef ENABLE_LOG
            SYNC_OUTPUT(m_itemName.c_str()) << (char*)&bufferRecvVec[0];
#endif

            nn_freemsg(receiveBuffer);
        }

        return m_commandsAtPull > 0;
    }


    ~ServerItem()
    {
        nn_shutdown(m_socket, 0);
    }

private:
    int     m_socket;
    int     m_commandsAtPull;

    std::string m_itemName;
};

int server(command::ICommandGenerator& commandGenerator)
{
    std::vector<std::shared_ptr<ServerItem>>    serverItems; 

    const uint16_t processCount = 4;

    for (int processIndex = 0; processIndex < processCount; ++processIndex)
    {
        serverItems.push_back(std::make_shared<ServerItem>(processIndex));
    }

    int commandIndex = 0;
    PerformanceProvider     performance;
    while (true)
    {

#ifdef ENABLE_LOG
        SYNC_OUTPUT("-COMMAND-") << "command number : " << commandIndex;
#endif
        if (!serverItems[commandIndex % processCount]->SendNextCommand(commandGenerator))
        {
            break;
        }

        ++commandIndex;
    }

    while (!serverItems.empty())
    {
        serverItems.erase(std::remove_if(serverItems.begin(), serverItems.end(), [](std::shared_ptr<ServerItem>& item){return !item->ReceiveCommandResponse(); }),
                          serverItems.end());
    }

    return -1;
}

int client(command::ICommandGenerator& commandGenerator, const std::string& socketName)
{
    SYNC_OUTPUT("*CLIENT*") << "nn_socket - Create socket";

    int sock = nn_socket(AF_SP, NN_PAIR);

    assert(sock >= 0);

    SYNC_OUTPUT("*CLIENT*") << "nn_socket - connect to url : " << socketName; 
    if (nn_connect(sock, socketName.c_str()) >= 0)
    {
        int commandNumber = 0;

        PerformanceProvider performance;
        while (true)
        {
            char *buf = NULL;
            // SYNC_OUTPUT("*CLIENT*") << "nn_socket - receive some bytes";
            int bytes = nn_recv(sock, &buf, NN_MSG, 0);

            if (bytes > 0)
            {
                // SYNC_OUTPUT("*CLIENT*") << "Try to deserialize received buffer.";
                std::stringstream serializeStream;

                BufferToStream(buf, bytes, serializeStream);

                auto command = commandGenerator.FromStream(serializeStream);

                std::string runResult;
                if (command != nullptr)
                {
                    runResult = command->Run();
                }

                nn_freemsg(buf);


                ++ commandNumber;

                bool wasSend = false;
                while (!wasSend)
                {
                    bytes = nn_send(sock, runResult.c_str(), runResult.size(), NN_DONTWAIT);

                    if (bytes >= 0)
                    {
#ifdef ENABLE_LOG
                        SYNC_OUTPUT("*CLIENT*") << "[" << commandNumber << "]" << "Send response: " << runResult;
#endif
                        wasSend = true;
                    }

                    if (nn_errno() == EAGAIN)
                    {
                        continue;
                    }

                    break;
                }

                if (wasSend)
                {
                    continue;
                }
            }

            assert(0 && "Error");

            break;
        }

        return nn_shutdown(sock, 0);
    }

    return -1;
}

}
}