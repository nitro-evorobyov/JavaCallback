

#pragma once

#include <functional>
#include <thread>

#include <boost/asio.hpp>

#include "define.h"
#include "sync_out_puter.h"
#include "command/command_impl.h"


int ClientMainTest()
{
    boost::asio::io_service     ioService;

    std::shared_ptr<boost::asio::io_service::work>  worker(new boost::asio::io_service::work(ioService));
    std::thread workerThread([&](){ ioService.run(); });

    boost::asio::ip::tcp::endpoint addressPoint(boost::asio::ip::address::from_string(SERVER_URI), SERVER_PORT);

    boost::asio::ip::tcp::socket    clientTransport(ioService);
    clientTransport.connect(addressPoint);

    bool    stopProcessing = false;
    bool    readyToSend = true;

    char    bufferRecv[2048]{};

    while (!stopProcessing)
    {
        if (readyToSend)
        {
            readyToSend = false;

            // SYNC_OUTPUT() << "[Client:]" << "Before send buffer:" << bufferSend;

            clientTransport.async_read_some(boost::asio::buffer(bufferRecv, 2048),
                                          [&](const boost::system::error_code& error, std::size_t bytes_transferred)
            {
                if (error)
                {
                    SYNC_OUTPUT("Client")  << "ERROR! During receiving buffer.";
                    stopProcessing = true;
                }


                nitro::command::CommandGenerator    generator;

                std::stringstream   commandStream;
                commandStream.str(&bufferRecv[0]);
                auto receivedCommand = generator.FromStream(commandStream);

                std::shared_ptr<std::string> response = std::make_shared<std::string>();

                if (receivedCommand != nullptr)
                {
                    *response = receivedCommand->Run();
                }

                clientTransport.async_write_some(boost::asio::buffer(*response),
                                                 [&, response](const boost::system::error_code& error, std::size_t bytes_transferred)
                {
                    if (error)
                    {
                        stopProcessing = true;
                    }

                    readyToSend = true;
                });
            });
        }

        std::this_thread::yield();
    }

    clientTransport.close();

    worker.reset();

    workerThread.join();

    return 0;

}

