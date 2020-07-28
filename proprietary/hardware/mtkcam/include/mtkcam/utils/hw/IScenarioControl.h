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

#ifndef _MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_UTILS_HW_ISCENARIOCONTROL_H_
#define _MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_UTILS_HW_ISCENARIOCONTROL_H_
//
#include <mtkcam/def/common.h>
#include <utils/RefBase.h>


/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {

#define FEATURE_CFG_ENABLE_MASK(x,y)       (x) |= (1<<(y))
#define FEATURE_CFG_IS_ENABLED(x,y)        (((x) & (1<<(y))) ? MTRUE: MFALSE)

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Bandwidth control & dvfs
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class IScenarioControl
    : public virtual android::RefBase
{
public:
    enum
    {
        Scenario_None,
        Scenario_NormalPreivew,
        Scenario_ZsdPreview,
        Scenario_VideoRecord,
        Scenario_VSS,
        Scenario_Capture,
        Scenario_ContinuousShot,
        Scenario_StreamingWithJpeg = Scenario_ContinuousShot, //hal3
        Scenario_VideoTelephony,
        Scenario_HighSpeedVideo,
    };

    enum
    {
        FEATURE_NONE = 0,
        FEATURE_ADV_EIS,
        FEATURE_IVHDR,
        FEATURE_MVHDR,
        FEATURE_ZVHDR,
        FEATURE_VFB,
        FEATURE_DUAL_PD,
        FEATURE_VSDOF_PREVIEW,
        FEATURE_VSDOF_RECORD,
        FEATURE_STEREO_CAPTURE,
        FEATURE_BMDENOISE_PREVIEW,
        FEATURE_BMDENOISE_CAPTURE,
        FEATURE_BMDENOISE_MFHR_CAPTURE,
        FEATURE_DUALZOOM_PREVIEW,
        FEATURE_DUALZOOM_RECORD,
        FEATURE_DUALZOOM_FUSION_CAPTURE,
        FEATURE_ADV_EIS_4K,
    };

    struct ControlParam
    {
        MINT32              scenario;
        MSize               sensorSize;
        MINT32              sensorFps;
        MINT32              featureFlag;
        MBOOL               enableBWCControl;
        MSize               videoSize;
        MINT32              camMode;
        MINT32              supportCustomOption;
        bool                enableDramClkControl;
        MINT32              dramOPPLevel;
        ControlParam()
            :scenario(Scenario_None),
             sensorSize(MSize(0,0)),
             sensorFps(0),
             featureFlag(FEATURE_NONE),
             enableBWCControl(MTRUE),
             videoSize(MSize(0,0)),
             camMode(0),
             supportCustomOption(0),
             enableDramClkControl(MFALSE),
             dramOPPLevel(0)
             {}
    };

public:
    static android::sp<IScenarioControl>
                                    create(MINT32 const openId);

public:
    virtual MERROR                  enterScenario(
                                        ControlParam const& param
                                    )                                       = 0;

    virtual MERROR                  enterScenario(
                                        MINT32 const scenario
                                    )                                       = 0;

    virtual MERROR                  exitScenario()                          = 0;
};


/******************************************************************************
 *
 ******************************************************************************/
};  //namespace NSCam
#endif  //_MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_UTILS_HW_ISCENARIOCONTROL_H_
