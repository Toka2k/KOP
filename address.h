#ifndef ___ADDRESS___
#define ___ADDRESS___

#define MAX_TABLE_SIZE (1 << ADDRESS_BITS)
#define tSize __table_size.size
#define INDEX_OUT_OF_BOUNDS -1
#define ADDRESS_BITS 14
#define RESERVED_ADDRESSES 2
#define SUCCESS 0

typedef struct {
    unsigned int address : ADDRESS_BITS; 
    unsigned int cost : 4;
    unsigned int nextHop : ADDRESS_BITS;
} unit;

typedef struct {
    unsigned short int UPDATE_WHEN_ADD : 1;
    unsigned short int REMOVE_WITH_ADDRESS : 1;
    unsigned short int REMOVE_WITH_NEXTHOP : 1;
} flags;

typedef struct {
    unsigned short int size: ADDRESS_BITS;
} size;

static unit __table[MAX_TABLE_SIZE] = {0};
static flags FLAGS = {0};
static size __table_size = {0};
static unit __reserved_addresses[] = {
    {0,0,0},
    {16383,0,0}
};

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
