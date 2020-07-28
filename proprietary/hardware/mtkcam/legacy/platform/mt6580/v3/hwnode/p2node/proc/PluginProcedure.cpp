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
 * MediaTek Inc. (C) 2010. All rights reserved.
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

#define DEFINE_OPEN_ID      (muOpenId)

#include "PluginProcedure.h"

/******************************************************************************
 *
 ******************************************************************************/
PluginProcedure::
PluginProcedure(CreateParams const &params)
        : mbEnableLog(params.bEnableLog),
          muOpenId(params.uOpenId),
          mpPoolAllocatorMap(params.pPoolAllocatorMap),
          mpPostProcessing(params.pPostProcessing),
          mbRawDomain(params.bRawDomain)
{
    muDumpBuffer = ::property_get_int32("vendor.debug.camera.dump.plugin", 0);
}


/******************************************************************************
 *
 ******************************************************************************/
PluginProcedure::
~PluginProcedure() {
    if (mpPostProcessing != NULL)
        delete mpPostProcessing;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
PluginProcedure::
onPluginPullParams(
        sp<Request> pRequest,
        FrameParams &frameParams)
{
    FUNC_START;
    if (pRequest->context.in_app_meta == NULL || pRequest->context.in_hal_meta == NULL) {
        MY_LOGEO("metadata should not be null");
    }

    frameParams.pInAppMeta = pRequest->context.in_app_meta;
    frameParams.pInHalMeta = pRequest->context.in_hal_meta;
    frameParams.bResized =  pRequest->context.resized;
    frameParams.uFrameNo = pRequest->getFrameNo();
    frameParams.captureFrame = pRequest->context.captureFrame;

    PostProcessing::ConditionParams condition;
    condition.pInAppMeta = frameParams.pInAppMeta->getMetadata();
    condition.pInHalMeta = frameParams.pInHalMeta->getMetadata();
    condition.bResized = frameParams.bResized;

    PostProcessing::DecisionParams &decision = frameParams.decision;
    if (!mpPostProcessing->decide(condition, decision) || !decision.verify()) {
        return NOT_ENOUGH_DATA;
    }

    MY_LOGDO_IF(mbEnableLog, "meet condition: frame[%d], resized:%d",
            frameParams.uFrameNo, frameParams.bResized);

    if (mbRawDomain && pRequest->context.in_buffer != NULL)
        frameParams.pInBuffer = pRequest->context.in_buffer;
    else if (!mbRawDomain && pRequest->context.work_buffer != NULL) {
        frameParams.pInBuffer = pRequest->context.work_buffer;
        // assign mdp buffer for next step copying
        pRequest->context.in_mdp_buffer = pRequest->context.work_buffer;
    }
    else {
        MY_LOGEO("no src");
        return UNKNOWN_ERROR;
    }
    pRequest->context.work_buffer.clear();

    MERROR ret = OK;
    IImageBuffer *pSrcBuffer = NULL;
    if (frameParams.pInBuffer.get()) {
        // force to get write-lock if in-place processing
        BufferHandle::BufferState_t waitState = (decision.processType == PostProcessing::eIN_PLACE_PROCESSING) ?
                                  BufferHandle::STATE_WRITABLE : BufferHandle::STATE_READABLE;
        if (OK != (ret = frameParams.pInBuffer->waitState(waitState))) {
            MY_LOGWO("src buffer err = %d", ret);
            return ret;
        }
        pSrcBuffer = frameParams.pInBuffer->getBuffer();
    }
    else {
        MY_LOGWO("no src");
        return BAD_VALUE;
    }

    if (decision.processType == PostProcessing::eIN_OUT_PROCESSING) {
        // create working buffer of 3rd party out with the specific format
        MUINT32 imgFormat = (decision.outImgFmt == PostProcessing::eIMG_FMT_DEFAULT) ?
                                pSrcBuffer->getImgFormat() : decision.outImgFmt;
        MSize imgSize = pSrcBuffer->getImgSize();
        String8 bufferName = String8::format("pool:s%dx%d:f%d",
                imgSize.w, imgSize.h, imgFormat);

        sp<PoolBufferAllocator> poolAllocator;
        if (mpPoolAllocatorMap->indexOfKey(bufferName) >= 0) {
            poolAllocator = mpPoolAllocatorMap->valueFor(bufferName);
        }
        else {
            poolAllocator = PoolBufferAllocator::create(imgFormat, imgSize);
            mpPoolAllocatorMap->add(bufferName,poolAllocator);
        }
        sp<BufferHandle> pBufferHandle = PoolBufferHandle::create(bufferName.string(), poolAllocator);

        if (pBufferHandle == NULL) {
            MY_LOGWO("working buffer create failed");
            return UNKNOWN_ERROR;
        }

        frameParams.pOutBuffer = pBufferHandle;

        if (mbRawDomain)
            // replace the input buffer for the next processor.
            pRequest->context.in_buffer = pBufferHandle;
        else
            pRequest->context.in_mdp_buffer = pBufferHandle;
    }

    FUNC_END;
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
PluginProcedure::
onPluginExecute(
        sp<Request> const,
        FrameParams const &frameParams)
{
    CAM_TRACE_NAME("P2:onPluginExecute");
    FUNC_START;

    IImageBuffer *pSrcBuffer = frameParams.pInBuffer->getBuffer();
    IImageBuffer *pDstBuffer = NULL;

    const PostProcessing::DecisionParams &decision = frameParams.decision;
    if (decision.processType == PostProcessing::eIN_OUT_PROCESSING)
    {
        MERROR ret = OK;
        if (frameParams.pOutBuffer.get()) {
            if (OK != (ret = frameParams.pOutBuffer->waitState(BufferHandle::STATE_READABLE))) {
                MY_LOGWO("dest buffer err = %d", ret);
                return ret;
            }
            pDstBuffer = frameParams.pOutBuffer->getBuffer();
        }
        else {
            MY_LOGWO("no dst");
            return BAD_VALUE;
        }
    }

    MBOOL success = MTRUE;
    {
        PostProcessing::ProcessParams processParams;
        processParams.pInBuffer = pSrcBuffer;
        processParams.pOutBuffer = pDstBuffer;
        processParams.pInAppMeta = frameParams.pInAppMeta->getMetadata();
        processParams.pInHalMeta = frameParams.pInHalMeta->getMetadata();
        processParams.bResized = frameParams.bResized;
        processParams.captureFrame = frameParams.captureFrame;
        processParams.uFrameNo = frameParams.uFrameNo;
        processParams.uProcessMode = decision.processMode;

        MY_LOGDO_IF(mbEnableLog, "start 3rd party algo + frameNo: %d, resized: %d",
                frameParams.uFrameNo, frameParams.bResized);

        if (!mpPostProcessing->process(processParams)) {
            MY_LOGEO("Post processing failed");
            success = MFALSE;
        }
        MY_LOGDO_IF(mbEnableLog, "start 3rd party algo -");
    }

    FUNC_END;
    return success ? OK : UNKNOWN_ERROR;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
PluginProcedure::
onPluginFinish(
        FrameParams const &frameParams,
        MBOOL const success)
{
    CAM_TRACE_NAME("P2:onPluginFinish");

    const PostProcessing::DecisionParams &decision = frameParams.decision;

    if (muDumpBuffer) {
        sp<IImageBuffer> pImgBuf = (decision.processType == PostProcessing::eIN_OUT_PROCESSING)?
                                   frameParams.pOutBuffer->getBuffer():
                                   frameParams.pInBuffer->getBuffer();

        if (!NSCam::Utils::makePath(P2_DEBUG_DUMP_PATH, 0660))
            MY_LOGWO("makePath[%s] fails", P2_DEBUG_DUMP_PATH);

        // ouput
        char filename[256] = {0};
        sprintf(filename, P2_DEBUG_DUMP_PATH "/pg-%04d-out-%dx%d.yuv",
                frameParams.uFrameNo,
                pImgBuf->getImgSize().w, pImgBuf->getImgSize().h);

        MY_LOGDO("[yuv][out] %d (%dx%d) fmt(0x%x)",
                frameParams.uFrameNo,
                pImgBuf->getImgSize().w, pImgBuf->getImgSize().h,
                pImgBuf->getImgFormat());

        pImgBuf->saveToFile(filename);
    }

    frameParams.pInBuffer->updateState(
            success ? BufferHandle::STATE_WRITE_OK : BufferHandle::STATE_WRITE_FAIL);

    if (decision.processType == PostProcessing::eIN_OUT_PROCESSING)
    {
        frameParams.pOutBuffer->updateState(
                success ? BufferHandle::STATE_WRITE_OK : BufferHandle::STATE_WRITE_FAIL);
    }
    return OK;
}
