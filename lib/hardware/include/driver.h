#ifndef ___DRIVER___
#define ___DRIVER___

#include <definitions.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned char byte;

int radio_init(float freq, byte power, byte ramptime, byte sf, byte bw, byte cr);

byte send_command(byte* cmd, byte cmdLen);
byte available();
byte clearIrqStatus();
byte writeBuffer(byte* buf, byte buflen);
byte readBuffer(byte* buf, byte buflen);
byte writeRegister(byte* buf, byte buflen, unsigned short address);
byte readRegister(byte* buf, byte buflen, unsigned short address);
byte enableIrq();
byte stopTimerOnPreamble();
byte rxPayloadLength();
byte calibrateImage();
byte calibrate();

byte setDio2AsRfSwitch();
byte setPacketTypeLora();
byte setBufferBaseAddress();
byte setSleep();
byte setStandby();
byte setFs();
byte setTx();
byte setRx();
byte setCAD();
byte setRxDutyCycle(int rxPeriod, int sleepPeriod);
byte setPaConfig();
byte setRxTxFallbackMode(byte mode);
byte setRfFrequency(float freq);
byte setTxParams(byte power, byte ramptime);
byte setModulationParams(byte sf, byte bw, byte cr);
byte setPacketParams();

byte getPacketType();
byte getStatus();
byte getRSSI();
byte getSNR();
byte getSignalRssi();
byte getRssiInst();
unsigned short getIrqStatus();

extern byte cmd[260];
extern byte status;

#ifdef __cplusplus
}
#endif

#endif