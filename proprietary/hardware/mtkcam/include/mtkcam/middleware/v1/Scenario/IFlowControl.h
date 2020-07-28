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

#ifndef _MTK_HARDWARE_INCLUDE_SCENARIO_IFLOWCONTROL_H_
#define _MTK_HARDWARE_INCLUDE_SCENARIO_IFLOWCONTROL_H_
//
#include <utils/RefBase.h>
#include <utils/Vector.h>
#include <mtkcam/middleware/v1/LegacyPipeline/buffer/StreamBufferProvider.h>
#include <mtkcam/middleware/v1/IParamsManager.h>
#include <mtkcam/middleware/v1/IParamsManagerV3.h>

using namespace android;
using namespace android::MtkCamUtils;
#include <mtkcam/middleware/v1/camutils/IImgBufQueue.h>
#include <inc/ImgBufProvidersManager.h>

#include "utils/Mutex.h"    // android mutex
using android::Mutex;

typedef NSCam::v3::Utils::HalMetaStreamBuffer HalMetaStreamBuffer;

/******************************************************************************
 *
 ******************************************************************************/
//
namespace NSCam {
namespace v1 {
class INotifyCallback;

namespace NSLegacyPipeline {

class IFlowControl
    : public virtual RefBase
{
public:

    struct ControlType_T{
        enum {
            CONTROL_DEFAULT,
            CONTROL_ENG,
            CONTROL_FEATURE,
            CONTROL_STEREO
        };
    };

    // for sendCommand
    enum eExtCmd
    {
        eExtCmd_define_start = 0x20000000,
        eExtCmd_setLtmEnable,
    };

    static sp< IFlowControl >   createInstance(
                                    char const*                pcszName,
                                    MINT32 const               i4OpenId,
                                    MINT32                     type,
                                    sp<IParamsManagerV3>       pParamsManagerV3,
                                    sp<ImgBufProvidersManager> pImgBufProvidersManager,
                                    sp<INotifyCallback> pCamMsgCbInfo
                                );

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////
    virtual                         ~IFlowControl() {};

    virtual char const*             getName()   const                       = 0;

    virtual int32_t                 getOpenId() const                       = 0;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interfaces for CamAdapter.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    /**
     * Start preview mode.
     */
    virtual status_t                startPreview()                          = 0;

    /**
     * Stop a previously started preview.
     */
    virtual status_t                stopPreview()                           = 0;

    /**
     * Start record mode. When a record image is available a CAMERA_MSG_VIDEO_FRAME
     * message is sent with the corresponding frame. Every record frame must be released
     * by a cameral hal client via releaseRecordingFrame() before the client calls
     * disableMsgType(CAMERA_MSG_VIDEO_FRAME). After the client calls
     * disableMsgType(CAMERA_MSG_VIDEO_FRAME), it is camera hal's responsibility
     * to manage the life-cycle of the video recording frames, and the client must
     * not modify/access any video recording frames.
     */
    virtual status_t                startRecording()                        = 0;

    /**
     * Stop a previously started recording.
     */
    virtual status_t                stopRecording()                         = 0;

    /**
     * Start auto focus, the notification callback routine is called
     * with CAMERA_MSG_FOCUS once when focusing is complete. autoFocus()
     * will be called again if another auto focus is needed.
     */
    virtual status_t                autoFocus()                             = 0;

    /**
     * Cancels auto-focus function. If the auto-focus is still in progress,
     * this function will cancel it. Whether the auto-focus is in progress
     * or not, this function will return the focus position to the default.
     * If the camera does not support auto-focus, this is a no-op.
     */
    virtual status_t                cancelAutoFocus()                       = 0;

    /**
     * Start pre-capture.
     */
    virtual status_t                precapture(int& flashRequired)          = 0;

    /**
     * Start capture. ONLY for VSS.
     */
    virtual status_t                takePicture()                           = 0;

    /**
     * Set the camera parameters. This returns BAD_VALUE if any parameter is
     * invalid or not supported.
     */
    virtual status_t                setParameters()                         = 0;

    /**
     * Reset the scenario control. This returns BAD_VALUE if any parameter is
     * invalid or not supported.
     */
    virtual status_t                changeToPreviewStatus()                 = 0;

    /**
     * Send command to camera driver.
     */
    virtual status_t                sendCommand(
                                        int32_t cmd,
                                        int32_t arg1,
                                        int32_t arg2
                                    )                                       = 0;

    /**
     *
     */
    virtual status_t                dump(
                                        int fd,
                                        Vector<String8>const& args
                                    )                                       = 0;

    /**
     *  pause the flow of dynamic switch sensor mode
     */
    virtual void                    pauseSwitchModeFlow()                 = 0;

    /**
     *  resume the flow of dynamic switch sensor mode
     */
    virtual void                    resumeSwitchModeFlow()                = 0;

    /**
     *  pause preview by stop P1Node
     */
    virtual android::status_t       pausePreviewP1NodeFlow()             = 0;

    /**
     *  resume preview by start P1Node
     */
    virtual android::status_t       resumePreviewP1NodeFlow()            = 0;
    /**
     *  get now switch mode status
    */
    virtual int getNowSensorModeStatusForSwitchFlow() = 0;
    /**
     *  set now switch mode status
     */
    virtual void setNowSensorModeStatusForSwitchFlow(int nowStatus) = 0;
    /**
     *
     */
    mutable Mutex                   mSwitchModeLock;
    mutable Mutex mNowSensorModeStatusForSwitchFlowLock;
    int mNowSensorModeStatusForSwitchFlow = 0;//0:disable switch  1:3HDR mode  2:binning mode

public:
    enum NowSensorModeStatusForSwitchFlow
    {
        eSensorModeStatus_Disable,//disable dynamic switch sensor mode flow
        eSensorModeStatus_3HDR,//3HDR on: preview sensor mode + 3HDR  (for low iso)
        eSensorModeStatus_Binning,//3HDR on: preview sensor mode + binning	(for high iso)
    };


};


/******************************************************************************
 *
 ******************************************************************************/
};  //namespace NSLegacyPipeline
};  //namespace v1
};  //namespace NSCam
#endif  //_MTK_HARDWARE_INCLUDE_SCENARIO_IFLOWCONTROL_H_
