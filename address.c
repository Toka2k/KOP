#include "address.h"

unit null = {0};

int check(unit check){
    for(int j = 0; j < RESERVED_ADDRESSES; j++){
        if (_memcmp(&check, &__reserved_addresses[j], sizeof(unit)) == 0){
            return -1;
        } else if (check.address == __reserved_addresses[j].address){
            return -2;
        } else if (check.nextHop == __reserved_addresses[j].address){
            return -3;
        }
    }

    if (check.cost == 15){
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
    for(; add.address != __table[i].address && _memcmp(&__table[i], &add, sizeof(unit)) && i < tSize; i++){}
    if (i == tSize && check(add) == 0){
        __table[tSize++] = add;
    } else if (FLAGS.UPDATE_WHEN_ADD && i < tSize && add.address == __table[i].address){
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
        for(; __table[i].address != remove.address && i < tSize; i++){}
    } else if (FLAGS.REMOVE_WITH_NEXTHOP){
        for(; __table[i].nextHop != remove.nextHop && i < tSize; i++){}
    } else if (FLAGS.REMOVE_WITH_NEXTHOP && FLAGS.REMOVE_WITH_ADDRESS){
        for(;   __table[i].nextHop != remove.nextHop &&\
                __table[i].address != remove.address &&\
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
    for(; __table[i].address != update.address; i++){}
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
        for(j = 0; add[i].address != __table[j].address && _memcmp(&add[i], &__table[j], sizeof(unit)) && j < tSize; j++){}
        if (j == tSize && check(add[i]) == 0){
            __table[tSize++] = add[i];
        } else if (FLAGS.UPDATE_WHEN_ADD && j < tSize && add[i].address == __table[j].address){
            __table[j] = add[i];
        }
    }

    return tSize;
}

int remove_units(int _size, unit* remove){
    int i = 0, j = 0;
    for (;i < _size; i++){
        if (FLAGS.REMOVE_WITH_ADDRESS){
            for(; __table[j].address != remove[i].address && j < tSize; j++){}
        } else if (FLAGS.REMOVE_WITH_NEXTHOP){
            for(; __table[j].nextHop != remove[i].nextHop && j < tSize; j++){}
        } else if (FLAGS.REMOVE_WITH_NEXTHOP && FLAGS.REMOVE_WITH_ADDRESS){
            for(;   __table[j].nextHop != remove[i].nextHop &&\
                    __table[j].address != remove[i].address &&\
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
        for(j = 0; __table[j].address != update[i].address && j < tSize; j++){}
        if (j < tSize && check(update[i]) == 0){
            __table[j] = update[i];
        }
    }

    return tSize;
}
