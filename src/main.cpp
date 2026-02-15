#include <Arduino.h>
#include <SPI.h>
#include <packet_handling.h>
#include <driver-al.h>
#include <protocols.h>
#include <WiFi.h>
#include <esp_now.h>

addr echo = {0};

void setup() {
    Serial.begin(115200);
    delay(500);
    
    radio_init();

    __my_address.address = 1;
    unit test = {0};
    switch(__my_address.address){
        case 1:
            test = initialize_unit(__my_address.address,0,__my_address.address);
            add_unit(test);
            test = initialize_unit(3,2,2);
            add_unit(test);
            test = initialize_unit(2,1,2);
            add_unit(test);
            echo.address = 3;
            break;
        case 2:
            test = initialize_unit(__my_address.address,0,__my_address.address);
            add_unit(test);
            test = initialize_unit(3,1,3);
            add_unit(test);
            test = initialize_unit(1,1,1);
            add_unit(test);
            echo.address = 0;
            break;
        case 3:
            test = initialize_unit(__my_address.address,0,__my_address.address);
            add_unit(test);
            test = initialize_unit(2,1,2);
            add_unit(test);
            test = initialize_unit(1,2,2);
            add_unit(test);
            echo.address = 0;
            break;
    }
    
    pinMode(2, OUTPUT);
    
    xTaskCreatePinnedToCore(Transmit, "Transmit task", 16384, NULL, 3, NULL, 1);
    xTaskCreatePinnedToCore(Receive, "Receive task", 16384, NULL, 3, NULL, 1);
    xTaskCreatePinnedToCore(process_packet, "Packet processing task", 16384, NULL, 2, NULL, 0);

    delay(100);

    //DHCP_REQ();
    Serial.printf("__my_address: %d\n", __my_address.address);
}

void loop() {
    delay(10);
    ECHO_REQ(echo);
    /*for(int i = 0; i < tSize; i++){
        Serial.printf("%d ", __table[i].haddress << 8 | __table[i].laddress);
    }
    Serial.println();*/
    vTaskDelay(30000);
}
