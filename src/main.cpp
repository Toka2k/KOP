#include <Arduino.h>
#include <SPI.h>
#include <driver.h>
#include <hardware.h>
#include <packet_buffering.h>
#include <protocols.h>

void my_loop(void* pvParameters);
unsigned short irq_status = 0;

void radio_loop(void* pvParameters){
    void (*callback)() = (void (*)())pvParameters;

    for(;;){
        if (digitalRead(LORA_DIO1)){
            while(xSemaphoreTake(radio_mutex, portMAX_DELAY) == 0){vTaskDelay(pdMS_TO_TICKS(1));}
            irq_status = getIrqStatus();

            if (irq_status & IRQ_RX_DONE){
                
                callback();
            }

            xSemaphoreGive(radio_mutex);
        }
        
        vTaskDelay(pdMS_TO_TICKS(1));
    }
}

void setup() {
    __my_address.address = 1;


    Serial.begin(115200);
    digitalWrite(LORA_RST, LOW);
    delay(100);
    digitalWrite(LORA_RST, HIGH);

    pinMode(2, OUTPUT);
    digitalWrite(2, LOW);
    
    Serial.println("Starting init...");

    radio_init(434000000.0, 0x16, 0x1, 0x7, 0x4, 0x1);

    if (getPacketType()){
        Serial.println("LoRa packet.");
    }
    
    //xTaskCreatePinnedToCore(my_loop, "Ping pong task", 8192, NULL, 1, NULL, 0);
    //xTaskCreatePinnedToCore(Transmit, "Transmit task", 2048, NULL, 3, NULL, 1);
    //xTaskCreatePinnedToCore(radio_loop, "Radio Loop task", 4096, NULL, 2, NULL, 1);
    //xTaskCreatePinnedToCore(process_packet, "Packet processing task", 2048, NULL, 2, NULL, 0);

    Serial.print("Everything successfuly initialized.");
}

void my_loop(void* pvParamaters){
    for(;;){
        addr broadcast = {0x3fff};

        if(ECHO_REQ(broadcast) == SUCCESS){
            Serial.println("Sent arp request!");
            digitalWrite(2, HIGH);
            delay(100);
            digitalWrite(2, LOW);
        }

        if(received.buf[received.index + received.count]->h.protocol_id == P_ARP && received.buf[received.index + received.count]->data[0] == 1){
            Serial.print("Received arp reply from: ");
            unpacked_header uh = UNPACK_HEADER(received.buf[received.index + received.count]->h);
            Serial.println(uh.mac_s);
        }

        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

void loop() {
    vTaskDelay(portMAX_DELAY);
}
