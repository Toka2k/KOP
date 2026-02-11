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

    xTaskCreatePinnedToCore(Transmit, "Transmit task", 2048, NULL, 3, NULL, 1);
    delay(1);
    xTaskCreatePinnedToCore(Receive, "Receive task", 2048, NULL, 3, NULL, 1);
    xTaskCreatePinnedToCore(radio_loop, "Radio Loop task", 2048, (void*) Receive, 2, NULL, 1);
    xTaskCreatePinnedToCore(process_packet, "Packet processing task", 2048, NULL, 2, NULL, 0);
    xTaskCreatePinnedToCore(i2c_rx_task, "i2c_rx", 4096, NULL, 10, NULL, 1);
    delay(100);
}


void loop() {
    vTaskDelay(30000);
}
