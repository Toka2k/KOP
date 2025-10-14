#ifndef __DHCP__
#define __DHCP__

#include "../../hardware/hardware.h"

int DHCP_SYNACK();
int DHCP_ACK();
int DHCP_LEASE();
int DHCP(byte* data, byte length);

#endif