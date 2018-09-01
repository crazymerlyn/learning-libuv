#include <stdio.h>
#include <uv.h>
#include <time.h>
#include <stdlib.h>

int64_t counter = 0;

void callback(uv_timer_t *handle) {
    printf("Time is %ld\n", time(0));
}

int main(int argc, const char *argv[])
{
    uv_timer_t req;
    uv_timer_init(uv_default_loop(), &req);
    uv_timer_start(&req, callback, 1000, 1000);

    uv_run(uv_default_loop(), UV_RUN_DEFAULT);
    return 0;
}

