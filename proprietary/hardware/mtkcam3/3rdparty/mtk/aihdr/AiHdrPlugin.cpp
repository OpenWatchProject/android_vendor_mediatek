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
 * MediaTek Inc. (C) 2019. All rights reserved.
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
#define LOG_TAG "AiHdrPlugin"

//
#include <cfenv>
#include <cstdlib>
#include <atomic>
#include <sstream>
#include <memory>

#include <utils/Errors.h>
#include <utils/List.h>
#include <utils/RefBase.h>
#include <utils/String8.h>
#include <cutils/properties.h>
//
#include <mtkcam/aaa/IHal3A.h>  // setIsp, CaptureParam_T
#include <mtkcam/drv/IHalSensor.h>
#include <mtkcam/utils/imgbuf/IIonImageBufferHeap.h>
#include <mtkcam/utils/std/Format.h>
#include <mtkcam/utils/sys/MemoryInfo.h>
#include <mtkcam/utils/std/Log.h>
#include <mtkcam/utils/std/Time.h>
#include <mtkcam/utils/std/JobQueue.h>
#include <mtkcam/utils/metastore/IMetadataProvider.h>
#include <mtkcam/utils/metastore/ITemplateRequest.h>
#include <mtkcam/utils/metadata/client/mtk_metadata_tag.h>
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>
#include <mtkcam/utils/TuningUtils/FileDumpNamingRule.h>  // tuning file naming

//
#include <mtkcam3/feature/ainr/AinrUlog.h>
#include <mtkcam3/pipeline/hwnode/NodeId.h>
#include <mtkcam3/3rdparty/plugin/PipelinePlugin.h>
#include <mtkcam3/3rdparty/plugin/PipelinePluginType.h>

//
#include <isp_tuning/isp_tuning.h>
//
#include <sys/stat.h>  // mkdir
#include <sys/prctl.h>  // prctl set name

//
#include "AiHdrShot.h"

// shot count
static std::atomic<int> gShotCount = 0;

/******************************************************************************
*
******************************************************************************/
namespace NSCam {
namespace NSPipelinePlugin {

class AiHdrPlugin : public MultiFramePlugin::IProvider {
    typedef MultiFramePlugin::Property Property;
    typedef MultiFramePlugin::Selection Selection;
    typedef MultiFramePlugin::Request::Ptr RequestPtr;
    typedef MultiFramePlugin::RequestCallback::Ptr RequestCallbackPtr;

 public:
    virtual const Property& property() {
        CAM_ULOGM_APILIFE();
        static Property prop;
        static std::once_flag initFlag;
        std::call_once(initFlag, [this](Property &){
            ainrLogD("set up property");
            prop.mName              = "MTK AIHDR";
            prop.mFeatures          = MTK_FEATURE_AIHDR;
            prop.mThumbnailTiming   = eTiming_MDP;
            prop.mPriority          = ePriority_Highest;
            prop.mZsdBufferMaxNum   = 8;  // maximum frames requirement
            prop.mNeedRrzoBuffer    = MTRUE;  // rrzo requirement for BSS

        }, prop);

        return prop;
    }

