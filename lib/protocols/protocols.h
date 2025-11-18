#ifndef ___PROTOCOLS___
#define ___PROTOCOLS___

#include "../hardware/hardware.h"
#include "../hardware/address_table.h"

#include "dhcp/dhcp.h"
#include "get_neighbours/get_neighbours.h"

#ifdef __cplusplus
extern "C" {
#endif

extern int (*protocols[256])(packet p);

#ifdef __cplusplus
}
#endif
#endif