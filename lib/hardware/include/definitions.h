#ifndef ___DEFINITIONS___
#define ___DEFINITIONS___

// PINS
#define LORA_NSS   5     // SPI chip select
#define LORA_RST   14    // Reset pin
#define LORA_RXEN  16    // TXEN pin
#define LORA_SCK   18    // SPI clock
#define LORA_MISO  19    // SPI MISO
#define LORA_MOSI  23    // SPI MOSI
#define LORA_DIO1  26    // DIO1 interrupt
#define LORA_BUSY  27    // BUSY pin

// HW_FLAGS
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

// Driver Errors
#define INVALID_SF (1)
#define INVALID_BW (2)
#define INVALID_CR (3)
#define INVALID_POWER (4)
#define INVALID_RAMPTIME (5)
#define INVALID_FREQ (6)
#define INVALID_MODE (7)

// generic errors
#define NULL_POINTER -1; 

// PROTOCOLS
#define P_   (0x0)
#define P_DB   (0x1)
#define P_ARP    (0x2)
#define P_DHCP  (0x3)

// Hardware
#define MAX_STORED_PACKETS (16)
#define MAX_NEIGHBOURS (256)
#define PAYLOAD_SIZE (256 - sizeof(packed_header))
#define HEADER_SIZE (sizeof(packed_header))
#define PACKET_SIZE (sizeof(packed_header) + ph.len)

#define RESERVED_ADDRESSES 2 
#define ADDRESS_BITS 14
#define MAX_TABLE_SIZE (1 << ADDRESS_BITS)

// IRQ Flags
#define IRQ_TX_DONE 0
#define IRQ_RX_DONE 1
#define IRQ_PREAMBLE_DETECTED 2
#define IRQ_HEADER_VALID 4
#define IRQ_HEADER_ERROR 5
#define IRQ_CRC_ERROR 6
#define IRQ_CAD_DONE 7
#define IRQ_CAD_DETECTED 8
#define IRQ_TIMEOUT 9

#endif