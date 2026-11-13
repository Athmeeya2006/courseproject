#include "scheduler.h"

const char *sched_algorithm_name(enum sched_algorithm algo)
{
    switch (algo) {
        case ALGO_FIFO:
            return "FIFO";
        case ALGO_PRIORITY:
            return "PRIORITY";
    }
    return "UNKNOWN";
}

bool config_parse(int argc, char *argv[], struct config *cfg)
{
    const char *pos[2];
    int npos = 0;

    memset(cfg, 0, sizeof(*cfg));
    cfg->discovery_port = DEFAULT_DISCOVERY_PORT;

    for (int i = 1; i < argc; i++) {
        const char *arg = argv[i];
        bool ok = false;

        if (arg[0] != '-') {
            if (npos == 2) {
                fprintf(stderr, "Error: unexpected argument '%s'\n", arg);
                return false;
            }
            pos[npos] = arg;
            npos++;
            continue;
        }

        if (i + 1 >= argc) {
            fprintf(stderr, "Error: %s needs a value\n", arg);
            return false;
        }
        i++;
        const char *val = argv[i];

        if (strcmp(arg, "-discovery-port") == 0) {
            ok = parse_port(val, &cfg->discovery_port);
        } else {
            fprintf(stderr, "Error: unknown option '%s'\n", arg);
            return false;
        }

        if (!ok) {
            fprintf(stderr, "Error: invalid value '%s' for %s\n", val, arg);
            return false;
        }
    }

    if (npos < 2) {
        fprintf(stderr, "Error: missing <port> or <scheduling_algorithm>\n");
        return false;
    }

    if (!parse_port(pos[0], &cfg->port)) {
        fprintf(stderr, "Error: invalid port '%s'\n", pos[0]);
        return false;
    }

    if (strcmp(pos[1], "FIFO") == 0) {
        cfg->algorithm = ALGO_FIFO;
    } else if (strcmp(pos[1], "PRIORITY") == 0) {
        cfg->algorithm = ALGO_PRIORITY;
    } else {
        fprintf(stderr, "Error: unknown scheduling algorithm '%s' (use FIFO or PRIORITY)\n", pos[1]);
        return false;
    }

    return true;
}

void config_usage(const char *prog)
{
    fprintf(stderr, "Usage: %s <port> <FIFO|PRIORITY> [-discovery-port <port>]\n", prog);
}
