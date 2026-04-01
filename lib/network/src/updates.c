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

held_entry* held_down = NULL;
unsigned short size_held_down = 0;

xSemaphoreHandle trigger_update;
xSemaphoreHandle neighbour_semaphore;
xSemaphoreHandle timers_semaphore;
xSemaphoreHandle changed_semaphore;

byte current_mode = FULL;

byte* missed_msg = NULL;

void init_updates(){
    missed_msg = malloc(sizeof(byte) * MAX_TABLE_SIZE);
    memset(missed_msg, 0, sizeof(byte) * MAX_TABLE_SIZE);

    trigger_update = xSemaphoreCreateBinary();
    neighbour_semaphore = xSemaphoreCreateBinary();
    timers_semaphore = xSemaphoreCreateBinary();
    changed_semaphore = xSemaphoreCreateBinary();

    xSemaphoreGive(trigger_update);
    xSemaphoreGive(neighbour_semaphore);
    xSemaphoreGive(timers_semaphore);
    xSemaphoreGive(changed_semaphore);

    xTaskCreate(increment_counter, "FULL UPDATE COUNTER", 4096, NULL, 2, NULL);
    xTaskCreate(increment_neighbour_counter, "TRIGGER UPDATE COUNTER", 4096, NULL, 2, NULL);
    xTaskCreate(full_update_task, "FULL UPDATE TASK", 4096, NULL, 2, NULL);
    xTaskCreate(trigger_update_task, "TRIGGER UPDATE TASK", 4096, NULL, 2, NULL);
    xTaskCreate(hello, "HELLO MESSAGE TASK", 4096, NULL, 2, NULL);
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
        address.address = (p->data[base] | p->data[base + 1] << 8) & 0x3fff;
        cost = 1 + (p->data[base + 2] | p->data[base + 3] << 8) & 0xfff;
        unit result = find_unit(address);
        
        unsigned short j;
        for(j = 0; j < size_held_down && held_down[j].address != address.address; j++){}
        if (j < size_held_down){
            if (held_down[j].timer - millis() < 1000 * HOLD_DOWN_S){
                continue;
            } else {
                xSemaphoreTake(timers_semaphore, portMAX_DELAY);

                held_down[j] = held_down[size_held_down - 1];

                if (size_held_down > 1) {
                    int new_size = size_held_down - 1;

                    void* tmp = realloc(held_down, new_size * sizeof(*held_down));
                    if (tmp != NULL) {
                        held_down = tmp;
                    }
                    size_held_down = new_size;

                } else {
                    free(held_down);
                    held_down = NULL;
                    size_held_down = 0;
                }

                xSemaphoreGive(timers_semaphore);
            }
        }

        if (address.address == __my_address.address){
            continue;
        } else if (_memcmp(&result, &null, sizeof(unit)) == 0 && cost < 0xfff){
            to_add[i] = initialize_unit(address.address, cost, uh.mac_s);

            xSemaphoreTake(changed_semaphore, portMAX_DELAY);
            changed[address.address / 32] |= 1 << address.address % 32;
            xSemaphoreGive(changed_semaphore);
        } else if ((result.hcost << 10 | result.cost << 2 | result.lcost) > cost && cost < 0xfff){
            to_add[i] = initialize_unit(address.address, cost, uh.mac_s);
            
            xSemaphoreTake(changed_semaphore, portMAX_DELAY);
            changed[address.address / 32] |= 1 << address.address % 32;
            xSemaphoreGive(changed_semaphore);
        } else if ((result.hnextHop << 8 | result.lnextHop) != uh.mac_s && cost < UNIT_COST(result)){
            to_add[i] = initialize_unit(address.address, cost, uh.mac_s);
            
            xSemaphoreTake(changed_semaphore, portMAX_DELAY);
            changed[address.address / 32] |= 1 << address.address % 32;
            xSemaphoreGive(changed_semaphore);
        } else { continue; }
        
        mark_route_refreshed(address);
    }

    add_units(iterations, to_add);
    free(to_add);

    return SUCCESS;
}

void invalidate_routes(addr neighbour){
    addr address = {0};
    unsigned short j = find_neighbour(neighbour);
    if(j < neighbours_size){
        remove_neighbour(neighbour);
    }

    for (short i = 0; i < TABLE_SIZE; i++){
        address.address = UNIT_ADDRESS(__table[i]);

        xSemaphoreTake(table_semaphore, portMAX_DELAY);
        if((__table[i].hnextHop << 8 | __table[i].lnextHop) == neighbour.address){
            __table[i].hcost = 0x3;
            __table[i].cost = 0xff;
            __table[i].lcost = 0x3;

            xSemaphoreTake(timers_semaphore, portMAX_DELAY);
            int temp = 1 + size_held_down;

            void* tmp = realloc(held_down, sizeof(*held_down) * temp);
            if (tmp != NULL) {
                held_down = tmp;
                held_down[temp - 1].address = address.address;
                held_down[temp - 1].timer = millis();
                size_held_down = temp;
            }
            xSemaphoreGive(timers_semaphore);

            xSemaphoreTake(changed_semaphore, portMAX_DELAY);
            changed[address.address / 32] |= 1 << (address.address % 32);
            xSemaphoreGive(changed_semaphore);
        }
        xSemaphoreGive(table_semaphore);
    }
}

