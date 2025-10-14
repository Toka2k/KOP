#ifndef ___ROUTING___
#define ___ROUTING___

#ifdef __cplusplus
extern "C" {
#endif

#include "../hardware/hardware.h"
#include <string.h>


extern int (*protocols[256])(byte*, byte);

int ask_for_address();
int get_address(byte* data, byte length);

#ifdef __cplusplus
}
#endif

#endif
