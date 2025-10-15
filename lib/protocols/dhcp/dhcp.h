#ifndef __DHCP__
#define __DHCP__

#include "../../hardware/hardware.h"

#define P_RIP   (0x0)
#define P_ARP   (0x1)
#define P_DB    (0x2)
#define P_DHCP  (0x3)

#ifdef __cplusplus
extern "C" {
#endif

extern int (*protocols[256])(byte*, byte);

int DHCP_REQ();
int DHCP_FIN();
int DHCP_ACK();
int DHCP_OFFER();
int DHCP(byte* data, byte length);

#ifdef __cplusplus
}
#endif
#endif