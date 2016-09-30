

#pragma once

#include <functional>
#include <thread>

#include <boost/bind.hpp>
#include <boost/asio.hpp>

#include "../define.h"
#include "../sync_out_puter.h"
#include "../command/command_impl.h"


char    bufferRecv[2048]{};

std::shared_ptr<nitro::serializer::Reader>  reader(new nitro::serializer::Reader(fileName));
nitro::command::CommandGenerator            generator(reader);

std::shared_ptr<boost::asio::ip::tcp::socket>   clientTransport;

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

    clientTransport->async_read_some(boost::asio::buffer(bufferRecv, 2048),
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
    auto receivedCommand = generator.FromStream(commandStream);

    std::shared_ptr<std::string> response = std::make_shared<std::string>();

    if (receivedCommand != nullptr)
    {
        *response = receivedCommand->Run();
    }

    clientTransport->async_write_some(boost::asio::buffer(*response),
                                      boost::bind(HandleWrite, _1, _2, response));
}


int ClientMainTest()
{
    boost::asio::io_service     ioService;

    std::shared_ptr<boost::asio::io_service::work>  worker(new boost::asio::io_service::work(ioService));
    std::thread workerThread([&](){ ioService.run(); });

    boost::asio::ip::tcp::endpoint addressPoint(boost::asio::ip::address::from_string(SERVER_URI), SERVER_PORT);

    clientTransport = std::make_shared<boost::asio::ip::tcp::socket>(ioService);
    clientTransport->connect(addressPoint);

    bool    stopProcessing = false;
    bool    readyToSend = true;



    clientTransport->async_read_some(boost::asio::buffer(bufferRecv, 2048),
                                     boost::bind(HandleRead, _1, _2));

    worker.reset();

    workerThread.join();

    return 0;

}


