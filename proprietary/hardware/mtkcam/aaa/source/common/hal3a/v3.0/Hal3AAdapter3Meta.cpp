/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
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

/********************************************************************************************
 *     LEGAL DISCLAIMER
 *
 *     (Header of MediaTek Software/Firmware Release or Documentation)
 *
 *     BY OPENING OR USING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *     THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE") RECEIVED
 *     FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON AN "AS-IS" BASIS
 *     ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED,
 *     INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR
 *     A PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY
 *     WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK
 *     ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
 *     NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION
 *     OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 *
 *     BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE LIABILITY WITH
 *     RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION,
TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/
#define LOG_TAG "Hal3Av3Meta"


#include <mtkcam/utils/std/Log.h>
#include <mtkcam/def/common.h>
#include <mtkcam/utils/std/common.h>
#include <cutils/properties.h>
//#include <camera_feature.h>
#include <faces.h>
#include "Hal3AAdapter3.h"

#include <mtkcam/utils/metadata/client/mtk_metadata_tag.h>
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>
#include <mtkcam/aaa/IDngInfo.h>
#include <mtkcam/utils/metastore/IMetadataProvider.h>
#include <mtkcam/utils/std/Trace.h>

#include <mtkcam/utils/hw/HwTransform.h>

#include <debug_exif/dbg_id_param.h>
#include <dip_reg.h>

using namespace NS3Av3;
using namespace NSCam;
using namespace NSCamHW;


#define GET_PROP(prop, dft, val)\
{\
   char value[PROPERTY_VALUE_MAX] = {'\0'};\
   property_get(prop, value, (dft));\
   (val) = atoi(value);\
}

#define MAX(a,b) ((a) >= (b) ? (a) : (b))
#define MIN(a,b) ((a) < (b) ? (a) : (b))

#define HAL3A_TEST_OVERRIDE (1)

#define HAL3AADAPTER3_LOG_SET_0 (1<<0)
#define HAL3AADAPTER3_LOG_SET_1 (1<<1)
#define HAL3AADAPTER3_LOG_SET_2 (1<<2)
#define HAL3AADAPTER3_LOG_GET_0 (1<<3)
#define HAL3AADAPTER3_LOG_GET_1 (1<<4)
#define HAL3AADAPTER3_LOG_GET_2 (1<<5)
#define HAL3AADAPTER3_LOG_GET_3 (1<<6)
#define HAL3AADAPTER3_LOG_GET_4 (1<<7)
#define HAL3AADAPTER3_LOG_PF    (1<<8)
#define HAL3AADAPTER3_LOG_USERS (1<<9)

#define HAL3A_REQ_PROC_KEY (2)
#define HAL3A_REQ_CAPACITY (HAL3A_REQ_PROC_KEY + 2)


MBOOL
Hal3AAdapter3::
doInit()
{
    CAM_LOGD("[%s]+ sensorIdx(%d)", __FUNCTION__, mi4SensorIdx);

    mu1CapIntent = -1; //MTK_CONTROL_CAPTURE_INTENT_PREVIEW;
    mu4Counter = 0;
    mu1Start = 0;
    mu1FdEnable = 0;
    mPrvCropRegion.p.x = 0;
    mPrvCropRegion.p.y = 0;
    mPrvCropRegion.s.w = 0;
    mPrvCropRegion.s.h = 0;

    mAppCropRegion.p.x = 0;// for App meta
    mAppCropRegion.p.y = 0;
    mAppCropRegion.s.w = 0;
    mAppCropRegion.s.h = 0;

    Hal3AAdapter3::mMapMat.clear();

    m_rStaticInfo.u1RollingShutterSkew = 0;

    //This is avoid back to camera after quit camera by homeKey
    if (m_rConfigInfo.i4SubsampleCount > 1 && m_rConfigInfo.i4RequestCount > 1) //Slow motion video recording
        mu4MetaResultQueueCapacity = m_rConfigInfo.i4SubsampleCount * Capacity;
    else
        mu4MetaResultQueueCapacity = Capacity;

    mpHal3aObj = Hal3AIf::getInstance(mi4SensorIdx, Hal3AIf::E_FlowControl_Type_NORMAL);

    mu4SensorDev = mpHal3aObj->getSensorDev();

    //This is enter camera first.
    // ResultPool - Get ResultPool object to update capacity and init ResultPoolObj
    if(mpResultPoolObj == NULL)
    {
        mpResultPoolObj = IResultPool::getInstance(mu4SensorDev);
        if(mpResultPoolObj)
        {
            // Remapping subsample with Batch mode and Burts mode.
            // Batch mode : subsample is 4, RequestCount is 1.
            //             use normal buffer size and subsample is 1
            // Burst mode : subsample is 4, RequestCount is 4.
            //             use SMVR buffer size and subsample is 4
            MINT32 i4LastSubsampleCount = (m_rConfigInfo.i4SubsampleCount == m_rConfigInfo.i4RequestCount) ? m_rConfigInfo.i4SubsampleCount : 1;
            CAM_LOGD("[%s] LastSubsampleCount(%d)", __FUNCTION__, i4LastSubsampleCount);
            mpResultPoolObj->setCapacity(mu4MetaResultQueueCapacity, i4LastSubsampleCount);
            mpResultPoolObj->init();
        }
        else
            CAM_LOGE("[%s] ResultPool getInstance fail!", __FUNCTION__);
    }

    mParams = Param_T();
    mAfParams = AF_Param_T();
    for(int i=0;i<MAX_METERING_AREAS;i++)
        mParams.rMeteringAreas.rAreas[i].i4Weight = 0;
    mpHal3aObj->setAfParams(mAfParams);
    mpHal3aObj->attachCb(this);
    CAM_LOGD("[%s]- sensorDev(%d), sensorIdx(%d), mpHal3aObj(%p), ResultPool(Addr, Capacity) = (%p, %d)", __FUNCTION__, mu4SensorDev, mi4SensorIdx, mpHal3aObj, mpResultPoolObj, mu4MetaResultQueueCapacity);
    return MTRUE;
}

MBOOL
Hal3AAdapter3::
doUninit()
{
    CAM_LOGD("[%s]+ sensorDev(%d), sensorIdx(%d)", __FUNCTION__, mu4SensorDev, mi4SensorIdx);
    // ResultPool - Uninit ResultPool object
    if(mpResultPoolObj)
    {
        mpResultPoolObj->uninit();
        mpResultPoolObj =NULL;
    }

    m_ScnModeOvrd.clear();
    mpHal3aObj->detachCb(this);
    mpHal3aObj->destroyInstance();
    mpHal3aObj = NULL;

    CAM_LOGD("[%s]- sensorDev(%d), sensorIdx(%d)", __FUNCTION__, mu4SensorDev, mi4SensorIdx);
    return MTRUE;
}
#if 0
unsigned int
Hal3AAdapter3::
queryTuningSize()
{
    return sizeof(dip_x_reg_t);
}
#endif
