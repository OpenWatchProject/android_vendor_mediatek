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

#ifdef LOG_TAG
    #undef LOG_TAG
#endif

#define LOG_TAG "MtkCam/DualCamBokehPolicy"

#include "DualCamBokehPolicy.h"

#include <cutils/properties.h>
//
#include <mtkcam/feature/featurePipe/IStreamingFeaturePipe.h>
#include <mtkcam/feature/featurePipe/IStreamingFeaturePipe_var.h>
#include <mtkcam/feature/stereo/hal/stereo_setting_provider.h>
#include <mtkcam/feature/stereo/hal/stereo_size_provider.h>
//
#include <mtkcam/utils/metastore/IMetadataProvider.h>
//
#include <mtkcam/middleware/v1/LegacyPipeline/IResourceContainer.h>
#include <mtkcam/middleware/v1/LegacyPipeline/StreamId.h>
// for policy
#include <mtkcam/utils/hw/CamManager.h>
//
#include <mtkcam/utils/metadata/client/mtk_metadata_tag.h>
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>

#include <algorithm>

#include <mtkcam/feature/DualCam/utils/DualCameraHWHelper.h>

using NSCam::NSCamFeature::NSFeaturePipe::FeaturePipeParam;
using NSCam::NSCamFeature::VarMap;

std::future<void> DualCamBokehPolicy::tPolicySetting;

#undef MY_LOGV
#undef MY_LOGD
#undef MY_LOGI
#undef MY_LOGW
#undef MY_LOGE
#undef MY_LOGA
#undef MY_LOGF

#define MY_LOGV(id, fmt, arg...)        CAM_LOGV("[%d]id:%d[%s] " fmt, mRefCount, id, __FUNCTION__, ##arg)
#define MY_LOGD(id, fmt, arg...)        CAM_LOGD("[%d]id:%d[%s] " fmt, mRefCount, id, __FUNCTION__, ##arg)
#define MY_LOGI(id, fmt, arg...)        CAM_LOGI("[%d]id:%d[%s] " fmt, mRefCount, id, __FUNCTION__, ##arg)
#define MY_LOGW(id, fmt, arg...)        CAM_LOGW("[%d]id:%d[%s] " fmt, mRefCount, id, __FUNCTION__, ##arg)
#define MY_LOGE(id, fmt, arg...)        CAM_LOGE("[%d]id:%d[%s] " fmt, mRefCount, id, __FUNCTION__, ##arg)
#define MY_LOGA(id, fmt, arg...)        CAM_LOGA("[%d]id:%d[%s] " fmt, mRefCount, id, __FUNCTION__, ##arg)
#define MY_LOGF(id, fmt, arg...)        CAM_LOGF("[%d]id:%d[%s] " fmt, mRefCount, id, __FUNCTION__, ##arg)

//
#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( (cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( (cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)

#define FUNCTION_IN()             CAM_LOGI("[%d][%s]+", mRefCount, __FUNCTION__)
#define FUNCTION_OUT()            CAM_LOGI("[%d][%s]-", mRefCount, __FUNCTION__)

#define BOKEH_WAIT_STABLE_COUNT       (3)
#define BOKEH_WAIT_CAM_ACTIVE_COUNT   (3)

#define THERMAL_POLICY_NAME "thermal_policy_03"
#define DEBUG_VSDOF 1
/******************************************************************************
 *  Metadata Access
 ******************************************************************************/
namespace BokehPolicy
{
template <typename T>
static inline MBOOL
tryGetMetadata(
    IMetadata const* pMetadata,
    MUINT32 const tag,
    T & rVal
)
{
    if (pMetadata == NULL) {
        return MFALSE;
    }
    //
    IMetadata::IEntry entry = pMetadata->entryFor(tag);
    if(!entry.isEmpty()) {
        rVal = entry.itemAt(0, Type2Type<T>());
        return MTRUE;
    }
    //
    return MFALSE;
}
};
/******************************************************************************
 * implement
 ******************************************************************************/
