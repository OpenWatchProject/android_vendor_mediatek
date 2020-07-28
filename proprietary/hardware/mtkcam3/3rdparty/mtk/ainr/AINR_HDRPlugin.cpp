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
#define LOG_TAG "AINR_HDRPlugin"
//
#include <mtkcam/utils/std/Log.h>
//
#include <stdlib.h>
#include <cfenv>
#include <atomic>
#include <utils/Errors.h>
#include <utils/List.h>
#include <utils/RefBase.h>
#include <utils/String8.h>
#include <sstream>
#include <math.h>
//
#include <mtkcam/utils/metadata/client/mtk_metadata_tag.h>
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>
//
//
#include <mtkcam/utils/imgbuf/IIonImageBufferHeap.h>
//
#include <mtkcam/drv/IHalSensor.h>
#include <mtkcam/utils/std/Format.h>
//
#include <mtkcam3/pipeline/hwnode/NodeId.h>

#include <mtkcam/utils/metastore/IMetadataProvider.h>
#include <mtkcam/utils/metastore/ITemplateRequest.h>
#include <mtkcam3/3rdparty/plugin/PipelinePlugin.h>
#include <mtkcam3/3rdparty/plugin/PipelinePluginType.h>
#include <mtkcam/utils/sys/MemoryInfo.h>
// MTKCAM
#include <mtkcam/aaa/IHal3A.h> // setIsp, CaptureParam_T
#include <mtkcam/aaa/aaa_hal_common.h>
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
//
#include <mtkcam3/feature/ainr/AinrUlog.h>
//
using namespace NSCam;
using namespace plugin;
using namespace android;
using namespace std;
using namespace NSCam::NSPipelinePlugin;
using namespace NS3Av3;
using namespace NSCam::TuningUtils;
using namespace NSIoPipe;
/******************************************************************************
 *
 ******************************************************************************/

#define AINR_FRAME_SE_IDX 0
#define AINR_FRAME_BSS_IDX 1

/******************************************************************************
* Static variable
******************************************************************************/