unsigned short* get_routes_to_advertise(addr neighbour, byte* out_len){
    byte iterations = (PAYLOAD_SIZE - sizeof(int) - PAYLOAD_SIZE % sizeof(int)) / sizeof(int);
    unsigned short *chunk = malloc(iterations * sizeof(short) * 2), *out = NULL;

    unsigned short address = 0, cost = 0;
    xSemaphoreTake(table_semaphore, portMAX_DELAY);

    if (current_mode == FULL){
        int start = advert_index * iterations;

        if (start >= TABLE_SIZE) {
            advert_index = 0;
            free(chunk);
            xSemaphoreGive(table_semaphore);
            return NULL;
        }

        byte i;
        for(i = 0; i < iterations && (start + i) < TABLE_SIZE; i++){
            int idx = start + i;

            address = __table[idx].haddress << 8 | __table[idx].laddress;
            cost = __table[idx].hcost << 10 | __table[idx].cost << 2 | __table[idx].lcost;

            chunk[i * 2] = address;

            addr search = {address};
            unit result = find_unit(search);
            // potentionaly use invalidate routes
            if((result.hnextHop << 8 | result.lnextHop) == neighbour.address){
                cost = 0xfff;
            }

            chunk[i * 2 + 1] = cost;
        }
        *out_len = i * sizeof(short) * 2;

        advert_index++;
    } else if (current_mode == DELTA){
        byte i;
        int temp = size_changed_addr;
        
        xSemaphoreTake(changed_semaphore, portMAX_DELAY);
        for(i = 0; i < iterations && i < size_changed_addr; i++){
            addr search = {changed_addresses[i]};
            unit result = find_unit(search);
            
            chunk[i * 2] = changed_addresses[i];
            chunk[i * 2 + 1] = result.hcost << 10 | result.cost << 2 | result.lcost;
            
            if((result.hnextHop << 8 | result.lnextHop) == neighbour.address){
                chunk[i * 2 + 1] = 0xfff;
            }
        }
        *out_len = i * sizeof(short) * 2;

        for (byte j = 0; j < i; j++) {
            unsigned short removed = changed_addresses[size_changed_addr - 1];

            changed[removed / 32] &= ~(1 << (removed % 32));

            size_changed_addr--;
        }
        xSemaphoreGive(changed_semaphore);

        if(i == 0){
            free(chunk);
            xSemaphoreGive(table_semaphore);
            return NULL;
        }
        
        xSemaphoreTake(changed_semaphore, portMAX_DELAY);
        if (size_changed_addr != 0){
            unsigned short* tmp = realloc(changed_addresses, sizeof(*changed_addresses) * size_changed_addr);
            if (tmp != NULL) {
                changed_addresses = tmp;
            }
        } else {
            free(changed_addresses);
            changed_addresses = NULL;
        }
       xSemaphoreGive(changed_semaphore);
    }

    out = malloc(*out_len);
    memcpy(out, chunk, *out_len);
    free(chunk);

    xSemaphoreGive(table_semaphore);
    return out;
}

void mark_route_refreshed(addr dest){
    unsigned short i = find_neighbour(dest);
    xSemaphoreTake(neighbour_semaphore, portMAX_DELAY);
    if(i == neighbours_size){ 
        xSemaphoreGive(neighbour_semaphore);
        return;
    }
    missed_msg[dest.address] = 0;
    xSemaphoreGive(neighbour_semaphore);

    for(i = 0; i < size_held_down; i++){
        if(held_down[i].address == dest.address){
            xSemaphoreTake(timers_semaphore, portMAX_DELAY);
            held_down[i] = held_down[size_held_down - 1];

            if (size_held_down > 1) {
                int new_size = size_held_down - 1;

                void* tmp = realloc(held_down, new_size * sizeof(*held_down));
                if (tmp != NULL) {
                    held_down = tmp;
                }
                size_held_down = new_size;

            } else {
                free(held_down);
                held_down = NULL;
                size_held_down = 0;
            }

            xSemaphoreGive(timers_semaphore);
        }
    }
}

