#include "routing.h"

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
    return uh;
}
