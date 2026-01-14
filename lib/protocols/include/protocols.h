#ifndef ___PROTOCOLS___
#define ___PROTOCOLS___

#include <packet_buffering.h>
#include <address_table.h>

#include <dhcp/dhcp.h>
#include <arp/arp.h>

#ifdef __cplusplus
extern "C" {
#endif

extern int (*protocols[256])(packet* p);

#ifdef __cplusplus
}
#endif
#endif