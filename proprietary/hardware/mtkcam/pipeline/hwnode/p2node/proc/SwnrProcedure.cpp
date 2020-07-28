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

#define PROCESSOR_NAME_SWNR ("SWNR")
#define DEFINE_OPEN_ID      (muOpenId)

#include "SwnrProcedure.h"


/******************************************************************************
 *
 ******************************************************************************/
SwnrProcedure::
SwnrProcedure(CreateParams const &params)
        : mbEnableLog(params.bEnableLog),
          muOpenId(params.uOpenId),
          mpSwnr(NULL)
{
    muDumpBuffer = ::property_get_int32("debug.camera.dump.swnr", 0);
}


/******************************************************************************
 *
 ******************************************************************************/
SwnrProcedure::
~SwnrProcedure() {
    if (mpSwnr != NULL)
        delete mpSwnr;
}


/******************************************************************************
 *
 ******************************************************************************/
sp<Processor>
SwnrProcedure::
createProcessor(CreateParams &params) {
    return new ProcessorBase<SwnrProcedure>(
            params.uOpenId, params, PROCESSOR_NAME_SWNR);
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
SwnrProcedure::
onSwnrPullParams(
        sp<Request> pRequest,
        FrameParams &frameParams)
{
    if (pRequest->context.nr_type != Request::NR_TYPE_SWNR) {
        return NOT_ENOUGH_DATA;
    }

    frameParams.uFrameNo = pRequest->getFrameNo();

    if (pRequest->context.work_buffer.get()) {
        frameParams.pInBuffer = pRequest->context.work_buffer;
        frameParams.pOutHalMeta = pRequest->context.out_hal_meta;
        pRequest->context.in_mdp_buffer = pRequest->context.work_buffer;
        pRequest->context.work_buffer.clear();
        pRequest->context.out_hal_meta.clear();
        frameParams.nIso = pRequest->context.iso;
    }
    else {
        MY_LOGEO("no src");
        return UNKNOWN_ERROR;
    }

    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
SwnrProcedure::
onSwnrExecute(
        sp<Request> const,
        FrameParams const &frameParams)
{
    CAM_TRACE_NAME("P2:onSwnrExecute");
    FUNC_START;

    MERROR ret = OK;
    IImageBuffer *pInBuffer = NULL;
    // input
    if (frameParams.pInBuffer.get()) {
        if (OK != (ret = frameParams.pInBuffer->waitState(BufferHandle::STATE_READABLE))) {
            MY_LOGWO("src buffer err = %d", ret);
            return ret;
        }
        pInBuffer = frameParams.pInBuffer->getBuffer();
    }
    else {
        MY_LOGWO("no src");
        return BAD_VALUE;
    }

    MBOOL success = MTRUE;
    {
        if (mpSwnr == NULL)
            mpSwnr = MAKE_SwNR(muOpenId);

        ISwNR::SWNRParam swnrParam;
        swnrParam.iso = frameParams.nIso;

        if (!mpSwnr->doSwNR(swnrParam, pInBuffer)) {
            MY_LOGEO("SWNR failed");
            success = MFALSE;
        } else {
            IMetadata *pOutHalMeta = frameParams.pOutHalMeta.get() ? frameParams.pOutHalMeta->getMetadata() : NULL;
            if (pOutHalMeta)
                mpSwnr->getDebugInfo(*pOutHalMeta);
            else
                MY_LOGW("no hal metadata for dumping debug info");
        }

    }

    FUNC_END;
    return success ? OK : UNKNOWN_ERROR;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
SwnrProcedure::
onSwnrFinish(
        FrameParams const &params,
        MBOOL const success)
{
    CAM_TRACE_NAME("P2:onSwnrFinish");

    if (muDumpBuffer) {
        sp<IImageBuffer> pImgBuf = params.pInBuffer->getBuffer();
        if (!NSCam::Utils::makePath(P2_DEBUG_DUMP_PATH, 0660))
            MY_LOGWO("makePath[%s] fails", P2_DEBUG_DUMP_PATH);

        // ouput
        char filename[256] = {0};
        sprintf(filename, P2_DEBUG_DUMP_PATH "/swnr-%04d-out-%dx%d.yuv",
                params.uFrameNo,
                pImgBuf->getImgSize().w, pImgBuf->getImgSize().h);

        MY_LOGDO("[yuv][out] %d (%dx%d) fmt(0x%x)",
                params.uFrameNo,
                pImgBuf->getImgSize().w, pImgBuf->getImgSize().h,
                pImgBuf->getImgFormat());

        pImgBuf->saveToFile(filename);
    }

    params.pInBuffer->updateState(
            success ? BufferHandle::STATE_WRITE_OK : BufferHandle::STATE_WRITE_FAIL);

    if (params.pOutHalMeta.get())
        params.pOutHalMeta->updateState(success ? MetaHandle::STATE_WRITE_OK : MetaHandle::STATE_WRITE_FAIL);

    return OK;
}