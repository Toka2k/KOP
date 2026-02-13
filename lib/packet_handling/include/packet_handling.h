#ifndef ___PACKET_HANDLING___
#define ___PACKET_HANDLING___

#include <Arduino.h>
#include <definitions.h>

#define SECRET_COUNT 1

#ifdef __cplusplus
extern "C" {
#endif

extern int (*protocols[PROTOCOLS])(packet* p);

extern addr neighbours[MAX_NEIGHBOURS];
extern int neighbours_size;

extern xSemaphoreHandle rxDoneSemaphore;
extern xSemaphoreHandle txDoneSemaphore;

extern xSemaphoreHandle radio_mutex;

extern QueueHandle_t to_process_queue;
extern QueueHandle_t to_send_queue;

int get_hw_flags();
addr find_addr(addr address);
void Receive(void* pvParameters);
void Transmit(void* pvParameters);
void process_packet(void* pvParameters);
packet packet_init(packed_header ph, byte* payload);
packed_header PACK_HEADER(unpacked_header uh);
unpacked_header UNPACK_HEADER(packed_header ph);
unsigned short HASH_PH(packed_header ph);
unsigned short HASH_UH(unpacked_header uh);
int route(addr dest, byte length, byte protocol_id, byte* data);

#ifdef __cplusplus
}
#endif
#endif