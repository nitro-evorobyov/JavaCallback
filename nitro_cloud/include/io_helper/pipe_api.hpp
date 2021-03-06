/* vim:set ts=3 sw=3 sts=3 et: */
/**
 * Copyright © 2008-2013 Last.fm Limited
 *
 * This file is part of libmoost.
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef MOOST_IO_DETAIL_HELPER_WIN32_HPP__
#define MOOST_IO_DETAIL_HELPER_WIN32_HPP__


#include <boost/asio.hpp>
#include <windows.h>
#include <strsafe.h>

namespace Nitro
{
namespace Community
{

class helper
{
public:
    static bool close(HANDLE in)
    {
        return CloseHandle(in) == TRUE;
    }

    template<class TChar>
    static void create_server(HANDLE& server, const std::basic_string<TChar>& pipeName)
    {
        unsigned int step = 0;
        const unsigned int max_tryes = 10;

        HANDLE  pipe_server;

        while (step < max_tryes)
        {

            pipe_server = CreateNamedPipe(pipeName.c_str(),
                                          PIPE_ACCESS_DUPLEX |      // read/write access 
                                          FILE_FLAG_OVERLAPPED,     // Overlapped
                                          PIPE_TYPE_MESSAGE |       // message type pipe 
                                          PIPE_READMODE_MESSAGE |   // message-read mode 
                                          PIPE_WAIT,                // blocking mode 
                                          PIPE_UNLIMITED_INSTANCES, // max. instances  
                                          2048,                  // output buffer size 
                                          2048,                  // input buffer size 
                                          0,                        // client time-out 
                                          nullptr);

            if (pipe_server != INVALID_HANDLE_VALUE)
            {
                break;
            }

            ++step;
        }

        if (pipe_server == INVALID_HANDLE_VALUE)
        {
            throw std::runtime_error("failed to create named pipe");
        }

        server = pipe_server;
    }


    template<class TChar>
    static HANDLE accept_connection(HANDLE& server_instance, const std::basic_string<TChar>& pipeName)
    {
        HANDLE     connection_instance = INVALID_HANDLE_VALUE;

        if (server_instance != INVALID_HANDLE_VALUE)
        {
            if (ConnectNamedPipe(server_instance, nullptr) || 
                (GetLastError() == ERROR_PIPE_CONNECTED))
            {
                connection_instance = server_instance;

                create_server(server_instance, pipeName);
            }
        }

        return connection_instance;
    }

    static bool is_valid(HANDLE&  io_to_check)
    {
        return io_to_check != INVALID_HANDLE_VALUE;
    }

    template<class TChar>
    static HANDLE connect_to_server(const std::basic_string<TChar>& pipeName)
    {
        const unsigned int  connectionTimes = 10;
        HANDLE         connection_instance = INVALID_HANDLE_VALUE;

        for (unsigned int connectionIndex = 0; connectionTimes > connectionIndex; ++connectionIndex)
        {
            connection_instance =
                CreateFile(pipeName.c_str(),
                GENERIC_READ | GENERIC_WRITE,
                0,
                nullptr,
                OPEN_EXISTING,
                FILE_FLAG_OVERLAPPED,
                nullptr);

            if (connection_instance != INVALID_HANDLE_VALUE)
                break;


            if (GetLastError() != ERROR_PIPE_BUSY)
            {
                throw std::runtime_error("Failed to open named pipe");
            }

            if (!WaitNamedPipe(pipeName.c_str(), 200))
            {
                throw std::runtime_error("Could not open pipe: 200 millisecond wait timed out.");
            }

        }

        if (connection_instance == INVALID_HANDLE_VALUE)
        {
            throw std::runtime_error("Could not create pipe. pipe is busy.");
        }
        return connection_instance;
    }

};
 
}
}

#endif
