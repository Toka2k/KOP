#ifndef ___DEFINITIONS___
#define ___DEFINITIONS___

// PACKET FLAGS
#define SUCCESS 0 
#define ERROR (1)
#define INVALID_HASH (1<<1)
#define INVALID_SEQNUM (1<<2)
#define INVALID_LENGTH (1<<3)
#define INVALID_ADDRESS (1<<4)
#define INVALID_PAYLOAD (1<<5)
#define NOT_NEIGHBOUR (1<<6)
#define EMPTY_BUF (1<<7)
#define CHANNEL_FREE (1<<8)
#define PACKET_RECEIVED (1<<9)
#define PACKET_LAST (1<<10)

// generic errors
#define NULL_POINTER (-1); 

// PROTOCOLS
#define PROTOCOLS (256)
#define P_NONE  (0x0)
#define P_DB    (0x1)
#define P_ARP   (0x2)
#define P_DHCP  (0x3)

// 1<<14 / num of units per packet
#define MAX_ITERATIONS (357)

// Reserved Addresses
#define LOCAL_BROADCAST (0x3fff)

// Hardware
#define MAX_STORED_PACKETS (16)
#define PACKET_SIZE (250)
#define MAX_NEIGHBOURS (256)
#define HEADER_SIZE (sizeof(packed_header))
#define PAYLOAD_SIZE (PACKET_SIZE - HEADER_SIZE)

#define RESERVED_ADDRESSES 2 
#define ADDRESS_BITS 14
#define MAX_TABLE_SIZE (1 << ADDRESS_BITS)
#define TABLE_SIZE __table_size.size

#ifdef __cplusplus
extern "C" {
#endif

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
    DEFCHANNEL = 0
};

typedef struct __attribute__((packed)){
    unsigned hcost : 2;
    unsigned haddress : 6;
    unsigned lcost : 2;
    unsigned hnextHop : 6;
    byte cost;
    byte laddress;
    byte lnextHop;
} unit;

typedef struct __attribute__((packed)){
    unsigned short int UPDATE_WHEN_ADD : 1;
    unsigned short int REMOVE_WITH_ADDRESS : 1;
    unsigned short int REMOVE_WITH_NEXTHOP : 1;
} flags;

typedef struct __attribute__((packed)){
    unsigned short int size: ADDRESS_BITS;
} size;

typedef struct __attribute__((packed)){
    unsigned short address: ADDRESS_BITS;
} addr;

#ifdef __cplusplus
}
#endif
#endif