    // if capture number is 4, "negotiate" would be called 4 times
    virtual MERROR negotiate(Selection& sel) {
        CAM_ULOGM_APILIFE();

        //
        mEnable = ::property_get_int32("vendor.debug.camera.aihdr.enable", -1);

        auto flashOn = sel.mState.mFlashFired;
        if (CC_UNLIKELY(flashOn == MTRUE)
                || CC_UNLIKELY(mEnable == 0)) {
            ainrLogD("not support AI-HDR due to Flash on(%d) or forceEnable(%d)"
                , flashOn, mEnable);
            return BAD_VALUE;
        }

        int shotInstance = gShotCount.load(std::memory_order_relaxed);
        if (sel.mRequestIndex == 0
                && CC_UNLIKELY(shotInstance > 1)) {
            ainrLogD("not support AI-HDR because of shot count(%d)", shotInstance);
            return BAD_VALUE;
        }

        std::shared_ptr<AiHdrShot> shot = nullptr;

        if (sel.mRequestIndex == 0) {
            // TODO(Yuan Jung): Remove forced false when m-stream ready
            mZSDMode = false;  // sel.mState.mZslRequest && sel.mState.mZslPoolReady;
            mRealIso = sel.mState.mRealIso;
            mShutterTime = sel.mState.mExposureTime;

            // Set uniqueKey in first frame
            setUniqueKey(NSCam::Utils::TimeTool::getReadableTime());

            IMetadata* appInMeta = nullptr;
            if (CC_LIKELY(sel.mIMetadataApp.getControl() != nullptr)) {
                appInMeta = sel.mIMetadataApp.getControl().get();
            } else {
                ainrLogF("mIMetadataApp getControl fail, we cannot judge hdr decision.");
                return BAD_VALUE;
            }

            if (CC_LIKELY(appInMeta != nullptr)) {
                MUINT8 sceneMode = 0;
                if (IMetadata::getEntry<MUINT8>(appInMeta, MTK_CONTROL_SCENE_MODE, sceneMode)) {
                    if (sceneMode == MTK_CONTROL_SCENE_MODE_HDR) {
                        if (mZSDMode) {
                            mMode = ainr::AinrMode_ZsdHdr;
                        } else {
                            mMode = ainr::AinrMode_NormalHdr;
                        }
                    } else {
                        ainrLogD("No need to do AiHdr because of not hdr scene mode(%d)", sceneMode);
                        return BAD_VALUE;
                    }
                } else {
                    ainrLogD("Cannot get appMeta MTK_CONTROL_SCENE_MODE");
                    return BAD_VALUE;
                }
            } else {
                ainrLogF("Cannot fetch app metadata, we cannot judge hdr decision.");
                return BAD_VALUE;
            }

            sel.mDecision.mZslEnabled = isZsdMode(mMode);
            sel.mDecision.mZslPolicy.mPolicy = v3::pipeline::policy::eZslPolicy_AfState
                                             | v3::pipeline::policy::eZslPolicy_ContinuousFrame
                                             | v3::pipeline::policy::eZslPolicy_ZeroShutterDelay;
            sel.mDecision.mZslPolicy.mTimeouts = 1000;
            ainrLogD("AiHdr mode(%d) zslStatus(%d)", mMode, sel.mDecision.mZslEnabled);

            // Judge whether to do AI-HDR decision
            MINT32 sensorId = sel.mState.mSensorId;
            createAiHdrShot(sensorId);
            shot = getHdrShot(getUniqueKey());
            if (CC_UNLIKELY(shot.get()== nullptr)) {
                ainrLogF("Get hdr shot fail");
                return BAD_VALUE;
            }

            auto &sensorSize = sel.mState.mSensorSize;
            shot->updateAinrStrategy(sensorSize);
        }

        // Get AiHdrShot
        if (shot.get() == nullptr) {
            shot = getHdrShot(getUniqueKey());
            if (CC_UNLIKELY(shot.get() == nullptr)) {
                removeAiHdrShot(getUniqueKey());
                ainrLogF("get AiHdrShot instance failed! cannot apply aiHdr.");
                return BAD_VALUE;
            }
        }

        sel.mRequestCount = shot->getCaptureNum();
        ainrLogD("AiHdr decision apply = %d, forceEnable = %d, frames = %d"
            , shot->getIsEnableAinr(), mEnable, sel.mRequestCount);

        bool enableAinr = false;
        if ((mEnable == 1) || shot->getIsEnableAinr()) {
            enableAinr = true;
            if (mEnable == 1) {
                constexpr int frame_num = 7;
                sel.mRequestCount = frame_num;
                shot->setCaptureNum(frame_num);
            }
        }

        if (!enableAinr
            || sel.mRequestCount == 0) {
            ainrLogD("getIsEnableAihdr(%d), requestCount(%d), enable(%d)"
                , shot->getIsEnableAinr()
                , sel.mRequestCount
                , mEnable);
            removeAiHdrShot(getUniqueKey());
            ainrLogE_IF(sel.mRequestCount == 0, "Suspect NVRAM data is EMPTY! Abnormal!");
            return BAD_VALUE;
        }

        auto coreVersion = shot->queryCoreVersion();
        ainrLogD("Ainr core version(0x%x)", coreVersion);

        EImageFormat resizedFormat = eImgFmt_FG_BAYER10;
        EImageSize resizedSize = eImgSize_Resized;
        if (coreVersion == ainr::AINRCORE_VERSION_2_0) {
            resizedFormat = eImgFmt_MTK_YUV_P010;
            resizedSize = eImgSize_Specified;
        }

        sel.mIBufferFull
            .setRequired(MTRUE)
            .addAcceptedFormat(eImgFmt_BAYER10)
            .addAcceptedSize(eImgSize_Full);

        sel.mIBufferResized
            .setRequired(MTRUE)
            .addAcceptedFormat(resizedFormat)
            .addAcceptedSize(resizedSize);

        sel.mIMetadataDynamic.setRequired(MTRUE);
        sel.mIMetadataApp.setRequired(MTRUE);
        sel.mIMetadataHal.setRequired(MTRUE);

        // Only main frame has output buffer
        if (sel.mRequestIndex == 0) {
            sel.mOBufferFull
                .setRequired(MTRUE)
                .addAcceptedFormat(eImgFmt_BAYER16_UNPAK)
                .addAcceptedSize(eImgSize_Full);

            sel.mOMetadataApp.setRequired(MTRUE);
            sel.mOMetadataHal.setRequired(MTRUE);

            gShotCount++;
            ainrLogD("AiHdr shot count(%d)", gShotCount.load(std::memory_order_relaxed));
        } else {
            sel.mOBufferFull.setRequired(MFALSE);
            sel.mOMetadataApp.setRequired(MFALSE);
            sel.mOMetadataHal.setRequired(MFALSE);
        }

        if (sel.mIMetadataApp.getControl() != nullptr) {
            MetadataPtr pAppAddtional = std::make_shared<IMetadata>();
            MetadataPtr pHalAddtional = std::make_shared<IMetadata>();

            IMetadata* pAppMeta = pAppAddtional.get();
            IMetadata* pHalMeta = pHalAddtional.get();
            IMetadata::setEntry<MINT32>(pHalMeta,
                    MTK_PIPELINE_UNIQUE_KEY, getUniqueKey());
            //
            MUINT8 bOriFocusPause  = 0;
            if ( !IMetadata::getEntry<MUINT8>(pHalMeta, MTK_FOCUS_PAUSE, bOriFocusPause) ) {
                ainrLogW("%s: cannot retrieve MTK_FOCUS_PAUSE from HAL metadata, assume "\
                        "it to 0", __FUNCTION__);
            }
            const bool bLastFrame = (sel.mRequestIndex+1 == sel.mRequestCount) ? true : false;
            IMetadata::setEntry<MUINT8>(pHalMeta, MTK_FOCUS_PAUSE, bLastFrame ? bOriFocusPause : 1);

            // EXIF require
            IMetadata::setEntry<MUINT8>(pHalMeta, MTK_HAL_REQUEST_REQUIRE_EXIF, 1);
            IMetadata::setEntry<MUINT8>(pHalMeta, MTK_HAL_REQUEST_DUMP_EXIF, 1);

            // AE Flare Enable, due to MTK_CONTROL_AE_MODE_OFF will disable AE_Flare
            // MTK_3A_FLARE_IN_MANUAL_CTRL_ENABLE to enable AE_Flare
            IMetadata::setEntry<MBOOL>(pHalMeta, MTK_3A_FLARE_IN_MANUAL_CTRL_ENABLE, MTRUE);

            shot->updateEvToMeta(sel.mRequestIndex, pAppMeta, pHalMeta);
            //shot->updateBssToMeta(sel.mRequestIndex, pAppMeta, pHalMeta);

            sel.mIMetadataApp.setAddtional(pAppAddtional);
            sel.mIMetadataHal.setAddtional(pHalAddtional);
        }

        // dummy frame
        {
            MetadataPtr pAppDummy = std::make_shared<IMetadata>();
            MetadataPtr pHalDummy = std::make_shared<IMetadata>();
            IMetadata* pAppMeta = pAppDummy.get();
            IMetadata* pHalMeta = pHalDummy.get();

            // last frame
            if (sel.mRequestIndex+1  == sel.mRequestCount) {
                sel.mPostDummy = shot->getDelayFrameNum();
                IMetadata::setEntry<MBOOL>(pHalMeta, MTK_3A_DUMMY_AFTER_REQUEST_FRAME, 1);
            }

            sel.mIMetadataApp.setDummy(pAppDummy);
            sel.mIMetadataHal.setDummy(pHalDummy);
        }

        return OK;
    }

