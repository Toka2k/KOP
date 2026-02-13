#include <Arduino.h>
#include <definitions.h>
#include <address_table.h>
#include <driver-al.h>
#include <WiFi.h>
#include <esp_now.h>

xSemaphoreHandle rxDoneSemaphore;
xSemaphoreHandle txDoneSemaphore;

xSemaphoreHandle radio_mutex;

QueueHandle_t received_queue;
QueueHandle_t to_process_queue;
QueueHandle_t to_send_queue;

esp_now_peer_info_t peerInfo;

packet p;

byte broadcastAddress[6] = {0xff,0xff,0xff,0xff,0xff,0xff};

void BlinkTask(void* pvParameters){
    digitalWrite(2, HIGH);
    vTaskDelay(pdMS_TO_TICKS(500));
    digitalWrite(2, LOW);
    vTaskDelay(pdMS_TO_TICKS(500));
}

void onSent(const byte* mac_addr, esp_now_send_status_t status){
    xSemaphoreGive(txDoneSemaphore);
    return;
}

void onRecv(const byte* mac_addr, const byte* incomingData, int len) {
    memcpy(&p, incomingData, len);
    
    xSemaphoreGive(rxDoneSemaphore);
    xQueueSend(received_queue, &p, portMAX_DELAY);
    
    return;
}

int radio_transmit(packet* p){
    esp_err_t result = esp_now_send(broadcastAddress, (byte*)p, PACKET_SIZE);
    
    if (result == ESP_OK) {
        return SUCCESS;
    } else {
        return ERROR;
    }
}

int radio_init(){
    init_address_table();

    WiFi.mode(WIFI_STA);
    if (esp_now_init() != ESP_OK) {
        Serial.println("Error initializing ESP-NOW");
        return ERROR;
    }

    memcpy(peerInfo.peer_addr, broadcastAddress, 6);
    peerInfo.channel = 0;  
    peerInfo.encrypt = false;

    if (esp_now_add_peer(&peerInfo) != ESP_OK){
        Serial.println("Failed to add peer");
        return ERROR;
    }

    esp_now_register_send_cb(esp_now_send_cb_t(onSent));
    esp_now_register_recv_cb(esp_now_recv_cb_t(onRecv));

    rxDoneSemaphore = xSemaphoreCreateBinary();
    txDoneSemaphore = xSemaphoreCreateBinary();
    
    radio_mutex = xSemaphoreCreateBinary();
    xSemaphoreGive(radio_mutex);

    received_queue = xQueueCreate(MAX_STORED_PACKETS, PACKET_SIZE);
    to_process_queue = xQueueCreate(MAX_STORED_PACKETS, PACKET_SIZE);
    to_send_queue = xQueueCreate(MAX_STORED_PACKETS, PACKET_SIZE);

    return SUCCESS;
}