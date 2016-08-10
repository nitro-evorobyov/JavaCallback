#pragma once

#include <string>
#include "task_result.h"

namespace nitro
{
namespace task
{


class Task
{
public:
    Task(int sleepCount, int failedStep);
    ~Task();

    void  Run();

private:
    int     m_sleepCount;
    int     m_failedStep;

    virtual TaskCallbackCommnad OnStarted(int taskId, const std::string& someMessage) = 0;
    virtual TaskCallbackCommnad OnProgress(int progress, int upperBound, const std::string& someMessage) = 0;
    virtual void OnFinished(TaskResult taskResult, const std::string& someMessage) = 0;

    virtual void HandleFail() = 0;
};

}
}