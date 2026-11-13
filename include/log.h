#ifndef LOG_H
#define LOG_H

void log_set_tag(const char *fmt, ...) __attribute__((format(printf, 1, 2)));
void log_line(const char *fmt, ...) __attribute__((format(printf, 1, 2)));

#endif
