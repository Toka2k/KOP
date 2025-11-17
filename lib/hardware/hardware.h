#ifndef ___HARDWARE___
#define ___HARDWARE___

#include <Arduino.h>
#include "address_table.h"
#include "definitions.h"
#include "packet_buffering.h"

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
    DEFCHANNEL = 0
};

extern int (*protocols[256])(packed_header, byte*, byte);

#ifdef __cplusplus
}
#endif

int get_hw_flags();
void OnReceive();
void send_packet(packet p);
packet packet_init(packed_header ph, byte* payload);
packed_header PACK_HEADER(unpacked_header uh);
unpacked_header UNPACK_HEADER(packed_header ph);
unsigned short HASH_PH(packed_header ph);
unsigned short HASH_UH(unpacked_header uh);
int ROUTING(packed_header ph, byte* data);



#endif