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

#ifndef _MTK_HAL_CAMCLIENT_FDCBCLIENT_H_
#define _MTK_HAL_CAMCLIENT_FDCBCLIENT_H_
//
#include "../MyUtils.h"
using namespace android;
using namespace MtkCamUtils;
//
#include <faces.h>
#include <system/camera.h>

//static int                bufScaleX[11] = {320, 256, 204, 160, 128, 104, 80, 64, 52, 40, 32};
//static int                bufScaleY[11] = {240, 192, 152, 120, 96, 80, 60, 48, 40, 32, 24};

namespace android {
namespace NSCamClient {
namespace NSFDClient {



struct CMD_Q_T
{
    MBOOL   isFDCB;
    MBOOL   isSDCB;
    MBOOL   isGSCB;
    MBOOL   isASDCB;
    MBOOL   isStopCmd;
    camera_frame_metadata_t FrameMeta;
    camera_face_t Faces[15];
    int     ASDScene;
};

/******************************************************************************
 *  Preview Client Handler.
 ******************************************************************************/
class FDCBClient : public Thread
{
public:     ////                    Instantiation.
    //
    static sp<FDCBClient>           createInstance();
                                    FDCBClient();
    virtual                         ~FDCBClient();

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////

    //
    //
    virtual void                    setCallbacks(sp<CamMsgCbInfo> const& rpCamMsgCbInfo);

    virtual int                     postCommand(CMD_Q_T &cmd);


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Operations in base class Thread
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////
    // Ask this object's thread to exit. This function is asynchronous, when the
    // function returns the thread might still be running. Of course, this
    // function can be called from a different thread.
    virtual void                    requestExit();

    // Good place to do one-time initializations
    virtual status_t                readyToRun();

private:
    // Derived class must implement threadLoop(). The thread starts its life
    // here. There are two ways of using the Thread object:
    // 1) loop: if threadLoop() returns true, it will be called again if
    //          requestExit() wasn't called.
    // 2) once: if threadLoop() returns false, the thread will exit upon return.
    virtual bool                    threadLoop();

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Command Queue.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

protected:  ////                    Operations.
    virtual bool                    getCommand(CMD_Q_T &cmd);

    inline  int32_t                 getThreadId() const    { return mi4ThreadId; }

    bool                            performCallback(CMD_Q_T &cmd);

protected:  ////                    Data Members.
    List<CMD_Q_T>                   mCmdQue;
    Mutex                           mCmdQueMtx;
    Condition                       mCmdQueCond;    //  Condition to wait: [ ! exitPending() && mCmdQue.empty() ]
    int32_t                         mi4ThreadId;

    sp<CamMsgCbInfo>                mpCamMsgCbInfo;         //  Pointer to Camera Message-Callback Info.
};


}; // namespace NSPreviewClient
}; // namespace NSCamClient
}; // namespace android
#endif  //_MTK_HAL_CAMCLIENT_PREVIEW_PREVIEWCLIENT_H_

