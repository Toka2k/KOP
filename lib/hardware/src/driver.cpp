#include <SPI.h>
#include <Arduino.h>
#include <driver.h>

//
//      E220-400M33S Driver
//

SPISettings settings = SPISettings(1000000, MSBFIRST, SPI_MODE0);
SemaphoreHandle_t irqSemaphore;
SemaphoreHandle_t txDoneSemaphore;
SemaphoreHandle_t rxDoneSemaphore;
SemaphoreHandle_t cadDoneSemaphore;
SemaphoreHandle_t radio_mutex;

QueueHandle_t irq_status_queue;

LLCC68_SETTINGS current_settings;

void radio_loop(void* pvParameters){
    irqSemaphore = xSemaphoreCreateBinary();
    txDoneSemaphore = xSemaphoreCreateBinary();
    rxDoneSemaphore = xSemaphoreCreateBinary();
    cadDoneSemaphore = xSemaphoreCreateBinary();

    radio_mutex = xSemaphoreCreateMutex();
    
    irq_status_queue = xQueueCreate(4, sizeof(unsigned short));

    unsigned short irq_status = 0;
    void (*callback)() = (void (*)())pvParameters;

    attachInterrupt(LORA_DIO1, dio1_isr, RISING);

    for(;;){
        xSemaphoreTake(irqSemaphore, portMAX_DELAY);
        xSemaphoreTake(radio_mutex, portMAX_DELAY);
        
        irq_status = getIrqStatus();
        clearIrqStatus(irq_status);

        xQueueSend(irq_status_queue, &irq_status, 0);
        
        if (irq_status & IRQ_TX_DONE){
            xSemaphoreGive(txDoneSemaphore);
        }
        if (irq_status & IRQ_RX_DONE){
            xSemaphoreGive(rxDoneSemaphore);
        }
        if (irq_status & IRQ_CAD_DONE){
            xSemaphoreGive(cadDoneSemaphore);
        }

        xSemaphoreGive(radio_mutex);
        vTaskDelay(pdMS_TO_TICKS(1));
    }
}

void radio_cleanup(unsigned short clearIrqParam){
    setStandby(STDBY_RC);
    clearIrqStatus(clearIrqParam);

    return;
}

unsigned short radio_transmit(packet* p){
    unsigned short irq_status = 0;

    writeBuffer(&p->h.length + HEADER_SIZE, 1);
    setPacketParams(1);
    setTx(calculate_timeout(current_settings.sf, current_settings.bw, current_settings.pl, current_settings.cr));

    xSemaphoreTake(txDoneSemaphore, portMAX_DELAY);
    xQueueReceive(irq_status_queue, &irq_status, portMAX_DELAY);

    writeBuffer((byte*)p, p->h.length + HEADER_SIZE);
    setPacketParams(p->h.length + HEADER_SIZE);
    setTx(calculate_timeout(current_settings.sf, current_settings.bw, current_settings.pl, current_settings.cr));
    
    xSemaphoreTake(txDoneSemaphore, portMAX_DELAY);
    xQueueReceive(irq_status_queue, &irq_status, portMAX_DELAY);
    return irq_status;
}

unsigned short radio_scanChannel(){
    unsigned short irq_status = 0;

    clearIrqStatus(IRQ_CAD_DONE | IRQ_CAD_DETECTED);
    setCAD();

    xSemaphoreTake(cadDoneSemaphore, portMAX_DELAY);
    xQueueReceive(irq_status_queue, &irq_status, portMAX_DELAY);
    return irq_status;
}

