#ifndef WORKER_CONFIG_H
#define WORKER_CONFIG_H

struct config {
    struct sched_addr fallback;
    int cores;
    int ram_mb;
    int gpus;
    int discovery_port;
};

bool config_parse(int argc, char *argv[], struct config *cfg);
void config_clamp(struct config *cfg);
void config_usage(const char *prog);

#endif
