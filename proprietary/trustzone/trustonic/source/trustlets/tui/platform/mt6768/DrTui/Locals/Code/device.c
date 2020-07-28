/*
 * Copyright (c) 2013 TRUSTONIC LIMITED
 * All rights reserved
 *
 * The present software is the confidential and proprietary information of
 * TRUSTONIC LIMITED. You shall not disclose the present software and shall
 * use it only in accordance with the terms of the license agreement you
 * entered into with TRUSTONIC LIMITED. This software may be subject to
 * export or import laws in certain countries.
 */
/**
 * Driver helper functions
 */


#include "drStd.h"
#include "DrApi/DrApi.h"
#include "drError.h"
#include "device.h"

#define TAG "[BOARD] "

drError_t mapIO(const deviceInfo_t *device)
{
    drError_t ret = E_DR_OK;
    if (DRAPI_OK != drApiMapPhysicalBuffer(
                        device->paddr,
                        device->len,
                        MAP_READABLE | MAP_WRITABLE | MAP_IO,
                        (void **)&device->addr)) {
        ret = E_DR_INTERNAL;
        drDbgPrintLnf(TAG "[%s] ERROR: cannot map %s PA 0x%x to VA 0x%x", __func__, device->name, device->paddr, device->addr);
    }

    return ret;
}

drError_t unmapIO(const deviceInfo_t *device)
{
    drError_t ret = E_DR_OK;

    if (DRAPI_OK != drApiUnmapBuffer((void *)device->addr)) {
        ret = E_DR_INTERNAL;
        drDbgPrintLnf(TAG "[%s] ERROR: could not unmap %s", __func__, device->name);
    }

    return ret;
}
