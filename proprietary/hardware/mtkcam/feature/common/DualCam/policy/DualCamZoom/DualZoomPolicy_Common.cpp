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
 * MediaTek Inc. (C) 2016. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,20
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
/******************************************************************************
*
*******************************************************************************/
#define LOG_TAG "MtkCam/DualCamZoomPolicy/Common"

// Standard C header file

// Android system/core header file

// mtkcam custom header file
#include <camera_custom_dualzoom.h>
#include <camera_custom_dualzoom_func.h>

// mtkcam global header file
#include <cutils/properties.h>
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>
#include <mtkcam/utils/metastore/IMetadataProvider.h>
#include <mtkcam/feature/stereo/hal/stereo_setting_provider.h>

// Module header file
#include <mtkcam/feature/DualCam/utils/DualCameraUtility.h>
#include <mtkcam/feature/DualCam/utils/DualCameraHWHelper.h>
#include <mtkcam/drv/mem/cam_cal_drv.h>
#include <mtkcam/feature/DualCam/FOVHal.h>

// Local header file
#include "DualZoomPolicy_Common.h"
#include "../utils/DualCameraUtility.DualZoomPolicy.PropDef.h"
/******************************************************************************
*
*******************************************************************************/
#define COMMON_LOGV(fmt, arg...)    CAM_LOGV("[%s] " fmt, __FUNCTION__, ##arg)
#define COMMON_LOGD(fmt, arg...)    CAM_LOGD("[%s] " fmt, __FUNCTION__, ##arg)
#define COMMON_LOGI(fmt, arg...)    CAM_LOGI("[%s] " fmt, __FUNCTION__, ##arg)
#define COMMON_LOGW(fmt, arg...)    CAM_LOGW("[%s] " fmt, __FUNCTION__, ##arg)
#define COMMON_LOGE(fmt, arg...)    CAM_LOGE("[%s] " fmt, __FUNCTION__, ##arg)
#define COMMON_LOGA(fmt, arg...)    CAM_LOGA("[%s] " fmt, __FUNCTION__, ##arg)
#define COMMON_LOGF(fmt, arg...)    CAM_LOGF("[%s] " fmt, __FUNCTION__, ##arg)

#define COMMON_FUN_IN()             COMMON_LOGD("+")
#define COMMON_FUN_OUT()            COMMON_LOGD("-")

class scoped_tracer
{
public:
    scoped_tracer(const char* functionName)
    : mFunctionName(functionName)
    {
        CAM_LOGD("[%s] +", mFunctionName);
    }
    ~scoped_tracer()
    {
        CAM_LOGD("[%s] -", mFunctionName);
    }
private:
    const char* const mFunctionName;
};
#define COMMON_SCOPED_TRACER() scoped_tracer ___scoped_tracer(__FUNCTION__ );


