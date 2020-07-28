/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2019. All rights reserved.
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

/**
 * @file ISPProfileMapping.cpp
 * @brief Manage the ISP profile mapping from feature/sensor combination/stages
 *
 */

#define LOG_TAG "ISPProfileMapping"
#include <mtkcam/utils/std/ULog.h>
#include <mtkcam3/feature/utils/ISPProfileMapper.h>
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>
#include <mtkcam/utils/metadata/client/mtk_metadata_tag.h>
#include <mtkcam/utils/metadata/IMetadata.h>
#include <mtkcam/utils/TuningUtils/CommonRule.h>

CAM_ULOG_DECLARE_MODULE_ID(MOD_ISP_PROFILE_MAPPER);

#define MY_LOGV(fmt, arg...)                  CAM_ULOGMV("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)                  CAM_ULOGMD("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)                  CAM_ULOGMI("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)                  CAM_ULOGMW("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)                  CAM_ULOGME("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)                  CAM_ULOGM_ASSERT("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)                  CAM_ULOGM_FATAL("[%s] " fmt, __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( (cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( (cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)

using namespace NSIspTuning;
using namespace NSCam;

// for the not supported chip, define the required-member
#if MTKCAM_ENABLE_ISPPROFILE_MAPPER == 0
const std::vector<ProfileMappingItem> gISPProfileMappingArray;
const std::vector<ProfileMappingItem> gISPProfileMappingArray_ISPHIDL;
const std::map<EProfileMappingStages, NSIspTuning::EIspProfile_T> gDefaultProfileMap;
#endif

/**
 * @brief Try to get metadata value
 * @param [in] pMetadata IMetadata instance
 * @param [in] tag the metadata tag to retrieve
 * @param [out] rVal the metadata value to be stored.
 * @return
 * - MTRUE indicates success.
 * - MFALSE indicates failure.
 */
template <typename T>
inline MBOOL
tryGetMetadata(
    IMetadata* pMetadata,
    MUINT32 const tag,
    T & rVal
)
{
    if( pMetadata == NULL ) {
        MY_LOGW("pMetadata == NULL");
        return MFALSE;
    }
    //
    IMetadata::IEntry entry = pMetadata->entryFor(tag);
    if( !entry.isEmpty() ) {
        rVal = entry.itemAt(0, Type2Type<T>());
        return MTRUE;
    }
    return MFALSE;
}

/**
 * @brief Try to set metadata value
 * @param [in] pMetadata IMetadata instance
 * @param [in] tag the metadata tag to configure
 * @param [in] rVal the metadata value to set
 * @return
 * - MTRUE indicates success.
 * - MFALSE indicates failure.
 */
template <typename T>
inline MVOID
trySetMetadata(
    IMetadata* pMetadata,
    MUINT32 const tag,
    T const& val
)
{
    if( pMetadata == NULL ) {
        MY_LOGW("pMetadata == NULL");
        return;
    }
    //
    IMetadata::IEntry entry(tag);
    entry.push_back(val, Type2Type<T>());
    pMetadata->update(tag, entry);
}

ISPProfileMapper*
ISPProfileMapper::
getInstance()
{
    static ISPProfileMapper instance;
    return &instance;
}

ISPProfileMapper::
ISPProfileMapper()
{
    _init();
}

void
ISPProfileMapper::
_init()
{
    // Default case
    for(ProfileMappingItem item : gISPProfileMappingArray)
    {
        const ProfileMappingItemKeys& key = item.mKey;
        MappingKey datakey = std::make_tuple(key.miFeatureID, key.mSensorComb, key.mScenario);
        miKeyFeatures = miKeyFeatures | key.miFeatureID;
        MY_LOGD("Proprietary table: key.miFeatureID=0x%" PRIx64 " miKeyFeatures=0x%" PRIx64 " ", key.miFeatureID, miKeyFeatures);
        mMappingItems[datakey] = item.mProfileMap;
        // apply default value
        for(auto itr=gDefaultProfileMap.begin(); itr!=gDefaultProfileMap.end();++itr)
        {
            if(mMappingItems[datakey].count(itr->first) == 0)
                mMappingItems[datakey][itr->first] = itr->second;
        }
    }
    // ISP HIDL case
    for(ProfileMappingItem item : gISPProfileMappingArray_ISPHIDL)
    {
        const ProfileMappingItemKeys& key = item.mKey;
        MappingKey datakey = std::make_tuple(key.miFeatureID, key.mSensorComb, key.mScenario);
        mMappingItems_ISPHIDL[datakey] = item.mProfileMap;
        // apply default value
        for(auto itr=gDefaultProfileMap.begin(); itr!=gDefaultProfileMap.end();++itr)
        {
            if(mMappingItems_ISPHIDL[datakey].count(itr->first) == 0)
                mMappingItems_ISPHIDL[datakey][itr->first] = itr->second;
        }
    }

}

