#include <arpa/inet.h>
#include <unistd.h>

#include "common.h"

int write_all(int fd, const void *buf, size_t n)
{
    const uint8_t *p = buf;

    while (n > 0) {
        ssize_t w = write(fd, p, n);
        if (w < 0) {
            if (errno == EINTR)
                continue;
            return -1;
        }
        p += w;
        n -= (size_t)w;
    }
    return 0;
}

int read_all(int fd, void *buf, size_t n)
{
    uint8_t *p = buf;

    while (n > 0) {
        ssize_t r = read(fd, p, n);
        if (r < 0) {
            if (errno == EINTR)
                continue;
            return -1;
        }
        if (r == 0)
            return -1; /* peer closed */
        p += r;
        n -= (size_t)r;
    }
    return 0;
}

int proto_send(int fd, uint32_t type, const void *payload, size_t len)
{
    uint32_t hdr[2];

    if (len > MSG_MAX_PAYLOAD)
        return -1;

    hdr[0] = htonl(type);
    hdr[1] = htonl((uint32_t)len);
    if (write_all(fd, hdr, sizeof(hdr)) < 0)
        return -1;
    if (len > 0 && write_all(fd, payload, len) < 0)
        return -1;
    return 0;
}

int proto_recv(int fd, uint32_t *type, uint8_t **payload, size_t *len)
{
    uint32_t hdr[2];
    uint32_t n;
    uint8_t *data = NULL;

    if (read_all(fd, hdr, sizeof(hdr)) < 0)
        return -1;

    n = ntohl(hdr[1]);
    if (n > MSG_MAX_PAYLOAD)
        return -1;

    if (n > 0) {
        data = malloc(n);
        if (data == NULL)
            return -1;
        if (read_all(fd, data, n) < 0) {
            free(data);
            return -1;
        }
    }

    *type = ntohl(hdr[0]);
    *payload = data;
    *len = n;
    return 0;
}

/* ---- buf ---- */

void buf_init(struct buf *b)
{
    b->data = NULL;
    b->len = 0;
    b->cap = 0;
    b->failed = false;
}

void buf_free(struct buf *b)
{
    free(b->data);
    buf_init(b);
}

static void buf_append(struct buf *b, const void *p, size_t n)
{
    if (b->failed || n == 0)
        return;

    if (b->len + n > b->cap) {
        size_t cap = b->cap ? b->cap : 256;
        while (cap < b->len + n)
            cap *= 2;
        uint8_t *grown = realloc(b->data, cap);
        if (grown == NULL) {
            b->failed = true;
            return;
        }
        b->data = grown;
        b->cap = cap;
    }
    memcpy(b->data + b->len, p, n);
    b->len += n;
}

void buf_put_u32(struct buf *b, uint32_t v)
{
    uint32_t be = htonl(v);
    buf_append(b, &be, sizeof(be));
}

void buf_put_i32(struct buf *b, int32_t v)
{
    buf_put_u32(b, (uint32_t)v);
}

void buf_put_u64(struct buf *b, uint64_t v)
{
    buf_put_u32(b, (uint32_t)(v >> 32));
    buf_put_u32(b, (uint32_t)(v & 0xffffffffu));
}

void buf_put_bytes(struct buf *b, const void *p, size_t n)
{
    if (n > UINT32_MAX) {
        b->failed = true;
        return;
    }
    buf_put_u32(b, (uint32_t)n);
    buf_append(b, p, n);
}

void buf_put_str(struct buf *b, const char *s)
{
    buf_put_bytes(b, s, strlen(s));
}

int buf_send(int fd, uint32_t type, const struct buf *b)
{
    if (b->failed)
        return -1;
    return proto_send(fd, type, b->data, b->len);
}

/* ---- reader ---- */

void reader_init(struct reader *r, const uint8_t *data, size_t len)
{
    r->data = data;
    r->len = len;
    r->pos = 0;
    r->failed = false;
}

static bool reader_take(struct reader *r, void *out, size_t n)
{
    if (r->failed || r->len - r->pos < n) {
        r->failed = true;
        return false;
    }
    memcpy(out, r->data + r->pos, n);
    r->pos += n;
    return true;
}

uint32_t reader_u32(struct reader *r)
{
    uint32_t be = 0;

    if (!reader_take(r, &be, sizeof(be)))
        return 0;
    return ntohl(be);
}

int32_t reader_i32(struct reader *r)
{
    return (int32_t)reader_u32(r);
}

uint64_t reader_u64(struct reader *r)
{
    uint64_t hi = reader_u32(r);
    uint64_t lo = reader_u32(r);

    return (hi << 32) | lo;
}

size_t reader_bytes(struct reader *r, const uint8_t **out)
{
    uint32_t n = reader_u32(r);

    *out = NULL;
    if (r->failed || r->len - r->pos < n) {
        r->failed = true;
        return 0;
    }
    *out = r->data + r->pos;
    r->pos += n;
    return n;
}

bool reader_str(struct reader *r, char *dst, size_t dst_size)
{
    const uint8_t *p;
    size_t n = reader_bytes(r, &p);

    if (r->failed || n >= dst_size) {
        r->failed = true;
        if (dst_size > 0)
            dst[0] = '\0';
        return false;
    }
    if (n > 0)
        memcpy(dst, p, n);
    dst[n] = '\0';
    return true;
}