/******************************************************************************
* namespace start
*******************************************************************************/
namespace NSCam
{
using Utility = DualCameraUtility;
/******************************************************************************
 * DualZoomPolicyHelper
******************************************************************************/
MBOOL
DualZoomPolicyHelper::
getDynamicTwinSupported()
{
    static const MBOOL ret = DualCameraHWHelper::getDynamicTwinSupported();
    return ret;
}
//
MBOOL
DualZoomPolicyHelper::
getWideDymanicTwinSupported()
{
    return DualZoomPolicyHelper::getDynamicTwinSupported() && true;
}
//
MBOOL
DualZoomPolicyHelper::
getEnableHWControlFlow()
{
    // using for status test, if disable it
    return true;
}
//
MBOOL
DualZoomPolicyHelper::
getWideStandbySupported()
{
    #if (DUALZOOM_WIDE_STANDY_EN == 1)
    {
        return true;
    }
    #else
    {
        return false;
    }
    #endif // DUALZOOM_WIDE_STANDY_EN
}
//
MBOOL
DualZoomPolicyHelper::
getMultiDistanceFovSupported()
{
    static const MBOOL ret = FOVHal::isSupportMultiFov();
    return ret;
}
//
MINT32
DualZoomPolicyHelper::
getLogLevel()
{
    return DualCameraUtility::getProperty<DUALZOOM_POLICY_LOG_LEVEL>();
}
//
MBOOL
DualZoomPolicyHelper::
tryGetCorrespondingCameraOpenId(OpenId openId, OpenId& correspondingOpenId)
{
    MBOOL ret = true;
    if(openId == DUALZOOM_WIDE_CAM_ID)
    {
        correspondingOpenId = DUALZOOM_TELE_CAM_ID;
    }
    else if(openId == DUALZOOM_TELE_CAM_ID)
    {
        correspondingOpenId = DUALZOOM_WIDE_CAM_ID;
    }
    else
    {
        COMMON_LOGD("invalid openId: %d", openId);
        ret = false;
    }
    return ret;
}
//
OpenId
DualZoomPolicyHelper::
getCorrespondingCameraOpenId(OpenId openId)
{
    OpenId ret = -1;
    tryGetCorrespondingCameraOpenId(openId, ret);
    return ret;
}
//
MBOOL
DualZoomPolicyHelper::
tryGetSensorActiveArray(MINT32 openId, MRect& activeArray)
{
    MBOOL ret = false;
    sp<IMetadataProvider> pMetadataProvider = NSMetadataProviderManager::valueFor(openId);
    if (!pMetadataProvider.get())
    {
        COMMON_LOGD("openId: %d, failed to get metadata provider", openId);
    }
    else
    {
        IMetadata static_meta = pMetadataProvider->getMtkStaticCharacteristics();
        {
            if(Utility::tryGetMetadata(static_meta, MTK_SENSOR_INFO_ACTIVE_ARRAY_REGION, activeArray))
            {
                COMMON_LOGD("openId: %d, sensor active array: (%d, %d, %d, %d)", openId, activeArray.p.x, activeArray.p.y, activeArray.s.w, activeArray.s.h);
                ret = true;
            }
            else
            {
                COMMON_LOGD("openId: %d, failed to get metadata: MTK_SENSOR_INFO_ACTIVE_ARRAY_REGION", openId);
            }
        }
    }
    return ret;
};

/******************************************************************************
 * Frame3AControlHelper
******************************************************************************/
MVOID
Frame3AControlHelper::
setFrame3AControl(OpenId openId, Fram3ASettingType type, MBOOL enable3ASync, MBOOL enableHWSync, MINT32 minFps, MINT32 maxFps, IMetadata* appMetadata, IMetadata* halMetadata, sp<IFrame3AControl>& frame3AControl)
{
    if (frame3AControl != nullptr)
    {
        Frame3ASetting_t setting =
        {
            .openId             = openId,
            .is3ASyncEnabled    = enable3ASync,
            .hwSyncMode         = (MUINT32)enableHWSync,
            .minFps             = minFps,
            .maxFps             = maxFps,
        };
        frame3AControl->set(&setting, type, appMetadata, halMetadata);
    }
    else
    {
        COMMON_LOGE("failed to setFrame3AControl, the argument frame3AControl is nullptr");
    }
}
//
MVOID
Frame3AControlHelper::
setFrameRate(OpenId openId, IMetadata* appMetadata, IMetadata* halMetadata, sp<IFrame3AControl>& frame3AControl, MBOOL isHigh)
{
    static const MINT32 highFrameRate = 0;
    static const MINT32 lowFrameRate = 15;

    if(frame3AControl != nullptr)
    {
        const MINT32 frameRate = isHigh ? highFrameRate : lowFrameRate;
        Frame3ASetting_t setting =
        {
            .openId             = openId,
            .minFps             = frameRate,
            .maxFps             = frameRate
        };
        const Fram3ASettingType type = F3A_TYPE_FRAMERATE_CHANGE;
        frame3AControl->set(&setting, type, appMetadata, halMetadata);
    }
    else
    {
        COMMON_LOGE("failed to setFrameRate, the argument frame3AControl is nullptr");
    }
}
//
MVOID
Frame3AControlHelper::
setHighFrameRate(OpenId openId, IMetadata* appMetadata, IMetadata* halMetadata, sp<IFrame3AControl>& frame3AControl)
{
    setFrameRate(openId, appMetadata, halMetadata, frame3AControl, true);
}
//
MVOID
Frame3AControlHelper::
setLowFrameRate(OpenId openId, IMetadata* appMetadata, IMetadata* halMetadata, sp<IFrame3AControl>& frame3AControl)
{
    setFrameRate(openId, appMetadata, halMetadata, frame3AControl, false);
}
//
MVOID
Frame3AControlHelper::
set3ASync(OpenId openId, IMetadata* appMetadata, IMetadata* halMetadata, sp<IFrame3AControl>& frame3AControl, MBOOL enable)
{
    if(frame3AControl != nullptr)
    {
        Frame3ASetting_t setting =
        {
            .openId             = openId,
            .is3ASyncEnabled    = enable,
            .hwSyncMode         = false
        };
        const Fram3ASettingType type = F3A_TYPE_3A_SYNC;
        frame3AControl->set(&setting, type, appMetadata, halMetadata);
    }
    else
    {
        COMMON_LOGE("failed to set3ASyc, the argument frame3AControl is nullptr");
    }
}
//
MVOID
Frame3AControlHelper::
enable3ASync(OpenId openId, IMetadata* appMetadata, IMetadata* halMetadata, sp<IFrame3AControl>& frame3AControl)
{
    set3ASync(openId, appMetadata, halMetadata, frame3AControl, true);
}
//
MVOID
Frame3AControlHelper::
disablele3ASync(OpenId openId, IMetadata* appMetadata, IMetadata* halMetadata, sp<IFrame3AControl>& frame3AControl)
{
    set3ASync(openId, appMetadata, halMetadata, frame3AControl, false);
}
MVOID
Frame3AControlHelper::
setFrameSync(OpenId openId, IMetadata* appMetadata, IMetadata* halMetadata, sp<IFrame3AControl>& frame3AControl, MBOOL enable)
{
    if(frame3AControl != nullptr)
    {
        Frame3ASetting_t setting =
        {
            .openId             = openId,
            .is3ASyncEnabled    = false,
            .hwSyncMode         = (MUINT32)enable
        };
        const Fram3ASettingType type = F3A_TYPE_FRAME_SYNC;
        frame3AControl->set(&setting, type, appMetadata, halMetadata);
    }
    else
    {
        COMMON_LOGE("failed to enableFrameSync, the argument frame3AControl is nullptr");
    }
}
//
MVOID
Frame3AControlHelper::
enableFrameSync(OpenId openId, IMetadata* appMetadata, IMetadata* halMetadata, sp<IFrame3AControl>& frame3AControl)
{
    setFrameSync(openId, appMetadata, halMetadata, frame3AControl, true);
}
//
MVOID
Frame3AControlHelper::
disableFrameSync(OpenId openId, IMetadata* appMetadata, IMetadata* halMetadata, sp<IFrame3AControl>& frame3AControl)
{
    setFrameSync(openId, appMetadata, halMetadata, frame3AControl, false);
}
//
MVOID
Frame3AControlHelper::
setMasterCamera(OpenId masterOpenId, OpenId slaveOpenId, IMetadata* appMetadata, IMetadata* halMetadata, sp<IFrame3AControl>& frame3AControl)
{
    if(frame3AControl != nullptr)
    {
        Frame3ASetting_t setting =
        {
            .openId   = masterOpenId,
            .masterId = masterOpenId,
            .slaveId  = slaveOpenId,
        };
        const Fram3ASettingType type = F3A_TYPE_MASTER_SLAVE_SET;
        frame3AControl->set(&setting, type, nullptr, halMetadata);
    }
    else
    {
        COMMON_LOGE("failed to setMasterCamera, the argument frame3AControl is nullptr");
    }
}
/******************************************************************************
 * DualZoomPolicyConverter
******************************************************************************/
DualZoomPolicyConverter::NameMap<const MTK_SYNC_CAEMRA_STATE> DualZoomPolicyConverter::statusNameMap =  {{MTK_SYNC_CAEMRA_STATE::MTK_SYNC_CAEMRA_INVALID,"invalid"},
                                                                                                        {MTK_SYNC_CAEMRA_STATE::MTK_SYNC_CAEMRA_ACTIVE, "active"},
                                                                                                        {MTK_SYNC_CAEMRA_STATE::MTK_SYNC_CAEMRA_GOTO_ACTIVE, "go_to_active"},
                                                                                                        {MTK_SYNC_CAEMRA_STATE::MTK_SYNC_CAEMRA_LOWFPS, "low_power"},
                                                                                                        {MTK_SYNC_CAEMRA_STATE::MTK_SYNC_CAEMRA_GOTO_LOWFPS, "go_to_low_fps"},
                                                                                                        {MTK_SYNC_CAEMRA_STATE::MTK_SYNC_CAEMRA_STANDBY, "standby"},
                                                                                                        {MTK_SYNC_CAEMRA_STATE::MTK_SYNC_CAEMRA_GOTO_STANDBY, "go_to_standby"},
                                                                                                        {MTK_SYNC_CAEMRA_STATE::MTK_SYNC_CAMERA_INIT, "init"}};
//
DualZoomPolicyConverter::NameMap<MINT32> DualZoomPolicyConverter::dropModeNameMap =     {{MTK_DUALZOOM_DROP_NONE,"none"},
                                                                                        {MTK_DUALZOOM_DROP_DIRECTLY, "directy"},
                                                                                        {MTK_DUALZOOM_DROP_NEED_P1, "need_p1"},
                                                                                        {MTK_DUALZOOM_DROP_NEED_SYNCMGR, "need_syncmgr"},
                                                                                        {MTK_DUALZOOM_DROP_NEED_SYNCMGR_NEED_STREAM_F_PIPE, "need_syncmgr_streat_f_pipe"}};
//
DualZoomPolicyConverter::NameMap<MINT32> DualZoomPolicyConverter::twinTGModeNameMap =   {{MTK_P1_TWIN_STATUS_NONE,"none"},
                                                                                        {MTK_P1_TWIN_STATUS_TG_MODE_1, "twin_tg_mode_1"},
                                                                                        {MTK_P1_TWIN_STATUS_TG_MODE_2, "twin_tg_mode_2"},
                                                                                        {MTK_P1_TWIN_STATUS_TG_MODE_3, "twin_tg_mode_3"}};
/******************************************************************************
 * DualZoomPolicySettings
 ******************************************************************************/
const MINT32 DualZoomPolicySettings::totalResumeAFCount         = 30;

const MUINT32 DualZoomPolicySettings::switchCameraZoomRatio     = DUALZOOM_SWICH_CAM_ZOOM_RATIO;

const MINT32 DualZoomPolicySettings::waitStableCount            = DUALZOOM_WAIT_STABLE_COUNT;
const MINT32 DualZoomPolicySettings::waitActiveToLowPowerCount  = DUALZOOM_WAIT_LOW_POWER_COUNT;
const MINT32 DualZoomPolicySettings::waitStandbyToActiveCount   = DUALZOOM_WAIT_CAM_STANDBY_TO_ACT;
const MINT32 DualZoomPolicySettings::waitLowFPSToActiveCount    = DUALZOOM_WAIT_CAM_LOWFPS_TO_ACT;
/******************************************************************************
 * OnlineFovThreshold
 ******************************************************************************/
class OnlineFovThreshold_Default
{
public:
    static const MINT32 ZoomRatio   = 1000;
    static const MINT32 Thermal     = DUALZOOM_FOV_ONLINE_TEMP_MAX;
    static const MINT32 WideDAC     = DUALZOOM_FOV_ONLINE_DAC_WIDE_MAX;
    static const MINT32 TeleDAC     = DUALZOOM_FOV_ONLINE_DAC_TELE_MAX;
    static const MINT32 Iso         = DUALZOOM_FOV_ONLINE_ISO_MAX;
    static const MINT32 ExpTime     = DUALZOOM_FOV_ONLINE_EXPTIME_MAX;
};
//
class OnlineFovThreshold_Imp
{
public:
    inline static const OnlineFovThreshold_Imp& getInstance();

public:
    OnlineFovThreshold_Imp();