    virtual void init() {
        CAM_ULOGM_APILIFE();
    }

    virtual MERROR process(RequestPtr pRequest,
                           RequestCallbackPtr pCallback) {
        CAM_ULOGM_APILIFE();

        // set thread's name
        ::prctl(PR_SET_NAME, "AiHdrPlugin", 0, 0, 0);

        // We need to make process method thread safe
        // Because plugin is a singleTon we need to protect it
        std::lock_guard<decltype(mProcessMx)> lk(mProcessMx);

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
        IMetadata* pAppMeta = nullptr;
        IMetadata* pHalMeta = nullptr;
        IMetadata* pAppMetaDynamic = nullptr;
        IMetadata* pOutHaMeta = nullptr;

        // Input metadata
        if (CC_UNLIKELY(pRequest->mIMetadataApp == nullptr)
                || CC_UNLIKELY(pRequest->mIMetadataHal == nullptr)
                || CC_UNLIKELY(pRequest->mIMetadataDynamic == nullptr)) {
            ainrLogF("Cannot get input metadata because of nullptr");
            return BAD_VALUE;
        }

        pAppMeta = pRequest->mIMetadataApp->acquire();
        pHalMeta = pRequest->mIMetadataHal->acquire();
        pAppMetaDynamic = pRequest->mIMetadataDynamic->acquire();
        if (CC_UNLIKELY(pAppMeta == nullptr)
            || CC_UNLIKELY(pHalMeta == nullptr)
            || CC_UNLIKELY(pAppMetaDynamic == nullptr)) {
            ainrLogF("one of metdata is null idx(%d)!!!", pRequest->mRequestIndex);
            return BAD_VALUE;
        }

        if (pRequest->mOMetadataHal) {
            pOutHaMeta = pRequest->mOMetadataHal->acquire();
            if (CC_UNLIKELY(pOutHaMeta == nullptr)) {
                ainrLogE("pOutHaMeta is null idx(%d)!!!", pRequest->mRequestIndex);
                return BAD_VALUE;
            }
        }

        // Get input buffer
        if (CC_LIKELY(pRequest->mIBufferFull != nullptr)) {
            pIImgBuffer = pRequest->mIBufferFull->acquire();
            if (CC_UNLIKELY(pIImgBuffer == nullptr)) {
                ainrLogF("Input buffer is null idx(%d)!!!", pRequest->mRequestIndex);
                return BAD_VALUE;
            }
            if (mDump) {
                std::string str = "InputImgo" + std::to_string(pRequest->mRequestIndex);
                MINT32 sensorId = pRequest->mSensorId;
                bufferDump(sensorId, pHalMeta, pIImgBuffer, TuningUtils::RAW_PORT_IMGO, str.c_str());
            }
        } else {
            ainrLogF("mIBufferFull is null");
            return BAD_VALUE;
        }

        // Get input rrzo buffer
        if (CC_LIKELY(pRequest->mIBufferResized != nullptr)) {
            pIImageBufferRrzo = pRequest->mIBufferResized->acquire();
            if (CC_UNLIKELY(pIImageBufferRrzo == nullptr)) {
                ainrLogE("Input buffer is null idx(%d)!!!", pRequest->mRequestIndex);
                return BAD_VALUE;
            }
            if (mDump & ainr::AinrDumpWorking) {
                std::string str = "InputRrzo" + std::to_string(pRequest->mRequestIndex);
                // dump input buffer
                MINT32 sensorId = pRequest->mSensorId;
                bufferDump(sensorId, pHalMeta, pIImageBufferRrzo, TuningUtils::RAW_PORT_RRZO, str.c_str());
            }
        } else {
            ainrLogE("mIBufferResized is null");
            return BAD_VALUE;
        }

        // Get output buffer
        if (pRequest->mOBufferFull != nullptr) {
            pOImgBuffer = pRequest->mOBufferFull->acquire();
            if (CC_UNLIKELY(pOImgBuffer == nullptr)) {
                ainrLogF("Output buffer is null idx(%d)!!!", pRequest->mRequestIndex);
                return BAD_VALUE;
            }
        }

        {
            std::lock_guard<decltype(mReqMx)> _reqlk(mReqMx);
            mvRequests.push_back(pRequest);
        }

        ainrLogD("collected request(%d/%d)",
                pRequest->mRequestIndex+1,
                pRequest->mRequestCount);

        /********************************Finish basic flow start to do AINR**********************************************/
        MINT32 processUniqueKey = 0;
        if (!IMetadata::getEntry<MINT32>(pHalMeta,
                MTK_PIPELINE_UNIQUE_KEY, processUniqueKey)) {
            ainrLogE("cannot get unique about ainr capture");
            return BAD_VALUE;
        } else {
            ainrLogD("Ainr MTK_PIPELINE_UNIQUE_KEY(%d)", processUniqueKey);
        }

        std::shared_ptr<AiHdrShot> shot = getHdrShot(processUniqueKey);
        if (CC_UNLIKELY(shot.get() == nullptr)) {
            removeAiHdrShot(processUniqueKey);
            ainrLogF("get AiHdrShot instance failed! cannot process AinrCtrler.");
            return BAD_VALUE;
        }

        // Add data (appMeta, halMeta, outHalMeta, IMGO, RRZO)
        // into AiHdrShot
        {
            ainr::AinrPipelinePack inputPack;
            inputPack.appMeta        = pAppMeta;
            inputPack.halMeta        = pHalMeta;
            inputPack.appMetaDynamic = pAppMetaDynamic;
            inputPack.outHalMeta     = pOutHaMeta;
            inputPack.imgoBuffer     = pIImgBuffer;
            inputPack.rrzoBuffer     = pIImageBufferRrzo;  // Remeber to modify as rrzo
            shot->addInputData(inputPack);
        }

        // Initialize
        if (pRequest->mRequestIndex == 0) {
            // Set up release function
            mCbFunction = [this](int32_t uniqueKey, int32_t mainFrameIndex) {
                std::lock_guard<decltype(mReqTableMx)> _lk(mReqTableMx);
            #if 0
                auto search = mReqTable.find(uniqueKey);
                if (search != mReqTable.end()) {
                    ainrLogD("Start to release buffers key(%d)", uniqueKey);
                    auto vRequests = search->second;
                    for (auto && req : vRequests) {
                        // Main frame should be keep.
                        if (req->mRequestIndex != mainFrameIndex) {
                            ainrLogD("Release index(%d)", req->mRequestIndex);
                            req->mIBufferFull->release();
                            req->mIBufferResized->release();
                            req->mIMetadataApp->release();
                            req->mIMetadataHal->release();
                            req->mIMetadataDynamic->release();

                            // Indicate next captures
                            if (req->mRequestIndex == req->mRequestCount - 1) {
                                std::lock_guard<decltype(mCbMx)> _cblk(mCbMx);
                                ainrLogD("Next capture callback");
                                auto cbFind = mCbTable.find(uniqueKey);
                                if (cbFind != mCbTable.end()) {
                                    if (cbFind->second != nullptr) {
                                        cbFind->second->onNextCapture(req);
                                    }
                                } else {
                                    ainrLogF("cannot find cb from table");
                                }
                            }
                        }
                    }
                } else {
                    ainrLogF("Can not find requests in reqTable by key(%d)", uniqueKey);
                }
                return;
            #endif
            };

            // Initialize AiHdrShot
            MSize  imgo       = pIImgBuffer->getImgSize();
            size_t imgoStride = pIImgBuffer->getBufStridesInBytes(0);
            MSize  rrzo       = pIImageBufferRrzo->getImgSize();
            // Setup debug exif
            shot->setAlgoMode(ainr::AIHDR);
            shot->setCaptureNum(pRequest->mRequestCount);
            shot->setSizeImgo(imgo);
            shot->setSizeRrzo(rrzo);
            shot->setStrideImgo(imgoStride);
            // config ainr controller
            shot->configAinrCore(pHalMeta, pAppMetaDynamic);
            shot->addOutData(pOImgBuffer);
            shot->registerCB(mCbFunction);

            // Back up callback
            {
                std::lock_guard<decltype(mCbMx)> _cblk(mCbMx);
                if (pCallback != nullptr) {
                    if (mCbTable.emplace(processUniqueKey, pCallback).second == false)
                        ainrLogE("Emplace callback fail!!");
                } else {
                    ainrLogW("Callback ptr is null!!!");
                }
            }
        }

        // Last frame
        if (pRequest->mRequestIndex == pRequest->mRequestCount - 1) {
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

            shot->execute();

            acquireJobQueue();
            auto __job = [this] (MINT32 key, RequestCallbackPtr pCb) {
                ainrLogD("Process uniqueKey(%d) job", key);
                std::shared_ptr<AiHdrShot>  __shot = getHdrShot(key);
                if (CC_UNLIKELY(__shot.get() == nullptr)) {
                    ainrLogF("Job Queue gets instance failed!");
                } else {
                    // Wait AINR postprocessing done
                    __shot->waitExecution();
                    __shot = nullptr;
                    // We finish postprocess, release uniqueKey
                    removeAiHdrShot(key);
                }

                // Decrease ainr counter
                gShotCount--;

                auto requestsPtr = getRequests(key);
                if (requestsPtr) {
                    for (auto req : (*requestsPtr)) {
                        ainrLogD("callback request(%d/%d) %p",
                                req->mRequestIndex+1,
                                req->mRequestCount, pCb.get());
                        if (pCb != nullptr) {
                            pCb->onCompleted(req, 0);
                        }
                    }
                }

                // Clear up resources
                removeRequests(key);

                // Clean up back up table
                {
                    std::lock_guard<decltype(mCbMx)> __lk(mCbMx);
                    if (mCbTable.count(key)) {
                        mCbTable[key] = nullptr;
                        mCbTable.erase(key);
                    }
                }
            };
            mMinorJobQueue->addJob(std::bind(__job, processUniqueKey, pCallback));
        }
        return 0;
    }

