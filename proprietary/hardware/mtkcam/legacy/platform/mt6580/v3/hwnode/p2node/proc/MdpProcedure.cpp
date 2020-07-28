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

#define PROCESSOR_NAME_MDP  "MDP"
#define DEFINE_OPEN_ID      (muOpenId)

#include "MdpProcedure.h"
#include <mtkcam/iopipe/SImager/IImageTransform.h>

using namespace NSIoPipe;

/******************************************************************************
 *
 ******************************************************************************/
sp<Processor>
MdpProcedure::
createProcessor(CreateParams &params) {
    return new ProcessorBase<MdpProcedure>(
            params.uOpenId, params, PROCESSOR_NAME_MDP);
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
MdpProcedure::
onMdpPullParams(
        sp<Request> pRequest,
        FrameParams &param_mdp)
{
    FUNC_START;
    if (!pRequest->context.in_mdp_buffer.get() && !pRequest->context.crop_info.get()) {
        return NOT_ENOUGH_DATA;
    }
    param_mdp.uFrameNo = pRequest->getFrameNo();
    param_mdp.in.mHandle = pRequest->context.in_mdp_buffer;
    param_mdp.pCropInfo = pRequest->context.crop_info;
    pRequest->context.in_mdp_buffer.clear();

    // input&output buffer
    vector<sp<BufferHandle>>::iterator iter = pRequest->context.out_buffers.begin();
    while (iter != pRequest->context.out_buffers.end()) {
        sp<BufferHandle> pOutBuffer = *iter;
        if (pOutBuffer.get() && pOutBuffer->getState() == BufferHandle::STATE_NOT_USED) {
            MdpProcedure::FrameOutput out;
            out.mHandle = pOutBuffer;
            out.mTransform = pOutBuffer->getTransform();
            param_mdp.vOut.push_back(out);
            (*iter).clear();
        }
        iter++;
    }
    FUNC_END;
    return (param_mdp.vOut.size() > 0) ? OK : UNKNOWN_ERROR;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
MdpProcedure::
onMdpExecute(
        sp<Request> const pRequest,
        FrameParams const &params)
{
    CAM_TRACE_NAME("P2:onMdpExecute");
    FUNC_START;

    MERROR ret = OK;

    IImageBuffer *pSrc = NULL;
    MSize srcSize;
    vector<IImageBuffer *> vDst;
    vector<MUINT32> vTransform;
    vector<MRect> vCrop;

    // input
    if (params.in.mHandle.get()) {
        if (OK != (ret = params.in.mHandle->waitState(BufferHandle::STATE_READABLE))) {
            MY_LOGWO("src buffer err = %d", ret);
            return ret;
        }
        pSrc = params.in.mHandle->getBuffer();
        srcSize = pSrc->getImgSize();
    }
    else {
        MY_LOGWO("no src");
        return BAD_VALUE;
    }

    // output
    for (size_t i = 0; i < params.vOut.size(); i++) {
        if (params.vOut[i].mHandle == NULL ||
            OK != (ret = params.vOut[i].mHandle->waitState(BufferHandle::STATE_WRITABLE))) {
            MY_LOGWO("dst buffer err = %d", ret);
            continue;
        }
        IImageBuffer *pDst = params.vOut[i].mHandle->getBuffer();

        if (pDst != NULL) {

            MINT32 const transform = params.vOut[i].mTransform;
            MSize dstSize = (transform & eTransform_ROT_90)
                            ? MSize(pDst->getImgSize().h, pDst->getImgSize().w)
                            : pDst->getImgSize();
            MRect crop;
            if (pRequest->context.in_mdp_crop) {
                MCropRect cropRect;
                Cropper::calcViewAngle(mbEnableLog, *params.pCropInfo, dstSize, cropRect);
                crop.p = cropRect.p_integral;
                crop.s = cropRect.s;
                vCrop.push_back(crop);
            } else {
                Cropper::calcBufferCrop(srcSize, dstSize, crop);
                vCrop.push_back(crop);
            }

            vDst.push_back(pDst);
            vTransform.push_back(params.vOut[i].mHandle->getTransform());

            MY_LOGDO_IF(P2_DEBUG_LOG, "request:%d[%d] Out(%d/%d) Rot(%d) Crop(%d,%d)(%dx%d) Size(%dx%d)",
                        pRequest->getFrameNo(), pRequest->getFrameSubNo(),
                        (i + 1), params.vOut.size(), transform,
                        crop.p.x, crop.p.y, crop.s.w, crop.s.h,
                        pDst->getImgSize().w, pDst->getImgSize().h);
        }
        else
            MY_LOGWO("mdp req:%d empty buffer", pRequest->getFrameNo());
    }

    if (pSrc == NULL || vDst.size() == 0) {
        MY_LOGEO("wrong mdp in/out: src %p, dst count %d", pSrc, vDst.size());
        return BAD_VALUE;
    }

    MBOOL success = MFALSE;
    {
#ifdef USING_MTK_LDVT
        success = MTRUE;
#else
        NSSImager::IImageTransform *pTrans = NSSImager::IImageTransform::createInstance();
        if (!pTrans) {
            MY_LOGEO("!pTrans");
            return UNKNOWN_ERROR;
        }
        success =
                pTrans->execute(
                        pSrc,
                        vDst[0],
                        (vDst.size() > 1) ? vDst[1] : NULL,
                        vCrop[0],
                        (vCrop.size() > 1) ? vCrop[1] : NULL,
                        vTransform[0],
                        (vTransform.size() > 1) ? vTransform[1] : NULL,
                        0xFFFFFFFF);

        pTrans->destroyInstance();
        pTrans = NULL;
#endif
    }
    FUNC_END;
    return success ? OK : UNKNOWN_ERROR;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
MdpProcedure::
onMdpFinish(
        FrameParams const &params,
        MBOOL const success)
{
    CAM_TRACE_NAME("P2:onMdpFinish");
    //params.in.mHandle->updateState(BufferHandle::Buffer_ReadDone);
    if (muDumpBuffer) {
        MY_LOGDO("[yuv] frame(%d) size(%d)", params.uFrameNo, params.vOut.size());

        sp<IImageBuffer> pImgBuf = NULL;
        if (!NSCam::Utils::makePath(P2_DEBUG_DUMP_PATH, 0660))
            MY_LOGWO("makePath[%s] fails", P2_DEBUG_DUMP_PATH);

        // ouput
        char filename[256] = {0};
        for (size_t i = 0; i < params.vOut.size(); i++) {
            pImgBuf = params.vOut[i].mHandle->getBuffer();

            sprintf(filename, P2_DEBUG_DUMP_PATH "/mdp-%04d-out-%dx%d.yuv",
                    params.uFrameNo,
                    pImgBuf->getImgSize().w, pImgBuf->getImgSize().h);

            MY_LOGDO("[yuv][out] %d (%dx%d) fmt(0x%x)",
                    params.uFrameNo,
                    pImgBuf->getImgSize().w, pImgBuf->getImgSize().h,
                    pImgBuf->getImgFormat());

            pImgBuf->saveToFile(filename);
        }
    }

    for (size_t i = 0; i < params.vOut.size(); i++)
        if (params.vOut[i].mHandle.get())
            params.vOut[i].mHandle->updateState(
                    success ? BufferHandle::STATE_WRITE_OK : BufferHandle::STATE_WRITE_FAIL
            );
    return OK;
}
