// cloud_client.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <thread>
#include <functional>

#include "helper.h"

#include "sync_out_puter.h"

std::mutex SyncOutPuter::globalStreamLock;

int _tmain(int argc, wchar_t* argv[])
{
    //::MessageBox(0, L"Attach!", L"Attach!", MB_OK);

    std::wstring    pipeName = argv[0];
    
    SYNC_OUTPUT() << "[Client:]" << "Start Main. Create pipe with name: " << pipeName.c_str();

    Nitro::Community::helper::native_io_t   connectPipe =  Nitro::Community::helper::connect_to_server(pipeName);

    SYNC_OUTPUT() << "[Client:]" << "Start Main."; 

    if (!Nitro::Community::helper::is_valid(connectPipe))
    {
        return -1;
    }


    boost::asio::io_service ioService;
    boost::asio::add_service(ioService, new boost::asio::windows::stream_handle_service(ioService));

    std::shared_ptr<boost::asio::io_service::work>  worker(new boost::asio::io_service::work(ioService));

    std::thread workerThread([&](){ ioService.run(); });

    Nitro::Community::helper::async_stream_t    streamToRead(ioService, connectPipe);

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

            streamToRead.async_write_some(boost::asio::buffer(bufferSend, 2048),
                                         [&](const boost::system::error_code& error, std::size_t bytes_transferred)
            {
                if (error)
                {
                    SYNC_OUTPUT() << "[Client:]" << "ERROR! During receiving buffer.";
                    stopProcessing = true;
                }


                SYNC_OUTPUT() << "[Client:]" << "After send buffer. Try to receive.";

                streamToRead.async_read_some(boost::asio::buffer(bufferRecv, 2048),
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

    Nitro::Community::helper::close(connectPipe);

    worker.reset();

    SYNC_OUTPUT() << "[Client:]" << "Exit main.";

    workerThread.join(); 

    //::MessageBox(0, L"Attach!", L"Attach!", MB_OK);
	return 0;
}

