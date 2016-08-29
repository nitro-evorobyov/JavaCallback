// cloud_client.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <string>

#include "client_main_test.hpp"

int _tmain(int argc, wchar_t* argv[])
{
    //::MessageBox(0, L"Attach!", L"Attach!", MB_OK);

    std::wstring    pipeName = argv[0];
    
    boost::asio::io_service ioService;
    boost::asio::add_service(ioService, new boost::asio::windows::stream_handle_service(ioService));

    return ClientMainTest<wchar_t, boost::asio::windows::stream_handle>(ioService, [&pipeName](boost::asio::windows::stream_handle& streamHandle) 
    {
        auto handle = Nitro::Community::helper::connect_to_server(pipeName);
        bool    isValid = Nitro::Community::helper::is_valid(handle);
        if (isValid)
        {
            streamHandle.assign(handle);
        }

        return isValid;
    });
}