int radio_init(float freq, byte power, byte ramptime, byte sf, byte bw, byte cr){
    current_settings.sf = sf;
    current_settings.bw = bw;
    current_settings.cr = cr;
    current_settings.pl = 1;
    current_settings.freq = freq;

    // pins:
    pinMode(LORA_RXEN,   OUTPUT);
    pinMode(LORA_RST,   OUTPUT);
    pinMode(LORA_NSS,   OUTPUT);
    pinMode(LORA_BUSY,  INPUT);
    pinMode(LORA_DIO1,  INPUT);

    digitalWrite(LORA_RXEN, HIGH);

    // Resetting E220 
    digitalWrite(LORA_RST, LOW);
    delay(1);
    digitalWrite(LORA_RST, HIGH);
    delay(1000);

    unsigned short irq_map =  IRQ_TX_DONE 
                            | IRQ_RX_DONE 
                            | IRQ_PREAMBLE_DETECTED
                            | IRQ_CAD_DONE
                            | IRQ_CAD_DETECTED
                            | IRQ_TIMEOUT;

    SPI.begin(LORA_SCK, LORA_MISO, LORA_MOSI, LORA_NSS);
    byte sync_word[2] = {0x24, 0x24};


    setStandby(STDBY_RC);
    setRegulatorMode(DC_TO_DC);
    setPacketTypeLora();
    setDio2AsRfSwitch();
    calibrate();
    setRfFrequency(freq);
    calibrateImage();

    setBufferBaseAddress();
    setModulationParams(sf, bw, cr);
    setPacketParams(1);
    setLoRaSymbNumTimeout(2);

    setPaConfig();
    setTxParams(power, ramptime);

    setRxTxFallbackMode(0x20);
    clearIrqStatus(0xFFFF);

    setDioIrqParams(irq_map, IRQ_TX_DONE | IRQ_RX_DONE | IRQ_CAD_DONE | IRQ_TIMEOUT);
    writeRegister(sync_word, 2, 0x740);

    Serial.println("Succesful radio initialization.");

    return SUCCESS;
}

//
// BAREBONE FUNCTIONS
//

int calculate_timeout(byte sf, byte bw, byte pl, byte cr){
    int bandwidth;
    switch(bw){
        case 0x4:
            bandwidth = 125000;
            break;
        case 0x5:
            bandwidth = 250000;
            break;
        case 0x6:
            bandwidth = 500000;
            break;
    }

    float symbol_duration = (1 << sf) / bandwidth;

    float payload_symbols = ((8 * pl) - (4 * sf) + 8) / (4 * sf);
    if (payload_symbols < 0){ payload_symbols = 0; }
    else { payload_symbols += 0.5;}

    payload_symbols = round(payload_symbols) * (4 + cr); // 4 + 1 = cr
    payload_symbols += 8 + 4.25 + 12; // 12 is preamble length

    float timeout_ms = (payload_symbols * symbol_duration) * 1.5;
    int timeout = (timeout_ms * 1000) / 15.625;

    return timeout;
}

void ARDUINO_ISR_ATTR dio1_isr(){
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    xSemaphoreGiveFromISR(irqSemaphore, &xHigherPriorityTaskWoken);

    if (xHigherPriorityTaskWoken) {
        portYIELD_FROM_ISR();
    }
}

status decode_status(){
    byte _s = getStatus();
    status s;
    s.cmd_status = (_s & 0x70) >> 4;
    s.chip_mode = (_s & 0xE) >> 1;
    return s;
}

void send_command(byte* cmd, unsigned short cmdLen){
    while (digitalRead(LORA_BUSY) == HIGH) { vTaskDelay(1); }

    SPI.beginTransaction(settings);
    digitalWrite(LORA_NSS, LOW);

    SPI.transfer(cmd, cmdLen);
    
    digitalWrite(LORA_NSS, HIGH);
    SPI.endTransaction();

    while (digitalRead(LORA_BUSY) == HIGH) { vTaskDelay(1); }

    return;
}

void clearDeviceErrors(){
    byte cmd[] = {0x07, 0x00, 0x00};

    send_command(cmd, 3);
    return;
}

void clearIrqStatus(unsigned short clearIrqParam){
    byte cmd[] = {0x02, (byte)((clearIrqParam & 0xFF00) >> 8), (byte)(clearIrqParam & 0xff)};

    send_command(cmd, 3);
    return;
}

void writeBuffer(byte* buf, byte buflen){
    byte cmd[buflen + 2] = {0x0E, 0};

    for (int i = 0; i < buflen; i++){
        cmd[i+2] = buf[i];
    }

    send_command(cmd, buflen + 2);
    return;
}

void readBuffer(byte* buf, byte buflen){
    byte cmd[buflen + 3] = {0x1E, 0x00, 0x00};

    for (int i = 0; i < buflen; i++){
        cmd[i+3] = 0x00;
    }

    send_command(cmd, buflen + 3);

    for(int i = 0; i < buflen; i++){
        buf[i] = cmd[i+3];
    }

    return;
}

void writeRegister(byte* buf, byte buflen, unsigned short address){
    byte cmd[buflen + 3] = {0x0D, (byte)((address & 0xFF00) >> 8), (byte)(address & 0xFF)};

    for (int i = 0; i < buflen; i++){
        cmd[i+3] = buf[i];
    }

    send_command(cmd, buflen + 3);
    return;
}

