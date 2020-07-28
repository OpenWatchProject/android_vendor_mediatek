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
 * MediaTek Inc. (C) 2016. All rights reserved.
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

#ifndef _MTK_CAMERA_STREAMING_FEATURE_PIPE_FOV_HAL_H_
#define _MTK_CAMERA_STREAMING_FEATURE_PIPE_FOV_HAL_H_

//#include "MtkHeader.h"
#include <mtkcam/def/common.h>
#include <mtkcam/drv/iopipe/PostProc/IHalPostProcPipe.h>
#include <mtkcam/drv/def/IPostProcFeFm.h>
#include <mtkcam3/feature/DualCam/DualCam.Common.h>
#include <mtkcam/drv/def/Dip_Notify_datatype.h>

#if (MTKCAM_HAVE_DUAL_ZOOM_MTK_FOV_SUPPORT == 1)
#define ENABLE_FOV_ALGO (1)
#else
#define ENABLE_FOV_ALGO (0)
#endif
/*
#if ENABLE_FOV_ALGO
#include <MTKFOV.h>
#endif
*/

namespace NSCam {

#define WPE_WARP_DATA_BYTE 4

class FOVHal
{
public:
    enum MTK_FOV_TYPE
    {
        MTK_FOV_TYPE_OFFLINE,
        MTK_FOV_TYPE_ONLINE,
        MTK_FOV_TYPE_CALIBRATE
    };
    enum RAW_RATIO
    {
        RATIO_16_9,
        RATIO_4_3
    };
public:
    struct SizeConfig
    {
        SizeConfig() = delete;
        SizeConfig(
            const MSize& feImgSize,
            const MUINT32& iBlockSize,
            MSize szTeleUseResol,
            MFLOAT zoomFactor);

        MSize mFEImgSize;
        MSize mFEOSize;
        MSize mFMOSize;
        MUINT32 miBlockSize;
        MSize mTeleUseResol;
        //indicate the view of tele is same with zoomed wide view.
        MFLOAT mAnchorZoomFactor;
    };
    struct InitConfig
    {
        MUINT8 *WorkingBuf;
    };
    struct Params
    {
        // Input
        MINT32 ZoomRatio;
        MSize  SrcImgSize;
        DUAL_ZOOM_FOV_INPUT_INFO FOVInfo;
        MINT32 FOVType;
        MBOOL  DoFOV;
        // Input, for calibration
        MBOOL  DoCalibration;
        MINT16* FE_master;
        MINT16* FE_slave;
        MINT16* FM_master;
        MINT16* FM_slave;
        MSize  FECrop_master;
        MSize  FECrop_slave;
        MINT32 MasterID;
        // Output
        MUINT32 *pWarpMapX;
        MUINT32 *pWarpMapY;
        MSize  WarpMapSize;
        MSize  WPEOutput;
        MSize  SensorMargin;
        MSize  ResizeMargin;
        MPoint FOVShift;
        float  FOVScale;
        MINT32 FOVUpdated;
        //MSize OutImageSize;
    };
    struct Configs
    {
        // Input
        MINT32 CamID;
        MINT32 ZoomRatio;
        MSize  SensorSize;
        MSize  CropSize;
        MBOOL  Is4K2K;
        MINT32 EisFactor;
        MSize  MainStreamingSize;
        MSize  SlaveStreamingSize;
    };
public:
    virtual ~FOVHal() {}
    static FOVHal* getInstance();
    static FOVHal::SizeConfig getSizeConfig(RAW_RATIO ratio);
    static MBOOL configTuning_FE(const SizeConfig& config, NSCam::NSIoPipe::FEInfo* pFETuning);
    static MBOOL configTuning_FM(const SizeConfig& config, NSCam::NSIoPipe::FMInfo* pFMTuning);
    static RAW_RATIO queryRatio(MSize inputSize);
    static MBOOL isSupportMultiFov();

    virtual MBOOL init(FOVHal::InitConfig config) = 0;
    virtual MBOOL uninit() = 0;
    virtual MBOOL doFOV(FOVHal::Params& Param) = 0;
    virtual MBOOL getFOVMarginandCrop(FOVHal::Configs config, MSize& Margin, MRect& Crop, float& Scale) = 0;
    virtual MBOOL getFOVCrop(FOVHal::Configs config, MRect& Crop) = 0;
    virtual MBOOL setPipelineConfig(FOVHal::Configs config);
    virtual MBOOL setCameraSyncInfo(int CamId, MBOOL needFlash);
    virtual MBOOL getCameraSyncInfo(int CamId, bool& needFlash);
    virtual MBOOL setCamId(int CamId);
    virtual MBOOL getMasterCamId(int& CamId);
    virtual MBOOL getFEFMSRZ1Info(const FOVHal::Configs& config,
                  DUAL_ZOOM_FOV_FEFM_INFO* pFOVInfo, _SRZ_SIZE_INFO_& rInfo,
                  DUAL_ZOOM_FOV_FEFM_INFO* pFOVInfo2, _SRZ_SIZE_INFO_& rInfo2) = 0;

    virtual MBOOL setCurrentCropInfo(MINT32 CamID, MINT32 ZoomRatio, MRect ActiveCrop, MSize Margin) = 0;
    virtual MBOOL getCurrentCropInfo(MINT32 CamID, MRect &Crop, MSize &Margin, MPoint &Offset, float &Scale) = 0;
private:
    /*#if ENABLE_FOV_ALGO
    MTKFOV* mFovObj;
    MUINT32 mWorkingBufSize;
    MUINT8* mWorkingBuf;
    MINT32  mDumpResult;
    FOVResultInfo mResult;
    #endif*/

};

} // namespace NSCam


#endif
