#include "common.h"

bool parse_int(const char *s, int min, int max, int *out)
{
    char *end;
    long value;

    errno = 0;
    value = strtol(s, &end, 10);
    if (end == s || *end != '\0' || errno != 0)
        return false;
    if (value < min || value > max)
        return false;

    *out = (int)value;
    return true;
}

bool parse_port(const char *s, int *out)
{
    return parse_int(s, 1, 65535, out);
}

bool parse_sched_addr(const char *host, const char *port, struct sched_addr *out)
{
    if (strlen(host) > HOST_MAX_LEN) {
        fprintf(stderr, "Error: scheduler address '%s' is too long\n", host);
        return false;
    }
    if (!parse_port(port, &out->port)) {
        fprintf(stderr, "Error: invalid scheduler port '%s'\n", port);
        return false;
    }

    strcpy(out->host, host);
    out->set = true;
    return true;
}

bool username_valid(const char *s)
{
    size_t len = strlen(s);

    if (len < 1 || len > USERNAME_MAX_LEN)
        return false;

    for (size_t i = 0; i < len; i++) {
        char c = s[i];
        bool ok = (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_' || c == '-' || c == '.';
        if (!ok)
            return false;
    }
    return true;
}
