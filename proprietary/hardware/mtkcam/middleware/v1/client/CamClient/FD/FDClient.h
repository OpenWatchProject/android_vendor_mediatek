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

#ifndef _MTK_HAL_CAMCLIENT_FDCLIENT_H_
#define _MTK_HAL_CAMCLIENT_FDCLIENT_H_
//
#include "../MyUtils.h"
using namespace android;
using namespace MtkCamUtils;
#include <mtkcam/middleware/v1/IParamsManager.h>
#include "inc/IAsdClient.h"
#include <mtkcam/middleware/v1/client/IFDClient.h>
#include "FDBufMgr.h"
#include "FDClient.CBThread.h"
//
#include <faces.h>
#include <system/camera.h>
//Hal3A
#include <mtkcam/aaa/IHal3A.h>
#include <mtkcam/aaa/IHalISP.h> 
//
#include <mtkcam/feature/FaceDetection/fd_hal_base.h>
#include <mtkcam/feature/Gesture/gs_hal_base.h>
#include <mtkcam/feature/ASD/asd_hal_base.h>

#include <mtkcam/feature/HeartRate/hr_hal_base.h>

#if (MTKCAM_HR_MONITOR_SUPPORT == '1')
#include <heartrate_device.h>
#endif

#if (MTKCAM_HR_MONITOR_SUPPORT == '1')
typedef bool   (*HeartrateCallback_t)(HR_detection_result_t const &result, void* user);
#endif

using namespace NS3Av3;

