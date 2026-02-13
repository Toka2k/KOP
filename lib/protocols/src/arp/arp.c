#include <arp/arp.h>
#include <address_table.h>
#include <driver-al.h>
#include <packet_handling.h>


int ECHO_REQ(addr address){
    unpacked_header uh = {address.address, __my_address.address, 0, 0, 1, P_ARP, 0};
    packed_header ph = PACK_HEADER(uh);

    byte data[1] = {0};

    packet p = packet_init(ph, data);
    unsigned short hmac = HASH_PH(p.h);
    p.h.hmac[0] = (hmac & 0xff00) >> 8;
    p.h.hmac[1] = hmac & 0xff;

    xQueueSend(to_send_queue, &p, portMAX_DELAY);
    return SUCCESS; 
}

int ECHO_REPLY(packet* p){
    unpacked_header uh = UNPACK_HEADER(p->h);
    uh.mac_d = uh.mac_s;
    uh.mac_s = __my_address.address;

    packed_header ph = PACK_HEADER(uh);
    byte data[1] = {1};
    *p = packet_init(ph, data);

    xQueueSend(to_send_queue, p, portMAX_DELAY);

    return SUCCESS;
}

int ARP(packet* p){
    if(p->h.length != 1){
        return INVALID_LENGTH;
    }
    if(p->data[0] == 0){
        return ECHO_REPLY(p);
    } else {
        return INVALID_PAYLOAD;
    }
}