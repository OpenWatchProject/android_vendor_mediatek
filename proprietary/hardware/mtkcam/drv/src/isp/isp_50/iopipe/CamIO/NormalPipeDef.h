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

#define _NORMALPIPE_GET_SENSORTYPE(sensorIdx)       (NormalPipe::pAllSensorInfo->mSenInfo[sensorIdx].mSInfo.sensorType)
#define _NORMALPIPE_GET_SENSOR_WIDTH(sensorIdx)     (NormalPipe::pAllSensorInfo->mSenInfo[sensorIdx].mSInfo.captureWidth)
#define _NORMALPIPE_GET_SENSOR_HEIGHT(sensorIdx)    (NormalPipe::pAllSensorInfo->mSenInfo[sensorIdx].mSInfo.captureHeight)
#define _NORMALPIPE_GET_SENSORCFG_CROP_W(sensorIdx) (NormalPipe::pAllSensorInfo->mSenInfo[sensorIdx].mConfig.crop.w)
#define _NORMALPIPE_GET_SENSORCFG_CROP_H(sensorIdx) (NormalPipe::pAllSensorInfo->mSenInfo[sensorIdx].mConfig.crop.h)
#define _NORMALPIPE_GET_SENSOR_DEV_ID(sensorIdx)    (NormalPipe::pAllSensorInfo->mSenInfo[sensorIdx].mDevId)
#define _NORMALPIPE_GET_SENSOR_FMT_BIT(sensorIdx)   (NormalPipe::pAllSensorInfo->mSenInfo[sensorIdx].mSInfo.rawSensorBit)
#define _NORMALPIPE_GET_TG_IDX(sensorIdx)           (NormalPipe::pAllSensorInfo->mSenInfo[sensorIdx].mDInfo.TgInfo)
#define _NORMALPIPE_GET_CLK_FREQ(sensorIdx)         (NormalPipe::pAllSensorInfo->mSenInfo[sensorIdx].mDInfo.TgCLKInfo)
#define _NORMALPIPE_GET_SENSOR_FMT_ORDER(sensorIdx) (NormalPipe::pAllSensorInfo->mSenInfo[sensorIdx].mSInfo.sensorFormatOrder)

#ifdef USE_IMAGEBUF_HEAP
#define _NOMRALPIPE_BUF_GETPA(buf, plane_id) (buf.mBuffer->getBufPA(plane_id))
#define _NOMRALPIPE_BUF_GETVA(buf, plane_id) (buf.mBuffer->getBufVA(plane_id))
#define _NOMRALPIPE_BUF_GETSZ(buf, plane_id) (buf.mBuffer->getBufSizeInBytes(plane_id))
#else
#define _NOMRALPIPE_BUF_GETPA(buf, plane_id) (buf.mPa)
#define _NOMRALPIPE_BUF_GETVA(buf, plane_id) (buf.mVa)
#define _NOMRALPIPE_BUF_GETSZ(buf, plane_id) (buf.mSize)
#endif

#define NPIPE_MEM_NEW(dstPtr, type, size, memInfo)\
do {\
    memInfo.NPipeAllocMemSum += size; \
    dstPtr = new type;\
} while(0)
#define NPIPE_MEM_DEL(dstPtr, size, memInfo)\
do{\
    memInfo.NPipeFreedMemSum += size; \
    delete dstPtr;\
    dstPtr = NULL;\
}while(0)

/******************************************************************************
 *
 ******************************************************************************/
inline MUINT32 NPIPE_MAP_FMT(MUINT32 const portidx, EImageFormat fmt)
{
    MUINT32 _fmt = fmt;

    if (portidx == EPortIndex_RRZO) {
        switch(fmt){
            case eImgFmt_BAYER8:
                _fmt = eImgFmt_FG_BAYER8;
                break;
            case eImgFmt_BAYER10:
                _fmt = eImgFmt_FG_BAYER10;
                break;
            case eImgFmt_BAYER12:
                _fmt = eImgFmt_FG_BAYER12;
                break;
            default:
                _fmt = eImgFmt_FG_BAYER10;
            break;
        }
    }
    return _fmt;
}

