#ifndef ___PACKET_BUFFERING___
#define ___PACKET_BUFFERING___

#include "hardware.h"

typedef struct __attribute__((packed)){
    packet* buf[MAX_STORED_PACKETS];
    byte count;
    byte index;
} buf_head;

buf_head received;
buf_head to_send;


void enqueue(buf_head bh, packet p);
void dequeue(buf_head bh);
#endif

// one core manages sending/receiving packets, the other should be proccessing packets

// Packet Buffer:
// - create queues for packets to be sent and received packets
// - modify functions to work with the queue instead of using send_packet() directly
// - get_packet(struct)