

#pragma once


#include <memory>
#include <functional>
#include <thread>
#include <mutex>
#include <vector>
#include <stdint.h>
#include <sstream>

#include "defines.h"

namespace Nitro
{
namespace Community
{

class TaskCounter
{
public:
    TaskCounter()
    {
        m_countLocker.lock();
        ++m_counter;
        m_countLocker.unlock();
    }

    ~TaskCounter()
    {
        m_countLocker.lock();
        --m_counter;
        m_countLocker.unlock();
    }


    static int GetTaskCount()
    {
        return m_counter;
    }

private:
    static std::mutex  m_countLocker;
    static int         m_counter;
};

#define INIT_TASK_COUNTER() std::mutex  Nitro::Community::TaskCounter::m_countLocker; \
        int  Nitro::Community::TaskCounter::m_counter = 0;

#define MESSAGE_SIZE    2048

template<class TChar>
class ServerTask
{
public:

    ServerTask(uv_loop_t* loopToInit, uv_stream_t* serverToInit)
        : m_currentState(State::None)
        , m_bufferSend(MESSAGE_SIZE, 0)
        , m_mainLoop(loopToInit)
        , m_pipe(new uv_pipe_t{})
        , m_work(new uv_work_t{})
        , m_id(std::rand() % 1024)
        , m_progress(0)
    {
        std::stringstream idStream;
        idStream << "TASK Id=" << m_id;
        m_strId = idStream.str();

        SYNC_OUTPUT(m_strId.c_str()) << "Start new task." << m_id;

        int r = uv_pipe_init(loopToInit, m_pipe.get(), NOIPC);;
        if (r) ERROR("initializing client pipe", r);

        SYNC_OUTPUT("SERVER") << "Init pipe data. Before accept.";

        r = uv_accept(serverToInit, (uv_stream_t*)m_pipe.get());

        m_pipe->data = this;
        if (r != 0)
        {
            uv_close((uv_handle_t*)m_pipe.get(), NULL);
            m_pipe.reset();
        }

        m_work->data = this;
        r = uv_queue_work(m_mainLoop, m_work.get(), SomeWorkHandleStat, AfterSomeWorkHandleStat);

        if (r != 0)
        {
            uv_close((uv_handle_t*)m_pipe.get(), NULL);
            m_pipe.reset();
            m_work.reset();
        }
    }


    ~ServerTask()
    {
        SYNC_OUTPUT(m_strId.c_str()) << "Finish Task.";
    }


    
    void StartListen() 
    {
        SYNC_OUTPUT(m_strId.c_str()) << "Async read start. State = " << m_currentState;

        if (m_pipe != nullptr)
        {
            SYNC_OUTPUT("SERVER") << "Accept succeeded. Start reading";
            uv_read_start((uv_stream_t*)m_pipe.get(), 
                          nitro_common::alloc_cb,
                          HandleReadStat);

            m_taskCounter = std::make_shared<TaskCounter>();
        }
    }


private:
    static void SomeWorkHandleStat(uv_work_t* req)
    {
        ServerTask* thisTask = reinterpret_cast<ServerTask*>(req->data);

        if (thisTask)
        {
            thisTask->m_progress++;
            std::this_thread::sleep_for(std::chrono::milliseconds(rand()%100));
            SYNC_OUTPUT(thisTask->m_strId.c_str()) << "******************* WORKING progress = " << thisTask->m_progress << "*******************";
            SYNC_OUTPUT(thisTask->m_strId.c_str()) << "Thread ID = " << std::this_thread::get_id();
        }
    }


    static void AfterSomeWorkHandleStat(uv_work_t* req, int status)
    {
        ServerTask* thisTask = reinterpret_cast<ServerTask*>(req->data);
        
        if (thisTask)
        {
            if (thisTask->m_currentState == State::Buy && thisTask->m_pipe == nullptr)
            {
                delete thisTask;
                return;
            }

            uv_queue_work(thisTask->m_mainLoop, req, SomeWorkHandleStat, AfterSomeWorkHandleStat);
        }
    }