    inline MINT32 getZoomRatio() const;

    inline MINT32 getThermal() const;

    inline MINT32 getWideDAC() const;

    inline MINT32 getTeleDAC() const;

    inline MINT32 getIso() const;

    inline MINT32 getExpTime() const;

private:
    MINT32 mZoomRatio;
    MINT32 mThermal;
    MINT32 mWideDAC;
    MINT32 mTeleDAC;
    MINT32 mIso;
    MINT32 mExpTime;
};
//
const OnlineFovThreshold_Imp&
OnlineFovThreshold_Imp::
getInstance()
{
    static const OnlineFovThreshold_Imp value;
    return value;
}
//
OnlineFovThreshold_Imp::
OnlineFovThreshold_Imp()
{
    mZoomRatio = property_get_int32("vendor.debug.fovonline.zoomratio", OnlineFovThreshold_Default::ZoomRatio);
    mThermal = property_get_int32("vendor.debug.fovonline.thermal", OnlineFovThreshold_Default::Thermal);
    mWideDAC = property_get_int32("vendor.debug.fovonline.widedac", OnlineFovThreshold_Default::WideDAC);
    mTeleDAC = property_get_int32("vendor.debug.fovonline.teledac", OnlineFovThreshold_Default::TeleDAC);
    mIso = property_get_int32("vendor.debug.fovonline.iso", OnlineFovThreshold_Default::Iso);
    mExpTime = property_get_int32("vendor.debug.fovonline.exptime", OnlineFovThreshold_Default::ExpTime);

    COMMON_LOGD("zoomRatio(%d), thermal(%d), wideDac(%d), teleDAC(%d), iso(%d), expTime(%d)",
        mZoomRatio, mThermal, mWideDAC, mTeleDAC, mIso, mExpTime);
}
//
MINT32
OnlineFovThreshold_Imp::
getZoomRatio() const
{
    return mZoomRatio;
}
//
MINT32
OnlineFovThreshold_Imp::
getThermal() const
{
    return mThermal;
}
//
MINT32
OnlineFovThreshold_Imp::
getWideDAC() const
{
    return mWideDAC;
}
//
MINT32
OnlineFovThreshold_Imp::
getTeleDAC() const
{
    return mTeleDAC;
}
//
MINT32
OnlineFovThreshold_Imp::
getIso() const
{
    return mIso;
}
//
MINT32
OnlineFovThreshold_Imp::
getExpTime() const
{
    return mExpTime;
}
//
MINT32
OnlineFovThreshold::
getZoomRatio()
{
    return OnlineFovThreshold_Imp::getInstance().getZoomRatio();
}
//
MINT32
OnlineFovThreshold::
getThermal()
{
    return OnlineFovThreshold_Imp::getInstance().getThermal();
}
//
MINT32
OnlineFovThreshold::
getWideDAC()
{
    return OnlineFovThreshold_Imp::getInstance().getWideDAC();
}
//
MINT32
OnlineFovThreshold::
getTeleDAC()
{
    return OnlineFovThreshold_Imp::getInstance().getTeleDAC();
}
//
MINT32
OnlineFovThreshold::
getIso()
{
    return OnlineFovThreshold_Imp::getInstance().getIso();
}
//
MINT32
OnlineFovThreshold::
getExpTime()
{
    return OnlineFovThreshold_Imp::getInstance().getExpTime();
}
//
/******************************************************************************
 * AAAJudgeSettings
 ******************************************************************************/
class AAAJudgeSettings_Default
{
public:
    static const MINT32 AFStrategy                 = 0;
    static const MINT32 AELVDifferenceThreshold    = DUALZOOM_AE_LV_DIFFERENCE;

