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
 * MediaTek Inc. (C) 2019. All rights reserved.
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

#include <mtkcam3/feature/utils/p2/DIPStream_V4L2Stream.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <linux/media.h>
#include <linux/videodev2.h>
#include <linux/v4l2-mediabus.h>
#include <linux/v4l2-subdev.h>
#include <mtkcam3/feature/utils/p2/P2Trace.h>
#if MTK_V4L2_READY
#include <mtkcam/utils/v4l2/MtkV4l2Utils.h>
#endif

#include <DpDataType.h>

#define ILOG_MODULE_TAG DIPStream_V4L2Stream
#include <mtkcam3/feature/utils/log/ILogHeader.h>

CAM_ULOG_DECLARE_MODULE_ID(MOD_FPIPE_COMMON);

#define ARRAY_SIZE(x) (sizeof(x)/sizeof(x[0]))

#define DIP_MEDIA_DEVICE_NAME "MTK-ISP-DIP-V4L2"
#define DIP_BATCH_MODE_SDEV_NAME "MTK-ISP-DIP-V4L2"
#define DIP_BATCH_MODE_VDEV_NAME "MTK-ISP-DIP-V4L2 Raw Input"

#define MAX_DEV_NUM 255
#define MAX_PATH_STRING_LENTH 1023

#define MAX_LOG_LENTH 1024

