#include <hardware.h>
#include <protocols.h>

int (*protocols[256])(packet* p) = {
    0, 0, ARP, DHCP, 0
};