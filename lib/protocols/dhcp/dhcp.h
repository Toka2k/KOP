#ifndef ___DHCP___
#define ___DHCP___

#define P_RIP   (0x0)
#define P_ARP   (0x1)
#define P_DB    (0x2)
#define P_DHCP  (0x3)

#include "../../hardware/hardware.h"
#include "../../hardware/definitions.h"

#ifdef __cplusplus
extern "C" {
#endif

int DHCP_REQ();
int DHCP_OFFER(byte* data);
int DHCP_ACK(packed_header ph, byte* data, byte length);
int DHCP_FIN(packed_header ph, byte* data, byte length);
int DHCP_ACC(packed_header ph);
int DHCP(packed_header ph, byte* data, byte length);

#ifdef __cplusplus
}
#endif
#endif