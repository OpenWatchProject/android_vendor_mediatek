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

#define LOG_TAG "MtkCam/CamClient/FDCBClient"
//
#include "FDClient.CBThread.h"
using namespace NSCamClient;
using namespace NSFDClient;
//
#include <sys/prctl.h>
#include <sys/resource.h>
#include <cutils/properties.h>

#include <mtkcam/utils/fwk/MtkCamera.h>
//



/******************************************************************************
*
*******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_LOGV("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( (cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( (cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)


/******************************************************************************
 *
 ******************************************************************************/
sp<FDCBClient>
FDCBClient::
createInstance()
{
    return  new FDCBClient();
}
/******************************************************************************
 *
 ******************************************************************************/
FDCBClient::
FDCBClient()
    : mCmdQue()
    , mCmdQueMtx()
    , mCmdQueCond()
    , mi4ThreadId(0)
    , mpCamMsgCbInfo(new CamMsgCbInfo)
{
    MY_LOGD("Create FD CB thread ++");
}
/******************************************************************************
 *
 ******************************************************************************/
FDCBClient::
~FDCBClient()
{
    MY_LOGD("destory FD CB thread --");
}

/******************************************************************************
 * Set camera message-callback information.
 ******************************************************************************/
void
FDCBClient::
setCallbacks(sp<CamMsgCbInfo> const& rpCamMsgCbInfo)
{
    *mpCamMsgCbInfo = *rpCamMsgCbInfo;
}
/******************************************************************************
 *
 ******************************************************************************/
// Ask this object's thread to exit. This function is asynchronous, when the
// function returns the thread might still be running. Of course, this
// function can be called from a different thread.
void
FDCBClient::
requestExit()
{
    MY_LOGD("+");
    Thread::requestExit();
    //
    CMD_Q_T cmd;
    cmd.isStopCmd = true;
    postCommand(cmd);
    //
    MY_LOGD("-");
}


/******************************************************************************
 *
 ******************************************************************************/
// Good place to do one-time initializations
status_t
FDCBClient::
readyToRun()
{
    ::prctl(PR_SET_NAME,"FDCBClient@Preview", 0, 0, 0);
    //
    mi4ThreadId = ::gettid();

    //  thread policy & priority
    //  Notes:
    //      Even if pthread_create() with SCHED_OTHER policy, a newly-created thread
    //      may inherit the non-SCHED_OTHER policy & priority of the thread creator.
    //      And thus, we must set the expected policy & priority after a thread creation.
    int const policy    = SCHED_OTHER;
    int const priority  = 0;
    //
    //
    struct sched_param sched_p;
    ::sched_getparam(0, &sched_p);
    sched_p.sched_priority = priority;  //  Note: "priority" is nice value
    sched_setscheduler(0, policy, &sched_p);
    setpriority(PRIO_PROCESS, 0, priority);
    //
    //
    MY_LOGD(
        "policy:(expect, result)=(%d, %d), priority:(expect, result)=(%d, %d)"
        , policy, ::sched_getscheduler(0)
        , priority, getpriority(PRIO_PROCESS, 0)
    );

    return NO_ERROR;
}


/******************************************************************************
 *
 ******************************************************************************/
int
FDCBClient::
postCommand(CMD_Q_T &cmd)
{
    Mutex::Autolock _lock(mCmdQueMtx);
    //
    if  ( ! mCmdQue.empty() )
    {
        //Command::EID const& rBegCmd = *mCmdQue.begin();
        MY_LOGD("que size:%d > 0 ", mCmdQue.size());
        if(!(cmd.isStopCmd)) {
            MY_LOGD("drop cb command since command queue is not empty, isFD(%d), isSD(%d), isGS(%d), isASD(%d)",
                                                                      cmd.isFDCB, cmd.isSDCB, cmd.isGSCB, cmd.isASDCB);
            return false;
        }
    }
    //
    mCmdQue.push_back(cmd);
    mCmdQueCond.broadcast();
    //
    //MY_LOGD("- new command::%d", cmd);
    return true;
}


/******************************************************************************
 *
 ******************************************************************************/
bool
FDCBClient::
getCommand(CMD_Q_T &cmd)
{
    bool ret = false;
    //
    Mutex::Autolock _lock(mCmdQueMtx);
    //
    //MY_LOGD("+ que size(%d)", mCmdQue.size());
    //
    //  Wait until the queue is not empty or this thread will exit.
    while   ( mCmdQue.empty() && ! exitPending() )
    {
        status_t status = mCmdQueCond.wait(mCmdQueMtx);
        if  ( NO_ERROR != status )
        {
            MY_LOGW("wait status(%d), que size(%d), exitPending(%d)", status, mCmdQue.size(), exitPending());
        }
    }
    //
    if  ( ! mCmdQue.empty() && ! exitPending())
    {
        //  If the queue is not empty, take the first command from the queue.
        ret = true;
        cmd = *mCmdQue.begin();
        mCmdQue.erase(mCmdQue.begin());
        //MY_LOGD("command: %p", &cmd);
    }
    //
    // MY_LOGD("- que size(%d), ret(%d)", mCmdQue.size(), ret);
    return  ret;
}


/******************************************************************************
 *
 ******************************************************************************/
bool
FDCBClient::
threadLoop()
{
    CMD_Q_T cmdQ;
    bool ret = false;
    if(getCommand(cmdQ)) {
        ret = true;
        performCallback(cmdQ);
    }
    return  ret;
}

/******************************************************************************
 *
 ******************************************************************************/
bool
FDCBClient::
performCallback(CMD_Q_T &cmd)
{

    bool ret = false;
    if(cmd.isStopCmd)
        return ret;
    MY_LOGD("performCB, isFD(%d), isSD(%d), isGS(%d), isASD(%d)", cmd.isFDCB, cmd.isSDCB, cmd.isGSCB, cmd.isASDCB);
    if(!exitPending() && cmd.isFDCB) {
        camera_memory_t* dummyBuffer = mpCamMsgCbInfo->mRequestMemory(-1, 1, 1, mpCamMsgCbInfo->mCbCookie);
        if  ( dummyBuffer )
        {
            cmd.FrameMeta.faces = cmd.Faces;
            if(!exitPending()) {
                mpCamMsgCbInfo->mDataCb(
                    CAMERA_MSG_PREVIEW_METADATA,
                    dummyBuffer,
                    0,
                    &(cmd.FrameMeta),
                    mpCamMsgCbInfo->mCbCookie
                );
            }
            dummyBuffer->release(dummyBuffer);
        }
    }
    if(!exitPending() && cmd.isSDCB) {
        mpCamMsgCbInfo->mNotifyCb(
            MTK_CAMERA_MSG_EXT_NOTIFY,
            MTK_CAMERA_MSG_EXT_NOTIFY_SMILE_DETECT,
            0,
            mpCamMsgCbInfo->mCbCookie
        );
    }
    if(!exitPending() && cmd.isGSCB) {
        mpCamMsgCbInfo->mNotifyCb(
            MTK_CAMERA_MSG_EXT_NOTIFY,
            MTK_CAMERA_MSG_EXT_NOTIFY_GESTURE_DETECT,
            0,
            mpCamMsgCbInfo->mCbCookie
        );
    }
    if(!exitPending() && cmd.isASDCB) {
        mpCamMsgCbInfo->mNotifyCb(
            MTK_CAMERA_MSG_EXT_NOTIFY,
            MTK_CAMERA_MSG_EXT_NOTIFY_ASD,
            cmd.ASDScene,
            mpCamMsgCbInfo->mCbCookie
        );
    }
    return  ret;
}


