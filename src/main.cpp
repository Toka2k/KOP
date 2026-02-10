#include <Arduino.h>
#include <SPI.h>
#include <driver-al.h>
#include <packet_handling.h>
#include <protocols.h>

void setup() {
    __my_address.address = 1;
    pinMode(2, OUTPUT);
    
    Serial.begin(115200);
    delay(500);

    radio_init(435000000.0, 0x16, 0x4, 0x7, 0x4, 0x1);

    //xTaskCreatePinnedToCore(Transmit, "Transmit task", 2048, NULL, 3, NULL, 1);
    //delay(1);
    //xTaskCreatePinnedToCore(Receive, "Receive task", 2048, NULL, 3, NULL, 1);
    //xTaskCreatePinnedToCore(radio_loop, "Radio Loop task", 2048, (void*) Receive, 2, NULL, 1);
    //xTaskCreatePinnedToCore(process_packet, "Packet processing task", 2048, NULL, 2, NULL, 0);

    delay(100);
}


void loop() {
    vTaskDelay(30000);
}
