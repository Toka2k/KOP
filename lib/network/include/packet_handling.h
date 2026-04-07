#ifndef ___PACKET_HANDLING___
#define ___PACKET_HANDLING___

#include <Arduino.h>
#include <definitions.h>

#define SECRET_COUNT 1

#ifdef __cplusplus
extern "C" {
#endif

extern int (*protocols[PROTOCOLS])(packet* p);

extern byte* my_seqnum, * neighbour_seqnum;
extern byte debug;

extern addr* neighbours;
extern unsigned short neighbours_size;

extern xSemaphoreHandle rxDoneSemaphore;
extern xSemaphoreHandle txDoneSemaphore;

extern xSemaphoreHandle radio_mutex;

extern QueueHandle_t to_process_queue;
extern QueueHandle_t to_send_queue;

void _print(const char* text);
void init_zero(void* ptr, int ptr_len, int type_size);
int get_hw_flags();
unsigned short find_neighbour(addr neighbour);
void add_neighbour(addr neighbour);
void remove_neighbour(addr neighbour);
byte track_seqnums(packed_header ph);
void Receive(void* pvParameters);
void Transmit(void* pvParameters);
void process_packet(void* pvParameters);
void init_network();
packet packet_init(packed_header ph, byte* payload);
packed_header PACK_HEADER(unpacked_header uh);
unpacked_header UNPACK_HEADER(packed_header ph);
unsigned short HASH_PH(packed_header ph);
unsigned short HASH_UH(unpacked_header uh);
int payload_hash(byte* data, byte length);
int route(addr dest, byte length, byte protocol_id, byte* data);

#ifdef __cplusplus
}
#endif
#endif