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
 * MediaTek Inc. (C) 2018. All rights reserved.
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
#define LOG_TAG "AINRPlugin"
//
#include <mtkcam/utils/std/Log.h>
//
#include <atomic>
#include <cfenv>
#include <cstdlib>
#include <sstream>

#include <utils/Errors.h>
#include <utils/List.h>
#include <utils/RefBase.h>
#include <utils/String8.h>
//
#include <mtkcam/utils/metadata/client/mtk_metadata_tag.h>
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>
//
//
#include <mtkcam/utils/imgbuf/IIonImageBufferHeap.h>
//
#include <mtkcam/drv/IHalSensor.h>
#include <mtkcam/utils/std/Format.h>
#include <mtkcam/utils/sys/MemoryInfo.h>
//
#include <mtkcam3/pipeline/hwnode/NodeId.h>

#include <mtkcam/utils/metastore/IMetadataProvider.h>
#include <mtkcam/utils/metastore/ITemplateRequest.h>
#include <mtkcam3/3rdparty/plugin/PipelinePlugin.h>
#include <mtkcam3/3rdparty/plugin/PipelinePluginType.h>
// MTKCAM
#include <mtkcam/aaa/IHal3A.h> // setIsp, CaptureParam_T
//
#include <mtkcam/utils/TuningUtils/FileDumpNamingRule.h> // tuning file naming
#include <mtkcam/utils/std/JobQueue.h>
//
#include <isp_tuning/isp_tuning.h>
//
#include <sys/stat.h> // mkdir
#include <sys/prctl.h> //prctl set name
#include <mtkcam/drv/iopipe/SImager/ISImager.h>
#include <cutils/properties.h>
//
#include <custom/feature/mfnr/camera_custom_mfll.h> //CUST_MFLL_AUTO_MODE
#include "AinrCtrler.h"
// Ulog
#include <mtkcam3/feature/ainr/AinrUlog.h>
// AINR isp Module
#include <mtkcam3/feature/ainr/IAinrFeFm.h>
//
using namespace NSCam;
using namespace plugin;
using namespace android;
using namespace std;
using namespace NSCam::NSPipelinePlugin;
using namespace NS3Av3;
using namespace NSCam::TuningUtils;
using namespace NSIoPipe;
using namespace ainr;

// shot count
static std::atomic<int> gShotCount = 0;

template<class T>
static inline void _releasePluginBuffer(T var) {
    if (var) var->release();
}

/******************************************************************************
*
******************************************************************************/
class AINRProcImpl : public MultiFramePlugin::IProvider
{
    typedef MultiFramePlugin::Property Property;
    typedef MultiFramePlugin::Selection Selection;
    typedef MultiFramePlugin::Request::Ptr RequestPtr;
    typedef MultiFramePlugin::RequestCallback::Ptr RequestCallbackPtr;

public:

    virtual const Property& property()
    {
        CAM_ULOGM_APILIFE();
        static Property prop;
        static bool inited;

        if (!inited) {
            MUINT8 thumbTiming = eTiming_Provider_01;
            int64_t memRestriction = NSCam::NSMemoryInfo::getSystemTotalMemorySize();
            std::fesetround(FE_TONEAREST);
            if (std::nearbyint(memRestriction/1024/1024/1024) < RESTRICT_MEM_AMOUNT)
            {
                thumbTiming = eTiming_MDP;
                ainrLogD("System Memory is (%" PRId64 ") <= 4.3G Mb, NOT ALLOW to Use AINR fast shot 2 shot "
                        "nearbyint Memory is (%.1f)", memRestriction,
                        nearbyint(memRestriction/1024/1024/1024));
            }

            prop.mName              = "MTK AINR";
            prop.mFeatures          = MTK_FEATURE_AINR;
            prop.mThumbnailTiming   = thumbTiming;
            prop.mPriority          = ePriority_Highest;
            prop.mZsdBufferMaxNum   = 8;  // maximum frames requirement
            prop.mNeedRrzoBuffer    = MTRUE;  // rrzo requirement for BSS
            inited                  = MTRUE;
        }

        return prop;
    };

    //if capture number is 4, "negotiate" would be called 4 times
    virtual MERROR negotiate(Selection& sel)
    {
        CAM_ULOGM_APILIFE();

        mEnable = ::property_get_int32("vendor.debug.camera.ainr.enable", -1);
        if(CC_UNLIKELY(mEnable == 0)) {
            ainrLogD("Force off AINR");
            return BAD_VALUE;
        }

        auto flashOn = sel.mState.mFlashFired;
        if (CC_UNLIKELY(flashOn == MTRUE)) {
            ainrLogD("not support AINR due to Flash on(%d)", flashOn);
            return BAD_VALUE;
        }

        if (CC_UNLIKELY(sel.mIMetadataApp.getControl() == nullptr)) {
            ainrLogF("AppMeta get control fail!!");
            return BAD_VALUE;
        }

        switch (sel.mIspHidlStage) {
            case ISP_HIDL_STAGE_DISABLED:
                return negotiateHal(sel);
            break;
            case ISP_HIDL_STAGE_REUEST_FRAME_FROM_CAMERA:
                return negotiateIspHidlCamera(sel);
            break;
            case ISP_HIDL_STAGE_PROCESS_FRAME_IN_ISP_HIDL:
                return negotiateIspHidlHal(sel);
            break;
            default:
                ainrLogF("cannot recongize mIspHidlStage");
            break;

        }

        return BAD_VALUE;
    };

