#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include "common.h"

int discovery_responder_open(int udp_port)
{
    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0)
        return -1;

    int one = 1;
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one)) < 0)
        goto fail;
#ifdef SO_REUSEPORT
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEPORT, &one, sizeof(one)) < 0)
        goto fail;
#endif

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons((uint16_t)udp_port);

    if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
        goto fail;
    return fd;

fail:;
    int saved = errno;
    close(fd);
    errno = saved;
    return -1;
}

void discovery_responder_handle(int fd, int tcp_port)
{
    char msg[64];
    struct sockaddr_in from;
    socklen_t from_len = sizeof(from);

    ssize_t n = recvfrom(fd, msg, sizeof(msg) - 1, 0, (struct sockaddr *)&from, &from_len);
    if (n < 0)
        return;
    msg[n] = '\0';

    /* Ignore anything that isn't exactly our request (other teams, stray traffic). */
    if (strcmp(msg, DISCOVERY_REQUEST) != 0)
        return;

    char reply[64];
    int len = snprintf(reply, sizeof(reply), "%s%d", DISCOVERY_REPLY_PREFIX, tcp_port);
    if (len < 0 || (size_t)len >= sizeof(reply))
        return;

    char ip[INET_ADDRSTRLEN] = "?";
    inet_ntop(AF_INET, &from.sin_addr, ip, sizeof(ip));

    if (sendto(fd, reply, (size_t)len, 0, (struct sockaddr *)&from, from_len) < 0) {
        log_line("Discovery request from %s:%u, reply failed: %s",
                 ip, (unsigned)ntohs(from.sin_port), strerror(errno));
        return;
    }
    log_line("Discovery request from %s:%u, replied with TCP port %d",
             ip, (unsigned)ntohs(from.sin_port), tcp_port);
}
