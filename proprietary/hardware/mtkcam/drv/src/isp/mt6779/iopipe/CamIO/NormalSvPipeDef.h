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

/******************************************************************************
 *
 ******************************************************************************/

#include "CommonPipeDef.h"

#define _NORMALSVPIPE_GET_SENSORTYPE(sensorIdx)       (NormalSvPipe::pAllSensorInfo->mSenInfo[sensorIdx].mSInfo.sensorType)
#define _NORMALSVPIPE_GET_SENSOR_FMT_BIT(sensorIdx)   (NormalSvPipe::pAllSensorInfo->mSenInfo[sensorIdx].mSInfo.rawSensorBit)
#define _NORMALSVPIPE_GET_SENSOR_FMT_ORDER(sensorIdx) (NormalSvPipe::pAllSensorInfo->mSenInfo[sensorIdx].mSInfo.sensorFormatOrder)
#define _NORMALSVPIPE_GET_SENSORCFG_CROP_W(sensorIdx) (NormalSvPipe::pAllSensorInfo->mSenInfo[sensorIdx].mConfig.crop.w)
#define _NORMALSVPIPE_GET_SENSORCFG_CROP_H(sensorIdx) (NormalSvPipe::pAllSensorInfo->mSenInfo[sensorIdx].mConfig.crop.h)
#define _NORMALSVPIPE_GET_SENSOR_DEV_ID(sensorIdx)    (NormalSvPipe::pAllSensorInfo->mSenInfo[sensorIdx].mDevId)
#define _NORMALSVPIPE_GET_TG_IDX(sensorIdx)           (NormalSvPipe::pAllSensorInfo->mSenInfo[sensorIdx].mDInfo.TgInfo)
#define _NORMALSVPIPE_GET_CLK_FREQ(sensorIdx)         (NormalSvPipe::pAllSensorInfo->mSenInfo[sensorIdx].mDInfo.TgCLKInfo)


#ifdef USE_IMAGEBUF_HEAP
#define _NOMRALSVPIPE_BUF_GETPA(buf, plane_id) (buf.mBuffer->getBufPA(plane_id))
#define _NOMRALSVPIPE_BUF_GETVA(buf, plane_id) (buf.mBuffer->getBufVA(plane_id))
#define _NOMRALSVPIPE_BUF_GETSZ(buf, plane_id) (buf.mBuffer->getBufSizeInBytes(plane_id))
#else
#define _NOMRALSVPIPE_BUF_GETPA(buf, plane_id) (buf.mPa)
#define _NOMRALSVPIPE_BUF_GETVA(buf, plane_id) (buf.mVa)
#define _NOMRALSVPIPE_BUF_GETSZ(buf, plane_id) (buf.mSize)
#endif

#define NSVPIPE_MEM_NEW(dstPtr, type, size, memInfo)\
do {\
    memInfo.NSvPipeAllocMemSum += size; \
    dstPtr = new type;\
} while(0)

#define NSVPIPE_MEM_DEL(dstPtr, size, memInfo)\
do{\
    memInfo.NSvPipeFreedMemSum += size; \
    delete dstPtr;\
    dstPtr = NULL;\
}while(0)