void readRegister(byte* buf, byte buflen, unsigned short address){
    byte cmd[buflen + 4] = {0x1D, (byte)((address & 0xFF00) >> 8), (byte)(address & 0xFF), 0x00};

    for (int i = 0; i < buflen; i++){
        cmd[i+4] = 0x00;
    }

    send_command(cmd, buflen + 4);

    for (int i = 0; i < buflen; i++){
        buf[i] = cmd[i+4];
    }

    return;
}

void calibrateImage(){
    byte cmd[] = {0x98, 0x6B, 0x6F};
    send_command(cmd, 3);
    return;
}

void calibrate(){
    setStandby(STDBY_RC);
    byte cmd[] = {0x89, 0x7F};

    send_command(cmd, 2);
    return;
}

//
// Modes
//

void setSleep(){
    setStandby(STDBY_RC);
    byte cmd[] = {0x84, 0x05};

    send_command(cmd, 2);
    return;
}

void setStandby(byte mode){
    byte cmd[] = {0x80, mode};

    send_command(cmd, 2);
    return;
}

void setFs(){
    byte cmd[] = {0xC1};

    send_command(cmd, 1);
    return;
}

void setTx(){
    byte cmd[] = {0x83, 0x00, 0x64, 0x00};

    send_command(cmd, 4);
    return;
}

void setRx(int timeout){
    byte cmd[] = {0x82, (byte)((timeout & 0xFF0000) >> 16), (byte)((timeout & 0xFF00) >> 8), (byte)(timeout & 0xFF)};

    send_command(cmd, 4);
    return;
}

void setCAD(){
    byte cmd[] = {0xC5};

    send_command(cmd, 1);
    return;
}

void setRxDutyCycle(int rxPeriod, int sleepPeriod){
    // 15,625 us steps
    byte cmd[] = {0x94,
        (byte)((rxPeriod & 0xFF0000) >> 16),
        (byte)((rxPeriod & 0xFF00) >> 8),
        (byte)(rxPeriod & 0xFF),
        (byte)((sleepPeriod & 0xFF0000) >> 16),
        (byte)((sleepPeriod & 0xFF00) >> 8),
        (byte)(sleepPeriod & 0xFF)
    };

    send_command(cmd, 7);
    return;
}

//
// Configuration
//

void setLoRaSymbNumTimeout(byte symbNum){
    byte cmd[] = {0xA0, symbNum};
    
    send_command(cmd, 2);
    return;
}

void setRegulatorMode(byte mode){
    byte cmd[] = {0x96, mode};
    
    send_command(cmd, 2);
    return;
}

void stopTimerOnPreamble(){
    byte cmd[] = {0x9f, 0x01};

    send_command(cmd, 2);
    return;
}

void setDioIrqParams(unsigned short irq_mask, unsigned short dio1_mask){
    byte cmd[] = {0x08,
    (byte)((irq_mask & 0xFF00) >> 8),
    (byte)(irq_mask & 0xff),
    (byte)((dio1_mask & 0xFF00) >> 8),
    (byte)(dio1_mask & 0xff),
    0x0, 0x0, 0x0, 0x0};

    send_command(cmd, 9);
    return;
}

void setCadParams(byte cadDetMin, byte cadDetMax, byte cadSymNum){
    byte cmd[] = {0x88,
    cadSymNum, // cad symbol lendth search
    cadDetMin,
    cadDetMax,
    0x00, // 0x00=STDBY_RC 0x01=RX
    0x00, // timeout[0] MSB
    0x02, // timeout[1] step = 15,625us
    0x80, // timeout[2] LSB
    };
    
    send_command(cmd, 8);
    return;
}

void setBufferBaseAddress(){
    byte cmd[] = {0x8F, 0x00, 0x00};
    
    send_command(cmd, 3);
    return;
}

void setDio2AsRfSwitch(){
    byte cmd[] = {0x9D, 0x01};
    
    send_command(cmd, 2);
    return;
}

void setPaConfig(){
    byte cmd[] = {0x95,
    0x04, // paDutyCycle
    0x07, // hpMax
    0x00, 0x01};

    send_command(cmd, 5);

    // setting Over Current Protection - 2,5mA steps
    byte overCurrent = 0x38;
    writeRegister(&overCurrent, 1, 0x08E7);
    return;
}

