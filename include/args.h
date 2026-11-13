#ifndef ARGS_H
#define ARGS_H

bool parse_int(const char *s, int min, int max, int *out);
bool parse_port(const char *s, int *out);
bool parse_sched_addr(const char *host, const char *port, struct sched_addr *out);
bool username_valid(const char *s);

#endif
