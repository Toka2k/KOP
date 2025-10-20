#ifndef ___DHCP___
#define ___DHCP___

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