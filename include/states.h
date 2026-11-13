#ifndef STATES_H
#define STATES_H

enum job_state {
    JOB_WAITING,
    JOB_QUEUED,
    JOB_RUNNING,
    JOB_COMPLETED,
    JOB_FAILED,
    JOB_CANCELLED,
    JOB_LOST,
    JOB_FAILED_PERMANENT,
};

enum worker_state {
    WORKER_ALIVE,
    WORKER_SUSPECTED,
    WORKER_DEAD,
    WORKER_DRAINING,
};

#endif
