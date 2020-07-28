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

#ifndef _MTK_HARDWARE_MTKCAM_ADAPTER_FLOWCONTROL_BASE_H_
#define _MTK_HARDWARE_MTKCAM_ADAPTER_FLOWCONTROL_BASE_H_
//
#include <utils/RefBase.h>
#include <utils/StrongPointer.h>
//
#include <mtkcam/middleware/v1/LegacyPipeline/ILegacyPipeline.h>
#include <mtkcam/middleware/v1/Scenario/IFlowControl.h>
#include <mtkcam/middleware/v1/LegacyPipeline/request/IRequestController.h>
#include <mtkcam/middleware/v1/LegacyPipeline/LegacyPipelineBuilder.h>
#include <mtkcam/utils/hw/HwInfoHelper.h>
#include <mtkcam/middleware/v1/LegacyPipeline/LegacyPipelineUtils.h>
#include <mtkcam/utils/hw/IPlugProcessing.h>
//
#include <mtkcam/aaa/IHal3A.h>
//
#include <mtkcam/utils/hw/IResourceConcurrency.h>
#include <mtkcam/utils/sys/CpuCtrl.h>


#define IMG_1080P_W         (1920)
#define IMG_1080P_H         (1080)
#define IMG_1080P_H_ALIGN   (1088)
#define IMG_1080P_SIZE      (IMG_1080P_W*IMG_1080P_H_ALIGN)

#define SWITCH_MODE_FROM_HIGH_LV_TO_LOW_LV_TH (4.0f)
#define SWITCH_MODE_FROM_LOW_LV_TO_HIGH_LV_TH (8.0f)
#define RESOURCE_CONCURRENCY_TIMEOUT_MS (3000)

using namespace NS3Av3;

/******************************************************************************
 *
 ******************************************************************************/
//
namespace NSCam {
namespace v1 {
namespace NSLegacyPipeline {


class FlowControlBase
    : public IFlowControl
    , public IFeatureFlowControl
    , public IRequestUpdater
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
                                                FlowControlBase(){}
    virtual                                     ~FlowControlBase() {}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    virtual android::status_t                   changeToPreviewStatus();

protected:
    virtual MINT32                              getScenario() const = 0;

    android::sp<IScenarioControl>               enterScenarioControl(MINT32 scenario);

    android::sp<IScenarioControl>               enterScenarioControl(MINT32 scenario, const MSize &sensorSize, const MINT32 &sensorFps, MINT32 featureconfig = 0, MSize videoSize = MSize(0,0), MINT32 camMode = 0);

    virtual MERROR                              decideLcsoImage(
                                                    NSCamHW::HwInfoHelper& helper,
                                                    MUINT32 const bitDepth,
                                                    MSize referenceSize,
                                                    MUINT const usage,
                                                    MINT32 const  minBuffer,
                                                    MINT32 const  maxBuffer,
                                                    sp<IImageStreamInfo>& rpImageStreamInfo
                                                );

    int                                         getPreviewRawBitDepth(NSCamHW::HwInfoHelper& helper);

    virtual void                                pauseSwitchModeFlow();
    virtual void                                resumeSwitchModeFlow();

    virtual android::status_t                   pausePreviewP1NodeFlow() {return OK;}
    virtual android::status_t                   resumePreviewP1NodeFlow() {return OK;}
    virtual int                                 getNowSensorModeStatusForSwitchFlow();
    virtual void                                setNowSensorModeStatusForSwitchFlow(int nowStatus);

protected:
    enum SwitchModeStatus
    {
        eSwitchMode_Undefined,
        eSwitchMode_HighLightMode,       //for high light environment to use high speed sensor mode
        eSwitchMode_LowLightLvMode,      //for low light environment to use binning sensor mode
    };

    SwitchModeStatus                            mSwitchModeStatus = eSwitchMode_Undefined;
    MBOOL                                       mbSwitchModeEnable = MFALSE;
    MBOOL                                       mbPauseSwitchModeFlow = MFALSE;
    MINT32                                      mPauseSwitchCount = 0;
    IHal3A*                                     mpHal3a = NULL;
    MBOOL                                       mbConstruct2ndPipeline = MFALSE;
    android::sp<IResourceConcurrency>           mP1NodeConcurrency = 0;
    android::sp<IResourceConcurrency>           mP2NodeConcurrency = 0;
    CpuCtrl*                                    mpCpuCtrl = 0;
    float                                       mfSwitchModeEnterHighLvTh = 0.0f;
    float                                       mfSwitchModeEnterLowLvTh = 0.0f;
};

/******************************************************************************
*
******************************************************************************/
};  //namespace NSPipelineContext
};  //namespace v1
};  //namespace NSCam
#endif // _MTK_HARDWARE_MTKCAM_ADAPTER_FLOWCONTROL_BASE_H_
