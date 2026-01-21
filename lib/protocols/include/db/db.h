#ifndef ___DB___
#define ___DB___

#include <definitions.h>

#ifdef __cplusplus
extern "C" {
#endif

enum DOWNLOAD_STATUS {
    IDLE, ACTIVE
};

enum ACTION {
    DOWNLOAD_REQ_ALL, DOWNLOAD_REQ_PART, DOWNLOAD_REQ_CORRUPTED, DOWNLOAD_ALL, DOWNLOAD_PART, DOWNLOAD_END, DOWNLOAD_ACK
};

int DB(packet* p);

#ifdef __cplusplus
}
#endif
#endif