#ifndef ___DHCP___
#define ___DHCP___

#include "../../hardware/hardware.h"
#include "../../hardware/address_table.h"
#include "../../hardware/definitions.h"

#ifdef __cplusplus
extern "C" {
#endif

int DHCP_REQ();
int DHCP(packet* p);

#ifdef __cplusplus
}
#endif
#endif