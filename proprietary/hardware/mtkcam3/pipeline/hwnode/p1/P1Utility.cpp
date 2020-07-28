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

#define LOG_TAG "MtkCam/P1NodeUtility"
//
#include "P1Common.h"
#include "P1Utility.h"


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
namespace NSCam {
namespace v3 {
namespace NSP1Node {


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

/******************************************************************************
 *
 ******************************************************************************/
INormalPipeModule* getNormalPipeModule()
{
    static auto pModule = INormalPipeModule::get();
    MY_LOGE_IF(!pModule, "INormalPipeModule::get() fail");
    return pModule;
}

/******************************************************************************
 *
 ******************************************************************************/
MUINT32 getResizeMaxRatio(MUINT32 imageFormat)
{
    static MUINT32 static_max_ratio = 0;
    // if need to query from NormalPipe every time, remove "static" as following
    // MUINT32 static_max_ratio = 0;
    if (static_max_ratio == 0) {
        if ( auto pModule = getNormalPipeModule() ) {
            NSCam::NSIoPipe::NSCamIOPipe::NormalPipe_QueryInfo info;
            pModule->query(
                NSCam::NSIoPipe::PORT_RRZO.index,
                NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd_BS_RATIO,
                (EImageFormat)imageFormat, 0, info);
            MY_LOGI("Get ENPipeQueryCmd_BS_RATIO (%d)", info.bs_ratio);
            static_max_ratio = info.bs_ratio;
        }
        if (static_max_ratio == 0) {
            MUINT32 ratio = RESIZE_RATIO_MAX_100X;
            MY_LOGI("Cannot get ENPipeQueryCmd_BS_RATIO, "
                "use default ratio (%d)", ratio);
            return ratio;
        }
    } else {
        //MY_LOGI("Got ENPipeQueryCmd_BS_RATIO(%d)", static_max_ratio);
    }
    return static_max_ratio;
}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL calculateCropInfoFull(
    MUINT32 pixelMode,
    MSize const& sensorSize,
    MSize const& bufferSize,
    MRect const& querySrcRect,
    MRect& resultSrcRect,
    MSize& resultDstSize,
    MINT32 mLogLevelI
)
{
    MBOOL bSkip = MFALSE;
    MBOOL bReject = MFALSE;
    if ((querySrcRect.size().w == sensorSize.w) &&
        (querySrcRect.size().h == sensorSize.h)) {
        MY_LOGI_IF((2 <= mLogLevelI), "No need to calculate");
        bSkip = MTRUE;
    }
    if ((querySrcRect.size().w > bufferSize.w || // cannot over buffer size
        querySrcRect.size().h > bufferSize.h) ||
        (((querySrcRect.leftTop().x + querySrcRect.size().w) > sensorSize.w) ||
        ((querySrcRect.leftTop().y + querySrcRect.size().h) > sensorSize.h))
        ) {
        MY_LOGI_IF((2 <= mLogLevelI), "Input need to check");
        bReject = MTRUE;
    }
    MY_LOGI_IF((bReject) || (3 <= mLogLevelI) || ((2 <= mLogLevelI) && bSkip),
        "[CropInfo] Input pixelMode(%d)"
        " sensorSize" P1_SIZE_STR "bufferSize" P1_SIZE_STR
        "querySrcRect" P1_RECT_STR,
        pixelMode,
        P1_SIZE_VAR(sensorSize), P1_SIZE_VAR(bufferSize),
        P1_RECT_VAR(querySrcRect));
    if (bSkip) {
        return MTRUE;
    } else if (bReject) {
        return MFALSE;
    }
    // TODO: query the valid value, currently do not crop in IMGO
    resultDstSize = MSize(sensorSize.w, sensorSize.h);
    resultSrcRect = MRect(MPoint(0, 0), resultDstSize);
    MY_LOGI_IF((2 <= mLogLevelI), "Result-Full SrcRect" P1_RECT_STR "DstSize"
        P1_SIZE_STR, P1_RECT_VAR(resultSrcRect), P1_SIZE_VAR(resultDstSize));
    return MTRUE;
};

/******************************************************************************
 *
 ******************************************************************************/
MBOOL calculateCropInfoResizer(
    MUINT32 pixelMode,
    MUINT32 imageFormat,
    MSize const& sensorSize,
    MSize const& bufferSize,
    MRect const& querySrcRect,
    MRect& resultSrcRect,
    MSize& resultDstSize,
    MINT32 mLogLevelI
)
{
    MBOOL bSkip = MFALSE;
    MBOOL bReject = MFALSE;
    if ((querySrcRect.size().w == sensorSize.w) &&
        (querySrcRect.size().h == sensorSize.h)) {
        MY_LOGI_IF((2 <= mLogLevelI), "No need to calculate");
        bSkip = MTRUE;
    } else if (
        (((querySrcRect.leftTop().x + querySrcRect.size().w) > sensorSize.w) ||
        ((querySrcRect.leftTop().y + querySrcRect.size().h) > sensorSize.h))
    ) {
        MY_LOGI_IF((2 <= mLogLevelI), "Input need to check");
        bReject = MTRUE;
    }
    MY_LOGI_IF((bReject) || (3 <= mLogLevelI) || ((2 <= mLogLevelI) && bSkip),
        "[CropInfo] Input pixelMode(%d) imageFormat(0x%x)"
        " sensorSize" P1_SIZE_STR "bufferSize" P1_SIZE_STR
        "querySrcRect" P1_RECT_STR,
        pixelMode, imageFormat,
        P1_SIZE_VAR(sensorSize), P1_SIZE_VAR(bufferSize),
        P1_RECT_VAR(querySrcRect));
    if (bSkip) {
        return MTRUE;
    } else if (bReject) {
        return MFALSE;
    }
    //
    MPoint::value_type src_crop_x = querySrcRect.leftTop().x;
    MPoint::value_type src_crop_y = querySrcRect.leftTop().y;
    MSize::value_type src_crop_w = querySrcRect.size().w;
    MSize::value_type src_crop_h = querySrcRect.size().h;
    MSize::value_type dst_size_w = 0;
    MSize::value_type dst_size_h = 0;
    // check X and W
    if (querySrcRect.size().w < bufferSize.w) {
        dst_size_w = querySrcRect.size().w;
        // check start.x
        if  ( auto pModule = getNormalPipeModule() )
        {
            NSCam::NSIoPipe::NSCamIOPipe::NormalPipe_QueryInfo info;
            pModule->query(
                NSCam::NSIoPipe::PORT_RRZO.index,
                NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd_CROP_START_X,
                (EImageFormat)imageFormat,
                src_crop_x, info);
            if ((MUINT32)src_crop_x != info.crop_x) {
                MY_LOGI_IF((2 <= mLogLevelI), "src_crop_x(%d) info.crop_x(%d)",
                    src_crop_x, info.crop_x);
            }
            src_crop_x = info.crop_x;
        }
        // check size.w
        if  ( auto pModule = getNormalPipeModule() )
        {
            NSCam::NSIoPipe::NSCamIOPipe::NormalPipe_QueryInfo info;
            pModule->query(
                NSCam::NSIoPipe::PORT_RRZO.index,
                NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd_X_PIX|
                NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd_STRIDE_BYTE,
                (EImageFormat)imageFormat,
                dst_size_w, info);
            if ((MUINT32)dst_size_w != info.x_pix) {
                MY_LOGI_IF((2 <= mLogLevelI), "dst_size_w(%d) info.x_pix(%d)",
                    dst_size_w, info.x_pix);
            }
            dst_size_w = info.x_pix;
        }
        //
        MSize::value_type cur_src_crop_x = src_crop_x;
        MSize::value_type cur_src_crop_w = src_crop_w;
        MSize::value_type cur_dst_size_w = dst_size_w;
        dst_size_w = MIN(dst_size_w, sensorSize.w);
        src_crop_w = dst_size_w;
        if (src_crop_w > querySrcRect.size().w) {
            if ((src_crop_x + src_crop_w) > sensorSize.w) {
                src_crop_x = sensorSize.w - src_crop_w;
            }
        }
        if ((cur_src_crop_x != src_crop_x) || (cur_src_crop_w != src_crop_w)
            || (cur_dst_size_w != dst_size_w)) {
            MY_LOGI_IF((2 <= mLogLevelI), "ValueChanged-XW src_crop_x(%d):(%d) "
                "src_crop_w(%d):(%d) dst_size_w(%d):(%d) sensor_w(%d)",
                cur_src_crop_x, src_crop_x, cur_src_crop_w, src_crop_w,
                cur_dst_size_w, dst_size_w, sensorSize.w);
        }
        MY_LOGI_IF((3 <= mLogLevelI),
            "CheckXW Crop<Buf(%d<%d) Res-Src:X(%d):W(%d)-Dst:W(%d) SensorW(%d)",
            querySrcRect.size().w, bufferSize.w,
            src_crop_x, src_crop_w, dst_size_w, sensorSize.w);
    } else {
        #if 0
        if ( auto pModule = getNormalPipeModule() ) {
            MUINT32 ratio = (RESIZE_RATIO_MAX_10X * 10);
            #if 1 // query width ratio from DRV
            NSCam::NSIoPipe::NSCamIOPipe::NormalPipe_QueryInfo info;
            pModule->query(
                NSCam::NSIoPipe::PORT_RRZO.index,
                NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd_BS_RATIO,
                (EImageFormat)imageFormat, 0, info);
            ratio = info.bs_ratio;
            #endif
            if (((MUINT32)src_crop_w * ratio) > ((MUINT32)bufferSize.w * 100))
            {
                MY_LOGW("calculateCropInfoResizer re-size width invalid "
                    "(%d):(%d) @(%d)", src_crop_w, bufferSize.w, ratio);
                return MFALSE;
            }
        }
        #endif
        MUINT32 ratio = getResizeMaxRatio(imageFormat);
        if (((MUINT32)src_crop_w * ratio) > ((MUINT32)bufferSize.w * 100)) {
            MY_LOGW("calculateCropInfoResizer re-size width invalid "
                "(%d):(%d) @(%d)", src_crop_w, bufferSize.w, ratio);
            return MFALSE;
        }
        dst_size_w = bufferSize.w;
        MY_LOGI_IF((3 <= mLogLevelI),
            "CheckXW Crop>Buf(%d>%d) Res-Src:X(%d):W(%d)-Dst:W(%d) SensorW(%d)",
            querySrcRect.size().w, bufferSize.w,
            src_crop_x, src_crop_w, dst_size_w, sensorSize.w);
    }
    // check Y and H
    if (querySrcRect.size().h < bufferSize.h) {
        dst_size_h = querySrcRect.size().h;
        dst_size_h = MIN(ALIGN_UPPER(dst_size_h, 2), sensorSize.h);
        src_crop_h = dst_size_h;
        if (src_crop_h > querySrcRect.size().h) {
            if ((src_crop_y + src_crop_h) > sensorSize.h) {
                MPoint::value_type cur_src_crop_y = src_crop_y;
                src_crop_y = sensorSize.h - src_crop_h;
                MY_LOGI_IF((2 <= mLogLevelI), "src_crop_y(%d):(%d) "
                    "sensor_h(%d) - src_crop_h(%d)", cur_src_crop_y, src_crop_y,
                    sensorSize.h, src_crop_h);
            }
        }
        MY_LOGI_IF((3 <= mLogLevelI),
            "CheckYH Crop<Buf(%d<%d) Res-Src:Y(%d):H(%d)-Dst:H(%d) SensorH(%d)",
            querySrcRect.size().h, bufferSize.h,
            src_crop_y, src_crop_h, dst_size_h, sensorSize.h);
    } else {
        #if 0
        if ( auto pModule = getNormalPipeModule() ) {
            MUINT32 ratio = (RESIZE_RATIO_MAX_10X * 10);
            #if 1 // query height ratio from DRV
            NSCam::NSIoPipe::NSCamIOPipe::NormalPipe_QueryInfo info;
            pModule->query(
                NSCam::NSIoPipe::PORT_RRZO.index,
                NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd_BS_RATIO,
                (EImageFormat)imageFormat, 0, info);
            ratio = info.bs_ratio;
            #endif
            if (((MUINT32)src_crop_h * ratio) > ((MUINT32)bufferSize.h * 100))
            {
                MY_LOGW("calculateCropInfoResizer re-size height invalid "
                    "(%d):(%d) @(%d)", src_crop_h, bufferSize.h, ratio);
                return MFALSE;
            }
        }
        #endif
        MUINT32 ratio = getResizeMaxRatio(imageFormat);
        if (((MUINT32)src_crop_h * ratio) > ((MUINT32)bufferSize.h * 100)) {
            MY_LOGW("calculateCropInfoResizer re-size height invalid "
                "(%d):(%d) @(%d)", src_crop_h, bufferSize.h, ratio);
            return MFALSE;
        }
        dst_size_h = bufferSize.h;
        MY_LOGI_IF((3 <= mLogLevelI),
            "CheckYH Crop>Buf(%d>%d) Res-Src:Y(%d):H(%d)-Dst:H(%d) SensorH(%d)",
            querySrcRect.size().h, bufferSize.h,
            src_crop_y, src_crop_h, dst_size_h, sensorSize.h);
    }
    resultDstSize = MSize(dst_size_w, dst_size_h);
    resultSrcRect = MRect(MPoint(src_crop_x, src_crop_y),
                            MSize(src_crop_w, src_crop_h));
    MY_LOGI_IF((2 <= mLogLevelI), "Result-Resize SrcRect" P1_RECT_STR "DstSize"
        P1_SIZE_STR, P1_RECT_VAR(resultSrcRect), P1_SIZE_VAR(resultDstSize));
    return MTRUE;
};

/******************************************************************************
 *
 ******************************************************************************/
MBOOL calculateCropInfoYuvResizer(
    MUINT32 pixelMode,
    MUINT32 imageFormat,
    MSize const& sensorSize,
    MSize const& bufferSize,
    MRect const& querySrcRect, // yuv crz src crop
    MRect& resultSrcRect,
    MSize& resultDstSize,
    MINT32 selectCrz,
    MINT32 mLogLevelI
)
{
    MBOOL bSkip = MFALSE;
    MBOOL bReject = MFALSE;

    if ((querySrcRect.size().w == sensorSize.w) &&
        (querySrcRect.size().h == sensorSize.h)) {
        MY_LOGI_IF((2 <= mLogLevelI), "No need to calculate");
        bSkip = MTRUE;
    } else if (
        (((querySrcRect.leftTop().x + querySrcRect.size().w) > sensorSize.w) ||
        ((querySrcRect.leftTop().y + querySrcRect.size().h) > sensorSize.h))
    ) {
        MY_LOGI_IF((2 <= mLogLevelI), "Input need to check");
        bReject = MTRUE;
    }
    MY_LOGI_IF((bReject) || (3 <= mLogLevelI) || ((2 <= mLogLevelI) && bSkip),
        "[CropInfo] Input pixelMode(%d) imageFormat(0x%x)"
        " sensorSize" P1_SIZE_STR "bufferSize" P1_SIZE_STR
        "querySrcRect" P1_RECT_STR,
        pixelMode, imageFormat,
        P1_SIZE_VAR(sensorSize), P1_SIZE_VAR(bufferSize),
        P1_RECT_VAR(querySrcRect));
    if (bSkip) {
        return MTRUE;
    } else if (bReject) {
        return MFALSE;
    }
    //
    MPoint::value_type src_crop_x = querySrcRect.leftTop().x;
    MPoint::value_type src_crop_y = querySrcRect.leftTop().y;
    MSize::value_type src_crop_w = querySrcRect.size().w;
    MSize::value_type src_crop_h = querySrcRect.size().h;
    MSize::value_type dst_size_w = 0;
    MSize::value_type dst_size_h = 0;
    // check X and W
    if (querySrcRect.size().w < bufferSize.w) {
        dst_size_w = querySrcRect.size().w;
        // check start.x
        if  ( auto pModule = getNormalPipeModule() )
        {
            NSCam::NSIoPipe::NSCamIOPipe::NormalPipe_QueryInfo info;
            pModule->query(
                (selectCrz == 1) ?
                (NSCam::NSIoPipe::PORT_CRZO_R1.index):
                (NSCam::NSIoPipe::PORT_CRZO_R2.index),
                NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd_CROP_START_X,
                (EImageFormat)imageFormat,
                src_crop_x, info);
            if ((MUINT32)src_crop_x != info.crop_x) {
                MY_LOGI_IF((2 <= mLogLevelI), "src_crop_x(%d) info.crop_x(%d)",
                    src_crop_x, info.crop_x);
            }
            src_crop_x = info.crop_x;
        }
        // check size.w
        if  ( auto pModule = getNormalPipeModule() )
        {
            NSCam::NSIoPipe::NSCamIOPipe::NormalPipe_QueryInfo info;
            pModule->query(
                (selectCrz == 1) ?
                (NSCam::NSIoPipe::PORT_CRZO_R1.index):
                (NSCam::NSIoPipe::PORT_CRZO_R2.index),
                NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd_X_PIX|
                NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd_STRIDE_BYTE,
                (EImageFormat)imageFormat,
                dst_size_w, info);
            if ((MUINT32)dst_size_w != info.x_pix) {
                MY_LOGI_IF((2 <= mLogLevelI), "dst_size_w(%d) info.x_pix(%d)",
                    dst_size_w, info.x_pix);
            }
            dst_size_w = info.x_pix;
        }
        //
        MSize::value_type cur_src_crop_x = src_crop_x;
        MSize::value_type cur_src_crop_w = src_crop_w;
        MSize::value_type cur_dst_size_w = dst_size_w;
        dst_size_w = MIN(dst_size_w, sensorSize.w);
        src_crop_w = dst_size_w;
        if (src_crop_w > querySrcRect.size().w) {
            if ((src_crop_x + src_crop_w) > sensorSize.w) {
                src_crop_x = sensorSize.w - src_crop_w;
            }
        }
        if ((cur_src_crop_x != src_crop_x) || (cur_src_crop_w != src_crop_w)
            || (cur_dst_size_w != dst_size_w)) {
            MY_LOGI_IF((2 <= mLogLevelI), "ValueChanged-XW src_crop_x(%d):(%d) "
                "src_crop_w(%d):(%d) dst_size_w(%d):(%d) sensor_w(%d)",
                cur_src_crop_x, src_crop_x, cur_src_crop_w, src_crop_w,
                cur_dst_size_w, dst_size_w, sensorSize.w);
        }
        MY_LOGI_IF((3 <= mLogLevelI),
            "CheckXW Crop<Buf(%d<%d) Res-Src:X(%d):W(%d)-Dst:W(%d) SensorW(%d)",
            querySrcRect.size().w, bufferSize.w,
            src_crop_x, src_crop_w, dst_size_w, sensorSize.w);
    } else {
        dst_size_w = bufferSize.w;
        MY_LOGI_IF((3 <= mLogLevelI),
            "CheckXW Crop>Buf(%d>%d) Res-Src:X(%d):W(%d)-Dst:W(%d) SensorW(%d)",
            querySrcRect.size().w, bufferSize.w,
            src_crop_x, src_crop_w, dst_size_w, sensorSize.w);
    }
    // check Y and H
    if (querySrcRect.size().h < bufferSize.h) {
        dst_size_h = querySrcRect.size().h;
        dst_size_h = MIN(ALIGN_UPPER(dst_size_h, 2), sensorSize.h);
        src_crop_h = dst_size_h;
        if (src_crop_h > querySrcRect.size().h) {
            if ((src_crop_y + src_crop_h) > sensorSize.h) {
                MPoint::value_type cur_src_crop_y = src_crop_y;
                src_crop_y = sensorSize.h - src_crop_h;
                MY_LOGI_IF((2 <= mLogLevelI), "src_crop_y(%d):(%d) "
                    "sensor_h(%d) - src_crop_h(%d)", cur_src_crop_y, src_crop_y,
                    sensorSize.h, src_crop_h);
            }
        }
        MY_LOGI_IF((3 <= mLogLevelI),
            "CheckYH Crop<Buf(%d<%d) Res-Src:Y(%d):H(%d)-Dst:H(%d) SensorH(%d)",
            querySrcRect.size().h, bufferSize.h,
            src_crop_y, src_crop_h, dst_size_h, sensorSize.h);
    } else {
        dst_size_h = bufferSize.h;
        MY_LOGI_IF((3 <= mLogLevelI),
            "CheckYH Crop>Buf(%d<%d) Res-Src:Y(%d):H(%d)-Dst:H(%d) SensorH(%d)",
            querySrcRect.size().h, bufferSize.h,
            src_crop_y, src_crop_h, dst_size_h, sensorSize.h);
    }
    resultDstSize = MSize(dst_size_w, dst_size_h);
    resultSrcRect = MRect(MPoint(src_crop_x, src_crop_y),
                            MSize(src_crop_w, src_crop_h));
    MY_LOGI_IF((2 <= mLogLevelI), "Result-Resize SrcRect" P1_RECT_STR "DstSize"
        P1_SIZE_STR, P1_RECT_VAR(resultSrcRect), P1_SIZE_VAR(resultDstSize));
    return MTRUE;


}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL verifySizeResizer(
    MUINT32 pixelMode,
    MUINT32 imageFormat,
    MSize const& sensorSize,
    MSize const& streamBufSize,
    MSize const& queryBufSize,
    MSize& resultBufSize,
    MINT32 mLogLevelI
)
{
    MY_LOGI_IF((3 <= mLogLevelI),
        "[CropInfo] +++ pixelMode(%d) imageFormat(0x%x)" " sensor" P1_SIZE_STR
        "streamBuf" P1_SIZE_STR "queryBuf" P1_SIZE_STR
        "resultBuf" P1_SIZE_STR,
        pixelMode, imageFormat, P1_SIZE_VAR(sensorSize),
        P1_SIZE_VAR(streamBufSize), P1_SIZE_VAR(queryBufSize),
        P1_SIZE_VAR(resultBufSize));
    //
    resultBufSize = streamBufSize;
    //
    // check origin stream buffer size
    if (queryBufSize.w > streamBufSize.w || queryBufSize.h > streamBufSize.h) {
        MY_LOGW("[CropInfo] MTK_P1NODE_RESIZER_SET_SIZE" P1_SIZE_STR " > "
            "STREAM_BUF_SIZE" P1_SIZE_STR
            " : ignore-MTK_P1NODE_RESIZER_SET_SIZE"
            " use-stream_buffer_size" P1_SIZE_STR,
            P1_SIZE_VAR(queryBufSize), P1_SIZE_VAR(streamBufSize),
            P1_SIZE_VAR(streamBufSize));
        return MFALSE;
    }
    //
    // check size.w and size.h should be even
    if (((((MUINT32)queryBufSize.w) & ((MUINT32)0x1)) > 0) ||
        ((((MUINT32)queryBufSize.h) & ((MUINT32)0x1)) > 0)) {
        MY_LOGW("[CropInfo] MTK_P1NODE_RESIZER_SET_SIZE" P1_SIZE_STR " != Even"
            " : ignore-MTK_P1NODE_RESIZER_SET_SIZE"
            " use-stream_buffer_size" P1_SIZE_STR,
            P1_SIZE_VAR(queryBufSize),
            P1_SIZE_VAR(streamBufSize));
        return MFALSE;
    }
    //
    // check size.w alignment limitation
    if  ( auto pModule = getNormalPipeModule() )
    {
        MSize::value_type dst_size_w = queryBufSize.w;
        NSCam::NSIoPipe::NSCamIOPipe::NormalPipe_QueryInfo info;
        pModule->query(
            NSCam::NSIoPipe::PORT_RRZO.index,
            NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd_X_PIX|
            NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd_STRIDE_BYTE,
            (EImageFormat)imageFormat,
            dst_size_w, info);
        if ((MUINT32)dst_size_w != info.x_pix) {
            MY_LOGW("[CropInfo] MTK_P1NODE_RESIZER_SET_SIZE" P1_SIZE_STR
                " size_w(%d) != x_pix(%d)"
                " : ignore-MTK_P1NODE_RESIZER_SET_SIZE"
                " use-stream_buffer_size" P1_SIZE_STR,
                P1_SIZE_VAR(queryBufSize), dst_size_w, info.x_pix,
                P1_SIZE_VAR(streamBufSize));
            return MFALSE;
        }
    }
    //
    #if 1 // about the limitation, check ratio with source-crop-size in the following calculateCropInfo (not sensor-size)
    // check size.w and size.h ratio limitation
    {
        MUINT32 ratio = getResizeMaxRatio(imageFormat);
        if ((((MUINT32)queryBufSize.w * 100) <
            ((MUINT32)sensorSize.w * ratio)) ||
            (((MUINT32)queryBufSize.h * 100) <
            ((MUINT32)sensorSize.h * ratio))) {
            MY_LOGW("[CropInfo] MTK_P1NODE_RESIZER_SET_SIZE" P1_SIZE_STR " < "
                "SensorSize" P1_SIZE_STR "x Ratio(0.%d) "
                " : ignore-MTK_P1NODE_RESIZER_SET_SIZE"
                " use-stream_buffer_size" P1_SIZE_STR,
                P1_SIZE_VAR(queryBufSize), P1_SIZE_VAR(sensorSize), ratio,
                P1_SIZE_VAR(streamBufSize));
            return MFALSE;
        }
    }
    #endif
    //
    resultBufSize = queryBufSize;
    MY_LOGI_IF((3 <= mLogLevelI),
        "[CropInfo] --- pixelMode(%d) imageFormat(0x%x)" " sensor" P1_SIZE_STR
        "streamBuf" P1_SIZE_STR "queryBuf" P1_SIZE_STR
        "resultBuf" P1_SIZE_STR,
        pixelMode, imageFormat, P1_SIZE_VAR(sensorSize),
        P1_SIZE_VAR(streamBufSize), P1_SIZE_VAR(queryBufSize),
        P1_SIZE_VAR(resultBufSize));
    return MTRUE;
};

/******************************************************************************
 *
 ******************************************************************************/
MBOOL verifySizeYuvResizer(
    MUINT32 pixelMode,
    MUINT32 imageFormat,
    MSize const& sensorSize,
    MSize const& streamBufSize,
    MSize const& queryBufSize, // FIXME
    MBOOL const& selectCrz,
    MSize& resultBufSize,
    MINT32 mLogLevelI
)
{
    MY_LOGI_IF((3 <= mLogLevelI),
        "[CropInfo] +++ pixelMode(%d) imageFormat(0x%x)" " sensor" P1_SIZE_STR
        "streamBuf" P1_SIZE_STR "queryBuf" P1_SIZE_STR
        "resultBuf" P1_SIZE_STR,
        pixelMode, imageFormat, P1_SIZE_VAR(sensorSize),
        P1_SIZE_VAR(streamBufSize), P1_SIZE_VAR(queryBufSize),
        P1_SIZE_VAR(resultBufSize));
    //
    resultBufSize = streamBufSize;
    //
    // check origin stream buffer size
    if (queryBufSize.w > streamBufSize.w || queryBufSize.h > streamBufSize.h) {
        MY_LOGW("[CropInfo] MTK_P1NODE_RESIZER_SET_SIZE" P1_SIZE_STR " > "
            "STREAM_BUF_SIZE" P1_SIZE_STR
            " : ignore-MTK_P1NODE_RESIZER_SET_SIZE"
            " use-stream_buffer_size" P1_SIZE_STR,
            P1_SIZE_VAR(queryBufSize), P1_SIZE_VAR(streamBufSize),
            P1_SIZE_VAR(streamBufSize));
        return MFALSE;
    }
    //
    // check size.w and size.h should be even
    if (((((MUINT32)queryBufSize.w) & ((MUINT32)0x1)) > 0) ||
        ((((MUINT32)queryBufSize.h) & ((MUINT32)0x1)) > 0)) {
        MY_LOGW("[CropInfo] MTK_P1NODE_RESIZER_SET_SIZE" P1_SIZE_STR " != Even"
            " : ignore-MTK_P1NODE_RESIZER_SET_SIZE"
            " use-stream_buffer_size" P1_SIZE_STR,
            P1_SIZE_VAR(queryBufSize),
            P1_SIZE_VAR(streamBufSize));
        return MFALSE;
    }
    //
    // check size.w alignment limitation
    if  ( auto pModule = getNormalPipeModule() )
    {
        MSize::value_type dst_size_w = queryBufSize.w;
        NSCam::NSIoPipe::NSCamIOPipe::NormalPipe_QueryInfo info;
        pModule->query(
            (selectCrz) ? (NSCam::NSIoPipe::PORT_CRZO_R1.index):(NSCam::NSIoPipe::PORT_CRZO_R2.index),
            NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd_X_PIX|
            NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd_STRIDE_BYTE,
            (EImageFormat)imageFormat,
            dst_size_w, info);
        if ((MUINT32)dst_size_w != info.x_pix) {
            MY_LOGW("[CropInfo] MTK_P1NODE_RESIZER_SET_SIZE" P1_SIZE_STR
                " size_w(%d) != x_pix(%d)"
                " : ignore-MTK_P1NODE_RESIZER_SET_SIZE"
                " use-stream_buffer_size" P1_SIZE_STR,
                P1_SIZE_VAR(queryBufSize), dst_size_w, info.x_pix,
                P1_SIZE_VAR(streamBufSize));
            return MFALSE;
        }
    }

    //
    resultBufSize = queryBufSize;
    MY_LOGI_IF((3 <= mLogLevelI),
        "[CropInfo] --- pixelMode(%d) imageFormat(0x%x)" " sensor" P1_SIZE_STR
        "streamBuf" P1_SIZE_STR "queryBuf" P1_SIZE_STR
        "resultBuf" P1_SIZE_STR,
        pixelMode, imageFormat, P1_SIZE_VAR(sensorSize),
        P1_SIZE_VAR(streamBufSize), P1_SIZE_VAR(queryBufSize),
        P1_SIZE_VAR(resultBufSize));
    return MTRUE;
};


/******************************************************************************
 *
 ******************************************************************************/
void
queryRollingSkew(
    MUINT const openId,
    MUINT32 const sensorMode,
    MUINT32 & nsRolling,
    MINT32 mLogLevelI
)
{
    IHalSensor* pSensorHalObj = NULL;
    IHalSensorList* const pHalSensorList = MAKE_HalSensorList();
    MUINT32 rolling = P1_SENSOR_ROLLING_SKEW_UNKNOWN;
    MUINT32 mode = sensorMode;
    nsRolling = rolling;
    //
    U_if (!pHalSensorList) {
        MY_LOGE("[Cam::%d] HalSensorList == NULL", openId);
        return;
    }
    pSensorHalObj = pHalSensorList->createSensor(LOG_TAG, openId);
    U_if (pSensorHalObj == NULL) {
        MY_LOGE("[Cam::%d] SensorHalObj is NULL", openId);
        return;
    }
    //
    MINT res = pSensorHalObj->sendCommand(
        pHalSensorList->querySensorDevIdx(openId),
        SENSOR_CMD_GET_SENSOR_ROLLING_SHUTTER,
        (MUINTPTR)(&rolling), (MUINTPTR)(&mode), (MUINTPTR)0);
    pSensorHalObj->destroyInstance(LOG_TAG);
    //
    if (res < 0) {
        MY_LOGI("[Cam::%d] mode(%d) SENSOR_CMD_GET_SENSOR_ROLLING_SHUTTER"
            " not support", openId, sensorMode);
        return;
    } else {
        if (rolling == P1_SENSOR_ROLLING_SKEW_UNKNOWN) {
            MY_LOGI("[Cam::%d] mode(%d) SENSOR_CMD_GET_SENSOR_ROLLING_SHUTTER"
                " result 0", openId, sensorMode);
        } else {
            nsRolling = rolling;
            MY_LOGI_IF((2 <= mLogLevelI),
                "[Cam::%d] mode(%d) SENSOR_CMD_GET_SENSOR_ROLLING_SHUTTER"
                " is %d(ns)", openId, sensorMode, nsRolling);
        }
    }
    return;
};


/******************************************************************************
 *
 ******************************************************************************/
void
queryStartExpTs(
    MUINT const openId,
    MUINT32 const sensorMode,
    MINT64 const & nsExpDuration,
    MINT64 const & nsStartFrm,
    MINT64 & nsStartExp,
    MINT32 mLogLevelI
)
{
    IHalSensor* pSensorHalObj = NULL;
    IHalSensorList* const pHalSensorList = MAKE_HalSensorList();
    MINT64 data = 0;
    MUINT32 mode = sensorMode;
    nsStartExp = data;
    data = nsExpDuration; // for arg3
    //
    U_if (!pHalSensorList) {
        MY_LOGE("[Cam::%d] HalSensorList == NULL", openId);
        return;
    }
    pSensorHalObj = pHalSensorList->createSensor(LOG_TAG, openId);
    U_if (pSensorHalObj == NULL) {
        MY_LOGE("[Cam::%d] SensorHalObj is NULL", openId);
        return;
    }
    //arg1, pointer of variable of scenario id,
    //arg2, pointer of variable of tSof,
    //arg3, pointer of variable of exposure time, and return the value of tSoE
    MINT res = pSensorHalObj->sendCommand(
        pHalSensorList->querySensorDevIdx(openId),
        SENSOR_CMD_GET_START_OF_EXPOSURE,
        (MUINTPTR)(&mode), (MUINTPTR)(&nsStartFrm), (MUINTPTR)(&data));
    pSensorHalObj->destroyInstance(LOG_TAG);
    //
    if (res < 0) {
        MY_LOGI_IF((2 <= mLogLevelI),
            "[Cam::%d] mode(%d) SENSOR_CMD_GET_START_OF_EXPOSURE not support"
            " (Result:%" PRId64 ") - (ExpDuration:%" PRId64 ")"
            " (StartFrame:%" PRId64 ")", openId, sensorMode, data,
            nsExpDuration, nsStartFrm);
        nsStartExp = 0; // not get from sensor
        return;
    } else {
        nsStartExp = data;
        MY_LOGI_IF((2 <= mLogLevelI),
            "[Cam::%d] mode(%d) SENSOR_CMD_GET_START_OF_EXPOSURE success"
            " (Result:%" PRId64 ") - (ExpDuration:%" PRId64 ")"
            " (StartFrame:%" PRId64 ")", openId, sensorMode, data,
            nsExpDuration, nsStartFrm);
    }
    return;
};


/******************************************************************************
 *
 ******************************************************************************/
void
generateMetaInfoStr(
    IMetadata::IEntry const & entry,
    android::String8 & string
)
{
    string.appendFormat("[TAG:0x%X _%d #%d]={ ",
        entry.tag(), entry.type(), entry.count());

    typedef IMetadata::Memory Memory;

    #undef P1_FMT_MUINT8
    #undef P1_FMT_MINT32
    #undef P1_FMT_MINT64
    #undef P1_FMT_MFLOAT
    #undef P1_FMT_MDOUBLE
    #undef P1_FMT_MPoint
    #undef P1_FMT_MSize
    #undef P1_FMT_MRect
    #undef P1_FMT_MRational
    #undef P1_META_CASE_STR

    #define P1_FMT_MUINT8(v)    "%d ", v
    #define P1_FMT_MINT32(v)    "%d ", v
    #define P1_FMT_MINT64(v)    "%" PRId64 " ", v
    #define P1_FMT_MFLOAT(v)    "%f ", v
    #define P1_FMT_MDOUBLE(v)   "%lf ", v
    #define P1_FMT_MPoint(v)    "%d,%d ", v.x, v.y
    #define P1_FMT_MSize(v)     "%dx%d ", v.w, v.h
    #define P1_FMT_MRect(v)     "%d,%d_%dx%d ", v.p.x, v.p.y, v.s.w, v.s.h
    #define P1_FMT_MRational(v) "%d:%d ", v.numerator, v.denominator
    #define P1_FMT_Memory(v)    "[%zu] ", v.size()
    #define P1_META_CASE_STR(T) \
        case TYPE_##T:\
            {\
                for (size_t i = 0; i < entry.count(); i++) {\
                    T value = entry.itemAt(i, Type2Type< T >() );\
                    string.appendFormat(P1_FMT_##T(value));\
                }\
            };\
            break;

    switch (entry.type()) {
        P1_META_CASE_STR(MUINT8);
        P1_META_CASE_STR(MINT32);
        P1_META_CASE_STR(MINT64);
        P1_META_CASE_STR(MFLOAT);
        P1_META_CASE_STR(MDOUBLE);
        P1_META_CASE_STR(MPoint);
        P1_META_CASE_STR(MSize);
        P1_META_CASE_STR(MRect);
        P1_META_CASE_STR(MRational);
        P1_META_CASE_STR(Memory);
        case TYPE_IMetadata:
            string.appendFormat("metadata ... ");
            break;
        default:
            string.appendFormat("UNKNOWN_%d", entry.type());
            break;
    };

    #undef P1_FMT_MUINT8
    #undef P1_FMT_MINT32
    #undef P1_FMT_MINT64
    #undef P1_FMT_MFLOAT
    #undef P1_FMT_MDOUBLE
    #undef P1_FMT_MPoint
    #undef P1_FMT_MSize
    #undef P1_FMT_MRect
    #undef P1_FMT_MRational
    #undef P1_META_CASE_STR

    string.appendFormat("} ");

}


/******************************************************************************
 *
 ******************************************************************************/
void
logMeta(
    MINT32 option,
    IMetadata const * pMeta,
    char const * pInfo,
    MUINT32 tag
)
{
    if (option <= 0) {
        return;
    }
    if ((pMeta == NULL) || (pInfo == NULL)) {
        return;
    }
    MUINT32 numPerLine = option;
    MUINT32 cnt = 0;
    MUINT32 end = 0;
    MBOOL found = MFALSE;
    android::String8 str("");
    if (pMeta->count() == 0) {
        str.clear();
        str += String8::format("%s metadata.count(0)", pInfo);
        if (tag > 0) {
            str += String8::format(" - while find MetaTag[0x%X=%d]", tag, tag);
        }
        MY_LOGI("%s", str.string());
        str.clear();
        return;
    }
    for (MUINT32 i = 0; i < pMeta->count(); i++) {
        if (tag != 0) {
            if (tag == pMeta->entryAt(i).tag()) {
                found = MTRUE;
                str.clear();
                str += String8::format("%s Found-MetaTag[0x%X=%d] ",
                    pInfo, pMeta->entryAt(i).tag(), tag);
                generateMetaInfoStr(pMeta->entryAt(i), str);
                MY_LOGI("%s", str.string());
                break;
            }
            continue;
        }
        //
        if (cnt == 0) {
            end = ((i + numPerLine - 1) < (pMeta->count() -1)) ?
                (i + numPerLine - 1) : (pMeta->count() -1);
            str.clear();
            str += String8::format("%s [%03d~%03d/%03d] ",
                pInfo, i, end, pMeta->count());
        }
        generateMetaInfoStr(pMeta->entryAt(i), str);
        cnt++;
        if (i == end) {
            cnt = 0;
            MY_LOGI("%s", str.string());
        }
    }
    if ((tag != 0) && (!found)) {
        MY_LOGI("%s NotFound-MetaTag[0x%X=%d]", pInfo, tag, tag);
    }
    str.clear();
    return;
}


//
#if 1
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  StuffBufferPool Implementation
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

/******************************************************************************
 *
 ******************************************************************************/
MBOOL
StuffBufferPool::compareLayout(
    MINT32 format, MSize size,
    MUINT32 stride0, MUINT32 stride1, MUINT32 stride2)
{
    return ((format == mFormat) &&
            (stride0 == mStride0) &&
            (stride1 == mStride1) &&
            (stride2 == mStride2) &&
            (size == mSize));
};

/******************************************************************************
 *
 ******************************************************************************/
MERROR
StuffBufferPool::
acquireBuffer(sp<IImageBuffer> & imageBuffer)
{
    FUNCTION_IN;
    //
    MERROR ret = OK;
    sp<IImageBuffer> pImgBuf = NULL;
    BufNote bufNote;
    size_t i = 0;
    imageBuffer = NULL;
    //
    for (i = 0; i < mvInfoMap.size(); i++) {
        bufNote = mvInfoMap.editValueAt(i);
        if (BUF_STATE_RELEASED == bufNote.mState) {
            sp<IImageBuffer> const pImageBuffer = mvInfoMap.keyAt(i);
            bufNote.mState = BUF_STATE_ACQUIRED;
            mvInfoMap.replaceValueAt(i, bufNote);
            pImgBuf = pImageBuffer;
            break;
        }
    }
    //
    if (pImgBuf != NULL) {
        MY_LOGD2(
            "StuffBuffer-Acquire (%s) index(%zu) (%zu/%d:%d)",
            bufNote.msName.string(), i, mvInfoMap.size(), mWaterMark,
            mMaxAmount);
        mUsage |= GRALLOC_USAGE_SW_WRITE_OFTEN;
        mUsage |= GRALLOC_USAGE_SW_READ_OFTEN;
        if (!(pImgBuf->lockBuf(bufNote.msName.string(), mUsage))) {
            MY_LOGE("[%s] Stuff ImgBuf lock fail", bufNote.msName.string());
            return BAD_VALUE;
        }
        imageBuffer = pImgBuf;
        return OK;
    }
    //
    MY_LOGD2("StuffBuffer-Acquire (NoAvailable) (%zu/%d)",
            mvInfoMap.size(), mWaterMark);
    //
    ret = createBuffer(imageBuffer);
    FUNCTION_OUT;
    return ret;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
StuffBufferPool::
releaseBuffer(sp<IImageBuffer> & imageBuffer)
{
    FUNCTION_IN;
    //
    MERROR ret = OK;
    if (imageBuffer == NULL) {
        MY_LOGW("Stuff ImageBuffer not exist");
        return BAD_VALUE;
    }
    ssize_t index = mvInfoMap.indexOfKey(imageBuffer);
    if (index < 0 || (size_t)index >= mvInfoMap.size()) {
        MY_LOGW("ImageBuffer(%p) not found (%zd/%zu)",
            imageBuffer.get(), index, mvInfoMap.size());
        return BAD_VALUE;
    }
    imageBuffer->unlockBuf(mvInfoMap.valueAt(index).msName.string());
    BufNote bufNote = mvInfoMap.editValueAt(index);
    bufNote.mState = BUF_STATE_RELEASED;
    mvInfoMap.replaceValueAt(index, bufNote);
    //
    if (mvInfoMap.size() >= mWaterMark) {
        ret = destroyBuffer(index);
    }
    //
    MY_LOGD2("StuffBuffer-Release (%s) index(%zu) (%zu/%d:%d)",
        bufNote.msName.string(), index, mvInfoMap.size(), mWaterMark,
        mMaxAmount);
    //
    FUNCTION_OUT;
    return ret;
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
StuffBufferPool::
createBuffer(sp<IImageBuffer> & imageBuffer)
{
    FUNCTION_IN;
    //
    imageBuffer = NULL;
    sp<IIonImageBufferHeap> pHeap = NULL;
    // for secure cam
    sp<ISecureImageBufferHeap> pSecHeap = NULL;

    sp<IImageBuffer> pImgBuf = NULL;

    // add information to buffer name
    android::String8 imgBufName = android::String8(msName);
    char str[256] = {0};
    snprintf(str, sizeof(str), ":Size%dx%d:Stride%d.%d.%d:Sn%d",
        mSize.w, mSize.h, mStride0, mStride1, mStride2, ++mSerialNum);
    imgBufName += str;
    //
    if (mvInfoMap.size() >= mMaxAmount) {
        MY_LOGW("[%s] the pool size is over max amount, "
            "please check the buffer usage and situation (%zu/%d)",
            imgBufName.string(), mvInfoMap.size(), mMaxAmount);
        return NO_MEMORY;
    }
    // create buffer
    MINT32 bufBoundaryInBytes[3] = {0, 0, 0};
    MUINT32 bufStridesInBytes[3] = {mStride0, mStride1, mStride2};
    if (mPlaneCnt == 0) { // ref. StuffBufferPool::CTR mStride0/1/2 checking
        MY_LOGE("[%s] Stuff ImageBufferHeap stride invalid (%d.%d.%d)",
            imgBufName.string(), mStride0, mStride1, mStride2);
        return BAD_VALUE;
    }
    IImageBufferAllocator::ImgParam imgParam =
        IImageBufferAllocator::ImgParam((EImageFormat)mFormat,
                mSize, bufStridesInBytes, bufBoundaryInBytes, (size_t)mPlaneCnt);
    if (!mbSecure) {
        pHeap =
            IIonImageBufferHeap::create(imgBufName.string(), imgParam);
        if (pHeap == NULL) {
            MY_LOGE("[%s] Stuff ImageBufferHeap create fail", imgBufName.string());
            return BAD_VALUE;
        }
        MINT reqImgFormat = pHeap->getImgFormat();
        ImgBufCreator creator(reqImgFormat);
        pImgBuf = pHeap->createImageBuffer(&creator);

    } else {
        pSecHeap =
            ISecureImageBufferHeap::create(imgBufName.string(), imgParam,
                ISecureImageBufferHeap::AllocExtraParam(0, 1, 0, MFALSE, mSecType));
        if (pSecHeap == NULL) {
            MY_LOGE("[%s] Stuff ImageBufferHeap create fail", imgBufName.string());
            return BAD_VALUE;
        }
        MINT reqImgFormat = pSecHeap->getImgFormat();
        ImgBufCreator creator(reqImgFormat);
        pImgBuf = pSecHeap->createImageBuffer(&creator);
    }

    if (pImgBuf == NULL) {
        MY_LOGE("[%s] Stuff ImageBuffer create fail", imgBufName.string());
        return BAD_VALUE;
    }
    // lock buffer
    mUsage |= GRALLOC_USAGE_SW_WRITE_OFTEN;
    mUsage |= GRALLOC_USAGE_SW_READ_OFTEN;
    if (!(pImgBuf->lockBuf(imgBufName.string(), mUsage))) {
        MY_LOGE("[%s] Stuff ImageBuffer lock fail", imgBufName.string());
        return BAD_VALUE;
    }
    BufNote bufNote(imgBufName, BUF_STATE_ACQUIRED);
    mvInfoMap.add(pImgBuf, bufNote);
    imageBuffer = pImgBuf;
    //
    MY_LOGD2("StuffBuffer-Create (%s) (%zu/%d:%d) "
        "ImgBuf(%p)(0x%X)(%dx%d,%zu,%zu)(P:0x%zx)(V:0x%zx) (isSecure:%d)",
        imgBufName.string(), mvInfoMap.size(), mWaterMark, mMaxAmount,
        imageBuffer.get(), imageBuffer->getImgFormat(),
        imageBuffer->getImgSize().w, imageBuffer->getImgSize().h,
        imageBuffer->getBufStridesInBytes(0), imageBuffer->getBufSizeInBytes(0),
        imageBuffer->getBufPA(0), imageBuffer->getBufVA(0), mbSecure);
    //
    FUNCTION_OUT;
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
StuffBufferPool::
destroyBuffer(sp<IImageBuffer> & imageBuffer)
{
    FUNCTION_IN;
    //
    MERROR ret = OK;
    if (imageBuffer == NULL) {
        MY_LOGW("Stuff ImageBuffer not exist");
        return BAD_VALUE;
    }
    //
    ssize_t index = mvInfoMap.indexOfKey(imageBuffer);
    if (index < 0 || (size_t)index >= mvInfoMap.size()) {
        MY_LOGW("ImageBuffer(%p) not found (%zd/%zu)",
            imageBuffer.get(), index, mvInfoMap.size());
        return BAD_VALUE;
    }
    ret = destroyBuffer(index);
    FUNCTION_OUT;
    return ret;
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
StuffBufferPool::
destroyBuffer(size_t index)
{
    FUNCTION_IN;
    //
    if (index >= mvInfoMap.size()) {
        MY_LOGW("index(%zu) not exist, size(%zu)", index, mvInfoMap.size());
        return BAD_VALUE;
    }
    BufNote bufNote = mvInfoMap.valueAt(index);
    sp<IImageBuffer> const pImageBuffer = mvInfoMap.keyAt(index);
    MY_LOGD2("StuffBuffer-Destroy (%s) index(%zu) state(%d) "
        "(%zu/%d:%d)", bufNote.msName.string(), index, bufNote.mState,
        mvInfoMap.size(), mWaterMark, mMaxAmount);
    if (bufNote.mState == BUF_STATE_ACQUIRED) {
        sp<IImageBuffer> pImgBuf = pImageBuffer;
        pImgBuf->unlockBuf(bufNote.msName.string());
    }
    // destroy buffer
    mvInfoMap.removeItemsAt(index);
    //pImgBuf = NULL;
    //
    FUNCTION_OUT;
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
StuffBufferManager::
collectBufferInfo(
    MUINT32 pixelMode,
    MBOOL isFull,
    MINT32 format,
    MSize size,
    std::vector< MUINT32 > & stride
)
{
    FUNCTION_IN;
    //
    Mutex::Autolock _l(mLock);
    //
    MY_LOGI3("pixel-mode-%d full:%d format[x%x] size(%dx%d)",
        pixelMode, isFull, format, size.w, size.h);
    MBOOL found = MFALSE;
    stride.clear();
    std::vector< InfoSet >::iterator it = mvInfoSet.begin();
    for(; it != mvInfoSet.end(); it++) {
        if ((it->mFormat == format) && (it->mSize == size)) {
            stride = it->mvStride;
            //for (size_t i = 0; i < stride.size(); i++)
            //    MY_LOGI1("Found Stride[%zu/%zu]=(%d)", i, stride.size(), stride[i]);
            found = MTRUE;
            break;
        }
    }
    if (found == MFALSE) { // add new InfoSet by the querying from DRV
        InfoSet addInfoSet(mOpenId, mLogLevel, mLogLevelI);
        addInfoSet.mFormat = format;
        addInfoSet.mSize = size; // save the size here, the size might be changed by HwInfoHelper
        //
        NSCamHW::HwInfoHelper helper(mOpenId);
        switch (format) { // for UFO case
            case eImgFmt_UFO_BAYER8:
            case eImgFmt_UFO_BAYER10:
            case eImgFmt_UFO_BAYER12:
            case eImgFmt_UFO_BAYER14:
            case eImgFmt_UFO_FG_BAYER8:
            case eImgFmt_UFO_FG_BAYER10:
            case eImgFmt_UFO_FG_BAYER12:
            case eImgFmt_UFO_FG_BAYER14:
                {
                    size_t ufoStride[3] = {0};
                    if (!helper.queryUFOStride(format, size, ufoStride)) {
                        MY_LOGE("QueryUFOStride - FAIL(%d-%d)[x%x](%dx%d)",
                            pixelMode, isFull, format, size.w, size.h);
                        return BAD_VALUE;
                    };
                    MY_LOGI0("add-BufInfoSet(%d)[%d][x%x](%dx%d)-"
                        "(%dx%d)(%zu,%zu,%zu)",
                        pixelMode, isFull, format, size.w, size.h,
                        addInfoSet.mSize.w, addInfoSet.mSize.h,
                        ufoStride[0], ufoStride[1], ufoStride[2]);
                    addInfoSet.mvStride.push_back(ufoStride[0]);
                    addInfoSet.mvStride.push_back(ufoStride[1]);
                    addInfoSet.mvStride.push_back(ufoStride[2]);
                }
                break;
            case eImgFmt_YV12: // 3-plane
                {
                    addInfoSet.mvStride.push_back(size.w);
                    addInfoSet.mvStride.push_back(size.w >> 1);
                    addInfoSet.mvStride.push_back(size.w >> 1);
                }
                break;
            case eImgFmt_NV21: // 2-plane
                {
                    addInfoSet.mvStride.push_back(size.w);
                    addInfoSet.mvStride.push_back(size.w);
                }
                break;
            case eImgFmt_YUY2: // 1-plane
                {
                    addInfoSet.mvStride.push_back(size.w << 1);
                }
                break;
            default: // IMGO/RRZO with non-UFO
                {
                    size_t stride = 0;
                    if (!helper.alignPass1HwLimitation(
                        pixelMode, format, isFull, size, stride)) {
                        MY_LOGE("QueryBufferInfo - FAIL(%d-%d)[x%x](%dx%d)",
                            pixelMode, isFull, format, size.w, size.h);
                        return BAD_VALUE;
                    } // not replace the size
                    MY_LOGI0("add-BufInfoSet(%d)[%d][x%x](%dx%d)-(%dx%d)(%zu)",
                        pixelMode, isFull, format, size.w, size.h,
                        addInfoSet.mSize.w, addInfoSet.mSize.h, stride);
                    if (size.w != addInfoSet.mSize.w) {
                        if ( auto pModule = getNormalPipeModule() ) {
                            NSCam::NSIoPipe::NSCamIOPipe::NormalPipe_QueryInfo
                                queryRst;
                            NSCam::NSIoPipe::NSCamIOPipe::NormalPipe_QueryIn
                                input;
                            input.width = addInfoSet.mSize.w;
                            pModule->query(
                                isFull ? NSCam::NSIoPipe::PORT_IMGO.index :
                                NSCam::NSIoPipe::PORT_RRZO.index,
                                NSCam::NSIoPipe::NSCamIOPipe::
                                    ENPipeQueryCmd_STRIDE_BYTE,
                                format,
                                input,
                                queryRst
                            );
                            stride = queryRst.stride_byte;
                            MY_LOGI0("add-BufInfoSet(%d)[%d][x%x]-(%dx%d) "
                                "Get ENPipeQueryCmd_STRIDE_BYTE(%zu)",
                            pixelMode, isFull, format,
                            addInfoSet.mSize.w, addInfoSet.mSize.h, stride);
                        } else {
                            MY_LOGE("CANNOT getNormalPipeModule");
                            return BAD_VALUE;
                        }
                    }
                    addInfoSet.mvStride.push_back(stride);
                }
                break;
        };
        stride = addInfoSet.mvStride;
        //for (size_t i = 0; i < stride.size(); i++)
        //    MY_LOGI1("Query Stride[%zu/%zu]=(%d)", i, stride.size(), stride[i]);
        mvInfoSet.push_back(addInfoSet);
    }
    //
    FUNCTION_OUT;
    return OK;
};

#endif


#if 1
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  StuffBufferManager Implementation
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

/******************************************************************************
 *
 ******************************************************************************/
MERROR
StuffBufferManager::
acquireStoreBuffer(sp<IImageBuffer> & imageBuffer,char const* szName,
    MINT32 format, MSize size, std::vector<MUINT32> & vStride,
    MUINT8 amountBasis, MUINT8 amountMax,
    MUINT8 multiple, MBOOL writable, MBOOL isSecure, SecType secType)
{
    FUNCTION_IN;
    //
    //P1_TRACE_F_BEGIN(SLG_I, "Stuff(%dx%d)%s", size.w, size.h, szName);
    //
    Mutex::Autolock _l(mLock);
    MERROR ret = OK;
    //
    sp<StuffBufferPool> bufPool = NULL;
    imageBuffer = NULL;
    //
    MUINT32 stride[3] = {0, 0, 0};
    size_t count = vStride.size();
    if (count > 3) {
        MY_LOGW("Fmt:0x%x (%dx%d) Cnt(%zu)", format, size.w, size.h, count);
        count = 3;
    }
    for (size_t i = 0; i < count; i++) {
        stride[i] = vStride[i];
    }
    //
    std::vector< sp<StuffBufferPool> >::iterator it = mvPoolSet.begin();
    for (; it != mvPoolSet.end(); it++) {
        sp<StuffBufferPool> sp = (*it);
        if ((sp != NULL) &&
            sp->compareLayout(format, size, stride[0], stride[1], stride[2])) {
            bufPool = sp;
            break;
        }
    }
    //
    if (bufPool == NULL) {
        sp<StuffBufferPool> newPool = new StuffBufferPool(szName, format, size,
            stride[0], stride[1], stride[2], amountBasis, amountMax,
            multiple, writable, mOpenId, mLogLevel, mLogLevelI, isSecure, secType);
        U_if (newPool == NULL) {
            MY_LOGE("Cannot new stuff buffer pool");
            return BAD_VALUE;
        }
        mvPoolSet.push_back(newPool);
        L_if (!mvPoolSet.empty()) {
            bufPool = mvPoolSet.at(mvPoolSet.size() -1);
        }
        MY_LOGD2("PoolSet.size(%zu)", mvPoolSet.size());
    }
    //
    U_if (bufPool == NULL) {
        MY_LOGE("Cannot create stuff buffer pool");
        return BAD_VALUE;
    } else {
        ret = bufPool->acquireBuffer(imageBuffer);
    }
    //
    //P1_TRACE_F_END(); // "Stuff"
    //
    FUNCTION_OUT;
    return ret;
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
StuffBufferManager::
releaseStoreBuffer(sp<IImageBuffer> & imageBuffer)
{
    FUNCTION_IN;
    //
    Mutex::Autolock _l(mLock);
    //
    if (imageBuffer == NULL) {
        MY_LOGW("Stuff ImageBuffer not exist");
        return BAD_VALUE;
    }
    //
    MINT const format = imageBuffer->getImgFormat();
    MSize const size = imageBuffer->getImgSize();
    MUINT32 stride[3] = {0, 0, 0};
    size_t count = imageBuffer->getPlaneCount();
    if (count > 3) {
        MY_LOGW("ImageBuffer Fmt:0x%x (%dx%d) PlaneCount(%zu)",
            imageBuffer->getImgFormat(), imageBuffer->getImgSize().w,
            imageBuffer->getImgSize().h, count);
        count = 3;
    }
    for (size_t i = 0; i < count; i++) {
        stride[i] = imageBuffer->getBufStridesInBytes(i);
    }
    //
    sp<StuffBufferPool> bufPool = NULL;
    std::vector< sp<StuffBufferPool> >::iterator it = mvPoolSet.begin();
    for(; it != mvPoolSet.end(); it++) {
        sp<StuffBufferPool> sp = (*it);
        if ((sp != NULL) &&
            sp->compareLayout(format, size, stride[0], stride[1], stride[2])) {
            bufPool = sp;
            break;
        }
    }
    //
    U_if (bufPool == NULL) {
        MY_LOGE("Cannot find stuff buffer pool");
        return BAD_VALUE;
    } else {
        bufPool->releaseBuffer(imageBuffer);
    }
    //
    FUNCTION_OUT;
    return OK;
}

#endif


#if 1
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  TimingChecker Implementation
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

/******************************************************************************
 *
 ******************************************************************************/
void
TimingChecker::Client::action(void)
{
    Mutex::Autolock _l(mLock);
    switch (mType) {
        case EVENT_TYPE_WARNING:
            MY_LOGW("[TimingChecker-W] [%s] (%dms) "
                "= ( %" PRId64 " - %" PRId64 " ns)",
                mStr.string(), mTimeInvMs,
                mEndTsNs, mBeginTsNs);
            break;
        case EVENT_TYPE_ERROR:
            MY_LOGE("[TimingChecker-E] [%s] (%dms) "
                "= ( %" PRId64 " - %" PRId64 " ns)",
                mStr.string(), mTimeInvMs,
                mEndTsNs, mBeginTsNs);
            break;
        case EVENT_TYPE_FATAL:
            MY_LOGF("[TimingChecker-F] [%s] (%dms) "
                "= ( %" PRId64 " - %" PRId64 " ns)",
                mStr.string(), mTimeInvMs,
                mEndTsNs, mBeginTsNs);
            // AEE trigger
            break;
        default: // EVENT_TYPE_NONE:
            // do nothing
            break;
    }
};

/******************************************************************************
 *
 ******************************************************************************/
void
TimingChecker::Client::onLastStrongRef(const void* /*id*/)
{
    dump("TC_Client::onLastStrongRef");
};

/******************************************************************************
 *
 ******************************************************************************/
void
TimingChecker::Client::dump(char const * tag)
{
    Mutex::Autolock _l(mLock);
    if (mLogLevelI >= 2) {
        char const * str =
            (tag != NULL) ? tag : "NULL";
        MY_LOGI2("[%s][%s] (%dms) = "
            "( %" PRId64 " - %" PRId64 " ns)",
            str, mStr.string(), mTimeInvMs,
            mEndTsNs, mBeginTsNs);
    }
};

/******************************************************************************
 *
 ******************************************************************************/
size_t
TimingChecker::RecStore::size(void)
{
    return mHeap.size();
};

/******************************************************************************
 *
 ******************************************************************************/
MBOOL
TimingChecker::RecStore::isEmpty(void)
{
    return mHeap.empty();
};

/******************************************************************************
 *
 ******************************************************************************/
MBOOL
TimingChecker::RecStore::addRec(RecPtr rp)
{
    if (rp == NULL) {
        return MFALSE;
    }
    //P1_TRACE_FUNC(SLG_O);
    mHeap.push(rp);
    //dump("AddRec");
    return MTRUE;
};

/******************************************************************************
 *
 ******************************************************************************/
TimingChecker::RecPtr const &
TimingChecker::RecStore::getMin(void)
{
    //P1_TRACE_FUNC(SLG_O);
    return mHeap.top();
};

/******************************************************************************
 *
 ******************************************************************************/
void
TimingChecker::RecStore::delMin(void)
{
    //P1_TRACE_FUNC(SLG_O);
    mHeap.pop();
    //dump("DelMin");
};

/******************************************************************************
 *
 ******************************************************************************/
void
TimingChecker::RecStore::dump(char const * tag)
{
    RecPtr* pTop = (RecPtr*)(&(mHeap.top()));
    MY_LOGI("RecPtrHeap @ %s",
        (tag != NULL) ? tag : "NULL");
    #if 1
    MY_LOGI("RecPtrHeap[0/%zu]@(%p) = (%p) "
            "( %" PRId64 " ns)",
            mHeap.size(), (void*)(pTop),
            (void*)(*pTop), (*pTop)->mTimeMarkNs);
    #else
    RecPtr* pCur = pTop;
    for (size_t i = 0; i < mHeap.size(); i++) {
        pCur = pTop + i;
        MY_LOGI0("RecPtrHeap[%zu/%zu]@(%p) = (%p) "
            "( %" PRId64 " ns)",
            i, mHeap.size(), (void*)(pCur),
            (void*)(*pCur), (*pCur)->mTimeMarkNs);
    }
    #endif
}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL
TimingChecker::doThreadLoop(void)
{
    Mutex::Autolock _l(mLock);
    //mData.clear();
    mWakeTiming = 0;
    mExitPending = MFALSE;
    mRunning = MTRUE;
    mEnterCond.broadcast();
    /*
        For less affecting, the TimingChecker caller
        might not wait for this thread loop ready.
        Hence, it checks the current time with the
        registered client's timing mark directly.
    */
    while (!mExitPending) {
        //MY_LOGD0("TC_Loop" " (%" PRId64 ") ", mWakeTiming);
        nsecs_t current = ::systemTime();
        if (mWakeTiming <= current) {
            mWakeTiming = checkList(current);
            //
            if (mWakeTiming == 0) {
                //CAM_TRACE_NAME("TC_wait()");
                //MY_LOGD0("TC_Loop" " (%" PRId64 ") " " wait +", mWakeTiming);
                mClientCond.wait(mLock);
                //MY_LOGD0("TC_Loop" " (%" PRId64 ") " " wait -", mWakeTiming);
            }
            //MY_LOGD0("TC_Loop" " (%" PRId64 ") " " continue", mWakeTiming);
            continue;
        }
        //
        nsecs_t sleep = mWakeTiming - current;
        //P1_TRACE_F_BEGIN(SLG_R, "TC_wait(%dms)", (MUINT32)(sleep / ONE_MS_TO_NS));
        //MY_LOGD0("TC_Loop" " (%" PRId64 ") " " waitRelative + sleep" " (%" PRId64 ") ", mWakeTiming, sleep);
        mClientCond.waitRelative(mLock, sleep);
        //MY_LOGD0("TC_Loop" " (%" PRId64 ") " " waitRelative - sleep" " (%" PRId64 ") ", mWakeTiming, sleep);
        //P1_TRACE_F_END(); // "TC_wait"
    }
    mRunning = MFALSE;
    mExitedCond.broadcast();
    return MFALSE;
};

/******************************************************************************
 *
 ******************************************************************************/
void
TimingChecker::doRequestExit(void)
{
    Mutex::Autolock _l(mLock);
    mWakeTiming = 0;
    mExitPending = MTRUE;
    //CAM_TRACE_NAME("TC_exit()");
    mEnterCond.broadcast();
    mClientCond.broadcast();
    // join loop
    MINT32 const cnt_max = 100000; //ms
    MINT32 cnt = cnt_max;
    while (mRunning) {
        //CAM_TRACE_NAME("TC_join()");
        mExitedCond.waitRelative(mLock, ONE_MS_TO_NS);
        if (--cnt <= 0) {
            MY_LOGW("wait for LoopExit timeout(%d ms)", cnt_max);
            break;
        }
    };
    // clear data
    while (!mData.isEmpty()) {
        //CAM_TRACE_NAME("TC_clean()");
    #if 0
        MY_LOGI0("RecordStore NOT empty, size(%zu),"
            " (%" PRId64 ") ", mData.size(),
            mData.getMin()->mTimeMarkNs);
    #endif
        RecPtr pRec = mData.getMin();
        U_if (pRec != NULL) {
            sp<TimingChecker::Client> c =
                pRec->mpClient.promote();
            if (c != NULL) {
                c->setLog(mOpenId, mLogLevel, mLogLevelI);
                c->dump("RecordStoreCleaning");
            } else {
                //MY_LOGI0("Client was already released");
            }
            delete (pRec);
            pRec = NULL;
        }
        mData.delMin();
    }
};

/******************************************************************************
 *
 ******************************************************************************/
sp<TimingChecker::Client>
TimingChecker::createClient(
    char const * str,
    MUINT32 uTimeoutMs,
    EVENT_TYPE eType)
{
    //P1_TRACE_FUNC(SLG_O);
    sp<TimingChecker::Client> client = new
        TimingChecker::Client(str, uTimeoutMs, eType);
    //MY_LOGD0("new sp client: s(%d) w(%d)", client->getStrongCount(), client->getWeakRefs()->getWeakCount());
    if (client == NULL) {
        MY_LOGE("CANNOT create TimingCheckerClient "
            "[%s]", str);
        return NULL;
    }
    {
        Mutex::Autolock _l(mLock);
        nsecs_t ts = client->getTimeStamp();
        RecPtr pRec = new Record(ts, client);
        if (pRec == NULL || !(mData.addRec(pRec))) {
            MY_LOGE("CANNOT new Record");
            client = NULL;
            return NULL;
        }
        if (mWakeTiming == 0 || ts < mWakeTiming) {
            //CAM_TRACE_NAME("TC_broadcast()");
            mClientCond.broadcast();
        }
    }
    return client;
};

/******************************************************************************
 *
 ******************************************************************************/
nsecs_t
TimingChecker::checkList(nsecs_t time)
{
    nsecs_t ts = 0;
    RecPtr pRec = NULL;
    //MY_LOGD0("checkList +++ [%zu] @" " (%" PRId64 ") ", mData.size(), time);
    while (!mData.isEmpty()) {
        pRec = mData.getMin();
        if (pRec == NULL) {
            MY_LOGE("Error Record in Store");
            ts = 0;
            break;
        }
        ts = pRec->mTimeMarkNs;
        if (ts <= time) {
            sp<TimingChecker::Client> c =
                pRec->mpClient.promote();
            if (c != NULL) {
                c->action();
            }
            delete pRec;
            mData.delMin();
            ts = 0;
        } else {
            //MY_LOGD0("checkList : NO Record needs to Pop-up");
            break;
        }
    }
    //MY_LOGD0("checkList --- (%" PRId64 ")", ts);
    return ts;
};

#endif


#if 1
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  TimingCheckerMgr Implementation
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

/******************************************************************************
 *
 ******************************************************************************/
void
TimingCheckerMgr::setEnable(MBOOL en)
{
    if (mpTimingChecker == NULL) {
        Mutex::Autolock _l(mLock);
        mIsEn = MFALSE;
        return;
    }
    MY_LOGD2("TimingChecker enable(%d)", en);
    {
        Mutex::Autolock _l(mLock);
        mIsEn = en;
        if (mIsEn) {
            return;
        }
    }
    // as (mIsEn == false)
    mpTimingChecker->doRequestExit();
};

/******************************************************************************
 *
 ******************************************************************************/
MBOOL
TimingCheckerMgr::getEnable(void)
{
    Mutex::Autolock _l(mLock);
    return mIsEn;
};

/******************************************************************************
 *
 ******************************************************************************/
void
TimingCheckerMgr::waitReady(void)
{
    if (mpTimingChecker == NULL) {
        return;
    }
    // no waiting for less affecting
    //mpTimingChecker->doWaitReady();
};

/******************************************************************************
 *
 ******************************************************************************/
void
TimingCheckerMgr::onCheck(void)
{
    if (mpTimingChecker == NULL) {
        return;
    }
    {
        Mutex::Autolock _l(mLock);
        if (!mIsEn) {
            return;
        }
    }
    // as (mIsEn == true)
    if (mpTimingChecker->doThreadLoop()) {
        MY_LOGD0("TimingChecker next loop");
    }
};

/******************************************************************************
 *
 ******************************************************************************/
sp<TimingChecker::Client>
TimingCheckerMgr::createClient(
    char const * str, MUINT32 uTimeoutMs,
    TimingChecker::EVENT_TYPE eType)
{
    if (mpTimingChecker == NULL) {
        return NULL;
    }
    return mpTimingChecker->createClient
        (str, uTimeoutMs * mFactor, eType);
};

#endif


#if 1
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  LongExposureStatus Implementation
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

/******************************************************************************
 *
 ******************************************************************************/
void
LongExposureStatus::config(
    MINT32 nOpenId,
    MINT32 nLogLevel,
    MINT32 nLogLevelI
)
{
    Mutex::Autolock _l(mLock);
    mOpenId = nOpenId;
    mLogLevel = nLogLevel;
    mLogLevelI = nLogLevelI;
    return;
};


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
LongExposureStatus::reset(MINT num)
{
    Mutex::Autolock _l(mLock);
    if (mvSet.empty()) {
        return MFALSE;
    }
    std::vector<MINT32>::iterator it = mvSet.begin();
    for (; it != mvSet.end(); it++) {
        if (num == *it) {
            mvSet.erase(it);
            break;
        }
    }
    if (mvSet.empty()) {
        mRunning = MFALSE;
    }
    MY_LOGI0("(%d/%zu) LongExposure[%d]",
            num, mvSet.size(), mRunning);
    return MTRUE;
};

/******************************************************************************
 *
 ******************************************************************************/
MBOOL
LongExposureStatus::set(MINT num, MINT64 exp_ns)
{
    Mutex::Autolock _l(mLock);
    if (exp_ns >= mThreshold && num > 0) {
        MBOOL isFound = MFALSE;
        std::vector<MINT32>::iterator it = mvSet.begin();
        for (; it != mvSet.end(); it++) {
            if (num == *it) {
                isFound = MTRUE;
                break;
            }
        }
        if (!isFound) {
            mvSet.push_back(num);
            mRunning = MTRUE;
        }
        MY_LOGI0("(%d/%zu) LongExposure[%d]",
                num, mvSet.size(), mRunning);
        return MTRUE;
    }
    return MFALSE;
};

/******************************************************************************
 *
 ******************************************************************************/
MBOOL
LongExposureStatus::get(void)
{
    Mutex::Autolock _l(mLock);
    MBOOL isRunning = mRunning;
    #if 0 //(!SUPPORT_LONG_EXPOSURE_ABORT)
    isRunning = MFALSE;
    #endif
    return isRunning;
};

#endif


#if 1
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  ProcedureStageControl Implementation
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

/******************************************************************************
 *
 ******************************************************************************/
MBOOL
ProcedureStageControl::reset(void)
{
    for (MUINT32 i = 0; i < mvpStage.size(); i++) {
        sp<StageNote> p = mvpStage.at(i);
        Mutex::Autolock _l(p->mLock);
        if (p->mWait) {
            p->mCond.broadcast();
        }
        p->mWait = MFALSE;
        p->mDone = MFALSE;
        p->mSuccess = MFALSE;
    }
    MY_LOGI2("StageCtrl reset OK");
    return MTRUE;
};

/******************************************************************************
 *
 ******************************************************************************/
MBOOL
ProcedureStageControl::wait(MUINT32 eStage, MBOOL& rSuccess)
{
    if (eStage >= mvpStage.size()) {
        MY_LOGW("wait - illegal (%d >= %zu)",
            eStage, mvpStage.size());
        return MFALSE;
    }
    //
    {
        sp<StageNote> p = mvpStage.at(eStage);
        Mutex::Autolock _l(p->mLock);
        if (!p->mDone) {
            P1_TRACE_F_BEGIN(SLG_S, "S_Wait(%d)", p->mId);
            MY_LOGI0("StageCtrl waiting(%d)", eStage);
            p->mWait = MTRUE;
            p->mCond.wait(p->mLock);
            P1_TRACE_F_END(); // "S_Wait"
        }
        p->mWait = MFALSE;
        rSuccess = p->mSuccess;
    }
    MY_LOGI0("StageCtrl wait(%d) OK", eStage);
    return MTRUE;
};

/******************************************************************************
 *
 ******************************************************************************/
MBOOL
ProcedureStageControl::done(MUINT32 eStage, MBOOL bSuccess)
{
    if (eStage >= mvpStage.size()) {
        MY_LOGW("done - illegal (%d >= %zu)",
            eStage, mvpStage.size());
        return MFALSE;
    }
    //
    {
        sp<StageNote> p = mvpStage.at(eStage);
        Mutex::Autolock _l(p->mLock);
        p->mDone = MTRUE;
        p->mSuccess = bSuccess;
        if (p->mWait) {
            MY_LOGI0("StageCtrl signal(%d)", eStage);
            p->mCond.broadcast();
        }
    }
    MY_LOGI2("StageCtrl done(%d) OK", eStage);
    return MTRUE;
};

#endif


#if 1
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  ConcurrenceControl Implementation
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

/******************************************************************************
 *
 ******************************************************************************/
MBOOL
ConcurrenceControl::initBufInfo_clean(void)
{
    Mutex::Autolock _l(mLock);
    if (mpBufInfo != NULL) {
        delete mpBufInfo;
        mpBufInfo = NULL;
        return MTRUE;
    }
    return MFALSE;
};

/******************************************************************************
 *
 ******************************************************************************/
MBOOL
ConcurrenceControl::initBufInfo_get(QBufInfo** ppBufInfo)
{
    Mutex::Autolock _l(mLock);
    if (mpBufInfo == NULL) {
        *ppBufInfo = NULL;
        return MFALSE;
    }
    *ppBufInfo = mpBufInfo;
    return MTRUE;
};

/******************************************************************************
 *
 ******************************************************************************/
MBOOL
ConcurrenceControl::initBufInfo_create(QBufInfo** ppBufInfo)
{
    Mutex::Autolock _l(mLock);
    if (mpBufInfo != NULL) {
        delete mpBufInfo;
        mpBufInfo = NULL;
    }
    //
    mpBufInfo = new QBufInfo();
    //
    if (mpBufInfo == NULL) {
        *ppBufInfo = NULL;
        return MFALSE;
    }
    *ppBufInfo = mpBufInfo;
    return MTRUE;
};

/******************************************************************************
 *
 ******************************************************************************/
void
ConcurrenceControl::setAidUsage(MBOOL enable)
{
    Mutex::Autolock _l(mLock);
    mIsAssistUsing = enable;
};

/******************************************************************************
 *
 ******************************************************************************/
MBOOL
ConcurrenceControl::getAidUsage(void)
{
    Mutex::Autolock _l(mLock);
    return mIsAssistUsing;
};

/******************************************************************************
 *
 ******************************************************************************/
void
ConcurrenceControl::cleanAidStage(void)
{
    setAidUsage(MFALSE);
    if (getStageCtrl() != NULL) {
        getStageCtrl()->reset();
    }
};

/******************************************************************************
 *
 ******************************************************************************/
sp<ProcedureStageControl>
ConcurrenceControl::getStageCtrl(void)
{
    return mpStageCtrl;
};

#endif


#if 1
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  HardwareStateControl Implementation
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

/******************************************************************************
 *
 ******************************************************************************/
void
HardwareStateControl::config(
    MINT32 nOpenId,
    MINT32 nLogLevel,
    MINT32 nLogLevelI,
    MINT32 nSysLevel,
    MUINT8 nBurstNum,
    INormalPipe* pCamIO,
    IHal3A_T* p3A,
    MBOOL isLegacyStandby
)
{
    Mutex::Autolock _l(mLock);
    mOpenId = nOpenId;
    mLogLevel = nLogLevel;
    mLogLevelI = nLogLevelI;
    mSysLevel = nSysLevel;
    mBurstNum = nBurstNum;
    mpCamIO = pCamIO;
    mp3A = p3A;
    mIsLegacyStandby = isLegacyStandby;
    //
    clean();
};


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
HardwareStateControl::isActive(void)
{
    #if 0 // force to disable HardwareStateControl
    return MFALSE;
    #endif
    // by DRV comment, SUSPEND is not supported in burst mode
    return (IS_BURST_OFF && (mpCamIO != NULL) && (mp3A != NULL));
};


/******************************************************************************
 *
 ******************************************************************************/
SENSOR_STATUS_CTRL
HardwareStateControl::checkReceiveFrame(IMetadata* pMeta)
{
    if (!isActive()) {
        return SENSOR_STATUS_CTRL_NONE;
    }
    //
    Mutex::Autolock _l(mLock);
    MINT32 ctrl = MTK_P1_SENSOR_STATUS_NONE;
    MBOOL tag = MFALSE;
    SENSOR_STATUS_CTRL ret = SENSOR_STATUS_CTRL_NONE;
    //
    if (tryGetMetadata<MINT32>(pMeta, MTK_P1NODE_SENSOR_STATUS, ctrl)) {
        tag = MTRUE;
        if (ctrl == MTK_P1_SENSOR_STATUS_SW_STANDBY ||
            ctrl == MTK_P1_SENSOR_STATUS_HW_STANDBY) {
            switch (mState) {
                case STATE_NORMAL:
                    mState = STATE_SUS_WAIT_NUM;
                    ret = SENSOR_STATUS_CTRL_STANDBY;
                    break;
                default:
                    break;
            };
            MY_LOGI0("[SUS-RES] meta-sus(%d) @(%d)",
                ctrl, mState);
        } else if (ctrl == MTK_P1_SENSOR_STATUS_STREAMING) {
            switch (mState) {
                case STATE_SUS_DONE:
                    mState = STATE_RES_WAIT_NUM;
                    ret = SENSOR_STATUS_CTRL_STREAMING;
                    break;
                default:
                    break;
            };
            MY_LOGI0("[SUS-RES] meta-res(%d) @(%d)",
                ctrl, mState);
        }
    }
    MY_LOGD3(
        "tag(%d) : sensor(%d) - state(%d)", tag, ctrl, mState);
    if (mState == STATE_RES_WAIT_NUM) {
        mShutterTimeUs = (MINT32)0;
        if (tryGetMetadata<MINT32>(pMeta, MTK_P1NODE_RESUME_SHUTTER_TIME_US,
            mShutterTimeUs)) {
            MY_LOGI0("[SUS-RES] re-streaming with (%d)us",
                mShutterTimeUs);
        } else {
            MY_LOGI0("[SUS-RES] re-streaming without time-set");
        }
    }
    return ret;
};


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
HardwareStateControl::checkReceiveRestreaming(void)
{
    if (!isActive()) {
        return MFALSE;
    }
    return (mState == STATE_RES_WAIT_NUM) ? MTRUE : MFALSE;
}


/******************************************************************************
 *
 ******************************************************************************/
void
HardwareStateControl::checkShutterTime(MINT32 & rShutterTimeUs)
{
    if (!isActive()) {
        return;
    }
    //
    Mutex::Autolock _l(mLock);
    if (mState >= STATE_RES_WAIT_NUM) {
        rShutterTimeUs = mShutterTimeUs;
        MY_LOGI0("[SUS-RES] ShutterTime(%d) "
            "@(%d)", mShutterTimeUs, mState);
    } else {
        rShutterTimeUs = 0;
        MY_LOGI0("[SUS-RES] none-ShutterTime(%d) "
            "@(%d)", mShutterTimeUs, mState);
    }
    return;
};


/******************************************************************************
 *
 ******************************************************************************/
void
HardwareStateControl::checkRestreamingNum(MINT32 num)
{
    if (!isActive()) {
        return;
    }
    //
    Mutex::Autolock _l(mLock);
    if (mState == STATE_RES_WAIT_NUM) {
        mStreamingSetNum = num;
        //
        mState = STATE_RES_WAIT_SYNC;
        MY_LOGI0("[SUS-RES] StreamingSet(%d) "
            "@(%d)", mStreamingSetNum, mState);
    }
    return;
};

#if 0
/******************************************************************************
 *
 ******************************************************************************/
void
HardwareStateControl::checkReceiveNode(MINT32 num, MBOOL bSkipEnQ,
    MBOOL & rIsNeedEnQ, MINT32 & rShutterTimeUs)
{
    if (!isActive()) {
        return;
    }
    //
    Mutex::Autolock _l(mLock);
    rIsNeedEnQ = MFALSE;
    if (mState == STATE_SUS_WAIT_NUM) {
        mStandbySetNum = num;
        mRequestPass = MFALSE;
        mState = STATE_SUS_WAIT_SYNC;
        MY_LOGI0("[SUS-RES] StandbySet(%d) @(%d)",
            mStandbySetNum, mState);
    } else if (mState == STATE_RES_WAIT_NUM) {
        mStreamingSetNum = num;
        mRequestPass = MFALSE;
        //
        rIsNeedEnQ = MFALSE;
        rShutterTimeUs = mShutterTimeUs;
        if (!bSkipEnQ) {
            #if P1_SUPPORT_DIR_RESTREAM
            // need check DRV support
            rIsNeedEnQ = MTRUE;
            #endif
        } // else force to skip the directly EnQ flow
        //
        if (!rIsNeedEnQ) { // NO Directly EnQ flow
            MBOOL ret = MFALSE;
            P1_TRACE_F_BEGIN(SLG_E, "P1:DRV-resume(%d)", mShutterTimeUs);
            ret = mpCamIO->resume(mShutterTimeUs);
            P1_TRACE_F_END(); // "P1:DRV-resume"
            if (!ret) {
                MY_LOGE("[SUS-RES] FAIL : num-res(%d) EnQ(%d:%d) "
                    "@(%d)", num, bSkipEnQ, rIsNeedEnQ, mState);
                mState = STATE_NORMAL;
                mStandbySetNum = 0;
                mStreamingSetNum = 0;
                mShutterTimeUs = 0;
                mvStoreNum.clear();
                return;
            }
            //
            P1_TRACE_F_BEGIN(SLG_E, "P1:3A-resume");
            mp3A->resume();
            P1_TRACE_F_END(); // "P1:3A-resume"
            //
            MY_LOGI0("[SUS-RES] Recover-Loop-N");
            mThreadCond.broadcast();
        } else { // Directly EnQ flow
            P1_TRACE_F_BEGIN(SLG_E, "P1:3A-resume(%d)", mStreamingSetNum);
            mp3A->resume(mStreamingSetNum);
            P1_TRACE_F_END(); // "P1:3A-resume"
        }
        //
        mState = STATE_RES_WAIT_SYNC;
        MY_LOGI0("[SUS-RES] SkipEnQ(%d) NeedEnQ(%d) StreamingSet(%d) "
            "@(%d)", bSkipEnQ, rIsNeedEnQ, mStreamingSetNum, mState);
    }
    return;
};
#endif


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
HardwareStateControl::checkCtrlStandby(MINT32 num)
{
    if (!isActive()) {
        return MFALSE;
    }
    //
    Mutex::Autolock _l(mLock);

    // TODO: remove STATE_SUS_WAIT_NUM
    if (mState == STATE_SUS_WAIT_NUM) {
        mStandbySetNum = num;
        mRequestPass = MFALSE;
        mState = STATE_SUS_WAIT_SYNC;
        MY_LOGI0("[SUS-RES] StandbySet(%d) @(%d)",
            mStandbySetNum, mState);
    }

    if (mState == STATE_SUS_WAIT_SYNC) {
        // TODO: remove mStandbySetNum
        mStandbySetNum = num;
        //
        P1_TRACE_S_BEGIN(SLG_E, "P1:3A-pause");
        mp3A->pause();
        P1_TRACE_S_END(); // "P1:3A-pause"
        //
        MBOOL ret = MFALSE;
        P1_TRACE_S_BEGIN(SLG_E, "P1:DRV-suspend");
        ret = mpCamIO->suspend();
        P1_TRACE_S_END(); // "P1:DRV-suspend"
        if (!ret) {
            MY_LOGE("[SUS-RES] FAIL : num-sus(%d) @(%d)",
                num, mState);
            mp3A->resume();
            clean();
            return MTRUE;
        }
        //
        mState = STATE_SUS_READY;
        mRequestCond.broadcast();
        MY_LOGI0("[SUS-RES] CurNum(%d) (%d/%d) @(%d)", num,
            mStandbySetNum, mStreamingSetNum, mState);
        return MTRUE;
    }
    return MFALSE;
};


#if 0
/******************************************************************************
 *
 ******************************************************************************/
MBOOL
HardwareStateControl::checkSetNum(MINT32 num)
{
    if (!isActive()) {
        return MFALSE;
    }
    //
    Mutex::Autolock _l(mLock);

    if (mState == STATE_SUS_WAIT_SYNC && mStandbySetNum == num) {
        P1_TRACE_S_BEGIN(SLG_E, "P1:3A-pause");
        mp3A->pause();
        P1_TRACE_S_END(); // "P1:3A-pause"
        //
        MBOOL ret = MFALSE;
        P1_TRACE_S_BEGIN(SLG_E, "P1:DRV-suspend");
        ret = mpCamIO->suspend();
        P1_TRACE_S_END(); // "P1:DRV-suspend"
        if (!ret) {
            MY_LOGE("[SUS-RES] FAIL : num-sus(%d) @(%d)",
                num, mState);
            mp3A->resume();
            mState = STATE_NORMAL;
            mStandbySetNum = 0;
            mStreamingSetNum = 0;
            mShutterTimeUs = 0;
            mvStoreNum.clear();
            mRequestCond.broadcast();
            mThreadCond.broadcast();
            return MTRUE;
        }
        //
        mState = STATE_SUS_READY;
        mRequestCond.broadcast();
        MY_LOGI0("[SUS-RES] CurNum(%d) (%d/%d) @(%d)", num,
            mStandbySetNum, mStreamingSetNum, mState);
        return MTRUE;
    }
    return MFALSE;
};
#endif


/******************************************************************************
 *
 ******************************************************************************/
void
HardwareStateControl::checkRequest(void)
{
    if (!isActive()) {
        return;
    }
    //
    Mutex::Autolock _l(mLock);
    if (mState == STATE_SUS_WAIT_SYNC ||
        mState == STATE_SUS_WAIT_NUM) {
        MY_LOGI0("[SUS-RES] Suspend-Request @(%d)", mState);
        P1_TRACE_S_BEGIN(SLG_E, "P1:pause");
        MY_LOGD0("[SUS-RES] wait pause +");
        mRequestCond.wait(mLock);
        MY_LOGD0("[SUS-RES] wait pause -");
        P1_TRACE_S_END(); // "P1:pause"
    }
    return;
};


/******************************************************************************
 *
 ******************************************************************************/
void
HardwareStateControl::checkThreadStandby(void)
{
    if (!isActive()) {
        return;
    }
    //
    Mutex::Autolock _l(mLock);
    if (mState == STATE_SUS_READY) {
        mState = STATE_SUS_DONE;
        MY_LOGI0("[SUS-RES] Suspend-Loop @(%d)", mState);
        P1_TRACE_S_BEGIN(SLG_E, "P1:suspend");
        MY_LOGD0("[SUS-RES] wait re-streaming +");
        mThreadCond.wait(mLock);
        MY_LOGD0("[SUS-RES] wait re-streaming -");
        P1_TRACE_S_END(); // "P1:pause"
    }
    return;
};


/******************************************************************************
 *
 ******************************************************************************/
void
HardwareStateControl::checkThreadWeakup(void)
{
    if (!isActive()) {
        return;
    }
    //
    Mutex::Autolock _l(mLock);
    if (mState == STATE_RES_WAIT_SYNC) {
        MY_LOGI0("[SUS-RES] Recover-Loop-W");
        mThreadCond.broadcast();
    }
    return;
};


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
HardwareStateControl::checkFirstSync(void)
{
    if (!isActive()) {
        return MFALSE;
    }
    //
    Mutex::Autolock _l(mLock);
    if (mState == STATE_RES_WAIT_SYNC) {
        mState = STATE_RES_WAIT_DONE;
        MY_LOGI0("[SUS-RES] FirstSync (%d/%d) @(%d)",
            mStandbySetNum, mStreamingSetNum, mState);
        return MTRUE;
    }
    return MFALSE;
};


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
HardwareStateControl::checkSkipSync(void)
{
    if (!isActive()) {
        return MFALSE;
    }
    //
    Mutex::Autolock _l(mLock);
    switch (mState) {
        case STATE_NORMAL:
        case STATE_SUS_WAIT_NUM:
        case STATE_SUS_WAIT_SYNC:
        case STATE_RES_WAIT_SYNC:
        case STATE_RES_WAIT_DONE:
            return MFALSE;
        //case STATE_RES_WAIT_NUM:
        //case STATE_SUS_READY:
        //case STATE_SUS_DONE:
        default:
            break;
    };
    MY_LOGI0("[SUS-RES] SkipSync (%d/%d) @(%d)",
        mStandbySetNum, mStreamingSetNum, mState);
    return MTRUE;
};


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
HardwareStateControl::checkSkipWait(void)
{
    if (!isActive()) {
        return MFALSE;
    }
    //
    Mutex::Autolock _l(mLock);
    if (mRequestPass) {
        MY_LOGI0("[SUS-RES] SkipWait pass (%d/%d) @(%d)",
            mStandbySetNum, mStreamingSetNum, mState);
        mRequestPass = MFALSE;
        return MTRUE;
    }
    switch (mState) {
        case STATE_NORMAL:
        case STATE_SUS_WAIT_NUM:
        case STATE_SUS_WAIT_SYNC:
        case STATE_SUS_READY:
        case STATE_SUS_DONE:
        case STATE_RES_WAIT_SYNC:
        case STATE_RES_WAIT_DONE:
            return MFALSE;
        //case STATE_RES_WAIT_NUM:
        default:
            break;
    };
    MY_LOGI0("[SUS-RES] SkipWait (%d/%d) @(%d)",
        mStandbySetNum, mStreamingSetNum, mState);
    return MTRUE;
};


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
HardwareStateControl::checkSkipBlock(void)
{
    if (!isActive()) {
        return MFALSE;
    }
    //
    Mutex::Autolock _l(mLock);
    if (mRequestPass) {
        MY_LOGI0("[SUS-RES] SkipBlock pass (%d/%d) @(%d)",
            mStandbySetNum, mStreamingSetNum, mState);
        mRequestPass = MFALSE;
        return MTRUE;
    }
    switch (mState) {
        case STATE_NORMAL:
        case STATE_SUS_WAIT_NUM:
        case STATE_RES_WAIT_NUM:
        case STATE_RES_WAIT_SYNC:
        case STATE_RES_WAIT_DONE:
            return MFALSE;
        //case STATE_SUS_WAIT_SYNC:
        //case STATE_SUS_READY:
        //case STATE_SUS_DONE:
        default:
            break;
    };
    MY_LOGI0("[SUS-RES] SkipBlock (%d/%d) @(%d)",
        mStandbySetNum, mStreamingSetNum, mState);
    return MTRUE;
};


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
HardwareStateControl::checkOnExecuting(void)
{
    if (!isActive()) {
        return MFALSE;
    }
    //
    Mutex::Autolock _l(mLock);
    switch (mState) {
        case STATE_NORMAL:
        case STATE_SUS_DONE:
            // standby / weak-up done
            return MFALSE;
        //case STATE_SUS_WAIT_NUM:
        //case STATE_SUS_WAIT_SYNC:
        //case STATE_SUS_READY:
        //case STATE_RES_WAIT_NUM:
        //case STATE_RES_WAIT_SYNC:
        //case STATE_RES_WAIT_DONE:
        default:
            break;
    };
    MY_LOGI0("[SUS-RES] OnExecuting (%d/%d) @(%d)",
        mStandbySetNum, mStreamingSetNum, mState);
    // zero buffer count is normal
    return MTRUE;
};


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
HardwareStateControl::checkBufferState(void)
{
    if (!isActive()) {
        // zero buffer count is abnormal
        return MFALSE;
    }
    //
    Mutex::Autolock _l(mLock);
    switch (mState) {
        case STATE_NORMAL:
        case STATE_SUS_WAIT_NUM:
        case STATE_SUS_WAIT_SYNC:
        case STATE_RES_WAIT_DONE:
            // zero buffer count is abnormal
            return MFALSE;
        //case STATE_SUS_READY:
        //case STATE_SUS_DONE:
        //case STATE_RES_WAIT_NUM:
        //case STATE_RES_WAIT_SYNC:
        default:
            break;
    };
    MY_LOGI0("[SUS-RES] NormalCase (%d/%d) @(%d)",
        mStandbySetNum, mStreamingSetNum, mState);
    // zero buffer count is normal
    return MTRUE;
};


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
HardwareStateControl::checkDoneNum(MINT32 num)
{
    if (!isActive()) {
        return MFALSE;
    }
    //
    Mutex::Autolock _l(mLock);
    switch (mState) {
        case STATE_NORMAL:
        case STATE_SUS_WAIT_NUM:
        case STATE_SUS_WAIT_SYNC:
        case STATE_SUS_READY:
            // do nothing
            return MFALSE;
        //case STATE_SUS_DONE:
        //case STATE_RES_WAIT_NUM:
        //case STATE_RES_WAIT_SYNC:
        //case STATE_RES_WAIT_DONE:
        default:
            break;
    };
    mvStoreNum.clear();
    if (mState == STATE_RES_WAIT_DONE && mStreamingSetNum == num) {
        mStandbySetNum = 0;
        mStreamingSetNum = 0;
        mShutterTimeUs = 0;
        mRequestPass = MFALSE;
        mState = STATE_NORMAL;
        //clean();
    }
    MY_LOGI0("[SUS-RES] CurNum(%d) SetNum(%d/%d) @(%d)", num,
        mStandbySetNum, mStreamingSetNum, mState);
    return MTRUE; // need to drop previous frame
};


/******************************************************************************
 *
 ******************************************************************************/
void
HardwareStateControl::checkNotePass(MBOOL pass)
{
    if (!isActive()) {
        return;
    }
    //
    Mutex::Autolock _l(mLock);
    mRequestPass = pass;
    MY_LOGI0("[SUS-RES] NoteNextRequestPass(%d) (%d/%d) @(%d)",
        mRequestPass, mStandbySetNum, mStreamingSetNum, mState);
    return;
};


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
HardwareStateControl::
checkPausing(void)
{
    if (!isActive()) {
        return MFALSE;
    }
    Mutex::Autolock _l(mLock);
    MY_LOGI1("[SUS-RES] Pausing (%d/%d) @(%d)",
        mStandbySetNum, mStreamingSetNum, mState);
    return (mState == STATE_SUS_DONE);
};


/******************************************************************************
 *
 ******************************************************************************/
void
HardwareStateControl::setDropNum(MINT32 num)
{
    if (!isActive()) {
        return;
    }
    //
    Mutex::Autolock _l(mLock);
    mvStoreNum.push_back(num);
    MY_LOGI0("[SUS-RES] CurNum(%d) (%d/%d) @(%d)", num,
        mStandbySetNum, mStreamingSetNum, mState);
};


/******************************************************************************
 *
 ******************************************************************************/
MINT32
HardwareStateControl::getDropNum(void)
{
    if (!isActive()) {
        return 0;
    }
    //
    Mutex::Autolock _l(mLock);
    MINT32 num = 0;
    if (!mvStoreNum.empty()) {
        std::vector<MINT32>::iterator it = mvStoreNum.begin();
        num = *it;
        mvStoreNum.erase(it);
    }
    return num;
};


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
HardwareStateControl::
isLegacyStandby(void)
{
    return mIsLegacyStandby;
}


/******************************************************************************
 *
 ******************************************************************************/
void
HardwareStateControl::reset(void)
{
    if (!isActive()) {
        return;
    }
    //
    Mutex::Autolock _l(mLock);
    if (mState != STATE_NORMAL) {
        MY_LOGI0("[SUS-RES] reset (%d/%d) @(%d ===>>> %d)",
            mStandbySetNum, mStreamingSetNum, mState, STATE_NORMAL);
    }
    mp3A = NULL;
    mpCamIO = NULL;
    clean();
    MY_LOGD2("HardwareStateControl RESET");
    return;
};


/******************************************************************************
 *
 ******************************************************************************/
void
HardwareStateControl::clean(void)
{
    mIsLegacyStandby = MFALSE;
    mState = STATE_NORMAL;
    mStandbySetNum = 0;
    mStreamingSetNum = 0;
    mShutterTimeUs = 0;
    mRequestPass = MFALSE;
    mvStoreNum.clear();
    mRequestCond.broadcast();
    mThreadCond.broadcast();
    return;
};


/******************************************************************************
 *
 ******************************************************************************/
void
HardwareStateControl::dump(void)
{
    MY_LOGI0("[SUS-RES] DUMP : num-sus(%d) num-res(%d) legacy(%d) @(%d)",
        mStandbySetNum, mStreamingSetNum, mIsLegacyStandby, mState);
    return;
};

#endif


#if 1
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  FrameNote Implementation
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

/******************************************************************************
 *
 ******************************************************************************/
MVOID
FrameNote::
set(MINT32 num) {
    U_if (mSlotCapacity == 0) {
        MY_LOGW("Capacity(%d)", mSlotCapacity);
        return;
    }
    RWLock::AutoWLock _wl(mLock);
    gettimeofday(&mLastTv, NULL);
    mLastTid = (MUINT32)gettid();
    mLastNum = num;
    //
    mSlotIndex = (mSlotIndex + 1) % mSlotCapacity;
    L_if (mSlotIndex < (MUINT32)mvSlot.size()) {
        mvSlot[mSlotIndex] = num;
    } else {
        MY_LOGW("index(%d) >= size(%d)", mSlotIndex, (MUINT32)mvSlot.size());
    }
    return;
};


/******************************************************************************
 *
 ******************************************************************************/
MVOID
FrameNote::
get(android::String8 * pStr) {
    U_if (mSlotCapacity == 0) {
        MY_LOGW("Capacity(%d)", mSlotCapacity);
        return;
    }
    U_if (pStr == NULL) {
        MY_LOGW("get string null");
        return;
    }
    RWLock::AutoRLock _rl(mLock);
    android::String8 info("");
    struct tm * tm = NULL;
    char date_time[32] = {0};
    if((tm = localtime(&(mLastTv.tv_sec))) == NULL) {
        snprintf(date_time, sizeof(date_time), "%s", "NO_LOCAL_TIME");
    } else {
        strftime((char *)(date_time), sizeof(date_time), "%H:%M:%S", tm);
    }
    info.appendFormat(" [Last-Frame-Num(%d_%s.%06ld@%05d) ",
        mLastNum, date_time, mLastTv.tv_usec, mLastTid);
    //
    MUINT32 currIdx = mSlotIndex;
    MUINT32 thisIdx = currIdx;
    MUINT32 cnt = (MUINT32)mvSlot.size();
    MINT32 num = P1NODE_FRAME_NOTE_NUM_UNKNOWN;
    for (MUINT32 i = 0; i < mSlotCapacity; i++) {
        L_if (thisIdx < cnt) {
            //MY_LOGI("this(%d)[%d] loop:%d", thisIdx, mvSlot[thisIdx], i);
            num = mvSlot[thisIdx];
            L_if (num != P1NODE_FRAME_NOTE_NUM_UNKNOWN) {
                info.appendFormat("%d ", num);
            }
        }
        // move to the previous slot
        thisIdx = (thisIdx + mSlotCapacity - 1) % mSlotCapacity;
    };
    info.appendFormat("... ]");
    //
    pStr->append(info);
    return;
};


#endif



#if 1
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  LogInfo Implementation
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

/******************************************************************************
 *
 ******************************************************************************/
void
LogInfo::
clear(void)
{
    RWLock::AutoWLock _wl(mLock);
    for (int cp = LogInfo::CP_FIRST; cp < LogInfo::CP_MAX; cp++) {
        mSlots[cp].clear();
    }
    #if 0 // print all
    #if 1 // print all CheckPoint
    inspect(LogInfo::IT_COMMON_DUMP);
    #else // print all InspectType with CheckPoint
    {
        for (MUINT8 i = (MUINT8)LogInfo::IT_COMMON_DUMP;
            i < (MUINT8)LogInfo::IT_MAX; i++) {
            inspect((LogInfo::InspectType)i);
        }
    }
    #endif
    // print all ClueCodeList
    {
        for (MUINT8 i = 0; i < CC_AMOUNT_MAX; i++) {
            MUINT64 bit = (MUINT64)((MUINT64)(0x1) << i);
            //MY_LOGI(P1_LOG_DUMP_TAG " ClueCode-Bit[0x%" PRIx64 "] @ (%d)", bit, i);
            if ((bit & ClueCode::CC_ALL) == bit) {
                android::String8 str("");
                bitStr(bit, &str);
                MY_LOGI(P1_LOG_DUMP_TAG " ClueCode-Bit[0x%" PRIx64 "] = %s ",
                    bit, str.string());
            }
        }
    }
    //
    for (MUINT8 i = 0; i < CC_AMOUNT_MAX; i++) {
        MUINT64 bit = (MUINT64)((MUINT64)(0x1) << i);
        if ((bit & ClueCode::CC_ALL) == bit) {
            std::vector< LogInfo::CheckPoint > vTag;
            bitTag(bit, vTag);
            /*
            MY_LOGI(P1_LOG_DUMP_TAG " ClueCode-Bit[0x%" PRIx64 "] = %s ",
                bit, str.string());
            */
        }
    }
    #endif
};


/******************************************************************************
 *
 ******************************************************************************/
void
LogInfo::
setMemo(LogInfo::CheckPoint cp,
    MINT64 param0, MINT64 param1, MINT64 param2, MINT64 param3)
{
    if (!getActive()) {
        return;
    }
    // for performance consideration, only RLock while per-frame memo set/get
    RWLock::AutoRLock _rl(mLock);
    //
    write(cp, param0, param1, param2, param3);
    return;
};


/******************************************************************************
 *
 ******************************************************************************/
void
LogInfo::
write(LogInfo::CheckPoint cp,
    MINT64 param0, MINT64 param1, MINT64 param2, MINT64 param3)
{
    if (!getActive()) {
        return;
    }
    struct timeval tv = {0, 0};
    gettimeofday(&tv, NULL);
    if (cp < LogInfo::CP_MAX) {
        RWLock::AutoWLock _wl(mSlots[cp].mLock);
        mSlots[cp].mTv = tv;
        mSlots[cp].mTid = (MUINT32)gettid();
        mSlots[cp].mParam[0] = param0;
        mSlots[cp].mParam[1] = param1;
        mSlots[cp].mParam[2] = param2;
        mSlots[cp].mParam[3] = param3;
    };
};


/******************************************************************************
 *
 ******************************************************************************/
void
LogInfo::
getMemo(LogInfo::CheckPoint cp, android::String8 * str)
{
    if (!getActive()) {
        return;
    }
    // for performance consideration, only RLock while per-frame memo set/get
    RWLock::AutoRLock _rl(mLock);
    //
    read(cp, str);
    return;
};


/******************************************************************************
 *
 ******************************************************************************/
void
LogInfo::
read(LogInfo::CheckPoint cp, android::String8 * str)
{
    if (!getActive()) {
        return;
    }
    //
    if (cp < LogInfo::CP_MAX && str != NULL) {
        RWLock::AutoRLock _rl(mSlots[cp].mLock);
        struct tm * tm = NULL;
        char date_time[32] = {0};
        if((tm = localtime(&(mSlots[cp].mTv.tv_sec))) == NULL) {
            snprintf(date_time, sizeof(date_time), "%s", "NO_LOCAL_TIME");
        } else {
            strftime((char *)(date_time), sizeof(date_time), "%H:%M:%S", tm);
        }
        str->appendFormat(" [ %s.%06ld_%05d-%05d= ", date_time,
            mSlots[cp].mTv.tv_usec, ((mSlots[cp].mTid > 0) ? mPid : 0),
            mSlots[cp].mTid);
        if (cp != mNotes[cp].idx) {
            str->appendFormat("< NOTE_MISMATCH - %d!=%d >", cp, mNotes[cp].idx);
        } else {
            str->appendFormat("<%s> ", mNotes[cp].main);
        }
        for (int i = 0; i < PARAM_NUM; i++) {
            str->appendFormat("%s(%" PRId64 ") ",
                mNotes[cp].sub[i], mSlots[cp].mParam[i]);
        }
        str->appendFormat("] ");
    };
};


/******************************************************************************
 *
 ******************************************************************************/
void
LogInfo::
extract()
{
    if (!getActive()) {
        return;
    }
    //
    gettimeofday(&(mData.mNowTv), NULL);
    mData.mNowTime =
        (mData.mNowTv.tv_sec * ONE_S_TO_US) + mData.mNowTv.tv_usec;
    mData.mNowTid = (MUINT32)gettid();
    for (int cp = LogInfo::CP_FIRST; cp < LogInfo::CP_MAX; cp ++) {
        mData.mTv[cp] = mSlots[cp].mTv;
        mData.mTime[cp] =
            (mSlots[cp].mTv.tv_sec * ONE_S_TO_US) + mSlots[cp].mTv.tv_usec;
        mData.mTid[cp] = mSlots[cp].mTid;
    }
    //
    #ifdef GET_DATA
    #undef GET_DATA
    #endif
    #define GET_DATA(slot, idx) (MINT32)(mSlots[slot].mParam[idx])
    //
    mData.mCbSyncType = GET_DATA(CP_CB_SYNC_REV, 0);
    mData.mCbProcType = GET_DATA(CP_CB_PROC_REV, 0);
    //
    mData.mStartSetType = GET_DATA(CP_START_SET_END, 0);
    mData.mStartSetMn = GET_DATA(CP_START_SET_END, 1);
    //
    mData.mPreSetKey = GET_DATA(CP_PRE_SET_END, 0);
    //
    mData.mSetFn = GET_DATA(CP_SET_END, 2);
    //
    mData.mSetMn = GET_DATA(CP_SET_END, 1);
    mData.mEnqMn = GET_DATA(CP_ENQ_END, 0);
    mData.mDeqMn = GET_DATA(CP_DEQ_END, 0);
    //
    mData.mBufStream = GET_DATA(CP_BUF_BGN, 0);
    mData.mBufMn = GET_DATA(CP_BUF_BGN, 1);
    mData.mBufFn = GET_DATA(CP_BUF_BGN, 2);
    mData.mBufRn = GET_DATA(CP_BUF_BGN, 3);
    //
    mData.mAcceptFn = GET_DATA(CP_REQ_ACCEPT, 0);
    mData.mAcceptRn = GET_DATA(CP_REQ_ACCEPT, 1);
    mData.mAcceptResult = GET_DATA(CP_REQ_ACCEPT, 3);
    //
    mData.mRevFn = GET_DATA(CP_REQ_REV, 0);
    mData.mRevRn = GET_DATA(CP_REQ_REV, 1);
    //
    mData.mOutFn = GET_DATA(CP_OUT_BGN, 0);
    mData.mOutRn = GET_DATA(CP_OUT_BGN, 1);
    //
    #undef GET_DATA
    //
    mData.mReady = MTRUE;
}


/******************************************************************************
 *
 ******************************************************************************/
void
LogInfo::
analyze(MBOOL bForceToPrint)
{
    if (!getActive()) {
        return;
    }
    //
    reset();
    extract();
    if (!mData.mReady) {
        return;
    }
    //
    MBOOL bBlockInStart = MFALSE;
    MBOOL bBlockInStop = MFALSE;
    MBOOL bBlockAfterFlush = MFALSE;
    #ifdef START_STOP_OK
    #undef START_STOP_OK
    #endif
    #define START_STOP_OK ((!bBlockInStart) && (!bBlockInStop) &&\
        (!bBlockAfterFlush))
    //
    MBOOL bStreaming = MTRUE;
    if (mData.mTime[CP_OP_STOP_END] > mData.mTime[CP_OP_START_BGN]) {
        bStreaming = MFALSE;
    }
    //
    // for start flow
    CHECK_STUCK(CP_OP_START_BGN, CP_OP_START_END, CcDeduce_OpStartBlocking);
    if (HAS(CC_DEDUCE, CcDeduce_OpStartBlocking)) {
        bBlockInStart = MTRUE;
    }
    // for stop flow
    CHECK_STUCK(CP_OP_STOP_BGN, CP_OP_STOP_END, CcDeduce_OpStopBlocking);
    if (HAS(CC_DEDUCE, CcDeduce_OpStopBlocking)) {
        bBlockInStop = MTRUE;
    }
    // for uninit() not called after flush()
    if (((!bBlockInStart) && (!bBlockInStop)) &&
        (mData.mTime[CP_API_FLUSH_END] > mData.mTime[CP_REQ_REV]) && // no queue acceptable request after flush
        (mData.mTime[CP_API_FLUSH_END] > mData.mTime[CP_API_FLUSH_BGN]) && // flush done
        (mData.mTime[CP_API_FLUSH_END] > mData.mTime[CP_API_UNINIT_BGN]) && // uninit is not call after flush done
        (DIFF_NOW(CP_API_FLUSH_END, P1_GENERAL_API_CHECK_US))) {
        ADD(CC_DEDUCE, CcDeduce_UninitNotCalledAfterFlush);
        bBlockAfterFlush = MTRUE;
    }
    //
    CHECK_OP(CP_START_SET_BGN, CP_START_SET_END, CcOpTimeout_StartSet);
    CHECK_OP(CP_PRE_SET_BGN, CP_PRE_SET_END, CcOpTimeout_PreSet);
    CHECK_OP(CP_SET_BGN, CP_SET_END, CcOpTimeout_Set);
    CHECK_OP(CP_BUF_BGN, CP_BUF_END, CcOpTimeout_Buf);
    CHECK_OP(CP_ENQ_BGN, CP_ENQ_END, CcOpTimeout_Enq);
    CHECK_OP(CP_OUT_BGN, CP_OUT_END, CcOpTimeout_Dispatch);
    //
    if (START_STOP_OK && bStreaming) {
        CHECK_WAIT(CP_REQ_RET, CP_REQ_REV,
            CcWaitOvertime_Request);
        CHECK_WAIT(CP_CB_SYNC_RET, CP_CB_SYNC_REV,
            CcWaitOvertime_3aCbSyncDone);
        CHECK_WAIT(CP_CB_PROC_RET, CP_CB_PROC_REV,
            CcWaitOvertime_3aCbProcFinish);
    }
    //
    // for no request arrival
    if (START_STOP_OK && bStreaming &&
        (mData.mSetFn <= mData.mRevFn) && // include non-set and set-first-request-Fn-0 cases // include dummy request (SetFn < 0) // since all of the accept request set to 3A
        (mData.mTime[CP_REQ_RET] > mData.mTime[CP_REQ_REV]) &&
        (DIFF_NOW(CP_REQ_RET, P1_GENERAL_API_CHECK_US)) &&
        (!((mData.mAcceptFn > mData.mRevFn) && // exclude the case about the next request is arrival but rejected since not-available
        (mData.mAcceptResult == REQ_REV_RES_REJECT_NOT_AVAILABLE) &&
        (mData.mTime[CP_REQ_ACCEPT] > mData.mTime[CP_SET_END])))) {
        ADD(CC_DEDUCE, CcDeduce_FwNoRequestAccept);
    }
    //
    // for 3A no first callback
    if (mData.mTime[CP_CB_PROC_REV] == 0 && mData.mTime[CP_START_SET_END] > 0) {
        if (mData.mStartSetType == START_SET_CAPTURE) {
            ADD(CC_DEDUCE, CcDeduce_3aNoFirstCbInCapture);
        } else if (mData.mStartSetType == START_SET_REQUEST) {
            ADD(CC_DEDUCE, CcDeduce_3aNoFirstCbInRequest);
        } else {
            ADD(CC_DEDUCE, CcDeduce_3aNoFirstCbInGeneral);
        }
    }
    //
    // for 3A callback stuck-with / look-for
    CHECK_STUCK(CP_SET_BGN, CP_SET_END, CcDeduce_3aStuckWithSet);
    CHECK_STUCK(CP_BUF_BGN, CP_BUF_END, CcDeduce_3aStuckWithBuf);
    CHECK_STUCK(CP_ENQ_BGN, CP_ENQ_END, CcDeduce_3aStuckWithEnq);
    if (HAS(CC_DEDUCE, CcDeduce_3aStuckWithSet) ||
        HAS(CC_DEDUCE, CcDeduce_3aStuckWithBuf) ||
        HAS(CC_DEDUCE, CcDeduce_3aStuckWithEnq)) {
        // 3A-stuck-clue defined
    } else if (START_STOP_OK && bStreaming) { // HAS(CcWaitOvertime_3aCbSyncDone) || HAS(CcWaitOvertime_3aCbProcFinish)
        if (mData.mTime[CP_CB_PROC_RET] > mData.mTime[CP_CB_SYNC_RET]) { // the last CB is PROC_FINISH
            if ((mData.mTime[CP_CB_PROC_RET] > mData.mTime[CP_CB_PROC_REV]) &&
                (mData.mTime[CP_CB_SYNC_REV] > mData.mTime[CP_CB_PROC_RET]) &&
                (DIFF_NOW(CP_CB_PROC_RET, P1_GENERAL_STUCK_JUDGE_US))) {
                ADD(CC_DEDUCE, CcDeduce_3aLookForCbSyncDone);
            }
        } else { // the last CB is SYNC_DONE
            if ((mData.mTime[CP_CB_SYNC_RET] > mData.mTime[CP_CB_SYNC_REV]) &&
                (mData.mTime[CP_CB_PROC_REV] > mData.mTime[CP_CB_SYNC_RET]) &&
                (DIFF_NOW(CP_CB_SYNC_RET, P1_GENERAL_STUCK_JUDGE_US))) {
                ADD(CC_DEDUCE, CcDeduce_3aLookForCbProcFinish);
            }
        }
    }
    //
    // for DRV DeQ case
    if (mData.mTime[CP_DEQ_BGN] > mData.mTime[CP_DEQ_END]) {
        if ((mData.mNowTime - mData.mTime[CP_DEQ_BGN]) >
            P1_GENERAL_WAIT_OVERTIME_US) {
            if (mData.mEnqMn > mData.mDeqMn) {
                if (((mData.mNowTime - mData.mTime[CP_ENQ_END]) >
                    P1_GENERAL_WAIT_OVERTIME_US) ||
                    (mData.mEnqMn >
                    (mData.mDeqMn + (mBurstNum * P1NODE_DEF_QUEUE_DEPTH)))) {
                    ADD(CC_DEDUCE, CcDeduce_DrvDeqDelay);
                }
            }
        }
    }
    //
    //
    #undef START_STOP_OK
    //
    if (mCode != CC_NONE || bForceToPrint) {
        MY_LOGI0(P1_LOG_NOTE_TAG P1_LOG_LINE_BGN);
        MY_LOGI0(P1_LOG_NOTE_TAG " ClueCode_ALL[0x%" PRIx64 "]", mCode);
        //
        MBOOL clueCP[LogInfo::CP_MAX];
        std::vector< LogInfo::CheckPoint > vCP;
        ::memset(clueCP, 0, sizeof(clueCP));
        for (MUINT8 i = 0; i < CC_AMOUNT_MAX; i++) {
            MUINT64 bit = (MUINT64)((MUINT64)(0x1) << i);
            //MY_LOGI0(P1_LOG_DUMP_TAG " ClueCode-Bit[0x%" PRIx64 "] @ (%d)", bit, i);
            if ((bit & mCode) == bit) {
                android::String8 str("");
                bitStr(bit, &str);
                MY_LOGI0(P1_LOG_NOTE_TAG " ClueCode-bit[0x%" PRIx64 "] = %s ",
                    bit, str.string());
                vCP.clear();
                bitTag(bit, vCP);
                for (size_t j = 0; j < vCP.size(); j++) {
                    if (vCP[j] < LogInfo::CP_MAX) {
                        clueCP[ vCP[j] ] = MTRUE;
                    }
                }
            }
        }
        for (int cp = LogInfo::CP_FIRST; cp < LogInfo::CP_MAX; cp ++) {
            if (clueCP[cp]) {
                android::String8 str(P1_LOG_NOTE_TAG);
                read((LogInfo::CheckPoint)cp, &str);
                MY_LOGI0("%s", str.string());
            }
        };
        //
        MY_LOGI0(P1_LOG_NOTE_TAG P1_LOG_LINE_END);
    }
    return;
}


/******************************************************************************
 *
 ******************************************************************************/
void
LogInfo::
inspect(LogInfo::InspectType type, char const * info)
{
    if (!getActive()) {
        return;
    }
    // excluding concurrence per-frame memo set/get
    RWLock::AutoWLock _wl(mLock);
    //
    MBOOL routine = MFALSE;
    switch (type) {
        case IT_PERIODIC_CHECK:
        case IT_STOP_NO_REQ_IN_GENERAL:
            routine = MTRUE;
            break;
        default:
            break;
    };
    analyze(!routine);
    if (routine && (mCode == CC_NONE)) {
        return; // no need to dump
    }
    //
    if (type < LogInfo::IT_MAX) {
        MY_LOGI0(P1_LOG_DUMP_TAG " [Burst:%d][Type:%d] %s", mBurstNum, type,
            mTexts[type].main);
    }
    //
    if (info != NULL) {
        MY_LOGI0(P1_LOG_DUMP_TAG " [Info] %s", info);
    }
    //
    MY_LOGI0(P1_LOG_DUMP_TAG P1_LOG_LINE_BGN);
    for (int cp = LogInfo::CP_FIRST; cp < LogInfo::CP_MAX; cp ++) {
        android::String8 str(P1_LOG_DUMP_TAG);
        read((LogInfo::CheckPoint)cp, &str);
        MY_LOGI0("%s", str.string());
    };
    MY_LOGI0(P1_LOG_DUMP_TAG P1_LOG_LINE_END);
    return;
};


#endif


};//namespace NSP1Node
};//namespace v3
};//namespace NSCam



