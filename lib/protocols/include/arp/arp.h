#ifndef ___ARP___
#define ___ARP___

#include <hardware.h>

#ifdef __cplusplus
extern "C" {
#endif

int ECHO_REQ(addr address);
int ARP(packet* p);

#ifdef __cplusplus
}
#endif

#endif