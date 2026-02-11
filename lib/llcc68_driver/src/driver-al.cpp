#include <Arduino.h>
#include <Wire.h>
#include <driver-al.h>

#define I2C_ADDR 0x42
#define I2C_SDA 21
#define I2C_SCL 22
#define I2C_FREQ 100000

SemaphoreHandle_t irqSemaphore;
SemaphoreHandle_t txDoneSemaphore;
SemaphoreHandle_t rxDoneSemaphore;
SemaphoreHandle_t cadDoneSemaphore;
SemaphoreHandle_t irqTimeoutSemaphore;
SemaphoreHandle_t radio_mutex;

QueueHandle_t irq_status_queue;

packet rx_packet;

static byte rx_buffer[PACKET_SIZE];

void semaphore_setup()
{
    irqSemaphore = xSemaphoreCreateBinary();
    txDoneSemaphore = xSemaphoreCreateBinary();
    rxDoneSemaphore = xSemaphoreCreateBinary();
    cadDoneSemaphore = xSemaphoreCreateBinary();
    irqTimeoutSemaphore = xSemaphoreCreateBinary();

    radio_mutex = xSemaphoreCreateMutex();
    irq_status_queue = xQueueCreate(8, sizeof(unsigned short));
}

int radio_init(double freq, byte power, byte ramptime, byte sf, byte bw, byte cr)
{
    Wire.begin(I2C_SDA, I2C_SCL, I2C_FREQ);
    semaphore_setup();

    xTaskCreate(
        i2c_rx_task,
        "i2c_rx",
        4096,
        NULL,
        10,
        NULL
    );

    xTaskCreate(
        radio_loop,
        "radio_loop",
        4096,
        NULL,
        9,
        NULL
    );

    return SUCCESS;
}

void radio_loop(void* pvParameters)
{
    unsigned short irq_status = 0;

    for (;;)
    {
        xSemaphoreTake(irqSemaphore, portMAX_DELAY);
        xSemaphoreTake(radio_mutex, portMAX_DELAY);

        xQueueSend(irq_status_queue, &irq_status, 0);

        if (irq_status & IRQ_TX_DONE)
            xSemaphoreGive(txDoneSemaphore);

        if (irq_status & IRQ_RX_DONE)
            xSemaphoreGive(rxDoneSemaphore);

        xSemaphoreGive(radio_mutex);
        vTaskDelay(pdMS_TO_TICKS(1));
    }
}

unsigned short radio_transmit(packet* p)
{
    xSemaphoreTake(radio_mutex, portMAX_DELAY);

    Wire.beginTransmission(I2C_ADDR);
    Wire.write((byte*)p, PACKET_SIZE);
    Wire.endTransmission();

    unsigned short irq = IRQ_TX_DONE;
    xQueueSend(irq_status_queue, &irq, 0);
    xSemaphoreGive(irqSemaphore);

    xSemaphoreTake(txDoneSemaphore, portMAX_DELAY);

    xSemaphoreGive(radio_mutex);
    return SUCCESS;
}

void i2c_rx_task(void* pvParameters)
{
    size_t received = 0;
    uint8_t* dst = (uint8_t*)&rx_packet;

    for (;;)
    {
        int count = Wire.requestFrom(I2C_ADDR, PACKET_SIZE);

        while (Wire.available() && received < PACKET_SIZE)
        {
            dst[received++] = Wire.read();
        }

        if (received == PACKET_SIZE)
        {
            unsigned short irq = IRQ_RX_DONE;

            xQueueSend(irq_status_queue, &irq, 0);
            xSemaphoreGive(irqSemaphore);

            received = 0;
        }

        vTaskDelay(pdMS_TO_TICKS(2));
    }
}

void radio_cleanup(unsigned short clearIrqParam)
{
    return;
}

unsigned short radio_scanChannel()
{
    return CHANNEL_FREE;
}
