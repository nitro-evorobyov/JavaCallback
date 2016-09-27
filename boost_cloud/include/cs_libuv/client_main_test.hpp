

#pragma once

#include <functional>
#include <thread>

#include <uv.h>


#include "../define.h"
#include "../sync_out_puter.h"
#include "../command/command_impl.h"


namespace nitro
{
namespace community
{

class ClientTask
{
public:

    ClientTask()
        : m_bufferSend(MESSAGE_SIZE, 0)
    {
        std::shared_ptr<nitro::serializer::Reader> reader(new nitro::serializer::Reader(fileName));
        m_commandGenerator = std::make_shared<nitro::command::CommandGenerator>(reader);

        SYNC_OUTPUT("Client") << "Start new client task.";

        m_streamWriter.data = this;
    }


    ~ClientTask()
    {
        SYNC_OUTPUT("Client") << "Finish Task.";

        //uv_close((uv_handle_t*)m_transport, nullptr);
    }


    void StartListen(uv_stream_t*  transport)
    {
        transport->data = this;
        uv_read_start(transport, HandleAllocate, HandleRead);
    }


private:

    // memory management 
    static void HandleAllocate(uv_handle_t *handle, size_t size, uv_buf_t *buf)
    {
        ClientTask* thisTask = reinterpret_cast<ClientTask*>(handle->data);

        if (thisTask->m_bufferRecv.size() < size)
        {
            thisTask->m_bufferRecv.resize(size + 1);
        }

        buf->base = &thisTask->m_bufferRecv[0];
        buf->len = size;
    }


    static void HandleRead(uv_stream_t* client, ssize_t nread, const uv_buf_t* buf)
    {
        ClientTask* thisTask = reinterpret_cast<ClientTask*>(client->data);

        if (thisTask == nullptr)
        {
            return;
        }

        if (nread < 0)
        {
            SYNC_OUTPUT("Client") << "read error: [" << uv_err_name(nread) << ": " << uv_strerror(nread) << "]";
            uv_close((uv_handle_t*)client, NULL);
        }
        else
        {

#ifdef ENABLE_LOG
            SYNC_OUTPUT("Client")
                << "Async read end. Read buffer was get:"
                << (char *)&buf->base[0];
#endif
            uv_read_stop(client);


            std::stringstream   commandStream;
            commandStream.str(&buf->base[0]);
            auto receivedCommand = thisTask->m_commandGenerator->FromStream(commandStream);


            if (receivedCommand != nullptr)
            {
                thisTask->m_bufferSend = receivedCommand->Run();


                uv_buf_t bufResp = uv_buf_init(&thisTask->m_bufferSend[0], thisTask->m_bufferSend.size());

                uv_write(&thisTask->m_streamWriter,
                         client,
                         &bufResp,
                         1,
                         HandleWrite);
            }


            std::this_thread::yield();
        }
    }


    static void HandleWrite(uv_write_t* req, int status)
    {
        ClientTask* thisTask = reinterpret_cast<ClientTask*>(req->data);

        if (thisTask == nullptr)
        {
            return;
        }

        req->handle->data = thisTask;

        uv_read_start(req->handle, HandleAllocate, HandleRead);
    }


    std::string             m_strId;

    uv_write_t              m_streamWriter;

    std::string             m_bufferSend;
    std::vector<char>       m_bufferRecv;

    std::shared_ptr<nitro::command::CommandGenerator>    m_commandGenerator;
};

}
}


void HandleNewConnection(uv_connect_t* req, int status)
{
    if (status == -1) {
        fprintf(stderr, "error on_write_end");
        return;
    }

    auto clientTask = new nitro::community::ClientTask();
    clientTask->StartListen(req->handle);
}


int ClientMainTest(const std::string& connectionUri)
{
    uv_loop_t* loop = uv_default_loop();

    SYNC_OUTPUT("SERVER") << "Start client";

    uv_connect_t    connect{};
    uv_tcp_t        socket{};

    uv_tcp_init(loop, &socket);

    struct sockaddr_in dest;
    uv_ip4_addr(connectionUri.c_str(), SERVER_PORT, &dest);

    uv_tcp_connect(&connect, &socket, (const struct sockaddr*)&dest, HandleNewConnection);


    uv_run(loop, UV_RUN_DEFAULT);

    system("pause");

    return 0;

}

