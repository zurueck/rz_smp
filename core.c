#include <stdlib.h>
#include <string.h>
#include "core.h"
#include "rz_rbuf.h"


int new_regist(struct smp_s *smp,
    struct smp_descriptor * desc, 
    int desc_size,
    int buf_size) 
{
    
    smp->desc = desc;
    smp->desc_size = desc_size;

    //do buffer
    smp->buf = rz_rbuf_create(buf_size);
    if (!smp->buf->start) {
        return -1;
    }

    return 0;
}

struct smp_descriptor* find_desc_name(struct smp_s *smp, const char *str) {
    int m;
    struct smp_descriptor *desc;
    
    desc = smp->desc;

    for (m = 0; m < smp->desc_size; m++, desc++) {
        if (!strncmp(desc->name, str, 4)) {
            return desc;
        }
    }

    return 0;
}

int find_offset_name(struct smp_s *smp, const char *str) {
    int m;
    struct smp_descriptor *desc;
    int offset;
    
    desc = smp->desc;

    for (m = 0; m < smp->desc_size; m++, desc++) {
        if (!strncmp(desc->name, str, 4)) {
            return offset;
        }
        offset += desc->size;
    }

    return 0;
}

// 0 = ok
// 
int verify_header(struct smp_s *smp) {
    char *bufp;
    int tmp, m;
    struct smp_descriptor *desc;
    
    desc = find_desc_name(smp, "HEAD");
    if (!desc) {
        return -2;    //EMEG
    }

    bufp= smp->buf->head;
    for (m = 0; m < desc->size; m++) {
        tmp |= (int)*(bufp) << (m * 8);
        bufp++;
        if (bufp = smp->buf->end) {
            bufp = smp->buf->start; 
        }
    }

    if (tmp == desc->value) { //little endien
        return 0;   
    }

    return -1;
}

int verify_checksum(struct smp_s *smp) {
    int payload_len;
    int m, n, tmp = 0;
    char *bufp;
    struct smp_descriptor *desc;
    int offset;
    int cs;

    bufp = smp->buf->head;
    
    //do all desc
    for (m = 0; m < smp->desc_size; m++) {
        if (desc->name == "PAYL") {
            //bufp at payload
            //do payload
            offset = find_offset_name(smp, "PAYL");
            bufp = smp->buf->head + offset;

            payload_len = get_payload_len(smp);
            for (m = 0; m < payload_len; m++) {
                // if end go to start
                tmp += *bufp++;
                if (bufp == smp->buf->end) {
                    bufp = smp->buf->start;   
                }
            }
            continue;
        }
        if (desc->type & DESC_TYPE_IN_CS) {
            for (n = 0; n < desc->size; n++) {
                //cs type
                tmp += *bufp++;
                if (bufp == smp->buf->end) {
                    bufp = smp->buf->start;   
                }
            } 
        }
    }

    //get checksum
    desc = find_desc_name(smp, "CHEC");
    cs = desc->value;
    
    if (tmp & 0xFF == cs & 0xFF) {
        return 0; //pass
    }

    return -1; //gg 
}

// 0 = not get
int get_packet_len(struct smp_s *smp) {
    struct smp_descriptor *desc;
    char *bufp;
    int packet_len = 0, offset = 0;
    int m, rc;
    
    desc = smp->desc;
    rc = 0;
    for (m = 0; m < smp->desc_size; m++, desc++) {
        if (desc->name == "LENG") {
            rc = 1;
            break;
        }
        offset += desc->size;
    }
    
    if (rc) { return 0; }
    if (rz_rbuf_get_count(smp->buf) < offset) { return 0; } 

    bufp = smp->buf->head + offset;
    for (m = 0; m < desc->size; m++) {
        packet_len |= (int)*bufp << (8 * m);
        bufp++;
        if (bufp = smp->buf->end) {
            bufp = smp->buf->start; 
        }
    }

    return packet_len;
}

int get_payload_len(struct smp_s *smp) {
    int packet_len, intra_len;
    
    packet_len = get_packet_len(smp);
    intra_len = get_intra_len(smp);

    return packet_len - intra_len;
}

int get_extra_len(struct smp_s *smp) {
    struct smp_descriptor *desc;
    int extra_len = 0;
    int m;

    desc = smp->desc;
    for (m = 0; m < smp->desc_size; m++, desc++) {
        if (!(desc->type & DESC_TYPE_IN_LEN)) {
            extra_len += desc->size;
        }
    }
    return extra_len;
}

int get_intra_len(struct smp_s *smp) {
    struct smp_descriptor *desc;
    int intra_len = 0;
    int m;

    desc = smp->desc;
    for (m = 0; m < smp->desc_size; m++, desc++) {
        if (desc->type & DESC_TYPE_IN_LEN) {
            intra_len += desc->size;
        }
    }
    return intra_len;
}

void do_packet(struct smp_s *smp) {
    int packet_len, extra_len;
    int tmp, cnt;
    int rc;
    struct rz_rbuf *rbuf = smp->buf;
    char value;

    
    while (rz_rbuf_get_count(rbuf)) {
        rc = verify_header(smp); 
        if (!rc) {
            rz_rbuf_pop(rbuf, &value);
            continue;
        }

        packet_len = get_packet_len(smp);
        if (!packet_len) {
            break;
        }
        if (packet_len > smp->max_payload) {
            rz_rbuf_pop(rbuf, &value);
            continue;
        }

        //extra len
        extra_len = get_extra_len(smp);
        cnt = rz_rbuf_get_count(smp->buf);
        if (cnt < packet_len + extra_len) {
            /* not enough data */
            break;
        }

        //check cs
        rc = verify_checksum(smp);
        if (!rc) {
            rz_rbuf_pop(rbuf, &value);
            continue;
        } 
        smp->packet_flag = 1 ;
        break;
    }
}   //do_packet

int phy_rx(struct smp_s *smp, char *s, int len) {
    int m;
     
    for (m = 0; m < len; m++) {
        if (rz_rbuf_push(smp->buf, *s)) {
            break;   
        }
        s++;
    } 

    //do_packet(t);
}


//#if CONFIG_SELFTEST
int main(int argc, char *argv[]) {
    
    
    
    return 0;
}
//#endif
