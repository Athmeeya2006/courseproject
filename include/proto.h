#ifndef PROTO_H
#define PROTO_H

/*
 * Wire protocol shared by scheduler, worker and client.
 *
 * Every TCP message is a frame:
 *
 *     +----------------+----------------+-----------------+
 *     | type (u32, BE) | len  (u32, BE) | payload (len B) |
 *     +----------------+----------------+-----------------+
 *
 * Payloads are built and parsed with the buf_* helpers in net.h, which write
 * integers in network byte order and strings as (u32 length, bytes). Never
 * send raw structs: padding and endianness differ between machines.
 *
 * Each message type below documents its payload, in order.
 */

#include <stdint.h>

/* ---- Limits and tunables (constants, not CLI flags) ---- */

#define JOB_QUEUE_CAPACITY      1024
#define MAX_ATTEMPTS            3        /* total attempts before FAILED_PERMANENT */
#define HEARTBEAT_INTERVAL_MS   2000
#define HEARTBEAT_DEAD_MISSES   3        /* 1 miss = SUSPECTED, 3 = DEAD */
#define SCHEDULE_TICK_MS        1000     /* pass at least once per second */
#define AGING_INTERVAL_S        10       /* +1 effective priority per 10 s queued */
#define PRIO_HIGHEST            1
#define PRIO_LOWEST             3
#define OUTPUT_POLL_MS          500      /* worker reads output.log this often */
#define CHECKPOINT_POLL_MS      500
#define LOG_BUFFER_BYTES        (64 * 1024) /* scheduler keeps last 64 KB per job */
#define MIGRATE_KILL_DELAY_S    2        /* SIGTERM, then SIGKILL after this */
#define QUOTA_PERCENT           50       /* > 50% of cluster cores = over quota */
#define CACHE_DEFAULT_MB        512
#define JOB_NPROC_LIMIT         64       /* RLIMIT_NPROC (shared by all jobs as nobody) */
#define CHECKPOINT_MAX_BYTES    (64 * 1024)

#define DISCOVERY_TIMEOUT_MS    2000
#define DISCOVERY_RETRIES       3
#define DISCOVERY_REQUEST       "OSN_DISCOVER"
#define DISCOVERY_REPLY_PREFIX  "OSN_SCHEDULER " /* followed by the TCP port */

#define MAX_DEPS                64
#define MAX_PATH_LEN            4096
#define MAX_ARGS_LEN            4096
#define HASH_HEX_LEN            40       /* SHA-1 as hex */
#define MSG_MAX_PAYLOAD         (64u * 1024u * 1024u) /* fits binary transfers */

/* ---- Message types ---- */

enum msg_type {
    MSG_INVALID = 0,

    /* worker -> scheduler */
    MSG_REGISTER = 1,        /* u32 cores, u32 ram_mb, u32 gpus, u32 n_hashes, n x str hash */
    MSG_HEARTBEAT,           /* u32 cpu_load_pct, u32 n_jobs, n x telemetry (see below) */
    MSG_JOB_OUTPUT,          /* u32 job_id, bytes chunk (str) */
    MSG_JOB_FINISHED,        /* u32 job_id, u32 outcome (enum job_outcome), i32 exit_code,
                                i32 signal, str diagnosis, u64 peak_ram_kb */
    MSG_DRAIN,               /* (empty) worker started draining */
    MSG_CHECKPOINT,          /* u32 job_id, u32 seq, bytes state (str) */
    MSG_CACHE_LIST,          /* u32 n_hashes, n x str hash (after eviction / restart) */
    MSG_CACHE_MISS,          /* u32 job_id, str hash: "I don't have it, send the file" */

