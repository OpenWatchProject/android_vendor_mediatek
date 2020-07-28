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
#define LOG_TAG "MFNRPlugin"
static const char* __CALLERNAME__ = LOG_TAG;

//
#include <mtkcam/utils/std/ULog.h>
//
#include <stdlib.h>
#include <utils/Errors.h>
#include <utils/List.h>
#include <utils/RefBase.h>
#include <sstream>
#include <unordered_map> // std::unordered_map
//
#include <mtkcam/utils/metadata/client/mtk_metadata_tag.h>
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>
#include <custom/feature/mfnr/camera_custom_mfll.h>
//zHDR
#include <mtkcam/utils/hw/HwInfoHelper.h> // NSCamHw::HwInfoHelper
#include <mtkcam3/feature/utils/FeatureProfileHelper.h> //ProfileParam
#include <mtkcam/drv/IHalSensor.h>
//
#include <mtkcam/utils/hw/FleetingQueue.h>
//
#include <mtkcam/utils/imgbuf/IIonImageBufferHeap.h>
//
#include <mtkcam/utils/std/Format.h>
#include <mtkcam/utils/std/Time.h>
//
#include <mtkcam3/pipeline/hwnode/NodeId.h>
//
#include <mtkcam/utils/metastore/IMetadataProvider.h>
#include <mtkcam/utils/metastore/ITemplateRequest.h>
#include <mtkcam/utils/metastore/IMetadataProvider.h>
#include <mtkcam3/3rdparty/plugin/PipelinePlugin.h>
#include <mtkcam3/3rdparty/plugin/PipelinePluginType.h>
#include "MFNRShotInfo.h"
//
#include <isp_tuning/isp_tuning.h>  //EIspProfile_T, EOperMode_*


//

using namespace NSCam;
using namespace plugin;
using namespace android;
using namespace mfll;
using namespace std;
using namespace NSCam::NSPipelinePlugin;
using namespace NSIspTuning;
/******************************************************************************
 *
 ******************************************************************************/
CAM_ULOG_DECLARE_MODULE_ID(MOD_LIB_MFNR);
#define MY_LOGV(fmt, arg...)        CAM_ULOGMV("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_ULOGMD("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_ULOGMI("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_ULOGMW("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_ULOGME("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
//
#define ASSERT(cond, msg)           do { if (!(cond)) { printf("Failed: %s\n", msg); return; } }while(0)


#define __DEBUG // enable debug

#ifdef __DEBUG
#include <memory>
#define FUNCTION_SCOPE \
auto __scope_logger__ = [](char const* f)->std::shared_ptr<const char>{ \
    CAM_ULOGMD("(%d)[%s] + ", ::gettid(), f); \
    return std::shared_ptr<const char>(f, [](char const* p){CAM_ULOGMD("(%d)[%s] -", ::gettid(), p);}); \
}(__FUNCTION__)
#else
#define FUNCTION_SCOPE
#endif

#define ISP_HIDL_TEMP_SOL_WITHOUT_TUNING_INDEX_HINT 1

#ifdef ISP_HIDL_TEMP_SOL_WITHOUT_TUNING_INDEX_HINT
// HIDL ISP shot instance
struct RecommendShooter {
    std::shared_ptr<MFNRShotInfo> shotInstance;
    RecommendShooter () : shotInstance(nullptr) { };
    ~RecommendShooter () { shotInstance = nullptr; };
};
static MUINT32 mRecommendCount;
static std::mutex mRecommendCountMx; // protect mRecommendCount
//
static NSCam::FleetingQueue<RecommendShooter, 30, 2> mRecommendQueue;
#else
// do nothing
#endif


/******************************************************************************
*
******************************************************************************/
class MFNRProviderImpl : public MultiFramePlugin::IProvider
{
    typedef MultiFramePlugin::Property Property;
    typedef MultiFramePlugin::Selection Selection;
    typedef MultiFramePlugin::Request::Ptr RequestPtr;
    typedef MultiFramePlugin::RequestCallback::Ptr RequestCallbackPtr;
    typedef MultiFramePlugin::RecommendedParam RecommendedParam;

public:


    virtual const Property& property()
    {
        FUNCTION_SCOPE;
        static Property prop;
        static bool inited;
        static MINT32 support10bit = 0;

        if (!inited) {
            prop.mName              = "MTK MFNR";
            prop.mFeatures          = MTK_FEATURE_MFNR;
            prop.mThumbnailTiming   = eTiming_P2;
            prop.mPriority          = ePriority_Highest;
#ifdef MFB_MAX_FRAME
            prop.mZsdBufferMaxNum   = MFB_MAX_FRAME; // maximum frames requirement
#else
            prop.mZsdBufferMaxNum   = 8; // maximum frames requirement
#endif
            prop.mNeedRrzoBuffer    = MTRUE; // rrzo requirement for BSS
            prop.mBoost             = eBoost_CPU;
            inited                  = MTRUE;

            support10bit = property_get_int32("vendor.debug.p2c.10bits.enable", 1);
            prop.mZsdBufferMaxNum = property_get_int32("vendor.debug.plugin.mfll.maxframe", prop.mZsdBufferMaxNum);

        }
        mSupport10BitOutput = support10bit;

        return prop;
    };

