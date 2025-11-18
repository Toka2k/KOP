#include "hardware.h"
#include "packet_buffering.h"

static packet received_buf[MAX_STORED_PACKETS] = {0};
static packet to_send_buf[MAX_STORED_PACKETS] = {0};

buf_head received = {&received_buf, 0, 0};
buf_head to_send = {&to_send_buf, 0, 0};

void enqueue(buf_head* bh, packet p){
    *(bh->buf)[bh->index] = p;
    bh->count = (bh->count + 1) % MAX_STORED_PACKETS;
    
    return;
}

void dequeue(buf_head* bh){
    *(bh->buf)[bh->index] = (packet){0};
    bh->index = (bh->index + 1) % MAX_STORED_PACKETS;
    if (bh->count != 0){
        bh->count--;
    } else {
        bh->index = 0;
    }
    return;
}