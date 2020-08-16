#ifndef RZ_RBUF_H
#define RZ_RBUF_H

#define RZ_RBUF_SUCCESS   0
#define RZ_RBUF_FAIL      -1

struct rz_rbuf {
    unsigned char *start;
    unsigned char *end;
    unsigned char *head;
    unsigned char *tail; 
    int count;
};

struct rz_rbuf* rz_rbuf_create(int size);
void rz_rbuf_delete(struct rz_rbuf* buf);

int rz_rbuf_push(struct rz_rbuf *buf, unsigned char value);
int rz_rbuf_pop(struct rz_rbuf *buf, unsigned char *value);
int rz_rbuf_get_count(struct rz_rbuf *buf);
void rz_rbuf_clear(struct rz_rbuf *buf);

int rz_rbuf_head_offset(struct rz_rbuf *buf, int offset);

#endif //RZ_RBUF_H
