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

#ifndef _MTKCAM_HWNODE_P2_CROPPER_H_
#define _MTKCAM_HWNODE_P2_CROPPER_H_

#include "P2_Header.h"
#include "P2_LMVInfo.h"
#include "P2_Param.h"

namespace P2
{

class Cropper
{
public:
    enum CropMask
    {
        USE_RESIZED     = 0x01,
        USE_EIS_12      = 0x02,
        USE_MARGIN      = 0x04,
    };

    Cropper();
    Cropper(const sp<P2Info> &p2Info, const LMVInfo &lmvInfo, const IMetadata *appMeta, const IMetadata *halMeta, const Logger &logger = Logger());
    virtual ~Cropper();

    MBOOL isValid() const;
    MBOOL isEISAppOn() const;
    MSize getSensorSize() const;
    MRect getResizedCrop() const;
    MRect getP1Crop() const;
    MSize getP1OutSize() const;
    const LMVInfo& getLMVInfo() const;
    MCropRect calcViewAngle(const MSize &size, MUINT32 cropFlag) const;
    MCropRect calcViewAngle(const MSize &size, MUINT32 cropFlag, const MSize &margin) const;
    MCropRect calcViewAngle(const MSize &size, MUINT32 cropFlag, const Logger &logger) const;
    MCropRect calcViewAngle(const MSize &size, MUINT32 cropFlag, const MSize &margin, const Logger &logger) const;
    MBOOL refineBoundary(const MSize &imgSize, MCropRect &crop) const;
    MBOOL refineBoundary(const MSize &imgSize, MCropRect &crop, const Logger &logger) const;
    MRect getCropRegion() const;
    MRect getActiveCrop() const;
    MRect toActive(const MCropRect &crop, MBOOL resize) const;
    MVOID dump() const;
    MVOID dump(const Logger &logger) const;

private:
    MBOOL initAppInfo(const IMetadata *inApp);
    MBOOL initHalInfo(const IMetadata *inHal);
    MBOOL initTransform();
    MBOOL initLMV();
    MBOOL queryEISAppOn(const IMetadata *inApp);
    MRect queryCropRegion(const IMetadata *meta, MBOOL eis);
    MVOID prepareLMV();
    MRect clip(const MRect &src, const MRect &box) const;
    MCropRect applyEIS12(const Logger &logger, const MCropRect &src, MBOOL useResize) const;
    MCropRect applyMargin(const Logger &logger, const MCropRect &src, const double &ratioW, const double &ratioH) const;
    MCropRect applyViewRatio(const Logger &logger, const MCropRect &src, const MSize &size) const;

private:
    sp<P2Info> mP2Info;
    LMVInfo mLMVInfo;
    Logger mLogger;

    MBOOL mIsValid;
    MINT32 mSensorMode;
    MBOOL mIsEISAppOn;
    MUINT32 mEISFactor;

    MSize mSensorSize;
    MSize mActiveSize;
    MRect mActiveCrop;
    MRect mSensorCrop;
    MRect mResizedCrop;
    MRect mP1Crop;
    MRect mP1DMA;
    MSize mP1OutSize;

    HwMatrix mActive2Sensor;
    HwMatrix mSensor2Active;
    simpleTransform mSensor2Resized;

    vector_f mActiveLMV;
    vector_f mSensorLMV;
    vector_f mResizedLMV;
};

} // namespace P2

#endif // _MTKCAM_HWNODE_P2_CROPPER_H_
