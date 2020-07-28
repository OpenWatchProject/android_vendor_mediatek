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
#include <algorithm>

#include "P2_StreamingProcessor.h"
#include "P2_Util.h"

#include <mtkcam/utils/std/ULog.h>
using namespace NSCam::Utils::ULog;

#define P2_STREAMING_THREAD_NAME "p2_streaming"

#define IDLE_WAIT_TIME_MS 66

using NSCam::NSCamFeature::NSFeaturePipe::IStreamingFeaturePipe;
using NSCam::NSCamFeature::NSFeaturePipe::FeaturePipeParam;
using NSCam::NSCamFeature::NSFeaturePipe::PathType;
using NSCam::NSCamFeature::NSFeaturePipe::SFPSensorInput;
using NSCam::NSCamFeature::NSFeaturePipe::SFPIOMap;
using NSCam::NSCamFeature::NSFeaturePipe::SFPSensorTuning;
using NSCam::NSCamFeature::NSFeaturePipe::SFPOutput;
using NSCam::NSCamFeature::VarMap;
using namespace NSCam::NR3D;
using NSCam::NSCamFeature::NSFeaturePipe::HAS_FSC;
using NSCam::NSCamFeature::NSFeaturePipe::OFFSET_FSC;
using NSCam::SecType;

namespace P2
{

#include "P2_DebugControl.h"
#define P2_CLASS_TAG    StreamingProcessor
#define P2_TRACE        TRACE_STREAMING_PROCESSOR
#include "P2_LogHeader.h"

CAM_ULOG_DECLARE_MODULE_ID(MOD_P2_STR_PROC);

StreamingProcessor::StreamingProcessor()
    : Processor(P2_STREAMING_THREAD_NAME)
    , m3dnrLogLevel(0)
    , mDebugDrawCropMask(0)
    , mspFDContainer(NULL)
{
//    MY_LOG_FUNC_ENTER();
    this->setIdleWaitMS(IDLE_WAIT_TIME_MS);
    mspFDContainer = IFDContainer::createInstance("StreamingProcessor", IFDContainer::eFDContainer_Opt_Read);
    if (mspFDContainer == NULL)
    {
        MY_LOGW("!!warn: IFDContainer::createInstance fail");
    }

    MY_LOG_FUNC_EXIT();
}

StreamingProcessor::~StreamingProcessor()
{
//    MY_LOG_S_FUNC_ENTER(mLog);
    this->uninit();
    mspFDContainer = NULL;
    MY_LOG_S_FUNC_EXIT(mLog);
}

MBOOL StreamingProcessor::onInit(const P2InitParam &param)
{
    ILog log = param.mP2Info.mLog;
    MY_LOG_S_FUNC_ENTER(log);
    P2_CAM_TRACE_NAME(TRACE_DEFAULT, "P2_Streaming:init()");

    MBOOL ret = MFALSE;

    mP2Info = param.mP2Info;
    mLog = mP2Info.mLog;
    mDebugDrawCropMask = property_get_int32("vendor.debug.camera.drawcrop.mask", 0);
    mBypssSFP = property_get_bool("vendor.debug.p2s.sfp.bypass", false);
    ret = initFeaturePipe(mP2Info.getConfigInfo()) && init3A();
    if( ret )
    {
        if( (mP2Info.getConfigInfo().mUsageHint.m3DNRMode &
            (E3DNR_MODE_MASK_UI_SUPPORT | E3DNR_MODE_MASK_HAL_FORCE_SUPPORT)) != 0 )
        {
            init3DNR();
        }
        mRssoMgr.init(mP2Info.getConfigInfo().mAllSensorID);
    }
    else
    {
        uninitFeaturePipe();
        uninit3A();
    }

    MY_LOG_S_FUNC_EXIT(log);
    return ret;
}

MVOID StreamingProcessor::onUninit()
{
    MY_LOG_S_FUNC_ENTER(mLog);
    P2_CAM_TRACE_NAME(TRACE_DEFAULT, "P2_Streaming:uninit()");
    mRssoMgr.uninit();
    uninit3DNR();
    uninitFeaturePipe();
    uninit3A();
    MY_LOG_S_FUNC_EXIT(mLog);
}

MVOID StreamingProcessor::onThreadStart()
{
    MY_LOG_S_FUNC_ENTER(mLog);
    P2_CAM_TRACE_NAME(TRACE_DEFAULT, "P2_Streaming:threadStart()");
    MY_LOG_S_FUNC_EXIT(mLog);
}

MVOID StreamingProcessor::onThreadStop()
{
    MY_LOG_S_FUNC_ENTER(mLog);
    P2_CAM_TRACE_NAME(TRACE_DEFAULT, "P2_Streaming:threadStop()");
    MY_LOG_S_FUNC_EXIT(mLog);
}

MBOOL StreamingProcessor::onConfig(const P2ConfigParam &param)
{
    MY_LOG_S_FUNC_ENTER(mLog);
    MBOOL ret = MTRUE;
    P2_CAM_TRACE_NAME(TRACE_DEFAULT, "P2_Streaming:config()");
    if( needReConfig(mP2Info.getConfigInfo(), param.mP2Info.getConfigInfo()) )
    {
        android::Mutex::Autolock _lock(mPayloadMutex);
        if( mPayloadList.size() )
        {
            MY_S_LOGE(mLog, "Config called before p2 is empty, size=%zu", mPayloadList.size());
            ret = MFALSE;
        }
        else
        {
            uninitFeaturePipe();
            ret = initFeaturePipe(param.mP2Info.getConfigInfo());
        }
    }
    if( ret )
    {
        mP2Info = param.mP2Info;
    }

    calcEISRatio(param.mP2Info.getConfigInfo().mUsageHint.mStreamingSize,
                 param.mP2Info.getConfigInfo().mAppStreamInfo);

    MY_S_LOGD(mLog, "supportPQ=%d, supportClearZoom=%d, supportDRE=%d, supportHFG=%d",
        mP2Info.getConfigInfo().mSupportPQ,
        mP2Info.getConfigInfo().mSupportClearZoom,
        mP2Info.getConfigInfo().mSupportDRE,
        mP2Info.getConfigInfo().mSupportHFG
        );

    MY_LOG_S_FUNC_EXIT(mLog);
    return ret;
}

MBOOL StreamingProcessor::makeRequestPacks(const vector<sp<P2Request>> &requests, vector<sp<P2RequestPack>>& rReqPacks) const
{
    const ILog &log = requests.empty() ? mLog : requests.front()->mLog;
    TRACE_S_FUNC_ENTER(log);
    MBOOL debugLog = (log.getLogLevel() >= 2);
    // index by output stream
    class MergeHelper
    {
    public:
        MBOOL isInputSubsetOf(const ILog& log, const MBOOL printLog, const MergeHelper& base)
        {
            for( const auto& myInput : mReq->mImg )
            {
                if(mReq->mImg[myInput.first] == NULL)
                    continue;
                const ID_IMG mirrorID = myInput.second->getMirrorID();
                MY_S_LOGD_IF(printLog, log, "myInput(%d),mirror(%d) (%s)", myInput.first, mirrorID, P2Img::getName(myInput.first));
                if( mirrorID == IN_RESIZED || mirrorID == IN_FULL || mirrorID == IN_LCSO || mirrorID == IN_LCSHO)
                {
                    if( base.mReq->mImg[myInput.first] == NULL )
                    {
                        MY_S_LOGD_IF(printLog, log, "myInput(%d)(%s) can not be found in base request !", myInput.first, P2Img::getName(myInput.first));
                        return MFALSE;
                    }
                }
            }
            return MTRUE;
        };
        MergeHelper(const sp<P2Request>& pReq, const MUINT32 outputIndex)
        : mReq(pReq)
        , mOutIndex(outputIndex)
        {};
        MergeHelper(const sp<P2Request>& pReq, const ID_IMG imgId)
        : mReq(pReq)
        , mId(imgId)
        {};
    public:
        sp<P2Request> mReq = nullptr;
        MUINT32 mOutIndex = -1;
        MBOOL merged = false;
        ID_IMG mId = OUT_YUV;
    };

    vector<MergeHelper> outputMap;
    for( const auto& req : requests )
    {
        if(debugLog)
        {
            req->dump();
        }
        if( !( req->isValidMeta(IN_APP) || req->hasValidMirrorMeta(IN_APP_PHY) )
              || !(req->hasValidMirrorMeta(IN_P1_HAL)) )
        {
            MY_S_LOGW(log, "Meta check failed: inApp(%d) inMirrorHal(%d) inMirrorAppPhy(%d)",
                    req->isValidMeta(IN_APP), req->hasValidMirrorMeta(IN_P1_HAL), req->hasValidMirrorMeta(IN_APP_PHY));
            continue;
        }
        if( !req->hasInput() || !req->hasOutput())
        {
            MY_S_LOGW(log, "req I/O Failed! hasI/O(%d/%d)", req->hasInput(), req->hasOutput());
            continue;
        }

        if( !isRequestValid(req) )
        {
            continue;
        }

        MUINT32 n = req->mImgOutArray.size();
        for( MUINT32 i = 0 ; i < n ; ++i )
        {
            MergeHelper outReq(req, i);
            outputMap.push_back(outReq);
        }

        if(req->mImg[OUT_FD] != NULL)
        {
            MergeHelper outReq(req, OUT_FD);
            outputMap.push_back(outReq);
        }
    }
    MY_S_LOGD_IF(debugLog, log, "#outputMap(%zu)", outputMap.size());

    // sort by number of inputs
    std::sort(outputMap.begin(), outputMap.end(),
        [](const auto& lhs, const auto& rhs)
        {
            return lhs.mReq->mImg.size() > rhs.mReq->mImg.size();
        }
    );

    // merge requests
    // Current limitation : being merged request must has only 1 output buffer....
    int n = outputMap.size();
    for( int base = 0 ; base < n ; ++base )
    {
        if( outputMap[base].merged )
        {
            continue;
        }
        MY_S_LOGD_IF(debugLog, log, "outputMap[%d] creates new pack", base);
        sp<P2RequestPack> reqPack = new P2RequestPack(log, outputMap[base].mReq, mP2Info.getConfigInfo().mAllSensorID);
        for( int target = n-1 ; target > base ; --target )
        {
            MY_S_LOGD_IF(debugLog, log, "checking target outputMap[%d]", target);
            MergeHelper &output = outputMap[target];
            if( !output.merged && output.isInputSubsetOf(log, debugLog, outputMap[base]) )
            {
                MY_S_LOGD_IF(debugLog, log, "target outputMap[%d] is subset of [%d]", target, base);
                reqPack->addOutput(output.mReq, output.mOutIndex);
                outputMap[target].merged = true;
            }
        }
        rReqPacks.push_back(reqPack);
    }
    TRACE_S_FUNC_EXIT(log);
    return (rReqPacks.size() > 0);
}

MBOOL StreamingProcessor::prepareInputs(const sp<Payload>& payload)
{
    const ILog &log = payload->mLog;
    TRACE_S_FUNC_ENTER(log);
    for(const auto& partialPayload : payload->mPartialPayloads)
    {
        auto& reqPack = partialPayload->mRequestPack;
        auto& inputs = reqPack->mInputs;
        prepareSMVR(reqPack, log);
        for(auto& it : inputs)
        {
            // LMV
            it.setUseLMV(isEIS12());
            // Feature Params
            prepareFeatureParam(it, log);
        }
    }
    TRACE_S_FUNC_EXIT(log);
    return MTRUE;
}

MBOOL StreamingProcessor::isNeedRSSO(P2Util::SimpleIn& input) const
{
    MBOOL ret = MFALSE;
    if( (input.mRequest->mP2Pack.getFrameData().mIsRecording &&
        EIS_MODE_IS_EIS_30_ENABLED(mPipeUsageHint.mEISInfo.mode) &&
        EIS_MODE_IS_EIS_IMAGE_ENABLED(mPipeUsageHint.mEISInfo.mode)) ||
        // 3dnr rule
        ( (mPipeUsageHint.m3DNRMode & (E3DNR_MODE_MASK_HAL_FORCE_SUPPORT | E3DNR_MODE_MASK_UI_SUPPORT)) &&
           (mPipeUsageHint.m3DNRMode & E3DNR_MODE_MASK_RSC_EN) )
      )
    {
        ret = MTRUE;
    }
    return ret;
}

MBOOL StreamingProcessor::prepareRSSO(const sp<Payload>& payload)
{
    TRACE_S_FUNC_ENTER(payload->mLog);

    for(const auto& partialPayload : payload->mPartialPayloads)
    {
        auto& reqPack = partialPayload->mRequestPack;
        auto& inputs = reqPack->mInputs;
        for(auto& it : inputs)
        {
            if( !isNeedRSSO(it) )
            {
                mRssoMgr.updateRSSO(payload->mLog, nullptr, it.getSensorId());
            }
            else
            {
                RSSOMgr::Cache cache;
                cache = mRssoMgr.updateRSSO(payload->mLog, it.mRSSO, it.getSensorId());
                it.mPreRSSO = cache.mPrev;
                if( isValid(it.mRSSO) )
                {
                    IImageBuffer *cRSSO = it.mRSSO->getIImageBufferPtr();
                    if( cRSSO != NULL )
                    {
                        cRSSO->setTimestamp(it.mRequest->mP2Pack.getSensorData().mP1TS);
                    }
                    it.mFeatureParam.setVar<IImageBuffer*>(SFP_VAR::CURR_RSSO, cRSSO);
                }
                IImageBuffer *pRSSO = cache.getIImageBufferPtr();
                if( pRSSO )
                {
                    it.mFeatureParam.setVar<IImageBuffer*>(SFP_VAR::PREV_RSSO, pRSSO);
                }
            }
        }
    }
    TRACE_S_FUNC_EXIT(payload->mLog);
    return MTRUE;
}

MBOOL StreamingProcessor::isRequestValid(const sp<P2Request>& request) const
{
    MBOOL valid = MTRUE;
    if( !isRequestValid_VSDOF(request) )
    {
        MY_S_LOGW(request->mLog, "request drop by VSDOF");
        valid = MFALSE;
    }
    return valid;
}

MBOOL StreamingProcessor::prepareOutputs(const sp<Payload>& payload) const
{
    P2_CAM_TRACE_CALL(TRACE_ADVANCED);
    TRACE_S_FUNC_ENTER(payload->mLog);
    MBOOL isFdDataOk = payload->prepareFdData(mP2Info, mspFDContainer.get());
    if(!isFdDataOk)
    {
        TRACE_S_FUNC(payload->mLog, "FD Data not OK!");
    }

    for(auto& partialPayload : payload->mPartialPayloads)
    {
        auto& reqPack = partialPayload->mRequestPack;
        auto& inputs = reqPack->mInputs;
        for(auto& it : reqPack->mOutputs)
        {
            for(auto& out : it.second)
            {
                // Crop
                const sp<Cropper> cropper = reqPack->mMainRequest->getCropper(out.getSensorId());
                auto& in = inputs[reqPack->mSensorInputMap[out.getSensorId()]];
                MUINT32 cropFlag = 0;
                cropFlag |= in.isResized() ? Cropper::USE_RESIZED : 0;
                cropFlag |= in.useLMV() ? Cropper::USE_EIS_12 : 0;
                cropFlag |= in.useCropRatio() ? Cropper::USE_CROP_RATIO : 0;
                MSizeF cropRatio = in.useCropRatio() ? in.getCropRatio() : MSizeF(0.0f, 0.0f);
                MUINT32 featureMask = in.mFeatureParam.mFeatureMask;
                MUINT32 dmaConstrain = (out.isMDPOutput() && HAS_FSC(featureMask)) ? DMAConstrain::NONE : DMAConstrain::ALIGN2BYTE;
                dmaConstrain |= isSubPixelEnabled() ? DMAConstrain::NONE : DMAConstrain::NOSUBPIXEL;
                MRectF cropF = cropper->calcViewAngleF(payload->mLog, out.mImg->getTransformSize(), cropFlag,
                    cropRatio.w, cropRatio.h, dmaConstrain);
                out.mCrop = cropF;
                out.mDMAConstrain = dmaConstrain;
                out.mImg->setImgInfo(MSize(0,0), in.mRequest->mP2Pack.getSensorData().mP1TSVector);

                // PQ
                if( out.mP2Obj.toPtrTable().hasPQ )
                {
                    const P2Pack &p2Pack = in.mRequest->mP2Pack;
                    P2Util::xmakeDpPqParam(p2Pack, out, payload->mpFdData);
                }
                // Set FD Crop
                if(out.isFD())
                {
                    MRect activeCrop = cropper->toActive(cropF, in.isResized());
                    in.mFeatureParam.setVar<MRect>(SFP_VAR::FD_CROP_ACTIVE_REGION, activeCrop);
                }
            }
        }
    }
    TRACE_S_FUNC_EXIT(payload->mLog);
    return MTRUE;
}

MVOID StreamingProcessor::releaseResource(const vector<sp<P2Request>> &requests, MUINT32 res) const
{
    if(!requests.empty())
    {
        sp<P2Request> firstReq = requests.front();
        firstReq->beginBatchRelease();
        for(auto&& req : requests)
        {
            req->releaseResource(res);
        }
        firstReq->endBatchRelease();
    }
}

MVOID StreamingProcessor::makeSFPIOOuts(const sp<Payload>& payload, const ERequestPath& path, FeaturePipeParam& featureParam) const
{
    TRACE_S_FUNC_ENTER(payload->mLog);
    unordered_map<MUINT32,sp<P2Request>>& paths = payload->mReqPaths[path];
    for( MUINT32 sensorID : mP2Info.getConfigInfo().mAllSensorID )
    {
        if( paths.find(sensorID) == paths.end() )
        {
            continue;
        }
        sp<P2Request>& request = paths[sensorID];
        MBOOL found = MFALSE;
        for(const auto &partialPayload : payload->mPartialPayloads)
        {
            auto& reqPack = partialPayload->mRequestPack;
            if( reqPack->contains(request) )
            {
                SFPIOMap sfpIO;
                // input tuning
                for(const P2Util::SimpleIn &in : reqPack->mInputs)
                {
                    SFPSensorTuning sensorTuning;
                    if( in.isResized() )
                    {
                        sensorTuning.addFlag(SFPSensorTuning::Flag::FLAG_RRZO_IN);
                    }
                    else
                    {
                        sensorTuning.addFlag(SFPSensorTuning::Flag::FLAG_IMGO_IN);
                    }
                    if( isValid(in.mLCEI) )
                    {
                        sensorTuning.addFlag(SFPSensorTuning::Flag::FLAG_LCSO_IN);
                    }
                    if( isValid(in.mLCSHO) )
                    {
                        sensorTuning.addFlag(SFPSensorTuning::Flag::FLAG_LCSHO_IN);
                    }
                    // tuning condition: 3DNR only support general path and master camera
                    if( NSCam::NSCamFeature::NSFeaturePipe::HAS_3DNR(featureParam.mFeatureMask)
                        && ((ERequestPath::eGeneral != path) || (in.getSensorId() != payload->mMasterID)) )
                    {
                        sensorTuning.addFlag(SFPSensorTuning::Flag::FLAG_FORCE_DISABLE_3DNR);
                        TRACE_S_FUNC(payload->mLog, "set SFPSensorTuning FLAG_FORCE_DISABLE_3DNR at path(%d) SensorId(%d)"
                            , path, in.getSensorId());
                    }
                    sfpIO.addInputTuning(in.getSensorId(), sensorTuning);
                    // TODO check has app input override metadata or not
                }
                // outputs
                for(const P2Util::SimpleOut &out : reqPack->mOutputs[request.get()])
                {
                    auto getOutTargetType = [&](const P2Util::SimpleOut& out)
                    {
                        if( ERequestPath::ePhysic == path )
                        {
                            return SFPOutput::OutTargetType::OUT_TARGET_PHYSICAL;
                        }
                        if( ERequestPath::eLarge == path )
                        {
                            return SFPOutput::OutTargetType::OUT_TARGET_UNKNOWN;
                        }
                        if( out.isRecord() )
                        {
                            return SFPOutput::OutTargetType::OUT_TARGET_RECORD;
                        }
                        if( out.isFD() )
                        {
                            return SFPOutput::OutTargetType::OUT_TARGET_FD;
                        }
                        if( out.isDisplay() )
                        {
                            return SFPOutput::OutTargetType::OUT_TARGET_DISPLAY;
                        }
                        return SFPOutput::OutTargetType::OUT_TARGET_UNKNOWN;
                    };
                    SFPOutput sfpOut(
                        out.mImg->getIImageBufferPtrs(),
                        out.mImg->getTransform(),
                        getOutTargetType(out)
                    );
                    sfpOut.mCropRect = out.mCrop;
                    sfpOut.mDMAConstrain = out.mDMAConstrain;
                    sfpOut.mCropDstSize = out.mImg->getTransformSize();
                    sfpOut.mpPqParam = out.mP2Obj.toPtrTable().pqParam;
                    sfpOut.mpDpPqParam = out.mP2Obj.toPtrTable().pqWDMA;
                    sfpIO.addOutput(sfpOut);
                }

                // metadata
                sfpIO.mHalOut = request->getMetaPtr(OUT_HAL);
                if(path == ERequestPath::ePhysic)
                {
                    sfpIO.mAppOut = request->getMetaPtr(OUT_APP_PHY, sensorID);
                }
                else
                {
                    sfpIO.mAppOut = request->getMetaPtr(OUT_APP);
                }

                switch(path)
                {
                    case ERequestPath::eGeneral:
                        sfpIO.mPathType = PathType::PATH_GENERAL;
                        featureParam.mSFPIOManager.addGeneral(sfpIO);
                        break;
                    case ERequestPath::ePhysic:
                        sfpIO.mPathType = PathType::PATH_PHYSICAL;
                        featureParam.mSFPIOManager.addPhysical(sensorID, sfpIO);
                        break;
                    case ERequestPath::eLarge:
                        sfpIO.mPathType = PathType::PATH_LARGE;
                        featureParam.mSFPIOManager.addLarge(sensorID, sfpIO);
                        break;
                    default:
                        MY_S_LOGE(payload->mLog, "unknow path(%d)", path);
                }

                found = MTRUE;
                break;
            }
        }

        if( !found )
        {
            MY_S_LOGE(payload->mLog, "can not find path(%d) for sensor(%d) !!", path, sensorID);
        }
    }
    TRACE_S_FUNC_EXIT(payload->mLog);

}


MBOOL StreamingProcessor::makeSFPIOMgr(const sp<Payload>& payload) const
{
    TRACE_S_FUNC_ENTER(payload->mLog);

    FeaturePipeParam& featureParam = *(payload->getMainFeaturePipeParam());
    featureParam.setVar<MUINT32>(SFP_VAR::DUALCAM_FOV_MASTER_ID, payload->mMasterID);

    // add sensor input
    unordered_map<MUINT32, SFPSensorInput> sensorInputs;
    for(const auto& partialPayload : payload->mPartialPayloads)
    {
        auto& reqPack = partialPayload->mRequestPack;
        auto& inputs = reqPack->mInputs;
        for(auto& in : inputs)
        {
            MUINT32 sID = in.getSensorId();
            if( in.isResized() )
            {
                sensorInputs[sID].mRRZO = toIImageBufferPtrs(in.mIMGI);
            }
            else
            {
                sensorInputs[sID].mIMGO = toIImageBufferPtrs(in.mIMGI);
            }

            sensorInputs[sID].mLCSO = toIImageBufferPtrs(in.mLCEI);
            sensorInputs[sID].mLCSHO = toIImageBufferPtrs(in.mLCSHO);
            sensorInputs[sID].mPrvRSSO = toIImageBufferPtrs(in.mPreRSSO);
            sensorInputs[sID].mCurRSSO = toIImageBufferPtrs(in.mRSSO);
            sensorInputs[sID].mFullYuv = toIImageBufferPtrs(in.mFullYuv);
            sensorInputs[sID].mRrzYuv1 = toIImageBufferPtrs(in.mRrzYuv1);
            sensorInputs[sID].mRrzYuv2 = toIImageBufferPtrs(in.mRrzYuv2);

            sensorInputs[sID].mHalIn = in.mRequest->getMetaPtr(IN_P1_HAL, sID);
            sensorInputs[sID].mAppIn = in.mRequest->getMetaPtr(IN_APP, sID);
            sensorInputs[sID].mAppInOverride = in.mRequest->getMetaPtr(IN_APP_PHY, sID);
            sensorInputs[sID].mAppDynamicIn = in.mRequest->getMetaPtr(IN_P1_APP, sID);

            // combine slave's FPP into master's
            if( sID != payload->mMasterID )
            {
                featureParam.addSlaveParam(sID, in.mFeatureParam);
                featureParam.setVar<MUINT32>(SFP_VAR::DUALCAM_FOV_SLAVE_ID, sID);
            }
        }
    }

    for(auto& it : sensorInputs)
    {
        featureParam.mSFPIOManager.addInput(it.first, it.second);
    }

    makeSFPIOOuts(payload, ERequestPath::eGeneral, featureParam);
    makeSFPIOOuts(payload, ERequestPath::ePhysic, featureParam);
    makeSFPIOOuts(payload, ERequestPath::eLarge, featureParam);

    TRACE_S_FUNC_EXIT(payload->mLog);
    return MTRUE;
}

sp<StreamingProcessor::Payload> StreamingProcessor::makePayLoad(const vector<sp<P2Request>> &requests, const vector<sp<P2RequestPack>>& reqPacks) const
{
    const ILog &log = requests.empty() ? mLog : requests.front()->mLog;
    TRACE_S_FUNC_ENTER(log);
    if( requests.empty() || reqPacks.empty() )
    {
        MY_S_LOGE(log, "empty reqs(%d) reqPacks(%d)!!", requests.empty(), reqPacks.empty());
        return nullptr;
    }

    MUINT32 masterID = requests.front()->mP2Pack.getFrameData().mMasterSensorID;
    MINT64 p1TS = requests.front()->mP2Pack.getSensorData().mP1TS;
    sp<Payload> payload = new Payload(const_cast<StreamingProcessor*>(this), log, masterID);

    payload->addRequests(requests);
    payload->addRequestPacks(reqPacks);

    TRACE_S_FUNC(log,"#Requests(%zu) merged into #RequestsPacks(%zu), p1Ts=%016" PRId64, requests.size(), reqPacks.size(), p1TS);

    TRACE_S_FUNC_EXIT(log);
    return payload;
}

MBOOL StreamingProcessor::onEnque(const vector<sp<P2Request>> &requests)
{
    P2_CAM_TRACE_CALL(TRACE_DEFAULT);
    const ILog &log = requests.empty() ? mLog : requests.front()->mLog;
    TRACE_S_FUNC_ENTER(log);
    vector<sp<P2RequestPack>> requestPacks;
    sp<Payload> payload = NULL;

    P2_CAM_TRACE_BEGIN(TRACE_ADVANCED, "P2_Streaming:onEnque->makeReqPacks_PayLoad");
    MBOOL ret = makeRequestPacks(requests, requestPacks);
    payload = makePayLoad(requests, requestPacks);
    ret &= (payload != nullptr);
    P2_CAM_TRACE_END(TRACE_ADVANCED);

    if(!ret)
    {
        MY_S_LOGW(log, "make request pack or payload failed !! Drop Frame.");
        CAM_ULOG_DISCARD(MOD_P2_STR_PROC, REQ_P2_STR_REQUEST, log.getLogFrameID());
        releaseResource(requests, P2Request::RES_ALL);
        return MFALSE;
    }

    P2_CAM_TRACE_BEGIN(TRACE_ADVANCED, "P2_Streaming:onEnque->->prepareInputsInfo");
    ret &= ret && checkFeaturePipeParamValid(payload)
            && prepareInputs(payload)
            && prepareRSSO(payload)
            && prepareOutputs(payload);
    P2_CAM_TRACE_END(TRACE_ADVANCED);
    if(!ret)
    {
        MY_S_LOGE(log, "prepare inputs or rsso or output failed !!");
        CAM_ULOG_DISCARD(MOD_P2_STR_PROC, REQ_P2_STR_REQUEST, log.getLogFrameID());
        return MFALSE;
    }

    releaseResource(requests, P2Request::RES_IMG);

    drawCropRegion(payload);

    if( !processP2(payload) )
    {
        MY_S_LOGE(log, "processP2 failed !!");
        return MFALSE;
    }

    TRACE_S_FUNC_EXIT(log);
    return MTRUE;
}
MBOOL StreamingProcessor::checkFeaturePipeParamValid(const sp<Payload> &payload)
{
    MBOOL ret = MFALSE;
    FeaturePipeParam* pFPP = payload->getMainFeaturePipeParam();
    if(pFPP != nullptr)
    {
       ret = MTRUE;
    }
    else
    {
        MY_S_LOGE(payload->mLog,"checkFeaturePipeParamValid return false.");
    }
    return ret;
}

MVOID StreamingProcessor::onNotifyFlush()
{
    TRACE_S_FUNC_ENTER(mLog);
    P2_CAM_TRACE_NAME(TRACE_DEFAULT, "P2_Streaming:notifyFlush()");
    if( mFeaturePipe )
    {
        mFeaturePipe->notifyFlush();
    }
    TRACE_S_FUNC_EXIT(mLog);
}

MVOID StreamingProcessor::onWaitFlush()
{
    MY_LOG_S_FUNC_ENTER(mLog);
    P2_CAM_TRACE_NAME(TRACE_DEFAULT, "P2_Streaming:waitFlush()");
    if( mFeaturePipe )
    {
        mFeaturePipe->flush();
    }
    waitFeaturePipeDone();
    mRssoMgr.releaseRSSO(mLog, RSSOMgr::POLICY_NO_CACHE);
    flushSMVRQ();
    MY_LOG_S_FUNC_EXIT(mLog);
}

MVOID StreamingProcessor::onIdle()
{
    MY_LOG_S_FUNC_ENTER(mLog);
    mRssoMgr.releaseRSSO(mLog, RSSOMgr::POLICY_COPY_CACHE);
    MY_LOG_S_FUNC_EXIT(mLog);
}

IStreamingFeaturePipe::UsageHint StreamingProcessor::getFeatureUsageHint(const P2ConfigInfo &config)
{
    TRACE_S_FUNC_ENTER(mLog);
    IStreamingFeaturePipe::UsageHint pipeUsage;
    switch( config.mP2Type )
    {
    case P2_PHOTO:
    case P2_PREVIEW:
        pipeUsage.mMode = IStreamingFeaturePipe::USAGE_P2A_FEATURE;
        break;
    case P2_CAPTURE:
    case P2_TIMESHARE_CAPTURE:
        pipeUsage.mMode = IStreamingFeaturePipe::USAGE_P2A_PASS_THROUGH;
        break;
    case P2_HS_VIDEO:
        pipeUsage.mMode = IStreamingFeaturePipe::USAGE_FULL;
        MY_S_LOGE(mLog, "Slow Motion should NOT use StreamingProcessor!!");
        break;
    case P2_BATCH_SMVR:
        pipeUsage.mMode = IStreamingFeaturePipe::USAGE_BATCH_SMVR;
        break;
    case P2_VIDEO:
        pipeUsage.mMode = IStreamingFeaturePipe::USAGE_FULL;
        break;
    case P2_DUMMY:
        pipeUsage.mMode = IStreamingFeaturePipe::USAGE_DUMMY;
        MY_S_LOGD(mLog, "Using Dummy streaming feature pipe");
        break;
    case P2_UNKNOWN:
    default:
        pipeUsage.mMode = IStreamingFeaturePipe::USAGE_FULL;
        break;
    }

    pipeUsage.mStreamingSize = config.mUsageHint.mStreamingSize;
    if( pipeUsage.mStreamingSize.w == 0 || pipeUsage.mStreamingSize.h == 0 )
    {
        MY_S_LOGW(mLog, "no size in UsageHint");
    }
    pipeUsage.mOutSizeVector = config.mUsageHint.mOutSizeVector;

    // Set EIS Info
    pipeUsage.mEISInfo = config.mUsageHint.mEisInfo;
    pipeUsage.m3DNRMode = config.mUsageHint.m3DNRMode;
    pipeUsage.mDSDNParam = queryDSDNParam(config);
    pipeUsage.mFSCMode = config.mUsageHint.mFSCMode;
    pipeUsage.mDualMode = config.mUsageHint.mDualMode;
    pipeUsage.mSMVRSpeed = config.mUsageHint.mSMVRSpeed;
    pipeUsage.mSecType = config.mUsageHint.mSecType;
    if ( pipeUsage.mSecType != SecType::mem_normal &&
         mP2Info.getPlatInfo() && !mP2Info.getPlatInfo()->supportSecure3DNR() )
    {
        pipeUsage.m3DNRMode = 0;
        MY_S_LOGW(mLog, "3dnr: disable 3DNR due to no secure support");
    }
    pipeUsage.mUseTSQ = config.mUsageHint.mUseTSQ;
    pipeUsage.mEnlargeRsso = config.mUsageHint.mEnlargeRsso;
    pipeUsage.mTP = config.mUsageHint.mTP;
    pipeUsage.mTPMarginRatio = config.mUsageHint.mTPMarginRatio;
    pipeUsage.mAppSessionMeta = config.mUsageHint.mAppSessionMeta;
    pipeUsage.mAllSensorIDs = config.mAllSensorID;
    pipeUsage.mResizedRawMap = config.mUsageHint.mResizedRawMap;
    pipeUsage.mSensorModule = config.mUsageHint.mSensorModule;

    pipeUsage.mOutCfg.mMaxOutNum = config.mUsageHint.mOutCfg.mMaxOutNum;
    pipeUsage.mOutCfg.mHasPhysical = config.mUsageHint.mOutCfg.mHasPhysical;
    pipeUsage.mOutCfg.mHasLarge = config.mUsageHint.mOutCfg.mHasLarge;
    pipeUsage.mOutCfg.mFDSize = config.mUsageHint.mOutCfg.mFDSize;
    pipeUsage.mOutCfg.mVideoSize = config.mUsageHint.mOutCfg.mVideoSize;

    pipeUsage.mTPICustomConfig = config.mUsageHint.mTPICustomConfig;
    TRACE_S_FUNC_EXIT(mLog);
    return pipeUsage;
}

MBOOL StreamingProcessor::needReConfig(const P2ConfigInfo &oldConfig, const P2ConfigInfo &newConfig)
{
    TRACE_S_FUNC_ENTER(mLog);
    MBOOL ret = MFALSE;
    const P2UsageHint &oldHint = oldConfig.mUsageHint;
    const P2UsageHint &newHint = newConfig.mUsageHint;
    if( (newHint.mEisInfo.mode != oldHint.mEisInfo.mode) ||
        (newHint.mEisInfo.factor != oldHint.mEisInfo.factor) ||
        (newHint.mStreamingSize != oldHint.mStreamingSize) )
    {
        ret = MTRUE;
    }
    MY_S_LOGI(mLog, "re-config=%d, EISMode(%d=>%d), EISFactor(%d=>%d), Size(%dx%d=>%dx%d)", ret, oldHint.mEisInfo.mode, newHint.mEisInfo.mode, oldHint.mEisInfo.factor, newHint.mEisInfo.factor, oldHint.mStreamingSize.w, oldHint.mStreamingSize.h, newHint.mStreamingSize.w, newHint.mStreamingSize.h);
    TRACE_S_FUNC_EXIT(mLog);
    return ret;
}

MBOOL StreamingProcessor::initFeaturePipe(const P2ConfigInfo &config)
{
    TRACE_S_FUNC_ENTER(mLog);
    MBOOL ret = MFALSE;

    mPipeUsageHint = getFeatureUsageHint(config);
    P2_CAM_TRACE_BEGIN(TRACE_DEFAULT, "P2_Streaming:FeaturePipe create");
    mFeaturePipe = IStreamingFeaturePipe::createInstance(mP2Info.getConfigInfo().mMainSensorID, mPipeUsageHint);
    P2_CAM_TRACE_END(TRACE_DEFAULT);
    if( mFeaturePipe == NULL )
    {
        MY_S_LOGE(mLog, "OOM: cannot create FeaturePipe");
    }
    else
    {
        P2_CAM_TRACE_BEGIN(TRACE_DEFAULT, "P2_Streaming:FeaturePipe init");
        ret = mFeaturePipe->init(getName());
        P2_CAM_TRACE_END(TRACE_DEFAULT);
        for( MUINT32 id : mP2Info.getConfigInfo().mAllSensorID )
        {
            if(id != mP2Info.getConfigInfo().mMainSensorID)
            {
                mFeaturePipe->addMultiSensorID(id);
            }
        }
        if( !ret )
        {
            MY_S_LOGE(mLog, "FeaturePipe init failed");
        }
    }
    TRACE_S_FUNC_EXIT(mLog);
    return ret;
}

MVOID StreamingProcessor::uninitFeaturePipe()
{
    TRACE_S_FUNC_ENTER(mLog);
    if( mFeaturePipe )
    {
        mFeaturePipe->uninit(getName());
        mFeaturePipe->destroyInstance();
        mFeaturePipe = nullptr;
    }
    TRACE_S_FUNC_EXIT(mLog);
}

MBOOL StreamingProcessor::init3A()
{
    TRACE_S_FUNC_ENTER(mLog);
    MBOOL ret = MTRUE;
    for( MUINT32 sensorID : mP2Info.getConfigInfo().mAllSensorID )
    {
        P2_CAM_TRACE_FMT_BEGIN(TRACE_DEFAULT, "P2_Streaming:3A/Isp(%u) create", sensorID);
        mHal3AMap[sensorID] = MAKE_Hal3A(sensorID, getName());
        mHalISPMap[sensorID] = MAKE_HalISP(sensorID, getName());
        P2_CAM_TRACE_END(TRACE_DEFAULT);
        if( mHal3AMap[sensorID] == nullptr )
        {
            MY_S_LOGE(mLog, "OOM: cannot create Hal3A(%u)", sensorID);
            ret = MFALSE;
        }
        if( mHalISPMap[sensorID] == nullptr )
        {
            MY_S_LOGE(mLog, "OOM: cannot create HalISP(%u)", sensorID);
            ret = MFALSE;
        }

    }
    TRACE_S_FUNC_EXIT(mLog);
    return ret;
}

MVOID StreamingProcessor::uninit3A()
{
    TRACE_S_FUNC_ENTER(mLog);
    for(auto& it : mHal3AMap)
    {
        if( it.second != nullptr )
        {
            it.second->destroyInstance(getName());
            it.second = nullptr;
        }
    }
    mHal3AMap.clear();
    for(auto& it : mHalISPMap)
    {
        if( it.second != nullptr )
        {
            it.second->destroyInstance(getName());
            it.second = nullptr;
        }
    }
    mHalISPMap.clear();
    TRACE_S_FUNC_EXIT(mLog);
}

MVOID StreamingProcessor::waitFeaturePipeDone()
{
    TRACE_S_FUNC_ENTER(mLog);
    android::Mutex::Autolock _lock(mPayloadMutex);
    while( mPayloadList.size() )
    {
        mPayloadCondition.wait(mPayloadMutex);
    }
    TRACE_S_FUNC_EXIT(mLog);
}

MVOID StreamingProcessor::incPayloadCount(const ILog &log)
{
    TRACE_FUNC_ENTER();
    TRACE_S_FUNC(log, "count=%d", mPayloadCount++);
    TRACE_FUNC_EXIT();
}

MVOID StreamingProcessor::decPayloadCount(const ILog &log)
{
    TRACE_FUNC_ENTER();
    TRACE_S_FUNC(log, "count=%d", mPayloadCount--);
    TRACE_FUNC_EXIT();
}

MVOID StreamingProcessor::incPayload(const sp<Payload> &payload)
{
    TRACE_S_FUNC_ENTER(payload->mLog);
    android::Mutex::Autolock _lock(mPayloadMutex);
    mPayloadList.push_back(payload);
    FeaturePipeParam* sFPP = payload->getMainFeaturePipeParam();
    if(sFPP != nullptr)
    {
        sFPP->setVar<sp<Payload>>(SFP_VAR::STREAMING_PAYLOAD, payload);
    }
    else
    {
        MY_S_LOGE(payload->mLog,"Error, getMainFeaturePipeParam return null !!");
    }
    TRACE_S_FUNC_EXIT(payload->mLog);
}

MBOOL StreamingProcessor::decPayload(FeaturePipeParam &param, const sp<Payload> &payload, MBOOL checkOrder)
{
    TRACE_S_FUNC_ENTER(payload->mLog);
    android::Mutex::Autolock _lock(mPayloadMutex);
    MBOOL ret = MFALSE;
    auto it = find(mPayloadList.begin(), mPayloadList.end(), payload);
    if( it != mPayloadList.end() )
    {
        if( checkOrder && it != mPayloadList.begin() )
        {
            MY_S_LOGW(payload->mLog, "callback out of order");
        }
        mPayloadList.erase(it);
        mPayloadCondition.broadcast();
        ret = MTRUE;
    }
    else
    {
        MY_S_LOGE(payload->mLog, "Payload not released: invalid data=%p list=%zu", payload.get(), mPayloadList.size());
    }

    param.clearVar<sp<Payload>>(SFP_VAR::STREAMING_PAYLOAD);
    FeaturePipeParam* fpp = payload->getMainFeaturePipeParam();
    if(fpp != nullptr)
    {
        fpp->clearVar<sp<Payload>>(SFP_VAR::STREAMING_PAYLOAD);
    }
    TRACE_S_FUNC_EXIT(payload->mLog);
    return ret;
}

MVOID StreamingProcessor::prepareFeatureParam(P2Util::SimpleIn& input, const ILog &log)
{
    TRACE_S_FUNC_ENTER(log);

    // Begin feature plug-in section
    prepareCommon(input, log);
    prepareDSDN(input, log);
    prepare3DNR(input, log);
    prepareFSC(input, log);
    prepareEIS(input, log);
    prepareTP(input, log);
    // End feaure plug-in section

    TRACE_S_FUNC_EXIT(log);
}

MBOOL StreamingProcessor::prepareTP(P2Util::SimpleIn& input, const ILog &log) const
{
    TRACE_S_FUNC_ENTER(log);
    MFLOAT tpMarginRatio = mP2Info.getConfigInfo().mUsageHint.mTPMarginRatio;
    if(tpMarginRatio != 1.0f)
    {
        input.addCropRatio("tpi", 1.0f / tpMarginRatio);
    }
    TRACE_S_FUNC_EXIT(log);
    return MTRUE;
}

MBOOL StreamingProcessor::prepareCommon(P2Util::SimpleIn& input, const ILog &log) const
{
    TRACE_S_FUNC_ENTER(log);
    const sp<P2Request> &request = input.mRequest;
    const sp<Cropper> cropper = request->getCropper(input.getSensorId());
    FeaturePipeParam &featureParam = input.mFeatureParam;
    IStreamingFeaturePipe::eAppMode mode = IStreamingFeaturePipe::APP_PHOTO_PREVIEW;
    featureParam.mDumpType = request->mDumpType;

    switch( request->mP2Pack.getFrameData().mAppMode )
    {
    case MTK_FEATUREPIPE_PHOTO_PREVIEW:
        mode = IStreamingFeaturePipe::APP_PHOTO_PREVIEW;
        break;
    case MTK_FEATUREPIPE_VIDEO_PREVIEW:
        mode = IStreamingFeaturePipe::APP_VIDEO_PREVIEW;
        break;
    case MTK_FEATUREPIPE_VIDEO_RECORD:
        mode = IStreamingFeaturePipe::APP_VIDEO_RECORD;
        break;
    case MTK_FEATUREPIPE_VIDEO_STOP:
        mode = IStreamingFeaturePipe::APP_VIDEO_STOP;
        break;
    default:
        mode = IStreamingFeaturePipe::APP_PHOTO_PREVIEW;
        break;
    }

    featureParam.setVar<IStreamingFeaturePipe::eAppMode>(SFP_VAR::APP_MODE, mode);
    featureParam.setVar<MINT64>(SFP_VAR::P1_TS, request->mP2Pack.getSensorData().mP1TS);
    featureParam.setVar<MBOOL>(SFP_VAR::IMGO_2IMGI_ENABLE, !input.isResized() );
    featureParam.setVar<MRect>(SFP_VAR::IMGO_2IMGI_P1CROP, cropper->getP1Crop());

    TRACE_S_FUNC_EXIT(log);
    return MTRUE;
}

MBOOL StreamingProcessor::processP2(const sp<Payload> &payload)
{
    P2_CAM_TRACE_CALL(TRACE_ADVANCED);
    TRACE_S_FUNC_ENTER(payload->mLog);
    MBOOL ret = MFALSE;
    if( payload->mLog.getLogLevel() >= 2 )
    {
        payload->print();
    }
    incPayload(payload);

    if( !makeSFPIOMgr(payload) )
    {
        MY_S_LOGE(payload->mLog, "make SFPIO failed !!");
        CAM_ULOG_DISCARD(MOD_P2_STR_PROC, REQ_P2_STR_REQUEST, payload->mLog.getLogFrameID());
        return MFALSE;
    }

    FeaturePipeParam* pFPP = payload->getMainFeaturePipeParam();
    pFPP->mCallback = sFPipeCB;
    pFPP->mP2Pack = payload->getMainRequest()->mP2Pack;

    P2_CAM_TRACE_BEGIN(TRACE_ADVANCED, "P2_Streaming:drv enq");
    if( mBypssSFP )
    {
        MY_S_LOGD(payload->mLog, "Skip SFP and direct return!");
        pFPP->mDIPParams.mDequeSuccess = MTRUE;
        sFPipeCB(FeaturePipeParam::MSG_FRAME_DONE, *pFPP);
        ret = MTRUE;
    }
    else
    {
        ret = mFeaturePipe->enque(*pFPP);
    }
    P2_CAM_TRACE_END(TRACE_ADVANCED);
    if( !ret )
    {
        MY_S_LOGW(payload->mLog, "enque failed");
        for(const auto& pp : payload->mPartialPayloads)
        {
            pp->mRequestPack->updateBufferResult(MFALSE);
        }
         FeaturePipeParam* fpp = payload->getMainFeaturePipeParam();
        if(fpp == nullptr)
        {
            MY_S_LOGE(payload->mLog,"error payload->getMainFeaturePipeParam() reutrn null");
        }
        else
        {
            decPayload(*fpp, payload, MFALSE);
        }
    }
    TRACE_S_FUNC_EXIT(payload->mLog);
    return ret;
}

MVOID StreamingProcessor::onFPipeCB(FeaturePipeParam::MSG_TYPE msg, FeaturePipeParam &param, const sp<Payload> &payload)
{
    TRACE_S_FUNC_ENTER(payload->mLog, "callback msg: %d, success:%d", msg, param.mDIPParams.mDequeSuccess);
    P2_CAM_TRACE_NAME(TRACE_ADVANCED, "P2_Streaming:onFPipeCB()");
    sp<P2Request> oneRequest = payload->mPartialPayloads.front()->mRequestPack->mMainRequest;
    if(oneRequest != nullptr)
    {
        oneRequest->beginBatchRelease();
    }
    else
    {
        MY_S_LOGE(payload->mLog, "cannot get P2Request to do batchRelease!!");
    }
    if( msg == FeaturePipeParam::MSG_FRAME_DONE )
    {
        for(const auto& pp : payload->mPartialPayloads)
        {
            pp->mRequestPack->updateBufferResult(param.mDIPParams.mDequeSuccess);
            pp->mRequestPack->updateMetaResult(param.mDIPParams.mDequeSuccess);
            if( param.getVar<MBOOL>(SFP_VAR::EIS_SKIP_RECORD, MFALSE) )
            {
                pp->mRequestPack->dropRecord();
            }
        }
        processTSQ(payload->getMainRequest(), getTSQ(payload, param));
        processVRQ(param, payload);
        P2_CAM_TRACE_BEGIN(TRACE_ADVANCED, "MSG_FRAME_DONE->earlyRelease");
        for(const auto& pp : payload->mPartialPayloads)
        {
            pp->mRequestPack->earlyRelease(P2Util::RELEASE_ALL);
        }

        P2_CAM_TRACE_END(TRACE_ADVANCED);
    }
    else if( msg == FeaturePipeParam::MSG_DISPLAY_DONE )
    {
        for(const auto& pp : payload->mPartialPayloads)
        {
            pp->mRequestPack->updateBufferResult(param.mDIPParams.mDequeSuccess);
            pp->mRequestPack->earlyRelease(P2Util::RELEASE_DISP);
        }
    }
    else if( msg == FeaturePipeParam::MSG_RSSO_DONE )
    {
        for(const auto& pp : payload->mPartialPayloads)
        {
            pp->mRequestPack->updateBufferResult(param.mDIPParams.mDequeSuccess);
            pp->mRequestPack->earlyRelease(P2Util::RELEASE_RSSO);
        }
    }
    else if( msg == FeaturePipeParam::MSG_FD_DONE )
    {
        for(const auto& pp : payload->mPartialPayloads)
        {
            pp->mRequestPack->updateBufferResult(param.mDIPParams.mDequeSuccess);
            pp->mRequestPack->earlyRelease(P2Util::RELEASE_FD);
        }
    }
    if(oneRequest != nullptr)
    {
        oneRequest->endBatchRelease();
    }
    TRACE_S_FUNC_EXIT(payload->mLog);
}

MBOOL StreamingProcessor::sFPipeCB(FeaturePipeParam::MSG_TYPE msg, FeaturePipeParam &param)
{
    TRACE_FUNC_ENTER();
    MBOOL ret = MTRUE;
    P2_CAM_TRACE_BEGIN(TRACE_ADVANCED, "get payload");
    sp<Payload> payload = param.getVar<sp<Payload>>(SFP_VAR::STREAMING_PAYLOAD, nullptr);
    P2_CAM_TRACE_END(TRACE_ADVANCED);
    if( payload == nullptr )
    {
        MY_LOGW("invalid payload = NULL, msg(%d)", msg);
        ret = MFALSE;
    }
    else if( payload->mParent == nullptr )
    {
        MY_LOGW("invalid payload(%p), parent = NULL", payload.get());
        payload = nullptr;
    }
    else
    {
        payload->mParent->onFPipeCB(msg, param, payload);
    }
    if( msg == FeaturePipeParam::MSG_FRAME_DONE &&
        payload != nullptr )
    {
        P2_CAM_TRACE_BEGIN(TRACE_ADVANCED, "payload->mParent->decPayload");
        payload->mParent->decPayload(param, payload, MTRUE);
        P2_CAM_TRACE_END(TRACE_ADVANCED);
    }
    TRACE_FUNC_EXIT();
    return ret;
}

MVOID StreamingProcessor::processVRQ(const FeaturePipeParam &param, const sp<Payload> &payload)
{
    TRACE_FUNC_ENTER();
    if( isBatchSMVR() )
    {
        processSMVRQ(param, payload);
    }
    else
    {
        for(const auto& pp : payload->mPartialPayloads)
        {
            pp->mRequestPack->updateVRTimestamp(1);
        }
    }
    TRACE_FUNC_EXIT();
}

StreamingProcessor::P2RequestPack::P2RequestPack(const ILog &log, const sp<P2Request>& pReq, const vector<MUINT32>& sensorIDs)
: mLog(log)
{
    mMainRequest = pReq;
    mRequests.insert(pReq);
    MUINT32 reqSensorID = pReq->getSensorID();

    for(const auto& sensorID : sensorIDs)
    {
        if( pReq->isValidImg(IN_RESIZED, sensorID) || pReq->isValidImg(IN_FULL, sensorID) )
        {
            P2Util::SimpleIn in(sensorID, pReq);
            if( pReq->isValidImg(IN_RESIZED, sensorID) )
            {
                in.setISResized(MTRUE);
                in.mIMGI = pReq->moveImg(IN_RESIZED, sensorID);
            }
            else if( pReq->isValidImg(IN_FULL, sensorID) )
            {
                in.setISResized(MFALSE);
                in.mIMGI = pReq->moveImg(IN_FULL, sensorID);
            }
            in.mLCEI    = pReq->moveImg(IN_LCSO,        sensorID);
            in.mLCSHO   = pReq->moveImg(IN_LCSHO,       sensorID);
            in.mRSSO    = pReq->moveImg(IN_RSSO,        sensorID);
            in.mFullYuv = pReq->moveImg(IN_FULL_YUV,    sensorID);
            in.mRrzYuv1 = pReq->moveImg(IN_RESIZED_YUV1,sensorID);
            in.mRrzYuv2 = pReq->moveImg(IN_RESIZED_YUV2,sensorID);

            mSensorInputMap[sensorID] = mInputs.size();
            mInputs.push_back(in);
        }
    }

    vector<P2Util::SimpleOut> vOut;
    // FD buffer is stored in mImg map

    if( isValid(pReq->mImg[OUT_FD]) )
    {
        P2Util::SimpleOut out(reqSensorID, pReq, pReq->mImg[OUT_FD]);
        out.setIsFD(MTRUE);
        vOut.push_back(out);
    }
    for(auto& it : pReq->mImgOutArray)
    {
        P2Util::SimpleOut out(reqSensorID, pReq, it);
        vOut.push_back(out);
    }
    mOutputs[pReq.get()] = vOut;
}

StreamingProcessor::P2RequestPack::~P2RequestPack()
{
}

MVOID StreamingProcessor::P2RequestPack::addOutput(const sp<P2Request>& pReq, const MUINT32 outputIndex)
{
    if( mRequests.find(pReq) != mRequests.end() )
    {
        if(pReq == mMainRequest)
        {
            MY_S_LOGD_IF(mLog.getLogLevel() >= 3, mLog, "already containes this request, ignore");
        }
        else
        {
            MY_S_LOGE(mLog, "Currently not support request, with more than 1 output, merged to other request!!");
        }
    }
    else
    {
        mRequests.insert(pReq);

        if(outputIndex >= 0)
        {
            P2Util::SimpleOut out(pReq->getSensorID(), pReq, pReq->mImgOutArray[outputIndex]);
            vector<P2Util::SimpleOut> vOut = {out};
            mOutputs[pReq.get()] = vOut;
        }
        else
        {
            MY_S_LOGE(mLog, "outputIndex < 0, maybe non app yuv desire merged --> Not Support currently.");
        }

        if(pReq->mImg[OUT_FD] != NULL || pReq->mImg[OUT_JPEG_YUV] != NULL || pReq->mImg[OUT_THN_YUV] != NULL)
        {
            MY_S_LOGE(mLog, "Currently not support OUT FD/JpegYUV /thumbYuv in non-first IOMap !!  Need Check it !!!");
        }
    }
}

MVOID StreamingProcessor::P2RequestPack::updateBufferResult(MBOOL result)
{
    for(const auto& outSet : mOutputs)
    {
        for(const auto& out : outSet.second)
        {
            if (out.mImg != NULL)
            {
                out.mImg->updateResult(result);
            }
        }
    }
}

MVOID StreamingProcessor::P2RequestPack::updateMetaResult(MBOOL result)
{
    for(auto&& req : mRequests)
    {
        req->updateMetaResult(result);
    }
}

MVOID StreamingProcessor::P2RequestPack::dropRecord()
{
    for(const auto& outSet : mOutputs)
    {
        for(const auto& out : outSet.second)
        {
            if( out.mImg != NULL && out.mImg->isRecord() )
            {
                out.mImg->updateResult(MFALSE);
            }
        }
    }
}

MVOID StreamingProcessor::P2RequestPack::updateVRTimestamp(MUINT32 run)
{
    MINT64 cam2FwTs = mMainRequest->mP2Pack.getSensorData().mP1TS;
    for(const auto& outSet : mOutputs)
    {
        for(const auto& out : outSet.second)
        {
            if( out.mImg != NULL && out.mImg->isRecord() )
            {
                out.mImg->updateVRTimestamp(run, cam2FwTs);
            }
        }
    }
}

MINT64 StreamingProcessor::P2RequestPack::getVRTimestamp()
{
    MINT64 ts = 0;
    for(const auto& outSet : mOutputs)
    {
        for(const auto& out : outSet.second)
        {
            if( out.mImg != NULL && out.mImg->isRecord() )
            {
                ts = out.mImg->getIImageBufferPtr()->getTimestamp();
                break;
            }
        }
    }
    return ts;
}

MVOID StreamingProcessor::P2RequestPack::earlyRelease(MUINT32 mask)
{
    for(auto& in : mInputs)
    {
        if( mask & P2Util::RELEASE_ALL )
        {
            in.releaseAllImg();
        }
        if( mask & P2Util::RELEASE_DISP )
        {
            in.mIMGI = nullptr;
            in.mLCEI = nullptr;
            in.mLCSHO = nullptr;
        }
        if( mask & P2Util::RELEASE_RSSO )
        {
            in.mRSSO = nullptr;
            in.mPreRSSO = nullptr;
        }
    }
    for(auto& outSet : mOutputs)
    {
        for(auto& out : outSet.second)
        {
            if( (mask & P2Util::RELEASE_ALL) ||
                ((mask & P2Util::RELEASE_DISP) && out.mImg->isDisplay()) ||
                ((mask & P2Util::RELEASE_FD) && out.isFD()) )
            {
                if (out.mImg != NULL)
                {
                    out.mImg = nullptr;
                }
            }
        }
    }
    if( mask & P2Util::RELEASE_ALL )
    {
        for(auto&& req : mRequests)
        {
            req->releaseResource(P2Request::RES_META);
        }
    }
}


MBOOL StreamingProcessor::P2RequestPack::contains(const sp<P2Request>& pReq) const
{
    return mRequests.find(pReq) != mRequests.end();
}

P2Util::SimpleIn* StreamingProcessor::P2RequestPack::getInput(MUINT32 sensorID)
{
    for(auto& in : mInputs)
    {
        if(in.getSensorId() == sensorID)
        {
            return &in;
        }
    }
    return NULL;
}

MUINT32 StreamingProcessor::P2RequestPack::getNumDisplay() const
{
    MUINT32 count = 0;
    auto it = mOutputs.find(mMainRequest.get());
    if( it != mOutputs.end() )
    {
        for( const P2Util::SimpleOut &out : it->second )
        {
            if( out.isDisplay() )
            {
                count = out.mImg->getIImageBufferPtrsCount();
                break;
            }
        }
    }
    return count;
}

MUINT32 StreamingProcessor::P2RequestPack::getNumRecord() const
{
    MUINT32 count = 0;
    auto it = mOutputs.find(mMainRequest.get());
    if( it != mOutputs.end() )
    {
        for( const P2Util::SimpleOut &out : it->second )
        {
            if( out.isRecord() )
            {
                count = out.mImg->getIImageBufferPtrsCount();
                break;
            }
        }
    }
    return count;
}

StreamingProcessor::PartialPayload::PartialPayload(const ILog &mainLog, const sp<P2RequestPack> &pReqPack)
: mRequestPack(pReqPack)
, mLog(mainLog)
{
}

StreamingProcessor::PartialPayload::~PartialPayload()
{
}

MVOID StreamingProcessor::PartialPayload::print() const
{
    //TODO print partialPayload : mInputs & mOutputs
}

StreamingProcessor::Payload::Payload(StreamingProcessor* parent, const ILog &mainLog, MUINT32 masterSensorId)
: mParent(parent)
, mLog(mainLog)
, mMasterID(masterSensorId)
{
}

StreamingProcessor::Payload::~Payload()
{
    if (mpFdData != NULL)
    {
        TRACE_FUNC("!!warn: mpFdData(%p) to be freed", mpFdData);
        delete mpFdData;
        mpFdData = NULL;
    }

    sp<P2Request> req = getAnyRequest();
    if(req != NULL)
    {
        req->beginBatchRelease();
        mPartialPayloads.clear();
        mReqPaths.clear();
        req->releaseResource(P2Request::RES_ALL);
        req->endBatchRelease();
    }
}

MVOID StreamingProcessor::Payload::addRequests(const vector<sp<P2Request>> &requests)
{
    for( const auto& it : requests )
    {
        if( it->isPhysic() )
        {
            mReqPaths[ERequestPath::ePhysic][it->getSensorID()] = it;
            continue;
        }
        if( it->isLarge() )
        {
            mReqPaths[ERequestPath::eLarge][it->getSensorID()] = it;
            continue;
        }
        mReqPaths[ERequestPath::eGeneral][it->getSensorID()] = it;
    }
}

MVOID StreamingProcessor::Payload::addRequestPacks(const vector<sp<P2RequestPack>>& reqPacks)
{
    for( const auto& it : reqPacks )
    {
        sp<PartialPayload> partialPayload= new PartialPayload(mLog, it);
        mPartialPayloads.push_back(partialPayload);
    }
}

MBOOL StreamingProcessor::Payload::prepareFdData(const P2Info &p2Info, IFDContainer *pFDContainer)
{
    MBOOL ret = MFALSE;
    const P2ConfigInfo confingInfo = p2Info.getConfigInfo();
    const P2PlatInfo *pPlatInfoPtr = p2Info.getPlatInfo();
    if( pPlatInfoPtr != NULL && pFDContainer != NULL )
    {
        if (confingInfo.mSupportDRE && pPlatInfoPtr->supportDRE())
        {
            if (pFDContainer != NULL)
            {
                MBOOL fdResult = MFALSE;
                FD_DATATYPE fdData;
                fdResult = pFDContainer->cloneLatestFD(fdData);
                TRACE_FUNC("query fd result = %d", fdResult);
                if( fdResult )
                {
                    mpFdData = new FD_DATATYPE();
                    fdData.clone(*mpFdData);
                    ret = MTRUE;
                }
            }
        }
    }

    TRACE_FUNC("param(pPlatInfoPtr=%p, pFDContainer=%p), DRE(configSupport=%d, platSupport=%d), mFdData=%p, ret=%d",
                pPlatInfoPtr, pFDContainer,
                confingInfo.mSupportDRE,
                (pPlatInfoPtr != NULL) ? pPlatInfoPtr->supportDRE() : -1,
                mpFdData, ret);

    return ret;
}

sp<P2Request> StreamingProcessor::Payload::getMainRequest()
{
    // main request order: General Request-> Physic_1"
    sp<P2Request> request = getPathRequest(ERequestPath::eGeneral, mMasterID);
    if( request == nullptr )
    {
         request = getPathRequest(ERequestPath::ePhysic, mMasterID);
    }
    if( request == nullptr )
    {
         request = getPathRequest(ERequestPath::eLarge, mMasterID);
    }
    MY_S_LOGE_IF(request == nullptr,mLog,"can not find main request !!");
    return request;
}

sp<P2Request> StreamingProcessor::Payload::getAnyRequest()
{
    for(auto& it : mReqPaths)
    {
        for(auto& it2 : it.second)
        {
            if(it2.second != NULL)
            {
                return it2.second;
            }
        }
    }
    return NULL;
}

sp<P2Request> StreamingProcessor::Payload::getPathRequest(ERequestPath path, const MUINT32& sensorID)
{
    if( mReqPaths.find(path) == mReqPaths.end() )
    {
        return nullptr;
    }
    else
    {
        if( mReqPaths[path].find(sensorID) == mReqPaths[path].end() )
        {
            return nullptr;
        }
        else
        {
            return mReqPaths[path][sensorID];
        }
    }
}

sp<StreamingProcessor::P2RequestPack> StreamingProcessor::Payload::getRequestPack(const sp<P2Request> &pReq)
{
    for(const auto& partialPayload : mPartialPayloads)
    {
        if( partialPayload->mRequestPack->contains(pReq) )
        {
            return partialPayload->mRequestPack;
        }
    }
    MY_S_LOGE(mLog,"req(%p) not belong to any P2RequestPack!!", pReq.get());
    return nullptr;
}

FeaturePipeParam* StreamingProcessor::Payload::getMainFeaturePipeParam()
{
    sp<P2Request> mainRequest = getMainRequest();
    if( mainRequest != nullptr )
    {
        sp<P2RequestPack> reqPack = getRequestPack(mainRequest);
        P2Util::SimpleIn *in = (reqPack != nullptr) ? reqPack->getInput(mMasterID) : nullptr;
        if( in != nullptr )
        {
            return &(in->mFeatureParam);
        }
    }
    MY_S_LOGE(mLog,"can not find main feature param !!");
    return nullptr;
}

MVOID StreamingProcessor::Payload::print() const
{
    TRACE_S_FUNC_ENTER(mLog);
    MY_S_LOGD(mLog, "MasterID = %d", mMasterID);
    for(const auto& partialPayload : mPartialPayloads)
    {
        partialPayload->print();
    }
    TRACE_S_FUNC_EXIT(mLog);
}

RSSOMgr::RSSOMgr()
{
    TRACE_FUNC_ENTER();
    TRACE_FUNC_EXIT();
}

RSSOMgr::~RSSOMgr()
{
    TRACE_FUNC_ENTER();
    uninit();
    TRACE_FUNC_EXIT();
}

MVOID RSSOMgr::init(const std::vector<MUINT32> &sensorIDs)
{
    TRACE_FUNC_ENTER();
    android::Mutex::Autolock _lock(mMutex);
    if( !mReady )
    {
        for( MUINT32 id : sensorIDs )
        {
            mCachePool[id].create("rsso_cache", NSCam::eImgFmt_STA_BYTE, MSize(512,512));
            sp<IImageBuffer> cache = mCachePool[id].getBuffer();
            MY_LOGD("triger: cache=%p", cache.get());
        }
    }
    TRACE_FUNC_EXIT();
}

MVOID RSSOMgr::uninit()
{
    TRACE_FUNC_ENTER();
    android::Mutex::Autolock _lock(mMutex);
    if( mReady )
    {
        for( auto &pool : mCachePool )
        {
            pool.second.destroy();
        }
    }
    TRACE_FUNC_EXIT();
}

RSSOMgr::Cache RSSOMgr::updateRSSO(const ILog &log, const sp<P2Img> &rsso, MUINT32 sensorID)
{
    TRACE_S_FUNC_ENTER(log);
    android::Mutex::Autolock _lock(mMutex);
    Cache holder;
    holder = mHolder[sensorID];
    mHolder[sensorID] = Cache(rsso);
    TRACE_S_FUNC_EXIT(log);
    return holder;
}

MVOID RSSOMgr::releaseRSSO(const ILog &log, RSSOMgr::Policy policy)
{
    TRACE_S_FUNC_ENTER(log);
    android::Mutex::Autolock _lock(mMutex);
    for( auto &holder : mHolder )
    {
        sp<IImageBuffer> cache = NULL;
        if( policy == RSSOMgr::POLICY_COPY_CACHE &&
            isValid(holder.second.mPrev) )
        {
            IImageBuffer *src = holder.second.mPrev->getIImageBufferPtr();
            cache = mCachePool[holder.first].getBuffer();
            if( cache != NULL )
            {
                cache->setExtParam(src->getImgSize());
                copyImageBuffer(src, cache.get());
            }
            MY_S_LOGD(log, "RSSOMgr: trigger rsso cache:%p", cache.get());
        }
        holder.second.mPrev = NULL;
        holder.second.mCache = cache;
    }
    TRACE_S_FUNC_EXIT(log);
}

RSSOMgr::Cache::Cache()
{
    TRACE_FUNC_ENTER();
    TRACE_FUNC_EXIT();
}

RSSOMgr::Cache::Cache(const sp<P2Img> &rsso)
    : mPrev(rsso)
{
    TRACE_FUNC_ENTER();
    TRACE_FUNC_EXIT();
}

RSSOMgr::Cache::Cache(const sp<IImageBuffer> &cache)
    : mCache(cache)
{
    TRACE_FUNC_ENTER();
    TRACE_FUNC_EXIT();
}

IImageBuffer* RSSOMgr::Cache::getIImageBufferPtr()
{
    TRACE_FUNC_ENTER();
    TRACE_FUNC_EXIT();
    return isValid(mPrev) ? mPrev->getIImageBufferPtr() : mCache.get();
}

} // namespace P2
