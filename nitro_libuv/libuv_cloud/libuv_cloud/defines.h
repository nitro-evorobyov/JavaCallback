
#pragma once

#include <iostream>
#include <uv.h>

#include "../../../nitro_cloud/include/sync_out_puter.h"

#define ERROR(msg, code) do {                                                         \
  fprintf(stderr, "%s: [%s: %s]\n", msg, uv_err_name((code)), uv_strerror((code)));   \
  assert(0);                                                                          \
} while(0);

#ifdef _WIN32
#define PIPE_NAME "\\\\.\\pipe\\mynamedpipe"
#else
#define PIPE_NAME "echo.sock"
#endif

#define NOIPC 0


namespace nitro_common
{
void alloc_cb(uv_handle_t *handle, size_t size, uv_buf_t *buf) {
    buf->base = (char*)malloc(size);
    buf->len = size;
}
}