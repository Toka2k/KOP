#include <updates.h>
#include <packet_handling.h>
#include <address_table.h>
#include <arp/arp.h>

// Update packets:
// 4 bytes hash, 2 bytes address, 2 bytes cost ...
enum MODE{
    DELTA = 0, FULL
};

unsigned short advert_index = 0;
unsigned int changed[MAX_TABLE_SIZE / 32] = {0};

unsigned short* changed_addresses = NULL;
unsigned short size_changed_addr = 0;

unsigned short* held_down;
unsigned long* timers; 
unsigned short size_held_down = 0;

xSemaphoreHandle trigger_update;

byte current_mode = FULL;

byte* missed_msg = NULL;

void init_updates(){
    missed_msg = malloc(sizeof(byte) * MAX_TABLE_SIZE / 4);
    memset(missed_msg, 0, sizeof(byte) * MAX_TABLE_SIZE / 4);

    xTaskCreate(increment_counter, "FULL UPDATE COUNTER", 2048, NULL, 2, NULL);
    xTaskCreate(increment_neighbour_counter, "TRIGGER UPDATE COUNTER", 2048, NULL, 2, NULL);
    xTaskCreate(full_update_task, "FULL UPDATE TASK", 2048, NULL, 2, NULL);
    xTaskCreate(trigger_update_task, "TRIGGER UPDATE TASK", 2048, NULL, 2, NULL);
    xTaskCreate(hello, "HELLO MESSAGE TASK", 1024, NULL, 1, NULL);
}

int process_update(packet* p){
    int hash = p->data[0] << 24 | p->data[1] << 16 | p->data[2] << 8 | p->data[3];
    if (payload_hash(p->data + sizeof(int), p->h.length - sizeof(int)) != hash){
        return INVALID_HASH;
    }

    unpacked_header uh = UNPACK_HEADER(p->h);

    byte iterations = (p->h.length - 4) / 4;
    addr address = {0};
    unsigned short cost = 0;
    unit* to_add = malloc(sizeof(unit) * iterations);
    memset(to_add, 0, sizeof(unit) * iterations);

    for (byte i = 0; i < iterations; i++){
        unsigned short base = 4 + i * 4;
        address.address = (p->data[base] << 8 | p->data[base + 1]) & 0x3fff;
        cost = (p->data[base + 2] << 8 | p->data[base + 3]) & 0xfff;
        unit result = find_unit(address);
        
        unsigned short j;
        for(j = 0; j < size_held_down && held_down[j] != address.address; j++){}
        if (j < size_held_down){
            if (timers[j] - millis() < 1000 * HOLD_DOWN_S){
                continue;
            } else {
                timers[j] = timers[size_held_down - 1];
                held_down[j] = held_down[size_held_down - 1];
                timers = realloc(timers, sizeof(long) * --size_held_down);
                held_down = realloc(held_down, sizeof(short) * size_held_down);
            }
        }

        if (address.address == __my_address.address){
            continue;
        } else if (_memcmp(&result, &null, sizeof(unit)) == 0){
            to_add[i] = initialize_unit(address.address, cost >= 0xfff ? 0xfff : cost + 1, uh.mac_s);
            changed[address.address / 32] |= 1 << address.address % 32;
        } else if ((result.hcost << 10 | result.cost << 2 | result.lcost) > cost + 1){
            to_add[i]= initialize_unit(address.address, cost >= 0xfff ? 0xfff : cost + 1, uh.mac_s);
            changed[address.address / 32] |= 1 << address.address % 32;
        } else if ((result.hnextHop << 8 | result.lnextHop) == uh.mac_s){
            to_add[i] = initialize_unit(address.address, cost >= 0xfff ? 0xfff : cost + 1, uh.mac_s);
            changed[address.address / 32] |= 1 << address.address % 32;
        } else { continue; }
        
        mark_route_refreshed(address);
    }

    add_units(iterations, to_add);
    free(to_add);

    return SUCCESS;
}

