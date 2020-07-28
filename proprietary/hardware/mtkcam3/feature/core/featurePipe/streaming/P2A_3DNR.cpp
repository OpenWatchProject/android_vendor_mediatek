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

#include "P2ANode.h"
#include "P2CamContext.h"

#include "DebugControl.h"
#define PIPE_CLASS_TAG "P2A_3DNR"
#define PIPE_TRACE TRACE_P2A_3DNR
#include <featurePipe/core/include/PipeLog.h>

#include <mtkcam/drv/iopipe/PostProc/IHalPostProcPipe.h>

#include "hal/inc/camera_custom_3dnr.h"
#include <mtkcam/aaa/IIspMgr.h>
#include <mtkcam/drv/IHalSensor.h>
#include <mtkcam3/feature/3dnr/3dnr_defs.h>

using namespace NSCam::NSIoPipe::NSPostProc;
using NSImageio::NSIspio::EPortIndex_IMG3O;
using NSImageio::NSIspio::EPortIndex_VIPI;
using NSCam::NSIoPipe::Input;
using NSCam::NSIoPipe::Output;
using namespace NSCam::NR3D;

#include <mtkcam/drv/iopipe/PostProc/IHalDpePipe.h>

CAM_ULOG_DECLARE_MODULE_ID(MOD_STREAMING_P2A);
using namespace NSCam::NSIoPipe::NSDpe;

namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {


MBOOL P2ANode::prepare3DNRMvInfo(Feature::P2Util::DIPParams &/*params*/, const RequestPtr &request, MUINT32 sensorID, const RSCData &rscData)
{
    TRACE_FUNC_ENTER();

#if 0
    VarMap<SFP_VAR> &varMap = request->getSensorVarMap(sensorID);
    MBOOL enable3dnrDsdnOnFrame = varMap.get<MBOOL>(SFP_VAR::NR3D_P2NR_CAN_ENABLE_ON_FRAME, MFALSE);
    if (enable3dnrDsdnOnFrame != MTRUE)
    {
        TRACE_FUNC_EXIT();
        return MFALSE;
    }
#endif

    sp<P2CamContext> p2CamContext = getP2CamContext(sensorID);
    Hal3dnrBase *p3dnr = p2CamContext->get3dnr();
    if (NULL == p3dnr)
    {
        MY_LOGW("!!warn: 3dnr: no hal3dnr!");
        TRACE_FUNC_EXIT();
        return MFALSE;
    }

    NR3DHALParam nr3dHalParam;

    // uniqueKey, requestNo, frameNo
    TuningUtils::FILE_DUMP_NAMING_HINT hint = request->mP2Pack.getSensorData(request->mMasterID).mNDDHint;
    nr3dHalParam.uniqueKey = hint.UniqueKey;
    nr3dHalParam.requestNo = hint.RequestNo;
    nr3dHalParam.frameNo = hint.FrameNo;

    nr3dHalParam.featMask = NR3D_FEAT_MASK_BASIC;
    // imgi related
    nr3dHalParam.isCRZUsed = request->isP2ACRZMode();
    nr3dHalParam.isIMGO = request->getVar<MBOOL>(SFP_VAR::IMGO_2IMGI_ENABLE, MFALSE);
    nr3dHalParam.isBinning = MFALSE; // useless

    // lmv related info
    NR3D::NR3DMVInfo defaultMvInfo;
    nr3dHalParam.gmvInfo = request->getVar<NR3D::NR3DMVInfo>(SFP_VAR::NR3D_MV_INFO, defaultMvInfo);

    //RSC related
    const SrcCropInfo srcCropInfo = request->getSrcCropInfo(sensorID);
    if (rscData.mData.mIsValid)
    {
        ImgBuffer pV = rscData.mData.mMV;
        if (pV.get() && pV->getImageBufferPtr())
            nr3dHalParam.rscInfo.pMV = pV->getImageBufferPtr()->getBufVA(0);
        pV = rscData.mData.mBV;
        if (pV.get() && pV->getImageBufferPtr())
            nr3dHalParam.rscInfo.pBV = pV->getImageBufferPtr()->getBufVA(0);
        nr3dHalParam.rscInfo.rrzoSize = srcCropInfo.mRRZOSize;
        nr3dHalParam.rscInfo.rssoSize = rscData.mData.mRssoSize;
        nr3dHalParam.rscInfo.staGMV = rscData.mData.mRscSta.value;
        nr3dHalParam.rscInfo.isValid = rscData.mData.mIsValid;
    }
    // output related, ex: img3o
    nr3dHalParam.dstRect = srcCropInfo.mSrcCrop;
    MY_LOGD_IF(m3dnrLogLevel, "3dnr: update3DNRMvInfo: sensorID(%d) f(%d) source crop(%d,%d,%d,%d)", sensorID, request->mRequestNo,
        srcCropInfo.mSrcCrop.p.x, srcCropInfo.mSrcCrop.p.y, srcCropInfo.mSrcCrop.s.w, srcCropInfo.mSrcCrop.s.h);
    // gyro
    nr3dHalParam.gyroData = request->getVar<NR3D::GyroData>(SFP_VAR::NR3D_GYRO, NR3D::GyroData());

    P2_CAM_TRACE_CALL(TRACE_DEFAULT);

    mNr3dHalResult.resetGMVInfo();
    MBOOL ret = p3dnr->update3DNRMvInfo(nr3dHalParam, mNr3dHalResult.gmvInfo, mNr3dHalResult.isGMVInfoUpdated);
    if (ret == MTRUE)
    {
        request->setVar<NR3D::NR3DMVInfo>(SFP_VAR::NR3D_MV_INFO, mNr3dHalResult.gmvInfo);
    }

    TRACE_FUNC_EXIT();
    return ret;
}

MBOOL P2ANode::prepare3DNR(Feature::P2Util::DIPParams& /* params */, const RequestPtr &request, MUINT32 sensorID, const RSCData &rscData)
{
    TRACE_FUNC_ENTER();

#if 0
    VarMap<SFP_VAR> &varMap = request->getSensorVarMap(sensorID);
    MBOOL enable3dnrOnFrame = varMap.get<MBOOL>(SFP_VAR::NR3D_P2A_CAN_ENABLE_ON_FRAME, MFALSE);
    if (enable3dnrOnFrame == MFALSE)
    {
        MY_LOGW("!!warn: 3dnr: no need due to enable3dnrOnFrame=0");
        return MFALSE;
    }
#endif

    P2_CAM_TRACE_CALL(TRACE_DEFAULT);
    sp<P2CamContext> p2CamContext = getP2CamContext(sensorID);
    Hal3dnrBase *p3dnr = p2CamContext->get3dnr();
    if (NULL == p3dnr)
    {
        MY_LOGW("!!warn: 3dnr: no hal3dnr!");
        TRACE_FUNC_EXIT();
        return MFALSE;
    }

    NR3DHALParam nr3dHalParam;

    // vipi related
    IImageBuffer *pVipiImgBuf = NULL;
    ImgBuffer prevFullImg = p2CamContext->getPrevFullImg();
    if (prevFullImg != NULL)
    {
        pVipiImgBuf = prevFullImg->getImageBufferPtr();
        nr3dHalParam.vipiInfo = NR3DHALParam::VipiInfo(MTRUE,
            pVipiImgBuf->getImgFormat(), pVipiImgBuf->getBufStridesInBytes(0),
            pVipiImgBuf->getImgSize()
            );
    }
    else
    {
        nr3dHalParam.vipiInfo =NR3DHALParam::VipiInfo();
    }
    // 3a related
    nr3dHalParam.pTuningData = NULL;
    nr3dHalParam.p3A = mp3A;

    nr3dHalParam.featMask = NR3D_FEAT_MASK_BASIC;

    // uniqueKey, requestNo, frameNo
    TuningUtils::FILE_DUMP_NAMING_HINT hint = request->mP2Pack.getSensorData(request->mMasterID).mNDDHint;
    nr3dHalParam.uniqueKey = hint.UniqueKey;
    nr3dHalParam.requestNo = hint.RequestNo;
    nr3dHalParam.frameNo = hint.FrameNo;

    // generic
    nr3dHalParam.needChkIso = MTRUE;
    nr3dHalParam.iso = request->getVar<MUINT32>(SFP_VAR::NR3D_ISO, 200);;
    nr3dHalParam.isoThreshold = request->getVar<MUINT32>(SFP_VAR::NR3D_P2A_ISO_THRESHOLD, 0);;
    // imgi related
    nr3dHalParam.isCRZUsed = request->isP2ACRZMode();
    nr3dHalParam.isIMGO = request->getVar<MBOOL>(SFP_VAR::IMGO_2IMGI_ENABLE, MFALSE);
    nr3dHalParam.isBinning = MFALSE; // useless

    // lmv related info
    NR3D::NR3DMVInfo defaultMvInfo;
    nr3dHalParam.gmvInfo = request->getVar<NR3D::NR3DMVInfo>(SFP_VAR::NR3D_MV_INFO, defaultMvInfo);
    //RSC related
    const SrcCropInfo srcCropInfo = request->getSrcCropInfo(sensorID);
    if (rscData.mData.mIsValid)
    {
        ImgBuffer pV = rscData.mData.mMV;
        if (pV.get() && pV->getImageBufferPtr())
            nr3dHalParam.rscInfo.pMV = pV->getImageBufferPtr()->getBufVA(0);
        pV = rscData.mData.mBV;
        if (pV.get() && pV->getImageBufferPtr())
            nr3dHalParam.rscInfo.pBV = pV->getImageBufferPtr()->getBufVA(0);
        nr3dHalParam.rscInfo.rrzoSize = srcCropInfo.mRRZOSize;
        nr3dHalParam.rscInfo.rssoSize = rscData.mData.mRssoSize;
        nr3dHalParam.rscInfo.staGMV = rscData.mData.mRscSta.value;
        nr3dHalParam.rscInfo.isValid = rscData.mData.mIsValid;
    }

    // output related, ex: img3o
    nr3dHalParam.dstRect = srcCropInfo.mSrcCrop;
    MY_LOGD_IF(m3dnrLogLevel, "do3dnrFlow_v3: sensorID(%d) f(%d) source crop(%d,%d,%d,%d)", sensorID, request->mRequestNo,
        srcCropInfo.mSrcCrop.p.x, srcCropInfo.mSrcCrop.p.y, srcCropInfo.mSrcCrop.s.w, srcCropInfo.mSrcCrop.s.h);
    // gyro
    nr3dHalParam.gyroData = request->getVar<NR3D::GyroData>(SFP_VAR::NR3D_GYRO, NR3D::GyroData());

    mNr3dHalResult.resetGMVInfo();
    if (p3dnr->update3DNRMvInfo(nr3dHalParam, mNr3dHalResult.gmvInfo, mNr3dHalResult.isGMVInfoUpdated) == MTRUE)
    {
        if (mNr3dHalResult.isGMVInfoUpdated)
        {
            if (request->needDSDN20()) // only update MVInfo when necessary
            {
                request->setVar<NR3D::NR3DMVInfo>(SFP_VAR::NR3D_MV_INFO, mNr3dHalResult.gmvInfo);
            }
        }
        else
        {
            MY_LOGW("3dnr: GMV is not updated");
        }
    }
    else
    {
        MY_LOGW("3dnr->update3DNRMVInfo return false, which may affect 3dnr quality");
    }

    mNr3dHalResult.resetNr3dHwParam();
    MBOOL ret = p3dnr->do3dnrFlow(nr3dHalParam, mNr3dHalResult);
    if (ret != MTRUE)
    {
        MY_LOGW("3dnr->do3dnrFlow failed");
    }

    TRACE_FUNC_EXIT();
    return ret;
}

MVOID P2ANode::dump_DIPparam(Feature::P2Util::DIPParams& rParams, const char *pSep)
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

    MY_LOGD_IF(m3dnrLogLevel >= 2, "%s_: rParams.mpfnDIPCallback: %p", pSep, rParams.mpfnDIPCallback);
    MY_LOGD_IF(m3dnrLogLevel >= 2, "%s_: rParams.mpCookie: %p", pSep, rParams.mpCookie);
#if 0
    MY_LOGD_IF(m3dnrLogLevel >= 2, "%s_: rParams.mpPrivaData: %p", pSep, rParams.mpPrivaData);
    MY_LOGD_IF(m3dnrLogLevel >= 2, "%s_: rParams.mvPrivaData.size(): %d", pSep, rParams.mvPrivaData.size());
#endif

    MY_LOGD_IF(m3dnrLogLevel >= 2, "%s_: rParams.mvDIPFrameParams[0].mTuningData: %p", pSep, rParams.mvDIPFrameParams[0].mTuningData);

    // mvIn
    MY_LOGD_IF(m3dnrLogLevel >= 2, "%s_: rParams.mvDIPFrameParams[0].mvIn.size(): %zu", pSep, rParams.mvDIPFrameParams[0].mvIn.size());
    for (i = 0; i < rParams.mvDIPFrameParams[0].mvIn.size(); ++i)
    {
        MY_LOGD_IF(m3dnrLogLevel >= 2, "\t%s_: --- rParams.mvIn[#%d]: start --- ", pSep, i);
        Input tmp = rParams.mvDIPFrameParams[0].mvIn[i];

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
    MY_LOGD_IF(m3dnrLogLevel >= 2, "%s_: rParams.mvDIPFrameParams[0].mvOut.size(): %zu", pSep, rParams.mvDIPFrameParams[0].mvOut.size());
    for (i = 0; i < rParams.mvDIPFrameParams[0].mvOut.size(); ++i)
    {
        MY_LOGD_IF(m3dnrLogLevel >= 2, "\t%s_: --- rParams.mvOut[#%d]: start --- ", pSep, i);
        Output tmp = rParams.mvDIPFrameParams[0].mvOut[i];

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

MVOID P2ANode::dump_vOutImageBuffer(const Feature::P2Util::DIPParams & params)
{
    // === default values initialized ===
    static int num_img3o_frame_to_dump = 0;
    static int dumped_frame_count = 0;
    static int is_dump_complete = 1;
    static int dump_round_count = 1;
    // ==================================

    // start dump process
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
        MY_LOGD_IF(m3dnrLogLevel >= 2, "mvOut size = %zu", params.mvDIPFrameParams[0].mvOut.size());

        for (size_t i =0; i < params.mvDIPFrameParams[0].mvOut.size(); ++i)
        {

//            MY_LOGD_IF(m3dnrLogLevel >= 2, "mkdbg: mvOut[%d].mPortID: %d", i, params.mvOut[i].mPortID);
            if (i == 0)
            {
                if (params.mDequeSuccess != 0)
                {
                    sprintf(vOut0_frame_str, "/sdcard/vOut0_frame-r%.2d_%.3d_%dx%d_OK.yuv",
                        dump_round_count, dumped_frame_count,
                        params.mvDIPFrameParams[0].mvOut[0].mBuffer->getImgSize().w, params.mvDIPFrameParams[0].mvOut[0].mBuffer->getImgSize().h
                        );
                }
                else
                {
                    sprintf(vOut0_frame_str, "/sdcard/vOut0_frame-r%.2d_%.3d_%dx%d_NG.yuv",
                        dump_round_count, dumped_frame_count,
                        params.mvDIPFrameParams[0].mvOut[0].mBuffer->getImgSize().w, params.mvDIPFrameParams[0].mvOut[0].mBuffer->getImgSize().h
                        );
                }
                params.mvDIPFrameParams[0].mvOut[0].mBuffer->saveToFile(vOut0_frame_str);
                MY_LOGD_IF(m3dnrLogLevel >= 2, "params.mvOut[0] saved: %p", params.mvDIPFrameParams[0].mvOut[0].mBuffer);
            }

            if (params.mvDIPFrameParams[0].mvOut[i].mPortID == PortID(EPortType_Memory, EPortIndex_IMG3O, PORTID_OUT) )
            {
                if (params.mDequeSuccess != 0)
                {
                    sprintf(vOutIMG3O_frame_str, "/sdcard/vOutIMG3O_frame-r%.2d_%.3d_%dx%d_OK.yuv",
                        dump_round_count, dumped_frame_count,
                        params.mvDIPFrameParams[0].mvOut[i].mBuffer->getImgSize().w, params.mvDIPFrameParams[0].mvOut[i].mBuffer->getImgSize().h
                        );
                }
                else
                {
                    sprintf(vOutIMG3O_frame_str, "/sdcard/vOutIMG3O_frame-r%.2d_%.3d_%dx%d_NG.yuv",
                        dump_round_count, dumped_frame_count,
                        params.mvDIPFrameParams[0].mvOut[i].mBuffer->getImgSize().w, params.mvDIPFrameParams[0].mvOut[i].mBuffer->getImgSize().h
                        );
                }
                params.mvDIPFrameParams[0].mvOut[i].mBuffer->saveToFile(vOutIMG3O_frame_str);
                MY_LOGD_IF(m3dnrLogLevel >= 2, "params.mvOut[%zu] EPortIndex_IMG3O saved: %p", i, params.mvDIPFrameParams[0].mvOut[i].mBuffer);
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


MVOID P2ANode::dump_imgiImageBuffer(const Feature::P2Util::DIPParams &)
{
    // Not implemented
}


} // namespace NSFeaturePipe
} // namespace NSCamFeature
} // namespace NSCam