    virtual MERROR negotiate(Selection& sel)
    {
        FUNCTION_SCOPE;
        int SelIndex = -1, SelCount = -1;
        MINT64 HidlQueryIndex = -1;
        MBOOL needUpdateStrategy = MTRUE;
        mOpenId = sel.mState.mSensorId;

        if (sel.mIspHidlStage == ISP_HIDL_STAGE_DISABLED) {
            SelIndex = sel.mRequestIndex;
            SelCount = sel.mRequestCount;
        }
        else if (sel.mIspHidlStage == ISP_HIDL_STAGE_REUEST_FRAME_FROM_CAMERA) {
            sel.mDecision.mProcess = false;
            sel.mRequestCount = 1;
            if (CC_LIKELY(sel.mIMetadataApp.getControl() != NULL)) {
                IMetadata* pAppMeta = sel.mIMetadataApp.getControl().get();
                if(!IMetadata::getEntry<MINT32>(pAppMeta, MTK_CONTROL_CAPTURE_HINT_FOR_ISP_FRAME_INDEX, SelIndex)) {
                    MY_LOGE("cannot get ISP_FRAME_INDEX");
                }
                if(!IMetadata::getEntry<MINT32>(pAppMeta, MTK_CONTROL_CAPTURE_HINT_FOR_ISP_FRAME_COUNT, SelCount)) {
                    MY_LOGE("cannot get ISP_FRAME_COUNT");
                }
                if(!IMetadata::getEntry<MINT64>(pAppMeta, MTK_CONTROL_CAPTURE_HINT_FOR_ISP_FRAME_TUNING_INDEX, HidlQueryIndex)) {
                    MY_LOGE("cannot get ISP_FRAME_TUNING_INDEX");
                } else {
                    MY_LOGD("get MTK_CONTROL_CAPTURE_HINT_FOR_ISP_FRAME_TUNING_INDEX:0x%jx", HidlQueryIndex);
                }
            } else {
                MY_LOGE("Error: mIMetadataApp is NULL!!");
            }
        }
        else if ( sel.mIspHidlStage == ISP_HIDL_STAGE_PROCESS_FRAME_IN_ISP_HIDL ) {
            sel.mDecision.mProcess = true;
            sel.mRequestCount = 1;
            SelIndex = sel.mStateIspHidl.frameIndex;
            SelCount = sel.mStateIspHidl.frameCount;
#ifdef ISP_HIDL_TEMP_SOL_WITHOUT_TUNING_INDEX_HINT
            if (CC_LIKELY(sel.mIMetadataApp.getControl() != NULL)) {
                IMetadata* pHalMeta = sel.mIMetadataHal.getControl().get();
                if (!IMetadata::getEntry<MINT64>(pHalMeta, MTK_FEATURE_MFNR_TUNING_INDEX_HINT, HidlQueryIndex)) {
                    MY_LOGD("cannot get MTK_FEATURE_MFNR_TUNING_INDEX_HINT");
                } else {
                    MY_LOGD("get MTK_FEATURE_MFNR_TUNING_INDEX_HINT: 0x%jx", HidlQueryIndex);
                }
            }
#endif
            if (HidlQueryIndex == -1 && CC_LIKELY(sel.mIMetadataApp.getControl() != NULL)) {
                IMetadata* pAppMeta = sel.mIMetadataApp.getControl().get();
                if (!IMetadata::getEntry<MINT64>(pAppMeta, MTK_CONTROL_CAPTURE_HINT_FOR_ISP_FRAME_TUNING_INDEX, HidlQueryIndex)) {
                    MY_LOGE("cannot get ISP_FRAME_TUNING_INDEX");
                } else {
                    MY_LOGD("get MTK_CONTROL_CAPTURE_HINT_FOR_ISP_FRAME_TUNING_INDEX: 0x%jx", HidlQueryIndex);
                }
            }
        }
        else {
            MY_LOGE("Error: should not be here, mIspHidlStage is unknown!!");
        }

        MY_LOGD("Collected Selection:(%d/%d), ISP mode: %d", SelIndex, SelCount, sel.mIspHidlStage);

        // create MFNRShotInfo
        if (SelIndex == 0) {

            mZSDMode = sel.mState.mZslRequest && sel.mState.mZslPoolReady;
            mFlashOn = sel.mState.mFlashFired;
            mRealIso = sel.mState.mRealIso;
            mShutterTime = sel.mState.mExposureTime;

            if (mRealIso <= 0)
                mRealIso = 0;
            if (mShutterTime <= 0)
                mShutterTime = 0;

            if (CC_UNLIKELY( mFlashOn && mZSDMode )) {
                MY_LOGD("do not use ZSD buffers due to Flash MFNR");
                mZSDMode = MFALSE;
            }

            if (sel.mIspHidlStage == ISP_HIDL_STAGE_DISABLED || sel.mIspHidlStage == ISP_HIDL_STAGE_REUEST_FRAME_FROM_CAMERA) {
                setUniqueKey(static_cast<MINT32>(NSCam::Utils::TimeTool::getReadableTime()));
            }
            else if (sel.mIspHidlStage == ISP_HIDL_STAGE_PROCESS_FRAME_IN_ISP_HIDL) {
                if (sel.mIMetadataHal.getControl() != NULL) {
                    IMetadata* pHalMeta = sel.mIMetadataHal.getControl().get();
                    MINT32 uniquekey = static_cast<MINT32>(NSCam::Utils::TimeTool::getReadableTime());
                    if (!IMetadata::getEntry<MINT32>(pHalMeta, MTK_PIPELINE_UNIQUE_KEY, uniquekey)) {
                        MY_LOGW("cannot get PIPELINE_UNIQUE_KEY");
                    } else {
                        MY_LOGD("MTK_PIPELINE_UNIQUE_KEY: %d", uniquekey);
                    }
                    setUniqueKey(uniquekey);

                    if (!IMetadata::getEntry<MINT32>(pHalMeta, MTK_FEATURE_MFNR_OPEN_ID, mOpenId)) {
                        MY_LOGW("cannot get OPEN_ID, set to sel.mStateIspHidl.sensorId");
                        mOpenId = sel.mStateIspHidl.sensorId;
                    }
                    MY_LOGD("mOpenId in ISP_HIDL: %d", mOpenId);
                    if (HidlQueryIndex != -1) {
                        std::lock_guard<decltype(mShotInfoMx)> lk(mShotInfoMx);
                        vector<RecommendShooter*> shooter = mRecommendQueue.getInfo(__CALLERNAME__, HidlQueryIndex, HidlQueryIndex);
                        if (!shooter.empty() && shooter[0]->shotInstance != nullptr) {
                            if(mShots[mUniqueKey] == nullptr) {
                                if(shooter[0]->shotInstance->getOpenId() == mOpenId) {
                                    MY_LOGD("replace shot instance to recommended one.");
                                    mShots[mUniqueKey] = shooter[0]->shotInstance;
                                    shooter[0]->shotInstance = nullptr;
                                    mShots[mUniqueKey]->updateUniqueKey(mUniqueKey);
                                    needUpdateStrategy = MFALSE;
                                } else {
                                    MY_LOGW("Reommended OpenId:%d is not the same with camera stage ID:%d", shooter[0]->shotInstance->getOpenId(), mOpenId);
                                }
                            } else {
                                MY_LOGW("Reommended shot is already exist.");
                            }
                        } else {
                            MY_LOGW("Reommended shot(%jx) is not found.", HidlQueryIndex);
                        }
                        mRecommendQueue.returnInfo(__CALLERNAME__, shooter);
                    }
                }
                else {
                    MY_LOGE("Error: mIMetadataHal is NULL!!");
                }
            }
            else {
                MY_LOGE("Error: should not be here, mIspHidlStage is unknown!!");
            }

            if (CC_LIKELY(sel.mIMetadataApp.getControl() != NULL)) {
                IMetadata* pAppMeta = sel.mIMetadataApp.getControl().get();
                mMfbMode = updateMfbMode(pAppMeta, mZSDMode);
            }

            if (CC_UNLIKELY( mMfbMode == MfllMode_Off )) {
                return BAD_VALUE;
            }

            createShotInfo(sel.mIspHidlStage, sel.mState.mSensorId);
#ifdef __DEBUG
            dumpShotInfo();
#endif

            sel.mDecision.mZslEnabled = isZsdMode(mMfbMode);
            sel.mDecision.mZslPolicy.mPolicy = v3::pipeline::policy::eZslPolicy_AfState
                                             | v3::pipeline::policy::eZslPolicy_ContinuousFrame
                                             | v3::pipeline::policy::eZslPolicy_ZeroShutterDelay;
            sel.mDecision.mZslPolicy.mTimeouts = 1000;
        }

        //get MFNRShotInfo
        std::shared_ptr<MFNRShotInfo> pShotInfo = getShotInfo(getUniqueKey());
        if (CC_UNLIKELY(pShotInfo.get() == nullptr)) {
            removeShotInfo(getUniqueKey());
            MY_LOGE("get MFNRShotInfo instance failed! cannot apply MFNR shot.");
            return BAD_VALUE;
        }

        //Mfll Core Version
        static const auto mfllCoreVersion = MFLL_MAKE_REVISION(MFLL_MAJOR_VER(pShotInfo->getMfnrCoreVersion()), MFLL_MINOR_VER(pShotInfo->getMfnrCoreVersion()), 0);
        //update policy
        if (SelIndex == 0) {
            if( sel.mIspHidlStage == ISP_HIDL_STAGE_PROCESS_FRAME_IN_ISP_HIDL ) {
                if(sel.mStateIspHidl.pAppImage_Input_Yuv != nullptr) {
                    pShotInfo->setSizeSrc(sel.mStateIspHidl.pAppImage_Input_Yuv->getImgSize());
                }
                else if (sel.mStateIspHidl.pAppImage_Input_RAW16 != nullptr) {
                    pShotInfo->setSizeSrc(sel.mStateIspHidl.pAppImage_Input_RAW16->getImgSize());
                }
                else {
                    MY_LOGE("Error: ISP HIDL no available input buffer");
                }
            }
            else {
                pShotInfo->setSizeSrc(sel.mState.mSensorSize);
            }
            MY_LOGD("set source size = %dx%d", pShotInfo->getSizeSrc().w, pShotInfo->getSizeSrc().h);

            switch (mfllCoreVersion) {
                /**
                 * TODO: new case must check FOV is porting or not first.
                 * case MFLL_MAKE_REVISION(new_isp_version):
                 *   break;
                 */
                case MFLL_MAKE_REVISION(1, 4, 0):
                case MFLL_MAKE_REVISION(2, 1, 0):
                    break;
                case MFLL_MAKE_REVISION(2, 0, 0):
                case MFLL_MAKE_REVISION(2, 5, 0):
                case MFLL_MAKE_REVISION(3, 0, 0):
                    if (CC_LIKELY( !sel.mState.mDualCamDedicatedYuvSize )) {
                        MY_LOGD("Fov is not found");
                    } else {
                        pShotInfo->setSizeSrc(sel.mState.mDualCamDedicatedYuvSize);
                        MY_LOGI("Apply FOV size, SensorSize(%dx%d), Fov(%dx%d), "
                            , sel.mState.mSensorSize.w, sel.mState.mSensorSize.h
                            , sel.mState.mDualCamDedicatedYuvSize.w, sel.mState.mDualCamDedicatedYuvSize.h);
                    }
                    MY_LOGD("Set multicam feature, mode:%d", sel.mState.mMultiCamFeatureMode);
                    pShotInfo->setMulitCamFeatureMode(sel.mState.mMultiCamFeatureMode);
                    break;
                default:
                    removeShotInfo(getUniqueKey());
                    MY_LOGE("MFNR Provider is not support for mfnrcore(%s)", pShotInfo->getMfnrCoreVersionString().c_str());
                    return BAD_VALUE;
            }

            if (needUpdateStrategy)
                pShotInfo->updateMfllStrategy();

            MY_LOGD("realIso = %d, shutterTime = %d, finalRealIso = %d, finalShutterTime = %d"
                , pShotInfo->getRealIso()
                , pShotInfo->getShutterTime()
                , pShotInfo->getFinalIso()
                , pShotInfo->getFinalShutterTime());
#ifdef ISP_HIDL_TEMP_SOL_WITHOUT_TUNING_INDEX_HINT
            m_recommendedKey = generateRecommendedKey(pShotInfo->getNvramIndex());
#endif
        }

        if (sel.mIspHidlStage == ISP_HIDL_STAGE_DISABLED) {
            sel.mRequestCount = pShotInfo->getCaptureNum();
            SelCount = sel.mRequestCount;

            MY_LOGD("Mfll apply = %d, frames = %d", pShotInfo->getIsEnableMfnr(), sel.mRequestCount);

            if (!pShotInfo->getIsEnableMfnr()
                || sel.mRequestCount == 0
                || !updateInputBufferInfo(mfllCoreVersion, sel)) {
                removeShotInfo(getUniqueKey());
                return BAD_VALUE;
            }
        }
        else {
            updateInputBufferInfo(mfllCoreVersion, sel);

            if (sel.mIspHidlStage == ISP_HIDL_STAGE_REUEST_FRAME_FROM_CAMERA) {
                sel.mDecision.mZslEnabled = isZsdMode(pShotInfo->getMfbMode());
                sel.mDecision.mZslPolicy.mPolicy = v3::pipeline::policy::eZslPolicy_AfState
                                                | v3::pipeline::policy::eZslPolicy_ContinuousFrame
                                                | v3::pipeline::policy::eZslPolicy_ZeroShutterDelay;
                sel.mDecision.mZslPolicy.mTimeouts = 1000;
            }
        }

        sel.mIBufferFull.setRequired(MTRUE).setAlignment(mYuvAlign.w, mYuvAlign.h);
        sel.mIBufferSpecified.setRequired(MTRUE).setAlignment(mQYuvAlign.w, mQYuvAlign.h);
        sel.mIMetadataDynamic.setRequired(MTRUE);
        sel.mIMetadataApp.setRequired(MTRUE);
        sel.mIMetadataHal.setRequired(MTRUE);


        // Without control metadata, it's no need to append additional metadata
        // Use default mfnr setting
        if (sel.mIMetadataApp.getControl() != NULL) {
            //per frame
            {
                MetadataPtr pAppAddtional = make_shared<IMetadata>();
                MetadataPtr pHalAddtional = make_shared<IMetadata>();

                IMetadata* pAppMeta = pAppAddtional.get();
                IMetadata* pHalMeta = pHalAddtional.get();

                // clone partial control metadata to additional metadata
                {
                    IMetadata* pAppMetaControl = sel.mIMetadataApp.getControl().get();
                    // flash mode
                    {
                        MUINT8 flashMode = MTK_FLASH_MODE_OFF;
                        if ( !IMetadata::getEntry<MUINT8>(pAppMetaControl, MTK_FLASH_MODE, flashMode) ) {
                            MY_LOGW("%s: cannot retrieve MTK_FLASH_MODE from APP control metadata, assume "\
                                    "it to MTK_FLASH_MODE_OFF", __FUNCTION__);
                        }
                        IMetadata::setEntry<MUINT8>(pAppMeta, MTK_FLASH_MODE, flashMode);
                    }
                }

                // update unique key
                IMetadata::setEntry<MINT32>(pHalMeta, MTK_PIPELINE_UNIQUE_KEY, getUniqueKey());
                IMetadata::setEntry<MINT32>(pHalMeta, MTK_FEATURE_MFNR_NVRAM_DECISION_ISO, pShotInfo->getFinalIso());

                if (sel.mIspHidlStage == ISP_HIDL_STAGE_REUEST_FRAME_FROM_CAMERA) {
                    IMetadata::setEntry<MUINT8>(pHalMeta, MTK_ISP_P2_TUNING_UPDATE_MODE, 0);
                    if (HidlQueryIndex != -1) {
                        MINT32 queryIdx = getQueryIndexFromRecommendedKey(HidlQueryIndex);
                        MY_LOGD("queryIdx:%d", queryIdx);
                        IMetadata::setEntry<MINT32>(pHalMeta, MTK_FEATURE_MFNR_NVRAM_QUERY_INDEX, queryIdx);
                    }
                    else {
                        MY_LOGD("fail to get tuning index, use %d from shotInfo instead", pShotInfo->getNvramIndex());
                        IMetadata::setEntry<MINT32>(pHalMeta, MTK_FEATURE_MFNR_NVRAM_QUERY_INDEX, pShotInfo->getNvramIndex());
                    }
                    #if (SUPPORT_YUV_BSS == 0)
                    /* MTK_FEATURE_BSS_SELECTED_FRAME_COUNT for raw domain bss */
                    IMetadata::setEntry<MINT32>(pHalMeta, MTK_FEATURE_BSS_SELECTED_FRAME_COUNT, SelCount);
                    IMetadata::setEntry<MBOOL>(pHalMeta, MTK_FEATURE_BSS_DOWNSAMPLE, MTRUE);
                    IMetadata::setEntry<MUINT8>(pHalMeta, MTK_FEATURE_PACK_RRZO, 1);
                    IMetadata::setEntry<MINT32>(pHalMeta, MTK_FEATURE_BSS_PROCESS, MTK_FEATURE_BSS_PROCESS_ENABLE);
                    IMetadata::setEntry<MBOOL>(pHalMeta, MTK_FEATURE_BSS_REORDER, MTRUE);
                    #endif
#ifdef ISP_HIDL_TEMP_SOL_WITHOUT_TUNING_INDEX_HINT
                    IMetadata::setEntry<MINT64>(pHalMeta, MTK_FEATURE_MFNR_TUNING_INDEX_HINT, m_recommendedKey);
#endif
                }
                else {
                    IMetadata::setEntry<MINT32>(pHalMeta, MTK_FEATURE_MFNR_NVRAM_QUERY_INDEX, pShotInfo->getNvramIndex());
                    IMetadata::setEntry<MUINT8>(pHalMeta, MTK_ISP_P2_TUNING_UPDATE_MODE, SelIndex?2:0);
                    IMetadata::setEntry<MINT32>(pHalMeta, MTK_FEATURE_BSS_SELECTED_FRAME_COUNT, pShotInfo->getBlendNum());
                    IMetadata::setEntry<MINT32>(pHalMeta, MTK_FEATURE_BSS_PROCESS, MTK_FEATURE_BSS_PROCESS_ENABLE);
                    IMetadata::setEntry<MBOOL>(pHalMeta, MTK_FEATURE_BSS_REORDER, MTRUE);
                }
                IMetadata::setEntry<MINT32>(pHalMeta, MTK_FEATURE_MFNR_FINAL_EXP, pShotInfo->getFinalShutterTime());
                IMetadata::setEntry<MINT32>(pHalMeta, MTK_FEATURE_MFNR_OPEN_ID, mOpenId);

                updatePerFrameMetadata(pShotInfo.get(), pAppMeta, pHalMeta, (SelIndex+1 == SelCount));

                sel.mIMetadataApp.setAddtional(pAppAddtional);
                sel.mIMetadataHal.setAddtional(pHalAddtional);
            }

            //dummy frame
            {
                MetadataPtr pAppDummy = make_shared<IMetadata>();
                MetadataPtr pHalDummy = make_shared<IMetadata>();

                IMetadata* pAppMeta = pAppDummy.get();
                IMetadata* pHalMeta = pHalDummy.get();

                //first frame
                if (SelIndex == 0) {
                    sel.mFrontDummy = pShotInfo->getDummyFrameNum();
                    if (pShotInfo->getIsFlashOn()) {
                        IMetadata::setEntry<MBOOL>(pHalMeta, MTK_3A_DUMMY_BEFORE_REQUEST_FRAME, 1);
                    }
                    // need pure raw for MFNR flow
                    IMetadata::setEntry<MINT32>(pHalMeta, MTK_P1NODE_RAW_TYPE, 1);
                }

                //last frame
                if (SelIndex+1 == SelCount) {
                    sel.mPostDummy = pShotInfo->getDelayFrameNum();
                    IMetadata::setEntry<MBOOL>(pHalMeta, MTK_3A_DUMMY_AFTER_REQUEST_FRAME, 1);
                    IMetadata::setEntry<MINT32>(pHalMeta, MTK_P1NODE_RAW_TYPE, 1);
                }

                sel.mIMetadataApp.setDummy(pAppDummy);
                sel.mIMetadataHal.setDummy(pHalDummy);
            }
        }

        if (SelIndex == 0) {
            switch (mfllCoreVersion) {
                case MFLL_MAKE_REVISION(1, 4, 0):
                    sel.mOBufferFull
                        .setRequired(MTRUE)
                        .addAcceptedFormat(eImgFmt_YUY2) // YUY2 first
                        .addAcceptedFormat(eImgFmt_NV21)
                        .addAcceptedFormat(eImgFmt_NV12)
                        .addAcceptedFormat(eImgFmt_I420)
                        .addAcceptedSize(eImgSize_Full);
                    break;
                case MFLL_MAKE_REVISION(2, 0, 0):
                    sel.mOBufferFull
                        .setRequired(MTRUE)
                        .addAcceptedFormat(eImgFmt_YUY2) // YUY2 first
                        .addAcceptedFormat(eImgFmt_NV12)
                        .addAcceptedFormat(eImgFmt_I420)
                        .addAcceptedSize(eImgSize_Full);
                    break;
                case MFLL_MAKE_REVISION(2, 1, 0):
                    sel.mOBufferFull
                        .setRequired(MTRUE)
                        .addAcceptedFormat(eImgFmt_YUY2) // YUY2 first
                        .addAcceptedFormat(eImgFmt_NV21)
                        .addAcceptedFormat(eImgFmt_NV12)
                        .addAcceptedFormat(eImgFmt_I420)
                        .addAcceptedSize(eImgSize_Full);
                    break;
                case MFLL_MAKE_REVISION(2, 5, 0):
                    sel.mOBufferFull
                        .setRequired(MTRUE)
                        .addAcceptedFormat(eImgFmt_NV12) // NV12 first
                        .addAcceptedFormat(eImgFmt_YUY2)
                        .addAcceptedFormat(eImgFmt_NV21)
                        .addAcceptedFormat(eImgFmt_I420)
                        .addAcceptedSize(eImgSize_Full);
                    break;
                case MFLL_MAKE_REVISION(3, 0, 0):
                    if (mSupport10BitOutput && sel.mState.mMultiCamFeatureMode != MTK_MULTI_CAM_FEATURE_MODE_VSDOF) // Normal MFNR
                        sel.mOBufferFull.addAcceptedFormat(eImgFmt_MTK_YUV_P010);
                    sel.mOBufferFull
                        .setRequired(MTRUE)
                        //.addAcceptedFormat(eImgFmt_MTK_YUV_P010) // YUV 10 bit Packed first
                        .addAcceptedFormat(eImgFmt_NV12) // NV12 first
                        .addAcceptedFormat(eImgFmt_YUY2)
                        .addAcceptedFormat(eImgFmt_NV21)
                        .addAcceptedFormat(eImgFmt_I420)
                        .addAcceptedSize(eImgSize_Full);
                    break;
                default:
                    removeShotInfo(getUniqueKey());
                    MY_LOGE("MFNR Provider is not support for mfnrcore(%s)", pShotInfo->getMfnrCoreVersionString().c_str());
                    return BAD_VALUE;
            }

            sel.mOMetadataApp.setRequired(MTRUE);
            sel.mOMetadataHal.setRequired(MTRUE);

        } else {
            sel.mOBufferFull.setRequired(MFALSE);
            sel.mOMetadataApp.setRequired(MFALSE);
            sel.mOMetadataHal.setRequired(MFALSE);
        }

        if(SelIndex+1 == SelCount && sel.mIspHidlStage == ISP_HIDL_STAGE_REUEST_FRAME_FROM_CAMERA)
        {

#ifdef ISP_HIDL_TEMP_SOL_WITHOUT_TUNING_INDEX_HINT
            MY_LOGD("Last selection (%d/%d) of ISP_HIDL_CAMERA, move ShotInfo to fleeting queue", SelIndex, SelCount);
            //store shot instance to mRecommendQueue
            {
                std::lock_guard<decltype(mShotInfoMx)> lk(mShotInfoMx);

                RecommendShooter* shooter = mRecommendQueue.editInfo(__CALLERNAME__, m_recommendedKey);

                if (CC_LIKELY(shooter != nullptr))
                    shooter->shotInstance = pShotInfo;
                else
                    MY_LOGW("Reommended shot is not kept.");

                mRecommendQueue.publishInfo(__CALLERNAME__, shooter);
            }
            removeShotInfo(getUniqueKey());
#else
            MY_LOGD("Last selection (%d/%d) of ISP_HIDL_CAMERA, remove ShotInfo", SelIndex, SelCount);
            removeShotInfo(getUniqueKey());
#endif
#ifdef __DEBUG
            dumpShotInfo();
#endif
        }

        return OK;
    };

