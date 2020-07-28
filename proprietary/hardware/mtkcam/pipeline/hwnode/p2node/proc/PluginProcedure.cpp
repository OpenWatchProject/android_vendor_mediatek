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

#define PROCESSOR_NAME_P2   ("Plugin")
#define DEFINE_OPEN_ID      (muOpenId)

#include "PluginProcedure.h"

#define LOG_TAG "MtkCam/P2Node_PLUG"

/******************************************************************************
 *
 ******************************************************************************/
PluginProcedure::
PluginProcedure(CreateParams const &params)
        : muOpenId(params.uOpenId),
          mbEnableLog(params.bEnableLog),
          mbRawDomain(params.bRawDomain),
          mpPostProcessing(params.pPostProcessing)
{
    muDumpBuffer = ::property_get_int32("debug.camera.dump.plugin", 0);
    ASSERT_TRUE(mbRawDomain || mpPostProcessing->profile().outImageFormat == PostProcessing::eIMG_FMT_DEFAULT,
                "do not support to assign the format of input image in raw-doamin processing");
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

    PostProcessing::PreConditionParams preConditionParams;
    preConditionParams.pInAppMeta = pRequest->context.in_app_meta->getMetadata();
    preConditionParams.pInHalMeta = pRequest->context.in_hal_meta->getMetadata();
    preConditionParams.bResized = pRequest->context.resized;
    frameParams.pInAppMeta = pRequest->context.in_app_meta;
    frameParams.pInHalMeta = pRequest->context.in_hal_meta;
    if (!mpPostProcessing->doOrNot(preConditionParams)) {
        return NOT_ENOUGH_DATA;
    }

    MY_LOGDO("meet condition: frame[%d]", pRequest->getFrameNo());
    if (mbRawDomain && pRequest->context.in_buffer.size() != 0)
        frameParams.pInHandle = pRequest->context.in_buffer[0];
    else if (!mbRawDomain && pRequest->context.work_buffer != NULL) {
        frameParams.pInHandle = pRequest->context.work_buffer;
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
    if (frameParams.pInHandle.get()) {
        // force to get write-lock if in-place processing
        BufferHandle::BufferState_t waitState = (mpPostProcessing->profile().processingType == PostProcessing::eIN_PLACE_PROCESSING) ?
                                  BufferHandle::STATE_WRITABLE : BufferHandle::STATE_READABLE;
        if (OK != (ret = frameParams.pInHandle->waitState(waitState))) {
            MY_LOGWO("src buffer err = %d", ret);
            return ret;
        }
        pSrcBuffer = frameParams.pInHandle->getBuffer();
    }
    else {
        MY_LOGWO("no src");
        return BAD_VALUE;
    }

    if (mpPostProcessing->profile().processingType == PostProcessing::eIN_OUT_PROCESSING) {
        // create working buffer of 3rd party out with the specific format
        MUINT32 outImgForamt = (mpPostProcessing->profile().outImageFormat == PostProcessing::eIMG_FMT_DEFAULT) ?
                                pSrcBuffer->getImgFormat() :
                                mpPostProcessing->profile().outImageFormat;

        sp<BufferHandle> pBufferHandle = WorkingBufferHandle::create(
                "PG",
                outImgForamt,
                pSrcBuffer->getImgSize()
        );

        MY_LOGD("src format:0x%x dst format:0x%x", pSrcBuffer->getImgFormat(), mpPostProcessing->profile().outImageFormat);

        if (pBufferHandle == NULL) {
            MY_LOGWO("working buffer create failed");
            return UNKNOWN_ERROR;
        }
        frameParams.pOutHandle = pBufferHandle;

        if (mbRawDomain) {
            // replace the input buffer for the next processor.
            pRequest->context.in_buffer.erase(pRequest->context.in_buffer.begin());
            pRequest->context.in_buffer.push_back(pBufferHandle);
        }
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

    IImageBuffer *pSrcBuffer = frameParams.pInHandle->getBuffer();
    IImageBuffer *pDstBuffer = NULL;
    if (mpPostProcessing->profile().processingType == PostProcessing::eIN_OUT_PROCESSING)
    {
        MERROR ret = OK;
        if (frameParams.pOutHandle.get()) {
            if (OK != (ret = frameParams.pOutHandle->waitState(BufferHandle::STATE_READABLE))) {
                MY_LOGWO("dest buffer err = %d", ret);
                return ret;
            }
            pDstBuffer = frameParams.pOutHandle->getBuffer();
        }
        else {
            MY_LOGWO("no dst");
            return BAD_VALUE;
        }
    }

    MBOOL success = MTRUE;
    {
        PostProcessing::ProcessingParams processingParams;
        processingParams.pInBuffer = pSrcBuffer;
        processingParams.pOutBuffer = pDstBuffer;
        processingParams.pInAppMeta = frameParams.pInAppMeta->getMetadata();
        processingParams.pInHalMeta = frameParams.pInHalMeta->getMetadata();
        processingParams.iOpenId    = muOpenId;
        MY_LOGDO("start 3rd party algo +");
        if (!mpPostProcessing->process(processingParams)) {
            MY_LOGEO("Post processing failed");
            success = MFALSE;
        }
        MY_LOGDO("start 3rd party algo -");
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
        FrameParams const &params,
        MBOOL const success)
{
    CAM_TRACE_NAME("P2:onPluginFinish");
    if (muDumpBuffer) {
        sp<IImageBuffer> pImgBuf = params.pOutHandle->getBuffer();

        if (!NSCam::Utils::makePath(P2_DEBUG_DUMP_PATH, 0660))
            MY_LOGWO("makePath[%s] fails", P2_DEBUG_DUMP_PATH);

        // ouput
        char filename[256] = {0};
        sprintf(filename, P2_DEBUG_DUMP_PATH "/pg-%04d-out-%dx%d.%s",
                params.uFrameNo,
                pImgBuf->getImgSize().w, pImgBuf->getImgSize().h, mbRawDomain ? "raw" : "yuv");

        MY_LOGDO("[yuv][out] %d (%dx%d) fmt(0x%x)",
                params.uFrameNo,
                pImgBuf->getImgSize().w, pImgBuf->getImgSize().h,
                pImgBuf->getImgFormat());

        pImgBuf->saveToFile(filename);
        if(mpPostProcessing->profile().processingType == PostProcessing::eIN_OUT_PROCESSING){
            // input
            pImgBuf = params.pInHandle->getBuffer();
            char filename[256] = {0};
            sprintf(filename, P2_DEBUG_DUMP_PATH "/pg-%04d-in-%dx%d.%s",
                params.uFrameNo,
                pImgBuf->getImgSize().w, pImgBuf->getImgSize().h, mbRawDomain ? "raw" : "yuv");

            MY_LOGDO("[yuv][in] %d (%dx%d) fmt(0x%x)",
                params.uFrameNo,
                pImgBuf->getImgSize().w, pImgBuf->getImgSize().h,
                pImgBuf->getImgFormat());

            pImgBuf->saveToFile(filename);
        }

    }

    params.pInHandle->updateState(
            success ? BufferHandle::STATE_WRITE_OK : BufferHandle::STATE_WRITE_FAIL);

    if (mpPostProcessing->profile().processingType == PostProcessing::eIN_OUT_PROCESSING)
    {
        params.pOutHandle->updateState(
                success ? BufferHandle::STATE_WRITE_OK : BufferHandle::STATE_WRITE_FAIL);
    }
    return OK;
}
