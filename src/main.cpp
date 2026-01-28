#include <Arduino.h>
#include <SPI.h>
#include <driver-al.h>
#include <address_table.h>
#include <packet_handling.h>
#include <protocols.h>

void setup() {
    __my_address.address = 1;
    add_unit(initialize_unit(__my_address.address, 0, __my_address.address));

    Serial.begin(115200);
    delay(500);

    radio_init(435000000.0, 0x16, 0x4, 0x7, 0x4, 0x1);

    xTaskCreatePinnedToCore(Transmit, "Transmit task", 8192, NULL, 3, NULL, 1);
    delay(1);
    xTaskCreatePinnedToCore(Receive, "Receive task", 8192, NULL, 3, NULL, 1);
    xTaskCreatePinnedToCore(radio_loop, "Radio Loop task", 8192, NULL, 2, NULL, 1);
    xTaskCreatePinnedToCore(process_packet, "Packet processing task", 8192, NULL, 2, NULL, 0);

    delay(100);
}

void loop() {
    addr a = {.address = LOCAL_BROADCAST};
    ECHO_REQ(a);
    vTaskDelay(30000);
}
