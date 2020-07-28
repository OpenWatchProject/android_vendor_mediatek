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

#include "MDPNode.h"

#define PIPE_CLASS_TAG "MDPNode"
#define PIPE_TRACE TRACE_MDP_NODE
#include <featurePipe/core/include/PipeLog.h>

#include <mtkcam/utils/metadata/client/mtk_metadata_tag.h>
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>
#include <mtkcam/utils/exif/DebugExifUtils.h>
#include <mtkcam/utils/TuningUtils/FileReadRule.h>
#include <isp_tuning/isp_tuning.h>
#include <mtkcam3/feature/utils/ISPProfileMapper.h>

CAM_ULOG_DECLARE_MODULE_ID(MOD_CAPTURE_MDP);

#if MTK_CAM_NEW_NVRAM_SUPPORT
//#include <mtkcam/utils/mapping_mgr/cam_idx_mgr.h>
#endif

using namespace NSCam::Utils;
using namespace NSCam::NSIoPipe;
using namespace NSCam::TuningUtils;
using namespace NSIspTuning;

using IImageTransform = NSCam::NSIoPipe::NSSImager::IImageTransform;

#define FD_TOLERENCE        (600000000)
#define DUMP_FILE_NAME_SIZE 1024
namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {
namespace NSCapture {

MDPNode::MDPNode(NodeID_T nid, const char* name, MINT32 policy, MINT32 priority)
    : CamNodeULogHandler(Utils::ULog::MOD_CAPTURE_MDP)
    , CaptureFeatureNode(nid, name, 0, policy, priority)
    , mpMdpSetting(NULL)
{
    TRACE_FUNC_ENTER();
    this->addWaitQueue(&mRequestPacks);
    mDebugDump = property_get_int32("vendor.debug.camera.p2.dump", 0) > 0;
    mDebugDumpFilter = property_get_int32("vendor.debug.camera.p2.dump.filter", 0xFFFF);

#if MTKCAM_TARGET_BUILD_VARIANT != 'u'
    mDebugDumpMDP           = MTRUE;
#else
    mDebugDumpMDP           = property_get_int32("vendor.debug.camera.mdp.dump", 0) > 0;
#endif

    TRACE_FUNC_EXIT();
}

MDPNode::~MDPNode()
{
    TRACE_FUNC_ENTER();
    TRACE_FUNC_EXIT();
}

MBOOL MDPNode::onData(DataID id, const RequestPtr& pRequest)
{
    TRACE_FUNC_ENTER();
    MY_LOGD_IF(mLogLevel, "R/F/S: %d/%d/%d %s arrived",
                pRequest->getRequestNo(), pRequest->getFrameNo(),
                pRequest->getMainSensorIndex(), PathID2Name(id));

    NodeID_T srcNodeId;
    NodeID_T dstNodeId;
    MBOOL ret = GetPath(id, srcNodeId, dstNodeId);

    if (!ret) {
        MY_LOGD("Can not find the path: %d", id);
        return ret;
    }

    if (dstNodeId != NID_MDP && dstNodeId != NID_MDP_B && dstNodeId != NID_MDP_C) {
        MY_LOGE("Unexpected dstNode node: %s", NodeID2Name(dstNodeId));
        return MFALSE;
    }

    if (pRequest->isReadyFor(dstNodeId)) {
        RequestPack pack = {
            .mNodeId = dstNodeId,
            .mpRequest = pRequest
        };
        mRequestPacks.enque(pack);
    }

    TRACE_FUNC_EXIT();
    return MTRUE;
}


MBOOL MDPNode::onInit()
{
    TRACE_FUNC_ENTER();
    CaptureFeatureNode::onInit();

    TRACE_FUNC_EXIT();
    return MTRUE;
}

MBOOL MDPNode::onUninit()
{
    TRACE_FUNC_ENTER();

    if (mpMdpSetting != NULL) {
        void* pBuffer = mpMdpSetting->buffer;
        if (pBuffer)
            free(pBuffer);

        delete mpMdpSetting;
        mpMdpSetting = NULL;
    }

    TRACE_FUNC_EXIT();
    return MTRUE;
}

MBOOL MDPNode::onThreadStart()
{
    TRACE_FUNC_ENTER();
    MBOOL ret = MTRUE;
    TRACE_FUNC_EXIT();
    return ret;
}

MBOOL MDPNode::onThreadStop()
{
    TRACE_FUNC_ENTER();
    TRACE_FUNC_EXIT();
    return MTRUE;
}

MBOOL MDPNode::onThreadLoop()
{
    TRACE_FUNC("Waitloop");
    RequestPack pack;

    if (!waitAllQueue()) {
        return MFALSE;
    }

    if (!mRequestPacks.deque(pack)) {
        MY_LOGE("Request deque out of sync");
        return MFALSE;
    } else if (pack.mpRequest == NULL) {
        MY_LOGE("Request out of sync");
        return MFALSE;
    }

    RequestPtr pRequest = pack.mpRequest;

    TRACE_FUNC_ENTER();

    incExtThreadDependency();
    TRACE_FUNC("Frame %d in MDP", pRequest->getRequestNo());

    if (!pRequest->isCancelled()) {
        onRequestProcess(pack.mNodeId, pRequest);
    }

    onRequestFinish(pack.mNodeId, pRequest);
    decExtThreadDependency();
    TRACE_FUNC_EXIT();
    return MTRUE;
}

#if MTKCAM_ENABLE_ISPPROFILE_MAPPER == 1
MINT32 MDPNode::getISPProfileForMDP(const RequestPtr& pRequest, NodeID_T nodeId)
{
    ISPProfileMapper* pProfileMapper = ISPProfileMapper::getInstance();
    sp<CaptureFeatureNodeRequest> pNodeReq = pRequest->getNodeRequest(nodeId);
    IMetadata* pIMetaApp        = pNodeReq->acquireMetadata(MID_MAN_IN_APP);
    IMetadata* pIMetaHal        = pNodeReq->acquireMetadata(MID_MAN_IN_HAL);

    MINT32 masterId = pRequest->getMainSensorIndex();
    MINT32 slaveId = pRequest->getSubSensorIndex();
    MBOOL isPhysical = pRequest->isPhysicalStream();

    MBOOL bIsDualMode = ((mUsageHint.mDualMode != 0) && (slaveId >= 0));
    ESensorCombination sensorCom = eSensorComb_Single;

    if(bIsDualMode)
    {
        auto sensorFmt = (masterId >= 0) ? getSensorRawFmt(masterId) : SENSOR_RAW_FMT_NONE;
        auto sensorFmt2 = getSensorRawFmt(slaveId);
        const MBOOL isBayerBayer = (sensorFmt != SENSOR_RAW_MONO) && (sensorFmt2 != SENSOR_RAW_MONO);
        const MBOOL isBayerMono = (sensorFmt != SENSOR_RAW_MONO) && (sensorFmt2 == SENSOR_RAW_MONO);

        if(isBayerBayer)
            sensorCom = eSensorComb_BayerBayer;
        else if(isBayerMono)
            sensorCom = eSensorComb_BayerMono;
        else
        {
            MY_LOGW("Unvalid sensor combination:main1 sensor fmt=%d main2 sensor fmt=%d", sensorFmt, sensorFmt2);
        }
    }

    ProfileMapperKey mappingKey = pProfileMapper->queryMappingKey(pIMetaApp, pIMetaHal, isPhysical, eMappingScenario_Capture, sensorCom);

    EIspProfile_T ispProfile;
    if(!pProfileMapper->mappingProfile(mappingKey, eStage_MDP, ispProfile))
    {
        MY_LOGD("Failed to get ISP Profile, stage: eStage_MDP, use EIspProfile_Capture as default", eStage_MDP);
        ispProfile = EIspProfile_Capture;
    }
    return ispProfile;
}

#else

MINT32 MDPNode::getISPProfileForMDP(const RequestPtr& pRequest, NodeID_T nodeId)
{
    MINT32 iDefaultProfile = EIspProfile_Capture;
#ifdef SUPPORT_AINR
    if(pRequest->hasFeature(FID_AINR))
        iDefaultProfile = EIspProfile_AINR_Main;
    else
#endif
#ifdef SUPPORT_MFNR
    if(pRequest->hasFeature(FID_MFNR) && !pRequest->isSingleFrame())
        iDefaultProfile = EIspProfile_MFNR_After_Blend;
    else
#endif
    if(pRequest->hasFeature(FID_HDR) || pRequest->hasFeature(FID_HDR_3RD_PARTY) ||
            pRequest->hasFeature(FID_HDR2_3RD_PARTY))
        iDefaultProfile = EIspProfile_Auto_mHDR_Capture;
    else if(pRequest->hasFeature(FID_BOKEH) || pRequest->hasFeature(FID_BOKEH_3RD_PARTY))
    {
        IspProfileHint hint = {eSAN_Master, eSCT_BayerBayer, eRIT_Imgo};
        const IspProfileInfo& profileInfo = IspProfileManager::get(hint);
        return profileInfo.mValue;
    }
    return iDefaultProfile;
}
#endif

MVOID MDPNode::updateSensorCropRegion(RequestPtr& pRequest, MRect cropRegion)
{
    const MINT32 requestNo = pRequest->getRequestNo();
    const MINT32 frameNo = pRequest->getFrameNo();
    const MINT32 sensorId = pRequest->getMainSensorIndex();
    MetadataID_T metaId = (pRequest->isPhysicalStream()) ? MID_MAN_OUT_APP_PHY : MID_MAN_OUT_APP;
    sp<MetadataHandle> metaPtr = pRequest->getMetadata(metaId);
    IMetadata* pMetaPtr = (metaPtr != nullptr) ? metaPtr->native() : nullptr;
    if (pMetaPtr == nullptr)
    {
        MY_LOGW("failed to get meta, R/F/S Num: %d/%d/%d, metaId:%d", requestNo, frameNo, metaId, sensorId);
        return;
    }

    auto updateCropRegion = [&](IMetadata::IEntry& entry, IMetadata* meta)
    {
        entry.push_back(cropRegion.p.x, Type2Type<MINT32>());
        entry.push_back(cropRegion.p.y, Type2Type<MINT32>());
        entry.push_back(cropRegion.s.w, Type2Type<MINT32>());
        entry.push_back(cropRegion.s.h, Type2Type<MINT32>());
        meta->update(entry.tag(), entry);
    };

    IMetadata::IEntry sensorCropRegionEntry(MTK_MULTI_CAM_SENSOR_CROP_REGION);
    updateCropRegion(sensorCropRegionEntry, pMetaPtr);
    MY_LOGD("update sensor crop region, R/F Num: %d/%d, sensorIndex:%d, metaId:%d, region:(%d, %d)x(%d, %d)",
        requestNo, frameNo,
        sensorId, metaId,
        cropRegion.p.x, cropRegion.p.y,
        cropRegion.s.w, cropRegion.s.h);
}

MBOOL MDPNode::onRequestProcess(NodeID_T nodeId, RequestPtr& pRequest)
{
    MINT32 requestNo = pRequest->getRequestNo();
    MINT32 frameNo = pRequest->getFrameNo();
    MINT32 masterId = pRequest->getMainSensorIndex();
    CAM_TRACE_FMT_BEGIN("mdp:process|r%df%ds%d", requestNo, frameNo, masterId);
    MY_LOGI("(%d) +, R/F/S Num: %d/%d/%d", nodeId, requestNo, frameNo, masterId);
    const MBOOL isReproc     = (pRequest->hasParameter(PID_REPROCESSING) ? (pRequest->getParameter(PID_REPROCESSING) > 0) : MFALSE);
    const MBOOL isIgnoreCrop = (pRequest->hasParameter(PID_IGNORE_CROP) ? (pRequest->getParameter(PID_IGNORE_CROP) > 0) : MFALSE);
    const MBOOL isCroppedFSYUV = (pRequest->hasParameter(PID_CROPPED_FSYUV) ? (pRequest->getParameter(PID_CROPPED_FSYUV) > 0) : MFALSE);
    const MBOOL disableCrop = (isIgnoreCrop || isCroppedFSYUV);
    MY_LOGD("isIgnoreCrop:%d, isCroppedFSYUV:%d", isIgnoreCrop, isCroppedFSYUV);
    MBOOL ret = MTRUE;
    MINT32 iMagicNo = 0;

    mBufferItems.clear();

    sp<CaptureFeatureNodeRequest> pNodeReq = pRequest->getNodeRequest(nodeId);

    if (pNodeReq == NULL)
        return MFALSE;

    IMetadata* pInHalMeta = pNodeReq->acquireMetadata(MID_MAN_IN_HAL);
    IMetadata* pInAppMeta = pNodeReq->acquireMetadata(MID_MAN_IN_APP);
    IMetadata* pInDynamicMeta = pNodeReq->acquireMetadata(MID_MAN_IN_P1_DYNAMIC);

    if (pInHalMeta != NULL) {
        tryGetMetadata<MINT32>(pInHalMeta, MTK_P1NODE_PROCESSOR_MAGICNUM, iMagicNo);
    }

    // use IImageTransform to handle image cropping
    std::unique_ptr<IImageTransform, std::function<void(IImageTransform*)>> transform (
            IImageTransform::createInstance(PIPE_CLASS_TAG, masterId),
            [](IImageTransform *p)
            {
                if (p)
                    p->destroyInstance();
            }
        );

    if (transform.get() == NULL) {
        MY_LOGE("IImageTransform is NULL, cannot generate output");
        return MFALSE;
    }

    // Input
    BufferID_T uIBufFull = pNodeReq->mapBufferID(TID_MAN_FULL_YUV, INPUT);
    IImageBuffer* pSrcBuffer = pNodeReq->acquireBuffer(uIBufFull);
    if (pSrcBuffer == NULL) {
        MY_LOGE("no source image!");
        return BAD_VALUE;
    }

    // to prevent last node using CPU to write this buffer don't flush cache
    pSrcBuffer->syncCache(eCACHECTRL_FLUSH);

    // Output
    String8 str = String8::format("Resized(%d) IgnoreCrop(%d) CroppedFSYUV(%d) masterId(%d) R/F/M:%d/%d/%d",
                            0,
                            isIgnoreCrop, isCroppedFSYUV,
                            masterId,
                            pRequest->getRequestNo(),
                            pRequest->getFrameNo(),
                            iMagicNo);

    sp<CropCalculator::Factor> pFactor = mpCropCalculator->getFactor(pInAppMeta, pInHalMeta, masterId);

    // the larger size has higher priority
    const TypeID_T typeIds[] = {
            TID_JPEG,
            TID_MAN_CROP1_YUV,
            TID_MAN_CROP2_YUV,
            TID_MAN_CROP3_YUV,
            TID_POSTVIEW,
            TID_THUMBNAIL,
            TID_MAN_CLEAN,
            TID_MAN_BOKEH,
    };

    MINT32 index = 0;
    MINT32 lastRoundDRE = 0;
    MSize bufOriSize = MSize(0, 0);
    std::vector<std::unique_ptr<IHolder>> mpHolders;
    // SUPPORT_PQ_SIZE is from Android.mk
    MINT8 curUsablePQSize = SUPPORT_PQ_SIZE;
    for (TypeID_T typeId : typeIds) {
        BufferID_T bufId = pNodeReq->mapBufferID(typeId, OUTPUT);
        if (bufId == NULL_BUFFER)
            continue;

        IImageBuffer* pDstBuffer = pNodeReq->acquireBuffer(bufId);
        if (pDstBuffer == NULL)
            continue;

        BufferItem& rItem = mBufferItems.editItemAt(mBufferItems.add());
        if(curUsablePQSize >0 && (typeId == TID_JPEG || typeId == TID_MAN_CROP1_YUV
            || typeId == TID_MAN_CROP2_YUV || typeId == TID_MAN_CROP3_YUV))
        {
            rItem.mIsCapture = MTRUE;
            curUsablePQSize--;
        }
        rItem.mpImageBuffer = pDstBuffer;
        rItem.mTransform = pNodeReq->getImageTransform(bufId);
        rItem.mTypeId = typeId;

        MSize dstSize = pDstBuffer->getImgSize();
        if (rItem.mTransform & eTransform_ROT_90)
            dstSize = MSize(dstSize.h, dstSize.w);

        MRect& crop = rItem.mCrop;
        if (disableCrop) {
            CropCalculator::evaluate(pSrcBuffer->getImgSize(), dstSize, crop);
        } else if (isReproc) {  // reproc with cropping
            CropCalculator::evaluate(pSrcBuffer->getImgSize(), dstSize, pFactor->mActiveCrop, crop);
        } else {
            mpCropCalculator->evaluate(pFactor, dstSize, crop);
        }

        rItem.mRound = index / 2;

#if MTKCAM_EARLY_FLIP_SUPPORT == 1
        // Do the un-flip in MDP
        auto doUnflipInMDP= [&](MUINT32& trans, MINT32 dvOri) -> MVOID {
            if (dvOri == 270) {
                MY_LOGD("dvOri %d, Capture vertical", dvOri);
                trans |= eTransform_FLIP_V;
            } else if (dvOri == 90) {
                MY_LOGD("dvOri %d, Capture vertical", dvOri);
                trans &= ~eTransform_FLIP_V;
            } else if (dvOri == 180) {
                MY_LOGD("dvOri %d, Capture horizontal",dvOri);
                trans |= eTransform_FLIP_H;
            } else if (dvOri == 0) {
                MY_LOGD("dvOri %d, Capture horizontal",dvOri);
                trans &= ~eTransform_FLIP_H;
            }
        };

        if(typeId == TID_JPEG) {
            MINT32 jpegFlipProp = property_get_int32("vendor.debug.camera.Jpeg.flip", 0);
            MINT32 jpegFlip = 0;
            if(!tryGetMetadata<MINT32>(pInAppMeta, MTK_CONTROL_CAPTURE_JPEG_FLIP_MODE, jpegFlip))
                MY_LOGD("cannot get MTK_CONTROL_CAPTURE_JPEG_FLIP_MODE");
            if (jpegFlip || jpegFlipProp) {
                MINT32 jpegOri = 0;
                tryGetMetadata<MINT32>(pInAppMeta, MTK_JPEG_ORIENTATION, jpegOri);
                MY_LOGD("before trans 0x%u", rItem.mTransform);
                doUnflipInMDP(rItem.mTransform, jpegOri);
                MY_LOGD("after trans 0x%u", rItem.mTransform);
            }
        }
#endif

        str += String8::format(", Type(%s) Rot(%d) Crop(%d,%d)(%dx%d) Size(%dx%d) Cap(%d)",
                    TypeID2Name(typeId),
                    rItem.mTransform,
                    crop.p.x, crop.p.y, crop.s.w, crop.s.h,
                    pDstBuffer->getImgSize().w, pDstBuffer->getImgSize().h,
                    rItem.mIsCapture);

        // Second-round(1) mdp might use the first-round(0) output
        if (rItem.mRound > 0) {
            for (MINT32 i = index - 1; i >= 0; i--) {
                const BufferItem& rCandidate = mBufferItems.itemAt(i);
                if (rItem.mRound <= rCandidate.mRound)
                    continue;

                if (rCandidate.mTransform != 0)
                    continue;

                const MSize& srcSize = rCandidate.mpImageBuffer->getImgSize();
                if (srcSize.w < dstSize.w || srcSize.h < dstSize.h)
                    continue;

                rItem.mpSourceBuffer = rCandidate.mpImageBuffer;
                MY_LOGD("Copy candidate: Index(%d) Ptr(%p)",
                        index, rItem.mpSourceBuffer);
                break;
            }

            // Use output buffer by privous round only if they have the same source buffer
            if (index % 2 == 1) {
                BufferItem& rFirst = mBufferItems.editItemAt(index - 1);

                MY_LOGD("Copy candidate: Index(%d) Ptr(%p), Index(%d) Ptr(%p)",
                        index - 1, rFirst.mpImageBuffer,
                        index, rItem.mpImageBuffer);

                if (rFirst.mpSourceBuffer == NULL ||
                    rItem.mpSourceBuffer == NULL ||
                    rFirst.mpSourceBuffer != rItem.mpSourceBuffer)
                {
                    rFirst.mpSourceBuffer = NULL;
                    rItem.mpSourceBuffer = NULL;
                    lastRoundDRE = rItem.mRound;
                }
            }
        }

        // output buffer no need to fulfill
        const MBOOL isForceMargin = (property_get_int32("vendor.debug.camera.p2capture.forceMargin", 0) > 0);
        MBOOL isSetNoMargin = MTRUE;
        if (pInAppMeta != NULL) {
            tryGetMetadata<MINT32>(pInAppMeta, MTK_CONTROL_CAPTURE_YUV_NO_MARGIN, isSetNoMargin);
        }
        if ((!isSetNoMargin || isForceMargin) && pRequest->isTargetBuffer(bufId)) {
            MY_LOGD("buffer Id(%d) is target buffer, setNoMargin(%d), forceMargin(%d)", bufId, isSetNoMargin, isForceMargin);
            rItem.mOriSize = rItem.mpImageBuffer->getImgSize();
            rItem.mDumpWithMargin = MTRUE;
            mpHolders.push_back(IHolder::createResizerInstance(rItem.mpImageBuffer, crop.s));
            updateSensorCropRegion(pRequest, crop);
        }

        index++;
    }


    MY_LOGD("%s", str.string());

    if (pSrcBuffer == NULL || mBufferItems.size() == 0) {
        MY_LOGE("wrong mdp in/out: src %p, dst count %zu", pSrcBuffer, mBufferItems.size());
        return BAD_VALUE;
    }

    if (mpFdReader == NULL)
        mpFdReader = IFDContainer::createInstance(LOG_TAG, mUsageHint.mIsHidlIsp ? IFDContainer::eFDContainer_ISP_Opt_Read : IFDContainer::eFDContainer_Opt_Read);

    MINT64 p1timestamp = 0;
    if (!tryGetMetadata<MINT64>(pInHalMeta, MTK_P1NODE_FRAME_START_TIMESTAMP, p1timestamp))
        MY_LOGD("Can't get p1timestamp meta");

    MBOOL hasMdpSetting = MFALSE;
    // Lock FD Data
    vector<FD_DATATYPE*> vFdData = mpFdReader->queryLock(p1timestamp - FD_TOLERENCE, p1timestamp);
    {
        FD_DATATYPE* pFdData = NULL;
        FD_DATATYPE fdData;

        if (vFdData.empty()) {
            MY_LOGD("There is no face at ts:%" PRId64 "", p1timestamp);
            if (mLogLevel)
                mpFdReader->dumpInfo();
        } else {
            pFdData = vFdData.back();
            auto& activeArray = mpCropCalculator->getActiveArray(masterId);
            if (pFactor->mSensorSize != activeArray.s) {
                MY_LOGD("Active Array(%d,%d)(%dx%d) Sensor Size(%dx%d)",
                    activeArray.p.x, activeArray.p.y,
                    activeArray.s.w, activeArray.s.h,
                    pFactor->mSensorSize.w, pFactor->mSensorSize.h);

                // clone
                MtkCameraFaceMetadata *pDetectedFaces      = &pFdData->facedata;
                MtkCameraFaceMetadata *pCloneDetectedFaces = &fdData.facedata;

                // copy to clone fdData
                pCloneDetectedFaces->number_of_faces = pDetectedFaces->number_of_faces;
                MY_LOGD("number_of_faces %d", pCloneDetectedFaces->number_of_faces);
                for (size_t i = 0 ; i < pCloneDetectedFaces->number_of_faces ; i++) {
                    pCloneDetectedFaces->faces[i].rect[0] = pDetectedFaces->faces[i].rect[0] << 1;
                    pCloneDetectedFaces->faces[i].rect[1] = pDetectedFaces->faces[i].rect[1] << 1;
                    pCloneDetectedFaces->faces[i].rect[2] = pDetectedFaces->faces[i].rect[2] << 1;
                    pCloneDetectedFaces->faces[i].rect[3] = pDetectedFaces->faces[i].rect[3] << 1;
                }
                // pointer to clone fdData
                pFdData = &fdData;
            }
        }

        MINT32 iTimestamp = 0;
        MINT32 iRealLv  = 0;
        MINT32 iIsoValue = 0;
        if (pInHalMeta)
        {
            tryGetMetadata<MINT32>(pInHalMeta, MTK_PIPELINE_UNIQUE_KEY, iTimestamp);
            tryGetMetadata<MINT32>(pInHalMeta, MTK_REAL_LV, iRealLv);
            tryGetMetadata<MINT32>(pInDynamicMeta, MTK_SENSOR_SENSITIVITY, iIsoValue);
        }

        for (size_t i = 0, n = mBufferItems.size(); i < n; i++) {
            str.clear();
            BufferItem& item = mBufferItems.editItemAt(i);

            MBOOL hasSource = item.mpSourceBuffer != NULL;
            if (hasSource) {
                CropCalculator::evaluate(
                        item.mpSourceBuffer->getImgSize(),
                        item.mpImageBuffer->getImgSize(),
                        item.mCrop);
            }

            // 0:DST_BUF_0 1:DST_BUF_1
            size_t port = i % 2;
            // DRE requires that ports should have same DRE config

            IImageTransform::PQParam pqParam;
            pqParam.frameNo = pRequest->getFrameNo();
            pqParam.requestNo = pRequest->getRequestNo();
            pqParam.timestamp = iTimestamp;
            pqParam.lv_value  = iRealLv;
            pqParam.iso = iIsoValue;
            pqParam.sensorId = masterId;
            pqParam.mode = IImageTransform::Mode::Capture_Single;
            pqParam.enable = MTRUE;
            pqParam.portIdx = port;

            // just one mdp port can support clearzoom in the same time on some platform
            MBOOL enableCZ = !hasSource && (nodeId == NID_MDP) && pRequest->hasFeature(FID_CZ) && item.mIsCapture;
            MBOOL enableHFG = !hasSource && (nodeId == NID_MDP) && pRequest->hasFeature(FID_HFG) && item.mIsCapture;
            MINT32 useIspProfile = this->getISPProfileForMDP(pRequest, nodeId);
            if (enableCZ) {
                pqParam.type = IImageTransform::PQType::ClearZoom;
                // set CZConfig
                MUINT32 idx = 0;
                pqParam.cz.p_customSetting = (void*)getTuningFromNvram(masterId, idx, iMagicNo, NVRAM_TYPE_CZ, pInHalMeta, mLogLevel, mUsageHint.mIsHidlIsp, useIspProfile);
                str += String8::format("CZ Port(%zu) nvram:%p idx:%d ispProfile:%d ",
                                        port, pqParam.cz.p_customSetting, idx, useIspProfile);
            }

            if (enableHFG) {
                pqParam.type |= IImageTransform::PQType::HFG;
                // set CZConfig
                MUINT32 idx = 0;
                pqParam.hfg.p_lowerSetting = (void*)getTuningFromNvram(masterId, idx, iMagicNo, NVRAM_TYPE_HFG, pInHalMeta, mLogLevel, mUsageHint.mIsHidlIsp, useIspProfile);
                pqParam.hfg.p_upperSetting = pqParam.hfg.p_lowerSetting;
                str += String8::format("HFG Port(%zu) nvram:%p idx:%d ispProfile:%d ",
                                        port, pqParam.hfg.p_lowerSetting, idx, useIspProfile);
            }

            // all mdp port can enable dre in the same time
            // only capture stream applies DRE
            MBOOL enableDRE = !hasSource && (nodeId == NID_MDP) && pRequest->hasFeature(FID_DRE);
            if (enableDRE) {
                pqParam.type |= IImageTransform::PQType::DRE;
                pqParam.dre.cmd  = IImageTransform::DREParam::CMD::DRE_Apply;
                pqParam.dre.type = (item.mRound == lastRoundDRE)
                            ? IImageTransform::DREParam::HisType::His_One_Time
                            : IImageTransform::DREParam::HisType::His_Conti;
                pqParam.dre.userId = generatePQUserID(pqParam.frameNo, pqParam.sensorId, pRequest->isPhysicalStream());
                pqParam.dre.pBuffer = NULL;
                MUINT32 idx = 0;
                pqParam.dre.p_customSetting = (void*)getTuningFromNvram(masterId, idx, iMagicNo, NVRAM_TYPE_DRE, pInHalMeta, mLogLevel, mUsageHint.mIsHidlIsp, useIspProfile);
                pqParam.dre.customIdx = idx;
                str += String8::format("DRE cmd:0x%x Type:0x%x User:%" PRIu64 " Buffer:%p nvram:%p idx:%d ispProfile:%d ",
                                        pqParam.dre.cmd, pqParam.dre.type,
                                        pqParam.dre.userId,
                                        pqParam.dre.pBuffer,
                                        pqParam.dre.p_customSetting,
                                        idx,
                                        useIspProfile);
            }

            pqParam.p_mdpSetting = NULL;
            // Dump debug info for JPEG YUV
            if (enableDRE || enableCZ || enableHFG) {

                if (mDebugDumpMDP) {
                    hasMdpSetting = MTRUE;
                    if (mpMdpSetting == NULL) {
                        mpMdpSetting = new MDPSetting();
                        mpMdpSetting->size = MDPSETTING_MAX_SIZE;
                        mpMdpSetting->buffer = ::malloc(MDPSETTING_MAX_SIZE);
                    }

                    if (mpMdpSetting->buffer == NULL) {
                        MY_LOGE("fail to allocate mdp dbg buffer!");
                    } else if (item.mIsCapture){
                        mpMdpSetting->offset = 0;
                        memset(mpMdpSetting->buffer, 0xFF, MDPSETTING_MAX_SIZE);
                        pqParam.p_mdpSetting = (void*) mpMdpSetting;
                        str += String8::format("setting:0x%p ", pqParam.p_mdpSetting);
                    }
                }

                // Face Data
                pqParam.p_faceInfor = pFdData;

                str += String8::format("PQtype:0x%x Iso:%d Timestamp:%d",
                        pqParam.type,
                        pqParam.iso,
                        pqParam.timestamp);
            }

            transform->setPQParameter(pqParam);

            if (str.length() > 0)
                MY_LOGD("%s", str.string());
            //
            MBOOL bNeedDump = this->needToDump(pRequest);

            if (i % 2 == 1 || i + 1 == n) {
                CAM_TRACE_BEGIN("mdp:execute");
                MBOOL hasSecond = i % 2 == 1;
                size_t base = hasSecond ? i - 1 : i;
                transform->setDumpInfo(IImageTransform::DumpInfos(requestNo, frameNo, p1timestamp));
                struct timeval current;
                gettimeofday(&current, NULL);
                ret = transform->execute(
                                hasSource ? mBufferItems[base].mpSourceBuffer : pSrcBuffer,
                                mBufferItems[base].mpImageBuffer,
                                (hasSecond) ? mBufferItems[base + 1].mpImageBuffer : 0L,
                                mBufferItems[base].mCrop,
                                (hasSecond) ? mBufferItems[base + 1].mCrop : 0L,
                                mBufferItems[base].mTransform,
                                (hasSecond) ? mBufferItems[base + 1].mTransform : 0L,
                                0xFFFFFFFF,
                                &current,
                                bNeedDump);
                CAM_TRACE_END();

                if(mDebugDump && bNeedDump)
                {
                    MINT32 iUniqueKey = 0;
                    tryGetMetadata<MINT32>(pInHalMeta, MTK_PIPELINE_UNIQUE_KEY, iUniqueKey);

                    char writepath[DUMP_FILE_NAME_SIZE] = {0};
                    char filename[DUMP_FILE_NAME_SIZE] = {0};
                    FILE_DUMP_NAMING_HINT hint;
                    hint.UniqueKey          = iUniqueKey;
                    hint.RequestNo          = pRequest->getRequestNo();
                    hint.FrameNo            = pRequest->getFrameNo();
                    extract_by_SensorOpenId(&hint, masterId);

                    // dump source
                    if(mBufferItems[base].mRound == 0)
                    {
                        snprintf(filename, DUMP_FILE_NAME_SIZE, "MDP_IN_TID_MAN_FULL_YUV");
                        extract(&hint, pSrcBuffer);
                        genFileName_YUV(writepath, DUMP_FILE_NAME_SIZE, &hint, TuningUtils::YUV_PORT_UNDEFINED, filename);
                        pSrcBuffer->saveToFile(writepath);
                    }
                }
            }
        }
    }
    mpFdReader->queryUnlock(vFdData);

    // Clear MDP setting's memory and update metadata
    if (hasMdpSetting) {
        unsigned char* pBuffer = static_cast<unsigned char*>(mpMdpSetting->buffer);
        MUINT32 size = mpMdpSetting->size;
        if (pBuffer) {
            MY_LOGI("Update Mdp debug info: addr %p, size %u %d %d", pBuffer, size, *pBuffer, *(pBuffer+1));
            if (mLogLevel > 2 && this->needToDump(pRequest)) {
                char filename[256] = {0};
                if (!makePath("data/vendor/camera_dump", 0660))
                    MY_LOGD("makePath[%s] fails", "data/vendor/camera_dump");
                sprintf(filename, "data/vendor/camera_dump/mdp_dump");
                saveBufToFile(filename, pBuffer, size);
            }

            IMetadata* pOutHalMeta = pNodeReq->acquireMetadata(MID_MAN_OUT_HAL);
            if (pOutHalMeta) {
                MY_LOGD_IF(mLogLevel, "set debug exif of mdp +");
                IMetadata exifMeta;
                tryGetMetadata<IMetadata>(pOutHalMeta, MTK_3A_EXIF_METADATA, exifMeta);
                if (DebugExifUtils::setDebugExif(
                        DebugExifUtils::DebugExifType::DEBUG_EXIF_RESERVE3,
                        static_cast<MUINT32>(MTK_RESVC_EXIF_DBGINFO_KEY),
                        static_cast<MUINT32>(MTK_RESVC_EXIF_DBGINFO_DATA),
                        size, pBuffer, &exifMeta) == nullptr)
                {
                    MY_LOGW("fail to set debug exif to metadata");
                }
                else
                {
                    trySetMetadata<IMetadata>(pOutHalMeta, MTK_3A_EXIF_METADATA, exifMeta);
                }
                MY_LOGD_IF(mLogLevel, "set debug exif of mdp -");
            }
        }
    }

    MY_LOGI("(%d) -, R/F/S Num: %d/%d/%d", nodeId, requestNo, frameNo, masterId);

    if (nodeId == NID_MDP && mDebugDump && this->needToDump(pRequest)) {

        MINT32 iUniqueKey = 0;
        tryGetMetadata<MINT32>(pInHalMeta, MTK_PIPELINE_UNIQUE_KEY, iUniqueKey);

        char filename[256] = {0};
        FILE_DUMP_NAMING_HINT hint;
        hint.UniqueKey          = iUniqueKey;
        hint.RequestNo          = pRequest->getRequestNo();
        hint.FrameNo            = pRequest->getFrameNo();
        extract_by_SensorOpenId(&hint, masterId);

        for (size_t i = 0; i < mBufferItems.size(); i++) {
            if ((i == 0 && mDebugDumpFilter & DUMP_ROUND1_DP0) ||
                (i == 1 && mDebugDumpFilter & DUMP_ROUND1_DP1))
            {
                // requested to down
            }
            else
                continue;

            const BufferItem& item = mBufferItems[i];

            IImageBuffer* pImgBuf = item.mpImageBuffer;
            extract(&hint, pImgBuf);
            if (item.mDumpWithMargin) {
                MY_LOGD("reset dump size");
                hint.ImgWidth = item.mOriSize.w;
                hint.ImgHeight = item.mOriSize.h;
            }
            genFileName_YUV(filename, sizeof(filename), &hint,
                    (i % 2 == 0) ? YUV_PORT_DP0 : YUV_PORT_DP1);

            pImgBuf->saveToFile(filename);
            MY_LOGD("Dump: %s", filename);
        }
    }

    CAM_TRACE_FMT_END();
    return ret;
}

MVOID MDPNode::onRequestFinish(NodeID_T nodeId, RequestPtr& pRequest)
{
    pRequest->decNodeReference(nodeId);

    if (nodeId == NID_MDP && pRequest->getParameter(PID_ENABLE_NEXT_CAPTURE) > 0)
    {
        if (pRequest->getParameter(PID_THUMBNAIL_TIMING) == NSPipelinePlugin::eTiming_MDP)
        {
            if (pRequest->mpCallback != NULL) {
                MY_LOGD("Notify next capture");
                pRequest->mpCallback->onContinue(pRequest);
            } else {
                MY_LOGW("have no request callback instance!");
            }
        }
    }

    dispatch(pRequest, nodeId);
}

MERROR MDPNode::evaluate(NodeID_T nodeId, CaptureFeatureInferenceData& rInfer)
{
    auto& srcData = rInfer.getSharedSrcData();
    auto& dstData = rInfer.getSharedDstData();
    auto& features = rInfer.getSharedFeatures();
    auto& metadatas = rInfer.getSharedMetadatas();

    MERROR status = OK;
    if (nodeId == NID_MDP) {
        auto& src_0 = srcData.editItemAt(srcData.add());
        src_0.mTypeId = TID_MAN_FULL_YUV;
        src_0.mSizeId = SID_FULL;

        // THUMBNAIL might have no any MDP feature
        {
            auto& dst_1 = dstData.editItemAt(dstData.add());
            dst_1.mTypeId = TID_THUMBNAIL;

            if(!rInfer.addNodeIO(nodeId, srcData, dstData, metadatas, features))
                status = BAD_VALUE;
            dstData.clear();
        }

        if (rInfer.hasFeature(FID_DRE))
            features.push_back(FID_DRE);

        // CROP1/CROP2 might have DRE feature
        {
            auto& dst_2 = dstData.editItemAt(dstData.add());
            dst_2.mTypeId = TID_MAN_CROP1_YUV;

            auto& dst_3 = dstData.editItemAt(dstData.add());
            dst_3.mTypeId = TID_MAN_CROP2_YUV;

            auto& dst_4 = dstData.editItemAt(dstData.add());
            dst_4.mTypeId = TID_MAN_CROP3_YUV;


            if(!rInfer.addNodeIO(nodeId, srcData, dstData, metadatas, features))
                status = BAD_VALUE;
            dstData.clear();
        }

        auto& dst_5 = dstData.editItemAt(dstData.add());
        dst_5.mTypeId = TID_JPEG;

        if (rInfer.mThumbnailTiming.hasBit(NSPipelinePlugin::eTiming_MDP)) {
            auto& dst_6 = dstData.editItemAt(dstData.add());
            dst_6.mTypeId = TID_POSTVIEW;
        }

        if (rInfer.hasFeature(FID_CZ))
            features.push_back(FID_CZ);
        if (rInfer.hasFeature(FID_HFG))
            features.push_back(FID_HFG);

    } else if (nodeId == NID_MDP_B) {
        auto& src_0 = srcData.editItemAt(srcData.add());
        src_0.mTypeId = TID_MAN_FULL_YUV;
        src_0.mSizeId = SID_FULL;

        auto& dst_1 = dstData.editItemAt(dstData.add());
        dst_1.mTypeId = TID_MAN_BOKEH;

    } else if (nodeId == NID_MDP_C) {
        auto& src_0 = srcData.editItemAt(srcData.add());
        src_0.mTypeId = TID_MAN_FULL_YUV;
        src_0.mSizeId = SID_FULL;

        auto& dst_1 = dstData.editItemAt(dstData.add());
        dst_1.mTypeId = TID_MAN_CLEAN;

    } else {
        MY_LOGE("Unknow id:%d for MDPNode", nodeId);
        return BAD_VALUE;
    }

    metadatas.push_back(MID_MAN_IN_P1_DYNAMIC);
    metadatas.push_back(MID_MAN_IN_APP);
    metadatas.push_back(MID_MAN_IN_HAL);
    metadatas.push_back(MID_MAN_OUT_APP);
    metadatas.push_back(MID_MAN_OUT_HAL);
    metadatas.push_back(MID_MAN_OUT_APP_PHY);

    if(!rInfer.addNodeIO(nodeId, srcData, dstData, metadatas, features))
        status = BAD_VALUE;

    return status;
}



} // NSCapture
} // namespace NSFeaturePipe
} // namespace NSCamFeature
} // namespace NSCam
