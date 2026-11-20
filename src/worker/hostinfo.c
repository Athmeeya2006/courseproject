#include "worker.h"

long hostinfo_online_cores(void)
{
    return sysconf(_SC_NPROCESSORS_ONLN);
}

long hostinfo_total_ram_mb(void)
{
    long pages = sysconf(_SC_PHYS_PAGES);
    long page_size = sysconf(_SC_PAGESIZE);

    if (pages <= 0 || page_size <= 0)
        return -1;

    return pages * page_size / (1024 * 1024);
}
