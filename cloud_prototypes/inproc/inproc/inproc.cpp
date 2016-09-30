// inproc.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <vector>

#include "command/command_impl.h"
#include "command/statistic.h"

#include "sample_prepare.h"
#include "sync_out_puter.h"
#include "performance_provider.h"

#include "define.h"

INIT_SYNCOUTPUTER()


int main(int argc, char* argv[])
{
    nitro::PrepareSample(fileName, "Main");

    nitro::command::CommandGenerator    commandGenerator;
    
    std::vector<std::string>    requestQueue;


    nitro::command::statistic::ResultCollection resultCollection(fileStatistic);

    {
        nitro::PerformanceProvider performance;

        while (commandGenerator.HasCommands())
        {
            auto command = commandGenerator.NextCommand();
            std::stringstream       commandBuffer;
            if (command)
            {
                nitro::command::Base::Serialize(commandBuffer, *command);

                requestQueue.push_back(commandBuffer.str());
                commandBuffer.str("");

                continue;
            }

            break;
        }

        auto commandFileReader = std::make_shared<nitro::serializer::Reader>(fileName);
        nitro::command::CommandDeserializer commandDeserializer(commandFileReader);

        for (auto& request : requestQueue)
        {
            std::stringstream  streamRead(request);
            auto requestCommand = commandDeserializer.FromStream(streamRead);

            assert(requestCommand);

            if (requestCommand != nullptr)
            {
                auto response = requestCommand->Run();

                resultCollection.AddResult(std::shared_ptr<nitro::command::statistic::Result>(new nitro::command::statistic::Result{ request, response }));
            }
        }
    }

    resultCollection.Dump();

	return 0;
}

