#include <Arduino.h>
#include <SPI.h>
#include <packet_handling.h>
#include <driver-al.h>
#include <protocols.h>
#include <WiFi.h>
#include <esp_now.h>

void setup() {
    radio_init();

    __my_address.address = 1;
    unit test = initialize_unit(0,0,0);
    for(int i = 1; i < 250; i++){
        test = initialize_unit(i,0,i);
        add_unit(test);
    }
    pinMode(2, OUTPUT);
    
    Serial.begin(115200);
    delay(500);

    xTaskCreatePinnedToCore(Transmit, "Transmit task", 16384, NULL, 3, NULL, 1);
    xTaskCreatePinnedToCore(Receive, "Receive task", 16384, NULL, 3, NULL, 1);
    xTaskCreatePinnedToCore(process_packet, "Packet processing task", 16384, NULL, 2, NULL, 0);

    delay(100);

    //DHCP_REQ();
}

void loop() {
    delay(1000);
    for(int i = 0; i < tSize; i++){
        Serial.printf("%d ", __table[i].haddress << 8 | __table[i].laddress);
    }
    Serial.println();
    vTaskDelay(30000);
}
