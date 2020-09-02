# Rz_smp

## Introduction
Rz_smp stands for simple module protocol. It is used for covering common protocol between MCU and modules. 

## Features

## Dependancy
* Rz_rbuf

## How to use
1. create descriptor array
2. declare and initial struct smp_s
3. regirst (1) and (2) by API new_regist()
4. use phy_rx() to push data
5. use do_packet() to do with the data
6. check smp->packet_flag,
   if 1, means packet is valid and payload is ready to be read
7. use get_payload() to get data

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
rz_smp use "name" to recognize some specific parts in packet. So there are some reserved name for system use. Except them, user can create their own name

Reserved name
* HEAD
* LENG
* PAYL
* CHEC

### type
type is a bit field controlled parameter. it help system understand this descriptor 
* PAYLOAD
it tells system its size is disable, and will be sent to upper layer
* IN_LEN
it tells system its length is included in LENG
* IN_CS
it tells system its value is included in CS
* check_sum
* CRC

### size
lenth of this descriptor in packet (bytes)

### value
some part in packet is typically magic number
e.g. HEADER, END, 1's compliment etc.


### example

    not finish yet

| HEADER   | LENG       | PAYLOAD  | CS |
| -------- | --------   | -------- | -- | 
|  AA      | HEADER+LENG+PAYLOAD+CS         | Text     |    |
|          |            |          |    |




###
## Configurations

## Todo
1. add direction feature
2. add doxygen style comment
3. more specific readme
4. add ending feature
5. add asynchronous feature
6. 64bit debug 
7. add RTOS supporting 
8. add xmodem mode
