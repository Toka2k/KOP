#ifndef ___HARDWARE___
#define ___HARDWARE___

#include <Arduino.h>
#include <definitions.h>

#include <address_table.h>
#include <driver.h>

#define SECRET_COUNT 1

#ifdef __cplusplus
extern "C" {
#endif

extern int (*protocols[256])(packet* p);

extern addr neighbours[MAX_NEIGHBOURS];
extern byte neighbours_size;

extern SemaphoreHandle_t radio_mutex;

int get_hw_flags();
addr find_addr(addr address);
void Receive(void* pvParameters);
void Transmit(void* pvParameters);
packet packet_init(packed_header ph, byte* payload);
packed_header PACK_HEADER(unpacked_header uh);
unpacked_header UNPACK_HEADER(packed_header ph);
unsigned short HASH_PH(packed_header ph);
unsigned short HASH_UH(unpacked_header uh);
void process_packet(void* pvParameters);
int route(addr dest, byte length, byte protocol_id, byte* data);

#ifdef __cplusplus
}
#endif

#endif