#include "task_jni.h"

namespace nitro
{
namespace task
{

TaskJni::TaskJni(int sleepCount, int failedStep)
    : Task(sleepCount, failedStep)
{
}

TaskJni::TaskJni(int sleepCount)
    : Task(sleepCount, -1)
{
}

TaskJni::~TaskJni()
{
}

void TaskJni::SetOnStart(OnStart onStart)
{
    m_startCallback = onStart;
}

void TaskJni::SetOnProgress(OnProgressCallback onProgress)
{
    m_progressCallback = onProgress;
}

void TaskJni::SetOnFinish(OnFinish onFinish)
{
    m_finishCallback = onFinish;
}


TaskCallbackCommnad TaskJni::OnStarted(int taskId, const std::string& /*someMessage*/)
{
    return m_startCallback();
}

TaskCallbackCommnad TaskJni::OnProgress(int progress, int upperBound, const std::string& /*someMessage*/)
{
    return m_progressCallback ? m_progressCallback(progress, upperBound) : TaskNotImplemented;
}

void TaskJni::OnFinished(TaskResult taskResult, const std::string& /*someMessage*/)
{
    m_finishCallback(taskResult);
}

}
}