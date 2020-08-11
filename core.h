#ifndef CORE_H
#define CORE_H

#define CONFIG_SELF_TEST        1
#define CONFIG_CMD_MAX_LEN      256

#define DESC_TYPE_START      0x01
#define DESC_TYPE_CS         0x02
#define DESC_TYPE_LEN        0x04
#define DESC_TYPE_PAYLOAD    0x08
#define DESC_TYPE_OTHER      0x10

#define DESC_TYPE_IN_LEN     0x40
#define DESC_TYPE_IN_CS      0x80

struct smp_descriptor {
    char *name;
    char type;
    int size;
    int value;
};

enum cs_type {
    CS_SUM,
    CS_XOR,
    CS_1SC,
};

struct smp_s {
    struct smp_descriptor *desc;
    int desc_size;
    
    struct rz_rbuf *buf;
    
    int max_payload;

    char header_flag :1;
    char len_flag :1;
    char cs_flag :1;
    char packet_flag :1;
    char reserved :4;
    
    void (*phy_tx)(struct smp_s*, char*, int);
};


int new_regist(struct smp_s *smp,
    struct smp_descriptor * desc, 
    int desc_size,
    int buf_size);

struct smp_descriptor* find_desc_name(struct smp_s *smp, const char *str);
int find_offset_name(struct smp_s *smp, const char *str);

int verify_header(struct smp_s *smp);
int verify_checksum(struct smp_s *smp);
int get_packet_len(struct smp_s *smp);
int get_payload_len(struct smp_s *smp);
int get_extra_len(struct smp_s *smp);
int get_intra_len(struct smp_s *smp);

void do_packet(struct smp_s *smp);
int phy_rx(struct smp_s *smp, char* s, int len);





#endif //CORE_H
