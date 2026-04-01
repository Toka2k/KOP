#ifndef ___UPDATES___
#define ___UPDATES___

#include <Arduino.h>
#include <definitions.h>

#ifdef __cplusplus
extern "C" {
#endif

extern byte* missed_msg;

extern xSemaphoreHandle trigger_update;
extern xSemaphoreHandle neighbour_semaphore;

void init_updates();
void invalidate_routes(addr neighbour);
void mark_route_refreshed(addr dest);
void get_changed_routes();

int process_update(packet* p);
unsigned short* get_routes_to_advertise(addr neighbour, byte* out_len);

// TASKS:
void hello(void* pvParameters);
void trigger_update_task(void* pvParameters);
void full_update_task(void* pvParameters);
void increment_neighbour_counter(void* pvParameters);
void increment_counter(void* pvParameters);
void cleanup_task(void* pvParameters);
#ifdef __cplusplus
}
#endif

#endif