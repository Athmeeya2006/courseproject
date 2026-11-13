#ifndef CLIENT_CONFIG_H
#define CLIENT_CONFIG_H

struct config {
    struct sched_addr fallback;
    bool has_username;
    char username[USERNAME_MAX_LEN + 1];
    int discovery_port;
};

bool config_parse(int argc, char *argv[], struct config *cfg);
void config_usage(const char *prog);

#endif
