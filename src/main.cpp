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
    
    init_network();
    init_updates();

    DHCP_REQ();
    int timer = millis();
    while(millis() - timer < 5000){if (__my_address.address != 0) { break; }}
    if(__my_address.address == 0){
        __my_address.address = 1;
    }
    unit test = initialize_unit(__my_address.address,0,__my_address.address);
    add_unit(test);
    Serial.printf("__my_address: %d\n", __my_address.address);

    pinMode(2, OUTPUT);
    
    delay(100);

}

void loop() {
    char* table = print_table(0);
    Serial.printf(table);
    Serial.printf("%d\n", TABLE_SIZE);
    free(table);

    vTaskDelay(10000);
}