inline MBOOL NormalSvPipe::FSM_CHECK(E_FSM op, const char *callee)
{
    MBOOL ret = MTRUE;

    this->m_FSMLock.lock();
	PIPE_INF("[%s]FSM_CHECK:cur:0x%x,tar:0x%x\n", callee, this->m_FSM, op);
    switch(op){
    case op_unknown:
        if(this->m_FSM != op_uninit)
            ret = MFALSE;
        break;
    case op_init:
        if(this->m_FSM != op_unknown)
            ret = MFALSE;
        break;
    case op_cfg:
        if(this->m_FSM != op_init)
            ret = MFALSE;
        break;
    case op_start:
        switch (this->m_FSM) {
        case op_cfg:
        case op_suspend:
            break;
        default:
            ret = MFALSE;
            break;
        }
        break;
    case op_stop:
        switch (this->m_FSM) {
        case op_start:
        case op_suspend:
            break;
        default:
            ret = MFALSE;
            break;
        }
        break;
    case op_suspend:
        if (this->m_FSM != op_start) {
            ret = MFALSE;
        }
        break;
    case op_uninit:
        switch(this->m_FSM){
        case op_init:
        case op_cfg:
        case op_stop:
            break;
        default:
            ret = MFALSE;
            break;
        }
        break;
    case op_cmd:
        switch(this->m_FSM){
        case op_cfg:
        case op_start:
        case op_stop:
        case op_suspend:
            break;
        default:
            ret= MFALSE;
            break;
        }
        break;
    case op_streaming:
        switch(this->m_FSM){
        case op_cfg:
        case op_start:
            break;
        default:
            ret= MFALSE;
            break;
        }
        break;
    default:
        ret = MFALSE;
        break;
    }
    if (ret == MFALSE) {
        PIPE_ERR("[%s]op error:cur:0x%x,tar:0x%x\n", callee, this->m_FSM,op);
    }

    this->m_FSMLock.unlock();

    return ret;
}

inline MBOOL NormalSvPipe::FSM_UPDATE(E_FSM op)
{
    switch(op){
    case op_unknown:
    case op_init:
        break;
    case op_cfg:
        break;
    case op_start:
    case op_suspend:
        break;
    case op_uninit:
        break;
    case op_stop:
        break;
    default:
        PIPE_ERR("op update error: cur:0x%x,tar:0x%x\n",this->m_FSM,op);
        return MFALSE;
    }

    this->m_FSMLock.lock();
    this->m_FSM = op;
    this->m_FSMLock.unlock();

    return MTRUE;
}

inline MUINT32 NSVPIPE_MAP_PORTID(MUINT32 const PortIdx)
{
    MUINT32 _idx = 0;

    if (PortIdx == PORT_IMGO.index)
       _idx = ePort_imgo;
    else
        PIPE_ERR("error:portidx:0x%x\n", PortIdx);

    return _idx;
}

inline MUINT32 NSVPIPE_MAP_FMT(MUINT32 const portidx, EImageFormat fmt)
{
    return fmt;
}

inline MBOOL NSVPIPE_GET_CAPIBILITY(
    ENPipeQueryCmd const cmd,
    NormalPipe_InputInfo const qryInput,
    PortInfo const pOut)
{
    MBOOL                ret = MTRUE;
    CAM_CAPIBILITY*      pinfo = CAM_CAPIBILITY::CreateInsatnce(THIS_NAME);
    NormalPipe_QueryInfo queryRst;
    MUINT32 PlaneMax = 0;

    switch(cmd) {
        case ENPipeQueryCmd_X_PIX:
        case ENPipeQueryCmd_CROP_X_PIX:
            if (pinfo->GetCapibility(pOut.index, cmd, qryInput, queryRst)) {
                if (queryRst.x_pix != qryInput.width) {
                    PIPE_ERR("port:0x%x width r invalid (cur:%d valid:%d)", pOut.index, pOut.u4ImgWidth, queryRst.x_pix);
                    ret = MFALSE;
                }
            }
            else {
                PIPE_ERR("GetCapibility fail: cmd(%d) fmt(0x%x) | dma(0x%x)",
                    (MUINT32)cmd, (MUINT32)qryInput.format, pOut.index);
                ret = MFALSE;
            }
            break;
        case ENPipeQueryCmd_STRIDE_BYTE:
            if (pinfo->GetCapibility(pOut.index, cmd, qryInput, queryRst)) {
                PlaneMax = ePlane_1st;

                for (MUINT32 PlaneID = 0; PlaneID <= PlaneMax; PlaneID++) {
                    if (queryRst.stride_B[PlaneID] > pOut.u4Stride[PlaneID]) {
                        PIPE_WRN("port:0x%x plane:%d stride(0x%x) is not recommend value(0x%x)",
                            pOut.index, PlaneID, pOut.u4Stride[PlaneID], queryRst.stride_B[PlaneID]);
                    }
                }
            }
            else {
                PIPE_ERR("GetCapibility fail: cmd(%d) fmt(0x%x) | dma(0x%x)",
                    (MUINT32)cmd, (MUINT32)qryInput.format, pOut.index);
                ret = MFALSE;
            }

            break;
        case ENPipeQueryCmd_D_Twin:
            if(pinfo->GetCapibility(0, cmd, qryInput, queryRst)){
                if(queryRst.D_TWIN == MFALSE){
                    PIPE_ERR("dynamic twin is not supported\n");
                    ret = MFALSE;
                }
            }
            else{
                PIPE_ERR("GetCapibility fail: cmd(%d) fmt(0x%x)",
                    (MUINT32)cmd, (MUINT32)qryInput.format);
                ret = MFALSE;
            }
            break;
         default:
            break;
    }

    pinfo->DestroyInstance(THIS_NAME);
    return ret;
}