    MERROR negotiateHal(Selection& sel)
    {
        CAM_ULOGM_APILIFE();

        auto spAppMeta = sel.mIMetadataApp.getControl();
        IMetadata* appInMeta = spAppMeta.get();
        if (sel.mRequestIndex == 0) {
            // Query 3A information from caller
            mZSDMode = sel.mState.mZslRequest && sel.mState.mZslPoolReady;
            mFlashOn = sel.mState.mFlashFired;
            mRealIso = sel.mState.mRealIso;
            mShutterTime = sel.mState.mExposureTime;

            // In HAL level, HAL takes the responsibility for AI shotCount
            if(gShotCount > 1) {
                ainrLogW("No need to do AINR. ShotCount(%d)"
                        , gShotCount.load(std::memory_order_relaxed));
                return BAD_VALUE;
            }

            // Update mfbMode
            mMfbMode = updateMfbMode(appInMeta, mZSDMode);
            if (CC_UNLIKELY(mMfbMode == AinrMode_Off)) {
                return BAD_VALUE;
            }

            // Create AinrCtrler in first frame
            setUniqueKey(NSCam::Utils::TimeTool::getReadableTime());
            MINT32 sensorId = sel.mState.mSensorId;
            createCtrlerInfo(sensorId);

            sel.mDecision.mZslEnabled = isZsdMode(mMfbMode);
            if(sel.mDecision.mZslEnabled) {
                ainrLogD("Ainr zsl mode on");
            } else {
                ainrLogD("Ainr zsl mode off");
            }

            sel.mDecision.mZslPolicy.mPolicy
                    = v3::pipeline::policy::eZslPolicy_AfState
                    | v3::pipeline::policy::eZslPolicy_ContinuousFrame
                    | v3::pipeline::policy::eZslPolicy_ZeroShutterDelay;
            sel.mDecision.mZslPolicy.mTimeouts = 1000;
        }

        // Get AinrCtrler
        std::shared_ptr<AinrCtrler> ctrler = getCtrlerInfo(getUniqueKey());
        if (CC_UNLIKELY(ctrler.get() == nullptr)) {
            removeCtrlerInfo(getUniqueKey());
            ainrLogF("get AinrCtrler instance failed! cannot apply AINR.");
            return BAD_VALUE;
        }

        bool enableAinr = false;
        // Update ainr policy
        if (sel.mRequestIndex == 0) {
            // Record core version/manual AE information if needed
            updateCaptureInfo(ctrler.get());

            ctrler->setSizeImgo(sel.mState.mSensorSize);
            ainrLogD("set source size = %dx%d", ctrler->getSizeImgo().w, ctrler->getSizeImgo().h);

            // Update strategy
            auto &sensorSize = sel.mState.mSensorSize;
            ctrler->updateAinrStrategy(sensorSize);

            ainrLogD("realIso = %d, shutterTime = %d, finalRealIso = %d, finalShutterTime = %d"
                    , ctrler->getRealIso()
                    , ctrler->getShutterTime()
                    , ctrler->getFinalIso()
                    , ctrler->getFinalShutterTime());
        }

        // Set up requestCount
        sel.mRequestCount = ctrler->getCaptureNum();

        if((mEnable == 1) || ctrler->getIsEnableAinr()) {
            ainrLogD("Enable ainr mEnable(%d), ctrlerEnable(%d)"
                , mEnable, ctrler->getIsEnableAinr());
            enableAinr = true;
            if (mEnable == 1) {
                constexpr int frame_num = 6;
                sel.mRequestCount = frame_num;
                ctrler->setCaptureNum(frame_num);
            }
        }

        if (!enableAinr
            || sel.mRequestCount == 0
            || mEnable == 0) {
            ainrLogD("CtrEnalbe(%d), requestCount(%d), enable(%d)"
                , ctrler->getIsEnableAinr()
                , sel.mRequestCount
                , mEnable);
            removeCtrlerInfo(getUniqueKey());
            ainrLogD_IF(sel.mRequestCount == 0, "Suspect NVRAM data is EMPTY!");
            return BAD_VALUE;
        }

        bool mainFrame = (sel.mRequestIndex == 0) ? true : false;
        if (mainFrame) {
            gShotCount++;
        }

        // Config selection buffers
        configSelBuffer(sel, mainFrame);

        // Set up BSS
        if (sel.mIMetadataApp.getControl() != nullptr) {
            MetadataPtr pAppAddtional = make_shared<IMetadata>();
            MetadataPtr pHalAddtional = make_shared<IMetadata>();

            IMetadata* pAppMeta = pAppAddtional.get();
            IMetadata* pHalMeta = pHalAddtional.get();

            IMetadata::setEntry<MINT32>(pHalMeta,
                    MTK_FEATURE_BSS_SELECTED_FRAME_COUNT, sel.mRequestCount);
            IMetadata::setEntry<MINT32>(pHalMeta,
                    MTK_FEATURE_BSS_FORCE_DROP_NUM, 0);
            IMetadata::setEntry<MINT32>(pHalMeta,
                    MTK_FEATURE_BSS_PROCESS, MTK_FEATURE_BSS_PROCESS_ENABLE);
            IMetadata::setEntry<MBOOL>(pHalMeta,
                    MTK_FEATURE_BSS_REORDER, MTRUE);
            IMetadata::setEntry<MINT32>(pHalMeta,
                    MTK_PIPELINE_UNIQUE_KEY, getUniqueKey());

            // AE shutter/iso decision if needed
            if (!mZSDMode) {
                update3AMetadata(mManualShutter, mManualIso, pAppMeta, pHalMeta);
            }

            // Debug exif requirement
            IMetadata::setEntry<MUINT8>(pHalMeta, MTK_HAL_REQUEST_REQUIRE_EXIF, 1);
            IMetadata::setEntry<MUINT8>(pHalMeta, MTK_HAL_REQUEST_DUMP_EXIF, 1);

            sel.mIMetadataApp.setAddtional(pAppAddtional);
            sel.mIMetadataHal.setAddtional(pHalAddtional);
        }

        //dummy frame
        if (sel.mRequestIndex + 1 == sel.mRequestCount) {
            MetadataPtr pAppDummy = make_shared<IMetadata>();
            MetadataPtr pHalDummy = make_shared<IMetadata>();

            IMetadata* pAppMeta = pAppDummy.get();
            IMetadata* pHalMeta = pHalDummy.get();
            IMetadata::setEntry<MBOOL>(pHalMeta,
                    MTK_3A_DUMMY_AFTER_REQUEST_FRAME, 1);

            sel.mPostDummy = ctrler->getDelayFrameNum();
            sel.mIMetadataApp.setDummy(pAppDummy);
            sel.mIMetadataHal.setDummy(pHalDummy);
        }

        return OK;
    };