    virtual void abort(std::vector<RequestPtr>& pRequests  __attribute__((unused))) {
        CAM_ULOGM_APILIFE();
        ainrLogD("AI-HDR abort");
        // Wait all jobs in queue to be finished
        acquireJobQueue();
        ainrLogD("JobQueue wait+");
        mMinorJobQueue->requestExit();
        mMinorJobQueue->wait();
        ainrLogD("JobQueue wait-");

        if (mPluginCB.get()) {
            for (auto &&req : pRequests) {
                mPluginCB->onAborted(req);
            }
        }
    }

    virtual void uninit() {
        CAM_ULOGM_APILIFE();
    }

    AiHdrPlugin()
        : mEnable(-1)
        , mMode(ainr::AinrMode_Off)
        , mRealIso(0)
        , mShutterTime(0)
        , mZSDMode(MFALSE)
        , mUniqueKey(0)
        , mPluginCB(nullptr) {
        CAM_ULOGM_APILIFE();
        mDump          = ::property_get_int32("vendor.debug.camera.aihdr.dump", 0);
    }

    virtual ~AiHdrPlugin() {
        CAM_ULOGM_APILIFE();
        removeAllShots();

        // Remove all callback ptr
        {
            std::lock_guard<decltype(mCbMx)> __lk(mCbMx);

            ainrLogD("Remove callback table");
            for (auto & cb : mCbTable) {
                cb.second = nullptr;
            }
            mCbTable.clear();
        }
    }