    static const MINT32 WideAEIsoLowThreshold      = DUALZOOM_AE_ISO_LOW_THRESHOLD;
    static const MINT32 WideAEIsoHighThreshold     = DUALZOOM_AE_ISO_HIGH_THRESHOLD;
    static const MINT32 WideAEIsoCountThreshold    = 3;
    static const MINT32 WideAFDacLowThreshold      = 0;
    static const MINT32 WideAFDacHighThreshold     = 0;
    static const MINT32 WideAFDacCountThreshold    = 3;

    static const MINT32 TeleAFDacThreshold         = 0;
    static const MINT32 TeleAEIsoLowThreshold      = WideAEIsoLowThreshold;
    static const MINT32 TeleAEIsoHighThreshold     = 1000;
    static const MINT32 TeleAEIsoCountThreshold    = 3;
};
//
class AAAJudgeSettings_Imp
{
public:
    inline static const AAAJudgeSettings_Imp& getInstance();

public:
    AAAJudgeSettings_Imp();

    inline MINT32 getAFStrategy() const;

    inline MINT32 getAELVDifferenceThreshold() const;

    inline MINT32 getWideAEIsoLowThreshold() const;

    inline MINT32 getWideAEIsoHighThreshold() const;

    inline MINT32 getWideAEIsoCountThreshold() const;

