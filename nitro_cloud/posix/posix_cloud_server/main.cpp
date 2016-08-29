

#if 0
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <iostream>
#include <thread>

using namespace std;

const char* epStr = "/tmp/socketDemo";

bool    isFinished = false;

void handle_accept(boost::asio::local::stream_protocol::acceptor&   acceptor,
                   boost::asio::local::stream_protocol::socket& socket)
{
    if(isFinished)
    {
        return;
    }

    char buf[2048] = {0};

    boost::asio::local::stream_protocol::socket socket_new(acceptor.get_io_service());
    acceptor.async_accept(socket_new,
                          boost::bind(&handle_accept,
                                      boost::ref(acceptor),
                                      boost::ref(socket_new)));
    size_t len = 0;
    while(1) {
        len += socket.read_some(boost::asio::buffer(buf + len, 2048 - len));
        cout << "read " << len << endl;
        if (buf[len] == '\0') {
            break;
        }
    }

    cout << "read " << len << " bytes" << endl;
    cout << buf << endl;

    boost::asio::write(socket, boost::asio::buffer(buf, len), boost::asio::transfer_all());

}

int main() {
    namespace local = boost::asio::local;

    boost::asio::io_service io_service;

    auto ioHolder = std::make_shared<boost::asio::io_service::work>(io_service);

    ::unlink(epStr);

    local::stream_protocol::endpoint ep(epStr);
    local::stream_protocol::acceptor acceptor(io_service, ep);


    std::thread thread([&]() { io_service.run(); });


    local::stream_protocol::socket socket(io_service);
    acceptor.async_accept(socket,
                          boost::bind(&handle_accept,
                                      boost::ref(acceptor),
                                      boost::ref(socket)));

    if(vfork() == 0){
        std::string applicationPath = "posix_cloud_client";
        char *argV[] = {&applicationPath[0], (char *) 0};
        int status = -1;

        status = execv("/mnt/hgfs/nitro_cloud/posix/bin/posix_cloud_client", argV);
        if (status == -1) {
            cout << "Launched Application";
            return true;
        }
        else {
            cout << "Launching application Failed";
            return false;
        }

        _exit(0);
    }

    ioHolder.reset();
    thread.join();
}

#else

#include <unistd.h>
#include "../../include/server_main_test.hpp"

int main()
{
    boost::asio::io_service ioSevice;

    std::basic_string<char>     pipeName = "/tmp/test_pipe";
    unlink(pipeName.c_str());
    return ServerMainTest<char>(pipeName, ioSevice, [&pipeName]() -> bool
    {
        try
        {
            if(vfork() == 0) {

                std::string applicationPath = "posix_cloud_client";
                char *argV[] = {&applicationPath[0], &pipeName[0], (char *) 0};
                int status = -1;

                status = execv("/mnt/hgfs/nitro_cloud/posix/bin/posix_cloud_client", argV);
                if (status == -1) {
                    SYNC_OUTPUT() << "Launched Application";
                    return true;
                }
                else {
                    SYNC_OUTPUT() << "Launching application Failed";
                    return false;
                }
            }

            return true;
        }
        catch(std::exception& ex)
        {
            SYNC_OUTPUT() << ex.what();
            return false;
        }
    });

    return 0;
}

#endif