inline E_INPUT NSVPIPE_MAP_CAM_TG(MUINT32 const sensorIdx)
{
    E_INPUT CamTG = TG_CAM_MAX;

    switch(_NORMALSVPIPE_GET_TG_IDX(sensorIdx)) {
        case CAM_SV_1:
            CamTG = TG_CAMSV_0;
            break;
        case CAM_SV_2:
            CamTG = TG_CAMSV_1;
            break;
        case CAM_SV_3:
            CamTG = TG_CAMSV_2;
            break;
        case CAM_SV_4:
            CamTG = TG_CAMSV_3;
            break;
        case CAM_SV_5:
            CamTG = TG_CAMSV_4;
            break;
        case CAM_SV_6:
            CamTG = TG_CAMSV_5;
            break;
        default:
            PIPE_ERR("Error tg idx mapping");
            break;
    }
    return CamTG;
}

inline MVOID NSVPIPE_GET_TGIFMT(MUINT32 const sensorIdx, EImageFormat &fmt) {
    switch (_NORMALSVPIPE_GET_SENSORTYPE(sensorIdx)) {
        case SENSOR_TYPE_RAW:
            switch(_NORMALSVPIPE_GET_SENSOR_FMT_BIT(sensorIdx)) {
                case RAW_SENSOR_8BIT:
                    fmt = NSCam::eImgFmt_BAYER8;
                    break;
                case RAW_SENSOR_10BIT:
                    fmt = NSCam::eImgFmt_BAYER10;
                    break;
                case RAW_SENSOR_12BIT:
                    fmt = NSCam::eImgFmt_BAYER12;
                    break;
                case RAW_SENSOR_14BIT:
                    fmt = NSCam::eImgFmt_BAYER14;
                    break;
                default:
                    PIPE_ERR("Err sen raw fmt(%d) err\n", _NORMALSVPIPE_GET_SENSOR_FMT_BIT(sensorIdx));
                    break;
            }
            break;
        case SENSOR_TYPE_YUV:
            switch(_NORMALSVPIPE_GET_SENSOR_FMT_ORDER(sensorIdx)) {
                case SENSOR_FORMAT_ORDER_UYVY:
                    fmt = NSCam::eImgFmt_UYVY;
                    break;
                case SENSOR_FORMAT_ORDER_VYUY:
                    fmt = NSCam::eImgFmt_VYUY;
                    break;
                case SENSOR_FORMAT_ORDER_YVYU:
                    fmt = NSCam::eImgFmt_YVYU;
                    break;
                case SENSOR_FORMAT_ORDER_YUYV:
                    fmt = NSCam::eImgFmt_YUY2;
                    break;
                default:
                    PIPE_ERR("Err sen yuv fmt err\n");
                    break;
            }
            break;
        case SENSOR_TYPE_JPEG:
            fmt = NSCam::eImgFmt_JPEG;
            break;
        default:
            PIPE_ERR("Err sen type(%d) err\n", _NORMALSVPIPE_GET_SENSORTYPE(sensorIdx));
            break;
    }
}

