#include <SPI.h>
#include <Arduino.h>
#include <driver.h>

//
//      E220-400M33S Driver
//

SPISettings settings = SPISettings(4000000, MSBFIRST, SPI_MODE0);
byte cmd[260] = {0}; 

void radio_cleanup(unsigned short clearIrqParam){
    setStandby(1);
    clearIrqStatus(clearIrqParam);
    setBufferBaseAddress();
    setRxDutyCycle(0xC0, 0xC0);

    return;
}

unsigned short radio_transmit(packet* p){
    unsigned short state = 0;
    // First we have to send packet length, that other nodes expect
    // than we send the actual packet
    if ( (state = setBufferBaseAddress()) != SUCCESS ) { return state; }
    if ( (state = writeBuffer(&p->h.length, 1)) != SUCCESS) { return state; };
    if ( (state = setPacketParams(1)) != SUCCESS ) { return state; }
    setTx();
    if ( (state = setBufferBaseAddress()) != SUCCESS ) { return state; }
    if ( (state = writeBuffer((byte*)p, p->h.length + HEADER_SIZE)) != SUCCESS) { return state; }
    if ( (state = setPacketParams(p->h.length + HEADER_SIZE)) != SUCCESS ) { return state; }
    setTx();
    unsigned short irq_status = 0;
    getIrqStatus(&irq_status);
    return irq_status;
}

unsigned short radio_scanChannel(){
    clearIrqStatus(IRQ_CAD_DONE | IRQ_CAD_DETECTED);
    setCAD();
    unsigned short irq_status = 0;
    byte status = 0;
    while(irq_status != IRQ_CAD_DONE && irq_status != IRQ_TIMEOUT){
        getIrqStatus(&irq_status);
        getStatus(&status);
        Serial.print("STATUS: "); Serial.print(status, BIN); 
        Serial.print(" IRQ STATUS: "); Serial.println(irq_status, BIN);
        delay(10);
    }
    return irq_status;
}

int radio_init(float freq, byte power, byte ramptime, byte sf, byte bw, byte cr){
    // pins:
    pinMode(LORA_RXEN,  OUTPUT);
    pinMode(LORA_RST, OUTPUT);
    pinMode(LORA_NSS,   OUTPUT);
    pinMode(LORA_BUSY,  INPUT);
    pinMode(LORA_DIO1,  INPUT);

    // Resetting E220 
    digitalWrite(LORA_RST, LOW);
    delay(50);
    digitalWrite(LORA_RST, HIGH);

    SPI.begin(LORA_SCK, LORA_MISO, LORA_MOSI, LORA_NSS);

    int state = 0; 
    if ( (state = setStandby(STDBY_RC)) != SUCCESS ) { return state; } 
    if ( (state = setPacketTypeLora()) != SUCCESS ) { return state; } 
    if ( (state = setModulationParams(sf, bw, cr)) != SUCCESS ) { return state; } 
    if ( (state = setPacketParams(STDBY_RC)) != SUCCESS ) { return state; } 
    if ( (state = setRfFrequency(freq)) != SUCCESS ) { return state; } 
    if ( (state = setPaConfig()) != SUCCESS ) { return state; }
    if ( (state = setTxParams(power, ramptime)) != SUCCESS ) { return state; } 
    if ( (state = setBufferBaseAddress()) != SUCCESS ) { return state; } 
    if ( (state = setDioIrqParams(0x3f7, 0x1)) != SUCCESS ) { return state; } 
    if ( (state = setDio2AsRfSwitch()) != SUCCESS ) { return state; }

    return SUCCESS;
}

//
// BAREBONE FUNCTIONS
//

byte available(){
    if (digitalRead(LORA_BUSY) == 0){
        return 1;
    }
    return 0;
}

byte send_command(byte* cmd, byte cmdLen){
    while (available() == 0) { vTaskDelay(1); }

    SPI.beginTransaction(settings);
    digitalWrite(LORA_NSS, LOW);
    SPI.transfer(cmd, cmdLen);
    digitalWrite(LORA_NSS, HIGH);
    SPI.endTransaction();

    return SUCCESS;
}

byte clearIrqStatus(unsigned short clearIrqParam){
    cmd[0] = 0x02;
    cmd[1] = clearIrqParam >> 8;
    cmd[2] = clearIrqParam & 0xff;
    return send_command(cmd, 3);
}

byte writeBuffer(byte* buf, byte buflen){
    cmd[0] = 0x0E;
    cmd[1] = 0; // offset

    for (int i = 0; i < buflen; i++){
        cmd[i+2] = buf[i];
    }

    return send_command(cmd, buflen + 2);
}

