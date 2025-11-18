#ifndef ___DHCP___
#define ___DHCP___

#include "../../hardware/hardware.h"
#include "../../hardware/address_table.h"
#include "../../hardware/definitions.h"

#ifdef __cplusplus
extern "C" {
#endif

int DHCP_REQ();
int DHCP_OFFER(byte* data);
int DHCP_ACK(packet p);
int DHCP_FIN(packet p);
int DHCP_ACC(packed_header ph);
int DHCP_DENY();
int DHCP_DROP();
int DHCP(packet p);

#ifdef __cplusplus
}
#endif
#endif