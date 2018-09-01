#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <uv.h>

uv_loop_t *loop;
uv_pipe_t queue;

typedef struct {
    uv_write_t req;
    uv_buf_t buf;
} write_req_t;

void free_write_req(uv_write_t *req) {
    write_req_t *wr = (write_req_t*) req;
    free(wr->buf.base);
    free(wr);
}

void alloc_buffer(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf) {
    *buf = uv_buf_init((char*)malloc(suggested_size), suggested_size);
}

void echo_write(uv_write_t *req, int status) {
    if (status < 0) {
        fprintf(stderr, "Write error %s\n", uv_err_name(status));
    }
    free_write_req(req);
}

void echo_read(uv_stream_t *client, ssize_t nread, const uv_buf_t *buf) {
    if (nread > 0) {
        write_req_t *req = (write_req_t*) malloc(sizeof(write_req_t));
        req->buf = uv_buf_init(buf->base, nread);
        uv_write((uv_write_t*)req, client, &req->buf, 1, echo_write);
        return;
    }

    if (nread < 0) {
        if (nread != UV_EOF)
            fprintf(stderr, "Read error %s\n", uv_err_name(nread));
        uv_close((uv_handle_t*)client, NULL);
    }

    free(buf->base);
}

void on_new_connection(uv_stream_t *q, ssize_t nread, const uv_buf_t *buf) {
    if (nread < 0) {
        if (nread != UV_EOF) fprintf(stderr, "Read error: %s\n", uv_strerror(nread));
        uv_close((uv_handle_t *)q, NULL);
        return;
    }

    uv_pipe_t *pipe = (uv_pipe_t*)q;
    if (!uv_pipe_pending_count(pipe)) {
        fprintf(stderr, "No pending count\n");
        return;
    }

    uv_handle_type pending = uv_pipe_pending_type(pipe);
    assert(pending == UV_TCP);

    uv_tcp_t *client = malloc(sizeof(uv_tcp_t));
    uv_tcp_init(loop, client);

    if (uv_accept(q, (uv_stream_t *)client) == 0) {
        uv_os_fd_t fd;
        uv_fileno((const uv_handle_t *)client, &fd);
        fprintf(stderr, "Worker %d: Accepted fd %d\n", getpid(), fd);
        uv_read_start((uv_stream_t*)client, alloc_buffer, echo_read);
    } else {
        uv_close((uv_handle_t *)client, NULL);
    }
}

int main(int argc, const char *argv[])
{
    loop = uv_default_loop();
    uv_pipe_init(loop, &queue, 1);
    uv_pipe_open(&queue, 0);
    uv_read_start((uv_stream_t *)&queue, alloc_buffer, on_new_connection);

    return uv_run(loop, UV_RUN_DEFAULT);
}