#include "hardware.h"
#include "../RadioLib/src/modules/LLCC68/LLCC68.h"

// DONT FORGET TO CHANGE THESE NUMBERS FOR PINS
Module m = Module(CS, IRQ, RST, GPIO);
LLCC68 radio = LLCC68(&m);

int hw_flags = 0;
double __channels[] = {8670E5, 8672E5, 8674E5, 8676E5, 8678E5, 8680E5};
static byte seqnum[MAX_NEIGHBOURS] = {0};
static byte neighbour_seqnum[MAX_NEIGHBOURS] = {0};
static addr neighbours[MAX_NEIGHBOURS] = {0};

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

void OnReceive(void){
    hw_flags = 0;
    packed_header ph = {0};
    int state = radio.readData((byte*)&ph, sizeof(packed_header));
    if (state != RADIOLIB_ERR_NONE){
        hw_flags |= ERROR;
        radio.startReceive();
        return;
    }

    unpacked_header uh = UNPACK_HEADER(ph);
    if (uh.mac_d != 0xcfff || uh.mac_d != __my_address.address){
        radio.finishReceive();
    } 

    //compare hmac
    if (*(unsigned short*)ph.hmac != HASH_PH(ph)){
        hw_flags |= INVALID_HASH; 
        radio.finishReceive();
        return;
    }

    //compare seqnum;
    int i = 0;
    for(; neighbours[i].address != uh.mac_s && i < MAX_NEIGHBOURS; i++){}
    if (i == MAX_NEIGHBOURS){
        hw_flags |= NOT_NEIGHBOUR;
        return;        
    }

    if (neighbour_seqnum[i] == ph.seqnum){
        neighbour_seqnum[i]++;
    } else {
        hw_flags |= INVALID_SEQNUM;
        radio.finishReceive();
        return;
    }

    byte data[ph.length];
    state = radio.readData(data, ph.length);
    protocols[ph.protocol_id](ph, data, ph.length);

    radio.startReceive();
    return;
}

int sendPacket(packet p){
    hw_flags = 0;

    //increment seqnum;
    int i = 0;
    for(; neighbours[i].address != ((p.h.addresses[0] << 6) | (p.h.addresses[1] & 0xfc) >> 2) && i < MAX_NEIGHBOURS; i++){}
    if (i == MAX_NEIGHBOURS){
        return NOT_NEIGHBOUR;
    }

    seqnum[i]++;
    p.h.seqnum = seqnum[i];

    
    //scaning
    while (radio.scanChannel() != RADIOLIB_CHANNEL_FREE){
        sleep(random() % 11);
    }

    int state = radio.transmit((char *)&p);
    if (state != RADIOLIB_ERR_NONE){
        return state;
    }
    return SUCCESS;
}

packet packet_init(packed_header ph, byte* _payload){
    packet p = {ph, 0};

    memcpy(p.data, _payload, ph.length);

    return p;
}

packed_header PACK_HEADER(unpacked_header uh){
    packed_header ph;
    ph.addresses[0] = (uh.mac_d & 0x3fc0) >> 6 ;
    ph.addresses[1] = (uh.mac_d & 0x003f) << 2 | (uh.mac_s & 0x3000) >> 12;
    ph.addresses[2] = (uh.mac_s & 0xff0) >> 4;
    ph.addresses[3] = (uh.mac_s & 0xf) << 4 | (uh.net_d & 0x3c00) >> 10;
    ph.addresses[4] = (uh.net_d & 0x3fc) >> 2;
    ph.addresses[5] = (uh.net_d & 0x3) << 6 | (uh.net_s & 0x3f00) >> 8;
    ph.addresses[6] = (uh.net_s & 0xff);
    ph.length = uh.length;
    ph.protocol_id = uh.protocol_id;
    ph.seqnum = 0;
    unsigned short hash = HASH_PH(ph);
    ph.hmac[0] = (hash & 0xff00) >> 8;
    ph.hmac[1] = hash & 0xff;

    return ph; 
}

unpacked_header UNPACK_HEADER(packed_header ph){
    unpacked_header uh;
    uh.mac_d = ph.addresses[0] << 6 | (ph.addresses[1] & 0xfc) >> 2;
    uh.mac_s = (ph.addresses[1] & 0x3) << 12 | ph.addresses[2] << 4 | (ph.addresses[3] & 0xf0) >> 4;
    uh.net_d = (ph.addresses[3] & 0xf) << 10 | ph.addresses[4] << 2 | (ph.addresses[5] & 0xc0) >> 6;
    uh.net_s = (ph.addresses[5] & 0x3f) << 8 | ph.addresses[6];
    uh.length = ph.length;
    uh.protocol_id = ph.protocol_id;
    uh.seqnum = ph.seqnum;
    uh.hmac[0] = ph.hmac[0];
    uh.hmac[1] = ph.hmac[1];

    return uh;
}