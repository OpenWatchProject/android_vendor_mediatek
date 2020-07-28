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

#define LOG_TAG "MtkCam/HwUtils/PlugProcessing"
//
#include <cutils/properties.h>
#include <mtkcam/Log.h>
//
#include "PlugProcessing.h"

using namespace android;
using namespace NSCam;


/******************************************************************************
 *
 ******************************************************************************/
android::sp<IPlugProcessing>
IPlugProcessing::
createInstance(MUINT32 const plugId, DEV_ID const devId)
{
    #ifdef PLUGPROC_INST_DEV
    #undef PLUGPROC_INST_DEV
    #endif
    #define PLUGPROC_INST_DEV(PLUG, DEV, INSTANCE)\
        case DEV:\
        {\
            static android::Mutex lock;\
            Mutex::Autolock _l(lock);\
            static wp<IPlugProcessing> staticPtr = NULL;\
            sp<IPlugProcessing> returnPtr = staticPtr.promote();\
            if (returnPtr == NULL) {\
                CAM_LOGI("PlugID:%d DevID:%d create new instance", PLUG, DEV);\
                returnPtr = new INSTANCE((MUINT32)((PLUG << 8) | DEV));\
                staticPtr = returnPtr;\
            }/* else CAM_LOGD("use previous instance");*/\
            return returnPtr;\
        }
    //
    #ifdef PLUGPROC_INST_CASE
    #undef PLUGPROC_INST_CASE
    #endif
    #define PLUGPROC_INST_CASE(PLUG, INSTANCE)\
        case PLUG:\
        {\
            switch (devId) {\
                PLUGPROC_INST_DEV(PLUG, DEV_ID_0, INSTANCE);\
                PLUGPROC_INST_DEV(PLUG, DEV_ID_1, INSTANCE);\
                PLUGPROC_INST_DEV(PLUG, DEV_ID_2, INSTANCE);\
                PLUGPROC_INST_DEV(PLUG, DEV_ID_3, INSTANCE);\
                PLUGPROC_INST_DEV(PLUG, DEV_ID_4, INSTANCE);\
                PLUGPROC_INST_DEV(PLUG, DEV_ID_5, INSTANCE);\
                PLUGPROC_INST_DEV(PLUG, DEV_ID_6, INSTANCE);\
                PLUGPROC_INST_DEV(PLUG, DEV_ID_7, INSTANCE);\
                default:\
                    CAM_LOGI("Dev-ID(%d) is not defined", devId);\
                    break;\
            };\
        };\
        break;
    //
    switch ((PLUG_ID)plugId) {
        PLUGPROC_INST_CASE(PLUG_ID_BASIC, PlugProcessingBasic);
        //PLUGPROC_INST_CASE(PLUG_ID_4CELL, PlugProcessing4Cell);
        PLUGPROC_INST_CASE(PLUG_ID_FCELL, PlugProcessingFCell);
        // add new case instance here
        default:
            CAM_LOGI("Plug-ID(%d) is not defined", plugId);
            break;
    };
    #undef PLUGPROC_INST_DEV
    #undef PLUGPROC_INST_CASE
    return NULL;
};

