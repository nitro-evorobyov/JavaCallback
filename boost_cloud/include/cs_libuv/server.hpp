
#pragma once

#include <string>
#include <mutex>
#include <memory>
#include <vector>

#include <uv.h>

#include "server_task.hpp"

namespace nitro
{
namespace community
{

class Server
{
public:
    Server(const std::string& connectionUri, 
           int  port,
           uv_loop_t*   loop)
      : m_loop(loop)
      , m_stopLoop(false)
    {
        signal(SIGINT, SignalInterruptedHandle);

        /* convert a humanreadable ip address to a c struct */
        union {
            sockaddr_in addr_in;
            sockaddr    addr;
        } sock_addr;

        uv_ip4_addr(connectionUri.c_str(), port, &sock_addr.addr_in);

        /* initialize the server */
        uv_tcp_init(m_loop, &m_server);
        /* bind the server to the address above */
        uv_tcp_bind(&m_server, &sock_addr.addr, 0);

        m_server.data = this;

        /* let the server listen on the address for new connections */
        int r = uv_listen((uv_stream_t *)&m_server, 128, HandleNewConnection);

        if (r)
        {
            SYNC_OUTPUT("Server") << "Error! Listening on socket: " << r;
            throw new std::exception("Error listening on socket");
        }


        SYNC_OUTPUT("Server")  << "Create server.";
    }

    ~Server()
    {
        m_taskPull.clear();

        SYNC_OUTPUT("Server") << "Finish server.";
    }


    void SendCommands(std::shared_ptr<command::ICommandGenerator>& commandGenerator)
    {
            m_pullLocker.lock();

            std::for_each(m_taskPull.begin(),
                          m_taskPull.end(),
                          [&commandGenerator](std::shared_ptr<ServerTask>& serverTask)
            {
                if (!serverTask->IsBusy())
                {
                    auto command = commandGenerator->NextCommand();
                    if (command != nullptr)
                    {
                        serverTask->SendCommad(command);
                    }
                }
            });

            m_pullLocker.unlock();
    }

    void Stop()
    {
        m_stopLoop = true;
    }

private:
    static void SignalInterruptedHandle(int ) {
        //int r;
        //uv_fs_t req;
        //r = uv_fs_unlink(m_loop, &req, PIPE_NAME, NULL);
        //if (r) ERROR("unlinking echo.sock", r);
        exit(0);
    }


    static void HandleNewConnection(uv_stream_t*  server, int status)
    {

        if (status < 0)
        {
            SYNC_OUTPUT("Server") << "New connection error: " << uv_strerror(status);
            return;
        }

        Server* serverThis = reinterpret_cast<Server*>(server->data);

        if (serverThis == nullptr)
        {
            return;
        }

        if (serverThis->m_stopLoop)
        {
            return;
        }
        serverThis->m_pullLocker.lock();

        auto serverTask = std::make_shared<ServerTask>(serverThis->m_loop, server);

        serverThis->m_taskPull.push_back(serverTask);
        serverThis->m_pullLocker.unlock();
    }



private:
    uv_loop_t*      m_loop;
    uv_tcp_t        m_server;

    bool            m_stopLoop;

    std::vector<std::shared_ptr<ServerTask>>        m_taskPull;
    std::mutex                                      m_pullLocker;
};

}
}