    MERROR negotiateIspHidlCamera(Selection& sel)
    {
        CAM_ULOGM_APILIFE();

        auto spAppMeta = sel.mIMetadataApp.getControl();
        IMetadata* appInMeta = spAppMeta.get();

        int frameCount = 1, frameIndex = -1;
        if(!IMetadata::getEntry<MINT32>(appInMeta,
                MTK_CONTROL_CAPTURE_HINT_FOR_ISP_FRAME_INDEX, frameIndex)) {
            ainrLogI("cannot get ISP_FRAME_INDEX");
            return BAD_VALUE;
        }
        if(!IMetadata::getEntry<MINT32>(appInMeta,
                MTK_CONTROL_CAPTURE_HINT_FOR_ISP_FRAME_COUNT, frameCount)) {
            ainrLogI("cannot get ISP_FRAME_COUNT");
            return BAD_VALUE;
        }
        ainrLogD("frameIndex(%d), frameCount(%d)", frameIndex, frameCount);

        // Check tuning hint
        MINT32 tuningType  = -1;
        IMetadata::getEntry<MINT32>(appInMeta,
                MTK_CONTROL_CAPTURE_HINT_FOR_ISP_TUNING, tuningType);
        if (tuningType != MTK_CONTROL_CAPTURE_HINT_FOR_ISP_TUNING_AINR) {
            ainrLogI("No need to do AINR, tuningtype(%d)", tuningType);
            return BAD_VALUE;
        }

        if (frameIndex == 0) {
            // Query 3A information from caller
            mZSDMode = sel.mState.mZslRequest && sel.mState.mZslPoolReady;

            // Check tuning information and AIS status
            MINT32 aisMode  = MTK_MFNR_FEATURE_AIS_OFF;
            if (!IMetadata::getEntry<MINT32>(appInMeta,
                    MTK_MFNR_FEATURE_AIS_MODE, aisMode)) {
                ainrLogI("There is no MTK_MFNR_FEATURE_AIS_MODE");
            }
            ainrLogD("AIS mode(%d)", aisMode);

            if (aisMode == MTK_MFNR_FEATURE_AIS_ON) {
                ainrLogD("AINR AIS mode on");
                mMfbMode = AinrMode_NormalAis;
            } else {
                mMfbMode = mZSDMode ? AinrMode_ZsdAinr : AinrMode_NormalAinr;
            }

            mFlashOn = sel.mState.mFlashFired;
            mRealIso = sel.mState.mRealIso;
            mShutterTime = sel.mState.mExposureTime;

            // Set uniqueKey in first frame
            setUniqueKey(NSCam::Utils::TimeTool::getReadableTime());

            // Create AinrCtrler
            MINT32 sensorId = sel.mState.mSensorId;
            createCtrlerInfo(sensorId);

            std::shared_ptr<AinrCtrler> ctrler = getCtrlerInfo(getUniqueKey());
            if (CC_UNLIKELY(ctrler.get() == nullptr)) {
                removeCtrlerInfo(getUniqueKey());
                ainrLogF("get AinrCtrler instance failed! cannot apply AINR.");
                return BAD_VALUE;
            }

            // Record core version/manual AE information if needed
            updateCaptureInfo(ctrler.get());

            // Finish we remove controller
            removeCtrlerInfo(getUniqueKey());
        }

        // Per-frame update zsl information
        sel.mDecision.mZslEnabled = isZsdMode(mMfbMode);
        if(sel.mDecision.mZslEnabled) {
            ainrLogD("Ainr zsl mode on");
        } else {
            ainrLogD("Ainr zsl mode off");
        }

        sel.mDecision.mZslPolicy.mPolicy
                = v3::pipeline::policy::eZslPolicy_AfState
                | v3::pipeline::policy::eZslPolicy_ContinuousFrame
                | v3::pipeline::policy::eZslPolicy_ZeroShutterDelay;
        sel.mDecision.mZslPolicy.mTimeouts = 1000;

        // In HidlCamera stage no need to do plugin process
        sel.mDecision.mProcess = MFALSE;

        // In ISPHIDL case frame count should be 1
        sel.mRequestCount = 1;

        // Config selection buffers
        bool mainFrame = (frameIndex == 0) ? true : false;
        configSelBuffer(sel, mainFrame);

        // Set up metadata
        if (sel.mIMetadataApp.getControl() != nullptr) {
            MetadataPtr pAppAddtional = make_shared<IMetadata>();
            MetadataPtr pHalAddtional = make_shared<IMetadata>();

            IMetadata* pAppMeta = pAppAddtional.get();
            IMetadata* pHalMeta = pHalAddtional.get();

            // PackNode may recongizes this BSS metadata to package buffers
            IMetadata::setEntry<MINT32>(pHalMeta,
                    MTK_FEATURE_BSS_SELECTED_FRAME_COUNT, frameCount);
            IMetadata::setEntry<MINT32>(pHalMeta,
                    MTK_FEATURE_BSS_FORCE_DROP_NUM, 0);
            IMetadata::setEntry<MINT32>(pHalMeta,
                    MTK_FEATURE_BSS_PROCESS, MTK_FEATURE_BSS_PROCESS_ENABLE);
            IMetadata::setEntry<MBOOL>(pHalMeta,
                    MTK_FEATURE_BSS_REORDER, MTRUE);
            IMetadata::setEntry<MUINT8>(pHalMeta, MTK_FEATURE_PACK_RRZO, 1);

            // Reszie image or feature extraction histogram
            if(!mUsingRrz) {
                IMetadata::setEntry<MBOOL>(pHalMeta,
                        MTK_FEATURE_BSS_DOWNSAMPLE, MTRUE);
                // We need feo
                IMetadata::setEntry<MINT32>(pHalMeta,
                        MTK_ISP_FEO_ENABLE, 1);
            }

            if (!mZSDMode) {
                update3AMetadata(mManualShutter, mManualIso, pAppMeta, pHalMeta);
            }

            // Debug exif requirement
            IMetadata::setEntry<MUINT8>(pHalMeta, MTK_HAL_REQUEST_REQUIRE_EXIF, 1);
            IMetadata::setEntry<MUINT8>(pHalMeta, MTK_HAL_REQUEST_DUMP_EXIF, 1);

            sel.mIMetadataApp.setAddtional(pAppAddtional);
            sel.mIMetadataHal.setAddtional(pHalAddtional);
        }

        return OK;
    };