void invalidate_routes(addr neighbour){
    addr address = {0};
    for (short i = 0; i < TABLE_SIZE; i++){
        address.address = UNIT_ADDRESS(__table[i]);

        unsigned short i = find_neighbour(neighbour);
        if(i < neighbours_size){
            remove_neighbour(neighbour);
        }

        if((__table[i].hnextHop << 8 | __table[i].lnextHop) == neighbour.address){
            __table[i].hcost = 0x3;
            __table[i].cost = 0xff;
            __table[i].lcost = 0x3;

            held_down = realloc(held_down, sizeof(short) * ++size_held_down);
            held_down[size_held_down - 1] = address.address;

            timers = realloc(timers, sizeof(long) * size_held_down);
            timers[size_held_down - 1] = millis();


            changed[address.address / 32] |= 1 << (address.address % 32);
        }
    }
}

unsigned short* get_routes_to_advertise(addr neighbour){
    byte iterations = (PAYLOAD_SIZE - sizeof(int) - PAYLOAD_SIZE % sizeof(int)) / sizeof(int);
    unsigned short* chunk = malloc(PAYLOAD_SIZE - sizeof(int) - PAYLOAD_SIZE % sizeof(int));

    unsigned short address = 0, cost = 0;

    if (current_mode == FULL){
        byte i;
        for(i = 0; i < iterations && i + iterations * advert_index < TABLE_SIZE; i++){
            address = __table[i + iterations * advert_index].haddress << 8 | __table[i + iterations * advert_index].laddress;
            cost = __table[i + iterations * advert_index].hcost << 10 | __table[i + iterations * advert_index].cost << 2 | __table[i + iterations * advert_index].lcost;

            chunk[i * 2] = address;
            
            addr search = {address};
            unit result = find_unit(search);
            if((result.hnextHop << 8 | result.lnextHop) == neighbour.address){
                cost = 0xfff;
            }

            chunk[i * 2 + 1] = cost;
        }

        if (i != iterations){
            advert_index = 0;
        } else{
            advert_index++;
        }
    } else if (current_mode == DELTA){
        byte i;
        for(i = 0; i < iterations && i < size_changed_addr; i++){
            addr search = {changed_addresses[i]};
            unit result = find_unit(search);
            
            chunk[i * 2] = changed_addresses[i];
            chunk[i * 2 + 1] = result.hcost << 10 | result.cost << 2 | result.lcost;
            
            if((result.hnextHop << 8 | result.lnextHop) == neighbour.address){
                chunk[i * 2 + 1] = 0xfff;
            }
        }

        for (byte j = 0; j < i; j++) {
            unsigned short removed = changed_addresses[0];

            changed[removed / 32] &= ~(1 << (removed % 32));
            size_changed_addr--;

            changed_addresses[0] = changed_addresses[size_changed_addr];
        }

        if(i == 0){
            return NULL;
        }
        size_changed_addr -= i;
        changed_addresses = realloc(changed_addresses, sizeof(unsigned short) * size_changed_addr);
    }
    return chunk;
}

void mark_route_refreshed(addr dest){
    unsigned short i = find_neighbour(dest);
    if(i == neighbours_size){ return; }
    missed_msg[dest.address/4] &= ~(0x3 << (dest.address % 4 * 2));

    for(i = 0; i < size_held_down; i++){
        if(held_down[i] == dest.address){
            timers[i] = timers[size_held_down - 1];
            held_down[i] = held_down[size_held_down - 1];
            timers = realloc(timers, sizeof(long) * --size_held_down);
            held_down = realloc(held_down, sizeof(short) * size_held_down);
        }
    }
}

void get_changed_routes(){
    unsigned short* temp = NULL;
    size_changed_addr = 0;
    if(changed_addresses != NULL){
        free(changed_addresses);
    }
    changed_addresses = NULL;

    for (short i = 0; i < MAX_TABLE_SIZE / 32; i++){
        if (changed[i] == 0){ continue; }
        for (byte j = 0; j < 32; j++){
            if (changed[i] & (1 << j)){
                size_changed_addr++;
                changed_addresses = realloc(changed_addresses, sizeof(unsigned short) * size_changed_addr);
                changed_addresses[size_changed_addr - 1] = i * 32 + j;
            }
        }
    }
    return;
}

