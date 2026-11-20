#ifndef SCHEDULER_SERVER_H
#define SCHEDULER_SERVER_H

/* Open the TCP socket workers and clients connect to: bound to port on all
 * interfaces with SO_REUSEADDR, already listening. Returns the fd or -1. */
int server_listen(int port);

#endif
