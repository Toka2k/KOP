#ifndef ___ROUTING___
#define ___ROUTING___

#ifdef __cplusplus
extern "C" {
#endif

#include "../RadioLib/src/modules/LLCC68/LLCC68.h"
#include "../hardware/hardware.h"
#include <string.h>

#define SECRET_COUNT 1



extern int (*protocols[256])(byte*, byte);
extern byte secret[SECRET_COUNT];

unsigned short HASH_PH(packed_header ph);
unsigned short HASH_UH(unpacked_header uh);
int ask_for_address();
int get_address(byte* data, byte length);

#ifdef __cplusplus
}
#endif

#endif