 private:
    MINT32 getUniqueKey() {
        CAM_ULOGM_APILIFE();

        std::lock_guard<decltype(mShotMx)> lk(mShotMx);
        return mUniqueKey;
    }

    void setUniqueKey(uint32_t key) {
        CAM_ULOGM_APILIFE();

        std::lock_guard<decltype(mShotMx)> lk(mShotMx);
        mUniqueKey = key;
    }


    void createAiHdrShot(MINT32 sensorId) {
        CAM_ULOGM_APILIFE();

        auto shot = std::make_shared<AiHdrShot>(mUniqueKey, sensorId, mRealIso, mShutterTime);
        std::lock_guard<decltype(mShotMx)> lk(mShotMx);
        mShotContainer[mUniqueKey] = std::move(shot);
        ainrLogD("Create ShotInfos: %d", mUniqueKey);
    }

    void removeAiHdrShot(uint32_t key) {
        CAM_ULOGM_APILIFE();

        std::lock_guard<decltype(mShotMx)> lk(mShotMx);
        if (mShotContainer.count(key)) {
            ainrLogD("Remvoe AiHdrShot: %d", key);
            mShotContainer[key] = nullptr;
            mShotContainer.erase(key);
        }
    }

    void removeAllShots() {
        CAM_ULOGM_APILIFE();

        std::lock_guard<decltype(mShotMx)> lk(mShotMx);
        for (auto && shot : mShotContainer) {
            shot.second = nullptr;
            ainrLogD("Remvoe AiHdrShot: %d", shot.first);
        }
        mShotContainer.clear();
    }

