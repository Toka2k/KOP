#include <Arduino.h>
#include <hardware.h>
#include <protocols.h>

void setup() {
    Serial.begin(9600);
    
    while(Serial.available() == 0){}
    
    String input = Serial.readStringUntil('\n');
    input.trim();

    unpacked_header uh = {0};

    int a, b, c, d;
    int count = sscanf(input.c_str(), "%d %d %d %d", &a, &b, &c, &d);

    uh.mac_d = a;
    uh.mac_s = b;
    uh.net_d = c;
    uh.net_s = d;

    packed_header ph = PACK_HEADER(uh);
    uh = UNPACK_HEADER(ph);

    char buf[256];
    sprintf(buf, "initial hash: %d. after conversion hash: %d\n", *(unsigned short*)uh.hmac, HASH_PH(ph));
    Serial.println(buf);
    Serial.println("--------");
    sprintf(buf, "uh.mac_d: %d\t\tph.mac_d: %d", uh.mac_d, ph.addresses[0] << 6 | (ph.addresses[1] & 0xfc) >> 2);
    Serial.println(buf); 
    sprintf(buf, "uh.mac_s: %d\t\tph.mac_s: %d", uh.mac_s, (ph.addresses[1] & 0x3) << 12 | ph.addresses[2] << 4 | (ph.addresses[3] & 0xf0) >> 4);
    Serial.println(buf); 
    sprintf(buf, "uh.net_d: %d\t\tph.net_d: %d", uh.net_d, (ph.addresses[3] & 0xf) << 10 | ph.addresses[4] << 2 | (ph.addresses[5] & 0xc0) >> 6);
    Serial.println(buf); 
    sprintf(buf, "uh.net_s: %d\t\tph.net_s: %d", uh.net_s, (ph.addresses[5] & 0x3f) << 8 | ph.addresses[6]);
    Serial.println(buf);
}

void loop() {
}
