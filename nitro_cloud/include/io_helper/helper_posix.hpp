
#pragma once

#include "server_task.hpp"

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
                    , m_acceptor(service, boost::asio::local::stream_protocol::endpoint(connectionName))
            {
                SYNC_OUTPUT() << "[Server:]" << "Create server.";
                SYNC_OUTPUT() << "[Server:]" << "Connection name: " << connectionName;
            }

            ~Server()
            {
                m_acceptor.close();
                SYNC_OUTPUT() << "[Server:]" << "Finish server.";
            }

            void Run()
            {
                SYNC_OUTPUT() << "[Server:]" "Start awaiting loop.";



                auto serverTask =
                        std::make_shared<ServerTask
                                <TChar, boost::asio::local::stream_protocol::socket>>(m_ioService);

                m_acceptor.async_accept(serverTask->GetTransport(),
                                        boost::bind(&Server::HandleAccept,
                                                  this,
                                                  serverTask,
                                                  boost::asio::placeholders::error));
            }

            void HandleAccept(
                    std::shared_ptr<ServerTask<TChar, boost::asio::local::stream_protocol::socket>> serverTask,
                    const boost::system::error_code& error) {

                if(error)
                {
                    return;
                }

                if (m_stopLoop)
                {
                    return;
                }

                serverTask->StartListen();
                serverTask = std::make_shared<ServerTask
                        <TChar, boost::asio::local::stream_protocol::socket>>(m_ioService);
                m_acceptor.async_accept(serverTask->GetTransport(),
                                        boost::bind(&Server::HandleAccept,
                                                  this,
                                                  serverTask,
                                                  boost::asio::placeholders::error));
            }

           void Stop()
            {
                m_stopLoop = true;
                m_serviceHolder.reset();

                boost::asio::local::stream_protocol::socket unblockSocket(m_acceptor.get_io_service());
                unblockSocket.connect(m_acceptor.local_endpoint());
                unblockSocket.close();
            }

            std::basic_string<TChar>     GetPipeName()
            {
                std::basic_string<TChar>    pathPoint;
                m_acceptor.local_endpoint().path(pathPoint);
                return pathPoint;
            }

        private:
            boost::asio::io_service&        m_ioService;
            bool                            m_stopLoop;

            std::shared_ptr<boost::asio::io_service::work>   m_serviceHolder;
            boost::asio::local::stream_protocol::acceptor    m_acceptor;
        };

    }
}