inline MVOID NSVPIPE_GET_TG_PIXID(MUINT32 const sensorIdx, ERawPxlID &pix_id)
{
    switch(NormalSvPipe::pAllSensorInfo->mSenInfo[sensorIdx].mSInfo.sensorFormatOrder){
        case SENSOR_FORMAT_ORDER_RAW_B:
            pix_id = ERawPxlID_B;
            break;
        case SENSOR_FORMAT_ORDER_RAW_Gb:
            pix_id = ERawPxlID_Gb;
            break;
        case SENSOR_FORMAT_ORDER_RAW_Gr:
            pix_id = ERawPxlID_Gr;
            break;
        case SENSOR_FORMAT_ORDER_RAW_R:
            pix_id = ERawPxlID_R;
            break;
        case SENSOR_FORMAT_ORDER_UYVY:
        case SENSOR_FORMAT_ORDER_VYUY:
        case SENSOR_FORMAT_ORDER_YUYV:
        case SENSOR_FORMAT_ORDER_YVYU:
            pix_id = ERawPxlID_B;
            break;
        default:
            PIPE_ERR("Error Pix_id: sensorIdx=%d, sensorFormatOrder=%d", sensorIdx, NormalSvPipe::pAllSensorInfo->mSenInfo[sensorIdx].mSInfo.sensorFormatOrder);
            break;
    }
}

inline MVOID NSVPIPE_GET_TG_GRAB(MUINT32 const sensorIdx, MUINT const scenarioId, MUINT32 &x, MUINT32 &y)
{
    switch(scenarioId) {
        case SENSOR_SCENARIO_ID_NORMAL_PREVIEW:
            x = NormalSvPipe::pAllSensorInfo->mSenInfo[sensorIdx].mSInfo.SensorGrabStartX_PRV;
            y = NormalSvPipe::pAllSensorInfo->mSenInfo[sensorIdx].mSInfo.SensorGrabStartY_PRV;
            break;
        case SENSOR_SCENARIO_ID_NORMAL_CAPTURE:
            x = NormalSvPipe::pAllSensorInfo->mSenInfo[sensorIdx].mSInfo.SensorGrabStartX_CAP;
            y = NormalSvPipe::pAllSensorInfo->mSenInfo[sensorIdx].mSInfo.SensorGrabStartY_CAP;
            break;
        case SENSOR_SCENARIO_ID_NORMAL_VIDEO:
            x = NormalSvPipe::pAllSensorInfo->mSenInfo[sensorIdx].mSInfo.SensorGrabStartX_VD;
            y = NormalSvPipe::pAllSensorInfo->mSenInfo[sensorIdx].mSInfo.SensorGrabStartY_VD;
            break;
        case SENSOR_SCENARIO_ID_SLIM_VIDEO1:
            x = NormalSvPipe::pAllSensorInfo->mSenInfo[sensorIdx].mSInfo.SensorGrabStartX_VD1;
            y = NormalSvPipe::pAllSensorInfo->mSenInfo[sensorIdx].mSInfo.SensorGrabStartY_VD1;
            break;
        case SENSOR_SCENARIO_ID_SLIM_VIDEO2:
            x = NormalSvPipe::pAllSensorInfo->mSenInfo[sensorIdx].mSInfo.SensorGrabStartX_VD2;
            y = NormalSvPipe::pAllSensorInfo->mSenInfo[sensorIdx].mSInfo.SensorGrabStartY_VD2;
            break;
        case SENSOR_SCENARIO_ID_CUSTOM1:
            x = NormalSvPipe::pAllSensorInfo->mSenInfo[sensorIdx].mSInfo.SensorGrabStartX_CST1;
            y = NormalSvPipe::pAllSensorInfo->mSenInfo[sensorIdx].mSInfo.SensorGrabStartY_CST1;
            break;
        case SENSOR_SCENARIO_ID_CUSTOM2:
            x = NormalSvPipe::pAllSensorInfo->mSenInfo[sensorIdx].mSInfo.SensorGrabStartX_CST2;
            y = NormalSvPipe::pAllSensorInfo->mSenInfo[sensorIdx].mSInfo.SensorGrabStartY_CST2;
            break;
        case SENSOR_SCENARIO_ID_CUSTOM3:
            x = NormalSvPipe::pAllSensorInfo->mSenInfo[sensorIdx].mSInfo.SensorGrabStartX_CST3;
            y = NormalSvPipe::pAllSensorInfo->mSenInfo[sensorIdx].mSInfo.SensorGrabStartY_CST3;
            break;
        case SENSOR_SCENARIO_ID_CUSTOM4:
            x = NormalSvPipe::pAllSensorInfo->mSenInfo[sensorIdx].mSInfo.SensorGrabStartX_CST4;
            y = NormalSvPipe::pAllSensorInfo->mSenInfo[sensorIdx].mSInfo.SensorGrabStartY_CST4;
            break;
        case SENSOR_SCENARIO_ID_CUSTOM5:
            x = NormalSvPipe::pAllSensorInfo->mSenInfo[sensorIdx].mSInfo.SensorGrabStartX_CST5;
            y = NormalSvPipe::pAllSensorInfo->mSenInfo[sensorIdx].mSInfo.SensorGrabStartY_CST5;
            break;
        default:
            x = 0;
            y = 0;
            PIPE_ERR("Error: scnario:%d sensorIdx:%d", scenarioId, sensorIdx);
            break;
    }
}

