int (*protocols[256])(packed_header, byte*, byte) = {
    RIP, ARP, DB, DHCP, NULL
};