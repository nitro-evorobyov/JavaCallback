

#pragma once

#include <functional>
#include <thread>

#include <boost/asio.hpp>

#include "sync_out_puter.h"

#include "helper.h"


std::mutex SyncOutPuter::globalStreamLock;

template<class TChar, class TAsioTransport>
int ClientMainTest(boost::asio::io_service& ioService, std::function<bool(TAsioTransport&)> transportInit)
{
    SYNC_OUTPUT() << "[Client:]" << "Start Main. Create pipe with name: ";

    // 

    std::shared_ptr<boost::asio::io_service::work>  worker(new boost::asio::io_service::work(ioService));
    std::thread workerThread([&](){ ioService.run(); });

    TAsioTransport     clientTransport(ioService);

    transportInit(clientTransport);//clientTransport.connect(boost::asio::local::stream_protocol::endpoint(pipeName));

    bool    stopProcessing = false;
    bool    readyToSend = true;

    char    bufferSend[2048] = "Test!";
    char    bufferRecv[2048]{};

    while (!stopProcessing)
    {
        if (readyToSend)
        {
            readyToSend = false;

            SYNC_OUTPUT() << "[Client:]" << "Before send buffer:" << bufferSend;

            clientTransport.async_write_some(boost::asio::buffer(bufferSend, 2048),
                                          [&](const boost::system::error_code& error, std::size_t bytes_transferred)
            {
                if (error)
                {
                    SYNC_OUTPUT() << "[Client:]" << "ERROR! During receiving buffer.";
                    stopProcessing = true;
                }


                SYNC_OUTPUT() << "[Client:]" << "After send buffer. Try to receive.";

                clientTransport.async_read_some(boost::asio::buffer(bufferRecv, 2048),
                                             [&](const boost::system::error_code& error, std::size_t bytes_transferred)
                {
                    if (error)
                    {
                        SYNC_OUTPUT() << "[Client:]" << "ERROR! During receiving buffer.";
                        stopProcessing = true;
                    }

                    SYNC_OUTPUT() << "[Client:]" << "After receive buffer. Buffer:" << bufferRecv;

                    readyToSend = true;
                });
            });
        }

        SYNC_OUTPUT() << "[Client:]" << "Clinet main thread iteration.";

        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    clientTransport.close();

    worker.reset();

    SYNC_OUTPUT() << "[Client:]" << "Exit main.";

    workerThread.join();

    //::MessageBox(0, L"Attach!", L"Attach!", MB_OK);

    return 0;

}

