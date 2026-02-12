#include <dhcp/dhcp.h>
#include <packet_handling.h>
#include <address_table.h>

static int req_random = 0;
static int off_random = 0;
TaskHandle_t* dhcp_lease_task = NULL;

short int get_unused_address(short int address){
    addr a = {address};
    unit result = find_unit(a);
    if(_memcmp(&result, &null, sizeof(unit)) == 0){
        return address;
    } 

    for(short int i = __highest_address.address; i < MAX_TABLE_SIZE; i++){
        __highest_address.address = i;
        result = find_unit(__highest_address);
        if(_memcmp(&result, &null, sizeof(unit)) == 0){
            return __highest_address.address;
        }
    }
}

int DHCP_REQ(){
    req_random = random() & 0xff;
    packed_header ph = {{0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff}, 2, P_DHCP, 0};
    
    byte payload[] = {req_random, 0};

    packet p = packet_init(ph, payload);

    xQueueSend(to_send_queue, &p, portMAX_DELAY);
    return SUCCESS;
}

int DHCP_OFFER(byte* data){
    short int a = get_unused_address(__highest_address.address);
    // PING THE ADDRESS, if we get response, we choose different address

    unpacked_header uh = {~0, __my_address.address, ~0, __my_address.address, 0};
    packed_header ph = PACK_HEADER(uh);
    
    ph.length = 4;
    ph.protocol_id = P_DHCP;
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
    
    xQueueSend(to_send_queue, &p, portMAX_DELAY);

    free(payload);

    return SUCCESS;
}

int DHCP_ACK(packet* p){
    if (p->h.length == 0){
        return INVALID_LENGTH;
    }
    // check for more errors
    unpacked_header received_off = UNPACK_HEADER(p->h);
    unpacked_header send = {0};

    send.mac_d = received_off.mac_s;
    send.net_d = received_off.net_s;

    send.mac_s = (*(p->data + 2) << 8 | *(p->data + 3));
    send.net_s = (*(p->data + 2) << 8 | *(p->data + 3));

    __my_address.address = send.mac_s;
    
    send.length = 2;
    send.protocol_id = P_DHCP;

    packed_header send_packed = PACK_HEADER(send);
    byte send_data[] = {req_random, 2};

    *p = packet_init(send_packed, send_data);
    
    xQueueSend(to_send_queue, p, portMAX_DELAY);

    return SUCCESS;
}

int DHCP_FIN(packet* p){
    unpacked_header received_ack = UNPACK_HEADER(p->h);
    unpacked_header send_fin = {received_ack.mac_s, __my_address.address, received_ack.net_s, __my_address.address, 0};

    // add to DB
    addr leased_address = {received_ack.mac_s};
    add_unit(initialize_unit(leased_address.address, 1, leased_address.address));
    if (HASH_PH(p->h) != ((p->h.hmac[0] << 8) + p->h.hmac[1])){
        routers[leased_address.address / 8] |= 1 << (leased_address.address % 8);
    } else {
        routers[leased_address.address / 8] &= ~(1 << (leased_address.address % 8));
    }

    send_fin.length = 2;
    send_fin.protocol_id = P_DHCP;

    packed_header send = PACK_HEADER(send_fin);
    byte _data[] = {off_random, 3};
    
    *p = packet_init(send, _data);

    xQueueSend(to_send_queue, p, portMAX_DELAY);
    xTaskCreatePinnedToCore(DHCP_LEASE_TASK, "dhcp lease task", 1028, (void*) &p->h, 2, dhcp_lease_task, 0);

    off_random = 0;

    return SUCCESS;
}

void DHCP_LEASE_TASK(void* pvParameters){
    unpacked_header uh = UNPACK_HEADER(*(packed_header*)pvParameters);
    addr dhcp_leased = {uh.mac_s};

    vTaskDelay(pdMS_TO_TICKS(30000));
    FLAGS.REMOVE_WITH_ADDRESS = 1;
    remove_unit(initialize_unit(dhcp_leased.address, 0, dhcp_leased.address));
    if(__my_address.address == dhcp_leased.address){
        __my_address.address = 0;
    }
}

int DHCP_ACC(packed_header ph){
    clear_table();

    xTaskCreatePinnedToCore(DHCP_LEASE_TASK, "dhcp lease task", 1028, (void*) &ph, 2, dhcp_lease_task, 0);

    add_unit(initialize_unit(__my_address.address, 0, __my_address.address));

    req_random = 0;

    return SUCCESS;    
}

int DHCP_DROP(){
    if(req_random != 0){
        __my_address.address = 0;
        req_random = 0;
    } else if(off_random != 0){
        off_random = 0;
    }
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
    xQueueSend(to_send_queue, &p, portMAX_DELAY);

    DHCP_DROP();

    return SUCCESS;
}

int DHCP(packet* p){
    if (p->data == NULL){
        return NULL_POINTER;
    }

    if (p->h.length < 1){
        return INVALID_LENGTH;
    }

    // We are proccessing one request or we are not processing anything.
    if (req_random && req_random != *p->data){
        return ERROR;
    }
    if (off_random && off_random != *p->data){
        return ERROR;
    }
    if (off_random == 0){
        off_random = *p->data;
    }

    if (*(p->data + 1) == 0){
        return DHCP_OFFER(p->data);
    } else if (*(p->data + 1) == 1){
        return DHCP_ACK(p);
    } else if (*(p->data + 1) == 2){
        return DHCP_FIN(p);
    } else if (*(p->data + 1) == 3){
        return DHCP_ACC(p->h);
    } else if (*(p->data + 1) == 4){
        return DHCP_DROP();
    }

    return SUCCESS;
}
