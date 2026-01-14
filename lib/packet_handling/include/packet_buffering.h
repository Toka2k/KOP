#ifndef ___PACKET_BUFFERING___
#define ___PACKET_BUFFERING___

#include <definitions.h>

#ifdef __cplusplus
extern "C" {
#endif

extern buf_head received;
extern buf_head to_send;

void enqueue(buf_head* bh, packet* p);
void dequeue(buf_head* bh);

#ifdef __cplusplus
}
#endif
#endif