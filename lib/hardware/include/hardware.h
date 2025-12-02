#ifndef ___HARDWARE___
#define ___HARDWARE___

#include <Arduino.h>
#include <address_table.h>
#include <definitions.h>

#define SECRET_COUNT 1

#ifdef __cplusplus
extern "C" {
#endif

typedef struct __attribute__((packed)){
    unsigned short mac_d : 14;
    unsigned short mac_s : 14;
    unsigned short net_d : 14;
    unsigned short net_s : 14;
    byte length;
    byte protocol_id;
    byte seqnum;
    byte hmac[2];
} unpacked_header;

typedef struct __attribute__((packed)){
    byte addresses[7];
    byte length;
    byte protocol_id;
    byte seqnum;
    byte hmac[2];
} packed_header; 

typedef struct __attribute__((packed)){
    packed_header h;
    byte data[PAYLOAD_SIZE];
} packet;

enum Channels{
    DEFCHANNEL = 0
};

extern int (*protocols[256])(packet* p);

extern addr neighbours[MAX_NEIGHBOURS];
extern byte neighbours_size;

extern byte cmd[260];
extern byte status;

int get_hw_flags();
addr find_addr(addr address);
void Receive(void);
void Transmit(void* pvParameters);
packet packet_init(packed_header ph, byte* payload);
packed_header PACK_HEADER(unpacked_header uh);
unpacked_header UNPACK_HEADER(packed_header ph);
unsigned short HASH_PH(packed_header ph);
unsigned short HASH_UH(unpacked_header uh);
void process_packet(void* pvParameters);
int route(addr dest, byte length, byte protocol_id, byte* data);

int radio_init();
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
byte setRxTxFallbackMode();
byte setRfFrequency(float freq);
byte setTxParams();
byte setModulationParams();
byte setPacketParams();

byte getPacketType();
byte getStatus();
byte getRSSI();
byte getSNR();
byte getSignalRssi();
byte getRssiInst();
unsigned short getIrqStatus();

#ifdef __cplusplus
}
#endif

#endif