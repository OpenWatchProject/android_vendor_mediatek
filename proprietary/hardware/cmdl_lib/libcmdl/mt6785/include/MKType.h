/*------------------------------------------------------------------------------
 * MediaTek Inc. (C) 2019. All rights reserved.
 *
 * Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 *----------------------------------------------------------------------------*/

#ifndef __MK_TYPE_H__
#define __MK_TYPE_H__

#include <stdio.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void* MKObject;
typedef void* MKBool;
typedef void* MKCommand;
typedef void* MKProgram;
typedef void* MKStream;

// defined same as CMDL_RunType_e structure in ./src/api/cmdl.h
typedef enum {
    MK_PROGRAM_INIT     = 0x0001,
    MK_PROGRAM_EXECUTE  = 0x0002,
    MK_PROGRAM_END      = 0x0004,
    MK_PROGRAM_ANY      = 0xFFFF
} MKProgramType;

// defined same as CMDL_Result_e structure in ./src/api/cmdl.h
typedef enum {
    MK_OK                           = 0,
    MK_OK_DISABLE                   = 1,
    MK_OK_HANDLE_FULL               = 2,
    MK_ERROR_GENERAL_FAILED         = 16,
    MK_ERROR_INVALID_PARAMS         = 17,
    MK_ERROR_INVALID_HANDLE         = 18,
    MK_ERROR_INVALID_NODE           = 19,
    MK_ERROR_BAD_RESOURCE_MAP       = 20,
    MK_ERROR_BAD_BUFFER_MAP         = 21,
    MK_ERROR_OUT_OF_MEMORY          = 22,
    MK_ERROR_COMMAND_RECORD_FAIL    = 23,
    MK_ERROR_COMMAND_RUN_FAIL       = 24,
    MK_ERROR_CL_DEVICE_NOT_MATCHED  = 25,
    MK_ERROR_NOT_FOUND              = 26,
    MK_ERROR_DUMP_FILE              = 27,
    MK_ERROR_SET_HOST_PTR_FAIL      = 28,
    MK_ERROR_COMPARE_SIZE_NOT_MATCH = 29,
    MK_ERROR_COMPARE_NOT_MATCH      = 30,
} MKErrorType;

#ifdef __cplusplus
}
#endif

#endif  /* __MK_TYPE_H__ */
