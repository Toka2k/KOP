#include "address_table.h"

//defining important variables and lists
unit null = {0};
static unit __table[MAX_TABLE_SIZE] = {0};
flags FLAGS = {0};
size __table_size = {0};
static unit __reserved_addresses[] = {
    {0, 0x3f, 0, 0, 0, 0xff, 0},
    {0, 0, 0, 0, 0, 0, 0}
};
int routers[1 << (ADDRESS_BITS - 5)] = {0};

// MANAGE HIGHEST ADDRESS WITH DHCP
addr __highest_address = {1};
addr __my_address = {0};

int check(unit check){
    for(int j = 0; j < RESERVED_ADDRESSES; j++){
        if (_memcmp(&check, &__reserved_addresses[j], sizeof(unit)) == 0){
            return -1;
        } else if ((check.haddress << 8 | check.laddress) == (__reserved_addresses[j].haddress << 8 | __reserved_addresses[j].laddress)){
            return -2;
        } else if ((check.hnextHop << 8 | check.lnextHop) == (__reserved_addresses[j].haddress << 8 | __reserved_addresses[j].laddress)){
            return -3;
        }
    }

    if ((check.hcost << 10 | check.cost << 2 | check.lcost) == (~0 & 0xfff)){
        return -4;
    }

    return SUCCESS;
}

int _memcmp(const void* buf1, const void* buf2, int count){
    if (!count){
        return 0;
    }

    while(--count && *(char*)buf1 == *(char*)buf2){
        buf1 = (char*)buf1 + 1;
        buf2 = (char*)buf2 + 1;
    }
    return (*((unsigned char*)buf1) - *((unsigned char*)buf2));
}   

int add_unit(unit add){
    int i = check(add);
    if (i){
        return i;
    }
    
    i = 0;
    for(; (add.haddress << 8 | add.laddress) != (__table[i].haddress << 8 | __table[i].laddress) && _memcmp(&__table[i], &add, sizeof(unit)) && i < tSize; i++){}
    if (i == tSize && check(add) == 0){
        __table[tSize++] = add;
    } else if (FLAGS.UPDATE_WHEN_ADD && i < tSize && (add.haddress << 8 | add.laddress) == (__table[i].haddress << 8 | __table[i].laddress)){
        __table[i] = add;
    }
    return tSize;
}

int remove_unit(unit remove){
    int i = check(remove);
    if (i){
        return i;
    }
    
    i = 0;
    if (FLAGS.REMOVE_WITH_ADDRESS){
        for(; (__table[i].haddress << 8 | __table[i].laddress) != (remove.haddress << 8 | remove.laddress) && i < tSize; i++){}
    } else if (FLAGS.REMOVE_WITH_NEXTHOP){
        for(; (__table[i].hnextHop << 8 | __table[i].lnextHop) != (remove.hnextHop << 8 | remove.lnextHop) && i < tSize; i++){}
    } else if (FLAGS.REMOVE_WITH_NEXTHOP && FLAGS.REMOVE_WITH_ADDRESS){
        for(;   (__table[i].hnextHop << 8 | __table[i].lnextHop) != (remove.hnextHop << 8 | remove.lnextHop) &&\
                (__table[i].haddress << 8 | __table[i].laddress) != (remove.haddress << 8 | remove.laddress) &&\
                i < tSize; i++){}
    } else {
        for(; _memcmp(&__table[i], &remove, sizeof(unit)) && i < tSize; i++){}
    }
    if (i < tSize){
        __table[i] = __table[--tSize];
        __table[tSize] = null; 
    }
    return tSize;
}

int update_unit(unit update){
    int i = check(update);
    if (i){
        return i;
    }
    i = 0;
    for(; (__table[i].haddress << 8 | __table[i].laddress) != (update.haddress << 8 | update.laddress); i++){}
    __table[i] = update;

    return tSize;
}

void clear_table(){
    for(int i = 0; i < tSize; i++){
        __table[i] = null;
    }

    return;
}

int add_units(int _size, unit* add){
    int i = 0, j = 0;
    for(; i < _size; i++){
        for(j = 0; (add[i].haddress << 8 | add[i].laddress) != (__table[j].haddress << 8 | __table[j].laddress) && _memcmp(&add[i], &__table[j], sizeof(unit)) && j < tSize; j++){}
        if (j == tSize && check(add[i]) == 0){
            __table[tSize++] = add[i];
        } else if (FLAGS.UPDATE_WHEN_ADD && j < tSize && (add[i].haddress << 8 | add[i].laddress) == (__table[j].haddress << 8 | __table[j].laddress)){
            __table[j] = add[i];
        }
    }

    return tSize;
}

int remove_units(int _size, unit* remove){
    int i = 0, j = 0;
    for (;i < _size; i++){
        if (FLAGS.REMOVE_WITH_ADDRESS){
            for(; (__table[j].haddress << 8 | __table[j].laddress) != (remove[i].haddress << 8 | remove[i].laddress) && j < tSize; j++){}
        } else if (FLAGS.REMOVE_WITH_NEXTHOP){
            for(; (__table[j].hnextHop << 8 | __table[j].lnextHop) != (remove[i].hnextHop << 8 | remove[i].lnextHop) && j < tSize; j++){}
        } else if (FLAGS.REMOVE_WITH_NEXTHOP && FLAGS.REMOVE_WITH_ADDRESS){
            for(;   (__table[j].hnextHop << 8 | __table[j].lnextHop) != (remove[i].hnextHop << 8 | remove[i].lnextHop) &&\
                    (__table[j].haddress << 8 | __table[j].laddress) != (remove[i].haddress << 8 | remove[i].laddress) &&\
                    j < tSize; j++){}
        } else {
            for(j = 0; _memcmp(&remove[i], &__table[j], sizeof(unit)) && j < tSize; j++){}
    }        if (j != tSize){
            __table[j] = __table[--tSize];
            __table[tSize] = null;
        }
    }

    return tSize;
}

int update_units(int _size, unit* update){
    int i = 0, j = 0;
    for(; i < _size; i++){
        for(j = 0; (__table[j].haddress << 8 | __table[j].laddress) != (update[i].haddress << 8 | update[i].laddress) && j < tSize; j++){}
        if (j < tSize && check(update[i]) == 0){
            __table[j] = update[i];
        }
    }

    return tSize;
}

unit initialize_unit(unsigned short addr, unsigned short cost, unsigned short nextHop){
    unit u;
    u.haddress = (addr & 0x3f00) >> 8;
    u.laddress = addr & 0xff;
    u.hcost = (cost & 0xc00) >> 10;
    u.cost = (cost & 0x3fc) >> 2;
    u.lcost = cost & 0x3;
    u.hnextHop = (nextHop & 0x3f00) >> 8;
    u.lnextHop = nextHop & 0xff;
    return u;
}

unit find_unit(addr address){
    unit result = {0};
    int i = 0;    

    for (; address.address != (__table[i].haddress << 8 | __table[i].laddress) && i < tSize; i++){}

    if(i < tSize){
        result = __table[i];
    }
    
    return result;
}