// TASKS:
void hello(void* pvParameters){
    for(;;){
        ECHO_REQ((addr){LOCAL_BROADCAST});
        vTaskDelay(8000);
    }
}

void increment_neighbour_counter(void* pvParameters){
    for(;;){
        for(unsigned short i = 0; i < neighbours_size; i++){
            if(missed_msg[neighbours[i].address / 4] == 0x3 << (neighbours[i].address)){
                invalidate_routes(neighbours[i]);
            }
            missed_msg[neighbours[i].address / 4] += 1 << (neighbours[i].address % 4 * 2);
        }
        xSemaphoreGive(trigger_update);
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}

void increment_counter(void* pvParameters){
    for(;;){
        for(unsigned short i = 0; i < TABLE_SIZE; i++){
            if (find_neighbour((addr) {UNIT_ADDRESS(__table[i])}) < neighbours_size){
                continue;
            }
            if (missed_msg[UNIT_ADDRESS(__table[i]) / 4] == 0x3 << (UNIT_ADDRESS(__table[i]) % 4 * 2)){
                invalidate_routes((addr){UNIT_ADDRESS(__table[i])});
            }
            missed_msg[UNIT_ADDRESS(__table[i]) / 4] += 1 << (UNIT_ADDRESS(__table[i]) % 4 * 2);
        }
        vTaskDelay(pdMS_TO_TICKS(20000));
    }
}

void trigger_update_task(void* pvParameters){
    trigger_update = xSemaphoreCreateBinary();
    byte len = 0;
    for(;;){
        xSemaphoreTake(trigger_update, portMAX_DELAY);
        unsigned short* chunk;

        get_changed_routes();
        
        do {
            if (size_changed_addr > (PAYLOAD_SIZE - sizeof(int) - 2) / sizeof(int)){
                len = PAYLOAD_SIZE - 2;
            } else {
                len = size_changed_addr * sizeof(int) + sizeof(int);
            }
            current_mode = DELTA;
            chunk = get_routes_to_advertise((addr){0});

            int hash = payload_hash((byte*)chunk, len - sizeof(int));

            byte payload[len];
            payload[0] = (hash & 0xff000000) >> 24;
            payload[1] = (hash & 0xff0000) >> 16;
            payload[2] = (hash & 0xff00) >> 8;
            payload[3] = hash & 0xff;

            memcpy(payload + sizeof(int), chunk, len - sizeof(int));
            free(chunk);

            route((addr){LOCAL_BROADCAST}, len, P_UPDATE, payload);
            vTaskDelay(pdMS_TO_TICKS(random() % 150 + 50));
        } while (chunk != NULL);

    }
}

void full_update_task(void* pvParameters){
    byte len = 0;
    for(;;){
        unsigned short* chunk;

        do {
            if (TABLE_SIZE - advert_index * (PAYLOAD_SIZE - sizeof(int) - 2) / sizeof(int) > (PAYLOAD_SIZE - sizeof(int) - 2) / sizeof(int)){
                len = PAYLOAD_SIZE - 2;
            } else {
                len = TABLE_SIZE % ((PAYLOAD_SIZE - sizeof(int) - 2) / sizeof(int)) * sizeof(int) + sizeof(int);
            }
            current_mode = FULL;
            chunk = get_routes_to_advertise((addr){0});

            int hash = payload_hash((byte*)chunk, len - sizeof(int));

            byte payload[len];
            payload[0] = (hash & 0xff000000) >> 24;
            payload[1] = (hash & 0xff0000) >> 16;
            payload[2] = (hash & 0xff00) >> 8;
            payload[3] = hash & 0xff;

            memcpy(payload + sizeof(int), chunk, len - sizeof(int));
            free(chunk);

            route((addr){LOCAL_BROADCAST}, len, P_UPDATE, payload);
        } while (chunk != NULL);
    }
}