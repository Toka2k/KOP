#include <Arduino.h>
#include <SPI.h>
#include <hardware.h>
#include <protocols.h>
#include <RadioLib.h>

extern LLCC68 radio;

void callback_check(void* pvParameters){
    radio.startReceive();
    digitalWrite(LORA_RXEN, HIGH);
    digitalWrite(LORA_TXEN, LOW);
    for(;;){
        if(radio.available() && radio.getPacketLength() > 12){
            Receive();
        }
        vTaskDelay(1);
    }
}

void setup() {

    // pins:
    pinMode(LORA_RXEN,  OUTPUT);
    pinMode(LORA_TXEN,  OUTPUT);

    SPI.begin(LORA_SCK, LORA_MISO, LORA_MOSI);

    int state = radio.begin(433.0, 125.0, 7, 4, RADIOLIB_SX126X_SYNC_WORD_PRIVATE, 33, 8, 0, false);   // Or 915.0
    if (state != RADIOLIB_ERR_NONE) {
        Serial.print("Init failed: ");
        Serial.println(state);
        while (true);
    }

    radio.setPacketReceivedAction(Receive);
    xTaskCreatePinnedToCore(Transmit, "Transmit task", 2048, NULL, 2, NULL, 1);
    xTaskCreatePinnedToCore(callback_check, "Radio Loop task", 4096, NULL, 3, NULL, 1);
    xTaskCreatePinnedToCore(process_packet, "Packet processing task", 2048, NULL, 3, NULL, 0);
}

void loop() {
    vTaskDelay(portMAX_DELAY);
}
