
#pragma once


#include <memory>
#include <functional>
#include <thread>
#include <mutex>

#include <boost/asio.hpp>
#include <boost/bind.hpp>

#include "define.h"
#include "sync_out_puter.h"
#include "command/command.h"

namespace nitro
{
namespace community
{

class ServerTaskAsync
{
public:
    ServerTaskAsync(boost::asio::io_service& ioService,
               command::statistic::ResultCollection&   resultCollector)
        : m_transport(ioService)
        , m_isBusy(false)
        , m_resultCollector(resultCollector)
    {
        std::stringstream   streamName;
        streamName << "Task[" << rand() % 1024 << "]";
        m_serverTaskName = streamName.str();

#ifdef ENABLE_LOG
        SYNC_OUTPUT(m_serverTaskName.c_str()) << "Start new task.";
#endif
    }


    ~ServerTaskAsync()
    {

#ifdef ENABLE_LOG
        SYNC_OUTPUT(m_serverTaskName.c_str()) << "Finish Task.";
#endif
        //helper::close(m_connection);
    }


    void SendCommad(std::shared_ptr<nitro::command::Base>& command)
    {
        m_isBusy = true;

        std::stringstream   commandBuffer;
        command::Base::Serialize(commandBuffer, *(command.get()));

        std::shared_ptr<std::string> sendMessageBuffer = std::make_shared<std::string>(commandBuffer.str());

        auto reuslt = std::make_shared<command::statistic::Result>();
        reuslt->request = *sendMessageBuffer;

        m_transport.async_write_some(
            boost::asio::buffer(*sendMessageBuffer),
            boost::bind(&ServerTaskAsync::HandleWrite,
            this,
            reuslt, 
            sendMessageBuffer,
            boost::asio::placeholders::error));

    }

    boost::asio::ip::tcp::socket&    GetTransport()
    {
        return m_transport;
    }

    bool IsBusy() const
    {
        return m_isBusy;
    }

private:
    void HandleRead(std::shared_ptr<command::statistic::Result> result,
                    std::shared_ptr<std::vector<char>> receiveMessageBuffer,
                    const boost::system::error_code& error)
    {
        m_isBusy = false;

        if (error) {
            SYNC_OUTPUT(m_serverTaskName.c_str()) << "ERROR!" << error;
            return;
        }

#ifdef ENABLE_LOG
        SYNC_OUTPUT(m_serverTaskName.c_str()) <<
            "Async read end. Read buffer was get:" << (char *)&(*receiveMessageBuffer)[0];
#endif
        result->response = (char *)&(*receiveMessageBuffer)[0];

        m_resultCollector.AddResult(result);
    }

    void HandleWrite(std::shared_ptr<command::statistic::Result> result,
                     std::shared_ptr<std::string> sendMessageBuffer,
                     const boost::system::error_code &error)
    {
        if (error) 
        {
            SYNC_OUTPUT(m_serverTaskName.c_str()) << "ERROR!" << error;

            m_isBusy = false;
            return;
        }

        std::shared_ptr<std::vector<char>> receiveMessageBuffer = std::make_shared<std::vector<char>>(MESSAGE_SIZE);

        m_transport.async_read_some(boost::asio::buffer(*receiveMessageBuffer),
                                    boost::bind(&ServerTaskAsync::HandleRead,
                                    this,
                                    result,
                                    receiveMessageBuffer,
                                    boost::asio::placeholders::error));
    }

private:
    bool                            m_isBusy;
    std::string                     m_serverTaskName;

    boost::asio::ip::tcp::socket            m_transport;
    command::statistic::ResultCollection&   m_resultCollector;
};

}
}