    std::shared_ptr<AiHdrShot> getHdrShot(uint32_t key) {
        CAM_ULOGM_APILIFE();
        std::lock_guard<decltype(mShotMx)> lk(mShotMx);

        ainrLogD("Get shot instance(%d)", key);
        auto find = mShotContainer.find(key);
        if (find != mShotContainer.end()) {
            return find->second;
        } else {
            ainrLogF("Can not find shot instance(%d)", key);
            return nullptr;
        }
    }

    std::vector<RequestPtr> *getRequests(MINT32 key) {
        CAM_ULOGM_APILIFE();

        std::lock_guard<decltype(mReqTableMx)> lk(mReqTableMx);

        auto find = mReqTable.find(key);
        if (find != mReqTable.end()) {
            return &find->second;
        } else {
            ainrLogF("Can not get requests from table");
            return nullptr;
        }
    }

    void removeRequests(MINT32 key) {
        CAM_ULOGM_APILIFE();

        std::lock_guard<decltype(mReqTableMx)> __lk(mReqTableMx);
        if (mReqTable.count(key)) {
            mReqTable[key].clear();
            mReqTable.erase(key);
            ainrLogD("Remvoe requests: %d from table", key);
        }
    }

    void acquireJobQueue() {
        static std::mutex       sResLock;
        static std::weak_ptr<
            NSCam::JobQueue<void()>
        >                       sResJobQ;

        // acquire resource from weak_ptr
        std::lock_guard<std::mutex> lk(sResLock);
        mMinorJobQueue = sResJobQ.lock();
        if (mMinorJobQueue.get() == nullptr) {
            mMinorJobQueue = std::shared_ptr< NSCam::JobQueue<void()> >(
                    new NSCam::JobQueue<void()>("AiHdrPluginJob"));
            sResJobQ = mMinorJobQueue;
        }
        return;
    }