MBOOL
ISPProfileMapper::
removeFeature(
    MUINT64 featureID,
    ProfileMapperKey& rMappingKey
)
{
    if (rMappingKey.type == eMappingKey_Proprietary) {
        rMappingKey.itemKey.miFeatureID &= (~featureID);
        MY_LOGD(" remove feature=0x%" PRIx64 " after featureID=0x%" PRIx64 " ", featureID, rMappingKey.itemKey.miFeatureID);
    } else {
        MY_LOGD(" No remove feature=0x%" PRIx64 " becasue it is not proprietary", featureID, rMappingKey.type);
    }
    return MTRUE;
}

bool
ISPProfileMapper::
mappingProfile(
    const ProfileMapperKey& key,
    const EProfileMappingStages& stage,
    EIspProfile_T& profile
)
{
    std::map<MappingKey, StageProfileMap> *pMappingItem = NULL;
    if(key.type == eMappingKey_Proprietary)
        pMappingItem = &mMappingItems;
    else
        pMappingItem = &mMappingItems_ISPHIDL;

    MappingKey dataKey = std::make_tuple(key.itemKey.miFeatureID, key.itemKey.mSensorComb, key.itemKey.mScenario);
    if((*pMappingItem).count(dataKey) == 0 || (*pMappingItem)[dataKey].count(stage) == 0)
    {
        MY_LOGE("No match profile, isProprietary:%d/FID=0x%" PRIx64 "/SenComb=%d/Scenario=%d/Stage=%d",
                key.type == eMappingKey_Proprietary, key.itemKey.miFeatureID,
                key.itemKey.mSensorComb, key.itemKey.mScenario, stage);
        return false;
    }

    profile  = (*pMappingItem)[dataKey][stage];
    MY_LOGD("isProprietary:%d/FID=0x%" PRIx64 "/SenComb=%d/Scenario=%d/Stage=%d, found IspProfile=%s(%d)",
                key.type == eMappingKey_Proprietary, key.itemKey.miFeatureID,
                key.itemKey.mSensorComb, key.itemKey.mScenario, stage,
                TuningUtils::getIspProfileName(profile), profile);
    return true;
}

const ProfileMapperKey
ISPProfileMapper::
queryMappingKey(
    IMetadata* pAppMeta,
    IMetadata* pInHalMeta,
    MBOOL isPhysical,
    EProfileMappinScenario scenario,
    ESensorCombination sensorCom
)
{
    ProfileMapperKey key;
    // check isp hidle pipeline
    MINT32 pipelineMode = -1;
    MBOOL bExistMeta = tryGetMetadata<MINT32>(pInHalMeta, MTK_HAL_REQUEST_ISP_PIPELINE_MODE, pipelineMode);
    MBOOL bIsISPHIDLPipe = bExistMeta && pipelineMode;
    //
    MINT32 iCaptureHint =  MTK_CONTROL_CAPTURE_HINT_FOR_ISP_TUNING_DEFAULT_NONE;
    if(tryGetMetadata<MINT32>(pAppMeta, MTK_CONTROL_CAPTURE_HINT_FOR_ISP_TUNING, iCaptureHint) || bIsISPHIDLPipe)
    {
        key.type = eMappingKey_ISPHIDL;
        key.itemKey.mScenario = scenario;
        key.itemKey.miFeatureID = iCaptureHint;
        key.itemKey.mSensorComb = sensorCom;
        MY_LOGD("Capture Hint For ISP Tuning: %d, isMetaExist:%d isISPHidl:%d",
                    iCaptureHint, bExistMeta, bIsISPHIDLPipe);
    }
    else
    {
        MINT64 iFeatureID = 0;
        if(isPhysical && !tryGetMetadata<MINT64>(pInHalMeta, MTK_FEATURE_CAPTURE_PHYSICAL, iFeatureID)) {
            MY_LOGE("Failed to find MTK_FEATURE_CAPTURE_PHYSICAL");
            key.type = eMappingKey_NonValid;
        }
        else if(!isPhysical && !tryGetMetadata<MINT64>(pInHalMeta, MTK_FEATURE_CAPTURE, iFeatureID))
        {
            MY_LOGE("Failed to find MTK_FEATURE_CAPTURE");
            key.type = eMappingKey_NonValid;
        }
        else
        {
            key.type = eMappingKey_Proprietary;
            key.itemKey.mScenario = scenario;
            MUINT64 remappFID = iFeatureID & miKeyFeatures;
            MY_LOGD("FID:0x%" PRIx64 " RemappedFID:0x%" PRIx64 " ", iFeatureID, remappFID);
            key.itemKey.miFeatureID = remappFID;
            key.itemKey.mSensorComb = sensorCom;
        }
    }
    return key;
}

bool
ISPProfileMapper::
isSupported()
{
    return MTKCAM_ENABLE_ISPPROFILE_MAPPER;
}
