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

#include "FDNode.h"

#define PIPE_CLASS_TAG "FDNode"
#define PIPE_TRACE TRACE_FD_NODE
#include <featurePipe/core/include/PipeLog.h>

#include <mtkcam/drv/iopipe/SImager/IImageTransform.h>
#include <mtkcam/utils/hw/HwTransform.h>

#include <mtkcam/utils/metadata/client/mtk_metadata_tag.h>
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>

CAM_ULOG_DECLARE_MODULE_ID(MOD_CAPTURE_FD);

#define FD_FACE_NUM                 (15)
#define FD_40_WORKING_BUF_SIZE      (1024 * 4)    // 4K if FD version >= 4.0
#define FD_WORKING_BUF_SIZE         (1024 * 1024 * 4)
#define FD_PURE_Y_BUF_SIZE          (640 * 480 * 2)

namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {
namespace NSCapture {

FDNode::FDNode(NodeID_T nid, const char* name, MINT32 policy, MINT32 priority)
    : CamNodeULogHandler(Utils::ULog::MOD_CAPTURE_FD)
    , CaptureFeatureNode(nid, name, 0, policy, priority)
    , mpFDHalObj(NULL)
    , mpFDWorkingBuffer(NULL)
    , mpPureYBuffer(NULL)
    , mpDetectedFaces(NULL)
{
    TRACE_FUNC_ENTER();
    this->addWaitQueue(&mRequests);
    TRACE_FUNC_EXIT();
}

FDNode::~FDNode()
{
    TRACE_FUNC_ENTER();
    TRACE_FUNC_EXIT();
}

MBOOL FDNode::onData(DataID id, const RequestPtr& pRequest)
{
    TRACE_FUNC_ENTER();
    MY_LOGD_IF(mLogLevel, "Frame %d: %s arrived", pRequest->getRequestNo(), PathID2Name(id));

    MBOOL ret = MTRUE;
    mRequests.enque(pRequest);

    TRACE_FUNC_EXIT();
    return ret;
}

MBOOL FDNode::onInit()
{
    TRACE_FUNC_ENTER();
    CaptureFeatureNode::onInit();

    mpDetectedFaces = new MtkCameraFaceMetadata();
    if (NULL != mpDetectedFaces) {
        mpDetectedFaces->number_of_faces = 0;
    } else {
        MY_LOGE("fail to allocate face metadata!");
        return MFALSE;
    }

    mpFDHalObj = halFDBase::createInstance(HAL_FD_OBJ_FDFT_SW);
    if (mpFDHalObj == NULL) {
        MY_LOGE("fail to create FD instance!");
        return MFALSE;
    }

    if (mpFDHalObj->halFDGetVersion() < HAL_FD_VER_HW40) {
        mpFDWorkingBuffer = new unsigned char[FD_WORKING_BUF_SIZE];
        mpPureYBuffer = new unsigned char[FD_PURE_Y_BUF_SIZE];
        if (mpFDWorkingBuffer == NULL || mpPureYBuffer == NULL) {
            MY_LOGE("fail to allocate working buffer!");
            return MFALSE;
        }
    } else {
        mpFDWorkingBuffer = new unsigned char[FD_40_WORKING_BUF_SIZE];
        if (mpFDWorkingBuffer == NULL) {
            MY_LOGE("fail to allocate working buffer!");
            return MFALSE;
        }
    }

    TRACE_FUNC_EXIT();
    return MTRUE;
}

MBOOL FDNode::onUninit()
{
    TRACE_FUNC_ENTER();

    if (mpFDHalObj != NULL) {
        mpFDHalObj->destroyInstance();
        mpFDHalObj = NULL;
    }

    if (mpFDWorkingBuffer != NULL) {
        delete mpFDWorkingBuffer;
        mpFDWorkingBuffer = NULL;
    }

    if (mpPureYBuffer != NULL) {
        delete mpPureYBuffer;
        mpPureYBuffer = NULL;
    }

    if (mpDetectedFaces != NULL) {
        delete mpDetectedFaces;
        mpDetectedFaces = NULL;
    }

    TRACE_FUNC_EXIT();
    return MTRUE;
}

MBOOL FDNode::onThreadStart()
{
    TRACE_FUNC_ENTER();
    MBOOL ret = MTRUE;
    TRACE_FUNC_EXIT();
    return ret;
}

MBOOL FDNode::onThreadStop()
{
    TRACE_FUNC_ENTER();
    TRACE_FUNC_EXIT();
    return MTRUE;
}


MBOOL FDNode::onThreadLoop()
{
    TRACE_FUNC_ENTER();
    if (!waitAllQueue())
    {
        TRACE_FUNC("Wait all queue exit");
        return MFALSE;
    }

    RequestPtr pRequest;
    if (!mRequests.deque(pRequest)) {
        MY_LOGE("Request deque out of sync");
        return MFALSE;
    } else if (pRequest == NULL) {
        MY_LOGE("Request out of sync");
        return MFALSE;
    }

    {
        onRequestProcess(pRequest);
    }
    TRACE_FUNC_EXIT();
    return MTRUE;

}

MBOOL FDNode::onRequestProcess(RequestPtr& pRequest)
{
    MINT32 requestNo = pRequest->getRequestNo();
    MINT32 frameNo = pRequest->getFrameNo();
    MINT32 masterId = pRequest->getMainSensorIndex();
    CAM_TRACE_FMT_BEGIN("fd:process|r%df%ds%d", requestNo, frameNo, masterId);
    MY_LOGD("+, R/F/S Num: %d/%d/%d", requestNo, frameNo, masterId);

    sp<CaptureFeatureNodeRequest> pNodeReq = pRequest->getNodeRequest(NID_FD);
    if (pNodeReq == NULL) {
        MY_LOGE("should not be here if no node request");
        return MFALSE;
    }

    // [1] acquire the FD buffer whose width must be not more than 640
    IImageBuffer* pInBuffer = NULL;
    BufferID_T uFDBuffer = pNodeReq->mapBufferID(TID_MAN_FD_YUV, INPUT);
    if (uFDBuffer != NULL_BUFFER)
        pInBuffer = pNodeReq->acquireBuffer(uFDBuffer);
    if (pInBuffer == NULL) {
        MY_LOGE("cannot acquire FD buffer!");
        return MFALSE;
    }

    IMetadata* pAppMeta = pNodeReq->acquireMetadata(MID_MAN_IN_APP);
    MINT32 jpegOrientation;
    if(!tryGetMetadata<MINT32>(pAppMeta, MTK_JPEG_ORIENTATION, jpegOrientation)) {
        MY_LOGE("cannot find MTK_JPEG_ORIENTATION in APP metadata, pAppMeta:%p", pAppMeta);
        return MFALSE;
    }

    IMetadata* pHalMeta = pNodeReq->acquireMetadata(MID_MAN_IN_HAL);
    MINT32 sensorMode;
    if (!tryGetMetadata<MINT32>(pHalMeta, MTK_P1NODE_SENSOR_MODE, sensorMode)) {
        MY_LOGE("cannot find MTK_P1NODE_SENSOR_MODE in HAL metadata, pHalMeta:%p", pHalMeta);
        return MFALSE;
    }

    NSCamHW::HwTransHelper hwTransHelper(masterId);
    NSCamHW::HwMatrix active2TGMatrix;
    if(!hwTransHelper.getMatrixFromActive(sensorMode, active2TGMatrix)) {
        MY_LOGW("failed to create instance, can not get active2TGMatrix, sensorMode:%d", sensorMode);
        return MFALSE;
    }

    MSize fdSize = pInBuffer->getImgSize();
    if (fdSize.w > 640) {
        MY_LOGE("can not support the buffer size(%dx%d)", fdSize.w, fdSize.h);
        return MFALSE;
    }

    // [2] FD init
    int ret = mpFDHalObj->halFDInit(fdSize.w, fdSize.h, mpFDWorkingBuffer, FD_WORKING_BUF_SIZE, 1, HAL_FD_MODE_MANUAL);

    if (ret) {
        MY_LOGE("fail to init FD object!");
        return MFALSE;
    }

    // [3] extract pure Y buffer
    if (mpPureYBuffer != NULL) {
        mpFDHalObj->halFDYUYV2ExtractY(
                mpPureYBuffer,
                (MUINT8 *) pInBuffer->getBufVA(0),
                fdSize.w, fdSize.h);
    }

    struct FD_Frame_Parameters param;
    param.pScaleImages = NULL;
    // on FD HW Version above 4.2
    param.pRGB565Image = (MUINT8 *) pInBuffer->getBufVA(0);
    param.pPureYImage  = mpPureYBuffer;
    param.pImageBufferVirtual = (MUINT8 *) pInBuffer->getBufVA(0);
    param.pImageBufferPhyP0 = (MUINT8 *) pInBuffer->getBufPA(0);
    param.pImageBufferPhyP1 = NULL;
    param.pImageBufferPhyP2 = NULL;
    param.Rotation_Info = jpegOrientation;
    param.SDEnable = 0;
    param.AEStable = 0;
    param.padding_w = 0;
    param.padding_h = 0;

    // [4] do face detection
    ret = mpFDHalObj->halFDDo(param);
    if (ret) {
        MY_LOGW("fail to do face dection. ret=%d", ret);
        return MFALSE;
    }

    MINT32 numFace = mpFDHalObj->halFDGetFaceResult(mpDetectedFaces);

    // [5] FD uninit
    ret = mpFDHalObj->halFDUninit();
    if (ret) {
        MY_LOGE("fail to uninit FD object!");
        return MFALSE;
    }

    MY_LOGD("fd result: orientation=%d,face num=%d", jpegOrientation,  numFace);

    // [6] update into app metadata
    // TODO: only support full size coordinate for now
    MRect cropRegion = mpCropCalculator->getActiveArray(masterId);

    MY_LOGD("fd crop region(%d,%d)(%dx%d) from metadata",
            cropRegion.p.x, cropRegion.p.y, cropRegion.s.w, cropRegion.s.h);

    // Map to FD's coordinate
    int aspect = cropRegion.s.w * fdSize.h - cropRegion.s.h * fdSize.w;
    // Pillar
    if (aspect > 0) {
        int newCropW = div_round(cropRegion.s.h * fdSize.w, fdSize.h);
        cropRegion.p.x += (cropRegion.s.w - newCropW) >> 1;
        cropRegion.s.w = newCropW;
    // Letter
    } else if (aspect < 0) {
        int newCropH = div_round(cropRegion.s.w * fdSize.h, fdSize.w);
        cropRegion.p.y += (cropRegion.s.h - newCropH) >> 1;
        cropRegion.s.h = newCropH;
    }

    MRect faceRect;
    MRect faceRectInTG;

    IMetadata::IEntry entryFaceRects(MTK_STATISTICS_FACE_RECTANGLES);
    IMetadata::IEntry entryFaceLandmarks(MTK_STATISTICS_FACE_LANDMARKS);
    IMetadata::IEntry entryFaceId(MTK_STATISTICS_FACE_IDS);
    IMetadata::IEntry entryFaceScores(MTK_STATISTICS_FACE_SCORES);
    IMetadata::IEntry entryPoseOriens(MTK_FACE_FEATURE_POSE_ORIENTATIONS);

    IMetadata::IEntry entryHalFaceRects(MTK_FEATURE_FACE_RECTANGLES);
    IMetadata::IEntry entryHalPoseOriens(MTK_FEATURE_FACE_POSE_ORIENTATIONS);

    IMetadata::IEntry outEntryFaceRects(MTK_STATISTICS_FACE_RECTANGLES);

    const size_t faceCount = mpDetectedFaces->number_of_faces;
    for (size_t i = 0; i < faceCount; i++) {
        MtkCameraFace& face = mpDetectedFaces->faces[i];

        // APP Face Rectangle
        faceRect.p.x = ((face.rect[0]+1000) * cropRegion.s.w/2000) + cropRegion.p.x;    // left
        faceRect.p.y = ((face.rect[1]+1000) * cropRegion.s.h/2000) + cropRegion.p.y;    // top
        faceRect.s.w = ((face.rect[2]+1000) * cropRegion.s.w/2000) + cropRegion.p.x;    // right
        faceRect.s.h = ((face.rect[3]+1000) * cropRegion.s.h/2000) + cropRegion.p.y;    // bottom
        entryFaceRects.push_back(faceRect, Type2Type<MRect>());

        // APP Face Landmark
        entryFaceLandmarks.push_back(face.left_eye[0], Type2Type<MINT32>());            // left eye X
        entryFaceLandmarks.push_back(face.left_eye[1], Type2Type<MINT32>());            // left eye Y
        entryFaceLandmarks.push_back(face.right_eye[0], Type2Type<MINT32>());           // right eye X
        entryFaceLandmarks.push_back(face.right_eye[1], Type2Type<MINT32>());           // right eye Y
        entryFaceLandmarks.push_back(face.mouth[0], Type2Type<MINT32>());               // mouth X
        entryFaceLandmarks.push_back(face.mouth[1], Type2Type<MINT32>());               // mouth Y

        // APP Face ID
        entryFaceId.push_back(face.id, Type2Type<MINT32>());

        // APP Face Score
        if (face.score > 100)
            face.score = 100;
        entryFaceScores.push_back(face.score, Type2Type<MUINT8>());

        // APP Face Pose
        const MINT32 poseX = 0;
        const MINT32 poseY = mpDetectedFaces->fld_rop[i];
        const MINT32 poseZ = mpDetectedFaces->fld_rip[i];
        entryPoseOriens.push_back(poseX, Type2Type<MINT32>());                              // pose X asix
        entryPoseOriens.push_back(poseY, Type2Type<MINT32>());    // pose Y asix
        entryPoseOriens.push_back(poseZ, Type2Type<MINT32>());    // pose Z asix

        // HAL Face Rectangle
        // active to tg domain
        MPoint tgLeftTop;
        MPoint tgRightBottom;
        active2TGMatrix.transform(faceRect.p, tgLeftTop);
        active2TGMatrix.transform(MPoint(faceRect.s.w, faceRect.s.h), tgRightBottom);
        // tg to pipe domain
        MPoint pipeLeftTop;
        MPoint pipeRightBottom ;

        if (!mpFOVCalculator->getIsEnable() || !mpFOVCalculator->transform(masterId, tgLeftTop, tgRightBottom, pipeLeftTop, pipeRightBottom)) {
            pipeLeftTop     = tgLeftTop;
            pipeRightBottom = tgRightBottom;
        }

        MY_LOGD("detected face rectangle, faceNum:%zu/%zu, act:(%d, %d)x(%d, %d), tg:(%d, %d)x(%d, %d), pi:(%d, %d)x(%d, %d)",
            i, faceCount,
            faceRect.p.x, faceRect.p.y, faceRect.s.w, faceRect.s.h,
            tgLeftTop.x, tgLeftTop.y, tgRightBottom.x, tgRightBottom.y,
            pipeLeftTop.x, pipeLeftTop.y, pipeRightBottom.x, pipeRightBottom.y);
        entryHalFaceRects.push_back(MRect(pipeLeftTop, MSize(pipeRightBottom.x, pipeRightBottom.y)), Type2Type<MRect>());
        outEntryFaceRects.push_back(MRect(pipeLeftTop, MSize(pipeRightBottom.x, pipeRightBottom.y)), Type2Type<MRect>());

        // HAL Face Pose
        MY_LOGD("detected face pose orientation, faceNum:%zu/%zu, (x, y, z):(%d, %d, %d)",
            i, faceCount,
            poseX, poseY, poseZ);
        entryHalPoseOriens.push_back(poseX, Type2Type<MINT32>());    // pose X asix
        entryHalPoseOriens.push_back(poseY, Type2Type<MINT32>());    // pose Y asix
        entryHalPoseOriens.push_back(poseZ, Type2Type<MINT32>());    // pose Z asix
    }

    pAppMeta->update(MTK_STATISTICS_FACE_RECTANGLES, entryFaceRects);
    pAppMeta->update(MTK_STATISTICS_FACE_LANDMARKS, entryFaceLandmarks);
    pAppMeta->update(MTK_STATISTICS_FACE_IDS, entryFaceId);
    pAppMeta->update(MTK_STATISTICS_FACE_SCORES, entryFaceScores);
    pAppMeta->update(MTK_FACE_FEATURE_POSE_ORIENTATIONS, entryPoseOriens);

    pHalMeta->update(MTK_FEATURE_FACE_RECTANGLES, entryHalFaceRects);
    pHalMeta->update(MTK_FEATURE_FACE_POSE_ORIENTATIONS, entryHalPoseOriens);

    IMetadata* pOAppMeta = pNodeReq->acquireMetadata(MID_MAN_OUT_APP);
    if (pOAppMeta != nullptr)
        pOAppMeta->update(outEntryFaceRects.tag(), outEntryFaceRects);

    // Release
    pRequest->decNodeReference(NID_FD);

    dispatch(pRequest);

    MY_LOGD("-, R/F/S Num: %d/%d/%d", requestNo, frameNo, masterId);
    CAM_TRACE_FMT_END();
    return ret;
}

MERROR FDNode::evaluate(NodeID_T nodeId, CaptureFeatureInferenceData& rInfer)
{
    (void) nodeId;
    auto& srcData = rInfer.getSharedSrcData();
    auto& dstData = rInfer.getSharedDstData();
    auto& features = rInfer.getSharedFeatures();
    auto& metadatas = rInfer.getSharedMetadatas();

    MERROR status = OK;
    if (rInfer.hasType(TID_MAN_FD_YUV))
    {

        MSize srcSize;
        if (rInfer.hasType(TID_MAN_FULL_RAW))
            srcSize = rInfer.getSize(TID_MAN_FULL_RAW);
        else if (rInfer.hasType(TID_MAN_RSZ_RAW))
            srcSize = rInfer.getSize(TID_MAN_RSZ_RAW);
        else
            return OK;

        // FD requires a buffer whose size is less than 640P
        MSize fdSize;
        if (srcSize.w > 640) {
            fdSize.w = 640;
            fdSize.h = align(div_round(srcSize.h * 640, srcSize.w), 1);
        } else
            fdSize = srcSize;

        if (rInfer.hasType(TID_MAN_FD_YUV)) {
            auto& src_0 = srcData.editItemAt(srcData.add());
            src_0.mTypeId = TID_MAN_FD_YUV;
            src_0.mSizeId = SID_SPECIFIED;
            src_0.mSize = fdSize;
            src_0.setFormat(eImgFmt_YUY2);

            auto& dst_0 = dstData.editItemAt(dstData.add());
            dst_0.mTypeId = TID_MAN_FD;
            dst_0.mSizeId = NULL_SIZE;

            metadatas.push_back(MID_MAN_IN_HAL);
            metadatas.push_back(MID_MAN_IN_APP);

            metadatas.push_back(MID_MAN_OUT_APP);

            if(!rInfer.addNodeIO(NID_FD, srcData, dstData, metadatas, features))
                status = BAD_VALUE;
        }
    }

    return status;
}

} // NSCapture
} // namespace NSFeaturePipe
} // namespace NSCamFeature
} // namespace NSCam
