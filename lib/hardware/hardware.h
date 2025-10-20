#ifndef ___HARDWARE___
#define ___HARDWARE___

#include <Arduino.h>
#include "address_table.h"
#include "definitions.h"
#include "../RadioLib/src/TypeDef.h"

#define PAYLOAD_SIZE (256 - sizeof(packed_header))
#define PACKET_SIZE (sizeof(packed_header) + ph.len)

#define MAX_NEIGHBOURS 256
#define SECRET_COUNT 1

#ifdef __cplusplus
extern "C" {
#endif

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

extern int (*protocols[256])(packed_header, byte*, byte);

#ifdef __cplusplus
}
#endif

void OnReceive();
int send_packet(packet p);
packet packet_init(packed_header ph, byte* payload);
packed_header PACK_HEADER(unpacked_header uh);
unpacked_header UNPACK_HEADER(packed_header ph);
unsigned short HASH_PH(packed_header ph);
unsigned short HASH_UH(unpacked_header uh);

extern int hw_flags;

#endif