//map portId to deque container index
inline MUINT32 NPIPE_MAP_PORTID(MUINT32 const PortIdx)
{
    MUINT32 _idx = 0;

    if (PortIdx == PORT_IMGO.index)
       _idx = ePort_imgo;
    else if (PortIdx == PORT_RRZO.index)
       _idx = ePort_rrzo;
    else if (PortIdx == PORT_EISO.index)
       _idx = ePort_eiso;
    else if (PortIdx == PORT_LCSO.index)
       _idx = ePort_lcso;
    else if (PortIdx == PORT_RSSO.index)
       _idx = ePort_rsso;
    else
        PIPE_ERR("error:portidx:0x%x\n", PortIdx);

    return _idx;
}

//switch to camiopipe index
inline MVOID NPIPE_GET_TGIFMT(MUINT32 const sensorIdx, EImageFormat &fmt) {
    switch (_NORMALPIPE_GET_SENSORTYPE(sensorIdx)) {
        case SENSOR_TYPE_RAW:
            switch(_NORMALPIPE_GET_SENSOR_FMT_BIT(sensorIdx)) {
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
                    PIPE_ERR("Err sen raw fmt(%d) err\n", _NORMALPIPE_GET_SENSOR_FMT_BIT(sensorIdx));
                    break;
            }
            break;
        case SENSOR_TYPE_YUV:
            switch(_NORMALPIPE_GET_SENSOR_FMT_ORDER(sensorIdx)) {
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
            PIPE_ERR("Err sen type(%d) err\n", _NORMALPIPE_GET_SENSORTYPE(sensorIdx));
            break;
    }
}

inline E_CAM_CTL_TG_FMT NPIPE_GET_TG_HWFMT(MUINT32 sensorIdx)
{
    E_CAM_CTL_TG_FMT HwVal = TG_FMT_RAW8;
    switch (_NORMALPIPE_GET_SENSORTYPE(sensorIdx)) {
        case SENSOR_TYPE_RAW:
            {
                switch (_NORMALPIPE_GET_SENSOR_FMT_BIT(sensorIdx)) {
                    case RAW_SENSOR_8BIT:
                        HwVal = TG_FMT_RAW8;
                        break;
                    case RAW_SENSOR_10BIT:
                        HwVal = TG_FMT_RAW10;
                        break;
                    case RAW_SENSOR_12BIT:
                        HwVal = TG_FMT_RAW12;
                        break;
                    case RAW_SENSOR_14BIT:
                        HwVal = TG_FMT_RAW14;
                        break;
                    default:
                        PIPE_ERR("Fmt:[%d] NOT Support", _NORMALPIPE_GET_SENSOR_FMT_BIT(sensorIdx));
                        break;
                    }
            }
            break;
        case SENSOR_TYPE_YUV:
            {
                switch (_NORMALPIPE_GET_SENSOR_FMT_BIT(sensorIdx)) {
                case SENSOR_FORMAT_ORDER_UYVY:
                case SENSOR_FORMAT_ORDER_VYUY:
                case SENSOR_FORMAT_ORDER_YVYU:
                case SENSOR_FORMAT_ORDER_YUYV:
                    HwVal = TG_FMT_YUV422;
                    break;
                default:
                    PIPE_ERR("Fmt:[%d] NOT Support", _NORMALPIPE_GET_SENSOR_FMT_BIT(sensorIdx));
                    break;
                }
            }
            break;
        default:
            PIPE_ERR("FmtType:[%d] NOT Support", _NORMALPIPE_GET_SENSORTYPE(sensorIdx));
            break;
    }
    return HwVal;
}

//switch to camiopipe idx
inline MVOID NPIPE_GET_TG_PIXID(MUINT32 const sensorIdx, ERawPxlID &pix_id)
{
    switch(NormalPipe::pAllSensorInfo->mSenInfo[sensorIdx].mSInfo.sensorFormatOrder){
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
            PIPE_ERR("Error Pix_id: sensorIdx=%d, sensorFormatOrder=%d", sensorIdx, NormalPipe::pAllSensorInfo->mSenInfo[sensorIdx].mSInfo.sensorFormatOrder);
            break;
    }
}

inline MVOID NPIPE_GET_TG_GRAB(MUINT32 const sensorIdx, MUINT const scenarioId, MUINT32 &x, MUINT32 &y)
{
    switch(scenarioId) {
        case SENSOR_SCENARIO_ID_NORMAL_PREVIEW:
            x = NormalPipe::pAllSensorInfo->mSenInfo[sensorIdx].mSInfo.SensorGrabStartX_PRV;
            y = NormalPipe::pAllSensorInfo->mSenInfo[sensorIdx].mSInfo.SensorGrabStartY_PRV;
            break;
        case SENSOR_SCENARIO_ID_NORMAL_CAPTURE:
            x = NormalPipe::pAllSensorInfo->mSenInfo[sensorIdx].mSInfo.SensorGrabStartX_CAP;
            y = NormalPipe::pAllSensorInfo->mSenInfo[sensorIdx].mSInfo.SensorGrabStartY_CAP;
            break;
        case SENSOR_SCENARIO_ID_NORMAL_VIDEO:
            x = NormalPipe::pAllSensorInfo->mSenInfo[sensorIdx].mSInfo.SensorGrabStartX_VD;
            y = NormalPipe::pAllSensorInfo->mSenInfo[sensorIdx].mSInfo.SensorGrabStartY_VD;
            break;
        case SENSOR_SCENARIO_ID_SLIM_VIDEO1:
            x = NormalPipe::pAllSensorInfo->mSenInfo[sensorIdx].mSInfo.SensorGrabStartX_VD1;
            y = NormalPipe::pAllSensorInfo->mSenInfo[sensorIdx].mSInfo.SensorGrabStartY_VD1;
            break;
        case SENSOR_SCENARIO_ID_SLIM_VIDEO2:
            x = NormalPipe::pAllSensorInfo->mSenInfo[sensorIdx].mSInfo.SensorGrabStartX_VD2;
            y = NormalPipe::pAllSensorInfo->mSenInfo[sensorIdx].mSInfo.SensorGrabStartY_VD2;
            break;
        case SENSOR_SCENARIO_ID_CUSTOM1:
            x = NormalPipe::pAllSensorInfo->mSenInfo[sensorIdx].mSInfo.SensorGrabStartX_CST1;
            y = NormalPipe::pAllSensorInfo->mSenInfo[sensorIdx].mSInfo.SensorGrabStartY_CST1;
            break;
        case SENSOR_SCENARIO_ID_CUSTOM2:
            x = NormalPipe::pAllSensorInfo->mSenInfo[sensorIdx].mSInfo.SensorGrabStartX_CST2;
            y = NormalPipe::pAllSensorInfo->mSenInfo[sensorIdx].mSInfo.SensorGrabStartY_CST2;
            break;
        case SENSOR_SCENARIO_ID_CUSTOM3:
            x = NormalPipe::pAllSensorInfo->mSenInfo[sensorIdx].mSInfo.SensorGrabStartX_CST3;
            y = NormalPipe::pAllSensorInfo->mSenInfo[sensorIdx].mSInfo.SensorGrabStartY_CST3;
            break;
        case SENSOR_SCENARIO_ID_CUSTOM4:
            x = NormalPipe::pAllSensorInfo->mSenInfo[sensorIdx].mSInfo.SensorGrabStartX_CST4;
            y = NormalPipe::pAllSensorInfo->mSenInfo[sensorIdx].mSInfo.SensorGrabStartY_CST4;
            break;
        case SENSOR_SCENARIO_ID_CUSTOM5:
            x = NormalPipe::pAllSensorInfo->mSenInfo[sensorIdx].mSInfo.SensorGrabStartX_CST5;
            y = NormalPipe::pAllSensorInfo->mSenInfo[sensorIdx].mSInfo.SensorGrabStartY_CST5;
            break;
        default:
            x = 0;
            y = 0;
            PIPE_ERR("Error: scnario:%d sensorIdx:%d", scenarioId, sensorIdx);
            break;
    }
}

inline MUINT32 NPIPE_GET_SENSOR_FPS(MUINT32 const sensorIdx, MUINT const scenarioId)
{
    MUINT32 fps = 0;
    switch(scenarioId) {
        case SENSOR_SCENARIO_ID_NORMAL_PREVIEW:
            fps = NormalPipe::pAllSensorInfo->mSenInfo[sensorIdx].mSInfo.previewFrameRate;
            break;
        case SENSOR_SCENARIO_ID_NORMAL_CAPTURE:
            fps = NormalPipe::pAllSensorInfo->mSenInfo[sensorIdx].mSInfo.captureFrameRate;
            break;
        case SENSOR_SCENARIO_ID_NORMAL_VIDEO:
            fps = NormalPipe::pAllSensorInfo->mSenInfo[sensorIdx].mSInfo.videoFrameRate;
            break;
        case SENSOR_SCENARIO_ID_SLIM_VIDEO1:
            fps = NormalPipe::pAllSensorInfo->mSenInfo[sensorIdx].mSInfo.video1FrameRate;
            break;
        case SENSOR_SCENARIO_ID_SLIM_VIDEO2:
            fps = NormalPipe::pAllSensorInfo->mSenInfo[sensorIdx].mSInfo.video2FrameRate;
            break;
        case SENSOR_SCENARIO_ID_CUSTOM1:
            fps = NormalPipe::pAllSensorInfo->mSenInfo[sensorIdx].mSInfo.custom1FrameRate;
            break;
        case SENSOR_SCENARIO_ID_CUSTOM2:
            fps = NormalPipe::pAllSensorInfo->mSenInfo[sensorIdx].mSInfo.custom2FrameRate;
            break;
        case SENSOR_SCENARIO_ID_CUSTOM3:
            fps = NormalPipe::pAllSensorInfo->mSenInfo[sensorIdx].mSInfo.custom3FrameRate;
            break;
        case SENSOR_SCENARIO_ID_CUSTOM4:
            fps = NormalPipe::pAllSensorInfo->mSenInfo[sensorIdx].mSInfo.custom4FrameRate;
            break;
        case SENSOR_SCENARIO_ID_CUSTOM5:
            fps = NormalPipe::pAllSensorInfo->mSenInfo[sensorIdx].mSInfo.custom5FrameRate;
            break;
        default:
            PIPE_ERR("Error: scnario:%d sensorIdx:%d", scenarioId, sensorIdx);
            break;
    }
    return fps;
}

//switch to camiopipe index
inline E_INPUT NPIPE_MAP_CAM_TG(MUINT32 const sensorIdx)
{
    E_INPUT CamTG = TG_CAM_MAX;

    switch(_NORMALPIPE_GET_TG_IDX(sensorIdx)) {
        case CAM_TG_1:
            CamTG = TG_A;
            break;
        case CAM_TG_2:
            CamTG = TG_B;
            break;
        default:
            PIPE_ERR("Error tg idx mapping");
            break;
    }
    return CamTG;
}

inline E_CamPixelMode NPIPE_MAP_PXLMODE(MUINT32 const sensorIdx, MUINT32 const Sen_PixMode)
{
    E_CamPixelMode tgPxlMode = ePixMode_NONE;

    if(NSCam::SENSOR_TYPE_YUV == _NORMALPIPE_GET_SENSORTYPE(sensorIdx)) {
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

inline NPIPE_PATH_E NPIPE_MAP_PATH(MUINT32 const sensorIdx)
{
    NPIPE_PATH_E cam_path = NPIPE_CAM_MAX;

    switch(_NORMALPIPE_GET_TG_IDX(sensorIdx)){
        case CAM_TG_1: cam_path = NPIPE_CAM_A;
            break;
        case CAM_TG_2: cam_path = NPIPE_CAM_B;
            break;
        default:
            PIPE_ERR("Unknown tg (%d)", (MUINT32)_NORMALPIPE_GET_TG_IDX(sensorIdx));
            break;
    }
    return cam_path;
}

inline MUINT32 NPIPE_MAP_SEN_TG(E_INPUT const tg_input)
{
    MUINT32 _CAM_TG_ = CAM_TG_NONE;

    switch(tg_input){
        case TG_A:
            _CAM_TG_ = CAM_TG_1;
            break;
        case TG_B:
            _CAM_TG_ = CAM_TG_2;
            break;
        default:
            PIPE_ERR("Unknown tg (%d)", (MUINT32)tg_input);
            break;
    }
    return _CAM_TG_;
}

inline MBOOL NPIPE_MAP_SIGNAL(EPipeSignal const eSignal, Irq_t::E_STATUS& status, Irq_t::E_STType& statusType)
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
        case EPipeSignal_AFDONE:
            status     = Irq_t::_AF_DONE_;
            statusType = Irq_t::_DMA_INT_;
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

inline MVOID TIME_SPEC_CAL(struct timespec &ts, MUINT32 &u4TimeoutMs)
{
    MUINT32 delta_s = u4TimeoutMs / 1000;
    MUINT32 delta_ns = (u4TimeoutMs - delta_s * 1000) * 1000000;
    MUINT32 const NPS = 1000000000; // ns per sec

    if ((ts.tv_nsec + delta_ns) >= NPS) { // 2*10^9=0x77359400 < 32bit
        MUINT32 dds = (delta_ns + ts.tv_nsec) / NPS;

        ts.tv_sec = (ts.tv_sec + delta_s + dds);
        ts.tv_nsec = (ts.tv_nsec + delta_ns) - dds * NPS;
    }
    else {
        ts.tv_sec = ts.tv_sec + delta_s;
        ts.tv_nsec = ts.tv_nsec + delta_ns;
    }
}

inline MUINTPTR ALIGN_CAL(MUINTPTR data, MUINT32 align_num)
{
    MUINT32 ShiftBit = 0;

    switch(align_num) {
        case 16:
            ShiftBit = 4;
            break;
        default:
            PIPE_ERR("unknow align num(%d)", align_num);
            break;
    }

    data = ((data + align_num - 1) >> ShiftBit) << ShiftBit;

    return data;
}
inline MBOOL CHECK_FMT_PAK(MINT Fmt)
{
    MBOOL ret = MTRUE;

    switch((EImageFormat)Fmt)
    {
        case eImgFmt_BAYER8:
        case eImgFmt_BAYER10:
        case eImgFmt_BAYER12:
        case eImgFmt_BAYER14:
        case eImgFmt_BAYER8_UNPAK:
        case eImgFmt_BAYER10_UNPAK:
        case eImgFmt_BAYER12_UNPAK:
        case eImgFmt_BAYER14_UNPAK:
            break;
        default:
            ret = MFALSE;
            break;
    }
    return ret;
}

inline MINT _MapPAKtoB(MINT Fmt)
{
    MINT ret = 0;

    switch(Fmt) {
    case eImgFmt_BAYER8:
        ret = eImgFmt_BAYER8_UNPAK;
        break;
    case eImgFmt_BAYER10:
        ret = eImgFmt_BAYER10_UNPAK;
        break;
    case eImgFmt_BAYER12:
        ret = eImgFmt_BAYER12_UNPAK;
        break;
    case eImgFmt_BAYER14:
        ret = eImgFmt_BAYER14_UNPAK;
        break;
    case eImgFmt_BAYER8_UNPAK:
        ret = eImgFmt_BAYER8;
        break;
    case eImgFmt_BAYER10_UNPAK:
        ret = eImgFmt_BAYER10;
        break;
    case eImgFmt_BAYER12_UNPAK:
        ret = eImgFmt_BAYER12;
        break;
    case eImgFmt_BAYER14_UNPAK:
        ret = eImgFmt_BAYER14;
        break;
    default:
        PIPE_ERR("unsupported PAK fmt:0x%x", Fmt);
        break;
    }
    return ret;
}

inline MBOOL CHECK_FMT_UF(MINT Fmt)
{
    MBOOL ret = MTRUE;

    switch((EImageFormat)Fmt)
    {
        case eImgFmt_UFO_BAYER8:
        case eImgFmt_UFO_BAYER10:
        case eImgFmt_UFO_BAYER12:
        case eImgFmt_UFO_BAYER14:
        case eImgFmt_UFO_FG_BAYER8:
        case eImgFmt_UFO_FG_BAYER10:
        case eImgFmt_UFO_FG_BAYER12:
        case eImgFmt_UFO_FG_BAYER14:
            break;
        default:
            ret = MFALSE;
            break;
    }
    return ret;
}

inline MINT _MapUFtoB(MINT Fmt)
{
    MINT ret = 0;

    switch(Fmt) {
    case eImgFmt_UFO_BAYER8:
        ret = eImgFmt_BAYER8;
        break;
    case eImgFmt_UFO_BAYER10:
        ret = eImgFmt_BAYER10;
        break;
    case eImgFmt_UFO_BAYER12:
        ret = eImgFmt_BAYER12;
        break;
    case eImgFmt_UFO_BAYER14:
        ret = eImgFmt_BAYER14;
        break;
    case eImgFmt_UFO_FG_BAYER8:
        ret = eImgFmt_FG_BAYER8;
        break;
    case eImgFmt_UFO_FG_BAYER10:
        ret = eImgFmt_FG_BAYER10;
        break;
    case eImgFmt_UFO_FG_BAYER12:
        ret = eImgFmt_FG_BAYER12;
        break;
    case eImgFmt_UFO_FG_BAYER14:
        ret = eImgFmt_FG_BAYER14;
        break;
    default:
        PIPE_ERR("unsupported uf fmt:0x%x", Fmt);
        break;
    }
    return ret;
}

inline MBOOL CHECK_FMT_UF_UNPAK(MINT Fmt)
{
    MBOOL ret = MTRUE;

    switch((EImageFormat)Fmt)
    {
        case eImgFmt_UFO_BAYER8:
        case eImgFmt_UFO_BAYER10:
        case eImgFmt_UFO_BAYER12:
        case eImgFmt_UFO_BAYER14:
            break;
        default:
            ret = MFALSE;
            break;
    }
    return ret;
}


inline MINT _MapUFtoUnPak(MINT Fmt)
{
    MINT ret = 0;

    switch(Fmt) {
    case eImgFmt_UFO_BAYER8:
        ret = eImgFmt_BAYER8_UNPAK;
        break;
    case eImgFmt_UFO_BAYER10:
        ret = eImgFmt_BAYER10_UNPAK;
        break;
    case eImgFmt_UFO_BAYER12:
        ret = eImgFmt_BAYER12_UNPAK;
        break;
    case eImgFmt_UFO_BAYER14:
        ret = eImgFmt_BAYER14_UNPAK;
        break;
    default:
        PIPE_ERR("unsupported uf fmt:0x%x", Fmt);
        break;
    }
    return ret;
}

inline MBOOL CHECK_FMT_MISMATCH(MUINT32 const PortIdx, MINT CfgFmt, MINT EnqFmt)
{
    MBOOL ret = MFALSE;

    if (CfgFmt == EnqFmt) {
        ret = MTRUE;
    } else if (PortIdx == EPortIndex_IMGO &&
             (CfgFmt == eImgFmt_BAYER10 || CfgFmt == eImgFmt_BAYER12) &&
             (EnqFmt == eImgFmt_BAYER10 || EnqFmt == eImgFmt_BAYER12 ||
              EnqFmt == eImgFmt_BAYER10_UNPAK || EnqFmt == eImgFmt_BAYER12_UNPAK)) {
        // Support IMGO Bayer10/Bayer12 per frame switching
        ret = MTRUE;
    } else {
        ret = (((CHECK_FMT_UF(CfgFmt))       && (_MapUFtoB(CfgFmt)     == EnqFmt)) ||
               ((CHECK_FMT_PAK(CfgFmt))      && (_MapPAKtoB(CfgFmt)    == EnqFmt)) ||
               ((CHECK_FMT_UF_UNPAK(CfgFmt)) && (_MapUFtoUnPak(CfgFmt) == EnqFmt)))
            ? (MTRUE) : (MFALSE);
    }

    return ret;
}

//assign magic number here is for DropCB when suspend
#ifdef USE_IMAGEBUF_HEAP
inline MVOID NPIPE_PASS_ENQ_BUFINFO(
    NSCam::NSIoPipe::NSCamIOPipe::BufInfo const src,
    NSImageio::NSIspio::BufInfo &dst, MBOOL mEnableSec)
{
    MUINT32 PlaneIdx = 0, PlaneMax = 0;
    MINT ImgFormat = src.mBuffer->getImgFormat();

    if(CHECK_FMT_UF(ImgFormat)) {
        /*only pass 1st and 2nd plane*/
        PlaneMax = (MUINT32)ePlane_2nd;

        /*3rd plane is UFDG_META_INFO struct*/
        dst.m_pPrivate = (MUINT32*)src.mBuffer->getBufVA(ePlane_3rd);
    }
    else{
        /*only pass 1st plane*/
        PlaneMax = (MUINT32)ePlane_1st;
    }

    if (mEnableSec) {
        dst.u4BufSize[ePlane_1st] = (MUINT32)src.mSize;
        dst.u4BufVA[ePlane_1st]   = (MUINTPTR)src.mVa;
        dst.memID[PlaneIdx]       = (MINT32)src.mMemID;
    }
    else {
        for(PlaneIdx = 0; PlaneIdx <= PlaneMax; PlaneIdx++){
            dst.u4BufSize[PlaneIdx] = (MUINT32)src.mBuffer->getBufSizeInBytes(PlaneIdx);
            dst.u4BufVA[PlaneIdx]   = (MUINTPTR)src.mBuffer->getBufVA(PlaneIdx);
            dst.u4BufPA[PlaneIdx]   = (MUINTPTR)src.mBuffer->getBufPA(PlaneIdx);
            dst.memID[PlaneIdx]     = (src.mBuffer->getFDCount() == (PlaneMax + 1))?((MUINTPTR)src.mBuffer->getFD(PlaneIdx)):((MUINTPTR)src.mBuffer->getFD(0));
        }
    }
    dst.img_fmt = (MUINT32)ImgFormat;
    dst.m_num   = src.FrameBased.mMagicNum_tuning;

}
#else
inline MVOID NPIPE_PASS_ENQ_BUFINFO(
    NSImageio::NSIspio::BufInfo const src,
    NSCam::NSIoPipe::NSCamIOPipe::BufInfo &dst)
{
    dst.u4BufSize[ePlane_1st] = (MUINT32)src.mSize;
    dst.u4BufVA[ePlane_1st]   = (MUINTPTR)src.mVa;
    dst.u4BufPA[ePlane_1st]   = (MUINTPTR)src.mPa;
    dst.m_num        = src.FrameBased.mMagicNum_tuning;
}
#endif

inline MBOOL NPIPE_PASS_DEQ_BUFINFO(
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
    case EPortIndex_RRZO:
        result.mCrop_s = src.crop_win;
        break;
    case EPortIndex_EISO:
    case EPortIndex_LCSO:
        result.mCrop_s = result.mCrop_d = 0;
        break;
    case EPortIndex_RSSO:
        result.mCrop_s = result.mCrop_d = 0;
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

#ifdef USE_IMAGEBUF_HEAP
inline void NormalPipe::FrameMgr::enque(IImageBuffer *pframe, MUINTPTR BufPA)
{
    Mutex::Autolock lock(mFrameMgrLock);
    if(BufPA){
        PIPE_DBG("Sec FrameMgr::enque+, 0x%x", BufPA);
        mvFrameQ.add(BufPA, pframe);//change from VA 2 PA is because of in camera3, drv may have no VA
    }else{
        PIPE_DBG("FrameMgr::enque+, 0x%" PRIXPTR "", pframe->getBufPA(0));
        mvFrameQ.add(pframe->getBufPA(ePlane_1st), pframe);//change from VA 2 PA is because of in camera3, drv may have no VA
    }
}

inline IImageBuffer* NormalPipe::FrameMgr::deque(MINTPTR key) //key == va
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

inline MVOID NPIPE_PASS_PORTINFO(
    MUINT32 const mpSensorIdx,
    PortInfo const tgi,
    portInfo const InPort,
    PortInfo* pOutPort)
{
    MBOOL bPushCamIO = MFALSE;
    MUINT32 PlaneIdx = 0;

    switch(pOutPort->index){
        case EPortIndex_RRZO:
            /*rrz's output size is a input infor. for twin_mgr, so rrz's output size is useful!*/
            pOutPort->u4ImgWidth = InPort.mDstSize.w;
            pOutPort->u4ImgHeight = InPort.mDstSize.h;
            break;
        default:
            pOutPort->u4ImgWidth = tgi.u4ImgWidth;
            pOutPort->u4ImgHeight = tgi.u4ImgHeight;
            break;
    }

    pOutPort->eImgFmt        = InPort.mFmt;
    pOutPort->u4PureRaw      = InPort.mPureRaw;
    pOutPort->u4PureRawPak   = InPort.mPureRawPak;
    pOutPort->crop1.x        = 0;
    pOutPort->crop1.y        = 0;
    pOutPort->crop1.floatX   = 0;
    pOutPort->crop1.floatY   = 0;
    pOutPort->crop1.w        = tgi.u4ImgWidth;
    pOutPort->crop1.h        = tgi.u4ImgHeight;
    pOutPort->ePxlMode       = tgi.ePxlMode;
    pOutPort->type           = NSCam::NSIoPipe::EPortType::EPortType_Memory;
    pOutPort->inout          = EPortDirection_Out;

    for(PlaneIdx = 0; PlaneIdx < (MUINT32)ePlane_max; PlaneIdx++){
        pOutPort->u4Stride[PlaneIdx] = InPort.mStride[PlaneIdx];
    }
}

inline MBOOL NPIPE_GET_CAPIBILITY(
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
                PlaneMax = (CHECK_FMT_UF(qryInput.format) == MTRUE) ? (ePlane_3rd) : (ePlane_1st);

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

//currently, normalpipe supported only CAM_A & CAM_B
inline MVOID CAM_MAP(E_CAMIO_CAM const input, ENPipe_CAM &output)
{
    output = ENPipe_UNKNOWN;
    if(input & EPipe_CAM_A)
        output = (ENPipe_CAM)((MUINT32)output | ENPipe_CAM_A);
    if(input & EPipe_CAM_B)
        output = (ENPipe_CAM)((MUINT32)output | ENPipe_CAM_B);
}

inline MBOOL NormalPipe::FSM_CHECK(E_FSM op, const char *callee)
{
    MBOOL ret = MTRUE;

    this->m_FSMLock.lock();

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
        if((sIQData.IQInfo.FSM[NPIPE_CAM_A] != IQ_unknown) && (sIQData.IQInfo.FSM[NPIPE_CAM_B] != IQ_unknown)){
            if ((sIQData.IQInfo.FSM[NPIPE_CAM_A] != IQ_cfg) ||
                (sIQData.IQInfo.FSM[NPIPE_CAM_B] != IQ_cfg)) {
                ret = MFALSE;
            }
        }
        else{ // 1cam case: we need to check if current cam is IQ_done, IQ_FSM of another cam is IQ_unknown
            if ((sIQData.IQInfo.FSM[NPIPE_CAM_A] != IQ_cfg) &&
                (sIQData.IQInfo.FSM[NPIPE_CAM_B] != IQ_cfg)) {
                ret = MFALSE;
            }
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

inline MBOOL NormalPipe::FSM_UPDATE(E_FSM op)
{
    switch(op){
    case op_unknown:
    case op_init:
        break;
    case op_cfg:
        updateIQFSM(IQ_cfg, NPIPE_MAP_PATH(mpSensorIdx));
        break;
    case op_start:
    case op_suspend:
        break;
    case op_uninit:
        updateIQFSM(IQ_unknown, NPIPE_MAP_PATH(mpSensorIdx));
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

inline MVOID NpipeIQContol::updateIQCB(MUINT32 const npipe_path)
{
    NPIPE_PATH_E CamNum = NPIPE_CAM_NONE;

    switch((ISP_HW_MODULE)npipe_path){
        case CAM_A:
            CamNum = NPIPE_CAM_A;
            break;
        case CAM_B:
            CamNum = NPIPE_CAM_B;
            break;
        default:
            PIPE_ERR("index error %d", npipe_path);
            break;
    }

    if (updateIQFSM(IQ_done, CamNum)) {
        if((sIQData.IQInfo.FSM[NPIPE_CAM_A] == IQ_done) && (sIQData.IQInfo.FSM[NPIPE_CAM_B] == IQ_done)) {
            if(sIQData.IQNotify != NULL) {
                PIPE_INF("IQ Setting done.");
                sIQData.IQNotify->p1TuningNotify(NULL, NULL);
                if ((updateIQFSM(IQ_cfg, NPIPE_CAM_A)) && (updateIQFSM(IQ_cfg, NPIPE_CAM_B))) {
                    sIQData.IQNotify = NULL;
                }
            }
            else {
                PIPE_ERR("IQNotify is NULL. IQNotify:0x%p", sIQData.IQNotify);
            }
        }
    }
}
inline MBOOL NpipeIQContol::updateIQFSM(E_CamPathIQFSM const op, MUINT32 const npipe_path)
{
    MBOOL ret = MTRUE;

    Mutex::Autolock lock(NPipeIQGlock);

    switch(op){
        case IQ_cfg:
            {
                switch(sIQData.IQInfo.FSM[npipe_path]){
                    case IQ_unknown:
                    case IQ_done:
                        break;
                    default:
                        ret = MFALSE;
                        break;
                }
            }
            break;
        case IQ_npipe_set:
            if(sIQData.IQInfo.FSM[npipe_path] != IQ_cfg)
                ret = MFALSE;
            break;
        case IQ_camio_set:
            if(sIQData.IQInfo.FSM[npipe_path] != IQ_npipe_set)
                ret = MFALSE;
            break;
        case IQ_done:
            if(sIQData.IQInfo.FSM[npipe_path] != IQ_camio_set)
                ret = MFALSE;
            break;
        case IQ_unknown:
            break;
        default:
            ret = MFALSE;
            break;
    }
    if (ret) {
        PIPE_INF("[%d] IQ op update: cur_%d, tar_%d\n", (MUINT32)npipe_path,
            (MUINT32)sIQData.IQInfo.FSM[npipe_path], (MUINT32)op);
        sIQData.IQInfo.FSM[npipe_path] = op;
    }
    else {
        PIPE_ERR("[%d] IQ op error: cur_%d, tar_%d\n", (MUINT32)npipe_path,
            (MUINT32)sIQData.IQInfo.FSM[npipe_path], (MUINT32)op);
    }

    return ret;
}

