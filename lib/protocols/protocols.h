#ifndef ___PROTOCOLS___
#define ___PROTOCOLS___

#include "../hardware/hardware.h"
#include "../hardware/address_table.h"

#include "dhcp/dhcp.h"

#ifdef __cplusplus
extern "C" {
#endif

extern int (*protocols[256])(packed_header, byte*, byte);

#ifdef __cplusplus
}
#endif
#endif