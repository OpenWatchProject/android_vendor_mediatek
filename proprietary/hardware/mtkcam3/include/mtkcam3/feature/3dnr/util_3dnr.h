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
#include <mtkcam3/feature/3dnr/3dnr_defs.h>



using namespace android;

using NSCam::MRect;
using NSCam::MSize;
using NSCam::MPoint;
using NSCam::NR3D::NR3DHwParam;
using NSCam::NR3D::NR3DMVInfo; // for backward-compatible
using NSCam::NR3D::NR3DTuningInfo;
using NSCam::NR3D::GyroData;

namespace NSCam {
    class IImageBuffer;
    class IMetadata;
};

namespace NSCam {
namespace NSIoPipe {
    struct MCropRect;
};
};

typedef struct PrepareNR3DTuningParam
{
    MUINT8 ispProfile;
    NSCam::IMetadata *pMeta_InApp;
    NSCam::IMetadata *pMeta_InHal;
    MSize inputSize;
    MRect inputCrop;
    NR3DHwParam Nr3dParam;
    NR3DMVInfo Nr3dMvInfo;
} PrepareNR3DTuningParam;

typedef struct Util3dnrParam {

    // === common ===
    MBOOL              logLevel = 0;
    MBOOL              isRscNeeded = MFALSE;
    MINT32             iso = 0;
    MRect              p1CropRect = MRect(0, 0);
    MBOOL              isGyroNeeded = MFALSE;

    // === p2a related ===
    MBOOL              is3dnrOn_p2a = MFALSE;
    MBOOL              isIMGO_p2a = MFALSE;
    MINT32             isoThr_p2a = 0;
    MBOOL              isCRZ_p2a = MFALSE;
    MSize              dstSize_p2a = MSize(0, 0);

    // === dsdn related ===
    MBOOL              is3dnrOn_dsdn = MFALSE;
    MINT32             isoThr_dsdn = 0;
};

typedef struct Util3dnrResult {

    // == common ===
    MBOOL              isValid = MFALSE;
    NR3DMVInfo         mvInfo;
    GyroData*          pGyroData = NULL;

} Util3dnrResult;


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
    void    adjustMVInfo(const MRect &cropP1Sensor, const MSize &dstSizeResizer,
                NR3DMVInfo &mvInfo);
    void    prepareFeatureData(const Util3dnrParam& in, Util3dnrResult &out);
    void    prepareGyro(NSCam::NR3D::GyroData *out_pGyroData);
    void    resetFrame() { mForceFrameReset = MTRUE; }
// === extended functions ===

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Variables.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    const MINT32    mOpenId = 0;
    MINT32          mLogLevel = 0;
    MINT32          mDebugEnable = 0;
    MBOOL           mForceFrameReset = MFALSE;
    android::sp<NSCam::Utils::SensorProvider> mpSensorProvider;
};


#endif  //_MTK_HARDWARE_INCLUDE_MTKCAM_FEATUREIO_UTIL_3DNR_H_

