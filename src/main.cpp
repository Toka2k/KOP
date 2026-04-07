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
    
    __my_address.address = 1;
    unit test = initialize_unit(__my_address.address,0,__my_address.address);
    add_unit(test);
    Serial.printf("__my_address: %d\n", __my_address.address);
    
    init_updates();

    pinMode(2, OUTPUT);
    
    delay(100);

}

void loop() {
    char* table = print_table(0);
    const char* text = "Hello world from node 2!";
    addr broadcast = {LOCAL_BROADCAST};

    switch(__my_address.address){
        case 1:
            debug = 1;
            Serial.printf(table);
            free(table);
            break;
        case 2:
            route(broadcast, strlen(text), P_EXAMPLE, (byte*)text);
            break;
        case 3:
            debug = 1;
            break;
    }

    vTaskDelay(10000);
}
