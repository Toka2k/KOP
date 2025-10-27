#ifndef ___ADDRESS_TABLE___
#define ___ADDRESS_TABLE___

#define MAX_TABLE_SIZE (1 << ADDRESS_BITS)
#define tSize __table_size.size
#define INDEX_OUT_OF_BOUNDS -1
#define INCORRECT_LENGTH -2
#define ADDRESS_BITS 14
#define RESERVED_ADDRESSES 2

// If ICACHE_RAM_ATTR is defeined this function will be placed in instruction ram which is used
// for time sensitivie functions
/*#if defined(ESP8266) || defined(ESP32)
    ICACHE_RAM_ATTR
#endif*/
#ifdef __cplusplus
extern "C"{
#endif

typedef unsigned char byte;

typedef struct __attribute__((packed)){
    unsigned hcost : 2;
    unsigned haddress : 6;
    unsigned lcost : 2;
    unsigned hnextHop : 6;
    byte cost;
    byte laddress;
    byte lnextHop;
} unit;

typedef struct __attribute__((packed)){
    unsigned short int UPDATE_WHEN_ADD : 1;
    unsigned short int REMOVE_WITH_ADDRESS : 1;
    unsigned short int REMOVE_WITH_NEXTHOP : 1;
} flags;

typedef struct __attribute__((packed)){
    unsigned short int size: ADDRESS_BITS;
} size;

typedef struct __attribute__((packed)){
    unsigned short address: ADDRESS_BITS;
} addr;

extern flags FLAGS;
extern size __table_size;
extern addr __highest_address;
extern addr __my_address;
extern int routers[1 << (ADDRESS_BITS - 5)];

int check(unit check);
int _memcmp(const void* buf1, const void* buf2, int count);
int add_unit(unit add);
int remove_unit(unit remove);
int update_unit(unit update);
void clear_table();
int add_units(int _size, unit* add);
int remove_units(int _size, unit* toRemove);
int update_units(int _size, unit* update);
unit initialize_unit(unsigned short addr, unsigned short cost, unsigned short nextHop);
unit find_unit(addr address);

#ifdef __cplusplus
}
#endif
#endif