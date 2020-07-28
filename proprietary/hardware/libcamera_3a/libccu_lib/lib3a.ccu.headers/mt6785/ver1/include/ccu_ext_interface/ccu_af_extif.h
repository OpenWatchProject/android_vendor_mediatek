#ifndef __CCU_AF_INTERFACE__
#define __CCU_AF_INTERFACE__

#include "ccu_ext_interface/ccu_types.h"

/******************************************************************************
* AF related interface
******************************************************************************/
typedef struct AFOINIT_INFO_IN
{
    U32 AFOBufsAddr[2][3];
    U32 AFORegAddr[2][3];
    /*
    U32 AFOBufsAddr2[3];
    U32 AFORegAddr2[3];
    */
    U32 magic_reg;
    U32 twin_reg;
} AFOINIT_INFO_IN_T;

typedef struct AFOINIT_INFO_OUT
{
    U32 AFORegAddr;
} AFOINIT_INFO_OUT_T;

typedef struct AFSetHWReg_INFO_IN
{
    U32 AFORegAddr;
} AFSetHWReg_INFO_IN_T;
/*
typedef struct AFAcquireBuf_Reg_INFO_OUT
{
    U32 AFOBufAddr;
    U32 AFOBufStride;

    U32 AFOBufConfigNum;

    //U32 AFOBufMaxSize;

    U32 AFOBufMagic_num;

    U32 AFORegAddr;
} AFAcquireBuf_Reg_INFO_OUT_T;
*/


typedef struct AFAcquireBuf_Reg_INFO_OUT_AP
{
    long long AFOBufAddr;//va
    U32 AFOBufmva;
    U32 AFOBufStride;

    U32 AFOBufConfigNum;
    /*
    U32 AFOBufMaxSize;
    */
    U32 AFOBufMagic_num;

    U32 AFORegAddr;
} AFAcquireBuf_Reg_INFO_OUT_AP_T;

#define AFO_BUFFER_COUNT 6
#define Half_AFO_BUFFER_COUNT AFO_BUFFER_COUNT/2
#define AF_REG_COUNT 4
#define Half_AF_REG_COUNT AF_REG_COUNT/2
#define RAW_number 2

#endif
