

#pragma once

#include <functional>
#include <list>

#include "server.hpp"
#include "../performance_provider.h"
#include "../command/command_impl.h"

namespace
{

struct WorkParams
{
    uv_loop_t* loop;
    uv_work_t* work;
    nitro::community::Server*  server;
};

}

static std::shared_ptr<nitro::command::ICommandGenerator>  commandGenerator = std::make_shared<nitro::command::CommandGenerator>();
static std::shared_ptr<nitro::PerformanceProvider>  performance;


static void SomeWorkHandleStat(uv_work_t* req)
{
    {
        WorkParams* workParams = reinterpret_cast<WorkParams*>(req->data);


        if (commandGenerator->HasCommands())
        {
            workParams->server->SendCommands(commandGenerator);
            //std::this_thread::sleep_for(std::chrono::microseconds(1300));
        }
    }

}

static void  AfterSomeWorkHandleStat(uv_work_t* req, int status)
{
    WorkParams* workParams = reinterpret_cast<WorkParams*>(req->data);

    if (commandGenerator->HasCommands())
    {
        uv_queue_work(workParams->loop, req, SomeWorkHandleStat, AfterSomeWorkHandleStat);
    }
    else
    {
        performance.reset();
    }
}


int ServerMainTest(const std::string&  socketAddr)
{
    uv_loop_t* loop = uv_default_loop();

    SYNC_OUTPUT("SERVER") << "Start server";

    nitro::community::Server    server(socketAddr, SERVER_PORT, loop);

    uv_work_t   work;

    WorkParams workParams = { loop, &work, &server };
    work.data = &workParams;

    uv_queue_work(loop, &work, SomeWorkHandleStat, AfterSomeWorkHandleStat);

    performance.reset(new nitro::PerformanceProvider());
    uv_run(loop, UV_RUN_DEFAULT);

    system("pause");

    server.Stop();


    return 0;
}

