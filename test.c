#include <stdio.h>
#include <assert.h>
#include "rz_smp.h"

#define OMG     1
#define KEYI    0


#if OMG 
struct smp_descriptor desc[] = {
    _ADD_SMP_DESC("HEAD", 0, 1, 0xAA),
    _ADD_SMP_DESC("LENG", DESC_TYPE_IN_CS, 1, 0),
    _ADD_SMP_DESC("VDAT", DESC_TYPE_PAYLOAD | DESC_TYPE_IN_LEN | DESC_TYPE_IN_CS, 0, 0),
    _ADD_SMP_DESC("CHEC", DESC_TYPE_IN_LEN, 1, 0),
};

unsigned char s1[] = {0xBB, 0x12, 0x00, 0xAA, 0x03, 0x01, 0x0A, 0x0E};
unsigned char s2[] = {0xBC, 0x13, 0x00, 0xAA, 0x03, 0x01, 0x03, 0x07};
unsigned char s3[] = {0xBB, 0xAA, 0x02, 0xAA, 0x03, 0x01, 0x0A, 0x0E};
#endif

#if KEYI
struct smp_descriptor desc[] = {
    _ADD_SMP_DESC("HEAD", 0, 1, 0xAA),
    _ADD_SMP_DESC("CONT", DESC_TYPE_PAYLOAD, 1, 0),
    _ADD_SMP_DESC("CMD",  DESC_TYPE_PAYLOAD, 1, 0),
    _ADD_SMP_DESC("LENG", DESC_TYPE_PAYLOAD, 1, 0),
    _ADD_SMP_DESC("VDAT", DESC_TYPE_PAYLOAD | DESC_TYPE_IN_LEN | DESC_TYPE_IN_CS, 0, 0),
    _ADD_SMP_DESC("CHEC", 0, 1, 0),
    _ADD_SMP_DESC("END", 0, 1, 0x3D),
};

unsigned char s1[] = {0xBB, 0xAA, 0x03, 0x01, 0x03, 0x01, 0x02, 0x03, 0x06, 0x3D};
unsigned char s2[] = {0xBC, 0x13, 0x00, 0xAA, 0x03, 0x01, 0x03, 0x07};
unsigned char s3[] = {0xBB, 0xAA, 0x02, 0xAA, 0x03, 0x01, 0x0A, 0x0E};
#endif

void callback(char* buf, int len) {

    for (int m=0; m<len; m++) {
        printf("%d = 0x%02X\n", m, *(buf+m));
    }
}    //call back


static struct smp_s s_smp = {
    .payload_upper_tx = callback,
    .max_cmd_len = 20,
};


int main(int argc, char *argv[]) {
    int c;
    struct smp_s *smp = &s_smp;
    unsigned char *p;
    int rc;
    char aa[128];

//auto packet, callback
#if CONFIG_SMP_AUTO_DO_PACKE
    #if CONFIG_SMP_CALLBACK
    // 1, 1

    #else
    // 1, 0

    #endif
#else
    #if CONFIG_SMP_CALLBACK
    // 0, 1

    #else
    //auto packet = 0, callback = 0

    rc = new_regist(smp, desc, sizeof(desc)/sizeof(struct smp_descriptor), 128);
    assert(rc == 0);

    p = smp->buf->start;

    phy_rx(smp, s1, sizeof(s1));
    p = smp->buf->start;
    phy_rx(smp, s2, sizeof(s2));

    p = smp->buf->start;
    do_packet(smp);
    assert(smp->packet_flag);

#if OMG
    c = get_payload(smp, aa);
    assert(c == 2);
    assert(aa[0] == 0x01);
    assert(aa[1] == 0x0A);

    do_packet(smp);
    c = get_payload(smp, aa);
    assert(c == 2);
    assert(aa[0] == 0x01);
    assert(aa[1] == 0x03);
#endif

#if KEYI
//unsigned char s1[] = {0xBB, 0xAA, 0x03, 0x01, 0x03, 0x01, 0x02, 0x03, 0x06, 0x3D};
    printf("extra_len = %d\n", smp->extra_len);

    c = get_payload(smp, aa);
    assert(c == 6);
    assert(aa[0] == 0x03);
    assert(aa[1] == 0x01);
    assert(aa[2] == 0x03);
    assert(aa[3] == 0x01);
    assert(aa[4] == 0x02);
    assert(aa[5] == 0x03);

  //  do_packet(smp);
  //  c = get_payload(smp, aa);
  //  assert(c == 2);
  //  assert(aa[0] == 0x01);
  //  assert(aa[1] == 0x03);


#endif


    #endif  //CONFIG_SMP_CALLBACK
#endif //CONFIG_SMP_AUTO_DO_PACKE


    printf("done\n");
    return 0;
}