    virtual void init()
    {
        FUNCTION_SCOPE;
        //nothing to do for MFNR
    };

    virtual MERROR queryRecommendedFrameSetting(RecommendedParam& param)
    {
        FUNCTION_SCOPE;

#ifdef ISP_HIDL_TEMP_SOL_WITHOUT_TUNING_INDEX_HINT
        MY_LOGW("%s: this function is not support due to temp solution is work.", __FUNCTION__);
#endif

        MINT32 hint = 0;
        if (!IMetadata::getEntry<MINT32>(param.pAppMetadata, MTK_CONTROL_CAPTURE_HINT_FOR_ISP_TUNING, hint)) {
            MY_LOGE("cannot get MTK_CONTROL_CAPTURE_HINT_FOR_ISP_TUNING");
            return android::BAD_VALUE;
        }
        if (hint != MTK_CONTROL_CAPTURE_HINT_FOR_ISP_TUNING_MFNR) {
            MY_LOGD("tining hint is NOT MTK_CONTROL_CAPTURE_HINT_FOR_ISP_TUNING_MFNR");
            return android::BAD_VALUE;
        }

        MUINT8 AEmode = MTK_CONTROL_AE_MODE_OFF;
        MINT32 Sensor_Sensitivity = 0;
        MINT64 Sensor_Exposure_Time = 0;
        MINT32 Cam_ID = 0;
        if (!IMetadata::getEntry<MUINT8>(param.pAppMetadata, MTK_CONTROL_AE_MODE, AEmode)) {
            MY_LOGW("cannot get MTK_CONTROL_AE_MODE, use default mode off");
        } else {
            MY_LOGD("MTK_CONTROL_AE_MODE: %d", AEmode);
        }

        if (!IMetadata::getEntry<MINT32>(param.pAppMetadata, MTK_SENSOR_SENSITIVITY, Sensor_Sensitivity)) {
            MY_LOGW("cannot get MTK_SENSOR_SENSITIVITY");
        } else {
            MY_LOGD("MTK_SENSOR_SENSITIVITY: %d", Sensor_Sensitivity);
        }

        if (!IMetadata::getEntry<MINT64>(param.pAppMetadata, MTK_SENSOR_EXPOSURE_TIME, Sensor_Exposure_Time)) {
            MY_LOGW("cannot get MTK_SENSOR_EXPOSURE_TIME");
        } else {
            MY_LOGD("MTK_SENSOR_EXPOSURE_TIME: %" PRId64 "", Sensor_Exposure_Time);
        }

        if (!IMetadata::getEntry<MINT32>(param.pAppMetadata, MTK_MULTI_CAM_STREAMING_ID, Cam_ID)) {
            MY_LOGW("cannot get MTK_MULTI_CAM_STREAMING_ID");
        } else {
            MY_LOGD("MTK_MULTI_CAM_STREAMING_ID: %d", Cam_ID);
        }

        std::shared_ptr<MFNRShotInfo> pShotInfo;
        {
            std::lock_guard<decltype(mShotInfoMx)> lk(mShotInfoMx);
            pShotInfo = std::shared_ptr<MFNRShotInfo>(new MFNRShotInfo(static_cast<MINT32>(NSCam::Utils::TimeTool::getReadableTime()), Cam_ID, MfllMode_NormalMfll, Sensor_Sensitivity, Sensor_Exposure_Time, (AEmode == MTK_CONTROL_AE_MODE_ON_ALWAYS_FLASH), false));
            if (CC_UNLIKELY(pShotInfo.get() == nullptr)) {
                MY_LOGE("get MFNRShotInfo instance failed! cannot process MFNR shot.");
                return BAD_VALUE;
            }
        }

        pShotInfo->updateMfllStrategy();
        int CaptureNum = pShotInfo->getCaptureNum();
        MY_LOGD("CaptureNum: %d", CaptureNum);
        if (CaptureNum <= 1) {
            MY_LOGD("Do Not recommend to do MFNR");
            return BAD_VALUE;
        }

        int64_t recommendedKey = generateRecommendedKey(pShotInfo->getNvramIndex());
        //store shot instance to mRecommendQueue
        {
            std::lock_guard<decltype(mShotInfoMx)> lk(mShotInfoMx);

            RecommendShooter* shooter = mRecommendQueue.editInfo(__CALLERNAME__, recommendedKey);

            if (CC_LIKELY(shooter != nullptr))
                shooter->shotInstance = pShotInfo;
            else
                MY_LOGW("Reommended shot is not kept.");

            mRecommendQueue.publishInfo(__CALLERNAME__, shooter);
        }


        for(int i = 0; i < CaptureNum; i++) {
            std::shared_ptr<IMetadata> pAppAddtional = make_shared<IMetadata>();
            IMetadata* pAppMeta = pAppAddtional.get();
            IMetadata::setEntry<MINT32>(pAppMeta, MTK_CONTROL_CAPTURE_HINT_FOR_ISP_FRAME_INDEX, i);
            IMetadata::setEntry<MINT32>(pAppMeta, MTK_CONTROL_CAPTURE_HINT_FOR_ISP_FRAME_INDEX, CaptureNum);
            IMetadata::setEntry<MINT32>(pAppMeta, MTK_CONTROL_CAPTURE_HINT_FOR_ISP_TUNING, MTK_CONTROL_CAPTURE_HINT_FOR_ISP_TUNING_MFNR);
            IMetadata::setEntry<MINT64>(pAppMeta, MTK_CONTROL_CAPTURE_HINT_FOR_ISP_FRAME_TUNING_INDEX, recommendedKey);
            param.framesSetting.push_back(pAppAddtional);
        }
        MY_LOGD("RecommendedParam.framesSetting size: %zu, tuning index :%d", param.framesSetting.size(), pShotInfo->getNvramIndex());

        return OK;
    }

