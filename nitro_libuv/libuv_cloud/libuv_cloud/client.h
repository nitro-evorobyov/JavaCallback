
#pragma once

#include <thread>
#include <uv.h>

#include "defines.h"

namespace nitro_client
{

static void write_cb(uv_write_t* req, int status);

static void close_cb(uv_handle_t* handle) {
    free(handle);
}

static void read_cb(uv_stream_t* client, ssize_t nread, const uv_buf_t* buf) {
    if (nread < 0) {
        fprintf(stderr, "read error: [%s: %s]\n", uv_err_name((nread)), uv_strerror((nread)));
        uv_close((uv_handle_t*)client, NULL);
        return;
    }

    SYNC_OUTPUT("Client") << "Successfuly get some bytes." << (char*)buf->base;

    uv_write_t *out_req = (uv_write_t *)malloc(sizeof(uv_write_t));
    uv_buf_t wrbuf = uv_buf_init("123456789\n", 10);


    LPCTSTR endKeyBuffer = "Send buy";

    if (strncmp(endKeyBuffer, (char*)buf->base, strlen(endKeyBuffer)) == 0)
    {
        uv_close((uv_handle_t*)client, NULL);
        SYNC_OUTPUT("Client") << "Goodbuy." << (char*)buf->base;
        return;
    }

    SYNC_OUTPUT("Client") << "Before send.";
    uv_write(out_req, (uv_stream_t*)client, &wrbuf, 1, write_cb);

}


static void write_cb(uv_write_t* req, int status) {
    if (status != 0) {
    }
    else {
        SYNC_OUTPUT("Client") << "Successfuly send message. Wait some time and response.";

        std::this_thread::sleep_for(std::chrono::milliseconds(100 + rand() % 500));

        uv_read_start((uv_stream_t*)req->handle, nitro_common::alloc_cb, read_cb);
    }
    free(req);
}


static void connect_cb(uv_connect_t* in_req, int status){
    if (status != 0) {
    }
    else {
        // send a message to the pipe
        SYNC_OUTPUT("Client") << "Server connected.";

        uv_write_t *out_req = (uv_write_t *)malloc(sizeof(uv_write_t));
        uv_buf_t wrbuf = uv_buf_init("Hello! How are you?\n", 10);

        SYNC_OUTPUT("Client") << "Try to send first message to the server.";
        uv_write(out_req, (uv_stream_t*)in_req->handle, &wrbuf, 1, write_cb);
    }
    free(in_req);
}

void RunClient()
{
    SYNC_OUTPUT("Client") << "Start client";

    uv_loop_t *loop = (uv_loop_t *)malloc(sizeof(uv_loop_t));
    uv_loop_init(loop);

    uv_pipe_t* handle = (uv_pipe_t*)malloc(sizeof(uv_pipe_t));
    uv_connect_t* connect = (uv_connect_t*)malloc(sizeof(uv_connect_t));

    uv_pipe_init(loop, handle, NOIPC);
    //uv_pipe_open(handle, socket(PF_UNIX, SOCK_STREAM, 0));

    SYNC_OUTPUT("Client") << "Try connect to server";

    uv_pipe_connect(connect, handle, PIPE_NAME, connect_cb);
    uv_run(loop, UV_RUN_DEFAULT);

    uv_loop_close(loop);
    free(loop);

    SYNC_OUTPUT("Client") << "Finish client";
}

}