

#if 1
#include <iostream>
#include "../../include/client_main_test.hpp"

int main(int argc, char* argv[], char* argp[])
{
    std::string     pipeName = argv[1];
    boost::asio::io_service ioService;
    return ClientMainTest
            <char, boost::asio::local::stream_protocol::socket>
            (ioService, [pipeName](boost::asio::local::stream_protocol::socket& clientTransport)
            {
                clientTransport.connect(boost::asio::local::stream_protocol::endpoint(pipeName));
                return true;
            });
}

#else

#include <boost/asio.hpp>
#include <iostream>

const char* epStr = "/tmp/socketDemo";

int main(int argc, const char* argv[]) {
    boost::asio::io_service io_service;
    boost::asio::local::stream_protocol::endpoint ep(epStr);
    boost::asio::local::stream_protocol::socket socket(io_service);

    socket.connect(ep);

    boost::asio::write(socket, boost::asio::buffer(argv[1], strlen(argv[1])), boost::asio::transfer_all());

    char buf[1024] = {0};

    size_t len = 0;
    while (1) {
        len += socket.read_some(boost::asio::buffer(buf + len, 2048 - len));
        std::cout << "read " << len << std::endl;
        if (buf[len] == '\0') {
            break;
        }
    }

    std::cout << "read " << len << " bytes\n";

    std::cout << buf << std::endl;
}

#endif