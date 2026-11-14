#ifndef NET_H
#define NET_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "proto.h"

/* Loop until all bytes are written/read. Retries on EINTR and partial I/O.
 * Return 0 on success, -1 on error or (for read_all) EOF before n bytes. */
int write_all(int fd, const void *buf, size_t n);
int read_all(int fd, void *buf, size_t n);

/* Send one frame (header + payload). Returns 0 or -1. */
int proto_send(int fd, uint32_t type, const void *payload, size_t len);

/* Receive one frame. On success returns 0, sets *type, *len and *payload to a
 * malloc'd buffer (NULL if len == 0) that the caller must free().
 * Returns -1 on error, EOF, or a payload larger than MSG_MAX_PAYLOAD. */
int proto_recv(int fd, uint32_t *type, uint8_t **payload, size_t *len);

/* ---- Payload builder: append fields in network byte order ---- */

struct buf {
    uint8_t *data;
    size_t len;
    size_t cap;
    bool failed;   /* set if an allocation failed; check before sending */
};

void buf_init(struct buf *b);
void buf_free(struct buf *b);
void buf_put_u32(struct buf *b, uint32_t v);
void buf_put_i32(struct buf *b, int32_t v);
void buf_put_u64(struct buf *b, uint64_t v);
void buf_put_bytes(struct buf *b, const void *p, size_t n); /* u32 length + bytes */
void buf_put_str(struct buf *b, const char *s);             /* same, without the NUL */
int buf_send(int fd, uint32_t type, const struct buf *b);

/* ---- Payload reader: consume fields; any overrun sets r->failed ---- */

struct reader {
    const uint8_t *data;
    size_t len;
    size_t pos;
    bool failed;
};

void reader_init(struct reader *r, const uint8_t *data, size_t len);
uint32_t reader_u32(struct reader *r);
int32_t reader_i32(struct reader *r);
uint64_t reader_u64(struct reader *r);
/* Points *out at the bytes inside the payload (not copied). Returns the length. */
size_t reader_bytes(struct reader *r, const uint8_t **out);
/* Copies a string into dst (always NUL-terminated); fails if it doesn't fit. */
bool reader_str(struct reader *r, char *dst, size_t dst_size);

#endif
