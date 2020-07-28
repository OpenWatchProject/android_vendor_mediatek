/* MediaTek Inc. (C) 2016. All rights reserved.
 *
 * Copyright Statement:
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 */

/*
* Description:
*   This file implements Aurisys and Audio DC Removal related variables.
*/
#ifndef __ARSI_DC_REMOVAL_H__
#define __ARSI_DC_REMOVAL_H__


#include <dc_removal_flt.h>


typedef enum {
    DCR_MODE_1 = 0,
    DCR_MODE_2,
    DCR_MODE_3
} DCR_MODE;

typedef enum {
    DCREMOVE_BIT16 = 0,
    DCREMOVE_BIT24,
} DCR_BITDEPTH;

typedef struct {
    uint32_t mInternalBufSize; // in byte
    signed char *mpInternalBuf;
} lib_workingbuf_info_t;

typedef struct {
    DCRemove_Handle *mDCRemoveHandle;
    lib_workingbuf_info_t mLibWorkingBuf;
    DCR_MODE mdcrmode;
    DCR_BITDEPTH mdcr_depth;
    bool benhance_on;
} dcremoval_lib_handle_t;


#endif // __ARSI_DC_REMOVAL_H__
