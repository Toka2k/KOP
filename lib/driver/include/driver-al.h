#ifndef ___DRIVER_AL___
#define ___DRIVER_AL___
#include <Arduino.h>
#include <definitions.h>

#ifdef __cplusplus
extern "C" {
#endif
extern xSemaphoreHandle rxDoneSemaphore;
extern xSemaphoreHandle txDoneSemaphore;

extern xSemaphoreHandle radio_mutex;

extern QueueHandle_t received_queue;
extern QueueHandle_t to_process_queue;
extern QueueHandle_t to_send_queue;

int radio_transmit(packet* p);
int radio_init();

#ifdef __cplusplus
}
#endif
#endif