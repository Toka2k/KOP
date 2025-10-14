#include "routing.h"
#include "../protocols/protocols.h"

int (*protocols[256])(byte[], byte) = {
    DHCP, NULL
};

int ask_for_address(){
    // replace with dhcp request function that sets devices address called __my_address.
    unpacked_header uh = {0x3fff,0x3fff,0x3fff,0x3fff, 0};
    packed_header ph = PACK_HEADER(uh);
    
    ph.length = 0;
    byte payload[] = {0};

    packet p = packet_init(ph, payload);
    sendPacket(p);

    return 0;
}

int get_neighbours(){
    return 0;
}
