#ifndef ___DRIVER___
#define ___DRIVER___

#include <definitions.h>

#ifdef __cplusplus
extern "C" {
#endif

void radio_loop(void* pvParameters);
void radio_cleanup(unsigned short clearIrqParam);
unsigned short radio_transmit(packet* p);
unsigned short radio_scanChannel();
int radio_init(float freq, byte power, byte ramptime, byte sf, byte bw, byte cr);

// Basic functions for chip communictaion
int calculate_timeout(byte sf, byte bw, byte pl, byte cr);
void ARDUINO_ISR_ATTR dio1_isr();
status decode_status();
void send_command(byte* cmd, unsigned short cmdLen);
void clearDeviceErrors();
void clearIrqStatus(unsigned short clearIrqParam);
void writeBuffer(byte* buf, byte buflen);
void readBuffer(byte* buf, byte buflen);
void writeRegister(byte* buf, byte buflen, unsigned short address);
void readRegister(byte* buf, byte buflen, unsigned short address);
void calibrateImage();
void calibrate();

// Modes
void setSleep();
void setStandby(byte mode);
void setFs();
void setTx(int timeout, byte pl);
void setRx(int timeout, byte pl);
void setCAD();
void setRxDutyCycle(int rxPeriod, int sleepPeriod);

// Configuration
void setLoRaSymbNumTimeout(byte symbNum);
void setRegulatorMode(byte mode);
void stopTimerOnPreamble();
void setDioIrqParams(unsigned short irq_mask, unsigned short dio1_mask);
void setCadParams(byte cadDetMin, byte cadDetMax, byte cadSymNum);
void setBufferBaseAddress();
void setDio2AsRfSwitch();
void setPaConfig();
void setRxTxFallbackMode(byte mode);
void setRfFrequency(float freq);
void setTxParams(byte power, byte ramptime);
void setModulationParams(byte sf, byte bw, byte cr);
void setPacketParams(byte packet_length);
void setPacketTypeLora();

unsigned short getDeviceErrors();
byte getRxPayloadLength();
byte getStatus();
byte getRSSI();
byte getSNR();
byte getSignalRSSI();
byte getRssiInst();
byte getPacketType();
unsigned short getIrqStatus();

extern SemaphoreHandle_t irqSemaphore;
extern SemaphoreHandle_t txDoneSemaphore;
extern SemaphoreHandle_t rxDoneSemaphore;
extern SemaphoreHandle_t cadDoneSemaphore;
extern SemaphoreHandle_t radio_mutex;

extern QueueHandle_t irq_status_queue;

#ifdef __cplusplus
}
#endif

#endif