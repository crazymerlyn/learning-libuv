#include "plugin.h"
#include <uv.h>
#include <stdio.h>
#include <stdlib.h>

typedef void (*init_plugin_function)();

void mfp_register(const char *name) {
    fprintf(stderr, "Registered plugin \"%s\"\n", name);
}

int main(int argc, const char *argv[])
{
    if (argc == 1) {
        fprintf(stderr, "Usage: %s [plugin1] [plugin2] ...\n", argv[0]);
        return 0;
    }

    uv_lib_t *lib = malloc(sizeof(uv_lib_t));
    for(int i = 1; i < argc; ++i) {
        fprintf(stderr, "Loading %s\n", argv[i]);
        if (uv_dlopen(argv[i], lib)) {
            fprintf(stderr, "Error: %s\n", uv_dlerror(lib));
            continue;
        }

        init_plugin_function init_plugin;
        if (uv_dlsym(lib, "initialize", (void **) &init_plugin)) {
            fprintf(stderr, "dlsym error: %s\n", uv_dlerror(lib));
            continue;
        }

        init_plugin();
    }
    return 0;
}
