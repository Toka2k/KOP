#ifndef ___ADDRESS_TABLE___
#define ___ADDRESS_TABLE___

#define MAX_TABLE_SIZE (1 << ADDRESS_BITS)
#define tSize __table_size.size
#define INDEX_OUT_OF_BOUNDS -1
#define INCORRECT_LENGTH -2
#define ADDRESS_BITS 14
#define RESERVED_ADDRESSES 2
#define SUCCESS 0 

typedef unsigned char byte;

typedef struct {
    byte hcost : 2;
    byte haddress : 6;
    byte lcost : 2;
    byte hnextHop : 6;
    byte cost;
    byte laddress;
    byte lnextHop;
} unit;

typedef struct {
    unsigned short int UPDATE_WHEN_ADD : 1;
    unsigned short int REMOVE_WITH_ADDRESS : 1;
    unsigned short int REMOVE_WITH_NEXTHOP : 1;
} flags;

typedef struct {
    unsigned short int size: ADDRESS_BITS;
} size;

typedef struct {
    unsigned short address: ADDRESS_BITS;
} addr;

extern unit __table[MAX_TABLE_SIZE];
extern flags FLAGS;
extern size __table_size;
extern unit __reserved_addresses[];
extern addr __highest_address;

int check(unit check);
int _memcmp(const void* buf1, const void* buf2, int count);
int add_unit(unit add);
int remove_unit(unit remove);
int update_unit(unit update);
void clear_table();
int add_units(int _size, unit* add);
int remove_units(int _size, unit* toRemove);
int update_units(int _size, unit* update);
#endif
