#ifndef RZ_RBUF_H
#define RZ_RBUF_H

struct rz_rbuf {
    char *start;
    char *end;
    char *head;
    char *tail; 
    int count;
};

//ring buffer
struct rz_rbuf* rz_rbuf_create(int size);
int rz_rbuf_push(struct rz_rbuf *buf, char value);
int rz_rbuf_pop(struct rz_rbuf *buf, char *value);
int rz_rbuf_get_count(struct rz_rbuf *buf);
char* rz_rbuf_get_head(struct rz_rbuf *buf);
void rz_rbuf_head_offset(struct rz_rbuf *buf, int offset);
void rz_rbuf_move_head(struct rz_rbuf *buf, char * new_head);

#endif
