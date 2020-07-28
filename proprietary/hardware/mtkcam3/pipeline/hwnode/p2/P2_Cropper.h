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
//#include "P2_Param.h"

namespace P2
{

class P2Cropper : public Cropper
{
public:
    P2Cropper();
    P2Cropper(const ILog &log, const P2SensorInfo *sensorInfo, const P2SensorData *sensorData, const LMVInfo &lmvInfo);
    virtual ~P2Cropper();

    MBOOL isValid() const;
    MBOOL isEISAppOn() const;
    MSize getSensorSize() const;
    MRect getResizedCrop() const;
    MRect getP1Crop() const;
    MSize getP1OutSize() const;
    MRect getP1BinCrop() const;
    MSize getP1BinSize() const;
    const LMVInfo& getLMVInfo() const;
    MCropRect calcViewAngle(const ILog &log, const MSize &size, MUINT32 cropFlag) const;
    MCropRect calcViewAngle(const ILog &log, const MSize &size, MUINT32 cropFlag, const float cropRatio) const;
    MRectF calcViewAngleF(const ILog &log, const MSize &size, MUINT32 cropFlag, const float widthCropRatio, const float heightCropRatio, const MUINT32 dmaConstrain) const;
    MBOOL refineBoundaryF(const ILog &log, const MSizeF &imgSize, MRectF &crop) const;
    MBOOL refineBoundary(const ILog &log, const MSize &imgSize, MCropRect &crop) const;
    MRect getCropRegion() const;
    MRect getActiveCrop() const;
    MRect toActive(const MCropRect &cropRect, MBOOL resize) const;
    MRect toActive(const MRectF &cropF, MBOOL resize) const;
    MRect toActive(const MRect &crop, MBOOL resize) const;
    MVOID dump(const ILog &log) const;

private:
    static MBOOL needCropperDump();
    MBOOL initAppInfo(const P2SensorData *data);
    MBOOL initHalInfo(const P2SensorData *data);
    MBOOL initTransform();
    MBOOL initLMV();
    MVOID prepareLMV();
    MRect clip(const MRect &src, const MRect &box) const;
    MRectF applyEIS12(const ILog &og, const MRectF &src, MBOOL useResize) const;
    MRectF applyCropRatio(const ILog &log, const MRectF &src, const float widthRatio, const float heightRatio) const;
    MRectF applyViewRatio(const ILog &log, const MRectF &src, const MSize &size) const;

private:
    ILog mLog;
    MUINT32 mSensorID = INVALID_SENSOR_ID;
    LMVInfo mLMVInfo;

    MBOOL mIsValid = MFALSE;
    MINT32 mSensorMode = 0;
    MBOOL mIsEISAppOn = MFALSE;
    MUINT32 mEISFactor;

    MSize mSensorSize;
    MSize mActiveSize;
    MRect mAppCrop;
    MRect mSimulatedAppCrop;
    MRect mActiveCrop;
    MRect mSensorCrop;
    MRect mResizedCrop;
    MRect mP1Crop;
    MRect mP1DMA;
    MSize mP1OutSize;
    MRect mP1BinCrop;
    MSize mP1BinSize;

    HwMatrix mActive2Sensor;
    HwMatrix mSensor2Active;
    simpleTransform mSensor2Resized;

    vector_f mActiveLMV;
    vector_f mSensorLMV;
    vector_f mResizedLMV;
};

} // namespace P2

#endif // _MTKCAM_HWNODE_P2_CROPPER_H_
