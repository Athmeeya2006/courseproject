#include <stdarg.h>
#include <time.h>

#include "common.h"

static char log_tag[64] = "LOG";

void log_set_tag(const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    vsnprintf(log_tag, sizeof(log_tag), fmt, ap);
    va_end(ap);
}

void log_line(const char *fmt, ...)
{
    time_t now = time(NULL);
    struct tm tm;
    char stamp[16] = "??:??:??";
    va_list ap;

    if (localtime_r(&now, &tm) != NULL)
        strftime(stamp, sizeof(stamp), "%H:%M:%S", &tm);

    flockfile(stdout);
    printf("[%s] [%s] ", stamp, log_tag);
    va_start(ap, fmt);
    vprintf(fmt, ap);
    va_end(ap);
    putchar('\n');
    fflush(stdout);
    funlockfile(stdout);
}
