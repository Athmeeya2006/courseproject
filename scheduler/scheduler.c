#include "scheduler.h"

int main(int argc, char *argv[])
{
    struct config cfg;

    if (!config_parse(argc, argv, &cfg)) {
        config_usage(argv[0]);
        return 1;
    }

    // todo: replace with actual code
    printf("[SCHEDULER] port=%d algorithm=%s discovery-port=%d\n", cfg.port, sched_algorithm_name(cfg.algorithm), cfg.discovery_port);
    return 0;
}
