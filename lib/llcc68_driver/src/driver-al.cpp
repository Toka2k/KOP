#include <Arduino.h>
#include <driver.h>
#include <driver-al.h>

SemaphoreHandle_t irqSemaphore;
SemaphoreHandle_t txDoneSemaphore;
SemaphoreHandle_t rxDoneSemaphore;
SemaphoreHandle_t cadDoneSemaphore;
SemaphoreHandle_t irqTimeoutSemaphore;
SemaphoreHandle_t radio_mutex;

QueueHandle_t irq_status_queue;

void radio_loop(void* pvParameters){

    unsigned short irq_status = 0;
    void (*callback)() = (void (*)())pvParameters;

    attachInterrupt(LORA_DIO1, dio1_isr, RISING);

    for(;;){
        xSemaphoreTake(irqSemaphore, portMAX_DELAY);
        xSemaphoreTake(radio_mutex, portMAX_DELAY);
        
        irq_status = getIrqStatus();
        clearIrqStatus(irq_status);

        xQueueSend(irq_status_queue, &irq_status, 0);
        
        if (irq_status & IRQ_TX_DONE){
            xSemaphoreGive(txDoneSemaphore);
        }
        if (irq_status & IRQ_RX_DONE){
            xSemaphoreGive(rxDoneSemaphore);
        }
        if (irq_status & IRQ_CAD_DONE){
            xSemaphoreGive(cadDoneSemaphore);
        }
        if (irq_status & IRQ_TIMEOUT){
            xSemaphoreGive(irqTimeoutSemaphore);
        }

        xSemaphoreGive(radio_mutex);
        vTaskDelay(pdMS_TO_TICKS(1));
    }
}

void radio_cleanup(unsigned short clearIrqParam){
    setStandby(STDBY_RC);
    clearIrqStatus(clearIrqParam);

    return;
}

unsigned short radio_transmit(packet* p){
    unsigned short irq_status = 0;

    writeBuffer(&p->h.length + HEADER_SIZE, 1);
    setPacketParams(1);
    setTx(calculate_timeout(current_settings.sf, current_settings.bw, current_settings.pl, current_settings.cr), 1);

    xSemaphoreTake(txDoneSemaphore, portMAX_DELAY);
    xQueueReceive(irq_status_queue, &irq_status, portMAX_DELAY);

    writeBuffer((byte*)p, p->h.length + HEADER_SIZE);
    setPacketParams(p->h.length + HEADER_SIZE);
    setTx(calculate_timeout(current_settings.sf, current_settings.bw, current_settings.pl, current_settings.cr), p->h.length + HEADER_SIZE);
    
    xSemaphoreTake(txDoneSemaphore, portMAX_DELAY);
    xQueueReceive(irq_status_queue, &irq_status, portMAX_DELAY);
    return irq_status;
}

unsigned short radio_scanChannel(){
    unsigned short irq_status = 0;
    setCAD();

    xSemaphoreTake(cadDoneSemaphore, portMAX_DELAY);
    xQueueReceive(irq_status_queue, &irq_status, portMAX_DELAY);
    return irq_status;
}

int radio_init(double freq, byte power, byte ramptime, byte sf, byte bw, byte cr){
    current_settings.sf = sf;
    current_settings.bw = bw;
    current_settings.cr = cr;
    current_settings.pl = 1;
    current_settings.freq = freq;
    
    radio_reset();

    unsigned short irq_map =  IRQ_TX_DONE 
                            | IRQ_RX_DONE 
                            | IRQ_PREAMBLE_DETECTED
                            | IRQ_CAD_DONE
                            | IRQ_CAD_DETECTED
                            | IRQ_TIMEOUT;

    semaphore_setup();
    radio_setup();
    byte sync_word[2] = {0x24, 0x24};

    xSemaphoreTake(radio_mutex, portMAX_DELAY);

    setStandby(STDBY_RC);
    calibrate();
    
    setPacketTypeLora();
    setRfFrequency(freq);
    setStandby(STDBY_XOSC);
    delay(1);
    calibrateImage();

    setBufferBaseAddress();
    setModulationParams(current_settings.sf, current_settings.bw, current_settings.cr);
    setPacketParams(current_settings.pl);
    
    clearIrqStatus(0xFFFF);

    setDioIrqParams(irq_map, irq_map/*IRQ_TX_DONE | IRQ_RX_DONE | IRQ_CAD_DONE | IRQ_TIMEOUT*/);
    writeRegister(sync_word, 2, 0x740);

    setRx(320, 5);

    xSemaphoreGive(radio_mutex);

    Serial.println("Succesful radio initialization.");

    return SUCCESS;
}

void semaphore_setup(){
    irqSemaphore = xSemaphoreCreateBinary();
    txDoneSemaphore = xSemaphoreCreateBinary();
    rxDoneSemaphore = xSemaphoreCreateBinary();
    cadDoneSemaphore = xSemaphoreCreateBinary();
    irqTimeoutSemaphore = xSemaphoreCreateBinary();

    xSemaphoreGive(irqSemaphore);
    xSemaphoreGive(txDoneSemaphore);
    xSemaphoreGive(rxDoneSemaphore);
    xSemaphoreGive(cadDoneSemaphore);
    xSemaphoreGive(irqTimeoutSemaphore);
    
    xSemaphoreTake(irqSemaphore, portMAX_DELAY);
    xSemaphoreTake(txDoneSemaphore, portMAX_DELAY);
    xSemaphoreTake(rxDoneSemaphore, portMAX_DELAY);
    xSemaphoreTake(cadDoneSemaphore, portMAX_DELAY);
    xSemaphoreTake(irqTimeoutSemaphore, portMAX_DELAY);

    radio_mutex = xSemaphoreCreateMutex();

    irq_status_queue = xQueueCreate(4, sizeof(unsigned short));
    return;
}

void ARDUINO_ISR_ATTR dio1_isr(){
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    xSemaphoreGiveFromISR(irqSemaphore, &xHigherPriorityTaskWoken);

    if (xHigherPriorityTaskWoken) {
        portYIELD_FROM_ISR();
    }
}