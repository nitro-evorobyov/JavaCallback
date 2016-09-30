// pdf_box.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <functional>
#include <thread>

#include <boost/bind.hpp>
#include <boost/asio.hpp>

#include "define.h"
#include "sync_out_puter.h"
#include "command/command_impl.h"

INIT_SYNCOUTPUTER()

char    bufferRecv[MESSAGE_SIZE]{};

std::shared_ptr<nitro::serializer::Reader>  reader(new nitro::serializer::Reader(fileName));
nitro::command::CommandDeserializer         deserializer(reader);

std::shared_ptr<boost::asio::ip::tcp::socket>   clientTransport;


#ifdef ASYNC_COMMUNCIATION

void HandleRead(const boost::system::error_code& error, std::size_t bytes_transferred);

void HandleWrite(const boost::system::error_code& error,
                 std::size_t bytes_transferred,
                 std::shared_ptr<std::string> bufferHolder)
{
    if (error)
    {
        SYNC_OUTPUT("Client") << "ERROR! During sending buffer.";
        return;
    }

    clientTransport->async_read_some(boost::asio::buffer(bufferRecv, MESSAGE_SIZE),
                                     boost::bind(HandleRead, _1, _2));
}


void HandleRead(const boost::system::error_code& error, std::size_t bytes_transferred)
{
    if (error)
    {
        SYNC_OUTPUT("Client") << "ERROR! During receiving buffer.";
        return;
    }

    std::stringstream   commandStream;
    commandStream.str(&bufferRecv[0]);
    auto receivedCommand = deserializer.FromStream(commandStream);

    std::shared_ptr<std::string> response = std::make_shared<std::string>();

    if (receivedCommand != nullptr)
    {
        *response = receivedCommand->Run();
    }

    clientTransport->async_write_some(boost::asio::buffer(*response),
                                      boost::bind(HandleWrite, _1, _2, response));
}


int main(int argc, char* argv[])
{
    boost::asio::io_service     ioService;

    std::shared_ptr<boost::asio::io_service::work>  worker(new boost::asio::io_service::work(ioService));

    std::thread workerThread([&](){ ioService.run(); });
    std::thread workerThread2([&](){ ioService.run(); });

    boost::asio::ip::tcp::endpoint addressPoint(boost::asio::ip::address::from_string(SERVER_URI), SERVER_PORT);

    clientTransport = std::make_shared<boost::asio::ip::tcp::socket>(ioService);
    clientTransport->connect(addressPoint);

    bool    stopProcessing = false;
    bool    readyToSend = true;



    clientTransport->async_read_some(boost::asio::buffer(bufferRecv, MESSAGE_SIZE),
                                     boost::bind(HandleRead, _1, _2));

    worker.reset();

    workerThread.join();
    workerThread2.join();

    return 0;
}

#else


int main(int argc, char* argv[])
{
    boost::asio::io_service     ioService;

    std::shared_ptr<boost::asio::io_service::work>  worker(new boost::asio::io_service::work(ioService));

    std::thread workerThread([&](){ ioService.run(); });

    boost::asio::ip::tcp::endpoint addressPoint(boost::asio::ip::address::from_string(SERVER_URI), SERVER_PORT);

    clientTransport = std::make_shared<boost::asio::ip::tcp::socket>(ioService);
    clientTransport->connect(addressPoint);

    bool    stopProcessing = false;
    bool    readyToSend = true;


    try
    {
        while (true)
        {
            boost::system::error_code ignored_error;

            clientTransport->read_some(boost::asio::buffer(bufferRecv, MESSAGE_SIZE), ignored_error);

            std::stringstream   commandStream;
            commandStream.str(&bufferRecv[0]);
            auto receivedCommand = deserializer.FromStream(commandStream);

            std::shared_ptr<std::string> response = std::make_shared<std::string>();

            if (receivedCommand != nullptr)
            {
                *response = receivedCommand->Run();
            }

            clientTransport->write_some(boost::asio::buffer(*response), ignored_error);
        }
    }
    catch (std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }

    worker.reset();

    workerThread.join();

    return 0;
}

#endif