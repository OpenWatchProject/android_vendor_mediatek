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
 * MediaTek Inc. (C) 2018. All rights reserved.
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
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFdTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

#ifndef _MTK_CAMERA_STREAMING_FEATURE_PIPE_OUTPUT_CONTROL_H_
#define _MTK_CAMERA_STREAMING_FEATURE_PIPE_OUTPUT_CONTROL_H_

#include <atomic>
#include <map>
#include "MtkHeader.h"
#include <featurePipe/core/include/IIBuffer.h>

namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {

class OutputControl
{
public:
    using P2IO = NSCam::Feature::P2Util::P2IO;
    enum GeneralTarget
    {
        GENERAL_DISPLAY = 1 << 0,
        GENERAL_RECORD  = 1 << 1,
        GENERAL_EXTRA   = 1 << 2,
        GENERAL_FD      = 1 << 3,
        GENERAL_ASYNC   = 1 << 4,
    };

    enum SensorTarget
    {
        SENSOR_PHYSICAL = 1 << 0,
        SENSOR_LARGE    = 1 << 1,
    };

    OutputControl(const std::vector<MUINT32> &allSensorID);
    ~OutputControl();

    MBOOL registerFillOut(GeneralTarget target, P2IO &out, MBOOL logw = MTRUE);
    MBOOL registerFillOut(GeneralTarget target, android::sp<IIBuffer> &out, MBOOL logw = MTRUE);
    MBOOL registerFillOuts(GeneralTarget target, std::vector<P2IO> &outList, MBOOL logw = MTRUE);
    MBOOL registerFillOut(SensorTarget target, MUINT32 sensorID, P2IO &out, MBOOL logw = MTRUE);
    MBOOL registerFillOuts(SensorTarget target, MUINT32 sensorID, std::vector<P2IO> &outList, MBOOL logw = MTRUE);

private:
    MBOOL registerGeneral(GeneralTarget target, MBOOL logw);
    MBOOL registerSensor(SensorTarget target, MUINT32 sensorID, MBOOL logw);

private:
    std::atomic<int> mGeneralOut;
    std::map<MUINT32, std::atomic<int>> mSensorOut;

};

} // namespace NSFeaturePipe
} // namespace NSCamFeature
} // namespace NSCam
#endif // _MTK_CAMERA_STREAMING_FEATURE_PIPE_OUTPUT_CONTROL_H_
