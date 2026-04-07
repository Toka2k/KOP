#include <example.h>
#include <packet_handling.h>
#include <Arduino.h>

int EXAMPLE(packet* p){
    _print(p->data);

    return SUCCESS;
}