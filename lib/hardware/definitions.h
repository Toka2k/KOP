#ifndef ___DEFINITIONS___
#define ___DEFINITIONS___

#define CS 1
#define IRQ 2
#define RST 3
#define GPIO 4

#define ERROR (1<<0)
#define INVALID_HASH (1<<1)
#define INVALID_SEQNUM (1<<2)
#define INVALID_LENGTH (1<<3)
#define NOT_NEIGHBOUR (1<<4)

#define P_ROUTING   (0x0)
#define P_ARP   (0x1)
#define P_DB    (0x2)
#define P_DHCP  (0x3)
#endif