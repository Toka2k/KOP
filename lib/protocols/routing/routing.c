#include "routing.h"

int ROUTING(packed_header ph, byte* data, byte length){
    unpacked_header received_uh = UNPACK_HEADER(ph);
    addr net_d = {received_uh.net_d};

    unit node = find_unit(net_d);
    if((node.haddress << 8 | node.laddress) == 0 || (node.hcost << 10 | node.cost << 2 | node.lcost) == 0 || (node.hnextHop << 8 | node.lnextHop) == 0){
        return ERROR;
    }

    unpacked_header send_uh = received_uh;
    send_uh.mac_s = __my_address.address;
    send_uh.mac_d = (node.hnextHop << 8 | node.lnextHop);

    packed_header send_ph = PACK_HEADER(send_uh);
    packet p = packet_init(ph, data);

    int state = send_packet(p);
    if (state != RADIOLIB_ERR_NONE){
        return state;
    }

    return SUCCESS;
}