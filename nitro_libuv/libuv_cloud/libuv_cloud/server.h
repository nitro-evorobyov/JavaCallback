
#pragma once 


#include "defines.h"
#include "server_task.h"

namespace nitro_server
{
uv_loop_t *loop;
uv_pipe_t server;

void sigint_cb(int sig) {
    int r;
    uv_fs_t req;
    r = uv_fs_unlink(loop, &req, PIPE_NAME, NULL);
    if (r) ERROR("unlinking echo.sock", r);
    exit(0);
}


void connect_cb(uv_stream_t *server, int status)
{
    if (status) {
        fprintf(stderr, "connection error: [%s: %s]\n", uv_err_name((status)), uv_strerror((status)));
        return;
    }

    SYNC_OUTPUT("SERVER") << "Handle connection";
    
    auto servertask = new Nitro::Community::ServerTask<TCHAR>(loop, server);
    servertask->StartListen();
}

void RunServer()
{
    int r;
    loop = uv_default_loop();

    SYNC_OUTPUT("SERVER") << "Start server";

    signal(SIGINT, sigint_cb);

    uv_pipe_init(loop, &server, NOIPC);

    r = uv_pipe_bind(&server, PIPE_NAME);
    if (r) ERROR("binding to echo.sock", r);

    SYNC_OUTPUT("SERVER") << "Start listen"; 
    r = uv_listen((uv_stream_t*)&server, 128, connect_cb);
    if (r) ERROR("listening on socket", r);

    uv_run(loop, UV_RUN_DEFAULT);
}


}