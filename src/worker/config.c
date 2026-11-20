#include "worker.h"

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

        if (strcmp(arg, "-cores") == 0) {
            ok = parse_int(val, 1, INT_MAX, &cfg->cores);
        } else if (strcmp(arg, "-ram") == 0) {
            ok = parse_int(val, 1, INT_MAX, &cfg->ram_mb);
        } else if (strcmp(arg, "-gpu") == 0) {
            ok = parse_int(val, 0, INT_MAX, &cfg->gpus);
        } else if (strcmp(arg, "-discovery-port") == 0) {
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

    if (cfg->cores == 0 || cfg->ram_mb == 0) {
        fprintf(stderr, "Error: -cores and -ram are required\n");
        return false;
    }

    if (npos == 1) {
        fprintf(stderr, "Error: give both <scheduler_ip> and <scheduler_port>, or neither\n");
        return false;
    }
    if (npos == 2 && !parse_sched_addr(pos[0], pos[1], &cfg->fallback))
        return false;

    return true;
}

void config_clamp(struct config *cfg)
{
    long cores = hostinfo_online_cores();
    long ram_mb = hostinfo_total_ram_mb();

    if (cores > 0 && cfg->cores > cores) {
        printf("[WORKER] WARNING: -cores %d is more than this machine has; clamping to %ld.\n", cfg->cores, cores);
        cfg->cores = (int)cores;
    }

    if (ram_mb > 0 && cfg->ram_mb > ram_mb) {
        printf("[WORKER] WARNING: -ram %d is more than this machine has; clamping to %ld MB.\n", cfg->ram_mb, ram_mb);
        cfg->ram_mb = (int)ram_mb;
    }
}

void config_usage(const char *prog)
{
    fprintf(stderr, "Usage: %s [<scheduler_ip> <scheduler_port>] -cores <num> -ram <MB> [-gpu <num>] [-discovery-port <port>]\n", prog);
}
