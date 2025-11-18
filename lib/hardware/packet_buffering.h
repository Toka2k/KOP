#ifndef ___PACKET_BUFFERING___
#define ___PACKET_BUFFERING___

#include "hardware.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct __attribute__((packed)){
    packet* buf[MAX_STORED_PACKETS];
    byte count;
    byte index;
} buf_head;

extern buf_head received;
extern buf_head to_send;

void enqueue(buf_head* bh, packet p);
void dequeue(buf_head* bh);

#ifdef __cplusplus
}
#endif
#endif