    virtual MERROR process(RequestPtr pRequest, RequestCallbackPtr pCallback)
    {
        FUNCTION_SCOPE;

        std::lock_guard<decltype(mProcessMx)> lk(mProcessMx);

        // restore callback function for abort API
        if(pCallback != nullptr) {
           m_callbackprt = pCallback;
        }

        //maybe need to keep a copy in member<sp>
        IMetadata* pAppMeta = pRequest->mIMetadataApp->acquire();
        IMetadata* pHalMeta = pRequest->mIMetadataHal->acquire();
        IMetadata* pHalMetaDynamic = pRequest->mIMetadataDynamic->acquire();
        MINT32 processUniqueKey = 0;

#if (SUPPORT_YUV_BSS==0)
        MINT8   RequestIndex = pRequest->mRequestBSSIndex;
        MINT8   RequestCount = pRequest->mRequestBSSCount;
#else
        MUINT8  RequestIndex = pRequest->mRequestIndex;
        MUINT8  RequestCount = pRequest->mRequestCount;
#endif

        // clean to zero
        IMetadata::setEntry<MUINT8>(pHalMeta, MTK_ISP_P2_TUNING_UPDATE_MODE, 0);

        if (!IMetadata::getEntry<MINT32>(pHalMeta, MTK_PIPELINE_UNIQUE_KEY, processUniqueKey)) {
            MY_LOGE("cannot get unique about MFNR capture");
            return BAD_VALUE;
        }

        //get MFNRShotInfo
        std::shared_ptr<MFNRShotInfo> pShotInfo = getShotInfo(processUniqueKey);
        if (CC_UNLIKELY(pShotInfo.get() == nullptr)) {
            removeShotInfo(processUniqueKey);
            MY_LOGE("get MFNRShotInfo instance failed! cannot process MFNR shot.");
            return BAD_VALUE;
        }

#if 1
        if (RequestIndex == 0) {
            MY_LOGE("TODO: please fix it. <initMfnrCore timing>");
            {
                std::lock_guard<std::mutex> __l(m_futureExeMx);
                auto t1 = std::async(std::launch::async, [this, pShotInfo]() {
                        MFLL_THREAD_NAME("initMfnrCore");
                        pShotInfo->initMfnrCore(); // init MFNR controller
                    });
                m_futureExe = std::shared_future<void>(std::move(t1));
            }
        }
#endif

        //wait initMfnrCore() done
        if (RequestIndex == 0) {
            pShotInfo->setRealBlendNum(RequestCount);
            pShotInfo->setMainFrameHalMetaIn(pRequest->mIMetadataHal->acquire());
            pShotInfo->setMainFrameHalMetaOut(pRequest->mOMetadataHal->acquire());
            std::shared_future<void> t1;
            {
                std::lock_guard<std::mutex> __l(m_futureExeMx);
                t1 = m_futureExe;
            }

            if (t1.valid())
                t1.wait();

            pShotInfo->execute();
        }

        /* create IMfllImageBuffer of Full/ Quarter YUV */
        sp<IMfllImageBuffer> mfllImgBuf_yuv_full = IMfllImageBuffer::createInstance("FullYuv");
        sp<IMfllImageBuffer> mfllImgBuf_yuv_quarter = IMfllImageBuffer::createInstance("QuarterYuv");
        sp<IMfllImageBuffer> mfllMixWorkingBuf = IMfllImageBuffer::createInstance("MixWorking");

        if (pRequest->mIBufferFull != nullptr) {
            IImageBuffer* pImgBuffer = pRequest->mIBufferFull->acquire();
            MY_LOGD("[IN] Full image VA: 0x%p, Size(%dx%d)", (void*)pImgBuffer->getBufVA(0), pImgBuffer->getImgSize().w, pImgBuffer->getImgSize().h);
            mfllImgBuf_yuv_full->setImageBuffer(pImgBuffer);
            mfllImgBuf_yuv_full->setAligned(mYuvAlign.w, mYuvAlign.h);
        }
        if (pRequest->mIBufferSpecified != nullptr) {
            IImageBuffer* pImgBuffer = pRequest->mIBufferSpecified->acquire();
            MY_LOGD("[IN] Quarter image VA: 0x%p, Size(%dx%d)", (void*)pImgBuffer->getBufVA(0), pImgBuffer->getImgSize().w, pImgBuffer->getImgSize().h);
            mfllImgBuf_yuv_quarter->setImageBuffer(pImgBuffer);
            mfllImgBuf_yuv_quarter->setAligned(mQYuvAlign.w, mQYuvAlign.h);
        }
        if (pRequest->mOBufferFull != nullptr) {
            IImageBuffer* pImgBuffer = pRequest->mOBufferFull->acquire();
            MY_LOGD("[OUT] Full image VA: 0x%p, Size(%dx%d)", (void*)pImgBuffer->getBufVA(0), pImgBuffer->getImgSize().w, pImgBuffer->getImgSize().h);
            mfllMixWorkingBuf->setImageBuffer(pImgBuffer);
            pShotInfo->setOutputBufToMfnrCore(mfllMixWorkingBuf);
        }

        if (pRequest->mIMetadataDynamic != nullptr) {
            IMetadata *meta = pRequest->mIMetadataDynamic->acquire();
            if (meta != NULL)
                MY_LOGD("[IN] Dynamic metadata count: %u", meta->count());
            else
                MY_LOGD("[IN] Dynamic metadata Empty");
        }

        MY_LOGD("collected request(%d/%d)",
                RequestIndex,
                RequestCount);

        if (CC_UNLIKELY( mvRequests.size() != RequestIndex ))
            MY_LOGE("Input sequence of requests from P2A is wrong");

        mvRequests.push_back(pRequest);

        MfllMotionVector mv = pShotInfo->calMotionVector(pHalMeta, RequestIndex);
        pShotInfo->addDataToMfnrCore(mfllImgBuf_yuv_full, mfllImgBuf_yuv_quarter, mv, pAppMeta, pHalMeta, pHalMetaDynamic, RequestIndex);

        if (RequestIndex == RequestCount - 1)
        {
            pShotInfo->waitExecution();
            MY_LOGD("have collected all requests");
            for (auto req : mvRequests) {
                MY_LOGD("callback request(%d/%d) %p",
                        RequestIndex,
                        RequestCount, pCallback.get());
                if (pCallback != nullptr) {
                    pCallback->onCompleted(req, 0);
                }
            }
            mvRequests.clear();
            removeShotInfo(processUniqueKey);
#ifdef __DEBUG
            dumpShotInfo();
#endif
        }

        return 0;
    };

