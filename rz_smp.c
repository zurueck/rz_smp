#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "rz_smp.h"


static struct smp_descriptor* find_desc_name(struct smp_s *smp, const char *str) 
{
    struct smp_descriptor *desc = smp->desc;
    int m;
    
    for (m = 0; m < smp->desc_size; m++, desc++) {
        if (!strncmp(desc->name, str, 4)) {
            return desc;
        }
    }

    return 0;
}

static int find_offset_name(struct smp_s *smp, const char *str) 
{
    struct smp_descriptor *desc = smp->desc;
    int m;
    int offset = 0;
    
    for (m = 0; m < smp->desc_size; m++, desc++) {
        if (!strncmp(desc->name, str, 4)) {
            return offset;
        }
        offset += desc->size;
    }

    return 0;
}

static int get_extra_len(struct smp_s *smp) 
{
    struct smp_descriptor *desc = smp->desc;
    int extra_len = 0;
    int m;

    for (m = 0; m < smp->desc_size; m++, desc++) {
        if (!(desc->type & DESC_TYPE_IN_LEN)) {
            extra_len += desc->size;
        }
    }

    return extra_len;
}

static int get_intra_len(struct smp_s *smp) 
{
#if CONFIG_SMP_FIXED_LENG
    return 0;
#else
    struct smp_descriptor *desc = smp->desc;
    int intra_len = 0;
    int m;

    for (m = 0; m < smp->desc_size; m++, desc++) {
        if (desc->type & DESC_TYPE_IN_LEN) {
            intra_len += desc->size;
        }
    }
    return intra_len;
#endif
}

// 0 = not get
static int get_cmd_len(struct smp_s *smp) 
{
    struct smp_descriptor *desc;
#if !CONFIG_SMP_FIXED_LENG
    unsigned char *bufp;
    int cmd_len = 0, offset;
    int m;
    
    desc = find_desc_name(smp, "LENG");
    offset = find_offset_name(smp, "LENG");

    //receive enough byte
    if (rz_rbuf_get_count(smp->buf) < offset + desc->size) { 
        return 0; 
    } 
    
    bufp = smp->buf->head + offset;
    for (m = 0; m < desc->size; m++) {
        cmd_len |= (int)*bufp << (8 * m);
        bufp++;
        if (bufp = smp->buf->end) {
            bufp = smp->buf->start; 
        }
    }
#else
    desc = find_desc_name(smp, "PAYL");
    return desc->size
#endif

    return cmd_len;
}

static int get_payload_len(struct smp_s *smp) 
{
    return get_cmd_len(smp)-get_intra_len(smp);
}

// 0 = ok
// 
static int verify_header(struct smp_s *smp) 
{
    struct smp_descriptor *desc;
    unsigned char *bufp;
    int tmp_header= 0, m;
    
    desc = find_desc_name(smp, "HEAD");
    if (!desc) {
        return SMP_RES_ERROR;
    }

    bufp = smp->buf->head;
    for (m = 0; m < desc->size; m++) {
        tmp_header |= (int)*(bufp) << (m * 8);
        bufp++;
        if (bufp = smp->buf->end) {
            bufp = smp->buf->start; 
        }
    }
    tmp_header = tmp_header & (0xFFFFFFFF >> (8 * (4 - desc->size)));
    
    if (tmp_header == desc->value) { //little endien
        return SMP_RES_OK;
    }

    return SMP_RES_ERROR;
}

