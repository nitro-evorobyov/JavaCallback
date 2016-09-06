
#include <thread>

#include "defines.h"
#include "server_task.h"


namespace work_test
{
uv_loop_t *loop;
uv_work_t workPool[30];

void SomeWorkHandleStat(uv_work_t* req);
void AfterSomeWorkHandleStat(uv_work_t* req, int status);



void RunWorks()
{
    loop = uv_default_loop();

    SYNC_OUTPUT("WORK TEST") << "Start loop";

    for (int workIndex = 0; workIndex < _countof(workPool); ++workIndex)
    {
        workPool[workIndex].data = (void*)workIndex;
        uv_queue_work(loop, &workPool[workIndex], SomeWorkHandleStat, AfterSomeWorkHandleStat);
    }

    uv_run(loop, UV_RUN_DEFAULT);
}

void SomeWorkHandleStat(uv_work_t* req)
{
    Sleep(1000);
    auto workIndex = reinterpret_cast<int>(req->data);
    SYNC_OUTPUT("WORK") << "Woking... Index = " << workIndex << ". Thread ID = " << std::this_thread::get_id();
}


void AfterSomeWorkHandleStat(uv_work_t* req, int status)
{
    uv_queue_work(loop, req, SomeWorkHandleStat, AfterSomeWorkHandleStat);
}


}