/******************************************************************************
*
******************************************************************************/
class AINR_HDRPlugin : public MultiFramePlugin::IProvider
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

            prop.mName              = "MTK_AINR_HDR";
            prop.mFeatures          = MTK_FEATURE_AINR_FOR_HDR;
            prop.mThumbnailTiming   = thumbTiming;
            prop.mPriority          = ePriority_Highest;
            prop.mZsdBufferMaxNum   = 6; // maximum frames requirement
            prop.mNeedRrzoBuffer    = MTRUE; // rrzo requirement for BSS
            inited                  = MTRUE;
        }

        return prop;
    };

    //if capture number is 4, "negotiate" would be called 4 times
    virtual MERROR negotiate(Selection& sel)
    {
        CAM_ULOGM_APILIFE();

        if(CC_UNLIKELY(mEnable == 0)) {
            ainrLogD("Force off AINR HDR");
            return BAD_VALUE;
        }

        auto flashOn = sel.mState.mFlashFired;
        if (CC_UNLIKELY(flashOn == MTRUE)) {
            ainrLogD("not support AINR due to Flash on(%d)", flashOn);
            return BAD_VALUE;
        }

        IMetadata* appInMeta = sel.mIMetadataApp.getControl().get();

        if (sel.mRequestIndex == 0) {
            //
            mZSDMode = 0;
            mFlashOn = sel.mState.mFlashFired;
            mRealIso = sel.mState.mRealIso;
            mShutterTime = sel.mState.mExposureTime;

            if(mShotCount > 1) {
                ainrLogD("No need shotCount(%d)", mShotCount.load(std::memory_order_relaxed));
                return BAD_VALUE;
            }

            // Set uniqueKey in first frame
            setUniqueKey(NSCam::Utils::TimeTool::getReadableTime());
            // AINR + HDR 0 ev should be ais pline??
            mMfbMode = AinrMode_NormalAis;
        #if 0
            if (CC_LIKELY(sel.mIMetadataApp.getControl() != nullptr)) {
                mMfbMode = updateMfbMode(appInMeta, mZSDMode);
            } else {
                mMfbMode = AinrMode_NormalAinr;
                ainrLogW("negotiate without metadata, using mMfbMode = AinrMode_NormalAinr.");
            }

            if (CC_UNLIKELY( mMfbMode == AinrMode_Off )) {
                ainrLogD("No need to do AINR because of AinrMode_Off");
                return BAD_VALUE;
            }
        #endif
            // Create AinrCtrler
            MINT32 sensorId = sel.mState.mSensorId;
            createCtrlerInfo(sensorId);

            sel.mDecision.mZslEnabled = 0;

            mShotCount++;

            ainrLogD("Shot count(%d)", mShotCount.load(std::memory_order_relaxed));
        }
        /*********************************************Judge whether to do AINR or not*****************************************/

        // Get AinrCtrler
        std::shared_ptr<AinrCtrler> ctrler = getCtrlerInfo(getUniqueKey());
        if (CC_UNLIKELY(ctrler.get() == nullptr)) {
            removeCtrlerInfo(getUniqueKey());
            ainrLogE("get AinrCtrler instance failed! cannot apply AINR.");
            return BAD_VALUE;
        }

        // Update ainr policy
        if (sel.mRequestIndex == 0) {
            ctrler->setSizeImgo(sel.mState.mSensorSize);
            ainrLogD("set source size = %dx%d", ctrler->getSizeImgo().w, ctrler->getSizeImgo().h);

            auto &sensorSize = sel.mState.mSensorSize;
            ctrler->updateAinrStrategy(sensorSize);

            ainrLogD("realIso = %d, shutterTime = %d, finalRealIso = %d, finalShutterTime = %d"
                , ctrler->getRealIso()
                , ctrler->getShutterTime()
                , ctrler->getFinalIso()
                , ctrler->getFinalShutterTime());

            // Negotiate
            // Because AINR + HDR need n+1 frame.
            sel.mRequestCount = ctrler->getCaptureNum() + 1;

            // TODO: Please put your scene mode here
            // We suggest customize here
            bool enAinrHDR = false;

            if(mEnable == 1) {
                ainrLogD("Ainr HDR forced on");
            }

            if(ctrler->getIsEnableAinr()) {
                ainrLogD("Ainr HDR is on iso(%d)", mRealIso);
            }

            if( (/* scene mode decision */ isSceneHDR(appInMeta) && ctrler->getIsEnableAinr())
                || (mEnable == 1) ) {
                enAinrHDR = true;
            }

            if (!enAinrHDR
                || sel.mRequestCount == 1) {//AINR+HDR need 1 more frame.
                ainrLogD("Enable ainr mEnable(%d), ctrlerEnable(%d)"
                    , mEnable, ctrler->getIsEnableAinr());
                ainrLogE_IF(sel.mRequestCount == 1, "Suspect NVRAM data is EMPTY! Abnormal!");
                removeCtrlerInfo(getUniqueKey());
                return BAD_VALUE;
            }
        }

        // Because AINR + HDR need n+1 frame. SE is first frame
        sel.mRequestCount = ctrler->getCaptureNum() + 1;

        ainrLogD("Ainr apply = %d, frames = %d", ctrler->getIsEnableAinr(), sel.mRequestCount);
        /************************************************************End*******************************************************/
        sel.mIBufferFull
            .setRequired(MTRUE)
            .addAcceptedFormat(eImgFmt_BAYER10)
            .addAcceptedSize(eImgSize_Full);

        sel.mIBufferResized
            .setRequired(MTRUE)
            .addAcceptedFormat(eImgFmt_FG_BAYER10)
            .addAcceptedSize(eImgSize_Resized);

        sel.mIMetadataDynamic.setRequired(MTRUE);
        sel.mIMetadataApp.setRequired(MTRUE);
        sel.mIMetadataHal.setRequired(MTRUE);

        //Only main frame has output buffer
        // TODO: Need to set outputbuffer as unpack16 format
        if (sel.mRequestIndex == 0) {
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

        // Set up per-frame metadata
        if (sel.mIMetadataApp.getControl() != nullptr) {
            MetadataPtr pAppAddtional = make_shared<IMetadata>();
            MetadataPtr pHalAddtional = make_shared<IMetadata>();

            IMetadata* pAppMeta = pAppAddtional.get();
            IMetadata* pHalMeta = pHalAddtional.get();

            int seIndex = sel.mRequestCount - 1;

            // Every frames should be indicated frame numbers
            IMetadata::setEntry<MINT32>(pHalMeta, MTK_FEATURE_BSS_SELECTED_FRAME_COUNT, ctrler->getCaptureNum());
            if (sel.mRequestIndex != seIndex) {
                //IMetadata::setEntry<MINT32>(pHalMeta, MTK_FEATURE_BSS_SELECTED_FRAME_COUNT, ctrler->getCaptureNum());
                IMetadata::setEntry<MINT32>(pHalMeta, MTK_FEATURE_BSS_PROCESS, MTK_FEATURE_BSS_PROCESS_ENABLE);
                IMetadata::setEntry<MBOOL>(pHalMeta, MTK_FEATURE_BSS_REORDER, MTRUE);
                IMetadata::setEntry<MINT32>(pHalMeta, MTK_FEATURE_BSS_FORCE_DROP_NUM, 0);
                // Indicate need BSS and no need to YUV processing
                IMetadata::setEntry<MUINT8>(pHalMeta, MTK_FEATURE_MULTIFRAMENODE_BYPASSED, MTK_FEATURE_MULTIFRAMENODE_NOT_BYPASSED);
                IMetadata::setEntry<MUINT8>(pHalMeta, MTK_FEATURE_CAP_YUV_PROCESSING, MTK_FEATURE_CAP_YUV_PROCESSING_NOT_NEEDED);
            } else {
                // Need to by pass BSS and indicate YUVNode needed
                IMetadata::setEntry<MUINT8>(pHalMeta, MTK_FEATURE_MULTIFRAMENODE_BYPASSED, MTK_FEATURE_MULTIFRAMENODE_TO_BE_BYPASSED);
                IMetadata::setEntry<MUINT8>(pHalMeta, MTK_FEATURE_CAP_YUV_PROCESSING, MTK_FEATURE_CAP_YUV_PROCESSING_NEEDED);
            }
            IMetadata::setEntry<MINT32>(pHalMeta, MTK_PIPELINE_UNIQUE_KEY, getUniqueKey());
            updatePerFrameMetadata(sel.mState.mSensorId, ctrler.get(), pAppMeta, pHalMeta, (sel.mRequestIndex+1 == sel.mRequestCount));

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
            if (sel.mRequestIndex == 0) {
                sel.mFrontDummy = ctrler->getDummyFrameNum();
                if (ctrler->getIsFlashOn()) {
                    IMetadata::setEntry<MBOOL>(pHalMeta, MTK_3A_DUMMY_BEFORE_REQUEST_FRAME, 1);
                }
            }

            //last frame
            if (sel.mRequestIndex+1  == sel.mRequestCount) {
                sel.mPostDummy = ctrler->getDelayFrameNum();
                IMetadata::setEntry<MBOOL>(pHalMeta, MTK_3A_DUMMY_AFTER_REQUEST_FRAME, 1);
            }

            sel.mIMetadataApp.setDummy(pAppDummy);
            sel.mIMetadataHal.setDummy(pHalDummy);
        }

        return OK;
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
        ::prctl(PR_SET_NAME, "AINR_HDRPlugin", 0, 0, 0);

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
                ainrLogE("Input buffer is null idx(%d)!!!", pRequest->mRequestBSSIndex);
                return BAD_VALUE;
            }
            if (mDump & AinrDumpWorking){
                std::string str = "InputRrzo" + std::to_string(pRequest->mRequestBSSIndex);
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
            if(CC_UNLIKELY(pOImgBuffer == nullptr)) {
                ainrLogE("Output buffer is null idx(%d)!!!", pRequest->mRequestBSSIndex);
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

        //get AinrCtrler. Should we use processUniqueKey
        std::shared_ptr<AinrCtrler> ctrler = getCtrlerInfo(processUniqueKey);
        if (CC_UNLIKELY(ctrler.get() == nullptr)) {
            removeCtrlerInfo(processUniqueKey);
            ainrLogE("get AinrCtrler instance failed! cannot process AinrCtrler.");
            return BAD_VALUE;
        }

        // Add data (appMeta, halMeta, outHalMeta, IMGO, RRZO)
        // into AinrCtrler. Only 0 ev frame need to move to ctrler
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
                    ainrLogD("Start to release buffers key(%d)", uniqueKey);
                    auto vRequests = search->second;
                    for (auto req : vRequests){
                        ainrLogD("Release index(%d)", req->mRequestIndex);
                        req->mIBufferFull->release();
                        req->mIBufferResized->release();
                        req->mIMetadataApp->release();
                        req->mIMetadataHal->release();
                        req->mIMetadataDynamic->release();

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
            MSize imgo        = pIImgBuffer->getImgSize();
            MSize rrzo        = pIImageBufferRrzo->getImgSize();
            size_t imgoStride = pIImgBuffer->getBufStridesInBytes(0);
            // Setup debug exif
            ctrler->setAlgoMode(AINR_YHDR);
            // Because no need to pass SE buffer to AINRCore
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

            acquireJobQueue();

            auto __job = [this] (MINT32 key, RequestCallbackPtr pCb) {
                std::shared_ptr<AinrCtrler>  ctr = getCtrlerInfo(key);
                // Wait AINR postprocessing done
                ctr->waitExecution();
                // We finish postprocess, release uniqueKey
                removeCtrlerInfo(key);
                // Remove AINR HDR shot
                mShotCount--;

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
                        mCbTable.erase(key);
                    }
                }
            };

            mMinorJobQueue->addJob( std::bind(__job, processUniqueKey, pCallback) );
        }
        return 0;
    }

    void getCaptureParam(MINT32 sensorId, CaptureParam_T &capParam, int ev)
    {
        std::unique_ptr <
                        IHal3A,
                        std::function<void(IHal3A*)>
                        > hal3a
                (
                    MAKE_Hal3A(sensorId, "AINR_HDR"),
                    [](IHal3A* p){ if (p) p->destroyInstance("HDR provider"); }
                );

        ExpSettingParam_T   expParam;
        hal3a->send3ACtrl(E3ACtrl_EnableAIS, 1, 0);
        hal3a->send3ACtrl(E3ACtrl_GetExposureInfo,  (MINTPTR)&expParam, 0);  // for update info in ZSD mode
        hal3a->send3ACtrl(E3ACtrl_GetExposureParam, (MINTPTR)&capParam, 0);
        hal3a->send3ACtrl(E3ACtrl_EnableAIS, 0, 0);

        // TODO: Need customer to implement exposure parameter
        if (ev < 0) {
            double tempGain     = (double) (capParam.u4AfeGain)
                                           * pow(2, (((double) ev / 10)));
            double tempExposure = (double) capParam.u4Eposuretime;

            if (tempGain < 1024) {
                tempExposure = (tempExposure / 1024) * tempGain;
                tempGain     = 1024;
            }

            capParam.u4Eposuretime = tempExposure;
            capParam.u4AfeGain     = tempGain;
        }

        dumpCaptureParam(capParam, "AINR HDR captureParam");
    }

    void dumpCaptureParam(
        const CaptureParam_T &capParam, const char* msg)
    {
        if (msg) ainrLogD("========= %s =========", msg);
        ainrLogD("u4ExposureMode  (%u)", capParam.u4ExposureMode);
        ainrLogD("u4Eposuretime   (%u)", capParam.u4Eposuretime);
        ainrLogD("u4AfeGain       (%u)", capParam.u4AfeGain);
        ainrLogD("u4IspGain       (%u)", capParam.u4IspGain);
        ainrLogD("u4RealISO       (%u)", capParam.u4RealISO);
        ainrLogD("u4FlareGain     (%u)", capParam.u4FlareGain);
        ainrLogD("u4FlareOffset   (%u)", capParam.u4FlareOffset);
        ainrLogD("i4LightValue_x10(%d)", capParam.i4LightValue_x10);
    }

    void updatePerFrameMetadata(MINT32 sensorId, const AinrCtrler* ctrler, IMetadata* pAppMeta, IMetadata* pHalMeta, bool bLastFrame)
    {
        CAM_ULOGM_APILIFE();
        // Set scene mode for manual AE
        IMetadata::setEntry<MUINT8>(pAppMeta, MTK_CONTROL_SCENE_MODE, MTK_CONTROL_SCENE_MODE_HDR);

        CaptureParam_T capParam;
        int ev = 0;

        if(bLastFrame) {
            ev = -10;
        }

        getCaptureParam(sensorId, capParam, ev);

        IMetadata::Memory capParams;
        capParams.resize(sizeof(CaptureParam_T));
        memcpy(capParams.editArray(), &capParam, sizeof(CaptureParam_T));
        IMetadata::setEntry<IMetadata::Memory>(
                pHalMeta, MTK_3A_AE_CAP_PARAM, capParams);

        {
            MUINT8 bOriFocusPause  = 0;
            if ( !IMetadata::getEntry<MUINT8>(pHalMeta, MTK_FOCUS_PAUSE, bOriFocusPause) ) {
                ainrLogW("%s: cannot retrieve MTK_FOCUS_PAUSE from HAL metadata, assume "\
                        "it to 0", __FUNCTION__);
            }

            // pause AF for (N - 1) frames and resume for the last frame
            IMetadata::setEntry<MUINT8>(pHalMeta, MTK_FOCUS_PAUSE, bLastFrame ? bOriFocusPause : 1);
            IMetadata::setEntry<MUINT8>(pHalMeta, MTK_HAL_REQUEST_REQUIRE_EXIF, 1);
            IMetadata::setEntry<MUINT8>(pHalMeta, MTK_HAL_REQUEST_DUMP_EXIF, 1);
        }

        //AE Flare Enable, due to MTK_CONTROL_AE_MODE_OFF will disable AE_Flare
        // MFNR must set MTK_3A_FLARE_IN_MANUAL_CTRL_ENABLE to enable AE_Flare
        // TODO: hal3a need to implement this command for each platform.
        {
            IMetadata::setEntry<MBOOL>(pHalMeta, MTK_3A_FLARE_IN_MANUAL_CTRL_ENABLE, MTRUE);
        }

    }

    virtual void abort(vector<RequestPtr>& pRequests)
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

    AINR_HDRPlugin()
        : mRealIso(0)
        , mShutterTime(0)
        , mZSDMode(MFALSE)
        , mFlashOn(MFALSE)
        , mUniqueKey(0)
        , mMfbMode(AinrMode_Off)
        , mPluginCB(nullptr)
        , mShotCount(0)
    {
        CAM_ULOGM_APILIFE();
        mEnable        = ::property_get_int32("vendor.debug.camera.ainrhdr.enable", -1);
        mDump          = ::property_get_int32("vendor.debug.camera.ainr.dump", 0);
    };

    virtual ~AINR_HDRPlugin()
    {
        CAM_ULOGM_APILIFE();
        removeAllCtrlers();
    };
