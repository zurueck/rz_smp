#include <stdlib.h>
#include "rz_rbuf.h"

struct rz_rbuf* rz_rbuf_create(int size) {
    struct rz_rbuf *buf = (struct rz_rbuf*)malloc(sizeof(struct rz_rbuf));
    if (!buf) {
        return 0;
    }

    //do buffer
    buf->start = (char *)malloc(sizeof(char) * size);
    if (!buf->start) {
        free(buf);
        return 0;
    }

    return buf;
}

int buf_get_count(struct rz_rbuf *buf) {
    int cnt;
    int size;

    size = buf->end - buf->start;
    cnt = buf->tail - buf->head;
    if (cnt < 0) {
        cnt += size;    
    }

    return cnt;
}

//done
int buf_push(struct rz_rbuf *buf, char value) {
    
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

int buf_pop(struct rz_rbuf *buf, char *value) {
    
    if (!buf->count) {return -1;}

    buf->count--;
    *value = *buf->head++;
    if (buf->head == buf->end) {
        buf->head = buf->start;
    }
    
    return 0;
}

char* buf_get_head(struct rz_rbuf *buf) {
    return buf->head;
}

void buf_move_head(struct rz_rbuf *buf, char * new_head) {
    int len;

    len = new_head - buf->head;
    if (len < 0) {
        len += buf->end - buf->start;
    }
    buf->count -= len;
    buf->head = new_head;
}