static int verify_checksum(struct smp_s *smp) 
{
    unsigned char *bufp = smp->buf->head;
    struct smp_descriptor *desc = smp->desc;
    char str_payload[] = "PAYL";
    int payload_len;
    int m, n;
    int cs, cs_calc = 0;

    
    //do all desc
    for (m = 0; m < smp->desc_size; m++, desc++) {
        if (!strncmp(desc->name, str_payload, 4)) {
            payload_len = get_payload_len(smp);
            for (n = 0; n < payload_len; n++) {
                cs_calc += *bufp++;
                if (bufp == smp->buf->end) {
                    bufp = smp->buf->start;   
                }
            }
            continue;
        }

        if (desc->type & DESC_TYPE_IN_CS) {
            for (n = 0; n < desc->size; n++) {
                cs_calc += *bufp++;
                if (bufp == smp->buf->end) {
                    bufp = smp->buf->start;   
                }
            } 
        }
    } //for

    //get checksum
    desc = find_desc_name(smp, "CHEC");
    cs = desc->value;
    if (cs_calc & 0xFF == cs & 0xFF) {
        return SMP_RES_OK; //pass
    }

    return SMP_RES_ERROR; //gg 
}

void do_callback(struct smp_s *smp) 
{
    int offset, len;
    unsigned char v;

    
    offset = find_offset_name(smp, "PAYL");
    len = get_payload_len(smp);
    rz_rbuf_head_offset(smp->buf, offset);

    //call back zero copy
    (*smp->payload_upper_tx)(smp->buf->head, len); 
    
    //pop data
    for (int m = 0; m < len; m++) {
        rz_rbuf_pop(smp->buf, &v);
    }
}

int new_regist(struct smp_s *smp, struct smp_descriptor *desc, int desc_size, int buf_size)
{
    
    //do buffer
    smp->buf = rz_rbuf_create(buf_size);
    if (!smp->buf->start) {
        return SMP_RES_ERROR;
    }
    
    smp->desc = desc;
    smp->desc_size = desc_size;

    //max payload
    if (!smp->max_cmd_len) {
        smp->max_cmd_len = buf_size;
    }

    return SMP_RES_OK;
}

void do_packet(struct smp_s *smp) 
{
    struct rz_rbuf *rbuf = smp->buf;
    int cmd_len, extra_len;
    int tmp, cnt;
    int rc;
    unsigned char value;

    if (smp->packet_flag) { 
        return; 
    } 

    while (rz_rbuf_get_count(rbuf)) {
        rc = verify_header(smp); 
        if (rc) {
            rz_rbuf_pop(rbuf, &value);
            continue;
        }

        cmd_len = get_cmd_len(smp);
        if (!cmd_len) {
            break;
        }
        //need modify
        if (cmd_len > smp->max_cmd_len) {
            rz_rbuf_pop(rbuf, &value);
            continue;
        }

        //extra len
        extra_len = get_extra_len(smp);
        cnt = rz_rbuf_get_count(smp->buf);
        if (cnt < cmd_len + extra_len) {
            /* not enough data */
            break;
        }

        //check cs
        rc = verify_checksum(smp);
        if (!rc) {
            rz_rbuf_pop(rbuf, &value);
            continue;
        } 

#if CONFIG_SMP_CALLBACK
        do_callback(smp);
#else
        smp->packet_flag = 1;
#endif
        break;
    }
} //do_packet

int phy_rx(struct smp_s *smp, char *s, int len) 
{
    int m;
     
    for (m = 0; m < len; m++) {
        if (rz_rbuf_push(smp->buf, *s)) {
            break;   
        }
        s++;
    } 
#if CONFIG_SMP_AUTO_DO_PACKET
    do_packet(smp);
#endif
}

int get_payload(struct smp_s *smp, char* buf) 
{
    int offset;
    int len;
    int rc;
    
    if (!smp->packet_flag) {
        return 0;
    }

    //find payload offset
    offset = find_offset_name(smp, "PAYL");
    //find payload len
    len = get_payload_len(smp);

    //buf offset
    rc = rz_rbuf_head_offset(smp->buf, offset);
    if (rc) {
        return 0;
    }

    //pop
    for (int m = 0; m < len; m++) {
        rz_rbuf_pop(smp->buf, buf);
        buf++;
    }

    //clear flag
    smp->packet_flag = 0;
    //head move to next head

#if CONFIG_SMP_AUTO_DO_PACKET
    do_packet(smp);
#endif

    return len;
}

