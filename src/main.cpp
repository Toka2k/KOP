#include <Arduino.h>
#include <SPI.h>
#include <driver.h>
#include <hardware.h>
#include <packet_buffering.h>
#include <protocols.h>

void my_loop(void* pvParameters);

void func_check(int state, const char* name){
    if(state == SUCCESS){
        Serial.print("Successful "); Serial.println(name);
    } else{
        Serial.print(name); Serial.print(" returned "); Serial.println(state);
    }

    return;
}

int driver_test(){
    unsigned short irq_status = 0;

    int count = 0;
    digitalWrite(LORA_RST, LOW);
    delay(100);
    digitalWrite(LORA_RST, HIGH);
    while(digitalRead(LORA_BUSY) == 1){
        count++;
        delayMicroseconds(10);
    }
    Serial.println(count);

    //
    // Barebone functions test
    //



    Serial.print("Initial: "); getIrqStatus(&irq_status); Serial.println(irq_status);
    Serial.println("Clearing..."); clearIrqStatus(0xffff);
    Serial.print("Cleared: "); getIrqStatus(&irq_status); Serial.println(irq_status);

    func_check(setBufferBaseAddress(), "setBufferBaseAddress();");

    byte data[10] = {0,1,2,3,4,5,6,7,8,9};

    writeBuffer(data, 10);
    readBuffer(data, 10);

    for(int i = 0; i < 10; i++){
        Serial.print(data[i]); Serial.print(" ");
    }
    Serial.println();

    byte sync_word[2] = {0x12,0x25};
    //writeRegister(sync_word, 2, 0x740);
    sync_word[0] = 0; sync_word[1] = 0;
    for (int i = 0; i < 16; i++) cmd[i] = 0x55;
    readRegister(sync_word, 2, 0x740);
    Serial.print("0x"); Serial.print(sync_word[0], HEX); Serial.print(" 0x"); Serial.println(sync_word[1], HEX);

    func_check(calibrate(), "calibrate();");
    func_check(calibrateImage(), "calibrateImage();");

    //
    // mode tests:
    //

    func_check(setSleep(),"setSleep();");
    delay(5000);
    // wake 
    digitalWrite(LORA_NSS, LOW);
    delay(1);
    digitalWrite(LORA_NSS, HIGH);

    func_check(setStandby(STDBY_RC), "setStandby(STDBY_RC);");
    func_check(setFs(),"setFs();");
    // func_check(setTx(), "setTx();");
    func_check(setRx(), "setRx();");
    func_check(setCAD(), "setCAD();");
    func_check(setRxDutyCycle(0xC0, 0xC0), "setRxDutyCycle(0xC0, 0xC0);");

    // configuration settings:
    func_check(stopTimerOnPreamble(), "stopTimerOnPreamble();");
    func_check(setDioIrqParams(0xFFFF, 0xFFFF), "setDioIrqParams(0x3F7, 0x3F7);");
    func_check(setCadParams(10, 22, 2), "setCadParams(10, 22, 2);");
    func_check(setDio2AsRfSwitch(), "setDio2AsRfSwitch();");
    func_check(setPaConfig(),"setPaConfig();");
    func_check(setRxTxFallbackMode(0x20),"setRxTxFallbackMode();");
    func_check(setRfFrequency(433000000.0),"setRfFrequency(433000000.0);");
    func_check(setTxParams(0x16, 0x04),"setTxParams(0x16, 0x04);");
    func_check(setModulationParams(0x7, 0x4, 0x1),"setModulationParams(0x7, 0x4, 0x1);");
    func_check(setPacketParams(10),"setPacketParams(10);");
    func_check(setPacketTypeLora(),"setPacketTypeLora();");

    // getters test:
    byte status = 0;
    Serial.print("Payload Length: "); Serial.println(getRxPayloadLength());
    Serial.print("Status: "); status = getStatus(); Serial.println(status, BIN);
    Serial.print("RSSI: "); Serial.println(getRSSI()/-2);
    Serial.print("SNR: "); Serial.println(getSNR()/-2);
    Serial.print("SIGNAL RSSI: "); Serial.println(getSignalRssi()/-2);
    Serial.print("INSTANT RSSI: "); Serial.println(getRssiInst()/-2);
    Serial.print("Packet Type: "); Serial.println(getPacketType());

    Serial.println(radio_scanChannel());

    
    return 0;
}

void setup() {
    __my_address.address = 1;

    attachInterrupt(LORA_DIO1, dio1_isr, RISING);

    Serial.begin(115200);
    Serial.println("Starting init...");

    radio_init(433000000.0, 0x16, 0x4, 0x7, 0x4, 0x1);

    setCAD();
    for(;;){
        if(xSemaphoreTake(irqSemaphore, portMAX_DELAY)){
            getIrqStatus(&irq_status);
            Serial.println(irq_status, HEX);
            clearIrqStatus(irq_status);
        }
    }

    //xTaskCreatePinnedToCore(my_loop, "Ping pong task", 8192, NULL, 1, NULL, 0);
    //xTaskCreatePinnedToCore(Transmit, "Transmit task", 2048, NULL, 3, NULL, 1);
    //xTaskCreatePinnedToCore(radio_loop, "Radio Loop task", 4096, NULL, 2, NULL, 1);
    //xTaskCreatePinnedToCore(process_packet, "Packet processing task", 2048, NULL, 2, NULL, 0);
    
    Serial.println("Everything successfuly initialized.");
}

void my_loop(void* pvParamaters){
    Serial.println("setup in my loop");
    for(;;){
        Serial.println("Iteration");
        while(xSemaphoreTake(radio_mutex, portMAX_DELAY) == 0){
            Serial.println("Waiting for mutex.");
            delay(500);
        };
       
        // TRANSMIT
        digitalWrite(LORA_RXEN, LOW);
        unpacked_header uh = {2,1,2,1,1,0};
        packed_header ph = PACK_HEADER(uh);
        byte b = 3;
        packet p = packet_init(ph, &b);

        int state = 0;
        if ( (state = setBufferBaseAddress()) != SUCCESS ) {Serial.print("state: "); Serial.println(state);}
        if ( (state = writeBuffer((byte*)&p, 13)) != SUCCESS) {Serial.print("state: "); Serial.println(state);};
        if ( (state = setPacketParams(13)) != SUCCESS ) {Serial.print("state: "); Serial.println(state);}
        setTx();
        // END TRANSMISSION

        delay(1000);
        state = digitalRead(LORA_DIO1);

        if (state == 1){
            Serial.println("Successful transmission.");
        } else {
            Serial.println("Something went wrong.");
        }

        xSemaphoreGive(radio_mutex);

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void loop() {
    vTaskDelay(portMAX_DELAY);
}
