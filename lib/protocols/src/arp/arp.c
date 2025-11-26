#include <arp/arp.h>
#include <packet_buffering.h>

int ECHO_REQ(addr address){
    unpacked_header uh = {address.address, __my_address.address, 0, 0, 1, P_ARP, 0};
    packed_header ph = PACK_HEADER(uh);

    byte data[1] = {0};

    packet p = packet_init(ph, data);
    enqueue(&to_send, &p);
    return SUCCESS; 
}

int ECHO_REPLY(packet* p){
    unpacked_header uh = UNPACK_HEADER(p->h);
    uh.mac_d = uh.mac_s;
    uh.mac_s = __my_address.address;

    p->h = PACK_HEADER(uh);
    *p = packet_init(p->h, p->data);

    enqueue(&to_send, p);

    return SUCCESS;
}

int ARP(packet* p){
    if(p->h.length != 1){
        return INVALID_LENGTH;
    }
    if(p->data[0] == 0){
        ECHO_REPLY(p);
    } else {
        return INVALID_PAYLOAD;
    }
}