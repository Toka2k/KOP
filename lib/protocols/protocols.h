#include "dhcp/dhcp.h"

#ifdef __cplusplus
extern "C" {
#endif

extern int (*protocols[256])(packed_header, byte*, byte);

#ifdef __cplusplus
}
#endif