

#pragma once


#include <memory>
#include <functional>
#include <thread>
#include <mutex>
#include <vector>
#include <stdint.h>
#include <sstream>

#include "uv.h"

#include "../sync_out_puter.h"
#include "../mem/serializer.h"
#include "../command/command.h"
#include "../define.h"


namespace nitro
{
namespace community
{

class ServerTask
{
public:

    ServerTask(uv_loop_t* loopToInit, uv_stream_t* serverToInit)
        : m_bufferSend(MESSAGE_SIZE, 0)
        , m_mainLoop(loopToInit)
        , m_transport(new uv_tcp_t{})
        , m_streamWriter(new uv_write_t{})
        , m_isBusy(false)
    {
        std::stringstream idStream;
        idStream << "TASK Id=" << std::rand() % 1024;
        m_strId = idStream.str();

        SYNC_OUTPUT(m_strId.c_str()) << "Start new task.";

        uv_tcp_init(m_mainLoop, m_transport.get());


        m_transport->data = this;


        if (uv_accept(serverToInit, (uv_stream_t*)m_transport.get()) != 0)
        {
            uv_close((uv_handle_t*)m_transport.get(), NULL);
            m_transport.reset();
            m_streamWriter.reset();
        }

        m_streamWriter->data = this; 
    }


    ~ServerTask()
    {
        SYNC_OUTPUT(m_strId.c_str()) << "Finish Task.";

        uv_close((uv_handle_t*)m_transport.get(), NULL);

        m_transport.reset();
        m_streamWriter.reset();
    }

    bool IsBusy() const
    {
        return m_isBusy;
    }


    void SendCommad(std::shared_ptr<nitro::command::Base>& command)
    {
        m_isBusy = true;

        std::stringstream   commandBuffer;
        command::Base::Serialize(commandBuffer, *(command.get()));

        m_bufferSend = commandBuffer.str();

        uv_buf_t bufResp = uv_buf_init(&m_bufferSend[0], m_bufferSend.size());

        uv_write(m_streamWriter.get(),
                 (uv_stream_t*)m_transport.get(),
                 &bufResp,
                 1,
                 HandleWrite);
    }




private:

    // memory management 
    static void HandleAllocate(uv_handle_t *handle, size_t size, uv_buf_t *buf) 
    {
        ServerTask* thisTask = reinterpret_cast<ServerTask*>(handle->data);
        
        if (thisTask->m_bufferRecv.size() < size)
        {
            thisTask->m_bufferRecv.resize(size + 1);
        }

        buf->base = &thisTask->m_bufferRecv[0];
        buf->len = size;
    }


    static void HandleRead(uv_stream_t* client, ssize_t nread, const uv_buf_t* buf)
    {
        ServerTask* thisTask = reinterpret_cast<ServerTask*>(client->data);

        if (thisTask == nullptr)
        {
            return;
        } 

        if (nread < 0)
        {
            SYNC_OUTPUT(thisTask->m_strId.c_str()) << "read error: [" << uv_err_name(nread) << ": " << uv_strerror(nread) << "]";
            uv_close((uv_handle_t*)client, NULL);
        }
        else
        {

#ifdef ENABLE_LOG
            SYNC_OUTPUT(thisTask->m_strId.c_str())
                << "Async read end. Read buffer was get:"
                << (char *)&buf->base[0];
#endif
            std::this_thread::yield();
        }

        thisTask->m_isBusy = false;
    }


    static void HandleWrite(uv_write_t* req, int status)
    {
        ServerTask* thisTask = reinterpret_cast<ServerTask*>(req->data);

        if (thisTask == nullptr ||
            thisTask->m_transport == nullptr)
        {
            return;
        }

        req->handle->data = thisTask;

        uv_read_start(req->handle,
                      HandleAllocate,
                      HandleRead);
    }


    std::string                     m_strId;
    uv_loop_t*                      m_mainLoop;
    
    std::shared_ptr<uv_tcp_t>       m_transport;
    std::shared_ptr<uv_write_t>     m_streamWriter;

    std::string                     m_bufferSend;
    std::vector<char>               m_bufferRecv;

    bool                            m_isBusy;
};


}
}