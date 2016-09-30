
#pragma once

#include "server_async_task.hpp"

namespace nitro
{
namespace community
{

class ServerAsync
    : public boost::noncopyable
{
public:
    ServerAsync(const std::string& connectionUri,
           boost::asio::io_service& service,
           command::statistic::ResultCollection&  resultCollector)
           : m_ioService(service)
           , m_strand(service)
           , m_serviceHolder(new boost::asio::io_service::work(service))
           , m_stopLoop(false)
           , m_acceptor(service, boost::asio::ip::tcp::endpoint(boost::asio::ip::address::from_string(connectionUri), 80))
           , m_resultCollector(resultCollector)
    {
        SYNC_OUTPUT("ServerAsync")  << "Create server.";
    }

    ~ServerAsync()
    {
        m_acceptor.close();
        SYNC_OUTPUT("ServerAsync") << "Finish server.";
    }

    void StartListen()
    {
        SYNC_OUTPUT("ServerAsync") << "Start awaiting loop.";

        auto serverTask = std::make_shared <ServerTaskAsync>(m_ioService, m_resultCollector);

        m_acceptor.async_accept(serverTask->GetTransport(),
                                boost::bind(&ServerAsync::HandleAccept,
                                this,
                                serverTask,
                                boost::asio::placeholders::error));
    }

    void SendCommands(std::shared_ptr<command::CommandGenerator>& commandGenerator)
    {
        m_ioService.post(m_strand.wrap([this, &commandGenerator]()
        {
            m_pullLocker.lock();

            std::for_each(m_taskPull.begin(),
                          m_taskPull.end(),
                          [&commandGenerator](std::shared_ptr<ServerTaskAsync>& serverTask)
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

        }));
    }

    void Stop()
    {
        m_stopLoop = true;
        m_serviceHolder.reset();

        boost::asio::ip::tcp::socket unblockSocket(m_acceptor.get_io_service());
        unblockSocket.connect(m_acceptor.local_endpoint());
        unblockSocket.close();
    }

private:
    void HandleAccept(std::shared_ptr<ServerTaskAsync> serverTask,
                      const boost::system::error_code& error)
    {
        if (error)
        {
            return;
        }

        if (m_stopLoop)
        {
            return;
        }

        m_pullLocker.lock();
        m_taskPull.push_back(serverTask);
        m_pullLocker.unlock();

        serverTask = std::make_shared <ServerTaskAsync>(m_ioService, m_resultCollector);
        m_acceptor.async_accept(serverTask->GetTransport(),
                                boost::bind(&ServerAsync::HandleAccept,
                                this,
                                serverTask,
                                boost::asio::placeholders::error));
    }



private:
    boost::asio::io_service&            m_ioService;
    boost::asio::io_service::strand     m_strand;
    boost::asio::ip::tcp::acceptor      m_acceptor;

    bool                                m_stopLoop;

    std::vector<std::shared_ptr<ServerTaskAsync>>   m_taskPull;
    std::mutex                                      m_pullLocker;
    std::shared_ptr<boost::asio::io_service::work>  m_serviceHolder;

    command::statistic::ResultCollection&           m_resultCollector;

};

}
}