    /* scheduler -> worker */
    MSG_REGISTER_ACK = 32,   /* u32 worker_id */
    MSG_JOB_ASSIGN,          /* u32 job_id, str path, str args, u32 cores, u32 ram_mb,
                                u32 time_s, u32 gpus, str hash ("" = PATH command) */
    MSG_CHECKPOINT_DATA,     /* u32 job_id, u32 seq, bytes state; sent BEFORE the binary */
    MSG_BINARY_DATA,         /* u32 job_id, str hash, str name, bytes file */
    MSG_BINARY_CACHED,       /* u32 job_id, str hash: "you already have this one" */
    MSG_JOB_KILL,            /* u32 job_id, u32 reason (enum kill_reason) */

    /* client -> scheduler */
    MSG_HELLO = 64,          /* str username (always the first client message) */
    MSG_SUBMIT,              /* job_spec, see buf layout in net.c / scheduler */
    MSG_NODES,               /* (empty) */
    MSG_STATUS,              /* u32 job_id */
    MSG_LOGS,                /* u32 job_id, u32 follow (0/1) */
    MSG_LOGS_UNFOLLOW,       /* u32 job_id (Ctrl+C in follow mode) */
    MSG_CANCEL,              /* u32 job_id */
    MSG_MIGRATE,             /* u32 job_id */
    MSG_CRONS,               /* (empty) */
    MSG_CANCEL_CRON,         /* u32 template_id */

    /* scheduler -> client */
    MSG_HELLO_ACK = 96,      /* (empty) */
    MSG_SUBMIT_REPLY,        /* u32 status (enum submit_status), u32 id, u32 waiting(0/1),
                                u32 is_template(0/1), str message */
    MSG_REPLY_TEXT,          /* str text: pre-formatted reply (nodes, status, crons, errors) */
    MSG_EVENT,               /* str text, printed as "[Event] <text>" */
    MSG_LOG_DATA,            /* u32 job_id, bytes chunk */
    MSG_LOG_END,             /* u32 job_id, u32 finished(0/1): end of logs / follow */
};

enum submit_status {
    SUBMIT_OK = 0,
    SUBMIT_QUEUE_FULL,
    SUBMIT_BAD_ARGS,
    SUBMIT_BAD_DEPENDENCY,   /* unknown job or group id */
    SUBMIT_EVERY_WITH_DEPS,  /* -every cannot be combined with -depends-on */
};

enum job_outcome {
    OUTCOME_COMPLETED = 0,
    OUTCOME_FAILED,
    OUTCOME_KILLED,          /* worker killed it on purpose; see kill_reason */
};

enum kill_reason {
    KILL_NONE = 0,
    KILL_TIMEOUT,            /* wall-clock (or SIGXCPU) */
    KILL_MEMORY,             /* macOS RSS watchdog */
    KILL_CANCEL,
    KILL_PREEMPT,
    KILL_MIGRATE,
    KILL_DISCONNECT,         /* worker lost the scheduler */
};

/* Per-job telemetry inside MSG_HEARTBEAT:
 * u32 job_id, u64 rss_kb, u64 peak_rss_kb, u64 page_faults, u64 cpu_time_ms */
struct job_telemetry {
    uint32_t job_id;
    uint64_t rss_kb;
    uint64_t peak_rss_kb;
    uint64_t page_faults;
    uint64_t cpu_time_ms;
};

/* What a client submits (MSG_SUBMIT payload, in this order):
 * str path, str args, u32 cores, u32 ram_mb, u32 time_s, u32 prio, u32 gpus,
 * u32 group (0 = none), u32 preemptible, u32 every_s (0 = not a cron),
 * u32 n_deps, n x u32 dep_job_id, u32 n_dep_groups, n x u32 dep_group_id */
struct job_spec {
    char path[MAX_PATH_LEN];
    char args[MAX_ARGS_LEN];
    uint32_t cores;
    uint32_t ram_mb;
    uint32_t time_s;
    uint32_t prio;
    uint32_t gpus;
    uint32_t group;
    uint32_t preemptible;
    uint32_t every_s;
    uint32_t n_deps;
    uint32_t deps[MAX_DEPS];
    uint32_t n_dep_groups;
    uint32_t dep_groups[MAX_DEPS];
};

#endif