    void DoResponse(std::vector<uint8_t>& responseBuffer)
    {
        ++m_currentState;

        std::stringstream     response;

        switch (m_currentState)
        {
            case State::Hello:
            {
                response << "Send hello from Server. Progress value = " << m_progress ;
            }
            break;
            case State::HowArYou:
            {
                response << "Send \"how are you\" from Server. Progress value = " << m_progress;
            }
            break;
            case State::BeforeRandomSleep:
            {
                response << "Send \"before random sleep\" from Server. Progress value = " << m_progress;
                std::this_thread::sleep_for(std::chrono::milliseconds(std::rand() % 2000));
            }
            break;
            case State::AfterRandomSleep:
            {
                response << "Send \"after random sleep\" from Server. Progress value = " << m_progress;
            }
            break;
            case State::Buy:
            {
                response << "Send buy from Server. Progress value = " << m_progress;
            }
            break;
        }

        int copyIndex = 0;
        auto responseString = response.str();
        for (auto& symbol : responseString)
        {
            responseBuffer[copyIndex++] = symbol;
        }
        responseBuffer[copyIndex++] = 0;
        responseBuffer[copyIndex++] = 0;
    }



    static void HandleReadStat(uv_stream_t* client, ssize_t nread, const uv_buf_t* buf) 
    {
        ServerTask* thisTask = reinterpret_cast<ServerTask*>(client->data);
        
        if (thisTask)
        {
            thisTask->HandleRead(client, nread, buf);
        }

    }

    void HandleRead(uv_stream_t* client, ssize_t nread, const uv_buf_t* buf)
    {
        SYNC_OUTPUT(m_strId.c_str())
            << "Async read end. Read buffer was get:"
            << (char *)&buf[0];

        if (nread < 0)
        {
            fprintf(stderr, "read error: [%s: %s]\n", uv_err_name(nread), uv_strerror(nread));

            if (nread == UV_EOF)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
            else
            {
                uv_close((uv_handle_t*)client, NULL);
            }
            return;
        }


        SYNC_OUTPUT(m_strId.c_str())
            << "Inside start read callback";

        uv_write_t *req = (uv_write_t*)malloc(sizeof(uv_write_t));
        req->data = this;

        DoResponse(m_bufferSend);

        free(buf->base);

        uv_buf_t bufResp{ m_bufferSend.size(), (char*) &m_bufferSend[0] };

        SYNC_OUTPUT(m_strId.c_str())
            << "Write answer.";
        uv_write (req,
                  client,
                  &bufResp,
                  1, 
                  HandleWriteStat);
    }

    static void HandleWriteStat(uv_write_t* req, int status)
    {
        ServerTask* thisTask = reinterpret_cast<ServerTask*>(req->data);

        if (thisTask)
        {
            thisTask->HandleWrite(req, status);

        }

    }

    void HandleWrite(uv_write_t* req, int status) 
    {
        SYNC_OUTPUT(m_strId.c_str()) << "Async write end.";

        if (m_currentState == State::Buy) 
        {
            uv_shutdown_t* shutdown = (uv_shutdown_t*)malloc(sizeof (uv_shutdown_t));
            shutdown->data = this;
            uv_shutdown(shutdown, (uv_stream_t*)m_pipe.get(), HandleShutdown);

            SYNC_OUTPUT(m_strId.c_str()) <<
                "current state is Buy. So we finish task.";


        }
        else
        {
            // std::this_thread::sleep_for(std::chrono::seconds(10));
            StartListen();
        }
    }

    static void HandleShutdown(uv_shutdown_t* req, int status)
    {
        ServerTask* thisTask = reinterpret_cast<ServerTask*>(req->data);

        if (thisTask)
        {
            uv_close((uv_handle_t*)thisTask->m_pipe.get(), HandleFinishClose);
        }
    }


    static void HandleFinishClose(uv_handle_t* handle)
    {
        ServerTask* thisTask = reinterpret_cast<ServerTask*>(handle->data);

        if (thisTask)
        {
            thisTask->m_pipe.reset();
        }
    }

    enum State
    {
        None,
        Hello,
        HowArYou,
        BeforeRandomSleep,
        AfterRandomSleep,
        Buy,
    };

    int                             m_id;
    std::string                     m_strId;

    int                             m_progress;
    int                             m_currentState;

    uv_loop_t*                      m_mainLoop;

    std::vector<uint8_t>            m_bufferSend;

    std::shared_ptr<uv_pipe_t>      m_pipe;
    std::shared_ptr<uv_work_t>      m_work;

    std::shared_ptr<TaskCounter>    m_taskCounter;
};


}
}