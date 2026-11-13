#include "client.h"

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

        if (strcmp(arg, "-user") == 0) {
            ok = username_valid(val);
            if (ok) {
                strcpy(cfg->username, val);
                cfg->has_username = true;
            }
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

    if (npos == 1) {
        fprintf(stderr, "Error: give both <scheduler_ip> and <scheduler_port>, or neither\n");
        return false;
    }
    if (npos == 2 && !parse_sched_addr(pos[0], pos[1], &cfg->fallback))
        return false;

    return true;
}

void config_usage(const char *prog)
{
    fprintf(stderr, "Usage: %s [<scheduler_ip> <scheduler_port>] [-user <username>] [-discovery-port <port>]\n", prog);
}
