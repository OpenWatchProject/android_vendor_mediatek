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

#ifndef _MTK_HAL_CAMADAPTER_MTKSTEREO_INC_MTKSTEREOCAMADAPTER_H_
#define _MTK_HAL_CAMADAPTER_MTKSTEREO_INC_MTKSTEREOCAMADAPTER_H_
//
/*******************************************************************************
*
*******************************************************************************/

#include <inc/IState.h>
using namespace NSCamState;
//
#include "inc/v3/CaptureCmdQueThread.h"
#include "inc/v3/ZipImageCallbackThread.h"
#include <mtkcam/middleware/v1/camutils/FrameworkCBThread.h>
//
#include <mtkcam/middleware/v1/IShot.h>
using namespace NSShot;
//
#include <vector>
using namespace std;
//
#include "EngParam.h"
//
#include <mtkcam/middleware/v1/Scenario/IFlowControl.h>
#include <mtkcam/middleware/v1/LegacyPipeline/processor/StreamingProcessor.h>
//
#include <mtkcam/aaa/IHal3A.h>
using namespace NS3Av3;
//
#include <mtkcam/utils/metastore/IMetadataProvider.h>
//
namespace android {
namespace NSStereoAdapter {
//
class CamAdapter : public BaseCamAdapter
                 , public IStateHandler
                 , public ICaptureCmdQueThreadHandler
                 , public IShotCallback
                 , public NSCam::v1::INotifyCallback
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  ICamAdapter Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////

    /**
     * Initialize the device resources owned by this object.
     */
    virtual bool                    init();

    /**
     * Uninitialize the device resources owned by this object. Note that this is
     * *not* done in the destructor.
     */
    virtual bool                    uninit();

    /**
     * Start preview mode.
     */
    virtual status_t                startPreview();

    /**
     * Stop a previously started preview.
     */
    virtual void                    stopPreview();

    /**
     * Returns true if preview is enabled.
     */
    virtual bool                    previewEnabled() const;

    /**
     * Start record mode. When a record image is available a CAMERA_MSG_VIDEO_FRAME
     * message is sent with the corresponding frame. Every record frame must be released
     * by a cameral hal client via releaseRecordingFrame() before the client calls
     * disableMsgType(CAMERA_MSG_VIDEO_FRAME). After the client calls
     * disableMsgType(CAMERA_MSG_VIDEO_FRAME), it is camera hal's responsibility
     * to manage the life-cycle of the video recording frames, and the client must
     * not modify/access any video recording frames.
     */
    virtual status_t                startRecording();

    /**
     * Stop a previously started recording.
     */
    virtual void                    stopRecording();

    /**
     * Returns true if recording is enabled.
     */
    virtual bool                    recordingEnabled() const;

    /**
     * Start auto focus, the notification callback routine is called
     * with CAMERA_MSG_FOCUS once when focusing is complete. autoFocus()
     * will be called again if another auto focus is needed.
     */
    virtual status_t                autoFocus();

    /**
     * Cancels auto-focus function. If the auto-focus is still in progress,
     * this function will cancel it. Whether the auto-focus is in progress
     * or not, this function will return the focus position to the default.
     * If the camera does not support auto-focus, this is a no-op.
     */
    virtual status_t                cancelAutoFocus();

    /**
     * Returns true if capture is on-going.
     */
    virtual bool                    isTakingPicture() const;

    /**
     * Take a picture.
     */
    virtual status_t                takePicture();

    /**
     * Cancel a picture that was started with takePicture.  Calling this
     * method when no picture is being taken is a no-op.
     */
    virtual status_t                cancelPicture();

    /**
     * set continuous shot speed
     */
    virtual status_t                setCShotSpeed(int32_t i4CShotSpeeed);

    /**
     * Set the camera parameters. This returns BAD_VALUE if any parameter is
     * invalid or not supported.
     */
    virtual status_t                setParameters();

    /**
     * Send command to camera driver.
     */
    virtual status_t                sendCommand(int32_t cmd, int32_t arg1, int32_t arg2);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IStateHandler Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////
    virtual status_t                onHandleStartPreview();
    virtual status_t                onHandleStopPreview();
    //
    virtual status_t                onHandlePreCapture();
    virtual status_t                onHandleNormalCapture();
    virtual status_t                onHandleCaptureDone();
    virtual status_t                onHandleCancelCapture();
    //
    virtual status_t                onHandleZSLCapture();

