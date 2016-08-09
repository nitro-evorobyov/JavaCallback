#pragma once

#include "task.h"

namespace nitro
{
namespace task
{

typedef  TaskCallbackCommnad (*OnProgressCallback)(int currentStep, int upperBound);
typedef  TaskCallbackCommnad (*OnStart)();
typedef  TaskCallbackCommnad(*OnFinish)(TaskResult taskResult);


class TaskJni
    : public Task
{
public:
    TaskJni(int sleepCount, int failedStep);
    TaskJni(int sleepCount);

    ~TaskJni();

    void SetOnStart(OnStart onStart);
    void SetOnProgress(OnProgressCallback onProgress);
    void SetOnFinish(OnFinish onFinish);

private:
    OnProgressCallback  m_progressCallback;
    OnStart             m_startCallback;
    OnFinish            m_finishCallback;

    TaskCallbackCommnad OnStarted(const std::string& someMessage) override;
    TaskCallbackCommnad OnProgress(int progress, int upperBound, const std::string& someMessage) override;
    void OnFinished(TaskResult taskResult, const std::string& someMessage) override;
};

}
}
