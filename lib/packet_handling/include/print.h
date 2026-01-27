#ifndef ___PRINT___
#define ___PRINT___

#include <stdarg.h>
#include <stdio.h>
#include <definitions.h>

#ifdef __cplusplus
extern "C"{
#endif

void print(const char* fmt, ...);
void print_packet(packet* p);
void print_array(const byte *arr, size_t len);

#ifdef __cplusplus
}
#endif
#endif