byte readBuffer(byte* buf, byte buflen){
    cmd[0] = 0x1E;
    cmd[1] = 0x00; // offset
    cmd[2] = 0x00; // nop

    for (int i = 0; i < buflen; i++){
        cmd[i+3] = 0x00;
    }

    byte state = send_command(cmd, buflen + 3);

    for(int i = 0; i < buflen; i++){
        buf[i] = cmd[i+3];
    }

    return state;
}

byte writeRegister(byte* buf, byte buflen, unsigned short address){
    cmd[0] = 0x0D;
    cmd[1] = address >> 8;
    cmd[2] = address & 0xff;

    for (int i = 0; i < buflen; i++){
        cmd[i+3] = buf[i];
    }

    return send_command(cmd, buflen + 3);
}

byte readRegister(byte* buf, byte buflen, unsigned short address){
    cmd[0] = 0x1D;
    cmd[1] = address >> 8;
    cmd[2] = address & 0xff;
    cmd[3] = 0x00;

    for (int i = 0; i < buflen; i++){
        cmd[i+4] = 0x00;
    }

    byte state = send_command(cmd, buflen + 4);

    for (int i = 0; i < buflen; i++){
        buf[i] = cmd[i+4];
    }

    return state;
}

byte calibrateImage(){
    cmd[0] = 0x98;
    // calibrate for 430-440Mhz
    cmd[1] = 0x6B; // 430MHz
    cmd[2] = 0x6F; // 440MHz

    return send_command(cmd, 3);
}

byte calibrate(){
    setStandby(STDBY_RC);
    // calibrate all blocks of llcc68
    cmd[0] = 0x89;
    cmd[1] = 0x7f;

    return send_command(cmd, 2);
}

//
// Modes
//

byte setSleep(){
    setStandby(STDBY_RC);

    cmd[0] = 0x84;
    cmd[1] = 0x05;

    return send_command(cmd, 2);
}

byte setStandby(byte mode){
    cmd[0] = 0x80;
    if (mode != STDBY_RC && mode != STDBY_XOSC){
        return INVALID_MODE;
    }

    cmd[1] = mode; // STDBY_RC = 0, STDBY_XOSC = 1

    return send_command(cmd, 2);
}

byte setFs(){
    cmd[0] = 0xC1;

    return send_command(cmd, 1);
}

byte setTx(){
    digitalWrite(LORA_RXEN, LOW);
    cmd[0] = 0x83;
    cmd[1] = 0x00;
    cmd[2] = 0x64;
    cmd[3] = 0x00;

    return send_command(cmd, 4);
}

byte setRx(){
    digitalWrite(LORA_RXEN, HIGH);
    cmd[0] = 0x82;
    cmd[1] = 0x00;
    cmd[2] = 0x00;
    cmd[4] = 0x00;

    return send_command(cmd, 4);
}

byte setCAD(){
    cmd[0] = 0xC5;

    return send_command(cmd, 1);
}

byte setRxDutyCycle(int rxPeriod, int sleepPeriod){
    // 15,625 us steps
    cmd[0] = 0x94;
    cmd[1] = (rxPeriod >> 16);
    cmd[2] = (rxPeriod >> 8);
    cmd[3] = (rxPeriod);
    cmd[4] = (sleepPeriod >> 16);
    cmd[5] = (sleepPeriod >> 8);
    cmd[6] = (sleepPeriod);

    return send_command(cmd, 7);
}

//
// Configuration
//

byte stopTimerOnPreamble(){
    cmd[0] = 0x9f;
    cmd[1] = 0x01;

    return send_command(cmd, 2);
}

byte setDioIrqParams(unsigned short irq_mask, unsigned short dio1_mask){
    cmd[0] = 0x08;
    short irqMask = 0x03f7;
    cmd[1] = (irq_mask >> 8);
    cmd[2] = (irq_mask & 0xff);
    cmd[3] = (dio1_mask >> 8);
    cmd[4] = (irqMask & 0xff);
    cmd[5] = 0x0;
    cmd[6] = 0x0;
    cmd[7] = 0x0;
    cmd[8] = 0x0;

    return send_command(cmd, 9);
}

byte setCadParams(byte cadDetMin, byte cadDetMax, byte cadSymNum){
    cmd[0] = 0x88;
    cmd[1] = cadSymNum; // cad symbol lendth search
    cmd[2] = cadDetMin;
    cmd[3] = cadDetMax;
    cmd[4] = 0x00; // 0x00=STDBY_RC 0x01=RX
    cmd[5] = 0x00; // timeout[0] MSB
    cmd[6] = 0x02; // timeout[1]
    cmd[7] = 0x80; // timeout[2] LSB
    
    return send_command(cmd, 8);
}

byte setBufferBaseAddress(){
    cmd[0] = 0x8f;
    cmd[1] = 0;
    cmd[2] = 0;
    return send_command(cmd, 3);
}

