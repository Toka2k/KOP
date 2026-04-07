#include <protocols.h>

#ifdef __cplusplus
extern "C" {
#endif

int (*protocols[PROTOCOLS])(packet* p) = {
    0, process_update, ARP, DHCP, EXAMPLE, 0
};

#ifdef __cplusplus
}
#endif