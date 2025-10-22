#include "../hardware/hardware.h"
#include "protocols.h"

int (*protocols[256])(packed_header, byte*, byte) = {
    0, 0, 0, DHCP, 0
};