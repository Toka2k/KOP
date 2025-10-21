#include "routing.h"

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

int 