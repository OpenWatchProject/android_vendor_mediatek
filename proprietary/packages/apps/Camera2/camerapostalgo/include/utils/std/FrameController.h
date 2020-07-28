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

#ifndef _MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_UTILS_STD_FRAMECONTROLLER_H_
#define _MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_UTILS_STD_FRAMECONTROLLER_H_

#include <utils/RefBase.h>
#include <utils/String8.h>
#include <utils/Timers.h>

/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {
namespace Utils {

/******************************************************************************
 *
 ******************************************************************************/
class FrameController : public virtual android::RefBase
{
public:
                                FrameController(android::String8 userName);
    virtual                     ~FrameController();

    virtual android::String8    getName();
    virtual void                bufferControl(nsecs_t frameTime);

protected:
    android::String8    mName;
    int32_t             miLogLevel;
    //1: colloect first 5 frame info
    //2: base on stage 1, find useful frame, initial display delay parameters
    //3: start flow control
    int                             miDisplayControlStage;
    int64_t                         mAverageDisplayTime;
    int                             mAverageCount;
    //
    int64_t                         miFirstDelayTime;
    int64_t                         miAdjDelay;
    int64_t                         miP2interval;
    //
    bool                            mbFirstReturnFrame;
    nsecs_t                         mnLastFrameTime;
    nsecs_t                         mnCurrentFrameTime;
    int64_t                         mnFrameWaitTime;
    nsecs_t                         mnLastEnqueSystemTime;
    nsecs_t                         mnOldDisplayDelayTime;
    nsecs_t                         mnNewDisplayDelayTime;
    nsecs_t                         mnMinAdjDisplay;
    nsecs_t                         mnMaxAdjDisplay;
    bool                            mbApplyFrameControl;
    nsecs_t                         mnTimeInterval;
    nsecs_t                         mnFrameInterval;
    int64_t                         mnFrameMaxPlusDelay;
    int64_t                         mnFrameMaxSleep; //ns
    int64_t                         mnFrameMinSleep; //us
    int64_t                         mnAdjSleepTime; //us
    bool                            mbFrameControlReset;
    bool                            mbFrameControlAdj;
};

/******************************************************************************
*
*******************************************************************************/
};  // namespace Utils
};  // namespace NSCam
#endif  //_MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_UTILS_STD_FRAMECONTROLLER_H_
