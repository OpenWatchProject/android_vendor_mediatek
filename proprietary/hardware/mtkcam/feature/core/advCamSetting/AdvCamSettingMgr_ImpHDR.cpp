/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2016. All rights reserved.
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

#define LOG_TAG "MtkCam/AdvCamSettingMgrHDR"

#include <cutils/properties.h>
#include <mtkcam/utils/std/Log.h>
#include <mtkcam/utils/metastore/IMetadataProvider.h>
#include <mtkcam/utils/metadata/client/mtk_metadata_tag.h>
#include <mtkcam/utils/std/TypeTraits.h>
#include <mtkcam/utils/hw/HwTransform.h>

//#include <mtkcam/feature/utils/FeatureProfileHelper.h>

#include "AdvCamSettingMgr_Imp.h"

using namespace NSCam;
using namespace NSCamHW;
using namespace android;

#undef __func__
#define __func__ __FUNCTION__

#define ACSM_LOG(fmt, arg...)    CAM_LOGD("[%s]" fmt, __func__, ##arg)
#define ACSM_INF(fmt, arg...)    CAM_LOGI("[%s]" fmt, __func__, ##arg)
#define ACSM_WRN(fmt, arg...)    CAM_LOGW("[%s] WRN(%5d):" fmt, __func__, __LINE__, ##arg)
#define ACSM_ERR(fmt, arg...)    CAM_LOGE("[%s] %s ERROR(%5d):" fmt, __func__,__FILE__, __LINE__, ##arg)

#define MY_LOGD_IF(cond, ...)       do { if ( (cond) >= (1) ) { ACSM_LOG(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) >= (1) ) { ACSM_INF(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) >= (1) ) { ACSM_WRN(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) >= (1) ) { ACSM_ERR(__VA_ARGS__); } }while(0)

/*******************************************************************************
* VHDR function
********************************************************************************/
/*******************************************************************************
* get available VHDR mode from metadata provider
********************************************************************************/
MUINT32 AdvCamSettingMgr_Imp::getSupportedVHDRMode(const MUINT32 sensorId)
{
    sp<IMetadataProvider> metaProvider = NSMetadataProviderManager::valueFor((int32_t)sensorId);
    if(metaProvider == NULL){
        ACSM_ERR("Can not get metadata provider for search vhdr mode!! set vhdrMode to none");
        return SENSOR_VHDR_MODE_NONE;
    }
    IMetadata staMeta = metaProvider->getMtkStaticCharacteristics();
    IMetadata::IEntry availVhdrEntry = staMeta.entryFor(MTK_HDR_FEATURE_AVAILABLE_VHDR_MODES);

    for(size_t i = 0 ; i < availVhdrEntry.count() ; i++)
    {
        if( availVhdrEntry.itemAt(i, Type2Type<MINT32>()) != SENSOR_VHDR_MODE_NONE)
        {
            return (MUINT32)availVhdrEntry.itemAt(i, Type2Type<MINT32>());
        }
    }
    ACSM_ERR("Can not get supported vhdr mode from MetaProvider!! (maybe FO not set?), set vhdrMode to none");
    return SENSOR_VHDR_MODE_NONE;
}

/*******************************************************************************
* get HDR mode from app metadata (also force set HDR mode from property in this function)
********************************************************************************/
HDRMode AdvCamSettingMgr_Imp::getHDRMode(const IMetadata*  appMetadata)
{
    HDRMode hdrMode = HDRMode::OFF;
    MINT32 hdrModeInt = 0;
    if(IMetadata::getEntry<MINT32>(appMetadata, MTK_HDR_FEATURE_HDR_MODE, hdrModeInt)){
        hdrMode = static_cast<HDRMode>((MUINT8)hdrModeInt);
    }

    if(mLogLevel >= 1)
    {
        MINT32 hdrModePpt = property_get_int32("vendor.debug.camera.vhdr.sethdrmode", -1);
        if(hdrModePpt >= 0 && hdrModePpt < toLiteral(HDRMode::NUM))
        {
            ACSM_LOG("Property set hdrmode app metadata:%d",hdrModePpt);
            hdrMode = static_cast<HDRMode>((MUINT8)hdrModePpt);
        }
    }
    return hdrMode;
}

/*******************************************************************************
* get VHDR mode with the condition of HDR mode
********************************************************************************/
MUINT32 AdvCamSettingMgr_Imp::getVHDRMode(const HDRMode &hdrMode,const sp<AdvCamSetting> &curSet)
{
    if (hdrMode == HDRMode::VIDEO_ON || hdrMode == HDRMode::VIDEO_AUTO){
        if(curSet == NULL){
            // First time get VHDRMode
            return getSupportedVHDRMode(mSensorIdx);
        } else {
            return curSet->vhdrMode;
        }
    } else {
        return SENSOR_VHDR_MODE_NONE;
    }
}

/*******************************************************************************
* if the status of vhdr mode change, need re-config
* @param[in/out] hdr_Mode : it could update hdr_Mode value
********************************************************************************/
MBOOL AdvCamSettingMgr_Imp::needReconfigByHDR(const IMetadata* appMetadata,const sp<AdvCamSetting> &curSet)
{
    HDRMode hdr_Mode = getHDRMode(appMetadata);
    MBOOL needReconfig = MFALSE;
#if (1 == VHDR_SUPPORTED)
    if(curSet == NULL || curSet->vhdrMode == SENSOR_VHDR_MODE_NONE) // current vhdr Off
    {
        if(hdr_Mode == HDRMode::VIDEO_ON || hdr_Mode == HDRMode::VIDEO_AUTO) // hope vhdr on
            needReconfig = MTRUE;
    }
    else // current vhdr On
    {
        if(hdr_Mode == HDRMode::OFF || hdr_Mode == HDRMode::ON || hdr_Mode == HDRMode::AUTO) // hope vhdr off
            needReconfig = MTRUE;
    }
    if(needReconfig)
    {
        MUINT32 curVhdrMode = (curSet == NULL) ? SENSOR_VHDR_MODE_NONE : curSet->vhdrMode;
        ACSM_LOG("Need Reconfig by vhdr changed. cur_vhdr(%u), future_HdrMode(%hhu)", curVhdrMode, hdr_Mode);
    }
#else
    (void)curSet;
    if(hdr_Mode == HDRMode::VIDEO_ON || hdr_Mode == HDRMode::VIDEO_AUTO) // hope vhdr on but vhdr not support
        ACSM_ERR("HDR Mode is %hhu but VHDR is not supported by FO !!", hdr_Mode);
#endif
    return needReconfig;
}

