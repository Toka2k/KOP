#include "dhcp.h"

static int req_random = 0;
static int off_random = 0;

int DHCP_REQ(){
    req_random = random() &0xff;
    packed_header ph = {{0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff}, 1, P_DHCP, 0};
    
    byte payload[] = {req_random};

    packet p = packet_init(ph, payload);
    int state = sendPacket(p);
    return state;
}

int DHCP_FIN(){

    return 0;
}

int DHCP_ACK(packed_header ph, byte* data, byte length){
    unpacked_header received = UNPACK_HEADER(ph);
    unpacked_header send = {0};

    send.mac_d = received.mac_s;
    send.net_d = received.net_s;

    send.mac_s = (*(data + 1) << 8 | *(data + 2));
    send.net_s = (*(data + 1) << 8 | *(data + 2));

    return 0;
}

int DHCP_OFFER(byte* data){
    // not reusable addresses
    short int a = __highest_address.address++;
    // check wether the address isnt used use a ping beffore sending an offer

    unpacked_header uh = {~0, __my_address.address, ~0, __my_address.address, 0};
    packed_header ph = PACK_HEADER(uh);
    
    ph.length = 3;
    byte* payload = malloc(ph.length);
    // first byte is identifier of dhcp message
    // we use random byte received from the device requesting an address
    // We add N to this byte and that identifies the device
    // and the function it should use
    payload[0] = off_random + 1;
    // dhcp offer address
    payload[1] = (a & 0x3f00) >> 8;
    payload[2] = a & 0xff;

    packet p = packet_init(ph, payload);
    sendPacket(p);

    free(payload);
    return SUCCESS;
}

int DHCP(packed_header ph, byte* data, byte length){
    if(length == 1){
        off_random = *data;
        return DHCP_OFFER(*data);
    }
    if (length > 1 && (off_random + 1) & 0xff == *data){
        return DHCP_ACK(ph, data, length);
    }
    return 0;
}