    virtual void abort(vector<RequestPtr>& pRequests)
    {
        FUNCTION_SCOPE;

#if (SUPPORT_YUV_BSS==0)
        MY_LOGD("not support abort() for RAW BSS");
#else
        std::lock_guard<decltype(mProcessMx)> lk(mProcessMx);

        bool bAbort = false;
        IMetadata *pHalMeta;
        MINT32 processUniqueKey = 0;
        std::shared_ptr<MFNRShotInfo> pShotInfo;

        for(auto req:pRequests) {
            bAbort = false;
            pHalMeta = req->mIMetadataHal->acquire();
            if(!IMetadata::getEntry<MINT32>(pHalMeta, MTK_PIPELINE_UNIQUE_KEY, processUniqueKey)){
                MY_LOGW("cannot get unique about MFNR capture");
            }
            pShotInfo = getShotInfo(processUniqueKey);

            /* if MFNR plugin receives abort, it will cancel mfll immediately */
            if(pShotInfo != nullptr){
               if (!pShotInfo->getDoCancelStatus()) {
                    pShotInfo->doCancel();
                    pShotInfo->waitExecution(); //wait mfll done
                    pShotInfo->setDoCancelStatus(true);
               }
            }else{
               MY_LOGW("pShotInfo is null");
            }

            if(m_callbackprt != nullptr){
               MY_LOGD("m_callbackprt is %p", m_callbackprt.get());
               /*MFNR plugin callback request to MultiFrameNode */
               for (Vector<RequestPtr>::iterator it = mvRequests.begin() ; it != mvRequests.end(); it++){
                    if((*it) == req){
                        mvRequests.erase(it);
                        m_callbackprt->onAborted(req);
                        bAbort = true;
                        break;
                    }
               }
            }else{
               MY_LOGW("callbackptr is null");
            }

            if (!bAbort){
               MY_LOGW("Desire abort request[%d] is not found", req->mRequestIndex);
            }

            if(mvRequests.empty()) {
               removeShotInfo(processUniqueKey);
               MY_LOGD("abort() cleans all the requests");
            }else {
               MY_LOGW("abort() does not clean all the requests");
            }
        }
#endif
    };