void setRxTxFallbackMode(byte mode){
    // 0x20 STDBY_RC    
    // 0x30 STDBY_XOSC     
    // 0x40 FS

    byte cmd[] = {0x93, mode};

    send_command(cmd, 2);
    return;
}

void setRfFrequency(float freq){
    current_settings.freq = freq;

    byte cmd[] = {0x86, 0x00, 0x00, 0x00, 0x00};
    unsigned int steps = ((float)freq / (32e6 / 33554432.0) + 0.5f);

    cmd[1] = steps >> 24;
    cmd[2] = steps >> 16;
    cmd[3] = steps >> 8;
    cmd[4] = steps;

    send_command(cmd, 5);
    return;
}

void setTxParams(byte power, byte ramptime){

    // 0xF7 <= power <= 0x16 (-9dbm to 22dbm)
    // ramptime <= 0x07 (see the table)
    byte cmd[] = {0x8E, power, ramptime};

    send_command(cmd, 3);
    return;
}

void setModulationParams(byte sf, byte bw, byte cr){
    current_settings.sf = sf;
    current_settings.bw = bw;
    current_settings.cr = cr;
    
    // 0x05 <= sf <= 0x0B
    // 0x04 <= bw <= 0x06
    // 0x01 <= cr <= 0x04

    switch(sf){
        case 0x5:
        case 0x6:
        case 0x7: 
        case 0x8:
            setCadParams(10, 22, 2);
            break;
        case 0x9:
            setCadParams(10, 23, 4);
            break;
        case 0xA:
            setCadParams(10, 24, 4);
            break;
        case 0xB:
            setCadParams(10, 25, 4);
            break;
    }

    byte cmd[9] = {0x8B, sf, bw, cr, 0x00};

    if (((1 << sf) / bw) >= 16) { cmd [4] = 0x1; }

    send_command(cmd, 9);
    return;
}

void setPacketParams(byte packet_length){
    current_settings.pl = packet_length;

    byte cmd[10] = {0x8C, 0x00, 0x0C, 0x00, packet_length, 0x00};
    //cmd[1] = preamble symbol length MSB
    //cmd[2] = preamble symbol length LSB
    //cmd[3] = implicit header
    //cmd[4] = packet length

    send_command(cmd, 10);
    return;
}

void setPacketTypeLora(){
    byte cmd[] = {0x8A, 0x01};
    
    send_command(cmd, 2);
    return;
}

//
//  Getters
//

unsigned short getDeviceErrors(){
    byte cmd[] = {0x17, 0x00, 0x00, 0x00};

    send_command(cmd, 4);

    return ((cmd[2] << 8) | cmd[3]);
}

byte getRxPayloadLength(){
    byte cmd[] = {0x13, 0x00, 0x00, 0x00};

    send_command(cmd, 4);
    return cmd[2];
}

byte getStatus(){
    byte cmd[] = {0xC0, 0x00};

    while (digitalRead(LORA_BUSY) == HIGH) { vTaskDelay(1); }

    SPI.beginTransaction(settings);
    digitalWrite(LORA_NSS, LOW);
    SPI.transfer(cmd, 2);
    digitalWrite(LORA_NSS, HIGH);
    SPI.endTransaction();

    return cmd[1];
}

byte getRSSI(){
    byte cmd[] = {0x14, 0x00, 0x00, 0x00, 0x00};
    
    send_command(cmd, 5);
    return (-cmd[2]/2);
}

byte getSNR(){
    byte cmd[] = {0x14, 0x00, 0x00, 0x00, 0x00};
    
    send_command(cmd, 5);
    return (cmd[3]/4);
}

byte getSignalRSSI(){
    byte cmd[] = {0x14, 0x00, 0x00, 0x00, 0x00};
    
    send_command(cmd, 5);
    return (-cmd[4]/2);
}

byte getRssiInst(){
    byte cmd[] = {0x15, 0x00, 0x00};
    
    send_command(cmd, 3);
    return (-cmd[2]/2);
}

byte getPacketType(){
    byte cmd[] = {0x11, 0x00, 0x00};
    send_command(cmd, 3);
    return cmd[2];
}

unsigned short getIrqStatus(){
    byte cmd[] = {0x12, 0x00, 0x00, 0x00};

    send_command(cmd, 4);
    return ((cmd[2] << 8) + cmd[3]);
}