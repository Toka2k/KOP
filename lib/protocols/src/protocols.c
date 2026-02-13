#include <protocols.h>

int (*protocols[PROTOCOLS])(packet* p) = {
    0, DB, ARP, DHCP, 0
};