    virtual void uninit()
    {
        FUNCTION_SCOPE;
        //nothing to do for MFNR
    };

    virtual ~MFNRProviderImpl()
    {
        FUNCTION_SCOPE;
        removeAllShotInfo();

#ifdef __DEBUG
        dumpShotInfo();
#endif
    };

private:
    MINT32 getUniqueKey()
    {
        FUNCTION_SCOPE;

        std::lock_guard<decltype(mShotInfoMx)> lk(mShotInfoMx);
        return mUniqueKey;
    }

    void setUniqueKey(MINT32 key)
    {
        FUNCTION_SCOPE;

        std::lock_guard<decltype(mShotInfoMx)> lk(mShotInfoMx);
        mUniqueKey = key;
    }

    void createShotInfo(IspHidlStage HidlStage, MINT32 sensorId)
    {
        FUNCTION_SCOPE;

        std::lock_guard<decltype(mShotInfoMx)> lk(mShotInfoMx);

        if (HidlStage != ISP_HIDL_STAGE_PROCESS_FRAME_IN_ISP_HIDL || mShots[mUniqueKey] == nullptr)
            mShots[mUniqueKey] = std::shared_ptr<MFNRShotInfo>(new MFNRShotInfo(mUniqueKey, sensorId, mMfbMode, mRealIso, mShutterTime, mFlashOn, (HidlStage==ISP_HIDL_STAGE_PROCESS_FRAME_IN_ISP_HIDL)));

        MY_LOGD("Create ShotInfos: %d sensorId: %d", mUniqueKey, sensorId);
    };

    void removeShotInfo(MINT32 key)
    {
        FUNCTION_SCOPE;

        std::lock_guard<decltype(mShotInfoMx)> lk(mShotInfoMx);
        if (mShots.count(key)) {
            mShots[key] = nullptr;
            mShots.erase(key);

            MY_LOGD("Remvoe ShotInfos: %d", key);
        }
    };

    void removeAllShotInfo()
    {
        FUNCTION_SCOPE;

        std::lock_guard<decltype(mShotInfoMx)> lk(mShotInfoMx);
        for (auto it = mShots.begin(); it != mShots.end(); ++it) {
            it->second = nullptr;
            MY_LOGD("Remvoe ShotInfos: %d", it->first);
        }
        mShots.clear();
    };

    std::shared_ptr<MFNRShotInfo> getShotInfo(MINT32 key)
    {
        FUNCTION_SCOPE;

        std::lock_guard<decltype(mShotInfoMx)> lk(mShotInfoMx);
        return mShots[key];
    };

    void dumpShotInfo()
    {
        FUNCTION_SCOPE;

        std::lock_guard<decltype(mShotInfoMx)> lk(mShotInfoMx);

        std::string usage;
        for ( auto it = mShots.begin(); it != mShots.end(); ++it )
            usage += " " + std::to_string(it->first);

        MY_LOGD("All ShotInfos:%s", usage.c_str());
    };

    void updatePerFrameMetadata(const MFNRShotInfo* pShotInfo, IMetadata* pAppMeta, IMetadata* pHalMeta, bool bLastFrame)
    {
        FUNCTION_SCOPE;

        ASSERT(pShotInfo, "updatePerFrameMetadata::pShotInfo is null.");
        ASSERT(pAppMeta,  "updatePerFrameMetadata::pAppMeta is null.");
        ASSERT(pHalMeta,  "updatePerFrameMetadata::pHalMeta is null.");

        // update_app_setting(pAppMeta, pShotInfo);
        {
            bool bNeedManualAe = [&](){
                // if using ZHDR, cannot apply manual AE
                if (mfll::isZhdrMode(pShotInfo->getMfbMode()))
                    return false;
                // if using FLASH, cannot apply menual AE
                if (pShotInfo->getIsFlashOn())
                    return false;
                // if uisng MFNR (since MFNR uses ZSD buffers), we don't need manual AE
                // but if MFNR using non-ZSD flow, we need to apply manual AE
                if (mfll::isMfllMode(pShotInfo->getMfbMode())) {
                    if (mfll::isZsdMode(pShotInfo->getMfbMode()))
                        return false;
                    else
                        return true;
                }
                /// otherwise, we need it
                return true;
            }();

            if (bNeedManualAe) {
                //manualAE cannot apply flash
                {
                    MUINT8 flashMode = MTK_FLASH_MODE_OFF;
                    if ( !IMetadata::getEntry<MUINT8>(pAppMeta, MTK_FLASH_MODE, flashMode) ) {
                        MY_LOGW("%s: cannot retrieve MTK_FLASH_MODE from APP additional metadata, assume "\
                                "it to MTK_FLASH_MODE_OFF", __FUNCTION__);
                    }
                    if (flashMode != MTK_FLASH_MODE_TORCH)
                        IMetadata::setEntry<MUINT8>(pAppMeta, MTK_FLASH_MODE, MTK_FLASH_MODE_OFF);
                }
                //
                IMetadata::setEntry<MUINT8>(pAppMeta, MTK_CONTROL_AE_MODE, MTK_CONTROL_AE_MODE_OFF);
                IMetadata::setEntry<MINT32>(pAppMeta, MTK_SENSOR_SENSITIVITY, pShotInfo->getFinalIso());
                IMetadata::setEntry<MINT64>(pAppMeta, MTK_SENSOR_EXPOSURE_TIME, pShotInfo->getFinalShutterTime() * 1000); // ms->us
                IMetadata::setEntry<MUINT8>(pAppMeta, MTK_CONTROL_AWB_LOCK, MTRUE);
            }

            IMetadata::setEntry<MBOOL>(pHalMeta, MTK_P1NODE_ISNEED_GMV, MTRUE);
        }

        {
            MUINT8 bOriFocusPause  = 0;
            if ( !IMetadata::getEntry<MUINT8>(pHalMeta, MTK_FOCUS_PAUSE, bOriFocusPause) ) {
                MY_LOGW("%s: cannot retrieve MTK_FOCUS_PAUSE from HAL metadata, assume "\
                        "it to 0", __FUNCTION__);
            }

            // update ISP profile for zHDR (single frame default)
            if (mfll::isZhdrMode(pShotInfo->getMfbMode())) {
                MY_LOGE("Zhdr is not support in MFNR plugin");
                bool isAutoHDR = mfll::isAutoHdr(pShotInfo->getMfbMode());
                MUINT sensorMode = SENSOR_SCENARIO_ID_NORMAL_CAPTURE;
                MSize sensorSize;
                NSCamHW::HwInfoHelper helper(pShotInfo->getOpenId());
                if (!helper.getSensorSize(sensorMode, sensorSize)) {
                    MY_LOGW("cannot get sensor size");
                }
                else {
                    // Prepare query Feature Shot ISP Profile
                    ProfileParam profileParam
                    {
                        sensorSize,
                        SENSOR_VHDR_MODE_ZVHDR, /*VHDR mode*/
                        sensorMode,
                        ProfileParam::FLAG_NONE,
                        ((isAutoHDR) ? (ProfileParam::FMASK_AUTO_HDR_ON) : (ProfileParam::FMASK_NONE)),
                    };

                    MUINT8 profile = 0;
                    if (FeatureProfileHelper::getShotProf(profile, profileParam))
                    {
                        MY_LOGD("ISP profile is set(%u)", profile);
                        // modify hal control metadata for zHDR
                        IMetadata::setEntry<MUINT8>(
                            pHalMeta , MTK_3A_ISP_PROFILE , profile);
                        IMetadata::setEntry<MUINT8>(
                            pHalMeta, MTK_3A_AE_CAP_SINGLE_FRAME_HDR, 1);
                    }
                    else
                    {
                        MY_LOGW("ISP profile is not set(%u)", profile);
                    }
                }
            }

            // pause AF for (N - 1) frames and resume for the last frame
            IMetadata::setEntry<MUINT8>(pHalMeta, MTK_FOCUS_PAUSE, bLastFrame ? bOriFocusPause : 1);
            IMetadata::setEntry<MUINT8>(pHalMeta, MTK_HAL_REQUEST_REQUIRE_EXIF, 1);
            IMetadata::setEntry<MUINT8>(pHalMeta, MTK_HAL_REQUEST_DUMP_EXIF, 1);

            // need pure raw for MFNR flow
            IMetadata::setEntry<MINT32>(pHalMeta, MTK_P1NODE_RAW_TYPE, 1);

            MINT32 customHintInMFNR = mfll::getCustomHint(pShotInfo->getMfbMode());
            MINT32 customHintInHal;
            // check customHint in metadata for customize feature
            if ( !IMetadata::getEntry<MINT32>(pHalMeta, MTK_PLUGIN_CUSTOM_HINT, customHintInHal) ) {
                MY_LOGW("%s: cannot retrieve MTK_PLUGIN_CUSTOM_HINT from HAL metadata, assume "\
                        "it to %d", __FUNCTION__, customHintInMFNR);
                IMetadata::setEntry<MINT32>( pHalMeta, MTK_PLUGIN_CUSTOM_HINT, customHintInMFNR);
            }
            else if (customHintInMFNR != customHintInHal) { // query and check the result
                MY_LOGW("%s: MTK_PLUGIN_CUSTOM_HINT in MFNR(%d) and Hal(%d) setting are different"
                        , __FUNCTION__, customHintInMFNR, customHintInHal);
            }
        }

        //AE Flare Enable, due to MTK_CONTROL_AE_MODE_OFF will disable AE_Flare
        // MFNR must set MTK_3A_FLARE_IN_MANUAL_CTRL_ENABLE to enable AE_Flare
        // TODO: hal3a need to implement this command for each platform.
        {
            IMetadata::setEntry<MBOOL>(pHalMeta, MTK_3A_FLARE_IN_MANUAL_CTRL_ENABLE, MTRUE);
        }
    }

