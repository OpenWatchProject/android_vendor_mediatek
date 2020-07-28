/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 * 
 * MediaTek Inc. (C) 2010. All rights reserved.
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
 */

/*****************************************************************************
 *
 * Filename:
 * ---------
 *   omx_func.h
 *
 * Project:
 * --------
 *   MT65xx
 *
 * Description:
 * ------------
 *   MTK OMX Video unit test code
 *
 * Author:
 * -------
 *   Bruce Hsu (mtk04278)
 *
 ****************************************************************************/

#ifndef __OMX_FUNC_H__
#define __OMX_FUNC_H__

typedef OMX_ERRORTYPE (*PfOMX_Init)(void);
typedef OMX_ERRORTYPE (*PfOMX_Deinit)(void);
typedef OMX_ERRORTYPE (*PfOMX_ComponentNameEnum)    (OMX_OUT OMX_STRING cComponentName, OMX_IN      OMX_U32 nNameLength,        OMX_IN      OMX_U32 nIndex);
typedef OMX_ERRORTYPE (*PfOMX_GetHandle)            (OMX_OUT OMX_HANDLETYPE* pHandle,   OMX_IN      OMX_STRING cComponentName,  OMX_IN      OMX_PTR pAppData,       OMX_IN  OMX_CALLBACKTYPE* pCallBacks);
typedef OMX_ERRORTYPE (*PfOMX_FreeHandle)           (OMX_IN  OMX_HANDLETYPE hComponent);
typedef OMX_ERRORTYPE (*PfOMX_SetupTunnel)          (OMX_IN  OMX_HANDLETYPE hOutput,    OMX_IN      OMX_U32 nPortOutput,        OMX_IN      OMX_HANDLETYPE hInput,  OMX_IN  OMX_U32 nPortInput);
typedef OMX_ERRORTYPE (*PfOMX_GetContentPipe)       (OMX_OUT OMX_HANDLETYPE *hPipe,     OMX_IN      OMX_STRING szURI);
typedef OMX_ERRORTYPE (*PfOMX_GetComponentsOfRole)  (OMX_IN  OMX_STRING role,           OMX_INOUT   OMX_U32 *pNumComps,         OMX_INOUT   OMX_U8  **compNames);
typedef OMX_ERRORTYPE (*PfOMX_GetRolesOfComponent)  (OMX_IN  OMX_STRING compName,       OMX_INOUT   OMX_U32 *pNumRoles,         OMX_OUT     OMX_U8 **roles);

typedef struct omx_core_function {
    PfOMX_Init                 pf_OMX_Init;
    PfOMX_Deinit               pf_OMX_Deinit;
    PfOMX_ComponentNameEnum    pf_OMX_ComponentNameEnum;
    PfOMX_GetHandle            pf_OMX_GetHandle;
    PfOMX_FreeHandle           pf_OMX_FreeHandle;
    PfOMX_SetupTunnel          pf_OMX_SetupTunnel;          //SetupTunnel is NULL!
    PfOMX_GetContentPipe       pf_OMX_GetContentPipe;
    PfOMX_GetComponentsOfRole  pf_OMX_GetComponentsOfRole;
    PfOMX_GetRolesOfComponent  pf_OMX_GetRolesOfComponent;
} TOmxCoreFuncs;

#endif//__OMX_FUNC_H__

