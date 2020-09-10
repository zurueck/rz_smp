# Rz_smp

## Introduction
Rz_smp stands for simple module protocol. It is used for covering common protocol between MCU and modules. 


## Dependancy
* Rz_rbuf
* standard library
 

## How to use
1. set configuration in rz_smp.h
2. Create descriptor array based on protocol 
3. Implement callback function phy_tx()
4. initial a variable struct smp_s
5. regist (1), (3)
6. use API phy_rx() to push data
7. use API do_packet() to analyse data anytime
8. check smp->packet_flag,
   if 1, means packet is valid and payload is ready to be read
10. use get_payload() to get data

### about descriptor
Descriptor is used for desciption each part of packet

```
struct smp_descriptor {
    char *name;
    char type;
    int size;
    int value;
};
```
#### name
rz_smp use "name" to recognize some specific parts in packet. So there are some reserved name for rz_smp use. Except them, Users can create their own name

Reserved name
* HEAD (header)
* LENG (length)
* VDAT (data with vary length)
* CHEC (checksum)

### type
type is a bit field controlled parameter. rz_smp will process data according to type
* DESC_TYPE_PAYLOAD
Indicate the part is included in payload and will be send to upper layer
* DESC_TYPE_IN_LEN
Indicate the part size is counted in LENG part
* DESC_TYPE_IN_CS
Indicate the part in included in calculating checksum
* DESC_CSTYPE_CS
CS type = checksum
* DESC_CSTYPE_CRC8
CS type = CRC8

### size
length of this part in packet (bytes)

### value
Magic numbe
e.g. HEADER, END, 1's compliment etc.


### example1


| HEADER | LENGTH     | PAYLOAD     | CS    |
| ------ | ---------- | ----------- | ----- |
| 1byte  | 1 byte     | cmd +arg    | 1byte |
| 0xAA   |  L + P + C | cmd +arg    | P + L |

            |----------------- length -----------------|
            |---------- CS ----------|
   
packet = 0xAA 0x04 0x01 0x03 0x08   
HEADER= 0xAA
LENGTH = 1 + 2 + 1 = 4
CS = L + P = 0x04 + (0x01+0x03) = 0x08

Declare descriptor
```
{
    .name = "HEAD",
    .type = 0,
    .size = 1,
    .value = 0xAA
}
{
    .name = "LENG",
    .type = DESC_TYPE_IN_LEN | DESC_TYPE_IN_CS,
    .size = 1,
    .value = 0,
}
{
    .name = "VDAT",
    .type = DESC_TYPE_IN_LEN | DESC_TYPE_IN_CS,
    .size = 0,  //VDAT size has to be 0
    .value = 0,
}
{
    .name = "CHEC",
    .type = DESC_TYPE_IN_LEN,
    .size = 1,
    .value = 0,
}
```
    
### example 2
fixed length

| START | DIR | SERIAL | PAYLOAD | CRC | END |
| ----- | --- | ------ | ------- | --- | --  | 
| 1B    | 1B  | 4B     | 64B     | 2B  | 1B  |
| 0x5A  | 0/1 | int    |         | crc | 0xA5|    |

                              |--- CS ---|

```
{
    .name = "HEAD",
    .type = DESC_TYPE_FIXED_VALUE,
    .size = 1,
    .value = 0x5A
}
{
    .name = "DIR",
    .type = 0,
    .size = 1,
    .value = 0,
}
{
    .name = "SERIAL",
    .type = 0,
    .size = 4,
    .value = 0,
}
{
    .name = "PAYL",
    .type = DESC_TYPE_IN_LEN,
    .size = 64,
    .value = 0,
}
{
    .name = "CHEC",
    .type = 0,
    .size = 2,
    .value = 0,
}
{
    .name = "END",
    .type = DESC_TYPE_FIXED_VALUE,
    .size = 1,
    .value = 0xA5,
}
```


## Configurations
* CONFIG_SMP_CALLBACK
  value: boolin
  if packet is ready, call payload_ypper_tx() automatically

* CONFIG_SMP_AUTO_DO_PACKET
  value: boolin
  auto run do_packet() everytime phy_rx() is called
  
* CONFIG_SMP_FIXED_PACKET_LENGTH
  value: boolin
  packet length is fixed, size in VDAT descriptor must be set

* CONFIG_SMP_TX_BUF_SIZE
  value: int range

## Todo
* add direction feature
* add doxygen style comment
* add ending feature
* add asynchronous feature
* 64bit debug 
* add RTOS supporting 
* add zero copy, not zero copy config
* add endian support (only little endian now)
* add other checksum method
* test.c
* seperate tx rx, make reentry work
* add malloc config (especially for tx)