    MfllMode updateMfbMode(IMetadata* pAppMeta, MBOOL isZSDMode) {

        IMetadata::IEntry const eIspMFNR       = pAppMeta->entryFor(MTK_CONTROL_CAPTURE_HINT_FOR_ISP_TUNING);
        IMetadata::IEntry const eMfb           = pAppMeta->entryFor(MTK_MFNR_FEATURE_MFB_MODE);
        IMetadata::IEntry const eAis           = pAppMeta->entryFor(MTK_MFNR_FEATURE_AIS_MODE);
        IMetadata::IEntry const eSceneMode     = pAppMeta->entryFor(MTK_CONTROL_SCENE_MODE);
        if (!eIspMFNR.isEmpty()){
            MY_LOGD("hint for ISP tuning mode: %d", eIspMFNR.itemAt(0, Type2Type<MINT32>()));
        } else {
            MY_LOGD("MTK_CONTROL_CAPTURE_HINT_FOR_ISP_TUNING is empty");
        }

        int mfbMode = [&]()
        {
            if ( ! eIspMFNR.isEmpty() && eIspMFNR.itemAt(0, Type2Type<MINT32>()) != MTK_CONTROL_CAPTURE_HINT_FOR_ISP_TUNING_MFNR){
                MY_LOGD("HINT_FOR_ISP_TUNING has value but is NOT MFNR, set mfllMode = MfllMode_Off");
                return MTK_MFB_MODE_OFF;
            }
            // If MTK specific parameter AIS on or MFB mode is AIS, set to AIS mode (2)
            else if (( ! eMfb.isEmpty() && eMfb.itemAt(0, Type2Type<MINT32>()) == MTK_MFNR_FEATURE_MFB_AIS)  ||
                     ( ! eAis.isEmpty() && eAis.itemAt(0, Type2Type<MINT32>()) == MTK_MFNR_FEATURE_AIS_ON)) {
                return MTK_MFB_MODE_AIS;
            }
            // Scene mode is Night or MFB mode is MFLL, set to MFLL mode (1)
            else if (( ! eMfb.isEmpty() && eMfb.itemAt(0, Type2Type<MINT32>()) == MTK_MFNR_FEATURE_MFB_MFLL) ||
                     ( ! eSceneMode.isEmpty() && eSceneMode.itemAt(0, Type2Type<MUINT8>()) == MTK_CONTROL_SCENE_MODE_NIGHT)) {
                return MTK_MFB_MODE_MFLL;
            }
            else if (( ! eMfb.isEmpty() && eMfb.itemAt(0, Type2Type<MINT32>()) == MTK_MFNR_FEATURE_MFB_AUTO)) {
                MY_LOGD("in MTK_MFNR_FEATURE_MFB_AUTO");
#ifdef CUST_MFLL_AUTO_MODE
                static_assert( ((CUST_MFLL_AUTO_MODE >= MTK_MFB_MODE_OFF)&&(CUST_MFLL_AUTO_MODE < MTK_MFB_MODE_NUM)),
                               "CUST_MFLL_AUTO_MODE is invalid in custom/feature/mfnr/camera_custom_mfll.h" );

                MY_LOGD("CUST_MFLL_AUTO_MODE:%d", CUST_MFLL_AUTO_MODE);
                return static_cast<mtk_platform_metadata_enum_mfb_mode>(CUST_MFLL_AUTO_MODE);
#else
#error "CUST_MFLL_AUTO_MODE is no defined in custom/feature/mfnr/camera_custom_mfll.h"
#endif
            }
            else if ( ! eIspMFNR.isEmpty() && eIspMFNR.itemAt(0, Type2Type<MINT32>()) == MTK_CONTROL_CAPTURE_HINT_FOR_ISP_TUNING_MFNR) {
                MY_LOGD("return MTK_MFB_MODE_MFLL");
                return MTK_MFB_MODE_MFLL;
            }
            // Otherwise, set MFB off (0)
            else {
                if( ! eIspMFNR.isEmpty())
                    MY_LOGD("eIspMFNRis not empty, value:%d", eIspMFNR.itemAt(0, Type2Type<MINT32>()));
                return MTK_MFB_MODE_OFF;
            }
        }();
        MY_LOGD("mfbMode value:%d", mfbMode);

        int bForceMfb = MfllProperty::isForceMfll();
        if (CC_UNLIKELY( bForceMfb == 0 )) {
            MY_LOGD("Force disable MFNR");
            mfbMode = MTK_MFB_MODE_OFF;
        }
        else if (CC_UNLIKELY(bForceMfb > 0)) {
            MY_LOGD("Force MFNR (bForceMfb:%d)", bForceMfb);
            mfbMode = bForceMfb;
        }

        MfllMode mfllMode = MfllMode_NormalMfll;

        // 0: Not specific, 1: MFNR, 2: AIS
        switch (mfbMode) {
            case MTK_MFB_MODE_MFLL:
                mfllMode = (CC_LIKELY(isZSDMode) ? MfllMode_ZsdMfll : MfllMode_NormalMfll);
                break;
            case MTK_MFB_MODE_AIS:
                mfllMode = MfllMode_NormalAis;//(CC_LIKELY(isZSDMode) ? MfllMode_ZsdAis : MfllMode_NormalAis);
                break;
            case MTK_MFB_MODE_OFF:
                mfllMode = MfllMode_Off;
                break;
            default:
                mfllMode = (CC_LIKELY(isZSDMode) ? MfllMode_ZsdMfll : MfllMode_NormalMfll);
                break;
        }

        MY_LOGD("MfllMode(0x%X), mfbMode(%d), isZsd(%d)",
                mfllMode, mfbMode, isZSDMode);

        return mfllMode;
    }