inline MUINT32 NSVPIPE_MAP_SEN_TG(E_INPUT const tg_input)
{
    MUINT32 _CAM_TG_ = CAM_TG_NONE;

    switch(tg_input){
        case TG_CAMSV_0:
            _CAM_TG_ = CAM_SV_1;
            break;
        case TG_CAMSV_1:
            _CAM_TG_ = CAM_SV_2;
            break;
        case TG_CAMSV_2:
            _CAM_TG_ = CAM_SV_3;
            break;
        case TG_CAMSV_3:
            _CAM_TG_ = CAM_SV_4;
             break;
        case TG_CAMSV_4:
            _CAM_TG_ = CAM_SV_5;
             break;
        case TG_CAMSV_5:
            _CAM_TG_ = CAM_SV_6;
            break;
        default:
            PIPE_ERR("Unknown tg (%d)", (MUINT32)tg_input);
            break;
    }
    return _CAM_TG_;
}

inline E_CamPixelMode NSVPIPE_MAP_PXLMODE(MUINT32 const sensorIdx, MUINT32 const Sen_PixMode)
{
    E_CamPixelMode tgPxlMode = ePixMode_NONE;

    if(NSCam::SENSOR_TYPE_YUV == _NORMALSVPIPE_GET_SENSORTYPE(sensorIdx)) {
        tgPxlMode = ePixMode_2;
    }
    else{
        switch (Sen_PixMode) {
            case ONE_PIXEL_MODE:
                tgPxlMode = ePixMode_1;
                break;
            case TWO_PIXEL_MODE:
                tgPxlMode = ePixMode_2;
                break;
            case FOUR_PIXEL_MODE:
                tgPxlMode = ePixMode_4;
                break;
            default:
                PIPE_ERR("Unknown tg pixel mode mapping (%d)", (MUINT32)Sen_PixMode);
                break;
        }
    }

    return tgPxlMode;
}

