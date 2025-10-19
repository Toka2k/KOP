#include "../hardware/hardware.h"
#include "protocols.h"

int (*protocols[256])(packed_header, byte*, byte) = {
    /*RIP, ARP, DB, */0, 0, 0, DHCP, 0
};