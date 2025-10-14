#include <Arduino.h>
#include <LoRaWWAN.h>

void setup() {
    unit a = initialize_unit(1024, 14, 1023);
    add_unit(a);
}

void loop() {
}