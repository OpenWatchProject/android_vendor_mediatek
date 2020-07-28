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
#define LOG_TAG "Hal3Av3Misc"

#include "Hal3AMisc.h"
#include <mtkcam/utils/std/Log.h>
#include <mtkcam/def/common.h>
#include <mtkcam/utils/std/common.h>
#include <utils/Atomic.h>
#include <vector>
#include <cutils/properties.h>
#include <array>

#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>
#include <mtkcam/utils/metadata/getDumpFilenamePrefix.h>
#include <mtkcam/aaa/IDngInfo.h>
#include <mtkcam/utils/std/Trace.h>
#include <aaa_trace.h>
#include <mtkcam/utils/metastore/IMetadataProvider.h>

#include <mtkcam/utils/hw/HwTransform.h>
#include <debug_exif/dbg_id_param.h>
#include <mtkcam/utils/exif/IBaseCamExif.h>

#include "isp_tuning_mgr.h"

#define GET_PROP(prop, dft, val)\
{\
   val = property_get_int32(prop,dft);\
}

#define MY_LOGD(fmt, arg...) \
    do { \
        CAM_LOGD(fmt, ##arg); \
        }while(0)

#define MY_LOGD_IF(cond, ...) \
    do { \
        if ( (cond) ){ CAM_LOGD(__VA_ARGS__); } \
        }while(0)

#define MY_INST NS3Av3::INST_T<Hal3AMisc>
static std::array<MY_INST, SENSOR_IDX_MAX> gMultiton;

Hal3AMisc*
Hal3AMisc::
getInstance(const MINT32 i4SensorIdx)
{
    MINT32 i4LogEn = 0;
    GET_PROP("vendor.debug.hal3amisc.log", 0, i4LogEn);
    MY_LOGD_IF(i4LogEn, "[%s] sensorIdx(%d)", __FUNCTION__, i4SensorIdx);

    if(i4SensorIdx >= SENSOR_IDX_MAX || i4SensorIdx < 0) {
        MY_LOGE("[%s] Unsupport sensor Index: %d\n", __FUNCTION__, i4SensorIdx);
        return nullptr;
    }

    MY_INST& rSingleton = gMultiton[i4SensorIdx];
    std::call_once(rSingleton.onceFlag, [&] {
        rSingleton.instance = std::make_unique<Hal3AMisc>(i4SensorIdx);
    } );
    (rSingleton.instance)->init();

    return rSingleton.instance.get();
}

Hal3AMisc::
Hal3AMisc(const MINT32 i4SensorIdx)
    : i4SensorIndex(i4SensorIdx)
    , LCSlock()
{
   LCSoutQ.clear();
}

MVOID
Hal3AMisc::
init()
{
    Mutex::Autolock lock(LCSlock);
    LCSoutQ.clear();
    return ;
}

MVOID
Hal3AMisc::
uninit()
{
    clearLCSOList_Out();
    return ;
}

MVOID
Hal3AMisc::
clearLCSOList_Out()
{
    Mutex::Autolock lock(LCSlock);
    LCSoutQ.clear();

    return ;
}

MINT32
Hal3AMisc::
updateLCSList_Out(const ISP_LCS_OUT_INFO_T& rLCSinfo)
{
    Mutex::Autolock lock(LCSlock);

    MINT32 i4Ret = -1;
    MINT32 i4Pos = 0;
    MINT32 i4Size = LCSoutQ.size();
    List<ISP_LCS_OUT_INFO_T>::iterator it = LCSoutQ.begin();

    for (it = LCSoutQ.begin(); it != LCSoutQ.end(); it++, i4Pos++)
    {
        if (it->i4FrmId == rLCSinfo.i4FrmId)
        {
            CAM_LOGW("overwirte LCSList_Out FrmID: %d", rLCSinfo.i4FrmId);
            *it = rLCSinfo;
            i4Ret = 1;
            break;
        }
    }

    if (i4Pos == i4Size)
    {
        LCSoutQ.push_back(rLCSinfo);
        i4Ret = 0;
    }

    // remove item
    if (LCSoutQ.size() > 15)
    {
        LCSoutQ.erase(LCSoutQ.begin());
    }

    return i4Ret;
}

MINT32
Hal3AMisc::
getLCSList_info_Out(MINT32 i4FrmId, ISP_LCS_OUT_INFO_T& rLCSinfo)
{
    Mutex::Autolock lock(LCSlock);

    MINT32 i4Ret = 0;
    MINT32 i4Pos = 0;
    MINT32 i4Size = LCSoutQ.size();
    List<ISP_LCS_OUT_INFO_T>::iterator it = LCSoutQ.begin();
    for (; it != LCSoutQ.end(); it++, i4Pos++)
    {
        if (it->i4FrmId == (MUINT32)i4FrmId)
        {
            rLCSinfo = *it;
            //CAM_LOGD("[%s] OK i4Pos(%d)", __FUNCTION__, i4Pos);
            break;
        }
    }

    if (i4Pos == i4Size)
    {
        // does not exist
        CAM_LOGD("[%s] NG i4Pos(%d)", __FUNCTION__, i4Pos);
        i4Ret = -1;
    }

    return i4Ret;
}

MINT32
Hal3AMisc::
getLCSListLast_Out(ISP_LCS_OUT_INFO_T& rLCSinfo)
{
    Mutex::Autolock lock(LCSlock);

    if (!LCSoutQ.empty())
    {
        List<ISP_LCS_OUT_INFO_T>::iterator it = LCSoutQ.end();
        it--;
        rLCSinfo = *it;
        return 0;
    }

    return -1;
}

ISP_LCS_OUT_INFO_T*
Hal3AMisc::
getLCSList_info_Out(MINT32 i4FrmId)
{
    Mutex::Autolock lock(LCSlock);

    ISP_LCS_OUT_INFO_T* pBuf = NULL;
    MINT32 i4Pos = 0;
    MINT32 i4Size = LCSoutQ.size();
    List<ISP_LCS_OUT_INFO_T>::iterator it = LCSoutQ.begin();
    for (; it != LCSoutQ.end(); it++, i4Pos++)
    {
        if (it->i4FrmId == (MUINT32)i4FrmId)
        {
            pBuf = &(*it);
            //CAM_LOGD("[%s] OK i4Pos(%d)", __FUNCTION__, i4Pos);
            break;
        }
    }

    if (i4Pos == i4Size)
    {
        // does not exist
        CAM_LOGD("[%s] Ref NG i4Pos(%d)", __FUNCTION__, i4Pos);
    }

    return pBuf;
}


































