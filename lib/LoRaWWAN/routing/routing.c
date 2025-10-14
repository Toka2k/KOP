#include "routing.h"
#include "../protocols/protocols.h"
int (*protocols[256])(byte[], byte) = {
    DHCP,
};

byte secret[SECRET_COUNT] = {19};

// created multiple hash functions because of the initiallization problems with
// packet headers.
unsigned short HASH_PH(packed_header ph){
    unsigned short hash = 0;

    int i = 0;
    for(i = 0; i < 7; i++){
        hash = (hash + ph.addresses[i]) * secret[i % SECRET_COUNT];
    }

    hash = (hash + ph.length) * secret[i % SECRET_COUNT]; ++i;
    hash = (hash + ph.protocol_id) * secret[i % SECRET_COUNT]; ++i;
    hash = (hash + ph.seqnum) * secret[i % SECRET_COUNT]; ++i;

    return hash;
}

unsigned short HASH_UH(unpacked_header uh){
    unsigned short hash = 0;
    int i = 0;
    hash = (hash + uh.mac_d) * secret[i % SECRET_COUNT]; ++i;
    hash = (hash + uh.mac_s) * secret[i % SECRET_COUNT]; ++i;
    hash = (hash + uh.net_d) * secret[i % SECRET_COUNT]; ++i;
    hash = (hash + uh.net_s) * secret[i % SECRET_COUNT]; ++i;
    hash = (hash + uh.length) * secret[i % SECRET_COUNT]; ++i;
    hash = (hash + uh.protocol_id) * secret[i % SECRET_COUNT]; ++i;
    hash = (hash + uh.seqnum) * secret[i % SECRET_COUNT]; ++i;

    return hash;
}

int ask_for_address(){
    unpacked_header uh = {0x3fff,0x3fff,0x3fff,0x3fff, 0};
    packed_header ph = PACK_HEADER(uh);
    
    ph.length = 0;
    byte payload[] = {0};

    packet p = packet_init(ph, payload);
    // send(p);

    return 0;
}

int get_neighbours(){
    return 0;
}
