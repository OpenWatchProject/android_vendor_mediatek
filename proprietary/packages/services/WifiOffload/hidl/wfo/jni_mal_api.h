#include "mal.h"

/**
 * JNI Mal API
 */
typedef struct JNIMalApi {
    void *handle;
    const char *libmal_version;
    void (* init)();
    void (* exit)();
} JNIMalApi;
