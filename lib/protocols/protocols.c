#include "../hardware/hardware.h"
#include "protocols.h"

int (*protocols[256])(packet p) = {
    0, 0, GET_NEIGHBOURS, DHCP, 0
};