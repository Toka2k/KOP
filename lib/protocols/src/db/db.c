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
const unsigned short max_units = (PAYLOAD_SIZE - sizeof(counter) - sizeof(short) - 1) / sizeof(unit);
byte length = (max_units * sizeof(unit)) + sizeof(counter) + sizeof(short) + 1;

int payload_hash(byte* data, byte length){
    int h = 0;
    for (int i = 0; i < length; i++){
        h = (h + (int)data[i]) * 47;
    }
    return h;
}

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
    length = (max_units * sizeof(unit)) + sizeof(counter) + sizeof(short) + 1;
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
            byte _length = (TABLE_SIZE % max_units * sizeof(unit)) + 7;
            hash = payload_hash(payload + 7, _length - 7);
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
    // We inform the sender how many corrupt packets we received
    byte _payload[3] = {DOWNLOAD_REQ_CORRUPTED, corrupted_total >> 8, corrupted_total};
    route(address, 3, P_DB, _payload);

    // we send the corrupted packet ids
    // It may not fit into one packet :/
    byte max_count = (PAYLOAD_SIZE - sizeof(byte)) / sizeof(short);
    
    byte payload[max_count * sizeof(short) + 1];
    payload[0] = DOWNLOAD_REQ_PART;

    byte its = ceil(corrupted_total / max_count);
    for(int i = 0; i < its; i++){
        if(i == its - 1){
            memcpy(payload + 1, corrupted + i * max_count, (corrupted_total % max_count) * sizeof(short));
            route(address, (corrupted_total % max_count) * sizeof(short) + 1, P_DB, payload);
        } else {
            memcpy(payload + 1, corrupted + i * max_count, max_count * sizeof(short));
            route(address, max_count * sizeof(short) + 1, P_DB, payload);
        }
    }

    return SUCCESS;
}

int db_download_part_loop(packet* p){
    unsigned int hash = 0;
    length = (max_units * sizeof(unit)) + sizeof(counter) + sizeof(short) + 1;
    byte payload[length];

    // We first inform the number of iterations to wait for
    unpacked_header uh = UNPACK_HEADER(p->h);
    
    addr address = {uh.net_s};

    payload[0] = DOWNLOAD_ACK;
    payload[1] = iterations >> 8;
    payload[2] = iterations;

    route(address, 3, P_DB, payload);

    // than we send the table
    for (counter = 0; counter < corrupted_total; counter++){
        if (counter == corrupted_total - 1){
            memcpy(payload + 7, &__table[corrupted[counter] * max_units], sizeof(unit) * (TABLE_SIZE % max_units));
            byte _length = (TABLE_SIZE % max_units * sizeof(unit)) + 7;
            hash = payload_hash(payload + 7, _length - 7);
        } else {
            memcpy(payload + 7, &__table[corrupted[counter] * max_units], sizeof(unit) * max_units);
            hash = payload_hash(payload + 7, length - 7);
        }

        payload[0] = DOWNLOAD_PART;

        payload[1] = counter >> 8;
        payload[2] = counter;
        
        payload[3] = hash >> 24;
        payload[4] = hash >> 16;
        payload[5] = hash >> 8;
        payload[6] = hash;

        route(address, length, P_DB, payload);
    }
}

int db_download_ack(packet* p){
    iterations = (p->data[1] << 8) | p->data[2];
    counter = 0;
    length = (max_units * sizeof(unit)) + sizeof(counter) + sizeof(short) + 1;

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
    } else if (p->data[0] == DOWNLOAD_REQ_CORRUPTED){
        corrupted_total = (p->data[1] << 8) + p->data[2];
        counter = 0;
        return SUCCESS;
    } else if (p->data[0] == DOWNLOAD_REQ_PART){
        byte its = (p->h.length - 1) / sizeof(short);
        for(int i = 0; i < its; i++){
            corrupted[counter + i] = (p->data[i * sizeof(short) + 1] << 8) | p->data[i * sizeof(short) + 2];
        }
        counter += its;
        return SUCCESS;
    }
    
    if (db_download_receive(p) != PACKET_LAST){ return PACKET_RECEIVED; }

    if (corrupted_total != 0){
        db_download_part_loop(p);
    }

    db_download_end();

    return SUCCESS;
}