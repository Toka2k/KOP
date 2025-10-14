#ifndef __DHCP__
#define __DHCP__

#include "../../hardware/hardware.h"

#ifdef __cplusplus
extern "C" {
#endif

int DHCP_SYNACK();
int DHCP_ACK();
int DHCP_LEASE();
int DHCP(byte* data, byte length);

#ifdef __cplusplus
}
#endif
#endif