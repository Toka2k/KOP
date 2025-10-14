#ifndef __HARDWARE__
#define __HARDWARE__

#include <Arduino.h>
#include "address_table.h"

#define ERROR (1<<0)
#define INVALID_HASH (1<<1)
#define INVALID_SEQNUM (1<<2)
#define NOT_NEIGHBOUR (1<<3)

#define PAYLOAD_SIZE (256 - sizeof(packed_header))
#define PACKET_SIZE (sizeof(packed_header) + ph.len)

#define MAX_NEIGHBOURS 256

#define CS 1
#define IRQ 2
#define RST 3
#define GPIO 4

typedef unsigned char byte;

typedef struct __attribute__((packed)){
    unsigned short mac_d : 14;
    unsigned short mac_s : 14;
    unsigned short net_d : 14;
    unsigned short net_s : 14;
    byte length;
    byte protocol_id;
    byte seqnum;
    byte hmac[2];
} unpacked_header;

typedef struct __attribute__((packed)){
    byte addresses[7];
    byte length;
    byte protocol_id;
    byte seqnum;
    byte hmac[2];
} packed_header; 

typedef struct __attribute__((packed)){
    packed_header h;
    byte data[PAYLOAD_SIZE];
} packet;

enum Channels{
    DEFCHANNEL = 0, U1CHANNEL, U2CHANNEL, U3CHANNEL, D1CHANNEL, NET
};

void OnReceive();
void sendPacket(packet p);
packet packet_init(packed_header ph, byte payload[PAYLOAD_SIZE]);
packed_header PACK_HEADER(unpacked_header uh);
unpacked_header UNPACK_HEADER(packed_header ph);

extern int (*protocols[256])(byte*, byte);
extern int hw_flags;

#endif