namespace NSCam {
namespace Feature {
namespace P2Util {

DIPStream_V4L2Stream::DIPStream_V4L2Stream(MUINT32 sensorIndex)
    : DIPStream(sensorIndex)
{
#if MTK_V4L2_READY
    mBypassV4L2 = ::property_get_bool("vendor.debug.utils.p2.v4l2.bypass", MFALSE);
#endif
}

DIPStream_V4L2Stream::~DIPStream_V4L2Stream()
{
    uninit();
}

MBOOL DIPStream_V4L2Stream::init(char const* , NSCam::NSIoPipe::EStreamPipeID , MUINT32 )
{
    TRACE_FUNC_ENTER();
    P2_CAM_TRACE_BEGIN(TRACE_ADVANCED, "DIPStream_V4L2Stream::init");
    MBOOL ret = MFALSE;
    P2_CAM_TRACE_END(TRACE_ADVANCED);
    TRACE_FUNC_EXIT();
    return ret;

}

MBOOL DIPStream_V4L2Stream::uninit(char const* szCallerName)
{
    TRACE_FUNC_ENTER();
    P2_CAM_TRACE_BEGIN(TRACE_ADVANCED, "DIPStream_V4L2Stream::uninit");
    MY_LOGD("uninit %s +", szCallerName);
    MY_LOGD("uninit %s +", szCallerName);
    P2_CAM_TRACE_END(TRACE_ADVANCED);
    TRACE_FUNC_EXIT();
    return MTRUE;
}

MBOOL DIPStream_V4L2Stream::enque(const DIPParams &dipParams)
{
    (void)dipParams;
    TRACE_FUNC_ENTER();
    P2_CAM_TRACE_BEGIN(TRACE_ADVANCED, "DIPStream_V4L2Stream::enque");
    MBOOL ret = MFALSE;
    P2_CAM_TRACE_END(TRACE_ADVANCED);
    TRACE_FUNC_EXIT();
    return ret;
}

MUINT32 DIPStream_V4L2Stream::getRegTableSize()
{
    return 0;
}

#if MTK_V4L2_READY
MBOOL DIPStream_V4L2Stream::init(char const* szCallerName, NSCam::NSIoPipe::EStreamPipeID pipeID, MUINT32 secTag)
{
    TRACE_FUNC_ENTER();
    P2_CAM_TRACE_BEGIN(TRACE_ADVANCED, "DIPStream_V4L2Stream::init");
    MBOOL ret = MFALSE;
    MY_LOGD("init %s +", szCallerName);
    (void)pipeID;
    (void)secTag;
    if((mMediaFd = open_media_device(DIP_MEDIA_DEVICE_NAME)) >= 0 &&
       (mHubFd = open_device(mMediaFd, DIP_BATCH_MODE_SDEV_NAME, 0)) >= 0 &&
       (mVideoFd = open_device(mMediaFd, DIP_BATCH_MODE_VDEV_NAME, 0)) >= 0 )
    {
        // stream on
        MY_LOGD("stream on...");
        ::ioctl(mVideoFd, MTKDIP_IOC_STREAMON);

        if( mDequeThread == NULL )
        {
            mDequeThread = new DequeThread(mVideoFd, mLogEn, mBypassV4L2);
            mDequeThread->run("Cam@P2DequeV4L2");
        }
        mInitState = MTRUE;
        ret = MTRUE;
    }
    else
    {
        uninit(szCallerName);
        ret = MFALSE;
    }
    MY_LOGD("init %s -", szCallerName);
    P2_CAM_TRACE_END(TRACE_ADVANCED);
    TRACE_FUNC_EXIT();
    return ret;

}

MBOOL DIPStream_V4L2Stream::uninit(char const* szCallerName)
{
    TRACE_FUNC_ENTER();
    P2_CAM_TRACE_BEGIN(TRACE_ADVANCED, "DIPStream_V4L2Stream::uninit");
    MY_LOGD("uninit %s +", szCallerName);
    if( mFrameParams != NULL )
    {
        delete [] mFrameParams;
        mFrameParams = NULL;
    }
    if( mDequeThread != NULL )
    {
        mDequeThread->signalStop();
        mDequeThread->join();
        mDequeThread = NULL;
    }
    // stream off
    if( mVideoFd >= 0 )
    {
        MY_LOGD("stream off...");
        ::ioctl(mVideoFd, MTKDIP_IOC_STREAMOFF);
        MY_LOGD("close VideoFd...");
        close_device(mVideoFd);
        mVideoFd = -1;
    }
    if( mHubFd >= 0 )
    {
        MY_LOGD("close HubFd...");
        close_device(mHubFd);
        mHubFd= -1;
    }
    if( mMediaFd >= 0 )
    {
        MY_LOGD("close MediaFd...");
        close_device(mMediaFd);
        mMediaFd = -1;
    }
    mInitState = MFALSE;
    MY_LOGD("uninit %s -", szCallerName);
    P2_CAM_TRACE_END(TRACE_ADVANCED);
    TRACE_FUNC_EXIT();
    return MTRUE;
}

MBOOL DIPStream_V4L2Stream::enque(const DIPParams &dipParams)
{
    TRACE_FUNC_ENTER();
    P2_CAM_TRACE_BEGIN(TRACE_ADVANCED, "DIPStream_V4L2Stream::enque");
    MBOOL ret = MFALSE;
    if( mDequeThread != NULL && mInitState == MTRUE)
    {
        MY_LOGD("eqbuf +");
        /**
         * add DIPParams mSeqNum
         */
        DIPParams eqDipParams = dipParams;
        eqDipParams.mSeqNum = mSeqNum++;

        /**
         * set init frame_pack
         */
        struct frame_param_pack v4l2Pack;
        memset(&v4l2Pack, 0, sizeof(v4l2Pack));

        size_t curBatchNum = eqDipParams.mvDIPFrameParams.size();
        reserveFrameParams(curBatchNum);
        if( mFrameParams != NULL )
        {
            memset(mFrameParams, 0, curBatchNum * sizeof(frame_param));
            v4l2Pack.frame_params = mFrameParams;
            v4l2Pack.num_frames = (uint8_t)curBatchNum;
            toV4_Pack(eqDipParams, v4l2Pack);
        }
        else
        {
            MY_LOGE("FrameParams pointer is NULL");
            mBypassV4L2 = MTRUE;
        }
        if( mLogEn )
        {
            dumpLog(v4l2Pack);
        }
        if( mBypassV4L2 )
        {
            MY_LOGD("Bypass V4L2 ioctl enque seqNum(%lu)", eqDipParams.mSeqNum);
        }
        else
        {
            ::ioctl(mVideoFd, MTKDIP_IOC_QBUF, &v4l2Pack);
        }
        MY_LOGD("eqbuf -");

        mDequeThread->signalDeque(eqDipParams);
        ret = MTRUE;
    }
    P2_CAM_TRACE_END(TRACE_ADVANCED);
    TRACE_FUNC_EXIT();
    return ret;
}

MVOID DIPStream_V4L2Stream::reserveFrameParams(size_t curBatchNum)
{
    if( curBatchNum > mFrameParamsSize )
    {
        MY_LOGD("Update FrameParams size : %zu -> %zu", mFrameParamsSize, curBatchNum);
        if( mFrameParams != NULL )
        {
            delete [] mFrameParams;
            mFrameParams = NULL;
        }
        mFrameParams = new frame_param[curBatchNum];
        mFrameParamsSize = curBatchNum;
    }
}

MUINT32 DIPStream_V4L2Stream::getRegTableSize()
{
    return MtkV4l2Utils::getRegTableSize();
}

MVOID DIPStream_V4L2Stream::toV4_Pack(const DIPParams &dipParams, frame_param_pack &pack)
{
    TRACE_FUNC_ENTER();
    P2_CAM_TRACE_BEGIN(TRACE_ADVANCED, "DIPStream_V4L2Stream::convertToV4L2Params");

    pack.seq_num = dipParams.mSeqNum;
    frame_param *frame = pack.frame_params;
    if( frame != NULL )
    {
        for( uint32_t i = 0 ; i < pack.num_frames ; ++i )
        {
            toV4_FrameParam(dipParams.mvDIPFrameParams[i], frame[i]);
        }
    }
    else
    {
        MY_LOGE("pack.frame_params is NULL");
    }

    P2_CAM_TRACE_END(TRACE_ADVANCED);
    TRACE_FUNC_EXIT();
}

MVOID DIPStream_V4L2Stream::toV4_FrameParam(const DIPFrameParams &dipFrameParam, frame_param &v4FrameParam)
{
    v4FrameParam.timestamp = dipFrameParam.Timestamp;
    v4FrameParam.type = dipFrameParam.mStreamTag;
    v4FrameParam.tuning_data.va = (uint64_t)dipFrameParam.mTuningData;
    v4FrameParam.num_inputs = (uint8_t)dipFrameParam.mvIn.size();
    v4FrameParam.num_outputs = (uint8_t)dipFrameParam.mvOut.size();

    for( uint32_t i = 0, inArrayCount = ARRAY_SIZE(v4FrameParam.inputs) ; i < v4FrameParam.num_inputs &&  i < inArrayCount ; ++i )
    {
        toV4_Input(dipFrameParam.mvIn[i], v4FrameParam.inputs[i]);
    }
    OutPortI outPortI = OutPortI();
    for( uint32_t i = 0, outArrayCount = ARRAY_SIZE(v4FrameParam.outputs) ; i < v4FrameParam.num_outputs && i < outArrayCount ; ++i )
    {
        // save i for crop info
        outPortI.setOutPortI((NSImageio::NSIspio::EPortIndex)dipFrameParam.mvOut[i].mPortID.index, i);
        toV4_Output(dipFrameParam.mvOut[i], v4FrameParam.outputs[i]);
    }
    for( uint32_t i = 0, cropRsInfoCount = dipFrameParam.mvCropRsInfo.size() ; i < cropRsInfoCount ; ++i )
    {
        MINT32 outPortId = outPortI.getOutPortI(dipFrameParam.mvCropRsInfo[i].mGroupID);
        if(outPortId >= 0 && outPortId < v4FrameParam.num_outputs)
        {
            toV4_CrpRsInfo(dipFrameParam.mvCropRsInfo[i], v4FrameParam.outputs[outPortId]);
        }
    }

    toV4_DipParam(dipFrameParam, v4FrameParam.dip_param);
}

MVOID DIPStream_V4L2Stream::toV4_DipParam(const DIPFrameParams &dipFrameParam, ext_param &v4ExtParam)
{
    v4ExtParam.frame_no = dipFrameParam.FrameNo;
    v4ExtParam.request_no = dipFrameParam.RequestNo;
    v4ExtParam.unique_key = dipFrameParam.UniqueKey;
    v4ExtParam.expected_end_time = dipFrameParam.ExpectedEndTime;
    v4ExtParam.num_srz_cfg = (uint8_t)dipFrameParam.mvModuleData.size();
    v4ExtParam.num_extra_param = (uint8_t)dipFrameParam.mvExtraParam.size();
    // dipFrameParam.mSecureFra : next stage

    for( uint32_t i = 0, moduleArrayCount = ARRAY_SIZE(v4ExtParam.srz_cfg) ; i < v4ExtParam.num_srz_cfg &&  i < moduleArrayCount ; ++i )
    {
        toV4_SrzConfig(dipFrameParam.mvModuleData[i], v4ExtParam.srz_cfg[i]);
    }
    for( uint32_t i = 0, extraArrayCount = ARRAY_SIZE(v4ExtParam.extra_param) ; i < v4ExtParam.num_extra_param &&  i < extraArrayCount ; ++i )
    {
        toV4_ExtraParam(dipFrameParam.mvExtraParam[i], v4ExtParam.extra_param[i]);
    }
}

MVOID DIPStream_V4L2Stream::toV4_Input(const NSIoPipe::Input &in, img_input &input)
{
    input.buffer.usage = in.mPortID.index;
    input.buffer.format.colorformat = MtkV4l2Utils::getColorFormat((NSCam::EImageFormat)in.mBuffer->getImgFormat(), in.mBuffer->getColorArrangement());
    input.buffer.format.width = in.mBuffer->getImgSize().w;
    input.buffer.format.height = in.mBuffer->getImgSize().h;
    // in.mBuffer->getColorProfile() : Kurt need confirm buffer.format.ycbcr_prof is need?
    // in.mBuffer->getFD() : next stage maybe need
    for( uint32_t pc = 0, planeCount = in.mBuffer->getPlaneCount() ; pc < planeCount && pc < IMG_MAX_PLANES ; ++pc )
    {
        input.buffer.iova[pc] = in.mBuffer->getBufPA(pc);
        input.buffer.format.plane_fmt[pc].size = in.mBuffer->getBufSizeInBytes(pc);
        input.buffer.format.plane_fmt[pc].stride = in.mBuffer->getBufStridesInBytes(pc);
        // in.mBuffer->getBufVA() : next stage maybe need
    }
}

MVOID DIPStream_V4L2Stream::toV4_Output(const NSIoPipe::Output &out, img_output &output)
{
    toV4_Transform(out.mTransform, output)
    // out.mSecureTag : next stage
    // out.mSecHandle : next stage
    // Buffer
    output.buffer.usage = out.mPortID.index;
    output.buffer.format.colorformat = MtkV4l2Utils::getColorFormat((NSCam::EImageFormat)out.mBuffer->getImgFormat(), out.mBuffer->getColorArrangement());
    output.buffer.format.width = out.mBuffer->getImgSize().w;
    output.buffer.format.height = out.mBuffer->getImgSize().h;
    // out.mBuffer->getColorProfile() : Kurt need confirm buffer.format.ycbcr_prof is need?
    // out.mBuffer->getFD() : next stage maybe need
    for( uint32_t pc = 0, planeCount = out.mBuffer->getPlaneCount() ; pc < planeCount ; ++pc )
    {
        output.buffer.iova[pc] = out.mBuffer->getBufPA(pc);
        output.buffer.format.plane_fmt[pc].size = out.mBuffer->getBufSizeInBytes(pc);
        output.buffer.format.plane_fmt[pc].stride = out.mBuffer->getBufStridesInBytes(pc);
        // out.mBuffer->getBufVA() : next stage maybe need
    }
}

MVOID DIPStream_V4L2Stream::toV4_Transform(const MINT32 transform, img_output &output)
{
    switch(transform)
    {
        case eTransform_None:
            output.rotation = 0;
            output.flags = 0;
            break;
        case eTransform_FLIP_H:
            output.rotation = 0;
            output.flags = IMG_CTRL_FLAG_HFLIP;
            break;
        case eTransform_FLIP_V:
            output.rotation = 180;
            output.flags = IMG_CTRL_FLAG_HFLIP;
            break;
        case eTransform_ROT_90:
            output.rotation = 90;
            output.flags = 0;
            break;
        case eTransform_ROT_180:
            output.rotation = 180;
            output.flags = 0;
            break;
        case eTransform_ROT_270:
            output.rotation = 270;
            output.flags = 0;
            break;
        default:
            output.rotation = 0;
            output.flags = 0;
            break;
    }
}

MVOID DIPStream_V4L2Stream::toV4_CrpRsInfo(const NSIoPipe::MCrpRsInfo &crpRsInfo, img_output &output)
{
    output.type = crpRsInfo.mMdpGroup;
    output.compose.left = crpRsInfo.mCropRect.p_integral.x;
    output.compose.top = crpRsInfo.mCropRect.p_integral.y;
    output.compose.width = crpRsInfo.mCropRect.s.w;
    output.compose.height = crpRsInfo.mCropRect.s.h;
    output.crop.left = crpRsInfo.mCropRect.p_integral.x;
    output.crop.top = crpRsInfo.mCropRect.p_integral.y;
    output.crop.width = crpRsInfo.mCropRect.s.w;
    output.crop.height = crpRsInfo.mCropRect.s.h;
    output.crop.left_subpix = crpRsInfo.mCropRect.p_fractional.x;
    output.crop.top_subpix = crpRsInfo.mCropRect.p_fractional.y;
    output.crop.width_subpix = crpRsInfo.mCropRect.w_fractional;
    output.crop.height_subpix = crpRsInfo.mCropRect.h_fractional;
}

MVOID DIPStream_V4L2Stream::toV4_SrzConfig(const NSIoPipe::ModuleInfo &moduleData, srz_config &srzConfig)
{
    _SRZ_SIZE_INFO_ *pSrcModule = (_SRZ_SIZE_INFO_ *)moduleData.moduleStruct;
    if(pSrcModule != NULL)
    {
        srzConfig.srz_module_id = (dip_srz_module)moduleData.moduleTag;
        srzConfig.in_w = pSrcModule->in_w;
        srzConfig.in_h = pSrcModule->in_h;
        srzConfig.out_w = pSrcModule->out_w;
        srzConfig.out_h = pSrcModule->out_h;
        srzConfig.crop_x = pSrcModule->crop_x;
        srzConfig.crop_y = pSrcModule->crop_y;
        srzConfig.crop_floatX = pSrcModule->crop_floatX;
        srzConfig.crop_floatY = pSrcModule->crop_floatY;
        srzConfig.crop_w = pSrcModule->crop_w;
        srzConfig.crop_h = pSrcModule->crop_h;
    }
}

MVOID DIPStream_V4L2Stream::toV4_ExtraParam(const NSIoPipe::ExtraParam &srcExtraParam, extra_param &dstExtraParam)
{
    dstExtraParam.cmd_idx = srcExtraParam.CmdIdx;
    switch(srcExtraParam.CmdIdx)
    {
        case NSIoPipe::EPIPE_FE_INFO_CMD:
            {
                NSIoPipe::FEInfo *pFEInfoSrc = (NSIoPipe::FEInfo *)srcExtraParam.moduleStruct;
                NSIoPipe::FEInfo *pFEInfoDst = (NSIoPipe::FEInfo *)dstExtraParam.module_struct;
                *pFEInfoDst = *pFEInfoSrc;
            }
            break;
        case NSIoPipe::EPIPE_FM_INFO_CMD:
            {
                NSIoPipe::FMInfo *pFMInfoSrc = (NSIoPipe::FMInfo *)srcExtraParam.moduleStruct;
                NSIoPipe::FMInfo *pFMInfoDst = (NSIoPipe::FMInfo *)dstExtraParam.module_struct;
                *pFMInfoDst = *pFMInfoSrc;
            }
            break;
        case NSIoPipe::EPIPE_MDP_PQPARAM_CMD:
            {
                NSIoPipe::PQParam *pPQParamSrc = (NSIoPipe::PQParam *)srcExtraParam.moduleStruct;
                NSIoPipe::PQParam *pPQParamDst = (NSIoPipe::PQParam *)dstExtraParam.module_struct;
                *pPQParamDst = *pPQParamSrc;
            }
            break;
        case NSIoPipe::EPIPE_IMG3O_CRSPINFO_CMD:
            {
                NSIoPipe::CrspInfo *pCrspInfoSrc = (NSIoPipe::CrspInfo *)srcExtraParam.moduleStruct;
                NSIoPipe::CrspInfo *pCrspInfoDst = (NSIoPipe::CrspInfo *)dstExtraParam.module_struct;
                *pCrspInfoDst = *pCrspInfoSrc;
            }
            break;
        default:
            MY_LOGD("unknown ext: idx=%d", srcExtraParam.CmdIdx);
            break;
    };
}

MVOID DIPStream_V4L2Stream::dumpLog(const frame_param_pack &param)
{
    TRACE_FUNC_ENTER();
    P2_CAM_TRACE_BEGIN(TRACE_ADVANCED, "DIPStream_V4L2Stream::dumpLog");

    const frame_param *frame = param.frame_params;
    if( frame != NULL )
    {
        for( uint32_t f = 0 ; f < param.num_frames ; ++f )
        {
            MY_LOGD("[FrameParams][frames# %u/%hhu] seq_num:%lu num_frames:%hhu timestamp=%lu type=%hhu num_inputs=%hhu num_outputs=%hhu, tuning_data.va=0x%lu",
                f, param.num_frames, param.seq_num, param.num_frames, frame[f].timestamp, frame[f].type, frame[f].num_inputs, frame[f].num_outputs, frame[f].tuning_data.va);

            for( uint32_t i = 0 ; i < frame[f].num_inputs ; ++i )
            {
                dumpLog(frame[f].inputs[i], i);
            }
            for( uint32_t i = 0 ; i < frame[f].num_outputs ; ++i )
            {
                dumpLog(frame[f].outputs[i], i);
            }

            dumpLog(frame[f].dip_param);
        }
    }
    else
    {
        MY_LOGE("param.frame_params is NULL");
    }

    P2_CAM_TRACE_END(TRACE_ADVANCED);
    TRACE_FUNC_EXIT();
}

MVOID DIPStream_V4L2Stream::dumpLog(const img_input &input, const uint32_t i)
{
    // Buffer
    MY_LOGD("[FrameParams][inputs %u] buffer usage=%u format.colorformat=%u format.width=%hu format.height=%hu",
        i, input.buffer.usage, input.buffer.format.colorformat, input.buffer.format.width, input.buffer.format.height);
    for( uint32_t pc = 0; pc < IMG_MAX_PLANES ; ++pc )
    {
        if( input.buffer.iova[pc] || input.buffer.format.plane_fmt[pc].size || input.buffer.format.plane_fmt[pc].stride )
        {
            MY_LOGD("[FrameParams][inputs %u] buffer format.plane_fmt[%u] iova=0x%09x size=%u stride=%hu",
                i, pc, input.buffer.iova[pc], input.buffer.format.plane_fmt[pc].size, input.buffer.format.plane_fmt[pc].stride);
        }
    }
}

MVOID DIPStream_V4L2Stream::dumpLog(const img_output &output, const uint32_t i)
{
    MY_LOGD("[FrameParams][outputs %u] flags=%hu rotation=%hd",
        i, output.flags, output.rotation);
    // Buffer
    MY_LOGD("[FrameParams][outputs %u] buffer usage=%u format.colorformat=%u format.width=%hu format.height=%hu",
        i, output.buffer.usage, output.buffer.format.colorformat, output.buffer.format.width, output.buffer.format.height);
    for( uint32_t pc = 0; pc < IMG_MAX_PLANES ; ++pc )
    {
        if( output.buffer.iova[pc] || output.buffer.format.plane_fmt[pc].size || output.buffer.format.plane_fmt[pc].stride )
        {
            MY_LOGD("[FrameParams][outputs %u] buffer format.plane_fmt[%u] iova=0x%09x size=%u stride=%hu",
                i, pc, output.buffer.iova[pc], output.buffer.format.plane_fmt[pc].size, output.buffer.format.plane_fmt[pc].stride);
        }
    }

    MY_LOGD("[FrameParams][outputs %u] CropRsInfo type=%hhu compose=(%hd,%hd)(%hux%hu) crop=(%hd,%hd)(%hux%hu) crop_subpix=(%u,%u)(%ux%u)",
        i, output.type,
        output.compose.left, output.compose.top, output.compose.width, output.compose.height,
        output.crop.left, output.crop.top, output.crop.width, output.crop.height,
        output.crop.left_subpix, output.crop.top_subpix, output.crop.width_subpix, output.crop.height_subpix);
}

MVOID DIPStream_V4L2Stream::dumpLog(const ext_param &v4ExtParam)
{
    struct timeval curr;
    gettimeofday(&curr, NULL);
    MY_LOGD("[DipParams] frame_no=%u request_no=%u unique_key=%u num_srz_cfg=%hhu num_extra_param=%hhu",
        v4ExtParam.frame_no, v4ExtParam.request_no, v4ExtParam.unique_key, v4ExtParam.num_srz_cfg, v4ExtParam.num_extra_param);
    MY_LOGD("[DipParams] CurrentTime=%u.%03u EndTime=%u.%03u",
        (MUINT32)curr.tv_sec%1000, (MUINT32)curr.tv_usec/1000, (MUINT32)v4ExtParam.expected_end_time.tv_sec%1000, (MUINT32)v4ExtParam.expected_end_time.tv_usec/1000);

    char dumpStr[MAX_LOG_LENTH] = {0};
    MINT32 length = 0;
    for( uint32_t i = 0 ; i < v4ExtParam.num_srz_cfg ; ++i )
    {
        MY_LOGD("[DipParams][moduleInfo/srz_config %u] srz_config: srz_module_id=%u in(%ux%u) out(%ux%u) crop(%u,%u) crop_float(%u,%u) crop_size(%lux%lu)",
            i, v4ExtParam.srz_cfg[i].srz_module_id,
            v4ExtParam.srz_cfg[i].in_w, v4ExtParam.srz_cfg[i].in_h, v4ExtParam.srz_cfg[i].out_w, v4ExtParam.srz_cfg[i].out_h,
            v4ExtParam.srz_cfg[i].crop_x, v4ExtParam.srz_cfg[i].crop_y, v4ExtParam.srz_cfg[i].crop_floatX, v4ExtParam.srz_cfg[i].crop_floatY,
            v4ExtParam.srz_cfg[i].crop_w, v4ExtParam.srz_cfg[i].crop_h);
    }
    for( uint32_t i = 0 ; i < v4ExtParam.num_extra_param ; ++i )
    {
        length = 0;
        length += snprintf(dumpStr + length, MAX_LOG_LENTH - length, "[DipParams][extraParam %u] cmd_idx=%u module_struct=", i, v4ExtParam.extra_param[i].cmd_idx);
        for( uint32_t j = 0 ; j < EXTRA_PARAM_SIZE ; ++j )
        {
            length += snprintf(dumpStr + length, MAX_LOG_LENTH - length, " %hhu", v4ExtParam.extra_param[i].module_struct[j]);
        }
        MY_LOGD("%s", dumpStr);
    }
}

MBOOL DIPStream_V4L2Stream::OutPortI::setOutPortI(NSImageio::NSIspio::EPortIndex portID, int i)
{
    MBOOL ret = MFALSE;
    switch(portID)
    {
        case NSImageio::NSIspio::EPortIndex_IMG2O:
            mImg2oId = i;
            ret = MTRUE;
            break;
        case NSImageio::NSIspio::EPortIndex_WDMAO:
            mWdmaoId = i;
            ret = MTRUE;
            break;
        case NSImageio::NSIspio::EPortIndex_WROTO:
            mWrotoId = i;
            ret = MTRUE;
            break;
        default:
            ret = MFALSE;
            break;
    }
    return ret;
}

MINT32 DIPStream_V4L2Stream::OutPortI::getOutPortI(MINT32 groupID)
{
    MINT32 outPortI = -1;
    switch(groupID)
    {
        case 1: // CRZ IMG2O
            outPortI = mImg2oId;
            break;
        case 2: // WDMAO
            outPortI = mWdmaoId;
            break;
        case 3: // WROTO
            outPortI = mWrotoId;
            break;
        default:
            MY_LOGW("unknown CropRsInfo GroupID: %d", groupID);
            break;
    }
    return outPortI;
}

int DIPStream_V4L2Stream::open_device(const char *dev_name, int non_block)
{
    TRACE_FUNC_ENTER();
    P2_CAM_TRACE_BEGIN(TRACE_ADVANCED, "DIPStream_V4L2Stream::open_device");

    MY_LOGD("open_device: %s, non_block=%d", dev_name, non_block);

    struct stat st;
    int fd = 0;
    if (-1 == stat(dev_name, &st)) {
        MY_LOGE("Cannot identify '%s': %d, %s", dev_name, errno, strerror(errno));
        return -1;
    }

    if (!S_ISCHR(st.st_mode)) {
        MY_LOGE("\t-%s is no devicen", dev_name);
        return -1;
    }

    fd = ::open(dev_name, O_RDWR /* required */ | (non_block ? O_NONBLOCK : 0), 0);
    if (-1 == fd) {
        MY_LOGE("\t-Cannot open '%s': %d, %s", dev_name, errno, strerror(errno));
        return -1;
    }
    P2_CAM_TRACE_END(TRACE_ADVANCED);
    TRACE_FUNC_EXIT();
    return fd;
}

void DIPStream_V4L2Stream::close_device(int fd)
{
    TRACE_FUNC_ENTER();
    P2_CAM_TRACE_BEGIN(TRACE_ADVANCED, "DIPStream_V4L2Stream::close_device");

    if (-1 == close(fd))
        MY_LOGE("\t-close error %d, %s", errno, strerror(errno));

    P2_CAM_TRACE_END(TRACE_ADVANCED);
    TRACE_FUNC_EXIT();
}

int DIPStream_V4L2Stream::open_media_device(const char *match_name)
{
    TRACE_FUNC_ENTER();
    P2_CAM_TRACE_BEGIN(TRACE_ADVANCED, "DIPStream_V4L2Stream::open_media_device");

    int media_fd = -1, ret = 0;

    for (int index = 0; index < 255; index++) {
        char media_device_path[32] = "\0";
        struct media_device_info info;
        memset(&info, 0, sizeof(struct media_device_info));

        sprintf(media_device_path, "/dev/media%d", index);
        media_fd = open_device(media_device_path, 0);

        MY_LOGD(">> check %s, fd=%d", media_device_path, media_fd);

        if (media_fd < 0) {
            MY_LOGE("Cannot open '%s': %d, %s", media_device_path, errno, strerror(errno));
            return -1;
        } else {
            ret = ::ioctl(media_fd, MEDIA_IOC_DEVICE_INFO, &info);
            if (ret < 0) {
                MY_LOGE("Failed to get device info for %s", media_device_path);
                exit(EXIT_FAILURE);
            } else {
                MY_LOGD("* Media Device Info: \n\tdriver:%s\n\tmodel:%s\n\tserial:%s\n\tbus_info:%s",
                        info.driver, info.model, info.serial, info.bus_info);
                if (strcmp(match_name, info.model) == 0) {
                    MY_LOGD("Found media device (%s)", match_name);
                    break;
                }
            }
        }
        close_device(media_fd);
    }

    P2_CAM_TRACE_END(TRACE_ADVANCED);
    TRACE_FUNC_EXIT();
    return media_fd;
}

int DIPStream_V4L2Stream::open_device(int media_fd, const char *match_name, int non_block)
{
    TRACE_FUNC_ENTER();
    P2_CAM_TRACE_BEGIN(TRACE_ADVANCED, "DIPStream_V4L2Stream::open_device");
    int video_fd = -1, ret = 0;
    struct media_entity_desc entity;

    for (int i = 0; i < MAX_DEV_NUM; i++) {
        memset(&entity, 0, sizeof(struct media_entity_desc));
        entity.id = i | MEDIA_ENT_ID_FLAG_NEXT;
        ret = ::ioctl(media_fd, MEDIA_IOC_ENUM_ENTITIES, &entity);

        if (ret < 0) {
            MY_LOGE("Failed to enum device entities");
            return -1;
        }


        /* match the node */
        if(!strcmp(entity.name, match_name)){
            char stringStream[MAX_PATH_STRING_LENTH]="\0";
            char sysname[MAX_PATH_STRING_LENTH]="\0";
            char *lastSlash = NULL;

            MY_LOGD("entity name: %s\n\t major: %d\t minor: %d",
                entity.name, entity.v4l.major, entity.v4l.minor);

            sprintf(stringStream, "/sys/dev/char/%d:%d", entity.v4l.major, entity.v4l.minor);
            MY_LOGD("\t dev-node: %s",stringStream);

            if (::readlink(stringStream, sysname, sizeof(sysname)) <1) {
                MY_LOGE("Failed to readlink of %s", stringStream);
                return -1;
            }

            sysname[MAX_PATH_STRING_LENTH-1] = '\0';
            MY_LOGD("\t sys-name:%s",sysname);
            lastSlash = strrchr(sysname, '/');
            if (lastSlash == NULL) {
                MY_LOGE("Invalid sysfs device path");
                return -1;
            } else {
                char video_device_name[MAX_PATH_STRING_LENTH]="\0";
                sprintf(video_device_name, "/dev/%s", lastSlash+1);
                MY_LOGD("\t video-dev-name is found!!:%s", video_device_name);
                return open_device(video_device_name, non_block);
            }
        }
    }
    P2_CAM_TRACE_END(TRACE_ADVANCED);
    TRACE_FUNC_EXIT();
    return video_fd;
}

DIPStream_V4L2Stream::DequeThread::DequeThread(int videoFd, MBOOL logEn, MBOOL bypassV4L2)
    : mVideoFd(videoFd)
    , mStop(MFALSE)
    , mLogEn(logEn)
    , mBypassV4L2(bypassV4L2)
{
    TRACE_FUNC_ENTER();
    TRACE_FUNC_EXIT();
}

DIPStream_V4L2Stream::DequeThread::~DequeThread()
{
    TRACE_FUNC_ENTER();
    TRACE_FUNC_EXIT();
}

MVOID DIPStream_V4L2Stream::DequeThread::signalDeque(const DIPParams &dipParams)
{
    TRACE_FUNC_ENTER();
    P2_CAM_TRACE_BEGIN(TRACE_ADVANCED, "DIPStream_V4L2Stream::DequeThread::signalDeque");
    android::Mutex::Autolock lock(mThreadMutex);
    mParamQueue.push(dipParams);
    mThreadCondition.broadcast();
    P2_CAM_TRACE_END(TRACE_ADVANCED);
    TRACE_FUNC_EXIT();
}

MVOID DIPStream_V4L2Stream::DequeThread::signalStop()
{
    TRACE_FUNC_ENTER();
    P2_CAM_TRACE_BEGIN(TRACE_ADVANCED, "DIPStream_V4L2Stream::DequeThread::signalStop");
    android::Mutex::Autolock lock(mThreadMutex);
    mStop = MTRUE;
    mThreadCondition.broadcast();
    P2_CAM_TRACE_END(TRACE_ADVANCED);
    TRACE_FUNC_EXIT();
}

android::status_t DIPStream_V4L2Stream::DequeThread::readyToRun()
{
    TRACE_FUNC_ENTER();
    TRACE_FUNC_EXIT();
    return android::NO_ERROR;
}

bool DIPStream_V4L2Stream::DequeThread::threadLoop()
{
    TRACE_FUNC_ENTER();
    P2_CAM_TRACE_BEGIN(TRACE_ADVANCED, "DIPStream_V4L2Stream::DequeThread::threadLoop");

    DIPParams dipParams;
    while( waitParam(dipParams) )
    {
        processParam(dipParams);
    }

    P2_CAM_TRACE_END(TRACE_ADVANCED);
    TRACE_FUNC_EXIT();

    return false;
}

MBOOL DIPStream_V4L2Stream::DequeThread::waitParam(DIPParams &dipParams)
{
    TRACE_FUNC_ENTER();
    P2_CAM_TRACE_BEGIN(TRACE_ADVANCED, "DIPStream_V4L2Stream::DequeThread::waitParam");
    android::Mutex::Autolock lock(mThreadMutex);
    MBOOL ret = MFALSE, done = MFALSE;

    do
    {
        if( mParamQueue.size() )
        {
            dipParams = mParamQueue.front();
            mParamQueue.pop();

            ret = MTRUE;
            done = MTRUE;
        }
        else if( mStop )
        {
            ret = MFALSE;
            done = MTRUE;
        }
        else
        {
            mThreadCondition.wait(mThreadMutex);
        }
    }
    while( !done );

    P2_CAM_TRACE_END(TRACE_ADVANCED);
    TRACE_FUNC_EXIT();

    return ret;
}

MVOID DIPStream_V4L2Stream::DequeThread::processParam(DIPParams &dipParams)
{
    TRACE_FUNC_ENTER();
    P2_CAM_TRACE_BEGIN(TRACE_ADVANCED, "DIPStream_V4L2Stream::DequeThread::processParam");

    // v4l2Params different
    struct frame_param_pack dqV4l2Params;
    MY_LOGD("dqbuf +");
    if( mBypassV4L2 )
    {
        MY_LOGD("Bypass V4L2 ioctl deque seqNum(%lu)", dipParams.mSeqNum);
        dqV4l2Params.seq_num = dipParams.mSeqNum;
    }
    else
    {
        ::ioctl(mVideoFd, MTKDIP_IOC_DQBUF, &dqV4l2Params);
    }
    MY_LOGD("dqbuf -");

    dipParams.mDequeSuccess = checkSeqNum(dipParams.mSeqNum, dqV4l2Params.seq_num);

    if(dipParams.mpfnDIPCallback)
    {
        dipParams.mpfnDIPCallback(dipParams);
    }
    else
    {
        MY_LOGE("Missing callback");
    }

    P2_CAM_TRACE_END(TRACE_ADVANCED);
    TRACE_FUNC_EXIT();
}

MBOOL DIPStream_V4L2Stream::DequeThread::checkSeqNum(const uint64_t enqueNum, const uint64_t dequeNum)
{
    MBOOL ret = MFALSE;
    MY_LOGD_IF(mLogEn, "Check sequnce number enqueNum(%lu) dequeNum(%lu)", enqueNum, dequeNum);
    ret = (enqueNum == dequeNum);
    if( !ret )
    {
        MY_LOGE("Sequnce number not match: enqueNum(%lu) dequeNum(%lu). P2 driver drop frame or deque order error", enqueNum, dequeNum);
    }

    return ret;
}
#endif //MTK_V4L2_READY
} // namespace P2Util
} // namespace Feature
} // namespace NSCam
