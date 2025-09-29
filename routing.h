#ifndef ___ROUTING___
#define ___ROUTING___
#include "address_table.h"

typedef unsigned char byte;

enum Channels{
    DEFCHANNEL = 0, U1CHANNEL, U2CHANNEL, U3CHANNEL, D1CHANNEL, NET
};

static double __channels[] = {8670E5, 8672E5, 8674E5, 8676E5, 8678E5, 8680E5};

typedef struct {
    unsigned short mac_d : 14;
    unsigned short mac_s : 14;
    unsigned short net_d : 14;
    unsigned short net_s : 14;
    byte length;
    byte protocol_id;
} unpacked_header;

typedef struct {
    byte addresses[7];
    byte length;
    byte protocol_id;
} packed_header; 

typedef struct {
    packed_header h;
    byte data[];
} packet_data;

static int (*protocols[256])(byte[]) = {
    //function names
};

packed_header PACK_HEADER(unpacked_header uh);
unpacked_header UNPACK_HEADER(packed_header ph);
#endif
