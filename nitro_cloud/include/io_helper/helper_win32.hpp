
#pragma once

#include "server_task.hpp"
#include "pipe_api.hpp"

namespace Nitro
{
namespace Community
{

template<class TChar>
class Server
    : public boost::noncopyable
{
public:
    Server(const std::basic_string<TChar>& connectionName,
           boost::asio::io_service& service)
           : m_ioService(service)
           , m_serviceHolder(new boost::asio::io_service::work(service))
           , m_stopLoop(false)
           , m_pipeName(connectionName)
           , m_pipeServer(INVALID_HANDLE_VALUE)
    {
        SYNC_OUTPUT() << "[Server:]" << "Create server.";
        SYNC_OUTPUT() << "[Server:]" << "Connection name: " << connectionName.c_str();
        helper::create_server(m_pipeServer, connectionName);
    }

    ~Server()
    {
        helper::close(m_pipeServer);
        SYNC_OUTPUT() << "[Server:]" << "Finish server.";
    }

    void Run()
    {
        SYNC_OUTPUT() << "[Server:]" "Start awaiting loop.";

        SYNC_OUTPUT() << "[Server:]" "Start awaiting loop.";

        while (true)
        {
            if (m_stopLoop)
            {
                break;
            }

            auto serverTask = std::make_shared<ServerTask<TChar, boost::asio::windows::stream_handle>> (m_ioService);
            serverTask->GetTransport().assign(helper::accept_connection(m_pipeServer, m_pipeName));
            if (!m_stopLoop)//helper::is_valid(serverTask.GetTransport()))
            {
                serverTask->StartListen();
                continue;
            }

            break;
        }
   }


    void Stop()
    {
        m_stopLoop = true;
        m_serviceHolder.reset();

        HANDLE unblockConnection = helper::connect_to_server(m_pipeName);
        helper::close(unblockConnection);
    }

private:
    boost::asio::io_service&        m_ioService;

    HANDLE                          m_pipeServer;
    std::basic_string<TChar>        m_pipeName;

    bool                            m_stopLoop;

    std::shared_ptr<boost::asio::io_service::work>   m_serviceHolder;
};

}
}