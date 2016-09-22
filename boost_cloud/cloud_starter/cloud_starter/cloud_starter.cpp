// cloud_starter.cpp : Defines the entry point for the console application.
//
#include <Windows.h>
#include <thread>
#include <vector>
#include <sys/stat.h>


#include "../../include/define.h"
#include "../../include/mem/serializer.h"
#include "../../include/sync_out_puter.h"

INIT_SYNCOUTPUTER()

void PrepareSample()
{
    SYNC_OUTPUT("STARTER") << "Prepare sample data.";
    srand((unsigned int)time_t(0));

    struct _stat buf;

    

    bool generateNew = _stat(fileName, &buf) != 0;

    if (generateNew)
    {
        std::vector<std::shared_ptr<nitro::element::Row>>  collectionToWrite;

        const uint16_t rowsCount = 3000;

        SYNC_OUTPUT("STARTER") << "Generate template data";

        for (int rowIndex = 0; rowIndex < rowsCount; ++rowIndex)
        {
            uint16_t first = rand() % _countof(stringIniter);
            uint16_t second = rand() % _countof(stringIniter);

            if (first == second)
            {
                if (second < _countof(stringIniter))
                    ++second;
                else
                    --second;
            }

            std::string rowData(&stringIniter[min(first, second)], &stringIniter[max(first, second)]);
            std::shared_ptr<nitro::element::Row>    row(new nitro::element::Row(rowData));

            collectionToWrite.push_back(row);
        }

        SYNC_OUTPUT("STARTER") << "Write to file template data";

        nitro::serializer::Writer writer(fileName);
        writer.Write(collectionToWrite);
    }
}


int main(int argc, char* argv[])
{
    PrepareSample();

    STARTUPINFO si{};
    PROCESS_INFORMATION pi{};

    si.cb = sizeof(si);
    si.wShowWindow = SW_SHOW;

    // Start the child process. 
    if (!CreateProcess(L"cloud_server.exe",   // No module name (use command line)
        0,        // Command line
        NULL,           // Process handle not inheritable
        NULL,           // Thread handle not inheritable
        FALSE,          // Set handle inheritance to FALSE
        CREATE_NEW_CONSOLE,              // No creation flags
        NULL,           // Use parent's environment block
        NULL,           // Use parent's starting directory 
        &si,            // Pointer to STARTUPINFO structure
        &pi)           // Pointer to PROCESS_INFORMATION structure
        )
    {
        // // // SYNC_OUTPUT() << "[Main:]" << "CreateProcess failed. Last error = " << GetLastError() << ".";
        return false;
    }

    std::this_thread::sleep_for(std::chrono::seconds(1));

    for (int clientIndex = 0; clientIndex < 4; ++clientIndex)
    {
        // Start the child process. 
        if (!CreateProcess(L"cloud_client.exe",   // No module name (use command line)
            0,        // Command line
            NULL,           // Process handle not inheritable
            NULL,           // Thread handle not inheritable
            FALSE,          // Set handle inheritance to FALSE
            CREATE_NEW_CONSOLE,              // No creation flags
            NULL,           // Use parent's environment block
            NULL,           // Use parent's starting directory 
            &si,            // Pointer to STARTUPINFO structure
            &pi)           // Pointer to PROCESS_INFORMATION structure
            )
        {
            // // // SYNC_OUTPUT() << "[Main:]" << "CreateProcess failed. Last error = " << GetLastError() << ".";
            return false;
        }
    }
	return 0;
}