    MERROR negotiateIspHidlHal(Selection& sel)
    {
        CAM_ULOGM_APILIFE();

        int frameCount = sel.mStateIspHidl.frameCount;
        int frameIndex = sel.mStateIspHidl.frameIndex;
        ainrLogD("frameIndex(%d), frameCount(%d)", frameIndex, frameCount);

        // Get appMetadata from featurePolicy
        auto spAppMeta = sel.mIMetadataApp.getControl();
        IMetadata* appInMeta = spAppMeta.get();

        if (frameIndex == 0) {
            // Query 3A information from caller
            mZSDMode = sel.mState.mZslRequest && sel.mState.mZslPoolReady;
            mFlashOn = sel.mState.mFlashFired;
            mRealIso = sel.mState.mRealIso;
            mShutterTime = sel.mState.mExposureTime;

            // Set uniqueKey in first frame
            setUniqueKey(NSCam::Utils::TimeTool::getReadableTime());
            {
                MINT32 tuningType  = -1;
                IMetadata::getEntry<MINT32>(appInMeta,
                        MTK_CONTROL_CAPTURE_HINT_FOR_ISP_TUNING, tuningType);
                if (tuningType != MTK_CONTROL_CAPTURE_HINT_FOR_ISP_TUNING_AINR) {
                    ainrLogD("No need to do AINR, tuningtype(%d)", tuningType);
                    return BAD_VALUE;
                }
                mMfbMode = AinrMode_NormalAinr;
            }

            // Create AinrCtrler
            MINT32 sensorId = sel.mStateIspHidl.sensorId;
            createCtrlerInfo(sensorId);
        }

        // Get AinrCtrler
        std::shared_ptr<AinrCtrler> ctrler = getCtrlerInfo(getUniqueKey());
        if (CC_UNLIKELY(ctrler.get() == nullptr)) {
            removeCtrlerInfo(getUniqueKey());
            ainrLogD("There is no ctrler no need to execute AINR.");
            return BAD_VALUE;
        }

        // In HidlCamera stage no need to do plugin process
        sel.mDecision.mProcess = MTRUE;

        // In ISPHIDL case frame count should be 1
        sel.mRequestCount = 1;

        // Config selection buffers
        bool mainFrame = (frameIndex == 0) ? true : false;
        if (mainFrame) {
            updateCaptureInfo(ctrler.get());
        }
        configSelBuffer(sel, mainFrame);

        // Set metadata
        if (sel.mIMetadataApp.getControl() != nullptr) {
            MetadataPtr pAppAddtional = make_shared<IMetadata>();
            MetadataPtr pHalAddtional = make_shared<IMetadata>();

            IMetadata* pAppMeta = pAppAddtional.get();
            IMetadata* pHalMeta = pHalAddtional.get();

            // Set up bss
            IMetadata::setEntry<MINT32>(pHalMeta,
                    MTK_FEATURE_BSS_SELECTED_FRAME_COUNT, frameCount);
            IMetadata::setEntry<MINT32>(pHalMeta,
                    MTK_FEATURE_BSS_FORCE_DROP_NUM, 0);
            IMetadata::setEntry<MINT32>(pHalMeta,
                    MTK_FEATURE_BSS_PROCESS, MTK_FEATURE_BSS_PROCESS_ENABLE);
            IMetadata::setEntry<MBOOL>(pHalMeta,
                    MTK_FEATURE_BSS_REORDER, MTRUE);

            // Set up plugin key
            IMetadata::setEntry<MINT32>(pHalMeta,
                    MTK_PIPELINE_UNIQUE_KEY, getUniqueKey());

            sel.mIMetadataApp.setAddtional(pAppAddtional);
            sel.mIMetadataHal.setAddtional(pHalAddtional);
        }

        return OK;
    };

    // Caller should guarantee pointer is avaliable
    void configSelBuffer(Selection& sel, bool mainFrame) {
        if (mUsingRrz || sel.mIspHidlStage == ISP_HIDL_STAGE_DISABLED) {
            EImageFormat resizedFormat = eImgFmt_FG_BAYER10;
            EImageSize resizedSize = eImgSize_Resized;
            switch (mVersion) {
                case AINRCORE_VERSION_1_0:
                    resizedFormat = eImgFmt_FG_BAYER10;
                    resizedSize = eImgSize_Resized;
                break;
                case AINRCORE_VERSION_2_0:
                    resizedFormat = eImgFmt_MTK_YUV_P010;
                    resizedSize = eImgSize_Specified;
                break;
                default:
                    ainrLogF("Core version not correct");
                break;
            }

            sel.mIBufferResized
                .setRequired(MTRUE)
                .addAcceptedFormat(resizedFormat)
                .addAcceptedSize(resizedSize);
        }

        sel.mIBufferFull
            .setRequired(MTRUE)
            .addAcceptedFormat(eImgFmt_BAYER10)
            .addAcceptedSize(eImgSize_Full);

        sel.mIMetadataDynamic.setRequired(MTRUE);
        sel.mIMetadataApp.setRequired(MTRUE);
        sel.mIMetadataHal.setRequired(MTRUE);

        // Only main frame has output buffer
        if (mainFrame) {
            sel.mOBufferFull
                .setRequired(MTRUE)
                .addAcceptedFormat(eImgFmt_BAYER12_UNPAK)
                .addAcceptedSize(eImgSize_Full);

            sel.mOMetadataApp.setRequired(MTRUE);
            sel.mOMetadataHal.setRequired(MTRUE);
        } else {
            sel.mOBufferFull.setRequired(MFALSE);
            sel.mOMetadataApp.setRequired(MFALSE);
            sel.mOMetadataHal.setRequired(MFALSE);
        }
    };

    virtual void init()
    {
        CAM_ULOGM_APILIFE();
    };

