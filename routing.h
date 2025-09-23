#ifndef ___ROUTING___
#define ___ROUTING___

#define DEFCHANL 0
#define DOWNLINK 5

static double __channels[] = {8670E5, 8672E5, 8674E5, 8676E5, 8678E5, 8680E5};

typedef struct{
    // may have to include payload length
    void* data;
}payload;

//  To do:
//      1. Address - GET/OFFER/ACK
//      2. Send data - SEND/ACK
//      3. Hello packets
//      4. 
//
//

//defining different packets



#include "address.h"
#endif
