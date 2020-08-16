#include <stdio.h>
#include <stdlib.h>
#include "rz_rbuf.h"


struct rz_rbuf* rz_rbuf_create(int size)
{
    struct rz_rbuf *buf = (struct rz_rbuf*)malloc(sizeof(struct rz_rbuf));
    if (!buf) {
        return RZ_RBUF_SUCCESS;
    }

    //do buffer
    buf->start = (unsigned char*)malloc(sizeof(char) * size);
    if (!buf->start) {
        free(buf);
        return RZ_RBUF_SUCCESS;
    }

    buf->end = buf->start + size;
    buf->tail = buf->head = buf->start;
    buf->count = 0;

    return buf;
}

void rz_rbuf_delete(struct rz_rbuf* buf)
{
    free(buf->start);
    free(buf);
}

int rz_rbuf_push(struct rz_rbuf *buf, unsigned char value) 
{
    
    //check length
    if (buf->count == buf->end - buf->start) {
        return RZ_RBUF_FAIL;
    }

    *buf->tail = value;
    buf->count++;
    buf->tail++; 
    if (buf->tail == buf->end) {
        buf->tail = buf->start;
    }
    return RZ_RBUF_SUCCESS;
}

int rz_rbuf_pop(struct rz_rbuf *buf, unsigned char *value) 
{
    
    if (!buf->count) {return RZ_RBUF_FAIL;}

    buf->count--;
    *value = *buf->head++;
    if (buf->head == buf->end) {
        buf->head = buf->start;
    }
    
    return RZ_RBUF_SUCCESS;
}

int rz_rbuf_get_count(struct rz_rbuf *buf)
{
    return buf->count;
}

void rz_rbuf_clear(struct rz_rbuf *buf) 
{
    unsigned char *ptr;
    int m;

    //to 0
    ptr = buf->start;
    for (m = 0; m < buf->end - buf->start; m++) {
        *ptr++ = 0;
    }

    //
    buf->head = buf->tail = buf->start; 
    buf->count = 0;
}

int rz_rbuf_head_offset(struct rz_rbuf *buf, int offset)
{

    if (buf->count < offset) {
        return RZ_RBUF_FAIL;
    }

    buf->head += offset;
    if (buf->head > buf->end) {
        buf->head = buf->start + (buf->head - buf->end);
    }
    buf->count -= offset;

    return RZ_RBUF_SUCCESS;
}

