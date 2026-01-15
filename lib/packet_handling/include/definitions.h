#ifndef ___DEFINITIONS___
#define ___DEFINITIONS___

// PINS
#define LORA_NSS   5     // SPI chip select
#define LORA_RST   14    // Reset pin
#define LORA_RXEN  16    // TXEN pin
#define LORA_TXEN  17    // TXEN pin
#define LORA_SCK   18    // SPI clock
#define LORA_MISO  19    // SPI MISO
#define LORA_MOSI  23    // SPI MOSI
#define LORA_DIO1  26    // DIO1 interrupt
#define LORA_BUSY  27    // BUSY pin

// PACKET FLAGS
#define SUCCESS 0 
#define ERROR (1)
#define INVALID_HASH (1<<1)
#define INVALID_SEQNUM (1<<2)
#define INVALID_LENGTH (1<<3)
#define INVALID_ADDRESS (1<<4)
#define INVALID_PAYLOAD (1<<5)
#define NOT_NEIGHBOUR (1<<6)
#define EMPTY_BUF (1<<7)
#define CHANNEL_FREE (1<<8)
#define PACKET_RECEIVED
#define PACKET_LAST (1<<10)

// Driver Errors
#define INVALID_SF (1)
#define INVALID_BW (2)
#define INVALID_CR (3)
#define INVALID_POWER (4)
#define INVALID_RAMPTIME (5)
#define INVALID_FREQ (6)
#define INVALID_MODE (7)
#define INVALID_BUF_LENGTH (8)

// generic errors
#define NULL_POINTER -1; 

// PROTOCOLS
#define P_NONE  (0x0)
#define P_DB    (0x1)
#define P_ARP   (0x2)
#define P_DHCP  (0x3)

#define MAX_ITERATIONS (349)

// Hardware
#define MAX_STORED_PACKETS (16)
#define MAX_NEIGHBOURS (256)
#define PAYLOAD_SIZE (256 - HEADER_SIZE)
#define HEADER_SIZE (12)

#define LDO (0)
#define DC_TO_DC (1)
#define STDBY_RC (0)
#define STDBY_XOSC (1)

#define RESERVED_ADDRESSES 2 
#define ADDRESS_BITS 14
#define MAX_TABLE_SIZE (1 << ADDRESS_BITS)
#define TABLE_SIZE __table_size.size

// IRQ Flags
#define IRQ_TX_DONE (1)
#define IRQ_RX_DONE (1<<1)
#define IRQ_PREAMBLE_DETECTED (1<<2)
#define IRQ_HEADER_VALID (1<<4)
#define IRQ_HEADER_ERROR (1<<5)
#define IRQ_CRC_ERROR (1<<6)
#define IRQ_CAD_DONE (1<<7)
#define IRQ_CAD_DETECTED (1<<8)
#define IRQ_TIMEOUT (1<<9)

typedef unsigned char byte;

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

typedef struct __attribute__((packed)){
    unsigned hcost : 2;
    unsigned haddress : 6;
    unsigned lcost : 2;
    unsigned hnextHop : 6;
    byte cost;
    byte laddress;
    byte lnextHop;
} unit;

typedef struct __attribute__((packed)){
    unsigned short int UPDATE_WHEN_ADD : 1;
    unsigned short int REMOVE_WITH_ADDRESS : 1;
    unsigned short int REMOVE_WITH_NEXTHOP : 1;
} flags;

typedef struct __attribute__((packed)){
    unsigned short int size: ADDRESS_BITS;
} size;

typedef struct __attribute__((packed)){
    unsigned short address: ADDRESS_BITS;
} addr;

typedef struct __attribute__((packed)){
    packet (*buf)[MAX_STORED_PACKETS];
    byte count;
    byte index;
} buf_head;

typedef struct __attribute__((packed)){
    byte cmd_status : 3;
    byte chip_mode : 3;
} status;

typedef struct __attribute__((packed)){
    float freq;
    byte sf;
    byte bw;
    byte cr;
    byte pl;
} LLCC68_SETTINGS;

#endif