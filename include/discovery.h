#ifndef DISCOVERY_H
#define DISCOVERY_H

/* UDP scheduler discovery (Sec 2C).
 * Request:  the text DISCOVERY_REQUEST ("OSN_DISCOVER"), broadcast to the discovery port.
 * Reply:    DISCOVERY_REPLY_PREFIX followed by the scheduler's TCP port,
 *           e.g. "OSN_SCHEDULER 8080". The asker takes the IP from the reply's sender. */

/* Scheduler side: open a UDP socket bound to udp_port on all interfaces, with
 * SO_REUSEADDR/SO_REUSEPORT so a quick restart doesn't fail. Returns the fd or -1. */
int discovery_responder_open(int udp_port);

/* Scheduler side: read one datagram from fd and, if it is a discovery request,
 * reply with tcp_port. Logs every request it answers. Call when fd is readable. */
void discovery_responder_handle(int fd, int tcp_port);

#endif
