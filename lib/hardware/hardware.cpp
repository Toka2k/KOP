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

// First set of magic numbers, is for hosts
// Second set of magic numbers, is for routers
byte secret[2][SECRET_COUNT] = {{19},{11}};

// created multiple hash functions because of the initiallization problems with
// packet headers.
unsigned short HASH_PH(packed_header ph){
    unsigned short hash = 0;

    int i = 0;
    for(i = 0; i < 7; i++){
        hash = (hash + ph.addresses[i]) * secret[(routers[(ph.addresses[1] & 0x3) << 12 | ph.addresses[2] << 4 | (ph.addresses[3] & 0xf0) / 32] & (1 << (ph.addresses[1] & 0x3) << 12 | ph.addresses[2] << 4 | (ph.addresses[3] & 0xf0) % 8)) >> (ph.addresses[1] & 0x3) << 12 | ph.addresses[2] << 4 | (ph.addresses[3] & 0xf0) % 8][i % SECRET_COUNT];
    }

    hash = (hash + ph.length) * secret[(routers[(ph.addresses[1] & 0x3) << 12 | ph.addresses[2] << 4 | (ph.addresses[3] & 0xf0) / 32] & (1 << (ph.addresses[1] & 0x3) << 12 | ph.addresses[2] << 4 | (ph.addresses[3] & 0xf0) % 8)) >> (ph.addresses[1] & 0x3) << 12 | ph.addresses[2] << 4 | (ph.addresses[3] & 0xf0) % 8][i % SECRET_COUNT]; ++i;
    hash = (hash + ph.protocol_id) * secret[(routers[(ph.addresses[1] & 0x3) << 12 | ph.addresses[2] << 4 | (ph.addresses[3] & 0xf0) / 32] & (1 << (ph.addresses[1] & 0x3) << 12 | ph.addresses[2] << 4 | (ph.addresses[3] & 0xf0) % 8)) >> (ph.addresses[1] & 0x3) << 12 | ph.addresses[2] << 4 | (ph.addresses[3] & 0xf0) % 8][i % SECRET_COUNT]; ++i;
    hash = (hash + ph.seqnum) * secret[(routers[(ph.addresses[1] & 0x3) << 12 | ph.addresses[2] << 4 | (ph.addresses[3] & 0xf0) / 32] & (1 << (ph.addresses[1] & 0x3) << 12 | ph.addresses[2] << 4 | (ph.addresses[3] & 0xf0) % 8)) >> (ph.addresses[1] & 0x3) << 12 | ph.addresses[2] << 4 | (ph.addresses[3] & 0xf0) % 8][i % SECRET_COUNT]; ++i;

    return hash;
}

unsigned short HASH_UH(unpacked_header uh){
    unsigned short hash = 0;
    int i = 0;
    hash = (hash + uh.mac_d) * secret[(routers[uh.mac_s / 32] & (1 << uh.mac_s % 32)) >> uh.mac_s % 32][i % SECRET_COUNT]; ++i;
    hash = (hash + uh.mac_s) * secret[(routers[uh.mac_s / 32] & (1 << uh.mac_s % 32)) >> uh.mac_s % 32][i % SECRET_COUNT]; ++i;
    hash = (hash + uh.net_d) * secret[(routers[uh.mac_s / 32] & (1 << uh.mac_s % 32)) >> uh.mac_s % 32][i % SECRET_COUNT]; ++i;
    hash = (hash + uh.net_s) * secret[(routers[uh.mac_s / 32] & (1 << uh.mac_s % 32)) >> uh.mac_s % 32][i % SECRET_COUNT]; ++i;
    hash = (hash + uh.length) * secret[(routers[uh.mac_s / 32] & (1 << uh.mac_s % 32)) >> uh.mac_s % 32][i % SECRET_COUNT]; ++i;
    hash = (hash + uh.protocol_id) * secret[(routers[uh.mac_s / 32] & (1 << uh.mac_s % 32)) >> uh.mac_s % 32][i % SECRET_COUNT]; ++i;
    hash = (hash + uh.seqnum) * secret[(routers[uh.mac_s / 32] & (1 << uh.mac_s % 32)) >> uh.mac_s % 32][i % SECRET_COUNT]; ++i;

    return hash;
}

void OnReceive(void){
    hw_flags = 0;
    packed_header ph = {0};
    int state = radio.readData((byte*)&ph, sizeof(packed_header));
    if (state != RADIOLIB_ERR_NONE){
        hw_flags |= ERROR;
        radio.finishReceive();
        radio.startReceive();
        return;
    }

    unpacked_header uh = UNPACK_HEADER(ph);
    if (uh.mac_d != 0x3fff || uh.mac_d != __my_address.address){
        radio.finishReceive();
        radio.startReceive();
        return;
    } 

    //compare hmac
    if (*(unsigned short*)ph.hmac != HASH_PH(ph)){
        hw_flags |= INVALID_HASH; 
        radio.finishReceive();
        radio.startReceive();
        return;
    }

    //compare seqnum;
    int i = 0;
    for(; neighbours[i].address != uh.mac_s && i < MAX_NEIGHBOURS; i++){}
    if (i == MAX_NEIGHBOURS){
        hw_flags |= NOT_NEIGHBOUR;
        radio.finishReceive();
        radio.startReceive();
        return; 
    }

    if (neighbour_seqnum[i] == ph.seqnum){
        neighbour_seqnum[i]++;
    } else {
        hw_flags |= INVALID_SEQNUM;
        radio.finishReceive();
        radio.startReceive();
        return;
    }

    byte data[ph.length];
    state = radio.readData(data, ph.length);
 
    if(uh.net_d != __my_address.address){
        ROUTING(ph, data, ph.length);
    } else {
        protocols[ph.protocol_id](ph, data, ph.length);
    }

    radio.startReceive();
    return;
}

int send_packet(packet p){
    hw_flags = 0;

    //increment seqnum;
    int i = 0;
    for(; neighbours[i].address != ((p.h.addresses[0] << 6) | (p.h.addresses[1] & 0xfc) >> 2) && i < MAX_NEIGHBOURS; i++){}
    if (i == MAX_NEIGHBOURS){
        return NOT_NEIGHBOUR;
    }

    seqnum[i]++;
    p.h.seqnum = seqnum[i];

    //calculate HMAC
    *(unsigned short*)p.h.hmac = HASH_PH(p.h);
    
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

    if (_payload != NULL){
        memcpy(p.data, _payload, ph.length);
    }

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

int ROUTING(packed_header ph, byte* data, byte length){
    unpacked_header received_uh = UNPACK_HEADER(ph);
    addr net_d = {received_uh.net_d};

    unit node = find_unit(net_d);
    int state = 0;
    if ((state = check(node)) == SUCCESS){
        return state;
    }

    unpacked_header send_uh = received_uh;
    send_uh.mac_s = __my_address.address;
    if ((node.hnextHop << 8 | node.lnextHop) != __my_address.address){
        send_uh.mac_d = (node.hnextHop << 8 | node.lnextHop);
    } else {
        send_uh.mac_d = send_uh.net_d;
    }

    packed_header send_ph = PACK_HEADER(send_uh);
    packet p = packet_init(ph, data);

    state = send_packet(p);
    if (state != RADIOLIB_ERR_NONE){
        return state;
    }

    return SUCCESS;
}