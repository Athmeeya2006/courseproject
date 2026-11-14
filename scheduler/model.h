#ifndef SCHEDULER_MODEL_H
#define SCHEDULER_MODEL_H

/*
 * Scheduler-side job, worker and cron-template records. These carry fields
 * for every section of the spec up front, so later parts don't have to
 * reshape them. Fields marked (Sec N) are unused until that section exists.
 */

#include <stdbool.h>
#include <stdint.h>
#include <time.h>

#include "proto.h"
#include "states.h"

#define USERNAME_BUF   32   /* USERNAME_MAX_LEN + 1 */
#define MAX_WORKER_HASHES 256

struct checkpoint {
    bool present;
    uint32_t seq;            /* checkpoint number shown in status/migrate */
    uint8_t *data;           /* malloc'd, <= CHECKPOINT_MAX_BYTES */
    size_t len;
    time_t received_at;      /* for "age" in status */
};

struct log_ring {            /* last LOG_BUFFER_BYTES of output (Sec 6) */
    uint8_t data[LOG_BUFFER_BYTES];
    size_t start;            /* index of oldest byte */
    size_t len;              /* bytes stored (<= LOG_BUFFER_BYTES) */
};

struct job {
    uint32_t id;
    char owner[USERNAME_BUF];
    struct job_spec spec;            /* everything the client asked for */
    enum job_state state;

    /* ordering (Sec 4B) */
    uint64_t submit_seq;             /* monotonically increasing; FIFO order */
    time_t submitted_at;
    time_t queued_at;                /* reset on every requeue; drives aging */
    uint32_t effective_prio;

    /* placement */
    int32_t worker_id;               /* -1 = none */
    uint32_t attempts;               /* counts worker-death retries only (Sec 4C) */

    /* dependencies and groups (Sec 8) */
    uint32_t n_deps;                 /* groups already resolved to job ids at submit */
    uint32_t deps[MAX_DEPS];

    /* preemption (Sec 10) */
    bool preempt_hold;               /* skip until preemptor starts */
    uint32_t held_for_job;           /* the prio-1 job that took our resources */

    /* cron (Sec 12) */
    uint32_t template_id;            /* 0 = not spawned by a cron */

    /* caching (Sec 13) */
    bool has_binary;                 /* false = PATH command, run by name */
    char hash[HASH_HEX_LEN + 1];
    char binary_name[256];

    /* migration (Sec 14) */
    int32_t migrated_from;           /* exclude this worker if another qualifies; -1 = none */
    struct checkpoint checkpoint;

    /* telemetry and results (Sec 5D/5E) */
    struct job_telemetry telemetry;
    int32_t exit_code;
    int32_t term_signal;
    char diagnosis[256];

    /* output (Sec 6) */
    struct log_ring output;
};

struct worker {
    uint32_t id;
    int fd;                          /* TCP socket; -1 once gone */
    enum worker_state state;
    char addr[64];

    uint32_t total_cores, free_cores;
    uint32_t total_ram_mb, free_ram_mb;
    uint32_t total_gpus, free_gpus;

    /* heartbeats (Sec 4C/4D) */
    time_t last_heartbeat;
    uint32_t missed_heartbeats;
    uint32_t cpu_load_pct;
    uint32_t running_jobs;

    /* executable cache (Sec 13) */
    uint32_t n_hashes;
    char hashes[MAX_WORKER_HASHES][HASH_HEX_LEN + 1];
};

struct cron_template {               /* Sec 12 */
    uint32_t id;                     /* own numbering, starts at 1 */
    char owner[USERNAME_BUF];
    struct job_spec spec;            /* copied into each spawned job */
    uint32_t every_s;
    uint32_t runs;
    time_t next_due;
    bool cancelled;
};

#endif
