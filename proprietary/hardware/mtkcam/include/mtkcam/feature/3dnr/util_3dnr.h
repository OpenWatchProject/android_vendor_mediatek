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

#ifndef _MTK_HARDWARE_INCLUDE_MTKCAM_FEATUREIO_UTIL_3DNR_H_
#define _MTK_HARDWARE_INCLUDE_MTKCAM_FEATUREIO_UTIL_3DNR_H_

#include <mtkcam/def/common.h>
#include <utils/RefBase.h>
#include <mtkcam/utils/sys/SensorProvider.h>
#include <mtkcam/feature/3dnr/3dnr_defs.h>


#define CRZ_NR3D_REPLACED_BY_TILE_SRC_CROP

using namespace android;

using NSCam::MRect;
using NSCam::MSize;
using NSCam::MPoint;

#define NR3D_LMV_MAX_GMV_DEFAULT 32

namespace NSCam {

    class IImageBuffer;
    class IMetadata;

namespace NSCamFeature {
namespace NSFeaturePipe {

    class FeaturePipeParam;

};
};
};

namespace NSCam {
namespace NSIoPipe {

    struct MCropRect;

};
};

/******************************************************************************
 *
 * @struct NR3DMVInfo
 * @brief parameter for set nr3d
 * @details
 *
 ******************************************************************************/

struct NR3DMVInfo
{
    enum Status {
        INVALID = 0,
        VALID
    };

    // 3dnr vipi: needs x_int/y_int/gmvX/gmvY
    // ISP smoothNR3D: needs gmvX/gmvY/confX/confY/maxGMV
    MINT32 status;
    MUINT32 x_int;
    MUINT32 y_int;
    MINT32 gmvX;
    MINT32 gmvY;
    MINT32 confX;
    MINT32 confY;
    MINT32 maxGMV;

    NR3DMVInfo()
        : status(INVALID)
        , x_int(0)
        , y_int(0)
        , gmvX(0)
        , gmvY(0)
        , confX(0)
        , confY(0)
        , maxGMV(NR3D_LMV_MAX_GMV_DEFAULT)
        {};
};


class Util3dnr : public LightRefBase<Util3dnr>
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Operations.
    Util3dnr(MINT32 openId);
    virtual ~Util3dnr();

    void    init(MINT32 force3dnr = 0);
    MBOOL   canEnable3dnr(MBOOL isUIEnable, MINT32 iso, MINT32 isoThreshold);
    void    modifyMVInfo(MBOOL canEnable3dnr, MBOOL isIMGO, const MRect &cropP1Sensor,
                const MSize &dstSizeResizer, NR3DMVInfo &mvInfo);
    void    prepareFeatureData(MBOOL canEnable3dnr, const NR3DMVInfo &mvInfo,
                MINT32 iso, MINT32 isoThreshold, MBOOL isCRZMode,
                NSCam::NSCamFeature::NSFeaturePipe::FeaturePipeParam &featureEnqueParams);
    void    prepareISPDataHAL1(MBOOL canEnable3dnr, const NR3DMVInfo &mvInfo, MBOOL isIMGO,
                const MRect &cropP1Sensor, MINT32 iso, MINT32 isoThreshold, MBOOL isCRZMode, MBOOL isSl2eEnable,
                NSCam::IImageBuffer *iBuffer, const NSCam::NSIoPipe::MCropRect &mCropRect,
                NSCam::IMetadata *pMeta_InHal);
    void    prepareISPData(MBOOL canEnable3dnr, const NR3DMVInfo &mvInfo,
                const MSize &inputSize, const MRect &inputCrop, MINT32 iso, MINT32 isoThreshold, MBOOL isSl2eEnable,
                NSCam::IMetadata *pMeta_InHal);
    MBOOL   prepareGyro(
                NSCam::NR3D::GyroData *outGyroData,
                NSCam::NSCamFeature::NSFeaturePipe::FeaturePipeParam *featureEnqueParams);
    void    resetFrame() { mforceFrameReset = MTRUE; }
    MBOOL   is3dnrDebugMode(void);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Variables.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    const MINT32    mOpenId;
    MINT32          mLogLevel;
    MINT32          mDebugLevel;
    MINT32          mforce3dnr;
    MBOOL           mforceFrameReset;
    android::sp<NSCam::Utils::SensorProvider> mpSensorProvider;
};


#endif  //_MTK_HARDWARE_INCLUDE_MTKCAM_FEATUREIO_UTIL_3DNR_H_