    virtual status_t                onHandleCancelPreviewCapture();
    //
    virtual status_t                onHandleStartRecording();
    virtual status_t                onHandleStopRecording();
    virtual status_t                onHandleVideoSnapshot();

//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    virtual status_t                onHandleCancelVideoSnapshot();
    virtual status_t                onHandleCancelNormalCapture();
    virtual status_t                onHandleCancelZSLCapture();
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    virtual status_t                onHandleNormalCaptureDone();
    virtual status_t                onHandleVideoSnapshotDone();
    virtual status_t                onHandleZSLCaptureDone();

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  ICaptureCmdQueThreadHandler Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////
    virtual bool                    onCaptureThreadLoop();

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IShotCallback Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////
    //  Directly include this inline file to reduce file dependencies since the
    //  interfaces in this file may often vary.
    #include "inc/v3/ImpShotCallback.inl"

private:
    //  [Focus Callback]
    status_t                        init3A();
    void                            uninit3A();
    void                            enableAFMove(bool flag);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  INotifyCallback Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    virtual void                    doNotifyCallback(
                                        int32_t _msgType,
                                        int32_t _ext1,
                                        int32_t _ext2
                                    );

    virtual void                    doDataCallback(
                                        int32_t  _msgType,
                                        void*    _data,
                                        uint32_t _size
                                    );
    virtual void                    doExtCallback(
                                        int32_t _msgType,
                                        int32_t _ext1,
                                        int32_t _ext2
                                    );

    virtual bool                    msgTypeEnabled( int32_t msgType );

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Instantiation.

    virtual                         ~CamAdapter();
                                    CamAdapter(
                                        String8 const&      rAppMode,
                                        int32_t const       i4OpenId,
                                        sp<IParamsManagerV3>  pParamsMgrV3
                                    );

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////

    bool                            updateShotInstance();
    status_t                        prepareToCapture();
    status_t                        forceStopAndCleanPreview();
    status_t                        enableFlashCalibration(int enable);
    int                             getFlashQuickCalibrationResult();
    bool                            ProcessCBMetaData(MUINTPTR const pMeta, ZipImageCallbackThread::callback_type const cbType);

protected:  ////

    MVOID                           dumpTuningParams();

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Helper function
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:
    int                             getSensorScenario();
    bool                           isSupportVideoSnapshot();


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////                    Data Members.
    //
    IStateManager*                  mpStateManager          = nullptr;
    //
    sp<ICaptureCmdQueThread>        mpCaptureCmdQueThread   = nullptr;
    wp<ZipImageCallbackThread>      mpZipCallbackThread     = nullptr;
    sp<IFrameworkCBThread>          mpFrameworkCBThread     = nullptr;
    //
    String8                         msAppMode;

    //
    bool                            mbTakePicPrvNotStop     = false;
    bool                            mbFixFps                = false;
    uint32_t                        mPreviewMaxFps          = 0;
    uint32_t                        mShotMode               = 0;
    //
    int32_t                         mLastVdoWidth           = 0;
    int32_t                         mLastVdoHeight          = 0;
    //
    EngParam                        mEngParam;
    //
    MUINT32                         mLogLevel               = 0;
    //
    sp<IFlowControl>                mpFlowControl           = nullptr;
    // af region, for capture use(temp work run solution)
    String8                         msFocusArea;
    MINT32                          miMaxFocusArea          = 0;
    //
    MBOOL                           mIsRaw16CBEnable        = MFALSE;

    MUINT32                         mFlowControlType = 0;
    MBOOL                           mbFlashOn = MFALSE;
    MBOOL                           mOldFlashSetting = MFALSE;
    MBOOL                           mbCancelAF = MFALSE;
};


};  // namespace NSMtkStereoCamAdapter
};  // namespace android
#endif  //_MTK_HAL_CAMADAPTER_MTKSTEREO_INC_MTKSTEREOCAMADAPTER_H_