namespace android {
namespace NSCamClient {
namespace NSFDClient {
struct Command
{
    //  Command ID.
    enum EID
    {
        eID_UNKNOWN,
        eID_EXIT,
        eID_WAKEUP,
    };
};
/******************************************************************************
 *  Preview Client Handler.
 ******************************************************************************/
class FDClient : public IFDClient
               , public Thread
               , public IHal3ACb
{
/////////////////////////////////////////////////////////////////////////////////////
//  Public operations
/////////////////////////////////////////////////////////////////////////////////////
public:
                                    FDClient(sp<IParamsManager> pParamsMgr);
    virtual                         ~FDClient();
    // Interfaces implementation
    virtual bool                    init();
    virtual bool                    uninit();
    virtual status_t                sendCommand(int32_t cmd, int32_t arg1, int32_t arg2);
    virtual bool                    stopPreview();
    virtual bool                    takePicture();
    virtual bool                    startPreview();
    virtual bool                    startRecording();
    virtual bool                    stopRecording();
    //
    virtual bool                    setImgBufProviderClient(
                                        sp<IImgBufProviderClient>const& rpClient
                                    );
    virtual void                    setCallbacks(sp<CamMsgCbInfo> const& rpCamMsgCbInfo);
    virtual void                    enableMsgType(int32_t msgType);
    virtual void                    disableMsgType(int32_t msgType);
    // for 3A callback
    virtual void                    doNotifyCb (
                                        MINT32  _msgType,
                                        MINTPTR _ext1,
                                        MINTPTR _ext2,
                                        MINTPTR _ext3
                                    );
/////////////////////////////////////////////////////////////////////////////////////
//  FD internal operations
/////////////////////////////////////////////////////////////////////////////////////
private:
    // thread implement
    virtual void                    requestExit();
    virtual status_t                readyToRun();
    virtual bool                    threadLoop();
    // thread operation
    virtual void                    postCommand(Command::EID cmd);
    virtual bool                    getCommand(Command::EID &cmd);
    virtual void                    onClientThreadLoop();
    inline  int32_t                 getThreadId() const    { return mi4ThreadId; }
    //  FD flow controls
    bool                            startFaceDetection();
    bool                            stopFaceDetection();
    bool                            startHeartrateDetection(int enable);
    bool                            stopHeartrateDetection();
    bool                            isEnabledState();
    bool                            isEnabledFD();
    bool                            isEnabledHR();
    bool                            isMsgEnabled();
    bool                            onStateChanged();
    bool                            doFD(ImgBufQueNode const& rQueNode, bool &rIsDetected_FD, bool &rIsDetected_SD, bool doSD, bool &rIsDetected_GD, bool doGD);
    bool                            performHRCallback(void *HR_result, int type);
    bool                            performCallback(bool isDetected_FD, bool isDetected_SD, bool isDetected_GD, int ASDScene);
    // Image buffer controls
    bool                            initBuffers(sp<IImgBufQueue>const& rpBufQueue);
    void                            uninitBuffers();
    bool                            createDetectedBuffers();
    bool                            createWorkingBuffers(sp<IImgBufQueue>const& rpBufQueue);
    bool                            createDDPWorkBuffers();
    bool                            createFDWorkBuffers();
    void                            destroyDetectedBuffers();
    void                            destroyWorkingBuffers();
    void                            destroyDDPWorkBuffers();
    void                            destroyFDWorkBuffers();
    bool                            waitAndHandleReturnBuffers(sp<IImgBufQueue>const& rpBufQueue, ImgBufQueNode & rQueNode);
    bool                            handleReturnBuffers(sp<IImgBufQueue>const& rpBufQueue, ImgBufQueNode const &rQueNode);

/////////////////////////////////////////////////////////////////////////////////////
//  FDclient internal variables
/////////////////////////////////////////////////////////////////////////////////////
private:
    // Camera control objects
    sp<CamMsgCbInfo>                mpCamMsgCbInfo;
    sp<IParamsManager>              mpParamsMgr;
    sp<IImgBufQueue>                mpImgBufQueue;
    sp<IImgBufProviderClient>       mpImgBufPvdrClient;
    sp<FDCBClient>                  mpCBClient; // thread for callback
    // Command & Thread control
    List<Command::EID>              mCmdQue;
    Mutex                           mCmdQueMtx;
    Condition                       mCmdQueCond;
    int32_t                         mi4ThreadId;
    mutable Mutex                   mModuleMtx;
    mutable Mutex                   mControlMtx;
    mutable Mutex                   mCallbackMtx;
    // Face Detection
    static const MINT32             mDetectedFaceNum = 15;
    static const MINT32             mBufCnt          = 5;
    volatile MINT32                 mIsFDStarted;
    bool                            mIsDetected_FD;
    MUINT32                         mFDBufferWidth;
    MUINT8                          mFDBufferPlanes;
    MINT32                          mRotation_Info;
    MUINT32                         mAEStable;
    halFDBase*                      mpFDHalObj;
    MtkCameraFaceMetadata*          mpDetectedFaces;
    MUINT8*                         mpDDPBuffer;
    MUINT8*                         mpExtractYBuffer;
    MUINT8*                         mpFDWorkingBuffer;
    MUINT32                         mFDWorkingBufferSize;
    // Smile detection
    bool                            mIsDetected_SD;
    bool                            mIsSDenabled;
    // Gesture detection
    halGSBase*                      mpGDHalObj;
    MtkCameraFaceMetadata*          mpDetectedGestures;
    bool                            mIsDetected_GD;
    bool                            mIsGDenabled;
    MUINT32                         mGestureRatio;
    // ASD
    sp<IAsdClient>                  mpASDClient;
    // Heart rate detection
    volatile int32_t                mIsHRStarted;
    volatile int32_t                mIsHRPreStarted;
    int                             mHRMode;
    int                             mISHRenabled;
    int                             mHRStarting;
    halHRBase*                      mpHRObj;
    int                             mIsHRSupported;
    void                            *mpHeartrateUser;
    #if (MTKCAM_HR_MONITOR_SUPPORT == '1')
    HeartrateCallback_t             mHeartrateCb;
    #endif
    // padding
    MUINT32                         mPadding_w;
    MUINT32                         mPadding_h;
    sp<FDBuffer>                    mFDPaddingImg;

    // main face information
    int                             mMainFace_X;
    int                             mMainFace_Y;
    bool                            mIsMainFaceEn;
    // Debug
    int                             mClientFDDumpOPT;
    MUINT32                         mLogLevel;
    MUINT32                         mForceDisableFD;
    MBOOL                           mSetCBDone;
    MINT32                          mHDRDetected;
    MINT32                          mFakeFaceEn;

    mutable Mutex                   mCamSwitchMtx;
    MINT32                          mMainCamId;
};


}; // namespace NSPreviewClient
}; // namespace NSCamClient
}; // namespace android
#endif  //_MTK_HAL_CAMCLIENT_FDCLIENT_H_