    inline MINT32 getWideAFDacLowThreshold() const;

    inline MINT32 getWideAFDacHighThreshold() const;

    inline MINT32 getWideAFDacCountThreshold() const;

    inline MINT32 getTeleAFDacThreshold() const;

    inline MINT32 getTeleAEIsoLowThreshold() const;

    inline MINT32 getTeleAEIsoHighThreshold() const;

    inline MINT32 getTeleAEIsoCountThreshold() const;

private:
    MVOID getWideAFDacThreshold(MINT32& lowThreshold , MINT32& highThreshold);

private:
    MINT32 mAFStrategy;
    MINT32 mAELVDifferenceThreshold;

    MINT32 mWideAEIsoLowThreshold;
    MINT32 mWideAEIsoHighThreshold;
    MINT32 mWideAEIsoCountThreshold;
    MINT32 mWideAFDacLowThreshold;
    MINT32 mWideAFDacHighThreshold;
    MINT32 mWideAFDacCountThreshold;

    MINT32 mTeleAFDacThreshold;
    MINT32 mTeleAEIsoLowThreshold;
    MINT32 mTeleAEIsoHighThreshold;
    MINT32 mTeleAEIsoCountThreshold;
};
//
const AAAJudgeSettings_Imp&
AAAJudgeSettings_Imp::
getInstance()
{
    static const AAAJudgeSettings_Imp value;
    return value;
}
//
AAAJudgeSettings_Imp::
AAAJudgeSettings_Imp()
{
    mAFStrategy = property_get_int32("vendor.debug.dualzoom.afstrategy", AAAJudgeSettings_Default::AFStrategy);
    mAELVDifferenceThreshold = property_get_int32("vendor.debug.dualzoom.aelvdif", AAAJudgeSettings_Default::AELVDifferenceThreshold);
    COMMON_LOGD("afStrategy(%d), aeLVDifTh(%d)", mAFStrategy, mAELVDifferenceThreshold);


    mWideAEIsoLowThreshold = property_get_int32("vendor.debug.dualzoom.wide.aeisolowth", AAAJudgeSettings_Default::WideAEIsoLowThreshold);
    mWideAEIsoHighThreshold = property_get_int32("vendor.debug.dualzoom.wide.aeisohighth", AAAJudgeSettings_Default::WideAEIsoHighThreshold);
    mWideAEIsoCountThreshold = property_get_int32("vendor.debug.dualzoom.wide.aeisocountth", AAAJudgeSettings_Default::WideAEIsoCountThreshold);
    mWideAFDacLowThreshold = property_get_int32("vendor.debug.dualzoom.wide,afdaccountth", AAAJudgeSettings_Default::WideAFDacLowThreshold);
    mWideAFDacHighThreshold = property_get_int32("vendor.debug.dualzoom.wide,afdaccountth", AAAJudgeSettings_Default::WideAFDacHighThreshold);
    mWideAFDacCountThreshold = property_get_int32("vendor.debug.dualzoom.wide,afdaccountth", AAAJudgeSettings_Default::WideAFDacCountThreshold);

    #if (MTKCAM_HAVE_DUAL_ZOOM_SUPPORT == 1)
    {
        if((mWideAFDacLowThreshold < 1) || (mWideAFDacHighThreshold < 1))
        {
            getWideAFDacThreshold(mWideAFDacLowThreshold, mWideAFDacHighThreshold);
        }
    }
    #endif // MTKCAM_HAVE_DUAL_ZOOM_SUPPORT
    COMMON_LOGD("wide: aeIsoLowTh(%d), aeIsoHighTh(%d), aeIsoCountTh(%d),afDacLowTh(%d), afDacHighTh(%d),  afDacCountTh(%d)",
        mWideAEIsoLowThreshold, mWideAEIsoHighThreshold, mWideAEIsoCountThreshold, mWideAFDacLowThreshold, mWideAFDacHighThreshold, mWideAFDacCountThreshold);


    mTeleAFDacThreshold  = property_get_int32("vendor.debug.dualzoom.tele.afdacth", AAAJudgeSettings_Default::TeleAFDacThreshold);
    mTeleAEIsoLowThreshold = property_get_int32("vendor.debug.dualzoom.tele.aeisolowth", AAAJudgeSettings_Default::TeleAEIsoLowThreshold);
    mTeleAEIsoHighThreshold = property_get_int32("vendor.debug.dualzoom.tele.aeisohighth", AAAJudgeSettings_Default::TeleAEIsoHighThreshold);
    mTeleAEIsoCountThreshold = property_get_int32("vendor.debug.dualzoom.tele.aeisocountth", AAAJudgeSettings_Default::TeleAEIsoCountThreshold);

    #if (MTKCAM_HAVE_DUAL_ZOOM_SUPPORT == 1)
    {
        if (mTeleAFDacThreshold < 1)
        {
            get_Dualzoom_AF_TeleDACTh(mTeleAFDacThreshold);
        }
    }
    #endif // MTKCAM_HAVE_DUAL_ZOOM_SUPPORT
    COMMON_LOGD("tele: afDacTh(%d), aeIsoLowTh(%d), aeIsoHighTh(%d), aeIsoCountTh(%d)",
        mTeleAFDacThreshold, mTeleAEIsoLowThreshold, mTeleAEIsoHighThreshold, mTeleAEIsoCountThreshold);
}
//
MVOID
AAAJudgeSettings_Imp::
getWideAFDacThreshold(MINT32& lowThreshold , MINT32& highThreshold)
{
    int32_t devIdx[2];
    int macroPos;
    int infPos;
    int midPos;
    StereoSettingProvider::getStereoSensorDevIndex(devIdx[0], devIdx[1]);
    //  get calibration data
    CAM_CAL_DATA_STRUCT calibrationData;
    CamCalDrvBase *pCamCalDrvObj = CamCalDrvBase::createInstance();
    {
        MUINT32 queryResult;

        queryResult = pCamCalDrvObj->GetCamCalCalData(devIdx[0], CAMERA_CAM_CAL_DATA_3A_GAIN, (void *)&calibrationData);
        midPos = calibrationData.Single2A.S2aAF_t.AF_Middle_calibration;
        macroPos = calibrationData.Single2A.S2aAf[1];
        infPos = calibrationData.Single2A.S2aAf[0];
        get_Dualzoom_AF_Thresholds(infPos, macroPos, highThreshold, lowThreshold);
    }
    pCamCalDrvObj->destroyInstance();

    COMMON_LOGD("af dac low/ high : %d/ %d, macro/ inf/ mid: %d/ %d/ %d", lowThreshold, highThreshold, macroPos, infPos, midPos);
}
//
MINT32
AAAJudgeSettings_Imp::
getAFStrategy() const
{
    return mAFStrategy;
}
//
MINT32
AAAJudgeSettings_Imp::
getAELVDifferenceThreshold() const
{
    return mAELVDifferenceThreshold;
}
//
MINT32
AAAJudgeSettings_Imp::
getWideAEIsoLowThreshold() const
{
    return mWideAEIsoLowThreshold;
}
//
MINT32
AAAJudgeSettings_Imp::
getWideAEIsoHighThreshold() const
{
    return mWideAEIsoHighThreshold;
}
//
MINT32
AAAJudgeSettings_Imp::
getWideAEIsoCountThreshold() const
{
    return mWideAEIsoCountThreshold;
}
//
MINT32
AAAJudgeSettings_Imp::
getWideAFDacLowThreshold() const
{
    return mWideAFDacLowThreshold;
}
//
MINT32
AAAJudgeSettings_Imp::
getWideAFDacHighThreshold() const
{
    return mWideAFDacHighThreshold;
}
//
MINT32
AAAJudgeSettings_Imp::
getWideAFDacCountThreshold() const
{
    return mWideAFDacCountThreshold;
}
//
MINT32
AAAJudgeSettings_Imp::
getTeleAFDacThreshold() const
{
    return mTeleAFDacThreshold;
}
//
MINT32
AAAJudgeSettings_Imp::
getTeleAEIsoLowThreshold() const
{
    return mTeleAEIsoLowThreshold;
}
//
MINT32
AAAJudgeSettings_Imp::
getTeleAEIsoHighThreshold() const
{
    return mTeleAEIsoHighThreshold;
}
//
MINT32
AAAJudgeSettings_Imp::
getTeleAEIsoCountThreshold() const
{
    return mTeleAEIsoCountThreshold;
}
//
//
MINT32
AAAJudgeSettings::
getAFStrategy()
{
    return AAAJudgeSettings_Imp::getInstance().getAFStrategy();
}
//
MINT32
AAAJudgeSettings::
getAELVDifferenceThreshold()
{
    return AAAJudgeSettings_Imp::getInstance().getAELVDifferenceThreshold();
}
//
MINT32
AAAJudgeSettings::
getWideAEIsoLowThreshold()
{
    return AAAJudgeSettings_Imp::getInstance().getWideAEIsoLowThreshold();
}
//
MINT32
AAAJudgeSettings::
getWideAEIsoHighThreshold()
{
    return AAAJudgeSettings_Imp::getInstance().getWideAEIsoHighThreshold();
}
//
MINT32
AAAJudgeSettings::
getWideAEIsoCountThreshold()
{
    return AAAJudgeSettings_Imp::getInstance().getWideAEIsoCountThreshold();
}
//
MINT32
AAAJudgeSettings::
getWideAFDacLowThreshold()
{
    return AAAJudgeSettings_Imp::getInstance().getWideAFDacLowThreshold();
}
//
MINT32
AAAJudgeSettings::
getWideAFDacHighThreshold()
{
    return AAAJudgeSettings_Imp::getInstance().getWideAFDacHighThreshold();
}
//
MINT32
AAAJudgeSettings::
getWideAFDacCountThreshold()
{
    return AAAJudgeSettings_Imp::getInstance().getWideAFDacCountThreshold();
}
//
MINT32
AAAJudgeSettings::
getTeleAFDacThreshold()
{
    return AAAJudgeSettings_Imp::getInstance().getTeleAFDacThreshold();
}
//
MINT32
AAAJudgeSettings::
getTeleAEIsoLowThreshold()
{
    return AAAJudgeSettings_Imp::getInstance().getTeleAEIsoLowThreshold();
}
//
MINT32
AAAJudgeSettings::
getTeleAEIsoHighThreshold()
{
    return AAAJudgeSettings_Imp::getInstance().getTeleAEIsoHighThreshold();
}
//
MINT32
AAAJudgeSettings::
getTeleAEIsoCountThreshold()
{
    return AAAJudgeSettings_Imp::getInstance().getTeleAEIsoCountThreshold();
}
//
/******************************************************************************
 * ForceFlow
 ******************************************************************************/
class ForceFlow_Imp
{
public:
    inline static const ForceFlow_Imp& getInstance();

public:
    ForceFlow_Imp();

