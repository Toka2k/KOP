#include <arp/arp.h>
#include <address_table.h>
#include <driver-al.h>
#include <packet_handling.h>


int ECHO_REQ(addr address){
    byte data[1] = {0};

    route(address, 1, P_ARP, data);
    return SUCCESS; 
}

int ECHO_REPLY(packet* p){
    addr address = {(p->h.addresses[5] & 0x3f) << 8 | p->h.addresses[6]};
    byte data[1] = {1};

    route(address, 1, P_ARP, data);

    digitalWrite(2, HIGH);
    vTaskDelay(100);
    digitalWrite(2, LOW);
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