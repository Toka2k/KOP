#include <protocols.h>

int (*protocols[PROTOCOLS])(packet* p) = {
    0, process_update, ARP, DHCP, 0
};