#ifndef ___DEFINITIONS___
#define ___DEFINITIONS___

// PINS
#define CS 1
#define IRQ 2
#define RST 3
#define GPIO 4

// HW_FLAGS
#define SUCCESS 0 
#define ERROR (1)
#define INVALID_HASH (1<<1)
#define INVALID_SEQNUM (1<<2)
#define INVALID_LENGTH (1<<3)
#define INVALID_ADDRESS (1<<4)
#define INVALID_PAYLOAD (1<<5)
#define NOT_NEIGHBOUR (1<<6)
#define EMPTY_BUF (1<<7)

// generic errors
#define NULL_POINTER -1; 

// PROTOCOLS
#define P_   (0x0)
#define P_DB   (0x1)
#define P_ARP    (0x2)
#define P_DHCP  (0x3)

// Hardware
#define MAX_STORED_PACKETS 64
#define MAX_NEIGHBOURS 256
#define PAYLOAD_SIZE (256 - sizeof(packed_header))
#define HEADER_SIZE (sizeof(packed_header))
#define PACKET_SIZE (sizeof(packed_header) + ph.len)

#define RESERVED_ADDRESSES 2 
#define ADDRESS_BITS 14
#define MAX_TABLE_SIZE (1 << ADDRESS_BITS)

#endif