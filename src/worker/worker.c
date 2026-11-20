#include "worker.h"

int main(int argc, char *argv[])
{
    struct config cfg;

    if (!config_parse(argc, argv, &cfg)) {
        config_usage(argv[0]);
        return 1;
    }

    config_clamp(&cfg);

    // todo: replace with actual code
    if (cfg.fallback.set)
        printf("[WORKER] fallback=%s:%d", cfg.fallback.host, cfg.fallback.port);
    else
        printf("[WORKER] fallback=none");
    printf(" cores=%d ram=%dMB gpu=%d discovery-port=%d\n", cfg.cores, cfg.ram_mb, cfg.gpus, cfg.discovery_port);
    return 0;
}
