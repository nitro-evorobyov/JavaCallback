
#pragma once
#include <boost/asio.hpp>

#include "define.h"
#include "sync_out_puter.h"
#include "command/command.h"

namespace nitro
{
namespace community
{

class Server
    : public boost::noncopyable
{
public:
    Server(const std::string& connectionUri,
           boost::asio::io_service& service,
           command::statistic::ResultCollection&  resultCollector)
           : m_ioService(service)
           , m_serviceHolder(new boost::asio::io_service::work(service))
           , m_acceptor(service, boost::asio::ip::tcp::endpoint(boost::asio::ip::address::from_string(connectionUri), 80))
           , m_socket(service)
           , m_resultCollector(resultCollector)
    {
        SYNC_OUTPUT("Server")  << "Create server.";
    }

    ~Server()
    {
        m_acceptor.close();
        SYNC_OUTPUT("Server") << "Finish server.";
    }

    void Listen()
    {
        SYNC_OUTPUT("Server") << "Listen.";

        m_acceptor.accept(m_socket);
    }

    void SendCommands(std::shared_ptr<command::CommandGenerator>& commandGenerator)
    {
        auto command = commandGenerator->NextCommand();

        try
        {
            while (command != nullptr)
            {
                std::stringstream   commandBuffer;
                command::Base::Serialize(commandBuffer, *(command.get()));

                std::string sendMessageBuffer = commandBuffer.str();

                boost::system::error_code ignored_error;
                boost::asio::write(m_socket, boost::asio::buffer(sendMessageBuffer),
                                   boost::asio::transfer_all(), ignored_error);

                std::array<char, 2048> receiveMessageBuffer{};

                m_socket.read_some(boost::asio::buffer(receiveMessageBuffer), ignored_error);

                auto reuslt = std::make_shared<command::statistic::Result>();
                reuslt->request = sendMessageBuffer;
                reuslt->response = &receiveMessageBuffer[0];

                m_resultCollector.AddResult(reuslt);

                command = commandGenerator->NextCommand();
            }
        }
        catch (std::exception& e)
        {
            std::cerr << e.what() << std::endl;
        }
    }

    void Close()
    {
        m_serviceHolder.reset();
    }


private:
    boost::asio::io_service&            m_ioService;
    boost::asio::ip::tcp::acceptor      m_acceptor;
    boost::asio::ip::tcp::socket        m_socket;

    std::shared_ptr<boost::asio::io_service::work>  m_serviceHolder;

    command::statistic::ResultCollection&           m_resultCollector;
};

}
}