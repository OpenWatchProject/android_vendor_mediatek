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
*      TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/

//! \file HDRPolicyHelper.cpp


#undef LOG_TAG
#define LOG_TAG "HDRPolicyHelper"

#include <cutils/properties.h>
#include <mtkcam/utils/std/Log.h>
#include <mtkcam/utils/std/ULog.h>
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>
#include <mtkcam/utils/LogicalCam/IHalLogicalDeviceList.h>
#include <mtkcam3/feature/vhdr/HDRPolicyHelper.h>
#include <mtkcam3/pipeline/policy/types.h>


#if (MTKCAM_HAVE_AEE_FEATURE == 1)
#include <aee.h>
#endif


CAM_ULOG_DECLARE_MODULE_ID(MOD_VHDR_HAL);


#undef MY_LOGD
#undef MY_LOGD
#undef MY_LOGI
#undef MY_LOGW
#undef MY_LOGE
#undef MY_LOGA
#undef MY_TRACE_API_LIFE
#undef MY_TRACE_FUNC_LIFE
#undef MY_TRACE_TAG_LIFE
#undef MY_LOGD_IF
#undef MY_LOGI_IF
#undef MY_LOGW_IF
#undef MY_LOGE_IF


#define CC_LIKELY( exp )    (__builtin_expect( !!(exp), true ))
#define CC_UNLIKELY( exp )  (__builtin_expect( !!(exp), false ))


#define MY_LOGD(fmt, arg...)       CAM_ULOGMD("[%s]" fmt, __func__, ##arg)
#define MY_LOGI(fmt, arg...)       CAM_ULOGMI("[%s]" fmt, __func__, ##arg)
#define MY_LOGW(fmt, arg...)       CAM_ULOGMW("[%s] WRN(%5d):" fmt, __func__, __LINE__, ##arg)
#define MY_LOGE(fmt, arg...)       CAM_ULOGME("[%s] %s ERROR(%5d):" fmt, __func__,__FILE__, __LINE__, ##arg)
#define MY_TRACE_API_LIFE()        CAM_ULOGM_APILIFE()
#define MY_TRACE_FUNC_LIFE()       CAM_ULOGM_FUNCLIFE()
#define MY_TRACE_TAG_LIFE(name)    CAM_ULOGM_TAGLIFE(name)

#if (MTKCAM_HAVE_AEE_FEATURE == 1)
static const unsigned int VHDR_AEE_DB_FLAGS = DB_OPT_NE_JBT_TRACES | DB_OPT_PROCESS_COREDUMP | DB_OPT_PROC_MEM | DB_OPT_PID_SMAPS |
                                              DB_OPT_LOW_MEMORY_KILLER | DB_OPT_DUMPSYS_PROCSTATS | DB_OPT_FTRACE;

