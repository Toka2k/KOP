#ifndef ___PROTOCOLS___
#define ___PROTOCOLS___

#include <address_table.h>
#include <definitions.h>

#include <dhcp/dhcp.h>
#include <arp/arp.h>
#include <db/db.h>

#ifdef __cplusplus
extern "C" {
#endif

extern int (*protocols[256])(packet* p);

#ifdef __cplusplus
}
#endif
#endif