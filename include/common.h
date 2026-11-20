#ifndef COMMON_H
#define COMMON_H

#include <errno.h>
#include <limits.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEFAULT_DISCOVERY_PORT 9999
#define USERNAME_MAX_LEN 31
#define HOST_MAX_LEN 255

struct sched_addr {
    bool set;
    char host[HOST_MAX_LEN + 1];
    int port;
};

#include "args.h"
#include "discovery.h"
#include "log.h"
#include "net.h"
#include "proto.h"
#include "states.h"

#endif
