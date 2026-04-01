#include <Arduino.h>
#include <definitions.h>
#include <address_table.h>
#include <packet_handling.h>
#include <driver-al.h>
#include <updates.h>
#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>

xSemaphoreHandle rxDoneSemaphore;
xSemaphoreHandle txDoneSemaphore;

xSemaphoreHandle radio_mutex;

QueueHandle_t received_queue;
QueueHandle_t to_process_queue;
QueueHandle_t to_send_queue;
QueueHandle_t sent_queue;

esp_now_peer_info_t peerInfo;

packet *p;

byte broadcastAddress[6] = {0xff,0xff,0xff,0xff,0xff,0xff};

void BlinkTask(void* pvParameters){
    digitalWrite(2, HIGH);
    vTaskDelay(pdMS_TO_TICKS(500));
    digitalWrite(2, LOW);
    vTaskDelay(pdMS_TO_TICKS(500));
}

void onSent(const byte* mac_addr, esp_now_send_status_t status){
    packet* sent;
    
    if (xQueueReceive(sent_queue, &sent, portMAX_DELAY)){
        free(sent);
    }

    xSemaphoreGive(txDoneSemaphore);

    return;
}

void onRecv(const byte* mac_addr, const byte* incomingData, int len) {
    p = (packet*)malloc(sizeof(packet));
    memcpy(p, incomingData, len);
    
    xSemaphoreGive(rxDoneSemaphore);
    xQueueSend(received_queue, p, portMAX_DELAY);

    free(p);
    
    return;
}

int radio_transmit(packet* p){
    packet *_p = (packet*)malloc(PACKET_SIZE);
    memcpy(_p, p, PACKET_SIZE);

    xQueueSend(sent_queue, &_p, portMAX_DELAY);
    esp_err_t result = esp_now_send(broadcastAddress, (byte*)_p, PACKET_SIZE);
    
    if (result == ESP_OK) {
        return SUCCESS;
    } else {
        Serial.printf("Transmit error: %d\n", result);
        return ERROR;
    }
}

int radio_init(){
    WiFi.mode(WIFI_STA);
    if (esp_now_init() != ESP_OK) {
        Serial.println("Error initializing ESP-NOW");
        return ERROR;
    }

    esp_wifi_set_channel(6, WIFI_SECOND_CHAN_NONE);

    memset(&peerInfo, 0, sizeof(peerInfo));
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
    
    sent_queue = xQueueCreate(MAX_STORED_PACKETS, sizeof(void*));

    uint8_t primary;
    wifi_second_chan_t second;
    esp_wifi_get_channel(&primary, &second);
    //Serial.print("HOME CHANNEL = ");
    //Serial.println(primary);
    
    return SUCCESS;
}