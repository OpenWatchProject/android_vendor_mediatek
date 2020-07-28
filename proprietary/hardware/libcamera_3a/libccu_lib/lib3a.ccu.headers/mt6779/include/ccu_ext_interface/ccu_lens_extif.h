#ifndef __CCU_LENS_INTERFACE__
#define __CCU_LENS_INTERFACE__

#include "ccu_ext_interface/ccu_types.h"

typedef struct LENS_INFO_IN
{
    I8 szLensName[32];
    U32 dma_buf_va_h;
    U32 dma_buf_va_l;
    U32 u32I2CId;
    U32 sensor_slot;
    U32 *dma_buf_mva;             // virtual address of I2C DMA buffer
} LENS_INFO_IN_T;

typedef struct LENS_INFO_OUT
{
    U8 u8SupportedByCCU;            // 0: not supported, 1: supported
} LENS_INFO_OUT_T;

#endif
