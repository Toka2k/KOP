#include <Arduino.h>
#include <driver-al.h>
#include <packet_handling.h>
#include <protocols.h>
#include <WiFi.h>
#include <esp_now.h>

void setup() {
    Serial.begin(115200);
    delay(500);

    radio_init();
    init_address_table();
    
    __my_address.address = 2;
    unit test = initialize_unit(__my_address.address,0,__my_address.address);
    add_unit(test);
    Serial.printf("__my_address: %d\n", __my_address.address);
    
    init_network();
    init_updates();

    pinMode(2, OUTPUT);
    
    delay(100);

}

void loop() {
    delay(10);
    for(int i = 0; i < tSize; i++){
        Serial.printf("%d ", __table[i].haddress << 8 | __table[i].laddress);
    }
    Serial.println();

    Serial.printf("Free heap: %u bytes\n", ESP.getFreeHeap());
    //Serial.printf("Min free heap: %u bytes\n", ESP.getMinFreeHeap());
    //Serial.printf("Max alloc heap: %u bytes\n", ESP.getMaxAllocHeap());

    vTaskDelay(3000);
}
