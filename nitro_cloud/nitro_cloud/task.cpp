
#include <thread>
#include <sstream>

#include "task.h"

#include "logger.h"

namespace nitro
{
namespace task
{


Task::Task(int sleepCount, int failedStep)
    : m_sleepCount(sleepCount)
    , m_failedStep(failedStep)
{
}

Task::~Task()
{
}

void Task::Run()
{
    std::thread([this](){

        std::thread::id threadId = std::this_thread::get_id();
        std::stringstream  javeMessageCreator;

        //NITRO_TRACE() << "Start new thread pid = " << threadId << ". Sleep count = " << m_sleepCount;
        if (m_failedStep >= 0)
        {
            //NITRO_TRACE() << ". Will fail on " << m_failedStep << " step.";
        }
        //NITRO_TRACE() << "Call: java callback - OnStarted";

        javeMessageCreator << "Java: Start." << " task id = " << threadId;
        if (OnStarted(javeMessageCreator.str()) == TaskCancel)
        {
            //NITRO_TRACE() << "OnStarted return TaskCancel. So thread canceled from java callback at beggin.";
            //NITRO_TRACE() << "Call: java callback - OnFinished";

            javeMessageCreator.str("");
            javeMessageCreator << "Java: TaskSwig canceled on startup.";
            OnFinished(Canceled, javeMessageCreator.str());

            return;
        }

        javeMessageCreator.str("");


        for (int step = 0; step < m_sleepCount; ++step)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));

            javeMessageCreator << "Java: In progress" << " task id = " << threadId;

            if (m_failedStep == step)
            {
                //NITRO_TRACE() << "Failed step handled.";
                //NITRO_TRACE() << "Call: java callback - OnFinished";

                javeMessageCreator.str("");
                javeMessageCreator << "Java: TaskSwig failed internal.";
                OnFinished(Failed, javeMessageCreator.str());
            }

            if (OnProgress(step, m_sleepCount, javeMessageCreator.str()) == TaskCancel)
            {
                //NITRO_TRACE() << "OnProgress return TaskCancel. So thread canceled from java callback at beggin.";
                //NITRO_TRACE() << "Call: java callback - OnFinished";

                javeMessageCreator.str("");
                javeMessageCreator << "Java: TaskSwig canceled in progress.";
                OnFinished(Canceled, javeMessageCreator.str());

                return;
            }

            javeMessageCreator.str("");
        }

        javeMessageCreator << "Java: Succeeded" << " task id = " << threadId;
        OnFinished(Succeeded, javeMessageCreator.str());
    }).detach();
}

}
}