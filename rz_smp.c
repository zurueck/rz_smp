#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "rz_smp.h"


static struct smp_descriptor* find_desc_name(struct smp_s *smp, const char *str) 
{
    struct smp_descriptor *desc = smp->desc;
    int m;
    
    for (m = 0; m < smp->desc_num; m++, desc++) {
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
    
    for (m = 0; m < smp->desc_num; m++, desc++) {
        if (!strncmp(desc->name, str, 4)) {
            return offset;
        }
        offset += desc->size;
    }

    return 0;
}

static int get_len_by_type(struct smp_s *smp, unsigned char type) {
    struct smp_descriptor *desc = smp->desc;
    int type_len = 0;
    int m;

    for (m = 0; m < smp->desc_num; m++, desc++) {
        if (desc->type & type || type == 0xFF) {
            type_len += desc->size;
        }
    }

    return type_len; 
}

static int get_extra_len(struct smp_s *smp) 
{
    return get_len_by_type(smp, 0xFF) - get_len_by_type(smp, DESC_TYPE_IN_LEN);
}

static int get_intra_len(struct smp_s *smp) 
{
#if CONFIG_SMP_FIXED_LENG
    return 0;
#else
    return get_len_by_type(smp, DESC_TYPE_IN_LEN);
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

    if (rz_rbuf_get_count(smp->buf) < offset + desc->size) { 
        //receive not enough byte
        return 0; 
    } 
    
    bufp = smp->buf->head + offset;
    for (m = 0; m < desc->size; m++) {
        cmd_len |= (int)*bufp << (8 * m);
        bufp++;
        if (bufp == smp->buf->end) {
            bufp = smp->buf->start; 
        }
    }
    return cmd_len;
#else
    desc = find_desc_name(smp, "VDAT");
    return desc->size
#endif
}

static int get_vdat_len(struct smp_s *smp) 
{
    return get_cmd_len(smp)-smp->intra_len;
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
        if (bufp == smp->buf->end) {
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
    char str_payload[] = "VDAT";
    int vdat_len;
    int m, n;
    int cs, cs_calc = 0;

    
    //do all desc
    for (m = 0; m < smp->desc_num; m++, desc++) {
        if (!strncmp(desc->name, str_payload, 4)) {
            vdat_len = get_vdat_len(smp);
            for (n = 0; n < vdat_len; n++) {
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
    if ((cs_calc & 0xFF) == (cs & 0xFF)) {
        return SMP_RES_OK; //pass
    }

    return SMP_RES_ERROR; //gg 
}

static void get_payload_offset_len(struct smp_s *smp, int *offset, int *len) {
    struct smp_descriptor *desc = smp->desc;
    int offset_flag = 0;
    int m;

    *offset = 0; 
    *len = 0;
    //get offset
    //get len
    for (m = 0; m < smp->desc_num; m++, desc++) {
        if (desc->type & DESC_TYPE_PAYLOAD) {
            offset_flag = 1;
            if (desc->size == 0) {
                *len += get_vdat_len(smp);
            } else {
                *len += desc->size; 
            }
        } else {
            if (!offset_flag) {
                *offset += desc->size;
            }
        }
    }
}

static int is_calcLenFlag_raised(struct smp_s *smp) {
    struct smp_descriptor *desc = smp->desc;
    char str_LENG[] = "LENG";
    int m;

    for (m = 0; m < smp->desc_num; m++, desc++) {
        if (!strncmp(desc->name, str_LENG, 4)) {
            return 1;
        }
        if (desc->type & DESC_TYPE_PAYLOAD) {
            return 0;
        }
    }

    return -1;
}

//len = payload leng from upper
//len - all known len = unknown len = vdat
static int cal_vdat_len(struct smp_s *smp, int len) {
    struct smp_descriptor *desc = smp->desc;
    int vdat_len;
    int m;

    vdat_len = len; 
    for (m = 0; m < smp->desc_num; m++, desc++) {
        if (desc->type & DESC_TYPE_PAYLOAD) {
            vdat_len -= desc->size;
        }
    }

    return vdat_len;
}

static int cal_tx_cs(struct smp_s *smp, unsigned char *buf) 
{
    struct smp_descriptor *desc = smp->desc;
    int m, n, 
        cs = 0;
    unsigned char *p = buf;
    
    for (m = 0; m < smp->desc_num; m++, desc++) {
        if (desc->type & DESC_TYPE_IN_CS) {
            for (n = 0; n < desc->size; n++) {
                cs += *p++;
            }
        } else {
            p += desc->size;
        }
    }

    return cs;
}




void do_callback(struct smp_s *smp) 
{
    int offset, len;
    unsigned char v;

    //get offset and len
    get_payload_offset_len(smp, &offset, &len);
    rz_rbuf_head_offset(smp->buf, offset);

    //call back zero copy
    (*smp->payload_upper_tx)((char *)smp->buf->head, len); 
    
    //pop data to null
    rz_rbuf_head_offset(smp->buf, len);
}

int new_regist(struct smp_s *smp, struct smp_descriptor *desc, int desc_num, int buf_size)
{
    
    //do buffer
    smp->buf = rz_rbuf_create(buf_size);
    if (!smp->buf->start) {
        return SMP_RES_ERROR;
    }
    
    smp->desc = desc;
    smp->desc_num = desc_num;

    //max payload
    if (!smp->max_cmd_len) {
        smp->max_cmd_len = buf_size;
    }

    smp->intra_len = get_intra_len(smp);
    smp->extra_len = get_extra_len(smp);
    smp->calc_len_flag = is_calcLenFlag_raised(smp);

    return SMP_RES_OK;
}

void do_packet(struct smp_s *smp) 
{
    struct rz_rbuf *rbuf = smp->buf;
    int cmd_len;
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
        cnt = rz_rbuf_get_count(smp->buf);
        if (cnt < cmd_len + smp->extra_len) {
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

    return 0;
}


int get_payload(struct smp_s *smp, char* buf) 
{
    int offset = 0;
    int len = 0;
    int rc;
    
    if (!smp->packet_flag) {
        return 0;
    }

    get_payload_offset_len(smp, &offset, &len);

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

int smp_send_data(struct smp_s *smp, unsigned char *buf, int len)
{
    struct smp_descriptor *desc = smp->desc;
    unsigned char txBuf[CONFIG_SMP_TX_BUF_SIZE];
    int m, n, tmp;
    unsigned char *p, *bufp;
    int offset, cs, packet_len, vdat_len;


    //write vdat size
    vdat_len = cal_vdat_len(smp, len);
    desc = find_desc_name(smp, "VDAT"); 
    desc->size = vdat_len;

    if (smp->calc_len_flag) {
        //write LENG value = vdat + intra
        desc = find_desc_name(smp, "LENG"); 
        desc->value = vdat_len + smp->intra_len;
    }

    //write all to txBuf
    desc = smp->desc;
    p = txBuf;
    bufp = buf;
    tmp = 0;
    for (m = 0; m < smp->desc_num; m++, desc++) {
        if (desc->type & DESC_TYPE_PAYLOAD) {
            if (!tmp) {
                for (n = 0; n < len; n++) {
                    *p++ = *bufp++;
                }
                tmp = 1;    //has read payload
            }
        } else {
            for (n = 0; n < desc->size; n++) {
                *p++ = (desc->value >> (8*n)) & 0xFF;   //little endian
            }
        }
    }

    //cs
    cs = cal_tx_cs(smp, txBuf);
    offset = find_offset_name(smp, "CHEC");
    *(txBuf + offset) = cs;
    
    //tx function pointer
    packet_len = smp->extra_len + vdat_len + smp->intra_len;
    (*smp->phy_tx)(smp, txBuf, packet_len);

    //clear vdat size
    desc = find_desc_name(smp, "VDAT"); 
    desc->size = 0;

    //clear leng value
    if (smp->calc_len_flag) {
        desc = find_desc_name(smp, "LENG"); 
        desc->value = 0;
    }

    return 0;
}