    inline MBOOL isSkip3ACheck() const;

    inline MBOOL isForceNotDoOnlineFov() const;

    inline MBOOL isForceNoFrameSync() const;

    inline MBOOL isForceNoDrop() const;

    inline MBOOL isForceNoCheckOnlineFovResult() const;

    inline MBOOL isForceActive() const;

private:
    MUINT32 mForceStatus;
    MBOOL   mIsSkip3ACheck;

private:
    enum
    {
        DUALCAM_FORCE_ACTIVE            = 1 << 0,
        DUALCAM_FORCE_NO_DROP           = 1 << 1,
        DUALCAM_FORCE_NO_SYNCMGR_SYNC   = 1 << 2,
        DUALCAM_FORCE_NO_FOV_ONLINE     = 1 << 3,
        DUALCAM_FORCE_NO_FOV_RULE_CHECK = 1 << 4,
    };
};
//
const ForceFlow_Imp&
ForceFlow_Imp::
getInstance()
{
    static const ForceFlow_Imp value;
    return value;
}
///
ForceFlow_Imp::
ForceFlow_Imp()
{
    mIsSkip3ACheck = property_get_int32("vendor.debug.camera.dualzoom.skip3a", 0);

    #if (MTKCAM_HAVE_DUAL_ZOOM_FUSION_SUPPORT == 1)
    {
        mForceStatus = property_get_int32("vendor.debug.dualzoom.forcestate", 1);
    }
    #else
    {
        mForceStatus = property_get_int32("vendor.debug.dualzoom.forcestate", 8);
    }
    #endif // MTKCAM_HAVE_DUAL_ZOOM_FUSION_SUPPORT
    COMMON_LOGD("isSkip3ACheck(%d), forceStatus(%d)", mIsSkip3ACheck, mForceStatus);
}
//
MBOOL
ForceFlow_Imp::
isSkip3ACheck() const
{
    return mIsSkip3ACheck;
}
//
MBOOL
ForceFlow_Imp::
isForceNotDoOnlineFov() const
{
    return (mForceStatus & DUALCAM_FORCE_NO_FOV_ONLINE);
}
//
MBOOL
ForceFlow_Imp::
isForceNoFrameSync() const
{
    return (mForceStatus & DUALCAM_FORCE_NO_SYNCMGR_SYNC);
}
//
MBOOL
ForceFlow_Imp::
isForceNoDrop() const
{
    return (mForceStatus & DUALCAM_FORCE_NO_DROP);
}
//
MBOOL
ForceFlow_Imp::
isForceNoCheckOnlineFovResult() const
{
    return (mForceStatus & DUALCAM_FORCE_NO_FOV_RULE_CHECK);
}
//
MBOOL
ForceFlow_Imp::
isForceActive() const
{
    return (mForceStatus & DUALCAM_FORCE_ACTIVE);
}
//
//
MBOOL
ForceFlow::
isSkip3ACheck()
{
    return ForceFlow_Imp::getInstance().isSkip3ACheck();
}
//
MBOOL
ForceFlow::
isForceNotDoOnlineFov()
{
    return ForceFlow_Imp::getInstance().isForceNotDoOnlineFov();
}
//
MBOOL
ForceFlow::
isForceNoFrameSync()
{
    return ForceFlow_Imp::getInstance().isForceNoFrameSync();
}
//
MBOOL
ForceFlow::
isForceNoDrop()
{
    return ForceFlow_Imp::getInstance().isForceNoDrop();
}
//
MBOOL
ForceFlow::
isForceNoCheckOnlineFovResult()
{
    return ForceFlow_Imp::getInstance().isForceNoCheckOnlineFovResult();
}
//
MBOOL
ForceFlow::
isForceActive()
{
    return ForceFlow_Imp::getInstance().isForceActive();
}
//
/******************************************************************************
*
*******************************************************************************/
} // NSCam