    virtual MERROR process(RequestPtr pRequest,
                           RequestCallbackPtr pCallback)
    {
        CAM_ULOGM_APILIFE();

        // We need to make process method thread safe
        // Because plugin is a singleTon we need to protect it
        std::lock_guard<decltype(mProcessMx)> lk(mProcessMx);

        //set thread's name
        ::prctl(PR_SET_NAME, "AINRPlugin", 0, 0, 0);

        // Back up callback function for abort API
        if (pCallback != nullptr) {
            mPluginCB = pCallback;
        }

        /*
        * Be aware of that metadata and buffer should acquire one time
        */
        IImageBuffer* pIImgBuffer = nullptr;
        IImageBuffer* pIImageBufferRrzo = nullptr;
        IImageBuffer* pOImgBuffer = nullptr;
        // Get out metadata
        IMetadata* pAppMeta = pRequest->mIMetadataApp->acquire();
        IMetadata* pHalMeta = pRequest->mIMetadataHal->acquire();
        IMetadata* pAppMetaDynamic = pRequest->mIMetadataDynamic->acquire();
        IMetadata* pOutHaMeta = nullptr;
        //IMetadata* pOMetadata_Hal = pRequest->mOMetadataHal->acquire();

        if(pRequest->mOMetadataHal) {
            pOutHaMeta = pRequest->mOMetadataHal->acquire();
            if(CC_UNLIKELY(pOutHaMeta == nullptr)) {
                ainrLogE("pOutHaMeta is null idx(%d)!!!", pRequest->mRequestBSSIndex);
                return BAD_VALUE;
            }
        }

        // Check metadata
        if(CC_UNLIKELY(pAppMeta == nullptr)
            || CC_UNLIKELY(pHalMeta == nullptr)) {
            ainrLogE("one of metdata is null idx(%d)!!!", pRequest->mRequestBSSIndex);
            return BAD_VALUE;
        }

        // Get input buffer
        if (CC_LIKELY(pRequest->mIBufferFull != nullptr)) {
            pIImgBuffer = pRequest->mIBufferFull->acquire();
            if(CC_UNLIKELY(pIImgBuffer == nullptr)) {
                ainrLogE("Input buffer is null idx(%d)!!!", pRequest->mRequestBSSIndex);
                return BAD_VALUE;
            }
            if (mDump){
                std::string str = "InputImgo" + std::to_string(pRequest->mRequestBSSIndex);

                // dump input buffer
                MINT32 sensorId = pRequest->mSensorId;
                bufferDump(sensorId, pHalMeta, pIImgBuffer, TuningUtils::RAW_PORT_IMGO, str.c_str());
            }
        } else {
            ainrLogE("mIBufferFull is null");
            return BAD_VALUE;
        }

        // Get input rrzo buffer
        if (CC_LIKELY(pRequest->mIBufferResized != nullptr)) {
            pIImageBufferRrzo = pRequest->mIBufferResized->acquire();
            if(CC_UNLIKELY(pIImageBufferRrzo == nullptr)) {
                ainrLogF("Input buffer is null idx(%d)!!!", pRequest->mRequestBSSIndex);
                return BAD_VALUE;
            }
            if (mDump & AinrDumpWorking){
                std::string str = "InputRrzo" + std::to_string(pRequest->mRequestBSSIndex);
                // dump input buffer
                MINT32 sensorId = pRequest->mSensorId;
                bufferDump(sensorId, pHalMeta, pIImageBufferRrzo, TuningUtils::RAW_PORT_RRZO, str.c_str());
            }
        } else {
            ainrLogD("mIBufferResized is null");
        }

        // Get output buffer
        if (pRequest->mRequestBSSIndex == 0) {
            if (pRequest->mOBufferFull != nullptr) {
                pOImgBuffer = pRequest->mOBufferFull->acquire();
                if(CC_UNLIKELY(pOImgBuffer == nullptr)) {
                    ainrLogF("Output buffer is null idx(%d)!!!", pRequest->mRequestBSSIndex);
                    return BAD_VALUE;
                }
            } else {
                ainrLogF("mOBufferFull is null idx(%d)!!!", pRequest->mRequestBSSIndex);
                return BAD_VALUE;
            }
        }

        {
            std::lock_guard<decltype(mReqMx)> _reqlk(mReqMx);
            mvRequests.push_back(pRequest);
        }

        ainrLogD("collected request(%d/%d)",
                pRequest->mRequestBSSIndex+1,
                pRequest->mRequestBSSCount);
        /********************************Finish basic flow start to do AINR**********************************************/
        MINT32 processUniqueKey = 0;
        if (!IMetadata::getEntry<MINT32>(pHalMeta,
                MTK_PIPELINE_UNIQUE_KEY, processUniqueKey)) {
            ainrLogE("cannot get unique about ainr capture");
            return BAD_VALUE;
        }
        ainrLogD("Ainr MTK_PIPELINE_UNIQUE_KEY(%d)", processUniqueKey);

        //get AinrCtrler. Should we use processUniqueKey
        std::shared_ptr<AinrCtrler> ctrler = getCtrlerInfo(processUniqueKey);
        if (CC_UNLIKELY(ctrler.get() == nullptr)) {
            removeCtrlerInfo(processUniqueKey);
            ainrLogE("get AinrCtrler instance failed! cannot process AinrCtrler.");
            return BAD_VALUE;
        }

        // Add data (appMeta, halMeta, outHalMeta, IMGO, RRZO)
        // into AinrCtrler
        {
            AinrPipelinePack inputPack;
            inputPack.appMeta     = pAppMeta;
            inputPack.halMeta     = pHalMeta;
            inputPack.outHalMeta  = pOutHaMeta;
            inputPack.imgoBuffer  = pIImgBuffer;
            inputPack.rrzoBuffer  = pIImageBufferRrzo; //Remeber to modify as rrzo
            ctrler->addInputData(inputPack);
        }

        // Initialize
        if (pRequest->mRequestBSSIndex == 0) {
            // Set up release function
            mCbFunction = [this](int32_t uniqueKey, int32_t mainFrameIndex) {
                std::lock_guard<decltype(mReqTableMx)> _lk(mReqTableMx);

                auto search = mReqTable.find(uniqueKey);
                if(search != mReqTable.end()) {
                    ainrLogD("Start to release buffers(%d)", uniqueKey);
                    auto vRequests = search->second;
                    for (auto req : vRequests) {
                        ainrLogD("Release index(%d)", req->mRequestIndex);
                        _releasePluginBuffer(req->mIBufferFull);
                        _releasePluginBuffer(req->mIBufferResized);
                        _releasePluginBuffer(req->mIMetadataApp);
                        _releasePluginBuffer(req->mIMetadataHal);
                        _releasePluginBuffer(req->mIMetadataDynamic);

                        // Indicate next captures
                        if(req->mRequestIndex == req->mRequestCount - 1) {
                            std::lock_guard<decltype(mCbMx)> _cblk(mCbMx);
                            ainrLogD("Next capture callback");
                            auto cbFind = mCbTable.find(uniqueKey);

                            if(cbFind != mCbTable.end()) {
                                if(cbFind->second != nullptr) {
                                    cbFind->second->onNextCapture(req);
                                }
                            } else {
                                ainrLogW("cannot find cb from table");
                            }
                        }
                    }
                } else {
                    ainrLogF("Can not find requests in reqTable by key(%d)", uniqueKey);
                }
                return;
            };

            // Initialize AinrCtrler
            MSize  imgo       = pIImgBuffer->getImgSize();
            size_t imgoStride = pIImgBuffer->getBufStridesInBytes(0);
            MSize  rrzo = {};
            if (pIImageBufferRrzo) {
                rrzo = pIImageBufferRrzo->getImgSize();
            } else {
                IMetadata::Memory feoInfo;
                if (!IMetadata::getEntry<IMetadata::Memory>(pHalMeta,
                        MTK_ISP_FEO_INFO, feoInfo)) {
                    ainrLogF("cannot get MTK_ISP_FEO_INFO");
                    return BAD_VALUE;
                }

                // structure check
                constexpr auto feoInfoSize = sizeof(IAinrFeFm::FeoInfo);
                if (CC_UNLIKELY(feoInfoSize != feoInfo.size())) {
                    ainrLogF("FeoInfo size not sync(%zu, %zu)",
                            feoInfoSize, feoInfo.size());
                    return BAD_VALUE;
                }

                IAinrFeFm::FeoInfo *infoPtr
                        = reinterpret_cast<IAinrFeFm::FeoInfo*>(feoInfo.editArray());
                rrzo = infoPtr->rrzoSize;
            }
            // Setup debug exif
            ctrler->setAlgoMode(AINR_ONLY);
            ctrler->setCaptureNum(pRequest->mRequestBSSCount);
            ctrler->setSizeImgo(imgo);
            ctrler->setSizeRrzo(rrzo);
            ctrler->setStrideImgo(imgoStride);
            // config ainr controller
            ctrler->configAinrCore(pHalMeta, pAppMetaDynamic);
            ctrler->addOutData(pOImgBuffer);
            ctrler->registerCB(mCbFunction);

            // Back up callback
            {
                std::lock_guard<decltype(mCbMx)> _cblk(mCbMx);
                if(pCallback != nullptr) {
                    if (mCbTable.emplace(processUniqueKey, pCallback).second == false)
                        ainrLogE("Emplace callback fail!!");
                } else {
                    ainrLogW("Callback ptr is null!!!");
                }
            }
        }

        // Last frame
        if (pRequest->mRequestBSSIndex == pRequest->mRequestBSSCount - 1)
        {
            ainrLogD("have collected all requests");
            {
                std::lock_guard<decltype(mReqTableMx)> _reqlk(mReqTableMx);
                // Store requests in reqTable
                if (mReqTable.emplace(processUniqueKey, mvRequests).second == false)
                    ainrLogE("Emplace requests fail!!");
            }

            {
                // Becuase we already copied the requests
                // Need to clear container
                std::lock_guard<decltype(mReqMx)> _reqlk(mReqMx);
                mvRequests.clear();
            }

            ctrler->execute();

            auto __job = [this] (MINT32 key, RequestCallbackPtr pCb) {
                ainrLogD("Process uniqueKey(%d) job", key);
                std::shared_ptr<AinrCtrler>  ctr = getCtrlerInfo(key);
                if (CC_UNLIKELY(ctr.get() == nullptr)) {
                    ainrLogE("Job Queue gets instance failed!");
                } else {
                    // Wait AINR postprocessing done
                    ctr->waitExecution();
                    ctr = nullptr;
                    // We finish postprocess, release uniqueKey
                    removeCtrlerInfo(key);
                }

                // Decrease ainr counter
                gShotCount--;

                auto requestsPtr = getRequests(key);

                if(requestsPtr) {
                    for (auto req : (*requestsPtr)) {
                        ainrLogD("callback request(%d/%d) %p",
                                req->mRequestBSSIndex+1,
                                req->mRequestBSSCount, pCb.get());
                        if (pCb != nullptr) {
                            pCb->onCompleted(req, 0);
                        }
                    }
                }

                // Clear up resources
                removeRequests(key);

                // Back up callback
                {
                    std::lock_guard<decltype(mCbMx)> __lk(mCbMx);

                    if(mCbTable.count(key)) {
                        mCbTable[key] = nullptr;
                        mCbTable.erase(key);
                    }
                }
            };
            acquireJobQueue();
            mMinorJobQueue->addJob( std::bind(__job, processUniqueKey, pCallback) );
        }
        return 0;
    }

