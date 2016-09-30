
#pragma once

#include <string>
#include <sys/stat.h>


#include "sync_out_puter.h"
#include "mem/serializer.h"

#include "define.h"

namespace nitro
{

void PrepareSample(const char* fileName, const char* logName)
{
    SYNC_OUTPUT(logName) << "Prepare sample data.";
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

            std::string rowData(&stringIniter[std::min(first, second)], &stringIniter[std::max(first, second)]);
            std::shared_ptr<nitro::element::Row>    row(new nitro::element::Row(rowData));

            collectionToWrite.push_back(row);
        }

        SYNC_OUTPUT("STARTER") << "Write to file template data";

        nitro::serializer::Writer writer(fileName);
        writer.Write(collectionToWrite);
    }
}

}
