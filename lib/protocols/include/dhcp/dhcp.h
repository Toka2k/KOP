#ifndef ___DHCP___
#define ___DHCP___

#include <packet_handling.h>
#include <address_table.h>
#include <definitions.h>

#ifdef __cplusplus
extern "C" {
#endif

int DHCP_REQ();
int DHCP(packet* p);

#ifdef __cplusplus
}
#endif
#endif