    void update3AMetadata(
            const int exp,
            const int iso,
            IMetadata* pAppMeta,
            IMetadata* pHalMeta)
    {
        CAM_ULOGM_APILIFE();

        // Manual AE
        ainrLogD("We set manual AE (iso,exp)=(%d,%d us)",
                iso, exp);
        IMetadata::setEntry<MUINT8>(pAppMeta, MTK_CONTROL_AE_MODE, MTK_CONTROL_AE_MODE_OFF);
        IMetadata::setEntry<MINT32>(pAppMeta, MTK_SENSOR_SENSITIVITY, iso);
        // us--->ns
        MINT64 shutter_ns = static_cast<MINT64>(exp) * 1000;
        IMetadata::setEntry<MINT64>(pAppMeta,
                MTK_SENSOR_EXPOSURE_TIME, shutter_ns);

        // Lock AWB
        IMetadata::setEntry<MUINT8>(pAppMeta, MTK_CONTROL_AWB_LOCK, MTRUE);

        // Lock AF
        IMetadata::setEntry<MUINT8>(pHalMeta, MTK_FOCUS_PAUSE, 1);

        // AE Flare Enable, due to MTK_CONTROL_AE_MODE_OFF will disable AE_Flare
        // MFNR/AINR must set MTK_3A_FLARE_IN_MANUAL_CTRL_ENABLE to enable AE_Flare
        // TODO: hal3a need to implement this command for each platform.
        IMetadata::setEntry<MBOOL>(pHalMeta, MTK_3A_FLARE_IN_MANUAL_CTRL_ENABLE, MTRUE);
    }

