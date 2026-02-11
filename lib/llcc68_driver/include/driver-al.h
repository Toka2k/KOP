#ifndef ___DRIVER_AL___
#define ___DRIVER_AL___

#include <Arduino.h>
#include <definitions.h>

#ifdef __cplusplus
extern "C" {
#endif

extern SemaphoreHandle_t irqSemaphore;
extern SemaphoreHandle_t txDoneSemaphore;
extern SemaphoreHandle_t rxDoneSemaphore;
extern SemaphoreHandle_t cadDoneSemaphore;
extern SemaphoreHandle_t irqTimeoutSemaphore;
extern SemaphoreHandle_t radio_mutex;

extern QueueHandle_t irq_status_queue;

extern packet rx_packet;

void radio_loop(void* pvParameters);
void radio_cleanup(unsigned short clearIrqParam);
unsigned short radio_transmit(packet* p);
unsigned short radio_scanChannel();
int radio_init(double freq, byte power, byte ramptime, byte sf, byte bw, byte cr);
void semaphore_setup();

void i2c_rx_task(void* pvParameters);

#ifdef __cplusplus
}
#endif
#endif