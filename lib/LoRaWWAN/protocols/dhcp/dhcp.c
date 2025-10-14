#include "dhcp.h"

//////////////////
//  PROTOCOLS   //
//////////////////

int DHCP_SYNACK(){

    return 0;
}

int DHCP_ACK(){

    return 0;
}

int DHCP_LEASE(){
    short int a = 0;

    unpacked_header uh = {~0,~0,~0,~0, 0};
    packed_header ph = PACK_HEADER(uh);
    
    ph.length = 3;
    byte payload[ph.length] = {0};
    payload[0] = 1;
    *(short *)(payload[1]) = a;

    packet p = packet_init(ph, payload);
    //send(p)

    return 0;
}

int DHCP(byte* data, byte length){
    if(length == 0){
        return DHCP_LEASE();
    }
    return 0;
}
