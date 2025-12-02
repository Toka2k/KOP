#include <Arduino.h>
#include <SPI.h>
#include <hardware.h>
#include <packet_buffering.h>
#include <protocols.h>

void my_loop(void* pvParameters);

void callback_check(void* pvParameters){
    setRx();
    for(;;){
        if(available() && rxPayloadLength() > 12 && getIrqStatus() & IRQ_RX_DONE){
            Receive();
        }
        vTaskDelay(1);
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

    // pins:
    pinMode(LORA_RXEN,  INPUT_PULLDOWN);
    pinMode(LORA_RST, INPUT_PULLUP);
    pinMode(LORA_NSS,   OUTPUT);
    pinMode(LORA_BUSY,  INPUT);
    pinMode(LORA_DIO1,  INPUT);

    SPI.begin(LORA_SCK, LORA_MISO, LORA_MOSI, LORA_NSS);
    setStandby();
    Serial.print("setStandby() -> "); Serial.println(cmd[0], HEX);

    setPacketTypeLora();
    Serial.print("setPacketTypeLora() -> "); Serial.println(cmd[0], HEX);
    

    Serial.print("Instant RSSI: "); Serial.println(getRssiInst());

    setRfFrequency(434000000.0);
    setPaConfig();
    setTxParams();
    setBufferBaseAddress();
    setModulationParams();
    setPacketParams();

    if (getPacketType()){
        Serial.println("LoRa packet.");
    }
    
    //xTaskCreatePinnedToCore(my_loop, "Ping pong task", 8192, NULL, 3, NULL, 0);
    //xTaskCreatePinnedToCore(Transmit, "Transmit task", 2048, NULL, 2, NULL, 1);
    //xTaskCreatePinnedToCore(callback_check, "Radio Loop task", 4096, NULL, 3, NULL, 1);
    //xTaskCreatePinnedToCore(process_packet, "Packet processing task", 2048, NULL, 3, NULL, 0);

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