    virtual void abort(vector<RequestPtr>& pRequests __attribute__((unused)))
    {
        CAM_ULOGM_APILIFE();

        // Wait all jobs in queue to be finished
        acquireJobQueue();
        mMinorJobQueue->wait();

        ainrLogD("AINR abort");
        if (mPluginCB.get()) {
            for (auto &&req : pRequests) {
                mPluginCB->onAborted(req);
            }
        }
    };

    virtual void uninit()
    {
        CAM_ULOGM_APILIFE();
    };

    AINRProcImpl()
        : mEnable(-1)
        , mRealIso(0)
        , mShutterTime(0)
        , mZSDMode(MFALSE)
        , mFlashOn(MFALSE)
        , mUniqueKey(0)
        , mMfbMode(AinrMode_Off)
        , mManualShutter(0)
        , mManualIso(0)
        , mVersion(AINRCORE_VERSION_0_0)
        , mPluginCB(nullptr)
    {
        CAM_ULOGM_APILIFE();
        mDump = ::property_get_int32("vendor.debug.camera.ainr.dump", 0);
        mUsingRrz = ::property_get_int32("vendor.debug.camera.ainr.userrz", 1);
    };

    virtual ~AINRProcImpl()
    {
        CAM_ULOGM_APILIFE();
        removeAllCtrlers();

        // Remove all callback ptr
        {
            std::lock_guard<decltype(mCbMx)> __lk(mCbMx);

            ainrLogD("Remove callback table");
            for(auto & cb : mCbTable) {
                cb.second = nullptr;
            }
            mCbTable.clear();
        }
    };
private:
    AinrMode updateMfbMode(IMetadata* pAppMeta, MBOOL isZSDMode) {

        IMetadata::IEntry const eMfb           = pAppMeta->entryFor(MTK_MFNR_FEATURE_MFB_MODE);
        IMetadata::IEntry const eAis           = pAppMeta->entryFor(MTK_MFNR_FEATURE_AIS_MODE);
        IMetadata::IEntry const eSceneMode     = pAppMeta->entryFor(MTK_CONTROL_SCENE_MODE);

        int mfbMode = [&]()
        {
            // If MTK specific parameter AIS on or MFB mode is AIS, set to AIS mode (2)
            if (( ! eMfb.isEmpty() && eMfb.itemAt(0, Type2Type<MINT32>()) == MTK_MFNR_FEATURE_MFB_AIS)  ||
                     ( ! eAis.isEmpty() && eAis.itemAt(0, Type2Type<MINT32>()) == MTK_MFNR_FEATURE_AIS_ON)) {
                return MTK_MFB_MODE_AIS;
            }
            // Scene mode is Night or MFB mode is MFLL, set to MFLL mode (1)
            else if (( ! eMfb.isEmpty() && eMfb.itemAt(0, Type2Type<MINT32>()) == MTK_MFNR_FEATURE_MFB_MFLL) ||
                     ( ! eSceneMode.isEmpty() && eSceneMode.itemAt(0, Type2Type<MUINT8>()) == MTK_CONTROL_SCENE_MODE_NIGHT)) {
                return MTK_MFB_MODE_MFLL;
            }
            else if (( ! eMfb.isEmpty() && eMfb.itemAt(0, Type2Type<MINT32>()) == MTK_MFNR_FEATURE_MFB_AUTO)) {
#ifdef CUST_MFLL_AUTO_MODE
                static_assert( ((CUST_MFLL_AUTO_MODE >= MTK_MFB_MODE_OFF)&&(CUST_MFLL_AUTO_MODE < MTK_MFB_MODE_NUM)),
                               "CUST_MFLL_AUTO_MODE is invalid in custom/feature/mfnr/camera_custom_mfll.h" );

                ainrLogD("CUST_MFLL_AUTO_MODE:%d", CUST_MFLL_AUTO_MODE);
                return static_cast<mtk_platform_metadata_enum_mfb_mode>(CUST_MFLL_AUTO_MODE);
#else
#error "CUST_MFLL_AUTO_MODE is no defined in custom/feature/mfnr/camera_custom_mfll.h"
#endif
            }
            // Otherwise, set MFB off (0)
            else {
                return MTK_MFB_MODE_OFF;
            }
        }();


        AinrMode ainrMode = AinrMode_Off;

        // 0: Not specific, 1: MFNR, 2: AIS
        switch (mfbMode) {
            case MTK_MFB_MODE_MFLL:
                ainrMode = (CC_LIKELY(isZSDMode) ? AinrMode_ZsdAinr : AinrMode_NormalAinr);
                break;
            case MTK_MFB_MODE_AIS:
                ainrMode = AinrMode_NormalAis;
                break;
            case MTK_MFB_MODE_OFF:
                ainrMode = AinrMode_Off;
                break;
            default:
                ainrMode = (CC_LIKELY(isZSDMode) ? AinrMode_ZsdAinr : AinrMode_NormalAinr);
                break;
        }

        ainrLogD("AinrMode(0x%X), mfbMode(%d), isZsd(%d)",
                ainrMode, mfbMode, isZSDMode);


        return ainrMode;
    }
    MINT32 getUniqueKey()
    {
        CAM_ULOGM_APILIFE();

        std::lock_guard<decltype(mCtrlerMx)> lk(mCtrlerMx);
        return mUniqueKey;
    }

    void setUniqueKey(uint32_t key)
    {
        CAM_ULOGM_APILIFE();

        std::lock_guard<decltype(mCtrlerMx)> lk(mCtrlerMx);
        mUniqueKey = key;
    }

    void createCtrlerInfo(MINT32 sensorId)
    {
        CAM_ULOGM_APILIFE();

        std::lock_guard<decltype(mCtrlerMx)> lk(mCtrlerMx);
        auto checkPair = mCtrlerContainer.emplace(mUniqueKey,
                std::make_shared<AinrCtrler>(mUniqueKey,
                sensorId, mMfbMode, mRealIso, mShutterTime, mFlashOn));
        if (checkPair.second == false) {
            ainrLogF("Create ctrler fail");
            return;
        }
        ainrLogD("Create ctrler(%d) sensorId(%d)", mUniqueKey, sensorId);
    };

