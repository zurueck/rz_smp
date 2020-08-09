#include <stdlib.h>
#include "core.h"
#include "rz_rbuf.h"


int new_regist(struct smp_s * smp, 
    int header, 
    int header_size, 
    int len_size, 
    int max_payload, 
    int cs_type,
    int buf_size) {

    smp->header = header;
    smp->header_size = header_size;
    smp->len_size = len_size;
    smp->max_payload = max_payload;
    smp->cs = cs_type;
    smp->header_flag = 0;
    smp->len_flag = 0;
    
//#ifdef BUFFER_SIZE
//   t->head = buf
//#elif defind(MALLOC)
//    malloc(sizeof())
//#endif

    //do buffer
    smp->buf->start = (char *)malloc(sizeof(char) * buf_size);
    if (!smp->buf->start) {
        return -1;
    }

    smp->buf->tail = smp->buf->head = smp->buf->start;
    smp->buf->end = smp->buf->start + buf_size;

    return 0;
}

//done
char* find_packet_header(struct smp_s * smp) {
    int header = smp->header;
    int header_size = smp->header_size;
    char *bufp = smp->buf->head;
    int m;
    int tmp;


    while (smp->buf->head != smp->buf->tail) {
        //find head byte 0
        while(*bufp != header & 0xFF) {
            goto HEAD_MOVE_FORWARD;
        }

        tmp = 0;
        for (m = 0; m < header_size; m++) {
            tmp |= *(bufp + m) << (m * 8);
        }

        if (tmp == header) {
            return bufp;   
        } 

HEAD_MOVE_FORWARD:
        //ptr++
        if (bufp == smp->buf->end) {
            bufp = smp->buf->start;
        } else {
            bufp++;
        } 
    }

    return 0;
}

//int get_used_size(struct smp_s *smp) {
//    int len;
//    
//    len = smp->tail - smp->head; 
//    if (len < 0) {
//        len = len + smp->buf->end - smp->buf->start;
//    }
//    return len;
//}

//done
//return 0 no data
//>0 leng
int get_packet_len(struct smp_s *smp) {
    int len = 0, m;
    char *ptr;
    struct rz_rbuf *buf = smp->buf;

    ptr = buf->head;
   
    //not receive enough byte
    if (buf->count < smp->header_size + smp->len_size) {
        return 0;
    }

    for (m = 0; m < smp->len_size; m++) {
        len |= *ptr << (8 * m);
        ptr++;
    }

    return len;
}

int verify_checksum(struct smp_s *smp) {
    int packet_len;
    int m;
    int cs;
    char *ptr;

    ptr = smp->buf->head;
    packet_len = smp->cmd_len + smp->header_size + smp->len_size;

    for (m=0; m<packet_len; m++ ) {
        cs += *ptr++;
    }

    if (cs&0xFF == *ptr) {
        return 0; //pass
    }

    return -1; //gg 
}


void do_packet(struct smp_s *smp) {
    int len;
    int tmp;
    int rc;
    struct rz_rbuf *buf = smp->buf;

    while(1) {
        //search header
        if (!(smp->header_flag)) {
            buf->head = find_packet_header(smp);
            if (!buf->head) {
                return; //not enough byte
            }
            buf_move_head(smp->buf, buf->head);
            smp->header_flag = 1;
        }

        //get smp length 
        //0 = no
        //x = has
        if (smp->header_flag && !smp->len_flag) {
            smp->cmd_len = get_packet_len(smp);
            //check smp length
            if (!smp->cmd_len) {
                return; //not enough byte
            }
            if (smp->cmd_len > CONFIG_CMD_MAX_LEN) {
                smp->header_flag = 0;
                buf_move_head(buf, ++buf->head);
                continue; //start from header again
            }
            smp->len_flag = 1;       
        }
        
        tmp = smp->cmd_len + smp->header_size + smp->len_size + 1;
        if (smp->buf->count >= tmp) {
            //receive whole packet
            rc = verify_checksum(smp);
            if (rc) {
                smp->header_flag = 0;
                smp->len_flag = 0;
                buf_move_head(buf, ++buf->head);
                continue; //start from header again
            }

            smp->cs_flag = 1;
//#if CONFIG_CALLBACK_READY
            // callback function pointer
//#endif
            return; //ready
        }
        return;     //not enough byte
    } //while(1)
}   //do_packet

int phy_rx(struct smp_s *smp, char *s, int len) {
    int m;
     
    for (m = 0; m < len; m++) {
        if (buf_push(smp->buf, *s)) {
            break;   
        }
        s++;
    } 

    //do_packet(t);
}


int main(int argc, char *argv[]) {
    
    
    
    return 0;
}
