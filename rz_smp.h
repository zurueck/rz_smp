#ifndef CORE_H
#define CORE_H
#include "rz_rbuf.h"

/* ======== configs ======== */
#define CONFIG_SMP_DIR_DESC             0
#define CONFIG_SMP_CALLBACK             0
#define CONFIG_SMP_AUTO_DO_PACKET       0
/* ======== end configs ======== */

#define SMP_RES_OK                      0
#define SMP_RES_ERROR                   -1

#define DESC_TYPE_PAYLOAD           0x08
#define DESC_TYPE_DIR_IN            0x10
#define DESC_TYPE_DIR_OUT           0x20
#define DESC_TYPE_IN_LEN            0x40
#define DESC_TYPE_IN_CS             0x80


struct smp_descriptor {
    char *name;
    char type;
    int size;
    int value;
};

#define _ADD_SMP_DESC(n, t, s, v) \
    { \
       .name = n, \
       .type = t, \
       .size = s, \
       .value = v, \
    }

enum cs_type {
    CS_SUM,
    CS_XOR,
    CS_1SC,
};

struct smp_s {
    struct smp_descriptor *desc;
    int desc_size;
    struct rz_rbuf *buf;

    unsigned char header_flag :1;
    unsigned char len_flag :1;
    unsigned char cs_flag :1;
    unsigned char packet_flag :1;
    unsigned char reserved :4;

    int max_cmd_len;

    void (*phy_tx)(struct smp_s*, char*, int);  //low level tx
    void (*payload_upper_tx)(char*, int);    //call back
};

int new_regist(struct smp_s *smp, struct smp_descriptor *desc, int desc_size, int buf_size);
void do_packet(struct smp_s *smp);
int phy_rx(struct smp_s *smp, char* s, int len);
int get_payload(struct smp_s *smp, char* buf);
//add zero copy function later


#endif //CORE_H