inline MVOID NSVPIPE_PASS_PORTINFO(
    MUINT32 const mpSensorIdx,
    PortInfo const tgi,
    portInfo const InPort,
    PortInfo* pOutPort)
{
    pOutPort->eImgFmt        = InPort.mFmt;
    pOutPort->u4PureRaw      = InPort.mPureRaw;
    pOutPort->u4PureRawPak   = InPort.mPureRawPak;
    pOutPort->crop1.x        = 0;
    pOutPort->crop1.y        = 0;
    pOutPort->crop1.floatX   = 0;
    pOutPort->crop1.floatY   = 0;
    pOutPort->crop1.w        = tgi.u4ImgWidth;
    pOutPort->crop1.h        = tgi.u4ImgHeight;
    pOutPort->u4ImgWidth     = tgi.u4ImgWidth;
    pOutPort->u4ImgHeight    = tgi.u4ImgHeight;
    pOutPort->ePxlMode       = tgi.ePxlMode;
    pOutPort->type           = NSCam::NSIoPipe::EPortType::EPortType_Memory;
    pOutPort->inout          = EPortDirection_Out;

    for(MUINT32 PlaneIdx = 0; PlaneIdx < (MUINT32)ePlane_max; PlaneIdx++){
        pOutPort->u4Stride[PlaneIdx] = InPort.mStride[PlaneIdx];
    }
}

inline MBOOL NSVPIPE_MAP_SIGNAL(EPipeSignal const eSignal, Irq_t::E_STATUS& status, Irq_t::E_STType& statusType)
{
    MBOOL ret = MTRUE;
    switch (eSignal) {
        case EPipeSignal_SOF:
            status     = Irq_t::_SOF_;
            statusType = Irq_t::_SIGNAL_INT_;
            break;
        case EPipeSignal_EOF:
            status     = Irq_t::_EOF_;
            statusType = Irq_t::_SIGNAL_INT_;
            break;
        case EPipeSignal_VSYNC:
            status     = Irq_t::_VSYNC_;
            statusType = Irq_t::_SIGNAL_INT_;
            break;
        case EPipeSignal_TG_INT:
            status     = Irq_t::_TG_INT_;
            statusType = Irq_t::_SIGNAL_INT_;
            break;
        default:
            PIPE_ERR("Invalid Signal(%d)", eSignal);
            ret = MFALSE;
            break;
    }
    return ret;
}

#ifdef USE_IMAGEBUF_HEAP
inline MVOID NSVPIPE_PASS_ENQ_BUFINFO(
    NSCam::NSIoPipe::NSCamIOPipe::BufInfo const src,
    NSImageio::NSIspio::BufInfo &dst)
{
    MUINT32 PlaneIdx = 0, PlaneMax = ePlane_1st;

    for(PlaneIdx = 0; PlaneIdx <= PlaneMax; PlaneIdx++){
        dst.u4BufSize[PlaneIdx] = (MUINT32)src.mBuffer->getBufSizeInBytes(PlaneIdx);
        dst.u4BufVA[PlaneIdx]   = (MUINTPTR)src.mBuffer->getBufVA(PlaneIdx);
        dst.u4BufPA[PlaneIdx]   = (MUINTPTR)src.mBuffer->getBufPA(PlaneIdx);
        dst.memID[PlaneIdx]     = (MUINTPTR)src.mBuffer->getFD(PlaneIdx);
    }

    dst.img_fmt = (MUINT32)src.mBuffer->getImgFormat();
    dst.m_num   = src.FrameBased.mMagicNum_tuning;

    dst.img_w      = src.FrameBased.mDstSize.w;
    dst.img_h      = src.FrameBased.mDstSize.h;
    dst.img_stride = src.mBuffer->getBufStridesInBytes(ePlane_1st);
    dst.crop_win   = src.FrameBased.mCropRect;
    dst.DstSize    = src.FrameBased.mDstSize;
}
#else
inline MVOID NSVPIPE_PASS_ENQ_BUFINFO(
    NSCam::NSIoPipe::NSCamIOPipe::BufInfo const src,
    NSImageio::NSIspio::BufInfo &dst)
{
    dst.u4BufSize[ePlane_1st] = (MUINT32)src.mSize;
    dst.u4BufVA[ePlane_1st]   = (MUINTPTR)src.mVa;
    dst.u4BufPA[ePlane_1st]   = (MUINTPTR)src.mPa;
    dst.m_num                 = src.FrameBased.mMagicNum_tuning;

    dst.img_w      = src.FrameBased.mDstSize.w;
    dst.img_h      = src.FrameBased.mDstSize.h;
    dst.img_stride = src.mStride;
    dst.crop_win   = src.FrameBased.mCropRect;
    dst.DstSize    = src.FrameBased.mDstSize;
}
#endif

