// nitro_nano.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <sstream>
#include <algorithm>
#include <assert.h>
#include <thread>
#include <sys/stat.h>

//#define SURVEY
//#define ENABLE_LOG

#ifdef SURVEY
#include "survey_socket.h"
#else
#include "pair_socket.h"
#endif



#include "command_impl.h"
#include "../../../nitro_cloud/include/sync_out_puter.h"

#pragma comment(lib, "nanomsg.lib")

const char  stringIniter[] = "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Vivamus euismod semper faucibus. Sed eget \
                              convallis nulla. Sed malesuada diam id erat vehicula, at blandit metus volutpat. Sed blandit pulvinar tellus. \
                              Mauris libero leo, porttitor non quam et, malesuada vestibulum felis. Aenean auctor laoreet fringilla. Aenean \
                              consequat, magna sed lacinia laoreet, leo libero rhoncus sapien, et ornare mauris neque ullamcorper ipsum. Mauris\
                               arcu lorem, mattis sed volutpat vel, tristique vel ante. Ut sagittis libero sit amet felis tempus aliquam. Donec\
                                efficitur condimentum urna id rutrum. Sed a posuere diam. Donec gravida sodales condimentum.";

const char fileName[] = "../test.txt";

INIT_SYNCOUTPUTER();




#ifndef SURVEY

int _tmain(int argc, _TCHAR* argv[])
{
#ifndef ENABLE_LOG
    nitro::AddExcludeLogWord("SERVER");
    nitro::AddExcludeLogWord("CommandGenerator");
#endif

    SYNC_OUTPUT("-SERVER-") << "Start server";
    srand((unsigned int)time_t(0));

    struct _stat buf;

    bool generateNew = _stat(fileName, &buf) != 0;

    if (generateNew)
    {
        std::vector<std::shared_ptr<nitro::element::Row>>  collectionToWrite;

        const uint16_t rowsCount = 3000;

        SYNC_OUTPUT("-SERVER-") << "Generate template data";

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

        SYNC_OUTPUT("-SERVER-") << "Write to file template data";

        nitro::serializer::Writer writer(fileName);
        writer.Write(collectionToWrite);
    }

    nitro::command::CommandGenerator    commandGenerator;

    SYNC_OUTPUT("-SERVER-") << "Go to the server mode. Listen connections.";
    nitro::socket_prototype::server(commandGenerator);

    //system("pause");

    return 0;
}


#else
int _tmain(int argc, _TCHAR* argv[])
{
    SYNC_OUTPUT("-SERVER-") << "Start server";
    srand((unsigned int)time_t(0));

    bool generateNew = true;

    if (generateNew)
    {
        std::vector<std::shared_ptr<nitro::element::Row>>  collectionToWrite;

        const uint16_t rowsCount = 3000;

        SYNC_OUTPUT("-SERVER-") << "Generate template data";

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

            std::string rowData(&stringIniter[std::min(first, second)], &stringIniter[std::max(first, second)]);
            std::shared_ptr<nitro::element::Row>    row(new nitro::element::Row(rowData));

            collectionToWrite.push_back(row);
        }

        SYNC_OUTPUT("-SERVER-") << "Write to file template data";

        nitro::serializer::Writer writer(fileName);
        writer.Write(collectionToWrite);
    }

    nitro::command::CommandGenerator    commandGenerator;

    SYNC_OUTPUT("-SERVER-") << "Go to the server mode. Listen connections.";
    nitro::socket_prototype::server(commandGenerator);

    return 0;
}
#endif