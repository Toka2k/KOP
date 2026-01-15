#include <db/db.h>
#include <address_table.h>
#include <packet_handling.h>
#include <packet_buffering.h>

byte state = IDLE;
addr peer = {0};
static unsigned short iterations = 0;
static unsigned short counter = 0;
static unsigned short corrupted_total = 0;
static unsigned short corrupted[MAX_ITERATIONS];
const unsigned short max_units = (PAYLOAD_SIZE - sizeof(counter) - sizeof(unsigned short) - 1) / sizeof(unit);
byte length = (max_units * sizeof(unit)) + sizeof(counter) + sizeof(unsigned short) + 1;

int payload_hash(byte* data, byte length){
    int h = 0;
    for (int i = 0; i < length; i++){
        h = (h + (int)data[i]) * 47;
    }
    return h;
}
// state will indicate wether we are sending or expecting
// based on that functions will be ran

// for each action i need to have two functions
//      - one sends the packet
//      - one handles the received packet

// Download entire table
// Download a part of the table

int db_download_all_request(addr address){
    byte action = DOWNLOAD_REQ_ALL;
    peer = address;
    route(address, 1, P_DB, &action);

    xSemaphoreTake(txDoneSemaphore, portMAX_DELAY);
    return SUCCESS;
}

int db_download_all_loop(packet* p){
    counter = 0;
    unsigned int hash = 0;
    iterations = ceil(TABLE_SIZE / max_units);
    byte payload[length];

    // We first inform the number of iterations to wait for
    unpacked_header uh = UNPACK_HEADER(p->h);
    
    addr address = {uh.net_s};

    payload[0] = DOWNLOAD_ACK;
    payload[1] = iterations >> 8;
    payload[2] = iterations;

    route(address, 3, P_DB, payload);

    // than we send the table
    for (; counter < iterations; counter++){
        if (counter == iterations - 1){
            memcpy(payload + 7, &__table[counter*max_units], sizeof(unit) * (TABLE_SIZE % max_units));
            length = (TABLE_SIZE % max_units * sizeof(unit)) + 7;
            hash = payload_hash(payload + 7, length - 7);
        } else {
            memcpy(payload + 7, &__table[counter*max_units], sizeof(unit) * max_units);
            hash = payload_hash(payload + 7, length - 7);
        }

        payload[0] = DOWNLOAD_ALL;

        payload[1] = counter >> 8;
        payload[2] = counter;
        
        payload[3] = hash >> 24;
        payload[4] = hash >> 16;
        payload[5] = hash >> 8;
        payload[6] = hash;

        route(address, length, P_DB, payload);
        xSemaphoreTake(txDoneSemaphore, portMAX_DELAY);
        delay(1);
    }

    return SUCCESS;
}

int db_download_part_request(addr address){
    byte action = DOWNLOAD_REQ_PART;
    peer = address;

    byte payload[corrupted_total + 1];
    payload[0] = action;
    memcpy(payload + 1, corrupted, sizeof(unsigned short) * corrupted_total);

    route(address, 1 + sizeof(unsigned short) * corrupted_total, P_DB, payload);

    xSemaphoreTake(txDoneSemaphore, portMAX_DELAY);
    return SUCCESS;
}

int db_download_ack(packet* p){
    iterations = (p->data[1] << 8) | p->data[2];
    counter = 0;
    length = (max_units * sizeof(unit)) + sizeof(counter) + sizeof(unsigned short) + 1;

    return SUCCESS;
}

int db_download_receive(packet* p){
    unsigned int hash = payload_hash(p->data + 7, length - 7);;
    unsigned int received_hash = (p->data[3] << 24) | (p->data[4] << 16) | (p->data[5] << 8) | (p->data[6]);

    if (hash != received_hash){
        corrupted[corrupted_total] = counter;
        corrupted_total++;

        return INVALID_HASH;
    }

    add_units((p->h.length - 7) / sizeof(unit), (unit*)(p->data+7));

    counter++;
    if(counter == iterations){
        return PACKET_LAST;
    }
    
    return SUCCESS;
}

int DB(packet* p){
    unpacked_header uh = UNPACK_HEADER(p->h);
    if(state == IDLE){
        peer.address = uh.net_s;
        iterations = 0, counter = 0, corrupted_total = 0;
    } else if(state == ACTIVE && uh.net_s != peer.address){
        return INVALID_ADDRESS;
    }

    if(p->data[0] == DOWNLOAD_REQ_ALL){
        state = ACTIVE;
        return db_download_all_loop(p);
    } else if (p->data[0] == DOWNLOAD_ACK){
        return db_download_ack(p);
    }
    
    if (db_download_receive(p) != PACKET_LAST){ return PACKET_RECEIVED; }

    if (corrupted_total == 0){
        db_download_end();
    }

    db_download_req_part();

    return SUCCESS;
}