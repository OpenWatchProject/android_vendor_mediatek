/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010 -2011. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

/** 
 * @file 
 *   omx_test.h
 *
 * @par Project:
 *   6575 
 *
 * @par Description:
 *   OMX IL testing
 *
 * @par Author:
 *   Fantasia Lin (mtk03850)
 *
 * @par $Revision: #1$
 * @par $Modtime:$
 * @par $Log:$
 *
 */
#ifndef MEDIATEK_OMX_TEST_H
#define MEDIATEK_OMX_TEST_H 1

// Set up for C function definitions, even when using C++
#if defined(__cplusplus)
extern "C" {
#endif

#include "OMX_Core.h"
#include "OMX_Component.h"

#define UT_LOG_E(x) x
#define UT_LOG_D(x) 
#define UT_LOG_W(x) 
#define UT_LOG_I(x) 

#define INPUT_PORT_INDEX    0
#define OUTPUT_PORT_INDEX 1
#define ALL_PORT_INDEX        0xFFFFFFFF

#define H264_NUM_INPUT_BUF    8
#define H264_NUM_OUTPUT_BUF   8

#define EXTERNAL_LOAD_SO 0

typedef OMX_ERRORTYPE (*pfnOMX_Init)(void);
typedef OMX_ERRORTYPE (*pfnOMX_Deinit)(void);
typedef OMX_ERRORTYPE (*pfnOMX_ComponentNameEnum)(OMX_OUT OMX_STRING cComponentName,    OMX_IN  OMX_U32 nNameLength,    OMX_IN  OMX_U32 nIndex);
typedef OMX_ERRORTYPE (*pfnOMX_GetHandle)(OMX_OUT OMX_HANDLETYPE* pHandle,     OMX_IN  OMX_STRING cComponentName,    OMX_IN  OMX_PTR pAppData,    OMX_IN  OMX_CALLBACKTYPE* pCallBacks);
typedef OMX_ERRORTYPE (*pfnOMX_FreeHandle)(OMX_IN  OMX_HANDLETYPE hComponent);
typedef OMX_ERRORTYPE (*pfnOMX_SetupTunnel)(OMX_IN  OMX_HANDLETYPE hOutput,    OMX_IN  OMX_U32 nPortOutput,    OMX_IN  OMX_HANDLETYPE hInput,    OMX_IN  OMX_U32 nPortInput);
typedef OMX_ERRORTYPE (*pfnOMX_GetContentPipe)(OMX_OUT OMX_HANDLETYPE *hPipe,    OMX_IN OMX_STRING szURI);
typedef OMX_ERRORTYPE (*pfnOMX_GetComponentsOfRole)(OMX_IN      OMX_STRING role,    OMX_INOUT   OMX_U32 *pNumComps,    OMX_INOUT   OMX_U8  **compNames);
typedef OMX_ERRORTYPE (*pfnOMX_GetRolesOfComponent)(OMX_IN      OMX_STRING compName,     OMX_INOUT   OMX_U32 *pNumRoles,    OMX_OUT     OMX_U8 **roles);

typedef enum _BuffOwnrShip{
    BOS_IL_CLIENT = 1,
    BOS_OMX_COMPONENT = 2,
}BuffOwnrShip;

typedef struct _OMXCoreIfaceTable{
    void *pHandle;      // this will keep the OMX core handle
    OMX_HANDLETYPE CompHandle;    //component handle
    OMX_CALLBACKTYPE OMX_CallBack; // callback function for OMX component
    pfnOMX_Init  _OMX_Init;
    pfnOMX_Deinit _OMX_Deinit;
    pfnOMX_ComponentNameEnum _OMX_ComponentNameEnum;
    pfnOMX_GetHandle _OMX_GetHandle;
    pfnOMX_FreeHandle _OMX_FreeHandle;
    pfnOMX_SetupTunnel _OMX_SetupTunnel;
    pfnOMX_GetContentPipe _OMX_GetContentPipe;
    pfnOMX_GetComponentsOfRole _OMX_GetComponentsOfRole;
    pfnOMX_GetRolesOfComponent _OMX_GetRolesOfComponent;
    char szComponentName[128];
    char szComponentRole[OMX_MAX_STRINGNAME_SIZE];
    int32_t width;
    int32_t height;
    uint32_t bitrate;
    uint32_t fps;                    // need to x10 ?
    uint32_t   i_keyint_max;        //Maximum GOP size
    OMX_BUFFERHEADERTYPE* pOutBufHdrs[H264_NUM_OUTPUT_BUF];
    OMX_BUFFERHEADERTYPE* pInBufHdrs[H264_NUM_INPUT_BUF];
    BuffOwnrShip OutBufOwner[H264_NUM_OUTPUT_BUF];
    BuffOwnrShip InBufOwner[H264_NUM_INPUT_BUF];    
    unsigned char* pInputBuffPool;
    unsigned char* pOutBuffPool;
}OMXCoreIfaceTable, *pOMXCoreIfaceTable;

int LoadOMXCore(char *pStrOMXCorePathFileName, pOMXCoreIfaceTable pOMXCoreIfaceTableInst);
int ReleaseOMXCore(pOMXCoreIfaceTable pOMXCoreIfaceTableInst);
int GetOmxComponentHandle(char *pStrOMXComponentName, pOMXCoreIfaceTable pOMXCoreIfaceTableInst, void *pAppData);
int ConfigOmxComponentPorts(pOMXCoreIfaceTable pOMXCoreIfaceTableInst);



#if defined(__cplusplus)
}
#endif

#endif /* MEDIATEK_OMX_TEST_H */