namespace NSCam
{
MINT32 DualCamBokehPolicy::totalRefCount = 0;
/******************************************************************************
 *
 ******************************************************************************/
DualCamBokehPolicy::
DualCamBokehPolicy():
    mRefCount(totalRefCount)
{
    MY_LOGD(-1, "set thermal policy");
    if(DualCamBokehPolicy::tPolicySetting.valid())
    {
        MY_LOGD(-1, "have share state");
        DualCamBokehPolicy::tPolicySetting.get();
    }
    // set thermal policy first.
    DualCamBokehPolicy::tPolicySetting = std::async(
                std::launch::async,
                []()
                {
                    Utils::CamManager::getInstance()->setThermalPolicy(THERMAL_POLICY_NAME, 1);
                });
    DualCamBokehPolicy::totalRefCount++;
    // get main1 & main2 openid
    if(!StereoSettingProvider::getStereoSensorIndex(miMain1Id, miMain2Id))
    {
        MY_LOGE(-1, "Cannot get sensor ids from StereoSettingProvider! (%d,%d)", miMain1Id, miMain2Id);
    }
    // check main2 is mono or not
    MUINT sensorRawFmtType_Main2 = StereoSettingProvider::getSensorRawFormat(miMain2Id);
    if(sensorRawFmtType_Main2 == SENSOR_RAW_MONO)
    {
        mMain2ISPProfile = NSIspTuning::EIspProfile_N3D_Preview_toW;
    }
    querySensorActiveDomain();
    querySensorCropRegion();
    mLogLevel = property_get_int32("vendor.debug.dualcam.policy", 0);
    MY_LOGD(-1, "ctr(%p) m1(%d) m2(%d) mLogLevel(%d) m1p(%d) m2p(%d)",
                        this,
                        miMain1Id,
                        miMain2Id,
                        mLogLevel,
                        mMain1ISPProfile,
                        mMain2ISPProfile);
}
/******************************************************************************
 *
 ******************************************************************************/
DualCamBokehPolicy::
~DualCamBokehPolicy()
{
    FUNCTION_IN();
    if(DualCamBokehPolicy::tPolicySetting.valid())
    {
        MY_LOGD(-1, "have share state");
        DualCamBokehPolicy::tPolicySetting.get();
    }
    MY_LOGD(-1, "set thermal policy");
    DualCamBokehPolicy::tPolicySetting = std::async(
                std::launch::async,
                [this]()
                {
                    Utils::CamManager::getInstance()->setThermalPolicy(THERMAL_POLICY_NAME, 0);
                });
    //
    mIFrame3AControl = NULL;
    //
    mMgrCb = NULL;
    //
    mvOpenIds.clear();
    MY_LOGD(-1, "dctr(%p)", this);
    FUNCTION_OUT();
}
/******************************************************************************
 *
 ******************************************************************************/
MVOID
DualCamBokehPolicy::
setCameraOpenIds(
    vector<MINT32> ids
)
{
    // reset openid list
    mvOpenIds.clear();
    mvOpenIds = ids;
    //
    if(ids.size() > 2)
    {
        MY_LOGW(-1, "should not happened, please check flow");
    }
    //
    mMain1SensorStatus = MTK_SYNC_CAEMRA_STATE::MTK_SYNC_CAEMRA_INVALID;
    mMain2SensorStatus = MTK_SYNC_CAEMRA_STATE::MTK_SYNC_CAEMRA_INVALID;
    // find main1 openid
    std::vector<MINT32>::iterator it = std::find(ids.begin(), ids.end(), miMain1Id);
    mMain1SensorStatus = (it != ids.end()) ? MTK_SYNC_CAEMRA_STATE::MTK_SYNC_CAEMRA_ACTIVE : MTK_SYNC_CAEMRA_STATE::MTK_SYNC_CAEMRA_INVALID;
    // find main2 openid
    it = std::find(ids.begin(), ids.end(), miMain2Id);
    mMain2SensorStatus = (it != ids.end()) ? MTK_SYNC_CAEMRA_STATE::MTK_SYNC_CAMERA_INIT : MTK_SYNC_CAEMRA_STATE::MTK_SYNC_CAEMRA_INVALID;
    //
    auto toString = [](const MTK_SYNC_CAEMRA_STATE &sensorStatus)
    {
        switch(sensorStatus)
        {
            case MTK_SYNC_CAEMRA_STATE::MTK_SYNC_CAEMRA_ACTIVE:
                return "ACTIVE";
            case MTK_SYNC_CAEMRA_STATE::MTK_SYNC_CAEMRA_INVALID:
                return "INVALID";
            case MTK_SYNC_CAEMRA_STATE::MTK_SYNC_CAMERA_INIT:
                return "INIT";
            default:
                return "UNKNOW";
        }
    };
    MY_LOGD(-1, "m1(%s) m2(%s)", toString(mMain1SensorStatus), toString(mMain2SensorStatus));
}
/******************************************************************************
 *
 ******************************************************************************/
MVOID
DualCamBokehPolicy::
setObject(
    void* obj,
    MINT32 arg1
)
{
    if(obj != nullptr)
    {
        mIFrame3AControl = (IFrame3AControl*)obj;
    }
}
/******************************************************************************
 *
 ******************************************************************************/
MVOID
DualCamBokehPolicy::
setParameter(
    std::string str
)
{
}
/******************************************************************************
 *
 ******************************************************************************/
MVOID
DualCamBokehPolicy::
addCallBack(
    sp<ISyncManagerCallback> Cb
)
{
    MY_LOGD(-1,"set callback");
    if(Cb == nullptr)
    {
        MY_LOGD(-1, "set nullptr?");
    }
    mMgrCb = Cb;
}
/******************************************************************************
 *
 ******************************************************************************/
MVOID
DualCamBokehPolicy::
updateSetting(
    MINT32 const i4OpenId,
    IMetadata* appMetadata,
    IMetadata* halMetadata,
    SyncManagerParams &syncParams
)
{
    Mutex::Autolock _l(mLock);
    if(appMetadata == nullptr)
    {
        MY_LOGE(i4OpenId, "appMetadata is nullptr");
        return;
    }
    if(halMetadata == nullptr)
    {
        MY_LOGE(i4OpenId, "halMetadata is nullptr");
        return;
    }
    // check main2 first
    if(i4OpenId == miMain2Id)
    {
        {
            mbCanUsingMain2 = DECISION_STATUS::MAIN2_ENABLE;
            changeCameraState(i4OpenId, mbCanUsingMain2, appMetadata, halMetadata, mMain2SensorStatus);
            setDropDisplayRequest(i4OpenId, halMetadata);
        }
        // skip 3dnr
        {
            IMetadata::IEntry tag_3dnr(MTK_NR_FEATURE_3DNR_MODE);
            tag_3dnr.push_back(MTK_NR_FEATURE_3DNR_MODE_OFF, Type2Type<MINT32>());
            appMetadata->update(MTK_NR_FEATURE_3DNR_MODE, tag_3dnr);
        }
        // update active domain value
        {
            IMetadata::IEntry entry(MTK_SCALER_CROP_REGION);
            entry.push_back(mActiveArrayCrop_Main2, Type2Type<MRect>());
            appMetadata->update(MTK_SCALER_CROP_REGION, entry);
        }
        // update sensor crop
        {
            IMetadata::IEntry entry(MTK_P1NODE_SENSOR_CROP_REGION);
            entry.push_back(mSensorCropRegion_Main2, Type2Type<MRect>());
            halMetadata->update(MTK_P1NODE_SENSOR_CROP_REGION, entry);
        }
        // set isp profile
        {
            IMetadata::IEntry entry(MTK_3A_ISP_PROFILE);
            entry.push_back(mMain2ISPProfile, Type2Type< MUINT8 >());
            halMetadata->update(entry.tag(), entry);
        }
    }
    // Check main2 open condition, when main1 update setting.
    if(i4OpenId == miMain1Id)
    {
        updateParams(syncParams);
        // update dof level to metadata.
        updateDofLevel(i4OpenId, appMetadata);
        // main1 nerver drop
        {
            // if main2 not active, drop main1.
            //setDropDisplayRequest(i4OpenId, halMetadata);
            IMetadata::IEntry tag_drop(MTK_DUALZOOM_DROP_REQ);
            tag_drop.push_back(MTK_DUALZOOM_DROP_NEVER_DROP, Type2Type<MINT32>());
            halMetadata->update(MTK_DUALZOOM_DROP_REQ, tag_drop);
        }
        // update active domain value
        {
            IMetadata::IEntry entry(MTK_SCALER_CROP_REGION);
            entry.push_back(mActiveArrayCrop, Type2Type<MRect>());
            appMetadata->update(MTK_SCALER_CROP_REGION, entry);
        }
        // update sensor crop
        {
            IMetadata::IEntry entry(MTK_P1NODE_SENSOR_CROP_REGION);
            entry.push_back(mSensorCropRegion, Type2Type<MRect>());
            halMetadata->update(MTK_P1NODE_SENSOR_CROP_REGION, entry);
        }
        // set isp profile
        {
            IMetadata::IEntry entry(MTK_3A_ISP_PROFILE);
            entry.push_back(mMain1ISPProfile, Type2Type< MUINT8 >());
            halMetadata->update(entry.tag(), entry);
        }
    }
    MINT32 doSync = 0;
    if(mMain1SensorStatus == MTK_SYNC_CAEMRA_STATE::MTK_SYNC_CAEMRA_ACTIVE &&
       mMain2SensorStatus == MTK_SYNC_CAEMRA_STATE::MTK_SYNC_CAEMRA_ACTIVE)
    {
        doSync = 1;
    }
    else
    {
        doSync = 0;
    }
    //
    IMetadata::IEntry tag(MTK_DUALZOOM_DO_FRAME_SYNC);
    tag.push_back(doSync, Type2Type<MINT32>());
    halMetadata->update(MTK_DUALZOOM_DO_FRAME_SYNC, tag);
    //
    if(mSet3ASyncMode == MTK_DUAL_CAM_AAA_SYNC_STATE::ENABLE)
    {
        setMasterCamera(i4OpenId, halMetadata);
        if(i4OpenId == miMain1Id)
        {
        // enable 3sync & hwsync
            MINT32 mode = F3A_TYPE_FRAME_SYNC;
            bool enable3aSync = false;
            if(mbNeedSet3ASync)
            {
                MY_LOGI(i4OpenId, "enable 3a sync");
                mode |= F3A_TYPE_3A_SYNC;
                enable3aSync = true;
                mbNeedSet3ASync = MFALSE; // if workaround is disable, need uncomment.
            }
            changeCameraMode(i4OpenId, mode, enable3aSync, true, 0, 0,
                         appMetadata, halMetadata);
            // for workaround hwsync, only for ccu version
            /*if(mbNeedSet3ASync)
            {
                IMetadata::IEntry tag(MTK_STEREO_HW_FRM_SYNC_MODE);
                tag.push_back(NS3Av3::E_HW_FRM_SYNC_MODE::E_HW_FRM_SYNC_MODE_ON, Type2Type<MINT32>());
                halMetadata->update(MTK_STEREO_HW_FRM_SYNC_MODE, tag);
                mbNeedSet3ASync = MFALSE;
            }/*/
        }
    }
#if DEBUG_VSDOF
    // for vsdof check request sensor size in hal metadata
    {
        MSize sensorSize;
        if(!BokehPolicy::tryGetMetadata<MSize>(halMetadata, MTK_HAL_REQUEST_SENSOR_SIZE, sensorSize))
        {
            MY_LOGE(i4OpenId, "cannot get MTK_HAL_REQUEST_SENSOR_SIZE");
        }
        else
        {
            MY_LOGD(i4OpenId, "Sensor size: (%dx%d)", sensorSize.w, sensorSize.h);
        }
        MUINT8 bNeedExif;
        if(!BokehPolicy::tryGetMetadata<MUINT8>(halMetadata, MTK_HAL_REQUEST_REQUIRE_EXIF, bNeedExif))
        {
            MY_LOGE(i4OpenId, "cannot get MTK_HAL_REQUEST_REQUIRE_EXIF");
        }
        else
        {
            MY_LOGD(i4OpenId, "need exif: (%d)", bNeedExif);
        }
    }
#endif
    MY_LOGD(i4OpenId, "-");
}
/******************************************************************************
 *
 ******************************************************************************/
MVOID
DualCamBokehPolicy::
updateAfterP1(
    MINT32 const i4OpenId,
    MUINT32 const frameNo,
    void* obj,
    MINT32 arg1
)
{
    return;
}
/******************************************************************************
 *
 ******************************************************************************/
MVOID
DualCamBokehPolicy::
updateFeatureMask(
    MUINT32 &featureMask
)
{
}
/******************************************************************************
 *
 ******************************************************************************/
MVOID
DualCamBokehPolicy::
lock(
    void* arg1
)
{
    FUNCTION_IN();
    Mutex::Autolock _l(mLock);
    if(arg1 == nullptr)
    {
        MY_LOGD(-1, "arg1 is nullptr");
        return;
    }
    if(MTK_SYNC_CAEMRA_STATE::MTK_SYNC_CAEMRA_ACTIVE != mMain2SensorStatus)
    {
        MY_LOGD(-1, "todo: need wait until main2 is active");
    }
    PolicyLockParams* params = (PolicyLockParams*) arg1;
    if(params != nullptr)
    {
        MY_LOGD(-1, "main2 is active");
        params->bUseDualCamShot = MTRUE;
        params->bIsMain2On = MTRUE;
    }
lbExit:
    FUNCTION_OUT();
}
/******************************************************************************
 *
 ******************************************************************************/
MVOID
DualCamBokehPolicy::
unlock(
    void* arg1
)
{
    FUNCTION_IN();
    Mutex::Autolock _l(mLock);
    FUNCTION_OUT();
}


/******************************************************************************
 *
 ******************************************************************************/
MINT32
DualCamBokehPolicy::
getPreviewCameraId(MINT32 *dropOthers)
{
    if (dropOthers)
    {
        *dropOthers = 0;
    }
    return miMain1Id;
}

/******************************************************************************
 *
 ******************************************************************************/
void
DualCamBokehPolicy::
setDropDisplayRequest(
    MINT32 const openId,
    IMetadata* halMetadata
)
{
    typedef MTK_SYNC_CAEMRA_STATE STATE;
    MBOOL drop_all = MFALSE;
    MBOOL drop_p2  = MFALSE;

    switch(mMain2SensorStatus){
        case STATE::MTK_SYNC_CAEMRA_GOTO_ACTIVE:
        case STATE::MTK_SYNC_CAMERA_INIT:
            drop_p2 = MTRUE;
            break;
        case STATE::MTK_SYNC_CAEMRA_ACTIVE:
            // need p1 + p2
            break;
        default:
            MY_LOGE(openId, "unknown status:%d, drop all", mMain2SensorStatus);
            drop_all = MTRUE;
    }

    MY_LOGD(openId, "main2 status(%d) drop_all(%d) drop_p2(%d)",
        mMain2SensorStatus, drop_all, drop_p2
    );

    if(drop_all)
    {
        MY_LOGD_IF(mLogLevel>0, openId, "drop request");
        IMetadata::IEntry tag_drop(MTK_DUALZOOM_DROP_REQ);
        tag_drop.push_back(MTK_DUALZOOM_DROP_DIRECTLY, Type2Type<MINT32>());
        halMetadata->update(MTK_DUALZOOM_DROP_REQ, tag_drop);
    }
    else if(drop_p2)
    {
        MY_LOGD_IF(mLogLevel>0, openId, "need p1");
        IMetadata::IEntry tag_drop(MTK_DUALZOOM_DROP_REQ);
        tag_drop.push_back(MTK_DUALZOOM_DROP_NEED_P1, Type2Type<MINT32>());
        halMetadata->update(MTK_DUALZOOM_DROP_REQ, tag_drop);
    }
    else
    {
        /*if(openId == miMain1Id)
        {
            MY_LOGD_IF(mLogLevel>0, openId, "main2 ready no drop main frame");
            IMetadata::IEntry tag_drop(MTK_DUALZOOM_DROP_REQ);
            tag_drop.push_back(MTK_DUALZOOM_DROP_NONE, Type2Type<MINT32>());
            halMetadata->update(MTK_DUALZOOM_DROP_REQ, tag_drop);
        }
        else*/
        {
        MY_LOGD_IF(mLogLevel>0, openId, "need p1 and p2");
        IMetadata::IEntry tag_drop(MTK_DUALZOOM_DROP_REQ);
        tag_drop.push_back(MTK_DUALZOOM_DROP_NEED_SYNCMGR_NEED_STREAM_F_PIPE, Type2Type<MINT32>());
        halMetadata->update(MTK_DUALZOOM_DROP_REQ, tag_drop);
        }
    }
}
/******************************************************************************
 *
 ******************************************************************************/
void
DualCamBokehPolicy::
updateParams(
    SyncManagerParams &syncParams
)
{
    miZoomRatio = syncParams.miZoomRatio;
    mDofLevel = syncParams.mDofLevel;
}
/******************************************************************************
 *
 ******************************************************************************/
void
DualCamBokehPolicy::
updateDofLevel(
    MINT32 const openId,
    IMetadata* halMetadata
)
{
    IMetadata::IEntry entry(MTK_STEREO_FEATURE_DOF_LEVEL);
    entry.push_back(mDofLevel, Type2Type< MINT32 >());
    halMetadata->update(entry.tag(), entry);
    MY_LOGD(openId, "Set DofLevel from UI (%d)", mDofLevel);
}
/******************************************************************************
 *
 ******************************************************************************/
void
DualCamBokehPolicy::
querySensorActiveDomain()
{
    if(!StereoSizeProvider::getInstance()->getPass1ActiveArrayCrop(StereoHAL::eSTEREO_SENSOR_MAIN1, mActiveArrayCrop)){
        MY_LOGE(-1, "can't get active arrayCrop from StereoSizeProvider for eSTEREO_SENSOR_MAIN1");
        return;
    }
    if(!StereoSizeProvider::getInstance()->getPass1ActiveArrayCrop(StereoHAL::eSTEREO_SENSOR_MAIN2, mActiveArrayCrop_Main2)){
        MY_LOGE(-1, "can't get active arrayCrop from StereoSizeProvider for eSTEREO_SENSOR_MAIN2");
        return;
    }

    MY_LOGD(-1, "StereoSizeProvider => active array crop main1(%d,%d,%dx%d), main2(%d,%d,%dx%d)",
        mActiveArrayCrop.p.x,
        mActiveArrayCrop.p.y,
        mActiveArrayCrop.s.w,
        mActiveArrayCrop.s.h,
        mActiveArrayCrop_Main2.p.x,
        mActiveArrayCrop_Main2.p.y,
        mActiveArrayCrop_Main2.s.w,
        mActiveArrayCrop_Main2.s.h
    );
}
/******************************************************************************
 *
 ******************************************************************************/
void
DualCamBokehPolicy::
querySensorCropRegion()
{
    MINT imgofmt_main1 = -1, rrzofmt_main1 = -1;
    MINT imgofmt_main2 = -1, rrzofmt_main2 = -1;
    // get raw fmt
    DualCameraHWHelper::tryGetRawFormat(miMain1Id, imgofmt_main1, rrzofmt_main1);
    DualCameraHWHelper::tryGetRawFormat(miMain2Id, imgofmt_main2, rrzofmt_main2);
    // get sensor crop
    MUINT32 q_stride;
    NSCam::MSize size;
    // main1
    StereoSizeProvider::getInstance()->getPass1Size(
            StereoHAL::eSTEREO_SENSOR_MAIN1,
            (EImageFormat)rrzofmt_main1,
            NSImageio::NSIspio::EPortIndex_RRZO,
            StereoHAL::eSTEREO_SCENARIO_CAPTURE, // in this mode, stereo only support zsd.
            (MRect&)mSensorCropRegion,
            size,
            q_stride);
    // main2
    StereoSizeProvider::getInstance()->getPass1Size(
            StereoHAL::eSTEREO_SENSOR_MAIN2,
            (EImageFormat)rrzofmt_main2,
            NSImageio::NSIspio::EPortIndex_RRZO,
            StereoHAL::eSTEREO_SCENARIO_CAPTURE, // in this mode, stereo only support zsd.
            (MRect&)mSensorCropRegion_Main2,
            size,
            q_stride);

    MY_LOGD(-1, "StereoSizeProvider => sensor crop main1(%d,%d,%dx%d), main2(%d,%d,%dx%d)",
        mSensorCropRegion.p.x,
        mSensorCropRegion.p.y,
        mSensorCropRegion.s.w,
        mSensorCropRegion.s.h,
        mSensorCropRegion_Main2.p.x,
        mSensorCropRegion_Main2.p.y,
        mSensorCropRegion_Main2.s.w,
        mSensorCropRegion_Main2.s.h
    );
}
/******************************************************************************
 *
 ******************************************************************************/
status_t
DualCamBokehPolicy::
changeCameraMode(
    MINT32  const               openId,
    MINT32                      type,
    MBOOL                       enableAAASync,
    MBOOL                       enableHWSync,
    MINT32                      minFps,
    MINT32                      maxFps,
    IMetadata*                  appMetadata,
    IMetadata*                  halMetadata)
{
    Frame3ASetting setting =
    {
        .openId   = openId,
        .is3ASyncEnabled = enableAAASync,
        .hwSyncMode = (MUINT32)enableHWSync,
        .minFps          = minFps,
        .maxFps          = maxFps,
    };
    if (mIFrame3AControl != NULL)
    {
        mIFrame3AControl->set(&setting, type, appMetadata, halMetadata);
    }

    return OK;
}
/****************************************setMasterCamera**************************************
 *
 ******************************************************************************/
status_t
DualCamBokehPolicy::
setMasterCamera(
    MINT32  const               openId,
    IMetadata*                  halMetadata)
{
    Frame3ASetting setting =
    {
        .openId   = openId,
        .masterId = miMain1Id,
        .slaveId  = miMain2Id,
    };
    MINT32 type = F3A_TYPE_MASTER_SLAVE_SET;

    if (mIFrame3AControl != NULL)
    {
        mIFrame3AControl->set(&setting, type, NULL, halMetadata);
    }

    return OK;
}
/******************************************************************************
 *
 ******************************************************************************/
void
DualCamBokehPolicy::
changeCameraState(
    MINT32  const               openId,
    DECISION_STATUS    const    canUsingMain2,
    IMetadata*                  appMetadata,
    IMetadata*                  halMetadata,
    MTK_SYNC_CAEMRA_STATE&      state
)
{
    auto stateToString = [](MTK_SYNC_CAEMRA_STATE &state)
    {
        switch(state)
        {
            case MTK_SYNC_CAEMRA_STATE::MTK_SYNC_CAEMRA_GOTO_STANDBY:
                return "GO_TO_STANDBY";
            case MTK_SYNC_CAEMRA_STATE::MTK_SYNC_CAEMRA_STANDBY:
                return "STANDBY";
            case MTK_SYNC_CAEMRA_STATE::MTK_SYNC_CAEMRA_GOTO_ACTIVE:
                return "GO_TO_ACTIVE";
            case MTK_SYNC_CAEMRA_STATE::MTK_SYNC_CAEMRA_ACTIVE:
                return "ACTIVE";
            case MTK_SYNC_CAEMRA_STATE::MTK_SYNC_CAMERA_INIT:
                return "INIT";
        }
        return "";
    };
    auto decision_to_string = [](DECISION_STATUS status)
    {
        switch(status)
        {
            case DECISION_STATUS::UNSTABLE:
                return "UNSTABLE";
            case DECISION_STATUS::MAIN2_ENABLE:
                return "MAIN2_ENABLE";
            case DECISION_STATUS::MAIN2_DISABLE:
                return "MAIN2_DISABLE";
        }
        return "";
    };
    MY_LOGD_IF(mPreMain2SensorStatus != state, openId, "main2(%s) state(%s) mWaitEnterAct(%d)",
                decision_to_string(canUsingMain2),
                stateToString(state),
                mWaitEnterAct);
    //
    if(state == MTK_SYNC_CAEMRA_STATE::MTK_SYNC_CAMERA_INIT)
    {
        if(canUsingMain2 == DECISION_STATUS::MAIN2_ENABLE)
        {
            // prepare to active
            mSet3ASyncMode = MTK_DUAL_CAM_AAA_SYNC_STATE::ENABLE;
            //
            state = MTK_SYNC_CAEMRA_STATE::MTK_SYNC_CAEMRA_GOTO_ACTIVE;
            //
            mWaitEnterAct = BOKEH_WAIT_CAM_ACTIVE_COUNT;
        }
    }
    else if (state == MTK_SYNC_CAEMRA_STATE::MTK_SYNC_CAEMRA_GOTO_ACTIVE)
    {
        if (mWaitEnterAct <= 0)
        {
            MY_LOGD(openId, "active done");
            state = MTK_SYNC_CAEMRA_STATE::MTK_SYNC_CAEMRA_ACTIVE;
        }
        else
        {
            mWaitEnterAct--;
        }
    }
    mPreMain2SensorStatus = state;
}
};