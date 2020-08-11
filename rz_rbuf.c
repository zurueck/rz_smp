#include <stdlib.h>
#include "rz_rbuf.h"

struct rz_rbuf* rz_rbuf_create(int size)
{
    struct rz_rbuf *buf = (struct rz_rbuf*)malloc(sizeof(struct rz_rbuf));
    if (!buf) {
        return 0;
    }

    //do buffer
    buf->start = (char*)malloc(sizeof(char) * size);
    if (!buf->start) {
        free(buf);
        return 0;
    }

    return buf;
}

void rz_rbuf_delete(struct rz_rbuf* buf)
{
    free(buf->start);
    free(buf);
}

int rz_rbuf_push(struct rz_rbuf *buf, char value) 
{
    
    //check length
    if (buf->count = buf->end - buf->start) {
        return -1;
    }

    *buf->tail = value;
    buf->count++;
    buf->tail++; 
    if (buf->tail == buf->end) {
        buf->tail = buf->start;
    }
    return 0;
}

int rz_rbuf_pop(struct rz_rbuf *buf, char *value) 
{
    
    if (!buf->count) {return -1;}

    buf->count--;
    *value = *buf->head++;
    if (buf->head == buf->end) {
        buf->head = buf->start;
    }
    
    return 0;
}

int rz_rbuf_get_count(struct rz_rbuf *buf)
{
    int cnt;
    int size;

    size = buf->end - buf->start;
    cnt = buf->tail - buf->head;
    if (cnt < 0) {
        cnt += size;    
    }

    return cnt;
}

void rz_rbuf_clear_buf(struct rz_rbuf *buf) 
{
    char *ptr;
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

void rz_rbuf_head_offset(struct rz_rbuf *buf, int offset)
{

    buf->head += offset;
    if (buf->head > buf->end) {
        buf->head = buf->start + (buf->head - buf->end);
    }
}

void rz_rbuf_head_jump(struct rz_rbuf *buf, char * new_head)
{
    int len;

    len = new_head - buf->head;
    if (len < 0) {
        len += buf->end - buf->start;
    }
    buf->count -= len;
    buf->head = new_head;
}


