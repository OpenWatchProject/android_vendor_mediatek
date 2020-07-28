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
 * MediaTek Inc. (C) 2017. All rights reserved.
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

#define LOG_TAG "MtkCamCustom/PlugFastening"
//
#include <cutils/properties.h>
#include <mtkcam/utils/std/Log.h>
//
#include "PlugFastening.h"

using namespace android;
using namespace NSCam;
using namespace NSCamCustom;

#define OPEN_VENDOR  NSCamCustom::IPlugFastening::OPEN_VENDOR_ID_S4Cell
/******************************************************************************
 * init function implementation
 ******************************************************************************/
extern "C"
bool
plugfastening_init(MINT32 id, NSCam::PlugInitParam initParam)
{
    bool ret = true;
    IPlugFastening* pPlugFastening = IPlugFastening::getInstance(OPEN_VENDOR);
    if (pPlugFastening != NULL) {
        ret = pPlugFastening->init(initParam);
    }
    return ret;
};


/******************************************************************************
 * uninit function implementation
 ******************************************************************************/
extern "C"
void
plugfastening_uninit(MINT32 id)
{
    IPlugFastening* pPlugFastening = IPlugFastening::getInstance(OPEN_VENDOR);
    if (pPlugFastening != NULL) {
        pPlugFastening->uninit();
    }
};


/******************************************************************************
 * send command function implementation
 ******************************************************************************/
extern "C"
bool
plugfastening_sendCommand(MINT32 id, MINT32 const cmd,
                                MINTPTR arg1 = 0, MINTPTR arg2 = 0,
                                MINTPTR arg3 = 0, MINTPTR arg4 = 0)
{
    bool ret = true;
    IPlugFastening* pPlugFastening = IPlugFastening::getInstance(OPEN_VENDOR);
    if (pPlugFastening != NULL) {
        ret = pPlugFastening->sendCommand(cmd, arg1, arg2, arg3, arg4);
    }
    return ret;
};


/******************************************************************************
 *
 ******************************************************************************/
IPlugFastening*
IPlugFastening::
getInstance(MUINT32 const openId)
{
    #ifdef PLUGFASTEN_INST_CASE
    #undef PLUGFASTEN_INST_CASE
    #endif
    #define PLUGFASTEN_INST_CASE(ID, CLASS_NAME)\
    case ID:\
    {\
        static CLASS_NAME singleton((OPEN_VENDOR_ID)openId);\
        pInstance = &singleton;\
    }\
    break;
    IPlugFastening* pInstance = NULL;
    switch ((OPEN_VENDOR_ID)openId) {
        PLUGFASTEN_INST_CASE(OPEN_VENDOR_ID_S4Cell,      PlugFasteningS4Cell);
        //PLUGFASTEN_INST_CASE(OPEN_VENDOR_ID_OVFCell,    PlugFasteningOVFCell);
        //PLUGFASTEN_INST_CASE(OPEN_VENDOR_ID_VENDOR_A,        PlugFasteningVendorA);
        //PLUGFASTEN_INST_CASE(OPEN_VENDOR_ID_VENDOR_B,        PlugFasteningVendorB);
        //PLUGFASTEN_INST_CASE(OPEN_VENDOR_ID_VENDOR_C_MAIN,   PlugFasteningVendorC);
        //PLUGFASTEN_INST_CASE(OPEN_VENDOR_ID_VENDOR_C_SUB,    PlugFasteningVendorC);
        // add new class/instance here
        default:
            CAM_LOGI("Open-ID(%d) is not defined", openId);
            break;
    }
    #undef PLUGFASTEN_INST_CASE
    return (IPlugFastening*)pInstance;
};

