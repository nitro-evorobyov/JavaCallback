
// Windows Header Files:
#include <windows.h>

#include <thread>
#include <sstream>

#include "task_swig.h"

#include "logger.h"


namespace nitro
{
namespace task
{

TaskSwig::TaskSwig(int sleepCount, int failedStep)
    : Task(sleepCount, failedStep)
{
}


TaskSwig::TaskSwig(int sleepCount)
    : Task(sleepCount, -1)
{
}

TaskSwig::~TaskSwig()
{
}

TaskCallbackCommnad TaskSwig::OnStarted(const std::string& /*someMessage*/)
{
    return TaskNotImplemented;
}

TaskCallbackCommnad TaskSwig::OnProgress(int /*progress*/, int /*upperBound*/, const std::string& /*someMessage*/)
{
    return TaskNotImplemented;
}

void TaskSwig::OnFinished(TaskResult /*taskResult*/, const std::string& /*someMessage*/)
{
}

}
}