inline MBOOL NSVPIPE_PASS_DEQ_BUFINFO(
    NSImageio::NSIspio::PortID const portID,
    NSImageio::NSIspio::BufInfo const src,
    NSCam::NSIoPipe::NSCamIOPipe::BufInfo &dst)
{
    ResultMetadata result;
    MBOOL ret = MTRUE;

    result.mDstSize         = src.DstSize;
    result.mTransform       = 0;
    result.mMagicNum_hal    = src.m_num;
    result.mMagicNum_tuning = src.m_num;
    result.mRawType         = src.raw_type;
    result.mTimeStamp       = src.getTimeStamp_ns();
#ifdef TS_PERF
    result.mTimeStamp_B     = src.getTimeStamp_B_ns();
#endif
    result.mHighlightData   = src.m_highlightData;
    result.eIQlv            = (E_CamIQLevel)src.eIQlv;

    switch (portID.index) {
    case EPortIndex_IMGO:
        result.mCrop_s = MRect(MPoint(0,0), MSize(src.img_w, src.img_h));
        result.mCrop_d = src.crop_win;
        break;
    default:
        PIPE_ERR("unsupported dmao:0x%x", portID.index);
        ret = MFALSE;
        break;
    }

    dst.mPortID   = portID.index;
    dst.mMetaData = result;
    dst.mSize     = src.u4BufSize[ePlane_1st];
    dst.mVa       = src.u4BufVA[ePlane_1st];
    dst.mPa       = src.u4BufPA[ePlane_1st];

    return ret;
}

inline MBOOL NSV_CHECK_FMT_UF(MINT Fmt)
{
    // ISP5.0 not support ufo for camsv
    return MFALSE;
}

#ifdef USE_IMAGEBUF_HEAP
inline void NormalSvPipe::FrameMgr::enque(IImageBuffer *pframe)
{
    Mutex::Autolock lock(mFrameMgrLock);
    PIPE_DBG("FrameMgr::enque+, 0x%" PRIXPTR "", pframe->getBufPA(0));
    mvFrameQ.add(pframe->getBufPA(ePlane_1st), pframe);//change from VA 2 PA is because of in camera3, drv may have no VA
}

inline IImageBuffer* NormalSvPipe::FrameMgr::deque(MINTPTR key) //key == va
{
    Mutex::Autolock lock(mFrameMgrLock);
    PIPE_DBG("FrameMgr::deque+ key=0x%" PRIXPTR "",key);
    IImageBuffer *pframe = mvFrameQ.valueFor(key);
    if (pframe != NULL) {
        mvFrameQ.removeItem(key);
        return pframe;
    }
    return NULL;
}
#endif

inline MUINT32 NSVPIPE_MAP_TGI(MUINT32 tgIdx)
{
    MUINT32 tgi = EPortIndex_UNKNOW;

    switch(tgIdx){
    case CAM_SV_1:
        tgi = EPortIndex_CAMSV_0_TGI;
        break;
    case CAM_SV_2:
        tgi = EPortIndex_CAMSV_1_TGI;
        break;
    case CAM_SV_3:
        tgi = EPortIndex_CAMSV_2_TGI;
        break;
    case CAM_SV_4:
        tgi = EPortIndex_CAMSV_3_TGI;
        break;
    case CAM_SV_5:
        tgi = EPortIndex_CAMSV_4_TGI;
        break;
    case CAM_SV_6:
        tgi = EPortIndex_CAMSV_5_TGI;
        break;
    default:
        PIPE_ERR("invalid tg index(%d) for camsv channels", tgIdx);
	}

    return tgi;
}
