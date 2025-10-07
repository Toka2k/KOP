#ifndef ___ROUTING___
#define ___ROUTING___
#include "address_table.h"

#define SECRET_COUNT 1

typedef unsigned char byte;

enum Channels{
    DEFCHANNEL = 0, U1CHANNEL, U2CHANNEL, U3CHANNEL, D1CHANNEL, NET
};

static double __channels[] = {8670E5, 8672E5, 8674E5, 8676E5, 8678E5, 8680E5};
static byte seqnum = 0;
static byte secret[SECRET_COUNT] = {19};

typedef struct {
    unsigned short mac_d : 14;
    unsigned short mac_s : 14;
    unsigned short net_d : 14;
    unsigned short net_s : 14;
    byte length;
    byte protocol_id;
    byte seqnum;
    byte hmac[2];
} unpacked_header;

typedef struct {
    byte addresses[7];
    byte length;
    byte protocol_id;
    byte seqnum;
    byte hmac[2];
} packed_header; 

typedef struct {
    packed_header h;
    byte data[255 - sizeof(packed_header)];
} packet;

static int (*protocols[256])(byte[]) = {
    //function names
};

// Receivers have to track their neighbours last seqnum and they compare onreceive 
// wether it is higher than the last one and compare the hmac
// 
 
packed_header PACK_HEADER(unpacked_header uh);
unpacked_header UNPACK_HEADER(packed_header ph);
unsigned short HASH_PH(packed_header ph);
unsigned short HASH_UH(unpacked_header uh);
#endif