    void removeCtrlerInfo(uint32_t key)
    {
        CAM_ULOGM_APILIFE();

        std::lock_guard<decltype(mCtrlerMx)> lk(mCtrlerMx);
        if (mCtrlerContainer.count(key)) {
            ainrLogD("Remvoe AinrCtrler: %d", key);
            mCtrlerContainer[key] = nullptr;
            mCtrlerContainer.erase(key);
        }
    };

    void removeAllCtrlers()
    {
        CAM_ULOGM_APILIFE();

        std::lock_guard<decltype(mCtrlerMx)> lk(mCtrlerMx);
        for (auto it = mCtrlerContainer.begin(); it != mCtrlerContainer.end(); ++it) {
            it->second = nullptr;
            ainrLogD("Remvoe AinrCtrler: %d", it->first);
        }
        mCtrlerContainer.clear();
    };

    std::shared_ptr<AinrCtrler> getCtrlerInfo(uint32_t key)
    {
        CAM_ULOGM_APILIFE();
        std::lock_guard<decltype(mCtrlerMx)> lk(mCtrlerMx);

        ainrLogD("Get ctrler: %d", key);
        auto find = mCtrlerContainer.find(key);
        if(find != mCtrlerContainer.end()) {
            return find->second;
        } else {
            ainrLogE("Can not find ctrler:%d", key);
            return nullptr;
        }
    };

    std::vector<RequestPtr> *getRequests(MINT32 key)
    {
        CAM_ULOGM_APILIFE();

        std::lock_guard<decltype(mReqTableMx)> lk(mReqTableMx);

        auto find = mReqTable.find(key);
        if(find != mReqTable.end()) {
            return &find->second;
        } else {
            ainrLogF("Can not get requests from table");
            return nullptr;
        }
    };

    void removeRequests(MINT32 key)
    {
        CAM_ULOGM_APILIFE();

        std::lock_guard<decltype(mReqTableMx)> __lk(mReqTableMx);
        if (mReqTable.count(key)) {
            mReqTable[key].clear();
            mReqTable.erase(key);

            ainrLogD("Remvoe requests: %d from table", key);
        }
    };

    // Update capture information in negotiate stage in first frame
    // No need to take care of thread safe or not.
    void updateCaptureInfo(const AinrCtrler* ctrler) {
        // Ainr core version update
        mVersion = ctrler->queryCoreVersion();
        ainrLogD("Ainr core version(0x%x)", mVersion);

        // Manual AE information if needed
        mManualShutter = ctrler->getFinalShutterTime();
        mManualIso = ctrler->getFinalIso();
        ainrLogD("Manual AE info (iso,exp)=(%d,%d us)",
                mManualIso, mManualShutter);
    }

    void acquireJobQueue()
    {
        static std::mutex       sResLock;
        static std::weak_ptr<
            NSCam::JobQueue<void()>
        >                       sResJobQ;

        // acquire resource from weak_ptr
        std::lock_guard<std::mutex> lk(sResLock);
        mMinorJobQueue = sResJobQ.lock();
        if (mMinorJobQueue.get() == nullptr) {
            mMinorJobQueue = std::shared_ptr< NSCam::JobQueue<void()> >(
                    new NSCam::JobQueue<void()>("AINRPluginJob")
                    );
            sResJobQ = mMinorJobQueue;
        }
        return;
    }

    void bufferDump(MINT32 sensorId, IMetadata *halMeta, IImageBuffer* buff, RAW_PORT type
            , const char *pUserString) {
        char fileName[512] = {};
        FILE_DUMP_NAMING_HINT dumpNamingHint = {};
        //
        if(!halMeta || !buff) {
            ainrLogE("HalMeta or buff is nullptr, dump fail");
            return;
        }

        MUINT8 ispProfile = NSIspTuning::EIspProfile_Capture;
        if (!IMetadata::getEntry<MUINT8>(halMeta, MTK_3A_ISP_PROFILE,
                ispProfile)) {
            ainrLogW("cannot get ispProfile at ainr capture");
        }

        // Extract hal metadata and fill up file name;
        extract(&dumpNamingHint, halMeta);
        // Extract buffer information and fill up file name;
        extract(&dumpNamingHint, buff);
        // Extract by sensor id
        extract_by_SensorOpenId(&dumpNamingHint, sensorId);
        // IspProfile
        dumpNamingHint.IspProfile = ispProfile;

        genFileName_RAW(fileName, sizeof(fileName), &dumpNamingHint,
                type, pUserString);
        buff->saveToFile(fileName);
    }
private:

    //
    int                             mEnable;
    int                             mDump;
    int                             mUsingRrz;
    const double                    RESTRICT_MEM_AMOUNT = (4.3); //4G 1024*1024*1024*4
    // file dump hint
    FILE_DUMP_NAMING_HINT           mDumpNamingHint;
    //
    std::mutex                      mProcessMx; // TO make plugin process thread safe

    // Requests
    std::mutex                      mReqMx;
    std::vector<RequestPtr>         mvRequests;
    // Callback
    std::mutex                      mCbMx;
    //RequestCallbackPtr              mCbPtr;
    std::unordered_map< MINT32
            , RequestCallbackPtr >  mCbTable;

    //
    MINT32                          mRealIso;
    MINT32                          mShutterTime;
    MBOOL                           mZSDMode;
    MBOOL                           mFlashOn;
    //
    std::mutex                      mCtrlerMx; // protect AinrCtrler
    uint32_t                        mUniqueKey;
    AinrMode                        mMfbMode;

    // Manual AE
    int                             mManualShutter;
    int                             mManualIso;

    // AINR core version
    AINRCORE_VERSION                mVersion;

    // Plugin callback
    RequestCallbackPtr              mPluginCB;

    // std function
    std::function<void(int32_t, int32_t)>
                                    mCbFunction;
    // Requests table
    std::mutex                      mReqTableMx;
    std::unordered_map< MINT32
        , std::vector<RequestPtr> > mReqTable;

    // Ctrler container
    std::unordered_map<uint32_t, std::shared_ptr<AinrCtrler>>
                                    mCtrlerContainer;
    // JobQueue
    std::shared_ptr<
        NSCam::JobQueue<void()>
    >                               mMinorJobQueue;
};

REGISTER_PLUGIN_PROVIDER(MultiFrame, AINRProcImpl);
