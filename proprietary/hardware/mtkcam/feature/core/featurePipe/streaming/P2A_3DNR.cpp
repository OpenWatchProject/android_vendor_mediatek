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

#include "P2A_3DNR.h"
#include "P2ANode.h"
#include "P2CamContext.h"

#include "DebugControl.h"
#define PIPE_CLASS_TAG "P2A_3DNR"
// #define PIPE_TRACE TRACE_P2A_3DNR
#define PIPE_TRACE 0
#include <featurePipe/core/include/PipeLog.h>

#include <mtkcam/drv/iopipe/PostProc/IHalPostProcPipe.h>

#include "hal/inc/camera_custom_3dnr.h"
#include <mtkcam/aaa/IIspMgr.h>
#include <mtkcam/drv/IHalSensor.h>
#include <mtkcam/feature/3dnr/3dnr_defs.h>

using namespace NSCam::NSIoPipe::NSPostProc;
using NSImageio::NSIspio::EPortIndex_IMG3O;
using NSImageio::NSIspio::EPortIndex_VIPI;
using NSCam::NSIoPipe::Input;
using NSCam::NSIoPipe::Output;

#include <mtkcam/drv/iopipe/PostProc/IHalDpePipe.h>
using namespace NSCam::NSIoPipe::NSDpe;

namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {

MBOOL P2ANode::do3dnrFlow(
    NSCam::NSIoPipe::QParams &enqueParams,
    const RequestPtr &request,
    MRect &dst_resizer_rect,
    eis_region &eisInfo,
    MINT32 iso,
    MINT32 isoThreshold,
    MUINT32 requestNo,
    const RSCData &rscData)
{
    TRACE_FUNC_ENTER();

    MBOOL ret = MFALSE;
    MBOOL isSupportV2Flow = MTRUE;//default: support V2 flow

    sp<P2CamContext> p2CamContext = getP2CamContext(request->getMasterID());
    IImageBuffer *pIMGBufferVIPI = NULL;
    ImgBuffer prevFullImg = p2CamContext->getPrevFullImg();
    if (prevFullImg != NULL)
        pIMGBufferVIPI = prevFullImg->getImageBufferPtr();

    hal3dnrBase *p3dnr = p2CamContext->get3dnr();
    if (NULL == p3dnr)
    {
        MY_LOGW("no hal3dnr!");
        return ret;
    }

    if (mPipeUsage.is3DNRModeMaskEnable(
                NSCam::NR3D::E3DNR_MODE_MASK_HAL_FORCE_SUPPORT))
    {
        isSupportV2Flow = ::property_get_int32("vendor.debug.3dnr.hal.v2", isSupportV2Flow);
    }

    if (isSupportV2Flow && NR3DCustom::isEnabled3DNR20() == false)
    {
        void *pIspPhyReg = (void*)enqueParams.mvFrameParams[0].mTuningData;
        MBOOL useCMV = request->isP2ACRZMode();

        NR3DHALParam nr3dHalParam;

        // 3a related
        nr3dHalParam.pTuningData = enqueParams.mvFrameParams[0].mTuningData;
        nr3dHalParam.p3A = mp3A;

        // generic
        nr3dHalParam.frameNo = requestNo;
        nr3dHalParam.iso = iso;
        nr3dHalParam.isoThreshold = isoThreshold;

        // imgi related
        nr3dHalParam.isCRZUsed = request->isP2ACRZMode();
        nr3dHalParam.isIMGO = request->getVar<MBOOL>(VAR_IMGO_2IMGI_ENABLE, MFALSE);
        nr3dHalParam.isBinning = MFALSE; // TODO: how to pass BinningInfo??

        // lmv related info
        nr3dHalParam.GMVInfo.gmvX = eisInfo.gmvX;
        nr3dHalParam.GMVInfo.gmvY = eisInfo.gmvY;
        nr3dHalParam.GMVInfo.x_int = eisInfo.x_int;
        nr3dHalParam.GMVInfo.y_int = eisInfo.y_int;
        nr3dHalParam.GMVInfo.confX = eisInfo.confX;
        nr3dHalParam.GMVInfo.confY= eisInfo.confY;

        //RSC related
        ImgBuffer pV = rscData.mData.mMV;
        if (pV.get() && pV->getImageBufferPtr())
            nr3dHalParam.RSCInfo.pMV = pV->getImageBufferPtr()->getBufVA(0);
        pV = rscData.mData.mBV;
        if (pV.get() && pV->getImageBufferPtr())
            nr3dHalParam.RSCInfo.pBV = pV->getImageBufferPtr()->getBufVA(0);
        nr3dHalParam.RSCInfo.rrzoSize = request->mFullImgSize;
        nr3dHalParam.RSCInfo.rssoSize = rscData.mData.mRssoSize;
        nr3dHalParam.RSCInfo.staGMV = rscData.mData.mRscSta.value;
        nr3dHalParam.RSCInfo.isValid = rscData.mData.mIsValid;

        // vipi related
        nr3dHalParam.pIMGBufferVIPI = pIMGBufferVIPI;

        // output related, ex: img3o
        nr3dHalParam.dst_resizer_rect = dst_resizer_rect;

        // gyro
        nr3dHalParam.gyroData = request->getVar<NR3D::GyroData>(VAR_3DNR_GYRO, NR3D::GyroData());

        ret = p3dnr->do3dnrFlow_v2(nr3dHalParam);
    }
    else
    {
        void *pIspPhyReg = (void*)enqueParams.mvFrameParams[0].mTuningData;
        MBOOL useCMV = request->isP2ACRZMode();
        NR3DGMVInfo GMVInfo;
        GMVInfo.gmvX = eisInfo.gmvX;
        GMVInfo.gmvY = eisInfo.gmvY;
        GMVInfo.x_int = eisInfo.x_int;
        GMVInfo.y_int = eisInfo.y_int;
        GMVInfo.confX = eisInfo.confX;
        GMVInfo.confY = eisInfo.confY;

        ret = p3dnr->do3dnrFlow(pIspPhyReg, useCMV, dst_resizer_rect, GMVInfo,
                pIMGBufferVIPI, iso, requestNo, mp3A);
    }

    TRACE_FUNC_EXIT();

    return ret;
}

MVOID P2ANode::dump_Qparam(QParams& rParams, const char *pSep)
{
    if (mPipeUsage.is3DNRModeMaskEnable(
        NSCam::NR3D::E3DNR_MODE_MASK_HAL_FORCE_SUPPORT) == 0)
        return;

    // start dump process
    MINT32 enableOption = ::property_get_int32("vendor.camera.3dnr.dump.qparam", 0);
    if (enableOption)
    {
        return;
    }
    if (m3dnrLogLevel == 0)
    {
        return;
    }

    TRACE_FUNC_ENTER();
    MUINT32 i =0;

    MY_LOGD_IF(m3dnrLogLevel >= 2, "%s_: rParams.mDequeSuccess: %d", pSep, rParams.mDequeSuccess);
    if (pSep != NULL && pSep[0] == 'd' && pSep[1] == 'd' && rParams.mDequeSuccess == 0)
    {
        MY_LOGD_IF(m3dnrLogLevel >= 2, "%s_!!! QPARAM DEQUE  FAIL !!!", pSep);
        return;
    }

    MY_LOGD_IF(m3dnrLogLevel >= 2, "%s_: rParams.mpfnCallback: %p", pSep, rParams.mpfnCallback);
    MY_LOGD_IF(m3dnrLogLevel >= 2, "%s_: rParams.mpCookie: %p", pSep, rParams.mpCookie);
#if 0
    MY_LOGD_IF(m3dnrLogLevel >= 2, "%s_: rParams.mpPrivaData: %p", pSep, rParams.mpPrivaData);
    MY_LOGD_IF(m3dnrLogLevel >= 2, "%s_: rParams.mvPrivaData.size(): %d", pSep, rParams.mvPrivaData.size());
#endif

    MY_LOGD_IF(m3dnrLogLevel >= 2, "%s_: rParams.mvFrameParams[0].mTuningData: %p", pSep, rParams.mvFrameParams[0].mTuningData);

    // mvIn
    MY_LOGD_IF(m3dnrLogLevel >= 2, "%s_: rParams.mvFrameParams[0].mvIn.size(): %zu", pSep, rParams.mvFrameParams[0].mvIn.size());
    for (i = 0; i < rParams.mvFrameParams[0].mvIn.size(); ++i)
    {
        MY_LOGD_IF(m3dnrLogLevel >= 2, "\t%s_: --- rParams.mvIn[#%d]: start --- ", pSep, i);
        Input tmp = rParams.mvFrameParams[0].mvIn[i];

        MY_LOGD_IF(m3dnrLogLevel >= 2, "\t%s_: mvIn[%d].portID.index: %d", pSep, i, tmp.mPortID.index);
        MY_LOGD_IF(m3dnrLogLevel >= 2, "\t%s_: mvIn[%d].portID.type: %d", pSep, i, tmp.mPortID.type);
        MY_LOGD_IF(m3dnrLogLevel >= 2, "\t%s_: mvIn[%d].portID.inout: %d", pSep, i, tmp.mPortID.inout);
        MY_LOGD_IF(m3dnrLogLevel >= 2, "\t%s_: mvIn[%d].portID.group: %d", pSep, i, tmp.mPortID.group);
        MY_LOGD_IF(m3dnrLogLevel >= 2, "\t%s_: mvIn[%d].portID.capbility: %d", pSep, i, tmp.mPortID.capbility);
        MY_LOGD_IF(m3dnrLogLevel >= 2, "\t%s_: mvIn[%d].portID.reserved: %d", pSep, i, tmp.mPortID.reserved);

        MY_LOGD_IF(m3dnrLogLevel >= 2, "\t%s_: Input.mBuffer: %p", pSep, tmp.mBuffer);

        if (tmp.mBuffer != NULL)
        {
            MY_LOGD_IF(m3dnrLogLevel >= 2, "\t%s_: mvIn[%d].mBuffer.getImgFormat(): %d", pSep, i, tmp.mBuffer->getImgFormat());
            MY_LOGD_IF(m3dnrLogLevel >= 2, "\t%s_: mvIn[%d].mBuffer.getImgSize(): w=%d, h=%d", pSep, i, tmp.mBuffer->getImgSize().w, tmp.mBuffer->getImgSize().h);
            MY_LOGD_IF(m3dnrLogLevel >= 2, "\t%s_: mvIn[%d].mBuffer.getImgBitsPerPixel(): %zu", pSep, i, tmp.mBuffer->getImgBitsPerPixel());

            MY_LOGD_IF(m3dnrLogLevel >= 2, "\t%s_: mvIn[%d].mBuffer.getPlaneCount(): %zu", pSep, i, tmp.mBuffer->getPlaneCount());
            for (size_t k =0; k < tmp.mBuffer->getPlaneCount(); ++k)
            {
                MY_LOGD_IF(m3dnrLogLevel >= 2, "\t%s_: mvIn[%d].mBuffer.getPlaneBitsPerPixel(%zu): %zu", pSep, i, k, tmp.mBuffer->getPlaneBitsPerPixel(k));
            }
            MY_LOGD_IF(m3dnrLogLevel >= 2, "\t%s_: mvIn[%d].mBuffer.getBitstreamSize(): %zu", pSep, i, tmp.mBuffer->getBitstreamSize());
            //            virtual IImageBufferHeap*       getImageBufferHeap()                const   = 0;
            //            MY_LOGD_IF(m3dnrLogLevel >= 2, "\t%s_: mvIn[%d].mBuffer.getExtOffsetInBytes(): %d", pSep, i, tmp.mBuffer->getExtOffsetInBytes());
            //!!NOTES: if VA/PA is going to be used, remember to use lockBuf()/unlockBuf()
            //            (tmp.mBuffer)->lockBuf(PIPE_CLASS_TAG, eBUFFER_USAGE_SW_READ_OFTEN);
            //            (tmp.mBuffer)->unlockBuf(PIPE_CLASS_TAG);
        }

        MY_LOGD_IF(m3dnrLogLevel >= 2, "%s_: --- rParams.mvIn[#%d]: end --- ", pSep, i);
    }

    // mvOut
    MY_LOGD_IF(m3dnrLogLevel >= 2, "%s_: rParams.mvFrameParams[0].mvOut.size(): %zu", pSep, rParams.mvFrameParams[0].mvOut.size());
    for (i = 0; i < rParams.mvFrameParams[0].mvOut.size(); ++i)
    {
        MY_LOGD_IF(m3dnrLogLevel >= 2, "\t%s_: --- rParams.mvOut[#%d]: start --- ", pSep, i);
        Output tmp = rParams.mvFrameParams[0].mvOut[i];

        MY_LOGD_IF(m3dnrLogLevel >= 2, "\t%s_: mvOut[%d].portID.index: %d", pSep, i, tmp.mPortID.index);
        MY_LOGD_IF(m3dnrLogLevel >= 2, "\t%s_: mvOut[%d].portID.type: %d", pSep, i, tmp.mPortID.type);
        MY_LOGD_IF(m3dnrLogLevel >= 2, "\t%s_: mvOut[%d].portID.inout: %d", pSep, i, tmp.mPortID.inout);
        MY_LOGD_IF(m3dnrLogLevel >= 2, "\t%s_: mvOut[%d].portID.group: %d", pSep, i, tmp.mPortID.group);
        MY_LOGD_IF(m3dnrLogLevel >= 2, "\t%s_: mvOut[%d].portID.capbility: %d", pSep, i, tmp.mPortID.capbility);
        MY_LOGD_IF(m3dnrLogLevel >= 2, "\t%s_: mvOut[%d].portID.reserved: %d", pSep, i, tmp.mPortID.reserved);

        MY_LOGD_IF(m3dnrLogLevel >= 2, "\t%s_: Input.mBuffer: %p", pSep, tmp.mBuffer);

        if (tmp.mBuffer != NULL)
        {
            MY_LOGD_IF(m3dnrLogLevel >= 2, "\t%s_: mvOut[%d].mBuffer.getImgFormat(): %d", pSep, i, tmp.mBuffer->getImgFormat());
            MY_LOGD_IF(m3dnrLogLevel >= 2, "\t%s_: mvOut[%d].mBuffer.getImgSize(): w=%d, h=%d", pSep, i, tmp.mBuffer->getImgSize().w, tmp.mBuffer->getImgSize().h);
            MY_LOGD_IF(m3dnrLogLevel >= 2, "\t%s_: mvOut[%d].mBuffer.getImgBitsPerPixel(): %zu", pSep, i, tmp.mBuffer->getImgBitsPerPixel());

            MY_LOGD_IF(m3dnrLogLevel >= 2, "\t%s_: mvOut[%d].mBuffer.getPlaneCount(): %zu", pSep, i, tmp.mBuffer->getPlaneCount());
            for (size_t k =0; k < tmp.mBuffer->getPlaneCount(); ++k)
            {
                MY_LOGD_IF(m3dnrLogLevel >= 2, "\t%s_: mvOut[%d].mBuffer.getPlaneBitsPerPixel(%zu): %zu", pSep, i, k, tmp.mBuffer->getPlaneBitsPerPixel(k));
            }
            MY_LOGD_IF(m3dnrLogLevel >= 2, "\t%s_: mvOut[%d].mBuffer.getBitstreamSize(): %zu", pSep, i, tmp.mBuffer->getBitstreamSize());
            //            virtual IImageBufferHeap*       getImageBufferHeap()                const   = 0;
            //            MY_LOGD_IF(m3dnrLogLevel >= 2, "\t%s_: mvOut[%d].mBuffer.getExtOffsetInBytes(): %d", pSep, i, tmp.mBuffer->getExtOffsetInBytes());
            //!!NOTES: if VA/PA is going to be used, remember to use lockBuf()/unlockBuf()
            //            (tmp.mBuffer)->lockBuf(PIPE_CLASS_TAG, eBUFFER_USAGE_SW_READ_OFTEN);
            //            (tmp.mBuffer)->unlockBuf(PIPE_CLASS_TAG);
        }

        MY_LOGD_IF(m3dnrLogLevel >= 2, "%s_: --- rParams.mvOut[#%d]: end --- ", pSep, i);
    }

    TRACE_FUNC_EXIT();
}

MVOID P2ANode::dump_vOutImageBuffer(const QParams & params)
{
    // === default values initialized ===
    static int num_img3o_frame_to_dump = 0;
    static int dumped_frame_count = 0;
    static int is_dump_complete = 1;
    static int dump_round_count = 1;
    // ==================================

    // start dump process
    char EnableOption[PROPERTY_VALUE_MAX] = {'\0'};
    if (num_img3o_frame_to_dump == 0  || dumped_frame_count == num_img3o_frame_to_dump)
    {

        num_img3o_frame_to_dump = 0;
        if (mPipeUsage.is3DNRModeMaskEnable(
            NSCam::NR3D::E3DNR_MODE_MASK_HAL_FORCE_SUPPORT) != 0)
        {
            num_img3o_frame_to_dump = ::property_get_int32("vendor.camera.3dnr.dump.img3o", 0);
        }
//        MY_LOGW("(dumped_frame_count=%d, num_img3o_frame_to_dump =%d), no need to dump vOut frames",
  //          dumped_frame_count, num_img3o_frame_to_dump);
        return;
    }

    //debug: start
    char vOut0_frame_str[64];
    char vOutIMG3O_frame_str[64];

    // start from scratch
    if (is_dump_complete)
    {
        is_dump_complete = 0;
        dumped_frame_count = 0;
    }

    if (dumped_frame_count  < num_img3o_frame_to_dump )
    {
        int is_img3o_dumped = 0;
        MY_LOGD_IF(m3dnrLogLevel >= 2, "mvOut size = %zu", params.mvFrameParams[0].mvOut.size());

        for (size_t i =0; i < params.mvFrameParams[0].mvOut.size(); ++i)
        {

//            MY_LOGD_IF(m3dnrLogLevel >= 2, "mkdbg: mvOut[%d].mPortID: %d", i, params.mvOut[i].mPortID);
            if (i == 0)
            {
                if (params.mDequeSuccess != 0)
                {
                    sprintf(vOut0_frame_str, "/sdcard/vOut0_frame-r%.2d_%.3d_%dx%d_OK.yuv",
                        dump_round_count, dumped_frame_count,
                        params.mvFrameParams[0].mvOut[0].mBuffer->getImgSize().w, params.mvFrameParams[0].mvOut[0].mBuffer->getImgSize().h
                        );
                }
                else
                {
                    sprintf(vOut0_frame_str, "/sdcard/vOut0_frame-r%.2d_%.3d_%dx%d_NG.yuv",
                        dump_round_count, dumped_frame_count,
                        params.mvFrameParams[0].mvOut[0].mBuffer->getImgSize().w, params.mvFrameParams[0].mvOut[0].mBuffer->getImgSize().h
                        );
                }
                params.mvFrameParams[0].mvOut[0].mBuffer->saveToFile(vOut0_frame_str);
                MY_LOGD_IF(m3dnrLogLevel >= 2, "params.mvOut[0] saved: %p", params.mvFrameParams[0].mvOut[0].mBuffer);
            }

            if (params.mvFrameParams[0].mvOut[i].mPortID == PortID(EPortType_Memory, EPortIndex_IMG3O, PORTID_OUT) )
            {
                if (params.mDequeSuccess != 0)
                {
                    sprintf(vOutIMG3O_frame_str, "/sdcard/vOutIMG3O_frame-r%.2d_%.3d_%dx%d_OK.yuv",
                        dump_round_count, dumped_frame_count,
                        params.mvFrameParams[0].mvOut[i].mBuffer->getImgSize().w, params.mvFrameParams[0].mvOut[i].mBuffer->getImgSize().h
                        );
                }
                else
                {
                    sprintf(vOutIMG3O_frame_str, "/sdcard/vOutIMG3O_frame-r%.2d_%.3d_%dx%d_NG.yuv",
                        dump_round_count, dumped_frame_count,
                        params.mvFrameParams[0].mvOut[i].mBuffer->getImgSize().w, params.mvFrameParams[0].mvOut[i].mBuffer->getImgSize().h
                        );
                }
                params.mvFrameParams[0].mvOut[i].mBuffer->saveToFile(vOutIMG3O_frame_str);
                MY_LOGD_IF(m3dnrLogLevel >= 2, "params.mvOut[%zu] EPortIndex_IMG3O saved: %p", i, params.mvFrameParams[0].mvOut[i].mBuffer);
                is_img3o_dumped = 1;
            }
        }

        if (is_img3o_dumped == 0)
        {
            MY_LOGD_IF(m3dnrLogLevel >= 2, "mkdbg: !!err: no IMG3O buffer dumped");
            MY_LOGD_IF(m3dnrLogLevel >= 2, "mkdbg: !!err: no IMG3O buffer dumped");
        }
        ++dumped_frame_count;

        if (dumped_frame_count  >= num_img3o_frame_to_dump)
        {
            // when the dump is complete...
            is_dump_complete = 1;
            num_img3o_frame_to_dump = 0;
            MY_LOGD_IF(m3dnrLogLevel >= 2, "dump round %.2d finished ... (dumped_frame_count=%d, num_img3o_frame_to_dump =%d)",
                dump_round_count++, dumped_frame_count, num_img3o_frame_to_dump);
        }
    }
}


MVOID P2ANode::dump_imgiImageBuffer(const QParams &)
{
    // Not implemented
}


} // namespace NSFeaturePipe
} // namespace NSCamFeature
} // namespace NSCam