    void bufferDump(MINT32 sensorId, IMetadata *halMeta, IImageBuffer* buff,
            TuningUtils::RAW_PORT type, const char *pUserString) {
        // dump input buffer
        char fileName[512] = "";
        TuningUtils::FILE_DUMP_NAMING_HINT dumpNamingHint {};
        //
        MUINT8 ispProfile = NSIspTuning::EIspProfile_Capture;

        if (!halMeta || !buff) {
            ainrLogE("HalMeta or buff is nullptr, dump fail");
            return;
        }

        if (!IMetadata::getEntry<MUINT8>(halMeta, MTK_3A_ISP_PROFILE, ispProfile)) {
            ainrLogW("cannot get ispProfile at ainr capture");
        }

        // Extract hal metadata and fill up file name;
        extract(&dumpNamingHint, halMeta);
        // Extract buffer information and fill up file name;
        extract(&dumpNamingHint, buff);
        // Extract by sensor id
        extract_by_SensorOpenId(&dumpNamingHint, sensorId);
        // IspProfile
        dumpNamingHint.IspProfile = ispProfile;  // EIspProfile_Capture;

        genFileName_RAW(fileName, sizeof(fileName), &dumpNamingHint, type, pUserString);
        buff->saveToFile(fileName);
    }

 private:
    //
    int                             mEnable;
    int                             mDump;
    const double                    RESTRICT_MEM_AMOUNT = (4.3);  // 4G 1024*1024*1024*4
    // file dump hint
    TuningUtils::FILE_DUMP_NAMING_HINT
                                    mDumpNamingHint;
    //
    std::mutex                      mProcessMx;  // TO make plugin process thread safe

    // mode
    ainr::AinrMode                  mMode;
    // Requests
    std::mutex                      mReqMx;
    std::vector<RequestPtr>         mvRequests;
    // Callback
    std::mutex                      mCbMx;
    std::unordered_map< MINT32
            , RequestCallbackPtr >  mCbTable;

    //
    MINT32                          mRealIso;
    MINT32                          mShutterTime;
    MBOOL                           mZSDMode;
    //
    std::mutex                      mShotMx;  // protect AiHdrShot
    uint32_t                        mUniqueKey;

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
    std::unordered_map<uint32_t, std::shared_ptr<AiHdrShot>>
                                    mShotContainer;
    // JobQueue
    std::shared_ptr<
        NSCam::JobQueue<void()>
    >                               mMinorJobQueue;
};

REGISTER_PLUGIN_PROVIDER(MultiFrame, AiHdrPlugin);
}  // namespace NSPipelinePlugin
}  // namespace NSCam
