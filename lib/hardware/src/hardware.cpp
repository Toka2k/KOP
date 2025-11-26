#include <hardware.h>
#include <packet_buffering.h>
#include "../../RadioLib/src/modules/LLCC68/LLCC68.h"

Module m = Module(LORA_NSS, LORA_DIO1, LORA_RST, LORA_BUSY);
LLCC68 radio = LLCC68(&m);

static int hw_flags = 0;
double __channels[] = {8680E5};
static byte my_seqnums[MAX_NEIGHBOURS] = {0};
static byte neighbour_seqnums[MAX_NEIGHBOURS] = {0};
addr neighbours[MAX_NEIGHBOURS] = {0};
byte neighbours_size = 0;

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

int get_hw_flags(){
    return hw_flags;
}

int cmp_addr(const void* a, const void* b){
    return ((*(addr*)a).address - (*(addr*)b).address);
}

int cmp_index(const void *a, const void *b){
    return neighbours[*(const int *)a].address - neighbours[*(const int *)b].address;
}

addr find_addr(addr address){
    int low = 0, high = neighbours_size - 1;

    while (low <= high) {
        int mid = (low + high) / 2;
        if (neighbours[mid].address == address.address)
            return neighbours[mid];
        else if (neighbours[mid].address < address.address){
            low = mid + 1;
        }
        else{
            high = mid - 1;
        }
    }

    return (addr){0};
}

void sort_neighbours(){
    int temp[neighbours_size] = {0};
    for (int i = 0; i < neighbours_size; i++){
        temp[i] = i;
    }

    qsort(temp, neighbours_size, sizeof(int), cmp_index);

    byte _my_seqnums[neighbours_size] = {0};
    byte _neighbour_seqnums[neighbours_size] = {0};
    addr _neighbours[neighbours_size] = {0};

    for(int i = 0; i < neighbours_size; i++){
        _my_seqnums[i] = my_seqnums[temp[i]];
        _neighbour_seqnums[i] = neighbour_seqnums[temp[i]];
        _neighbours[i] = neighbours[temp[i]];
    }
    
    for(int i = 0; i < neighbours_size; i++){
        my_seqnums[i] = _my_seqnums[i];
        neighbour_seqnums[i] = _neighbour_seqnums[i];
        neighbours[i] = _neighbours[i];
    }
    return;
}

//
//      CORE 2
//

void Receive(void){
    hw_flags = 0;
    packed_header ph = {0};
    // if we successfuly read data we continue
    int state = radio.readData((byte*)&ph, sizeof(packed_header));
    if (state != RADIOLIB_ERR_NONE){
        hw_flags |= ERROR;
        radio.finishReceive();
        radio.startReceive();
        return;
    }

    unpacked_header uh = UNPACK_HEADER(ph);

    //compare hmac
    if (*(unsigned short*)ph.hmac != HASH_PH(ph)){
        hw_flags |= INVALID_HASH; 
        radio.finishReceive();
        radio.startReceive();
        return;
    }

    addr neighbour = {0};
    neighbour.address = uh.mac_s;
    addr result = find_addr(neighbour);
    addr zero = {0};

    // if its not our neighbour, we add them to neighbours
    if (_memcmp(&result, &zero, sizeof(addr)) == 0){
        neighbours[neighbours_size % MAX_NEIGHBOURS] = neighbour;

        if (neighbours_size < MAX_NEIGHBOURS){
            sort_neighbours();
            neighbours_size += 1;
        }
        add_unit(initialize_unit(uh.mac_s, 0, uh.mac_s));
    }

    // if its not for me or local broadcast, we drop the packet
    if (uh.mac_d != 0x3fff || uh.mac_d != __my_address.address){
        radio.finishReceive();
        radio.startReceive();
        return;
    }
    
    //compare seqnum
    int i = 0;
    for(; neighbours[i].address != uh.mac_s && i < neighbours_size; i++){}
    if (i == neighbours_size){
        hw_flags |= NOT_NEIGHBOUR;
        radio.finishReceive();
        radio.startReceive();
        return; 
    }

    // We track seqnums of neighbours
    if (neighbour_seqnums[i] == ph.seqnum){
        neighbour_seqnums[i]++;
    } else {
        hw_flags |= INVALID_SEQNUM;
        radio.finishReceive();
        radio.startReceive();
        return;
    }

    byte data[ph.length];
    state = radio.readData(data, ph.length);
 
    packet p = packet_init(ph, data);
 
    enqueue(&received, &p);

    radio.startReceive();
    return;
}

