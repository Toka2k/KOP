#include "dhcp.h"

static int req_random = 0;
static int off_random = 0;

short int get_unused_address(short int address){
    addr a = {address};
    if(find_unit(a) == 0){
        return address;
    }; 

    for(int i = __highest_address.address; i < MAX_TABLE_SIZE && find_unit(__highest_address) == null; i++){}
}

int DHCP_REQ(){
    req_random = random() & 0xff;
    packed_header ph = {{0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff}, 2, P_DHCP, 0};
    
    byte payload[] = {req_random, 0};

    packet p = packet_init(ph, payload);

    send_packet(p);
    int flags;
    if (flags = get_hw_flags() != SUCCESS){
        return flags;
    }
    return SUCCESS;
}

int DHCP_OFFER(byte* data){
    short int a = get_unused_address(__highest_address.address);
    // PING THE ADDRESS, if we get response, we choose different address

    unpacked_header uh = {~0, __my_address.address, ~0, __my_address.address, 0};
    packed_header ph = PACK_HEADER(uh);
    
    ph.length = 4;
    byte* payload = malloc(ph.length);

    if (payload == NULL){
        return NULL_POINTER;
    }

    // first byte is temp identifier of device requesting
    // second byte is identifier of dhcp message
    payload[0] = off_random;
    payload[1] = 1;
    // dhcp offer address
    payload[2] = (a & 0x3f00) >> 8;
    payload[3] = a & 0xff;

    packet p = packet_init(ph, payload);
    
    send_packet(p);

    free(payload);

    int flags;
    if (flags = get_hw_flags != SUCCESS){
        return flags;
    }

    return SUCCESS;
}

int DHCP_ACK(packed_header ph, byte* data, byte length){
    if (length <= 0){
        return INVALID_LENGTH;
    }
    // check for more errors
    unpacked_header received_off = UNPACK_HEADER(ph);
    unpacked_header send = {0};

    send.mac_d = received_off.mac_s;
    send.net_d = received_off.net_s;

    send.mac_s = (*(data + 1) << 8 | *(data + 2));
    send.net_s = (*(data + 1) << 8 | *(data + 2));

    __my_address.address = send.mac_s;
    
    send.length = 2;
    send.protocol_id = P_DHCP;

    packed_header send_packed = PACK_HEADER(send);
    byte send_data[] = {req_random, 2};

    packet p = packet_init(ph, send_data);
    
    send_packet(p);
    int flags;
    if (flags = get_hw_flags() != SUCCESS){
        return flags;
    }

    return SUCCESS;
}

int DHCP_FIN(packed_header ph, byte* data, byte length){
    unpacked_header received_ack = UNPACK_HEADER(ph);
    unpacked_header send_fin = {received_ack.mac_s, __my_address.address, received_ack.net_s, __my_address.address, 0};

    // add to DB
    addr leased_address = {(*(data + 1) << 8 | *(data + 2))};
    add_unit(initialize_unit(leased_address.address, 1, __my_address.address));
    if (HASH_PH(ph) != *(unsigned short*)ph.hmac){
        routers[leased_address.address / 8] |= 1 << (leased_address.address % 8);
    } else {
        routers[leased_address.address / 8] &= ~(1 << (leased_address.address % 8));
    }

    send_fin.length = 2;
    send_fin.protocol_id = P_DHCP;

    packed_header send = PACK_HEADER(send_fin);
    byte _data[] = {off_random, 3};
    
    packet p = packet_init(send, _data);

    send_packet(p);

    int flags;
    if (flags = get_hw_flags() != SUCCESS){
        return flags;
    }

    off_random = 0;

    return SUCCESS;
}

int DHCP_ACC(packed_header ph){
    clear_table();

    add_unit(initialize_unit(__my_address.address, 0, __my_address.address));

    req_random = 0;

    return SUCCESS;    
}

int DHCP_DENY(){
    unpacked_header uh = {~0, __my_address.address, ~0, __my_address.address, 2, P_DHCP, 0};
    packed_header ph = PACK_HEADER(uh);

    byte data[2] = {0, 4};
    if (off_random){
        data[0] = off_random;
    } else {
        data[0] = req_random;
    }

    off_random = 0;
    req_random = 0;

    packet p = packet_init(ph, data);
    send_packet(p);

    int flags;
    if (flags = get_hw_flags() != SUCCESS){
        return flags;
    }

    return SUCCESS;
}

int DHCP(packed_header ph, byte* data, byte length){
    // Add some flag to prevent multiple dhcp requests to interfere
    // add some checks to see wether we sent dhcp req
    if (data == NULL){
        return NULL_POINTER;
    }

    if (length < 1){
        return INVALID_LENGTH;
    }

    if (off_random != *data || req_random != *data){
        return ERROR;
    }

    if (*(data + 1) == 0){
        return DHCP_OFFER(data);
    } else if (*(data + 1) == 1){
        return DHCP_ACK(ph, data, length);
    } else if (*(data + 1) == 2){
        return DHCP_FIN(ph, data, length);
    } else if (*(data + 1) == 3){
        return DHCP_ACC(ph);
    } else if (*(data + 1) == 4){
        return DHCP_DENY();
    }

    return SUCCESS;
}
