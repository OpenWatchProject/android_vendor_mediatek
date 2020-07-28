/* Copyright Statement: *
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

#ifndef _MTK_HARDWARE_INCLUDE_MTKCAM_FEATUREIO_3DNR_HAL_BASE_H_
#define _MTK_HARDWARE_INCLUDE_MTKCAM_FEATUREIO_3DNR_HAL_BASE_H_

#include <mtkcam/def/common.h>
#include <mtkcam/utils/metadata/IMetadata.h>
#include <mtkcam3/feature/3dnr/3dnr_defs.h>

using namespace NSCam;
using namespace android;
using NSCam::NR3D::NR3DHwParam;
using NSCam::NR3D::NR3DMVInfo;
using NSCam::NR3D::GyroData;
using NSCam::NR3D::NR3DTuningInfo;
using NSCam::NR3D::NR3DHALParam;
using NSCam::NR3D::NR3DHALResult;
using NSCam::NR3D::NR3DRSCInfo;

namespace NS3Av3 {

    class IHal3A;

};

#if 0
namespace NSCam {

    class IImageBuffer;

};
#endif
/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {
namespace NSIoPipe {
namespace NSPostProc { //for NR3D* &pNr3dParam

class Hal3dnrBase
{
public:     ////                    Operations.
    //
    static  Hal3dnrBase* createInstance(char const *userName, const MUINT32 sensorIdx);
    virtual void           destroyInstance(char const *userName, const MUINT32 sensorIdx) = 0;
    virtual                ~Hal3dnrBase() {};

    virtual MBOOL          init(const char *userName) = 0;
    virtual MBOOL          uninit(const char *userName) = 0;

    virtual MBOOL          update3DNRMvInfo(
                                const NR3DHALParam& nr3dHalParam,
                                // input
                                void *pTnrLib, const MUINT32 featMask,
                                const MUINT32 nLayers,
                                const MSize srcCropSizes[],
                                const MSize dstSizes[],
                                const MINT32 uniqueKey,
                                const MINT32 requestNo, const MINT32 frameNo,
                                const NR3DMVInfo& lmvInfo,
                                const NR3DRSCInfo& rscInfo,
                                // output
                                NR3D::NR3DMVInfo outGmvInfoResults[],
                                MBOOL &outIsGMVInfoUpdated);
    virtual MBOOL          update3DNRMvInfo(const NR3DHALParam& nr3dHalParam,
                               NR3DMVInfo &gmvInfoResult, MBOOL &outIsGMVInfoUpdated) = 0; // 3dnrLib only
    virtual MBOOL          do3dnrFlow(const NR3DHALParam& nr3dHalParam,
                               NR3DHALResult &nr3dHalResult) = 0;

    // !!NOTES: legacy = isp50 or older
    virtual void           configNR3D_legacy(void *pTuningData, void *p3A, const MRect& dstRect,
                               const NR3DHwParam &nr3dHwParam) = 0;
    virtual MBOOL          updateISPMetadata(NSCam::IMetadata *pMeta_InHal,
                               const NR3DTuningInfo &tuningInfo) = 0;

};

}
}
}

#endif  //_MTK_HARDWARE_INCLUDE_MTKCAM_FEATUREIO_3DNR_HAL_BASE_H_