void Transmit(void* pvParameters){
    for (;;){
        hw_flags = 0;

        //read packet from queue
        if (to_send.count == 0){
            hw_flags |= EMPTY_BUF;
            continue;
        }
        packet p = *to_send.buf[to_send.index];

        //increment seqnum;
        int i = 0;
        for(; neighbours[i].address != ((p.h.addresses[0] << 6) | (p.h.addresses[1] & 0xfc) >> 2) && i < MAX_NEIGHBOURS; i++){}
        if (i == MAX_NEIGHBOURS){
            hw_flags |= NOT_NEIGHBOUR;
            dequeue(&to_send);
            continue;
        }

        my_seqnums[i]++;
        p.h.seqnum = my_seqnums[i];

        //calculate HMAC
        int hmac = HASH_PH(p.h);
        p.h.hmac[0] = (hmac & 0xff00) >> 8;
        p.h.hmac[1] = hmac & 0xff;

        //scaning
        while (radio.scanChannel() != RADIOLIB_CHANNEL_FREE){
            sleep(random() % 11);
        }

        int state = radio.transmit((char *)&p, p.h.length + HEADER_SIZE);
        if (state != RADIOLIB_ERR_NONE){
            hw_flags |= ERROR;
            dequeue(&to_send);
            continue;
        }

        dequeue(&to_send);
        radio.startReceive();
        vTaskDelay(10);
    }
}

//
//      End of CORE 2
//

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

void process_packet(void* pvParameters){
    for(;;){
        hw_flags = 0;

        //read packet from queue
        if (to_send.count == 0){
            hw_flags |= EMPTY_BUF;
            continue;
        }
        packet p = *received.buf[received.index];

        unpacked_header received_uh = UNPACK_HEADER(p.h);
        addr net_d = {received_uh.net_d};

        unit node = find_unit(net_d);
        if ((check(node)) != SUCCESS){
            hw_flags |= INVALID_ADDRESS;
            continue;
        }

        unpacked_header send_uh = received_uh;
        send_uh.mac_s = __my_address.address;
        if ((node.hnextHop << 8 | node.lnextHop) != __my_address.address){
            send_uh.mac_d = (node.hnextHop << 8 | node.lnextHop);
        } else {
            // Proccessing packets
            protocols[p.h.protocol_id](&p);
            continue;
        }
        
        packed_header send_ph = PACK_HEADER(send_uh);
        p = packet_init(p.h, p.data);

        enqueue(&to_send, &p);
        hw_flags &= SUCCESS;
    }
}

int route(addr dest, byte length, byte protocol_id, byte* data){
    hw_flags = 0;
    unpacked_header uh = {0, __my_address.address, dest.address, __my_address.address, length, protocol_id, 0};

    unit nextHop = find_unit(dest);
    if (_memcmp(&nextHop, &null, sizeof(unit))){
        hw_flags |= INVALID_ADDRESS;
        return hw_flags;
    }

    uh.mac_d = (nextHop.hnextHop << 8 | nextHop.lnextHop); 
    packed_header ph = PACK_HEADER(uh);
    
    packet p = packet_init(ph, data);
    enqueue(&to_send, &p);
    
    hw_flags = SUCCESS;
    return hw_flags;
}