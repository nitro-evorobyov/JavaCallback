// cloud_client.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <string>

#include "client_main_test.hpp"

int _tmain(int argc, wchar_t* argv[])
{
    //::MessageBox(0, L"Attach!", L"Attach!", MB_OK);

    std::wstring    pipeName = argv[0];
    
    return ClientMainTest(pipeName);
}