void get_changed_routes(){
    xSemaphoreTake(changed_semaphore, portMAX_DELAY);
    
    size_changed_addr = 0;
    if(changed_addresses != NULL){
        free(changed_addresses);
    }
    changed_addresses = NULL;

    for (short i = 0; i < MAX_TABLE_SIZE / 32; i++){
        
        if (changed[i] == 0){ 
            continue;
        }
        for (byte j = 0; j < 32; j++){
            if (changed[i] & (1 << j)){
                int temp = size_changed_addr + 1;
                void* tmp = realloc(changed_addresses, sizeof(*changed_addresses) * temp);
                if (tmp != NULL){
                    changed_addresses = tmp;
                    size_changed_addr = temp;
                    changed_addresses[size_changed_addr - 1] = i * 32 + j;
                }
            }
        }
    }
    xSemaphoreGive(changed_semaphore);
    return;
}

// TASKS:
void hello(void* pvParameters){
    addr a = {LOCAL_BROADCAST};
    for(;;){
        ECHO_REQ(a);
        vTaskDelay(pdMS_TO_TICKS(8000));
    }
}

void increment_neighbour_counter(void* pvParameters){
    for(;;){
        xSemaphoreTake(neighbour_semaphore, portMAX_DELAY);
        for(unsigned short i = 0; i < neighbours_size; i++){
            if(missed_msg[neighbours[i].address] == 3){
                xSemaphoreGive(neighbour_semaphore);                
                invalidate_routes(neighbours[i]);
                xSemaphoreTake(neighbour_semaphore, portMAX_DELAY);
            }
            missed_msg[neighbours[i].address] += 1;
        }
        xSemaphoreGive(neighbour_semaphore);

        xSemaphoreGive(trigger_update);
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}

void increment_counter(void* pvParameters){
    for(;;){
        for(unsigned short i = 0; i < TABLE_SIZE; i++){
            short idx = find_neighbour((addr) {UNIT_ADDRESS(__table[i])});
            xSemaphoreTake(neighbour_semaphore, portMAX_DELAY);
            if (idx < neighbours_size || UNIT_ADDRESS(__table[i]) == __my_address.address){
                xSemaphoreGive(neighbour_semaphore);
                continue;
            }
            
            if (missed_msg[UNIT_ADDRESS(__table[i])] == 0x3){
                xSemaphoreGive(neighbour_semaphore);
                invalidate_routes((addr){UNIT_ADDRESS(__table[i])});
                xSemaphoreTake(neighbour_semaphore, portMAX_DELAY);
            }
            missed_msg[UNIT_ADDRESS(__table[i])] += 1;
            xSemaphoreGive(neighbour_semaphore);
        }
        vTaskDelay(pdMS_TO_TICKS(20000));
    }
}

void trigger_update_task(void* pvParameters){
    for(;;){
        byte len = 0;
        xSemaphoreTake(trigger_update, portMAX_DELAY);
        unsigned short* chunk;

        get_changed_routes();
        
        while(1) {
            xSemaphoreTake(neighbour_semaphore, portMAX_DELAY);
            current_mode = DELTA;
            chunk = get_routes_to_advertise((addr){0}, &len);
            xSemaphoreGive(neighbour_semaphore);

            if(chunk == NULL){ 
                break;
            }

            int hash = payload_hash((byte*)chunk, len);
            len += sizeof(int);

            byte* payload = malloc(sizeof(byte) * len);
            if(payload == NULL){ break; }
            payload[0] = (hash & 0xff000000) >> 24;
            payload[1] = (hash & 0xff0000) >> 16;
            payload[2] = (hash & 0xff00) >> 8;
            payload[3] = hash & 0xff;

            memcpy(payload + sizeof(int), chunk, len - sizeof(int));
            free(chunk);

            route((addr){LOCAL_BROADCAST}, len, P_UPDATE, payload);
            free(payload);

            vTaskDelay(pdMS_TO_TICKS(random() % 150 + 50));
        }
    }
}

void full_update_task(void* pvParameters){
    byte len = 0;
    for(;;){

        unsigned short* chunk;

        while(1) {
            xSemaphoreTake(neighbour_semaphore, portMAX_DELAY);
            current_mode = FULL;
            chunk = get_routes_to_advertise((addr){0}, &len);
            xSemaphoreGive(neighbour_semaphore);

            if(chunk == NULL){
                break;
            }

            int hash = payload_hash((byte*)chunk, len);
            len += sizeof(int);

            byte* payload = malloc(sizeof(byte) * len);
            if(payload == NULL){ break; }
            payload[0] = (hash & 0xff000000) >> 24;
            payload[1] = (hash & 0xff0000) >> 16;
            payload[2] = (hash & 0xff00) >> 8;
            payload[3] = hash & 0xff;

            memcpy(payload + sizeof(int), chunk, len - sizeof(int));

            route((addr){LOCAL_BROADCAST}, len, P_UPDATE, payload);
            free(chunk);
            free(payload);
            
            vTaskDelay(pdMS_TO_TICKS(random() % 150 + 50));
        }
        vTaskDelay(pdMS_TO_TICKS(20000));
    }
}