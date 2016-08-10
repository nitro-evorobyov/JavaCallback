#pragma once

#include "task.h"


namespace nitro
{
namespace task
{

class TaskSwig
    : public Task
{
public:
    TaskSwig(int sleepCount, int failedStep);
    TaskSwig(int sleepCount);

    virtual ~TaskSwig();

    TaskCallbackCommnad OnStarted(int taskId, const std::string& someMessage) override;
    TaskCallbackCommnad OnProgress(int progress, int upperBound, const std::string& someMessage) override;
    void OnFinished(TaskResult taskResult, const std::string& someMessage) override;

private:
    void HandleFail() override;
};

}
}