private:
    // Example code for scene decision
    bool isSceneHDR(IMetadata* pAppMeta) {
        if(pAppMeta == nullptr) {
            ainrLogE("AppMeta is nullptr");
            return false;
        }

        MUINT8 sceneMode = 0;

        IMetadata::getEntry<MUINT8>(
            pAppMeta, MTK_CONTROL_SCENE_MODE, sceneMode);

        if( sceneMode == MTK_CONTROL_SCENE_MODE_HDR ) {
            ainrLogD("It is HDR scene");
            return true;
        } else {
            ainrLogD("scene mode is not HDR is (%d)", sceneMode);
            return false;
        }
    }
    // End example
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


        AinrMode ainrMode = AinrMode_NormalAinr;

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
        mCtrlerContainer[mUniqueKey] = std::shared_ptr<AinrCtrler>(new AinrCtrler(mUniqueKey, sensorId, mMfbMode, mRealIso, mShutterTime, mFlashOn));

        ainrLogD("Create ctrler: %d", mUniqueKey);
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
        return mCtrlerContainer[key];
    };

    std::vector<RequestPtr> *getRequests(MINT32 key)
    {
        CAM_ULOGM_APILIFE();

        std::lock_guard<decltype(mReqTableMx)> lk(mReqTableMx);
        std::vector<RequestPtr> defaultRequests;

        auto find = mReqTable.find(key);

        if(find != mReqTable.end()) {
            return &find->second;
        } else {
            ainrLogE("Can not get requests from table");
            assert(0);
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

    void bufferDump(MINT32 sensorId, IMetadata *halMeta, IImageBuffer* buff, RAW_PORT type, const char *pUserString){
        // dump input buffer
        char                      fileName[512];
        FILE_DUMP_NAMING_HINT     dumpNamingHint;
        //
        MUINT8 ispProfile = NSIspTuning::EIspProfile_Capture;

        if(!halMeta || !buff) {
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
        dumpNamingHint.IspProfile = ispProfile; //EIspProfile_Capture;

        genFileName_RAW(fileName, sizeof(fileName), &dumpNamingHint, type, pUserString);
        buff->saveToFile(fileName);
    }
private:

    //
    int                             mEnable;
    int                             mDump;
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

    // Plugin callback
    RequestCallbackPtr              mPluginCB;

    // std function
    std::function<void(int32_t, int32_t)>
                                    mCbFunction;
    // Requests table
    std::mutex                      mReqTableMx;
    std::unordered_map< MINT32
        , std::vector<RequestPtr> > mReqTable;

    // shot count
    std::atomic<int>                mShotCount;

    // Ctrler container
    std::unordered_map<uint32_t, std::shared_ptr<AinrCtrler>>
                                    mCtrlerContainer;
    // JobQueue
    std::shared_ptr<
        NSCam::JobQueue<void()>
    >                               mMinorJobQueue;
};

REGISTER_PLUGIN_PROVIDER(MultiFrame, AINR_HDRPlugin);
