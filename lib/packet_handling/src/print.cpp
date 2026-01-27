#include <Arduino.h>
#include <print.h>
#include <packet_handling.h>

void print_array(const byte *arr, size_t len){
    for (size_t i = 0; i < len; i++) {
        Serial.printf("%02X ", arr[i]);
    }
    Serial.print("\r\n");
}

void print_packet(packet* p){
    unpacked_header uh = UNPACK_HEADER(p->h);
    print("mac_d: %02X, mac_s: %02X", uh.mac_d, uh.mac_s);
    print("net_d: %02X, net_s: %02X", uh.net_d, uh.net_s);

    print("length: %02X, protocol_id: %02X, seqnum: %02X", uh.length, uh.protocol_id, uh.seqnum);
    print("hmac: %04X", (p->h.hmac[0] << 8) + p->h.hmac[1]);

    print("Data:");
    print_array(p->data, p->h.length);

    print("");
    return;
}

void print(const char *fmt, ...)
{
    char buffer[256];

    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);

    Serial.print(buffer);
    Serial.print("\r\n");
}