#define MY_LOGA(fmt, arg...)                                                            \
        do {                                                                            \
            android::String8 const str = android::String8::format(fmt, ##arg);          \
            MY_LOGE("ASSERT(%s)", str.c_str());                                         \
            aee_system_exception(LOG_TAG, NULL, VHDR_AEE_DB_FLAGS, str.c_str());        \
            raise(SIGKILL);                                                             \
        } while(0)
#else
#define MY_LOGA(fmt, arg...)                                                            \
        do {                                                                            \
            android::String8 const str = android::String8::format(fmt, ##arg);          \
            MY_LOGE("ASSERT(%s)", str.c_str());                                         \
        } while(0)
#endif



#define MY_LOGD_IF(cond, ...)       do { if ( (cond) )  { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) )  { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) )  { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) )  { MY_LOGE(__VA_ARGS__); } }while(0)


#define DEBUG_VHDR      (0)
#define DEBUG_APP_HDR   (-1)
#define DEBUG_DUMMY_HDR (1)


using namespace NSCam::v3::pipeline::policy;
using namespace NSCam::v3::pipeline::policy::featuresetting;
using namespace NS3Av3; //IHal3A


HDRPolicyHelper::
HDRPolicyHelper(
    int32_t sensorIdx,
    IMetadata const& staticMeta
)
{
    mSensorIdx     = sensorIdx;
    mHDRHalMode    = getHDRHalMode(sensorIdx, staticMeta);
    mHDRSensorMode = toHDRSensorMode(mHDRHalMode);
    mDebugLevel    = ::property_get_int32("vendor.debug.camera.hal3.vhdr", DEBUG_VHDR);
    mNumDummy      = ::property_get_int32("vendor.debug.camera.hal3.dummycount", DEBUG_DUMMY_HDR);// for dummyconunt debug after doing capture
    mManualUnpack  = ::property_get_int32("vendor.debug.camera.hal3.unpack", 0);
    mNeedProcraw   = ::property_get_int32("vendor.debug.camera.ProcRaw", 0);

    if( mDebugLevel ) {
        uint32_t forceAppMode = ::property_get_int32("vendor.debug.camera.hal3.appHdrMode", DEBUG_APP_HDR);
        if( forceAppMode >= static_cast<uint32_t>(HDRMode::OFF) && forceAppMode < static_cast<uint32_t>(HDRMode::NUM) ) {
            mHDRAppMode = static_cast<HDRMode>((uint8_t)forceAppMode);
            mForceAppMode = true;
            MY_LOGD("[DebugMode]Force AppHdrMode(%hhu)", mHDRAppMode);
        }
        MY_LOGD("HDRPolicyHelper(%p):sensorIdx(%d),HDRHalMode(0x%x),HDRSensorMode(0x%x)",
                this, mSensorIdx, mHDRHalMode, mHDRSensorMode);
    }

    if( ::property_get_int32("vendor.debug.mstream.nosubframe", 0) ) {
        mForceMainFrame = true;
    }

    std::shared_ptr<IHal3A> hal3a
    (
        MAKE_Hal3A(mSensorIdx, LOG_TAG),
        [](IHal3A* p){ if(p) p->destroyInstance(LOG_TAG); }
    );
    if (hal3a.get()) {
        mHal3a = hal3a;
    }
    else {
        MY_LOGE("Cannot get HAL3A instance!");
    }

}

auto
HDRPolicyHelper::
getHDRHalMode(
    int32_t sensorIdx,
    IMetadata const& staticMeta
) -> uint32_t
{
    uint32_t hdrMode = MTK_HDR_FEATURE_HDR_HAL_MODE_OFF;

    IMetadata::IEntry availMstream = staticMeta.entryFor(MTK_HDR_FEATURE_AVAILABLE_MSTREAM_HDR_MODES);
    for( size_t i = 0 ; i < availMstream.count() ; i++ )
    {
        hdrMode = availMstream.itemAt(i, Type2Type<MINT32>());
        if( hdrMode != MTK_HDR_FEATURE_HDR_HAL_MODE_OFF ) {
            return hdrMode;
        }
    }

    IMetadata::IEntry availVhdrEntry = staticMeta.entryFor(MTK_HDR_FEATURE_AVAILABLE_VHDR_MODES);
    for( size_t i = 0 ; i < availVhdrEntry.count() ; i++ ) {
        hdrMode = availVhdrEntry.itemAt(i, Type2Type<MINT32>());
        if( hdrMode != MTK_HDR_FEATURE_VHDR_MODE_OFF ) {
            auto pHalDeviceList = MAKE_HalLogicalDeviceList();
            if( CC_UNLIKELY( pHalDeviceList == NULL) ) {
                MY_LOGE("pHalDeviceList::get fail");
            }
            else {
                NSCam::SensorStaticInfo sensorStaticInfo;
                pHalDeviceList->querySensorStaticInfo(pHalDeviceList->querySensorDevIdx(sensorIdx), &sensorStaticInfo);
                if (sensorStaticInfo.HDR_Support > 0) {
                    return toHDRHalMode(hdrMode);
                }
                else {
                    MY_LOGW("Sensor capability mismatch!metadata(0x%x),HDR_Support(%d)", hdrMode, sensorStaticInfo.HDR_Support);
                }
            }
        }
    }

    return MTK_HDR_FEATURE_HDR_HAL_MODE_OFF;
}

auto
HDRPolicyHelper::
toHDRHalMode(
    uint32_t vHdrMode
) -> uint32_t
{
    switch( vHdrMode )
    {
    case MTK_HDR_FEATURE_VHDR_MODE_MVHDR:
        return MTK_HDR_FEATURE_HDR_HAL_MODE_MVHDR;
    default:
        return MTK_HDR_FEATURE_HDR_HAL_MODE_OFF;
    }
}

auto
HDRPolicyHelper::
toHDRSensorMode(
    uint32_t hdrHalMode
) -> uint32_t
{
    switch( hdrHalMode )
    {
    case MTK_HDR_FEATURE_HDR_HAL_MODE_MVHDR:
        return SENSOR_VHDR_MODE_MVHDR;
    default:
        return SENSOR_VHDR_MODE_NONE;
    }
}

auto
HDRPolicyHelper::
getHDRAppMode(
) -> HDRMode
{
    return mHDRAppMode;
}

auto
HDRPolicyHelper::
getHDRHalMode(
) -> uint32_t
{
    return mHDRHalMode;
}

auto
HDRPolicyHelper::
getHDRHalRequestMode(
    HDR_REQUEST_TYPE type
) -> uint32_t
{
    if( isHDR() ) {
        if( getHDRHalMode() & MTK_HDR_FEATURE_HDR_HAL_MODE_MSTREAM_CAPTURE_PREVIEW ) {
            switch( type ) {
            case HDR_REQUEST_CAPTURE:
                return MTK_HDR_FEATURE_HDR_HAL_MODE_MSTREAM_CAPTURE;
            case HDR_REQUEST_PREVIEW:
                return MTK_HDR_FEATURE_HDR_HAL_MODE_MSTREAM_PREVIEW;
            case HDR_REQUEST_PREVIEW_CAPTURE:
                return MTK_HDR_FEATURE_HDR_HAL_MODE_MSTREAM_CAPTURE_PREVIEW;
            }
        }
        return getHDRHalMode();
    }
    return MTK_HDR_FEATURE_HDR_HAL_MODE_OFF;
}

auto
HDRPolicyHelper::
getHDRSensorMode(
) -> uint32_t
{
    return mHDRSensorMode;
}

auto
HDRPolicyHelper::
getHDRExposure(
    FRAME_TYPE type
) -> uint32_t
{
    if( isHDR() && ( getHDRHalMode() & MTK_HDR_FEATURE_HDR_HAL_MODE_MSTREAM_CAPTURE_PREVIEW )  ) {
        switch( type ) {
        case FRAME_TYPE_MAIN:
            return MTK_3A_FEATURE_AE_EXPOSURE_LEVEL_NORMAL;
        case FRAME_TYPE_PRESUB:
            return MTK_3A_FEATURE_AE_EXPOSURE_LEVEL_SHORT;
        case FRAME_TYPE_SUB:
        case FRAME_TYPE_PREDUMMY:
        case FRAME_TYPE_POSTDUMMY:
        case FRAME_TYPE_UNKNOWN:
        default:
            return MTK_3A_FEATURE_AE_EXPOSURE_LEVEL_NONE;
        }
    }
    return MTK_3A_FEATURE_AE_EXPOSURE_LEVEL_NONE;
}

auto
HDRPolicyHelper::
getDebugLevel(
) -> uint32_t
{
    return mDebugLevel;
}

auto
HDRPolicyHelper::
getGroupSize(
) -> size_t
{
    if( needPreSubFrame() && (mHDRHalMode & MTK_HDR_FEATURE_HDR_HAL_MODE_MSTREAM_CAPTURE_PREVIEW) ) {
        return MSTREAM_GROUP_SIZE;
    }
    return 1;
}

auto
HDRPolicyHelper::
getDummySize(
) -> size_t
{
    return mNumDummy;
}


auto
HDRPolicyHelper::
getDebugMessage(
) -> android::String8
{
    return android::String8::format("HDRPolicyHelper(%p)=SensorIdx(%d),HDRAppMode(%hhu),HDRHalMode(0x%x),HDRSensorMode(0x%x),"
                                    "isAppHDR(%d),isAppVideoHDR(%d),isHalHDR(%d),isHDR(%d),isZSLHDR(%d),isMulitFrameHDR(%d),"
                                    "needReconfiguration(%d)",
                                    this, mSensorIdx, getHDRAppMode(), getHDRHalMode(), getHDRSensorMode(),
                                    isAppHDR(), isAppVideoHDR(), isHalHDR(), isHDR(), isZSLHDR(), isMulitFrameHDR(),
                                    needReconfiguration());
}

auto
HDRPolicyHelper::
isHDR(
) -> bool
{
    return ( isAppHDR() && isHalHDR() && isZSLHDR() ) || ( isAppVideoHDR() && isHalHDR() );
}

auto
HDRPolicyHelper::
isZSLHDR(
) -> bool
{
    return ( mHDRHalMode & MTK_HDR_FEATURE_HDR_HAL_MODE_MSTREAM_CAPTURE );
}

auto
HDRPolicyHelper::
isMulitFrameHDR(
) -> bool
{
    return ( mHDRHalMode & MTK_HDR_FEATURE_HDR_HAL_MODE_MSTREAM_CAPTURE_PREVIEW );
}

auto
HDRPolicyHelper::
needReconfiguration(
) -> bool
{
    return mNeedReconfigure;
}

auto
HDRPolicyHelper::
needUnpack(
) -> bool
{
    return isHDR() && (mHDRHalMode & MTK_HDR_FEATURE_HDR_HAL_MODE_MSTREAM_CAPTURE_PREVIEW) && mNeedProcraw;
}

auto
HDRPolicyHelper::
needDummy(
) -> bool
{
    return ( isHDR() && !isZSLHDR() && ( mAddDummy && mNumDummy >= 1) );
}

auto
HDRPolicyHelper::
needPreSubFrame(
) -> bool
{
    return isHDR() && isMulitFrameHDR() && !mForceMainFrame;
}

auto
HDRPolicyHelper::
negotiateRequestPolicy(
    AdditionalFrameInfo& additionalFrame
) -> bool
{
    if( needPreSubFrame() ) {
        for (size_t i = 0; i < (getGroupSize() - 1) ; i++) {
        // TODO: No needRRZO/LCSO in flowA
            uint32_t p1Dma = P1_IMGO;

            std::shared_ptr<NSCam::IMetadata> additionalHal = std::make_shared<IMetadata>();

            int ret = 0;
            if( mNeedProcraw == 1 ) {
                // set processed raw
                ret |= IMetadata::setEntry<MINT32>(additionalHal.get(), MTK_P1NODE_RAW_TYPE, 0);
            }
            else {
                // set pure raw
                ret |= IMetadata::setEntry<MINT32>(additionalHal.get(), MTK_P1NODE_RAW_TYPE, 1);
            }
            // bypass LCE
            ret |= IMetadata::setEntry<MINT32>(additionalHal.get(), MTK_3A_ISP_BYPASS_LCE , 1);

            ret |= IMetadata::setEntry<MINT32>(additionalHal.get(), MTK_3A_FEATURE_AE_EXPOSURE_LEVEL,
                                               getHDRExposure(FRAME_TYPE_PRESUB));

            if( ret != 0 ) {
                MY_LOGE("set fail");
                return false;
            }

            additionalFrame.addOneFrame(FRAME_TYPE_PRESUB, p1Dma, nullptr, additionalHal);
        }
    }

    if( needDummy() ) {
        for( size_t i = 0; i < mNumDummy; i++ ) {
            additionalFrame.addOneFrame(FRAME_TYPE_PREDUMMY, 0, nullptr, nullptr);
        }
        mAddDummy = false;
    }

    MY_LOGD_IF(getDebugLevel(), "preSubFrame(%zu),preDummy(%zu),subFrame(%zu),postDummy(%zu)",
        additionalFrame.getAdditionalFrameSet().preSubFrame.size(), additionalFrame.getAdditionalFrameSet().preDummy.size(),
        additionalFrame.getAdditionalFrameSet().subFrame.size(), additionalFrame.getAdditionalFrameSet().postDummy.size());

    if( getDebugLevel() >= 3 ) {
        for( auto &preSubFrame : additionalFrame.getAdditionalFrameSet().preSubFrame ) {
            preSubFrame.appMetadata->dump();
            preSubFrame.halMetadata->dump();
        }
        for( auto &preDummy : additionalFrame.getAdditionalFrameSet().preDummy ) {
            preDummy.appMetadata->dump();
            preDummy.halMetadata->dump();
        }
        for( auto &subFrame : additionalFrame.getAdditionalFrameSet().subFrame ) {
            subFrame.appMetadata->dump();
            subFrame.halMetadata->dump();
        }
        for( auto &postDummy : additionalFrame.getAdditionalFrameSet().postDummy ) {
            postDummy.appMetadata->dump();
            postDummy.halMetadata->dump();
        }
    }

    return true;
}


auto
HDRPolicyHelper::
notifyDummy(
) -> bool
{
    if( isHDR() && !isZSLHDR() && (mNumDummy >= 1) ) {
        mAddDummy = true;
        MY_LOGD("Dummy Notified:isAppHDR(%d),isHalHDR(%d),isZSLHDR(%d),AddDummy(%d),NumDummy(%zu)",
                isAppHDR(), isHalHDR(), isZSLHDR(), mAddDummy, mNumDummy);
    }
    return true;
}

auto
HDRPolicyHelper::
handleReconfiguration(
) -> bool
{
    mNeedReconfigure = false;
    MY_LOGD("HDR trigger Reconfiguration");
    return true;
}

auto
HDRPolicyHelper::
isAppHDR(
) -> bool
{
    return ( mHDRAppMode == HDRMode::ON || isAppVideoHDR() );
}

auto
HDRPolicyHelper::
isAppVideoHDR(
) -> bool
{
    return ( mHDRAppMode == HDRMode::VIDEO_ON || mHDRAppMode == HDRMode::VIDEO_AUTO );
}

auto
HDRPolicyHelper::
isHalHDR(
) -> bool
{
    return ( mHDRHalMode != MTK_HDR_FEATURE_HDR_HAL_MODE_OFF );
}

auto
HDRPolicyHelper::
updateAppConfigMode(
    HDRMode appHdrMode
) -> bool
{
    if( !mConfigured ) {
        updateAppHDRMode(appHdrMode);
        MY_LOGD_IF(getDebugLevel(), "AppConfigMode(%hhu)", appHdrMode);
        mConfigured = true;
    }
    return true;
}

auto
HDRPolicyHelper::
updateAppRequestMode(
    HDRMode appHdrMode,
    uint32_t appMode
) -> bool
{
    return updateAppHDRMode(strategyAppHDRMode(appHdrMode, appMode));
}

auto
HDRPolicyHelper::
updateAppHDRMode(
    HDRMode appHdrMode
) -> bool
{
    if( !mForceAppMode ) {
        if( mHDRAppMode != appHdrMode ) {
            bool isSwitchOff = isHDR();

            MY_LOGD("App HDR mode Changed: (%hhu) => (%hhu). mConfigured(%d)", mHDRAppMode, appHdrMode, mConfigured);
            mHDRAppMode = appHdrMode;

            bool isSwitchOn = isHDR();
            if( mConfigured && (isSwitchOff || isSwitchOn) ) {
                mNeedReconfigure = true;
            }
        }
    }
    return true;
}

auto
HDRPolicyHelper::
strategyAppHDRMode(
    HDRMode inAppHdrMode,
    uint32_t appMode
) -> HDRMode
{
    if( isHDR() ) {
        // Add more strategy and combine here for future
        if( mHDRHalMode == MTK_HDR_FEATURE_HDR_HAL_MODE_MVHDR ) {
            return evaluateISOStrategy(inAppHdrMode, appMode);
        }
    }
    return inAppHdrMode;
}

auto
HDRPolicyHelper::
evaluateISOStrategy(
    HDRMode inAppHdrMode,
    uint32_t appMode
) -> HDRMode
{
    HDRMode outAppHdrMode = inAppHdrMode;
    SwitchModeStatus outIsoStatus = mIsoSwitchModeStatus;
    int isoStatus_Low  = ISO_UNKNOWN;
    int isoStatus_High = ISO_UNKNOWN;

    if( appMode == MTK_FEATUREPIPE_VIDEO_RECORD || appMode == MTK_FEATUREPIPE_VIDEO_STOP ) {
        // Low light (high iso)
        // strategy = HDR OFF
        if( mIsoSwitchModeStatus == eSwitchMode_LowLightMode ) {
            outAppHdrMode = HDRMode::OFF;
        }
    }
    else {
        auto hal3a = mHal3a;
        if( hal3a.get() ) {
            {
                std::lock_guard<std::mutex> _l(mHal3aLocker);
                hal3a->send3ACtrl(E3ACtrl_GetISOThresStatus, (MINTPTR)&isoStatus_Low, (MINTPTR)&isoStatus_High);
            }

            if( mIsoSwitchModeStatus == eSwitchMode_HighLightMode ) {
                // High light (low iso) -> Low light (high iso) w/ stable
                // strategy = HDR OFF
                if( isoStatus_High == ISO_LARGER_AND_STABLE ) {
                    outIsoStatus = eSwitchMode_LowLightMode;
                    outAppHdrMode = HDRMode::OFF;
                }
            }
            else if( mIsoSwitchModeStatus == eSwitchMode_LowLightMode ) {
                // Low light (high iso)
                // strategy = HDR OFF
                outAppHdrMode = HDRMode::OFF;

                // Low light (high iso) -> High light (low iso) w/ stable
                // strategy = HDR ON
                if( isoStatus_Low == ISO_SMALLER_AND_STABLE ) {
                    outIsoStatus = eSwitchMode_HighLightMode;
                    //outAppHdrMode = HDRMode::VIDEO_ON;
                    outAppHdrMode = inAppHdrMode;
                }
            }
        }
        else {
            MY_LOGE("create IHal3A instance failed! cannot get current real iso for strategy");
        }
    }

    MY_LOGD_IF((mIsoSwitchModeStatus != outIsoStatus) || getDebugLevel(),
                "[needReconfig(%d):status(%d=>%d)]App(%d),AppHdrMode(%hhu=>%hhu),iso(%d,%d)",
                (mIsoSwitchModeStatus != outIsoStatus), mIsoSwitchModeStatus, outIsoStatus,
                appMode, inAppHdrMode, outAppHdrMode, isoStatus_Low, isoStatus_High);

    mIsoSwitchModeStatus = outIsoStatus;

    return outAppHdrMode;
}


auto
AdditionalFrameInfo::
addOneFrame(
    FRAME_TYPE frameType,
    uint32_t p1Dma,
    std::shared_ptr<NSCam::IMetadata> additionalApp,
    std::shared_ptr<NSCam::IMetadata> additionalHal
) -> bool
{
    std::shared_ptr<NSCam::IMetadata>  outMetaApp = std::make_shared<IMetadata>();
    std::shared_ptr<NSCam::IMetadata>  outMetaHal = std::make_shared<IMetadata>();
    if (additionalApp.get() != nullptr) {
        *outMetaApp += *additionalApp;
    }
    if (additionalHal.get() != nullptr) {
        *outMetaHal += *additionalHal;
    }

    FrameInfo newFrame {
        .p1Dma = p1Dma,
        .appMetadata = outMetaApp,
        .halMetadata = outMetaHal,
    };

    switch (frameType) {
    case FRAME_TYPE_PRESUB:
        mAdditionalFrameSet.preSubFrame.push_back(newFrame);
        break;
    case FRAME_TYPE_PREDUMMY:
        mAdditionalFrameSet.preDummy.push_back(newFrame);
        break;
    case FRAME_TYPE_SUB:
        mAdditionalFrameSet.subFrame.push_back(newFrame);
        break;
    case FRAME_TYPE_POSTDUMMY:
        mAdditionalFrameSet.postDummy.push_back(newFrame);
        break;
    case FRAME_TYPE_UNKNOWN:
    case FRAME_TYPE_MAIN:
    default:
        MY_LOGW("No push invalid frametype(%d)", frameType);
        return false;
    }

    return true;
}

auto
AdditionalFrameInfo::
getAdditionalFrameSet(
) -> AdditionalFrameSet
{
    return mAdditionalFrameSet;
}

