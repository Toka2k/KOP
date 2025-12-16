#ifndef ___DRIVER___
#define ___DRIVER___

#include <definitions.h>

#ifdef __cplusplus
extern "C" {
#endif

void radio_loop(void* pvParameters);
int radio_init(float freq, byte power, byte ramptime, byte sf, byte bw, byte cr);
void radio_cleanup(unsigned short clearIrqParam);
unsigned short radio_scanChannel();
unsigned short radio_transmit(packet* p);

void ARDUINO_ISR_ATTR dio1_isr();
byte send_command(byte* cmd, byte cmdLen);
byte available();
byte clearIrqStatus(unsigned short clearIrqParam);
byte writeBuffer(byte* buf, byte buflen);
byte readBuffer(byte* buf, byte buflen);
byte writeRegister(byte* buf, byte buflen, unsigned short address);
byte readRegister(byte* buf, byte buflen, unsigned short address);
byte setDioIrqParams(unsigned short irq_mask, unsigned short dio1_mask);
byte stopTimerOnPreamble();
byte calibrateImage();
byte calibrate();

byte setCadParams(byte cadDetMin, byte cadDetMax, byte cadSymNum);
byte setDio2AsRfSwitch();
byte setPacketTypeLora();
byte setBufferBaseAddress();
byte setSleep();
byte setStandby(byte mode);
byte setFs();
byte setTx();
byte setRx();
byte setCAD();
byte setRxDutyCycle(int rxPeriod, int sleepPeriod);
byte setPaConfig();
byte setRxTxFallbackMode(byte mode);
byte setRfFrequency(float freq);
byte setTxParams(signed char power, byte ramptime);
byte setModulationParams(byte sf, byte bw, byte cr);
byte setPacketParams(byte packet_length);

byte getPacketType();
byte getStatus();
byte getRSSI();
byte getSNR();
byte getSignalRssi();
byte getRssiInst();
byte getRxPayloadLength();
byte getIrqStatus(unsigned short* irq_status);

extern byte cmd[260];
extern SemaphoreHandle_t irqSemaphore;
extern SemaphoreHandle_t radio_mutex;
#ifdef __cplusplus
}
#endif

#endif