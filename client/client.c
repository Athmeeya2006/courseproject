#include "client.h"

int main(int argc, char *argv[])
{
    struct config cfg;

    if (!config_parse(argc, argv, &cfg)) {
        config_usage(argv[0]);
        return EXIT_FAILURE;
    }

    // todo: add discovery and connect before the prompt
    if (!cfg.has_username) {
        if (!login_prompt_username(cfg.username)) {
            printf("\n");
            return EXIT_FAILURE;
        }
        cfg.has_username = true;
    }

    // todo: replace with actual code
    printf("Logged in as %s.\n", cfg.username);
    if (cfg.fallback.set)
        printf("[Client] fallback=%s:%d", cfg.fallback.host, cfg.fallback.port);
    else
        printf("[Client] fallback=none");
    printf(" discovery-port=%d\n", cfg.discovery_port);
    return EXIT_SUCCESS;
}
