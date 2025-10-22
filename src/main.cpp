#include <Arduino.h>
#include <hardware.h>
#include <protocols.h>

void setup() {
    Serial.begin(9600);
}

void loop() {

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

    Serial.println(*(unsigned short*)uh.hmac);
    Serial.println(HASH_PH(ph));
    Serial.println("--------");


    delay(1000);
}
