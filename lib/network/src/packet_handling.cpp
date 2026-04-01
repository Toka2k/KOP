#include <packet_handling.h>
#include <updates.h>
#include <address_table.h>
#include <driver-al.h>

static int hw_flags = 0;

byte* my_seqnum, * neighbour_seqnum;

addr* neighbours;
unsigned short neighbours_size = 0;

// First set of magic numbers, is for hosts
// Second set of magic numbers, is for routers
byte secret[2][SECRET_COUNT] = {{19},{11}};

void init_zero(void* ptr, int ptr_len, int type_size){
    for(int i = 0; i < ptr_len * type_size; i++){
        *(((byte*)ptr) + i) = 0;
    }
}

unsigned short HASH_PH(packed_header ph){
    unsigned short hash = 0;

    int i = 0;
    for (i = 0; i < 7; i++){
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

int payload_hash(byte* data, byte length){
    int h = 0;
    for (int i = 0; i < length; i++){
        h = (h + (int)data[i]) * 47;
    }
    return h;
}

int get_hw_flags(){
    return hw_flags;
}

unsigned short find_neighbour(addr neighbour){
    xSemaphoreTake(neighbour_semaphore, portMAX_DELAY);
    for(unsigned short i = 0; i < neighbours_size; i++){
        if(neighbour.address == neighbours[i].address){
            xSemaphoreGive(neighbour_semaphore);
            return i;
        }
    }

    xSemaphoreGive(neighbour_semaphore);
    return neighbours_size;
}

void resize_neighbour(){
    void* tmp = (addr*)realloc(neighbours, sizeof(addr) * neighbours_size);
    if (tmp != NULL){
        neighbours = (addr*)tmp;
    }

    tmp = (byte*)realloc(my_seqnum, sizeof(byte) * neighbours_size);
    if (tmp != NULL){
        my_seqnum = (byte*)tmp;
    }

    tmp = (byte*)realloc(neighbour_seqnum, sizeof(byte) * neighbours_size);
    if (tmp != NULL){
        neighbour_seqnum = (byte*)tmp;
    }
}

void add_neighbour(addr neighbour){
    unsigned short index = find_neighbour(neighbour);
    xSemaphoreTake(neighbour_semaphore, portMAX_DELAY);
    if ((index == neighbours_size || neighbours_size == 0) && neighbour.address != __my_address.address){
        ++neighbours_size;
        resize_neighbour();
        
        neighbours[neighbours_size - 1] = neighbour;
        my_seqnum[neighbours_size - 1] = 0;
        neighbour_seqnum[neighbours_size - 1] = 0;
    }
    xSemaphoreGive(neighbour_semaphore);
}

void remove_neighbour(addr neighbour){
    unsigned short i = find_neighbour(neighbour);
    xSemaphoreTake(neighbour_semaphore, portMAX_DELAY);
    if(i == neighbours_size || neighbours_size == 0){
        xSemaphoreGive(neighbour_semaphore);
        return;
    }

    neighbour_seqnum[i] = neighbour_seqnum[neighbours_size - 1];
    my_seqnum[i] = my_seqnum[neighbours_size - 1];
    neighbours[i] = neighbours[neighbours_size - 1];

    --neighbours_size;
    if (neighbours_size == 0) { 
        xSemaphoreGive(neighbour_semaphore);
        return;
    }

    resize_neighbour();
    xSemaphoreGive(neighbour_semaphore);
}

byte track_seqnums(packed_header ph){
    unpacked_header uh = UNPACK_HEADER(ph);
    unsigned short i;
    addr address;
    if(uh.mac_d == LOCAL_BROADCAST){
        address.address = uh.mac_s;
        add_neighbour(address);
        return SUCCESS;
    }
    if(__my_address.address == uh.mac_s){
        if(uh.mac_d == LOCAL_BROADCAST || uh.mac_d == 0){
            return SUCCESS;
        }
        address.address = uh.mac_d;

        add_neighbour(address);
        i = find_neighbour(address);

        xSemaphoreTake(neighbour_semaphore, portMAX_DELAY);
        byte seq = ++my_seqnum[i];
        xSemaphoreGive(neighbour_semaphore);
        return seq;
    } else if (__my_address.address == uh.mac_d){
        if(uh.mac_s == LOCAL_BROADCAST || uh.mac_s == 0){
            return SUCCESS;
        }
        address.address = uh.mac_s;

        add_neighbour(address);

        i = find_neighbour(address);
        xSemaphoreTake(neighbour_semaphore, portMAX_DELAY);
        neighbour_seqnum[i]++;
        xSemaphoreGive(neighbour_semaphore);

        mark_route_refreshed(address);
        
        xSemaphoreTake(neighbour_semaphore, portMAX_DELAY);
        if (neighbour_seqnum[i] == ph.seqnum){
            xSemaphoreGive(neighbour_semaphore);
            return SUCCESS;
        } else {
            xSemaphoreGive(neighbour_semaphore);
            remove_neighbour(address);
            return INVALID_SEQNUM;
        }
    }

    return INVALID_SEQNUM;
}

//
//      CORE 2
//

void Receive(void* pvParameters){
    packet p;
    for(;;){
        hw_flags = 0;

        xSemaphoreTake(rxDoneSemaphore, portMAX_DELAY);
        xQueueReceive(received_queue, &p, portMAX_DELAY);
        xSemaphoreTake(radio_mutex, portMAX_DELAY);

        unpacked_header uh = UNPACK_HEADER(p.h);

        Serial.println("\nRECEIVED");
        Serial.printf("mac_d: 0x%02X \tmac_s: 0x%02X\nnet_d: 0x%02X \tnet_s: 0x%02X\n", uh.mac_d, uh.mac_s, uh.net_d, uh.net_s);
        Serial.printf("length: %d, protocol_id: %d, hmac: 0x%02X\n", uh.length, uh.protocol_id, p.h.hmac[0] <<  8 | p.h.hmac[1]); 
        for (int i = 0; i < p.h.length; i++){
            Serial.printf("0x%02X ", p.data[i]);
        }
        Serial.println("\n==============");

        //compare hmac
        if (((p.h.hmac[0] << 8) + p.h.hmac[1]) != HASH_PH(p.h)){
            hw_flags |= INVALID_HASH; 
            xSemaphoreGive(radio_mutex);
            continue;
        }
        
        // tracks neighbours and their seqnums
        if (track_seqnums(p.h) != SUCCESS){
            xSemaphoreGive(radio_mutex);
            continue;
        }
        
        unit res = find_unit((addr){uh.mac_s});
        if (_memcmp(&res, &null, sizeof(unit)) == 0 || UNIT_COST(res) <= 0xfff){
            FLAGS.UPDATE_WHEN_ADD = 1;
            add_unit(initialize_unit(uh.mac_s, 1, uh.mac_s));
        }

        // if its not for me or local broadcast, we drop the packet
        if (uh.mac_d != LOCAL_BROADCAST && uh.mac_d != __my_address.address){
            xSemaphoreGive(radio_mutex);
            continue;
        }

        xSemaphoreGive(radio_mutex);
        xQueueSend(to_process_queue, &p, portMAX_DELAY);
    }
}

void Transmit(void* pvParameters){
    packet p;

    for (;;){
        hw_flags = 0;

        xQueueReceive(to_send_queue, &p, portMAX_DELAY);
        xSemaphoreTake(radio_mutex, portMAX_DELAY);

        unpacked_header uh = UNPACK_HEADER(p.h);
        
        Serial.println("\nTRANSMITED");
        Serial.printf("mac_d: 0x%02X \tmac_s: 0x%02X\nnet_d: 0x%02X \tnet_s: 0x%02X\n", uh.mac_d, uh.mac_s, uh.net_d, uh.net_s);
        Serial.printf("length: %d, protocol_id: %d, hmac: 0x%02X\n", uh.length, uh.protocol_id, p.h.hmac[0] <<  8 | p.h.hmac[1]); 
        for (int i = 0; i < p.h.length; i++){
            Serial.printf("0x%02X ", p.data[i]);
        }
        Serial.println("\n==============");

        p.h.seqnum = track_seqnums(p.h);

        //calculate HMAC
        unsigned short hmac = HASH_PH(p.h);
        p.h.hmac[0] = (hmac & 0xff00) >> 8;
        p.h.hmac[1] = hmac & 0xff;

        radio_transmit(&p);

        xSemaphoreGive(radio_mutex);
        vTaskDelay(10);
    }
}

void process_packet(void* pvParameters){
    packet p;
    for (;;){
        xQueueReceive(to_process_queue, &p, portMAX_DELAY);
        hw_flags = 0;

        unpacked_header received_uh = UNPACK_HEADER(p.h);
        addr net_d = {received_uh.net_d};

        unit node = find_unit(net_d);
        if ((check(node)) != SUCCESS && received_uh.mac_d != LOCAL_BROADCAST){
            hw_flags |= INVALID_ADDRESS;
            continue;
        }

        unpacked_header send_uh = received_uh;

        short next_hop = (node.hnextHop << 8 | node.lnextHop);
        
        if (next_hop != __my_address.address && (next_hop != 0)){
            send_uh.mac_s = __my_address.address;
            send_uh.mac_d = next_hop;

            packed_header send_ph = PACK_HEADER(send_uh);
            p = packet_init(send_ph, p.data);

            xQueueSend(to_send_queue, &p, portMAX_DELAY);
            hw_flags &= SUCCESS;
        } else {
            // Proccessing packets
            protocols[p.h.protocol_id](&p);
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

//
//      End of CORE 2
//

void init_network(){
    radio_mutex = xSemaphoreCreateBinary();
    xSemaphoreGive(radio_mutex);

    received_queue = xQueueCreate(MAX_STORED_PACKETS, PACKET_SIZE);
    to_process_queue = xQueueCreate(MAX_STORED_PACKETS, PACKET_SIZE);
    to_send_queue = xQueueCreate(MAX_STORED_PACKETS, PACKET_SIZE);

    xTaskCreatePinnedToCore(Transmit, "Transmit task", 16384, NULL, 3, NULL, 1);
    xTaskCreatePinnedToCore(Receive, "Receive task", 16384, NULL, 3, NULL, 1);
    xTaskCreatePinnedToCore(process_packet, "Packet processing task", 16384, NULL, 2, NULL, 0);

    return;
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

int route(addr dest, byte length, byte protocol_id, byte* data){
    hw_flags = 0;
    unpacked_header uh = {0, __my_address.address, dest.address, __my_address.address, length, protocol_id, 0};

    unit nextHop = find_unit(dest);
    if (_memcmp(&nextHop, &null, sizeof(unit)) == 0 && dest.address != LOCAL_BROADCAST){
        hw_flags |= INVALID_ADDRESS;
        return hw_flags;
    }

    if(dest.address == LOCAL_BROADCAST){
        uh.mac_d = LOCAL_BROADCAST;
        uh.net_d = 0;
    } else{
        uh.mac_d = (nextHop.hnextHop << 8 | nextHop.lnextHop);
    }
    
    packed_header ph = PACK_HEADER(uh);
    
    packet p = packet_init(ph, data);
    xQueueSend(to_send_queue, &p, portMAX_DELAY);
    
    hw_flags = SUCCESS;
    return hw_flags;
}
