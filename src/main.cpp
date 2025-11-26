#include <Arduino.h>
#include <SPI.h>
#include <hardware.h>
#include <packet_buffering.h>
#include <protocols.h>
#include <RadioLib.h>

extern LLCC68 radio;
void my_loop(void* pvParameters);

void callback_check(void* pvParameters){
    radio.startReceive();
    for(;;){
        if(radio.available() && radio.getPacketLength() > 12){
            Receive();
        }
        vTaskDelay(1);
    }
}

void setup() {
    __my_address.address = 1;

    Serial.begin(9600);

    pinMode(2, OUTPUT);
    digitalWrite(2, LOW);
    
    SPI.begin(18, 19, 23, 5);
    
    // pins:
    pinMode(LORA_RXEN,  OUTPUT);
    pinMode(LORA_TXEN,  OUTPUT);
    pinMode(LORA_NSS,   OUTPUT);
    pinMode(LORA_RST,   OUTPUT);
    pinMode(LORA_BUSY,  INPUT);
    pinMode(LORA_DIO1,  INPUT);

    digitalWrite(LORA_RST, LOW);
    delay(5);
    digitalWrite(LORA_RST, HIGH);

    pinMode(LORA_RST, INPUT);

    delay(2000);
    int state = radio.begin(433.0, 125.0, 7, 4, RADIOLIB_SX126X_SYNC_WORD_PRIVATE, 33, 8, 0, false);
    if (state != RADIOLIB_ERR_NONE) {
        Serial.print("Init failed: ");
        Serial.println(state);
        int i = 0;
        while (true){
            i++;
            digitalWrite(2, HIGH);
            delay(500);
            digitalWrite(2,LOW);
            delay(500);
            if(i == 3){
                return;
            }
        }
    }

    radio.setPacketReceivedAction(Receive);
    //xTaskCreatePinnedToCore(Transmit, "Transmit task", 2048, NULL, 2, NULL, 1);
    //xTaskCreatePinnedToCore(callback_check, "Radio Loop task", 4096, NULL, 3, NULL, 1);
    //xTaskCreatePinnedToCore(process_packet, "Packet processing task", 2048, NULL, 3, NULL, 0);
    //xTaskCreatePinnedToCore(my_loop, "Ping pong task", 8192, NULL, 3, NULL, 0);

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

        vTaskDelay(500);
    }
}

void loop() {
    vTaskDelay(portMAX_DELAY);
}
