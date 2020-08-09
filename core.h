#ifndef CORE_H
#define CORE_H

enum cs_type {
    CS_TYPE_SUM,
    CS_TYPE_XOR,
};


struct smp_s {
    char *name;
    int header;
    int header_size;
    int len_size;
    int max_payload;
    enum cs_type cs;
    struct rz_rbuf *buf;

    char header_flag :1;
    char len_flag :1;
    char cs_flag :1;
    char reserved :5;
    
    void (*phy_tx)(struct smp_s*, char*, int);

    //header + size + cmd +  cs
    int cmd_len;

};

int new_regist(struct smp_s * smp, 
    int header, 
    int header_size, 
    int len_size, 
    int max_payload, 
    int cs_type,
    int buf_size);


char* find_packet_header(struct smp_s *smp);
int get_packet_len(struct smp_s *smp);
int verify_checksum(struct smp_s *smp);


void do_packet(struct smp_s *smp);
int phy_rx(struct smp_s *smp, char* s, int len);





#endif //CORE_H
