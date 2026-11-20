#include "scheduler.h"

int main(int argc, char *argv[])
{
    struct config cfg;

    if (!config_parse(argc, argv, &cfg)) {
        config_usage(argv[0]);
        return 1;
    }

    /* A peer closing its socket must give us EPIPE, not kill the scheduler. */
    signal(SIGPIPE, SIG_IGN);
    log_set_tag("SCHEDULER");

    int listen_fd = server_listen(cfg.port);
    if (listen_fd < 0) {
        log_line("Error: cannot listen on TCP port %d: %s", cfg.port, strerror(errno));
        return 1;
    }
    log_line("Started on port %d. Algorithm: %s", cfg.port, sched_algorithm_name(cfg.algorithm));

    int disc_fd = discovery_responder_open(cfg.discovery_port);
    if (disc_fd < 0) {
        log_line("Error: cannot open discovery responder on UDP port %d: %s",
                 cfg.discovery_port, strerror(errno));
        close(listen_fd);
        return 1;
    }
    log_line("Discovery responder listening on UDP port %d.", cfg.discovery_port);
    log_line("Listening for incoming connections...");

    // todo (Part 2B): accept workers/clients on listen_fd
    for (;;) {
        struct pollfd pfd = { .fd = disc_fd, .events = POLLIN, .revents = 0 };

        if (poll(&pfd, 1, -1) < 0) {
            if (errno == EINTR)
                continue;
            log_line("Error: poll failed: %s", strerror(errno));
            break;
        }
        if (pfd.revents & POLLIN)
            discovery_responder_handle(disc_fd, cfg.port);
    }

    close(disc_fd);
    close(listen_fd);
    return 1;
}
