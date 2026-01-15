#include <packet_handling.h>
#include <protocols.h>

int (*protocols[256])(packet* p) = {
    0, DB, ARP, DHCP, 0
};