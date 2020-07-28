#ifndef __DFPS_PRIV_H__
#define __DFPS_PRIV_H__

#include <dfps/dynamicfps_defs.h>

using namespace DFPS;

typedef struct private_module {
    dfps_module_t base;

    int32_t id;
} private_module_t;

typedef struct private_device {
    dfps_device_t base;

    int32_t id;
    void *private_data;
} private_device_t;

#endif
