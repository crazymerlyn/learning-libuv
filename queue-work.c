#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <uv.h>

uv_loop_t *loop;

long fib_(int n) {
    long a = 0, b = 1;
    while (n--) {
        b += a;
        a = b - a;
    }
    return a;
}

void fib(uv_work_t *req) {
    int n = *(int *) req->data;
    if (random() % 2) sleep(1);
    else sleep(3);

    long fib = fib_(n);
    fprintf(stderr, "%dth fibonacci is %lu\n", n, fib);
}

void after_fib(uv_work_t *req, int status) {
    fprintf(stderr, "Done calculating %dth fibonacci\n", *(int *)req->data);
}

#define FIB_UNTIL 10

int main() {
    loop = uv_default_loop();

    int data[FIB_UNTIL];
    uv_work_t req[FIB_UNTIL];
    for (int i = 0; i < FIB_UNTIL; ++i) {
        data[i] = i;
        req[i].data = &data[i];
        uv_queue_work(loop, &req[i], fib, after_fib);
    }

    return uv_run(loop, UV_RUN_DEFAULT);
}
