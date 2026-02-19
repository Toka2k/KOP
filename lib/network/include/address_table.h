#ifndef ___ADDRESS_TABLE___
#define ___ADDRESS_TABLE___

#include <definitions.h>
#include <stdlib.h>

#define tSize __table_size.size

#ifdef __cplusplus
extern "C"{
#endif

extern unit null;
extern flags FLAGS;
extern size __table_size;
extern unit* __table;
extern addr __highest_address;
extern addr __my_address;
extern int* routers;

void init_address_table();
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