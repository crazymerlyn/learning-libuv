#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <uv.h>

#define FIB_UNTIL 10

uv_loop_t *loop;
uv_work_t fib_reqs[FIB_UNTIL];
int data[FIB_UNTIL];

void signal_handler(uv_signal_t *req, int signum) {
    printf("Signal received!\n");
    for (int i = 0; i < FIB_UNTIL; ++i) {
        uv_cancel((uv_req_t *)&fib_reqs[i]);
    }
    uv_signal_stop(req);
}

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
    if (status == UV_ECANCELED) {
        fprintf(stderr, "Calculation of %dth fibonacci cancelled\n", *(int *)req->data);
        return;
    }
    fprintf(stderr, "Done calculating %dth fibonacci\n", *(int *)req->data);
}

int main() {
    loop = uv_default_loop();

    for (int i = 0; i < FIB_UNTIL; ++i) {
        data[i] = i;
        fib_reqs[i].data = &data[i];
        uv_queue_work(loop, &fib_reqs[i], fib, after_fib);
    }

    uv_signal_t sig;
    uv_signal_init(loop, &sig);
    uv_signal_start(&sig, signal_handler, SIGINT);

    return uv_run(loop, UV_RUN_DEFAULT);
}
