#ifndef SCHEDULER_CONFIG_H
#define SCHEDULER_CONFIG_H

// SCHED_FIFO is defined in sched.h
enum sched_algorithm {
    ALGO_FIFO,
    ALGO_PRIORITY,
};

struct config {
    int port;
    enum sched_algorithm algorithm;
    int discovery_port;
};

bool config_parse(int argc, char *argv[], struct config *cfg);
void config_usage(const char *prog);
const char *sched_algorithm_name(enum sched_algorithm algo);

#endif
