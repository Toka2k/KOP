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
#define NOT_NEIGHBOUR (1<<5)

// generic errors
#define NULL_POINTER -1; 

// PROTOCOLS
#define P_ARP   (0x1)
#define P_DB    (0x2)
#define P_DHCP  (0x3)
#endif