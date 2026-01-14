#include <Arduino.h>
#include <SPI.h>
#include <driver-al.h>
#include <packet_handling.h>
#include <packet_buffering.h>
#include <protocols.h>

void setup() {
    __my_address.address = 1;

    Serial.begin(115200);
    delay(500);
    Serial.println("Starting init...");

    radio_init(435000000.0, 0x16, 0x4, 0x7, 0x4, 0x1);
    Serial.println(getStatus(), BIN);

    //xTaskCreatePinnedToCore(Transmit, "Transmit task", 2048, NULL, 3, NULL, 1);
    //xTaskCreatePinnedToCore(Receive, "Receive task", 2048, NULL, 3, NULL, 1);
    xTaskCreatePinnedToCore(radio_loop, "Radio Loop task", 4096, (void*) Receive, 2, NULL, 1);
    //xTaskCreatePinnedToCore(process_packet, "Packet processing task", 2048, NULL, 2, NULL, 0);
    //Serial.println(radio_scanChannel(), BIN);
}


void loop() {
    vTaskDelay(portMAX_DELAY);
}