byte setDio2AsRfSwitch(){
    cmd[0] = 0x9D;
    cmd[1] = 1;
    return send_command(cmd, 2);
}

byte setPaConfig(){
    cmd[0] = 0x95;
    cmd[1] = 0x04; // paDutyCycle
    cmd[2] = 0x07; // hpMax
    cmd[3] = 0x00;
    cmd[4] = 0x01;

    byte state = send_command(cmd, 5);

    // setting Over Current Protection - 2,5mA steps
    cmd[5] = 0xFF;
    writeRegister(cmd+5, 1, 0x08E7);
    return state;
}

byte setRxTxFallbackMode(byte mode){
    if (mode != 0x20 && mode != 0x30 && mode != 0x40) { return INVALID_MODE; }

    // 0x20 STDBY_RC    
    // 0x30 STDBY_XOSC     
    // 0x40 FS

    cmd[0] = 0x93;
    cmd[1] = mode;

    return send_command(cmd, 2);
}

byte setRfFrequency(float freq){
    if(freq < 430000000.0 || freq > 440000000.0) { return INVALID_FREQ; }
    cmd[0] = 0x86;
    *(unsigned int*)(cmd + 1) = ((float)freq / (32e6 / 33554432.0) + 0.5f);

    return send_command(cmd, 5);
}

byte setTxParams(signed char power, byte ramptime){

    if (power < -9 || power > 22) { return INVALID_POWER; }
    if (ramptime > 0x07) { return INVALID_RAMPTIME; }

    cmd[0] = 0x8E;
    cmd[1] = power; // power
    cmd[2] = ramptime; // ramp time

    return send_command(cmd, 3);
}

byte setModulationParams(byte sf, byte bw, byte cr){
    if (sf < 0x5 || sf > 0xB) { return INVALID_SF; }
    if (bw < 0x4 || bw > 0x6) { return INVALID_BW; }
    if (cr < 0x1 || cr > 0x4) { return INVALID_CR; }

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

    cmd[0] = 0x8B;

    cmd[1] = sf; // sf 5-11
    cmd[2] = bw; // bw 125khz 250khz 500khz
    cmd[3] = cr; // cr 4/5

    for (int i = 4; i < 9; i++){
        cmd[i] = 0x00;
    }

    if (sf >= 0xA) { cmd [4] = 0x1; }

    return send_command(cmd, 9);
}

byte setPacketParams(byte packet_length){
    cmd[0] = 0x8C;
    cmd[1] = 0x00; // preamble symbol length MSB
    cmd[2] = 0x0C; // preamble symbol length LSB
    cmd[3] = 0x00; // implicit header
    cmd[4] = packet_length; // packet length

    for (int i = 5; i < 10; i++){
        cmd[i] = 0x00;
    }
    
    return send_command(cmd, 10);
}

byte setPacketTypeLora(){
    cmd[0] = 0x8A;
    cmd[1] = 1;
    return send_command(cmd, 2);
}

//
//  Getters
//

byte getRxPayloadLength(){
    cmd[0] = 0x13;
    for (int i = 0; i < 3; i++){
        cmd[i + 1] = 0x00;
    }

    send_command(cmd, 4);

    return cmd[2];
}

byte getStatus(byte* status){
    cmd[0] = 0xC0;
    cmd[1] = 0;

    byte state = send_command(cmd, 2);
 
    *status = cmd[1];

    return state;
}

byte getRSSI(){
    cmd[0] = 0x14;
    
    for (int i = 1; i < 5; i++){
        cmd[i] = 0x00;
    }
    
    send_command(cmd, 5);
    return (-cmd[2]/2);
}

byte getSNR(){
    cmd[0] = 0x14;
    
    for (int i = 1; i < 5; i++){
        cmd[i] = 0x00;
    }
    
    send_command(cmd, 5);
    return (cmd[3]/4);
}

byte getSignalRssi(){
    cmd[0] = 0x14;
    
    for (int i = 1; i < 5; i++){
        cmd[i] = 0x00;
    }
    
    send_command(cmd, 5);
    return (-cmd[4]/2);
}

byte getRssiInst(){
    cmd[0] = 0x15;
    cmd[1] = 0x00;
    cmd[2] = 0x00;

    send_command(cmd, 3);

    return (-cmd[2]/2);
}

byte getPacketType(){
    cmd[0] = 0x11;
    cmd[1] = 0x00;
    cmd[2] = 0x00;
    send_command(cmd, 3);
    return cmd[2];
}

byte getIrqStatus(unsigned short* irq_status){
    cmd[0] = 0x12;  cmd[2] = 0;
    cmd[1] = 0;     cmd[3] = 0;
    byte state = send_command(cmd, 4);
    *irq_status = (cmd[2] << 8) + cmd[3];
    return state;
}