    bool updateInputBufferInfo(MINT32 mfllCoreVersion, Selection& sel)
    {
        std::shared_ptr<MFNRShotInfo> pShotInfo = getShotInfo(getUniqueKey());

        if (pShotInfo == nullptr)
            return false;

        switch (mfllCoreVersion) {
            case MFLL_MAKE_REVISION(1, 4, 0):
                if (CC_LIKELY( pShotInfo->getIsFullSizeMc() )) {
                    sel.mIBufferFull.addAcceptedFormat(eImgFmt_I422);
                    sel.mIBufferSpecified.addAcceptedFormat(eImgFmt_Y8);
                } else {
                    sel.mIBufferFull.addAcceptedFormat(eImgFmt_BAYER10);
                    sel.mIBufferSpecified.addAcceptedFormat(eImgFmt_YUY2);
                }
                //
                sel.mIBufferFull.addAcceptedSize(eImgSize_Full);
                sel.mIBufferSpecified.addAcceptedSize(eImgSize_Quarter);
                mYuvAlign  = MSize(2, 2);
                mQYuvAlign = MSize(2, 2);
                break;

            case MFLL_MAKE_REVISION(2, 0, 0):
                if (CC_LIKELY( pShotInfo->getIsFullSizeMc() )) {
                    sel.mIBufferFull.addAcceptedFormat(eImgFmt_I422);
                    sel.mIBufferSpecified.addAcceptedFormat(eImgFmt_Y8);
                } else {
                    sel.mIBufferFull.addAcceptedFormat(eImgFmt_YUY2);
                    sel.mIBufferSpecified.addAcceptedFormat(eImgFmt_I422);
                }
                //
                sel.mIBufferFull.addAcceptedSize(eImgSize_Full);
                sel.mIBufferSpecified.addAcceptedSize(eImgSize_Specified).setSpecifiedSize(
                    calcDownScaleSize(pShotInfo->getSizeSrc(), 1, 2) );
                mYuvAlign  = MSize(16, 16);
                mQYuvAlign = MSize(16, 16);
                break;

            case MFLL_MAKE_REVISION(2, 1, 0):
                if (CC_LIKELY( pShotInfo->getIsFullSizeMc() )) {
                    sel.mIBufferFull.addAcceptedFormat(eImgFmt_I422);
                    sel.mIBufferSpecified.addAcceptedFormat(eImgFmt_Y8);
                } else {
                    sel.mIBufferFull.addAcceptedFormat(eImgFmt_YUY2);
                    sel.mIBufferSpecified.addAcceptedFormat(eImgFmt_I422);
                }
                //
                sel.mIBufferFull.addAcceptedSize(eImgSize_Full);
                sel.mIBufferSpecified.addAcceptedSize(eImgSize_Quarter);
                mYuvAlign  = MSize(16, 16);
                mQYuvAlign = MSize(16, 16);
                break;

            case MFLL_MAKE_REVISION(2, 5, 0):
                sel.mIBufferFull.addAcceptedFormat(eImgFmt_NV12);
                sel.mIBufferSpecified.addAcceptedFormat(eImgFmt_Y8);
                //
                if (CC_LIKELY( !pShotInfo->getEnableDownscale() )) {
                    sel.mIBufferFull.addAcceptedSize(eImgSize_Full);
                } else {
                    if (CC_UNLIKELY( pShotInfo->getDownscaleRatio() < 1 )) {
                        MY_LOGE("MFNR Provider is not support due to error DownScaleFactor %d", pShotInfo->getDownscaleRatio());
                        return false;
                    }
                    sel.mIBufferFull.addAcceptedSize(eImgSize_Specified).setSpecifiedSize(
                        calcDownScaleSize(pShotInfo->getSizeSrc(), 1,  pShotInfo->getDownscaleRatio()) );
                }
                sel.mIBufferSpecified.addAcceptedSize(eImgSize_Specified).setSpecifiedSize(
                    calcDownScaleSize(pShotInfo->getSizeSrc(), 1, 4) );
                mYuvAlign  = MSize(16, 16);
                mQYuvAlign = MSize(16, 16);
                break;

            case MFLL_MAKE_REVISION(3, 0, 0):
                if (sel.mState.mMultiCamFeatureMode != MTK_MULTI_CAM_FEATURE_MODE_VSDOF) { // Normal MFNR
                    sel.mIBufferFull.addAcceptedFormat(eImgFmt_MTK_YUV_P010);
                    sel.mIBufferSpecified.addAcceptedFormat(eImgFmt_NV12/*eImgFmt_Y8*/); //Due to postview issue
                } else {
                    MY_LOGE("TODO: please fix it. <MFNR3.0 should apply 10bit YUV>");
                    pShotInfo->setInputYuvFmt(InputYuvFmt_Nv12);
                    sel.mIBufferFull.addAcceptedFormat(eImgFmt_NV12);
                    sel.mIBufferSpecified.addAcceptedFormat(eImgFmt_Y8);
                }
                //
                if( sel.mIspHidlStage == ISP_HIDL_STAGE_REUEST_FRAME_FROM_CAMERA ) {
                    MSize fullSize = pShotInfo->getSizeSrc();
                    MSize quaterSize = calcDownScaleSize(fullSize, 1, (!pShotInfo->getEnableDownscale()?4:2));
                    sel.mIBufferFull.addAcceptedSize(eImgSize_Specified).setSpecifiedSize(fullSize);
                    sel.mIBufferSpecified.addAcceptedSize(eImgSize_Specified).setSpecifiedSize(quaterSize);
                }
                else if (CC_UNLIKELY( pShotInfo->getDownscaleDividend() < 1 || pShotInfo->getDownscaleDividend() > pShotInfo->getDownscaleDivisor() )) {
                    MY_LOGE("MFNR Provider is not support due to error DownScaleFactor is %d/%d", pShotInfo->getDownscaleDividend(), pShotInfo->getDownscaleDivisor());
                    return false;
                } else {
                    MSize fullSize = calcDownScaleSize(pShotInfo->getSizeSrc(), pShotInfo->getDownscaleDividend(), pShotInfo->getDownscaleDivisor());
                    MSize quaterSize = calcDownScaleSize(fullSize, 1, (!pShotInfo->getEnableDownscale()?4:2));
                    sel.mIBufferFull.addAcceptedSize(eImgSize_Specified).setSpecifiedSize(fullSize);
                    sel.mIBufferSpecified.addAcceptedSize(eImgSize_Specified).setSpecifiedSize(quaterSize);
                }
                mYuvAlign  = MSize(64, 16);
                mQYuvAlign = MSize(16, 16);
                break;

            default:
                MY_LOGE("MFNR Provider is not support for mfnrcore(%s)", pShotInfo->getMfnrCoreVersionString().c_str());
                return false;
        }

        return true;
    }

    MSize calcDownScaleSize(const MSize& m, int dividend, int divisor)
    {
        if (CC_LIKELY( divisor ))
            return MSize(m.w * dividend / divisor, m.h * dividend / divisor);

        MY_LOGW("%s: divisor is zero", __FUNCTION__);
        return m;
    }

    inline MINT64 generateRecommendedKey(MINT32 queryIndex)
    {
        std::lock_guard<decltype(mRecommendCountMx)> lk(mRecommendCountMx);

        mRecommendCount++;
        MINT64 ret = (MINT64(mRecommendCount) << 32) | (MINT64(queryIndex) & 0xFFFFFFFF);
        MY_LOGD("%s: 0x%x --> 0x%jx ", __FUNCTION__, queryIndex, ret);
        return ret;
    }

    inline MINT32 getQueryIndexFromRecommendedKey(MINT64 recommendKey)
    {
        MINT32 ret = MINT32(recommendKey & 0xFFFFFFFF);
        MY_LOGD("%s: 0x%jx --> 0x%x ", __FUNCTION__, recommendKey, ret);
        return ret;
    }


private:

    MINT32                          mUniqueKey;
    MINT32                          mOpenId;
    MINT32                          mSupport10BitOutput;
    MfllMode                        mMfbMode;
    MINT32                          mRealIso;
    MINT32                          mShutterTime;
    MBOOL                           mZSDMode;
    MBOOL                           mFlashOn;
    MSize                           mYuvAlign;
    MSize                           mQYuvAlign;

    Vector<RequestPtr>              mvRequests;
    std::mutex                      mShotInfoMx; // protect MFNRShotInfo
    std::mutex                      mProcessMx; // protect MFNRShotInfo
    std::shared_future<void>        m_futureExe;
    mutable std::mutex              m_futureExeMx;

    std::unordered_map<MUINT32, std::shared_ptr<MFNRShotInfo>>
                                    mShots;

    RequestCallbackPtr              m_callbackprt;

#ifdef ISP_HIDL_TEMP_SOL_WITHOUT_TUNING_INDEX_HINT
    int64_t                         m_recommendedKey;
#else
    // HIDL ISP shot instance
    struct RecommendShooter {
        std::shared_ptr<MFNRShotInfo> shotInstance;
        RecommendShooter () : shotInstance(nullptr) { };
        ~RecommendShooter () { shotInstance = nullptr; };
    };
    //
    MUINT32                         mRecommendCount;
    std::mutex                      mRecommendCountMx; // protect mRecommendCount
    NSCam::FleetingQueue<RecommendShooter, 5, 2> mRecommendQueue;
#endif
};

REGISTER_PLUGIN_PROVIDER(MultiFrame, MFNRProviderImpl);

