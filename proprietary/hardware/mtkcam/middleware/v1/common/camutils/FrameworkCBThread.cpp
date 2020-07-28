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

#define LOG_TAG "MtkCam/FrameworkCBThread"
//
#include <utils/List.h>
//
#include <mtkcam/utils/std/Log.h>
#include <mtkcam/def/common.h>
//
#include <mtkcam/def/PriorityDefs.h>
using namespace android;
//
#include <mtkcam/middleware/v1/camutils/FrameworkCBThread.h>
using namespace MtkCamUtils;
//
#include <sys/prctl.h>
//
/******************************************************************************
*
*******************************************************************************/
#define ENABLE_LOG_PER_FRAME        (1)


/******************************************************************************
*
*******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_LOGV("[%d::%s] " fmt, getOpenId(), __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("[%d::%s] " fmt, getOpenId(), __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("[%d::%s] " fmt, getOpenId(), __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("[%d::%s] " fmt, getOpenId(), __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("[%d::%s] " fmt, getOpenId(), __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("[%d::%s] " fmt, getOpenId(), __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("[%d::%s] " fmt, getOpenId(), __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( (cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( (cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)

#define FUNC_NAME       MY_LOGD("");
#define FUNC_START      MY_LOGD("+");
#define FUNC_END        MY_LOGD("-");

namespace android {
namespace MtkCamUtils {

/*******************************************************************************
*   IFrameworkCBThread
*******************************************************************************/
class FrameworkCBThread : public IFrameworkCBThread
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Operations in base class Thread
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////
    // Ask this object's thread to exit. This function is asynchronous, when the
    // function returns the thread might still be running. Of course, this
    // function can be called from a different thread.
    virtual void        requestExit();

    // Good place to do one-time initializations
    virtual status_t    readyToRun();

private:
    // Derived class must implement threadLoop(). The thread starts its life
    // here. There are two ways of using the Thread object:
    // 1) loop: if threadLoop() returns true, it will be called again if
    //          requestExit() wasn't called.
    // 2) once: if threadLoop() returns false, the thread will exit upon return.
    virtual bool        threadLoop();

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Operations in IFrameworkCBThread
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////        Interfaces.
    virtual status_t    init();
    virtual void        uninit();
    virtual void        postCB(callback_data const& rCmd);
    virtual void        stopCB();

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    FrameworkCBThread(MINT32 i4OpenId, sp<CamMsgCbInfo> pCamMsgCbInfo);
   ~FrameworkCBThread() {FUNC_NAME;};
   MINT32               getOpenId() const  { return mOpenId; }

protected:
    virtual bool        getCB(callback_data& rCmd);

protected:
    MINT32              mOpenId;
    sp<CamMsgCbInfo>    mpCamMsgCbInfo;
    List<callback_data> mCmdQue;
    Mutex               mCmdQueMtx;
    Condition           mCmdQueCond;    //  Condition to wait: [ ! exitPending() && mCmdQue.empty() ]
    sp<FrameworkCBThread> mMyPtr;
    MBOOL               mbStopCB;
};


}; // namespace NSDefaultAdapter
}; // namespace android

/******************************************************************************
*
*******************************************************************************/
FrameworkCBThread::
FrameworkCBThread(
    int32_t const    i4OpenId,
    sp<CamMsgCbInfo> pCamMsgCbInfo
)
    : IFrameworkCBThread()
    , mpCamMsgCbInfo(pCamMsgCbInfo)
    , mCmdQue()
    , mCmdQueMtx()
    , mCmdQueCond()
    , mOpenId(i4OpenId)
    , mMyPtr(this)
    , mbStopCB(MFALSE)
{
    FUNC_NAME;
}


/******************************************************************************
*
*******************************************************************************/
// Ask this object's thread to exit. This function is asynchronous, when the
// function returns the thread might still be running. Of course, this
// function can be called from a different thread.
void
FrameworkCBThread::
requestExit()
{
    MY_LOGD("+");
    //Thread::requestExit();
    //
    {
        Mutex::Autolock _l(mCmdQueMtx);
        // push finish message to trigger exit
        callback_data const& rCmd = {CALLBACK_TYPE_NOTIFY,0,0,0,NULL,NULL};
        mCmdQue.push_back(rCmd);
        mCmdQueCond.broadcast();
    }
    //join();
    MY_LOGD("-");
}

/******************************************************************************
*
*******************************************************************************/
// Good place to do one-time initializations
status_t
FrameworkCBThread::
readyToRun()
{
    MY_LOGD("+");
    ::prctl(PR_SET_NAME,"CamAdapter@FrameworkCB", 0, 0, 0);
    //  thread policy & priority
    //  Notes:
    //      Even if pthread_create() with SCHED_OTHER policy, a newly-created thread
    //      may inherit the non-SCHED_OTHER policy & priority of the thread creator.
    //      And thus, we must set the expected policy & priority after a thread creation.
    int const policy    = SCHED_OTHER;
    int const priority  = NICE_CAMERA_FRAMEWORK_CB;
    //
    struct sched_param sched_p;
    ::sched_getparam(0, &sched_p);
    //
    //  set
    sched_p.sched_priority = priority;  //  Note: "priority" is real-time priority.
    ::sched_setscheduler(0, policy, &sched_p);
    //
    //  get
    ::sched_getparam(0, &sched_p);
    //
    MY_LOGD(
        "policy:(expect, result)=(%d, %d), priority:(expect, result)=(%d, %d)"
        , policy, ::sched_getscheduler(0)
        , priority, sched_p.sched_priority
    );
    MY_LOGD("-");
    return NO_ERROR;
}

/******************************************************************************
*
*******************************************************************************/
void
FrameworkCBThread::
postCB(callback_data const& rCmd)
{
    Mutex::Autolock _lock(mCmdQueMtx);
    mCmdQue.push_back(rCmd);
    mCmdQueCond.broadcast();
    MY_LOGD("type %d:0x%08X", rCmd.callbackType, rCmd.type);
}

/******************************************************************************
*
*******************************************************************************/
void
FrameworkCBThread::
stopCB()
{
    Mutex::Autolock _lock(mCmdQueMtx);
    mbStopCB = MTRUE;
    MY_LOGI("- mCmdQue.size(%d)", mCmdQue.size());
}

/******************************************************************************
*
*******************************************************************************/
bool
FrameworkCBThread::
getCB(callback_data& rCmd)
{
    bool ret = false;
    //
    Mutex::Autolock _lock(mCmdQueMtx);
    //
    MY_LOGD_IF(0, "+ que size(%d)", mCmdQue.size());
    //
    //  Wait until the queue is not empty or this thread will exit.
    while (mCmdQue.empty() && !exitPending())
    {
        status_t status = mCmdQueCond.wait(mCmdQueMtx);
        if  (NO_ERROR != status)
        {
            MY_LOGW("wait status(%d), que size(%d), exitPending(%d)", status, mCmdQue.size(), exitPending());
        }
    }
    //
    if  (!mCmdQue.empty())
    {
        //  If the queue is not empty, take the first command from the queue.
        ret = true;
        rCmd = *mCmdQue.begin();
        mCmdQue.erase(mCmdQue.begin());
        MY_LOGD("type %d:0x%08X, que size(%d)",
                rCmd.callbackType,
                rCmd.type,
                mCmdQue.size());
    }
    //
    MY_LOGD_IF(0, "- que size(%d), ret(%d)", mCmdQue.size(), ret);
    return  ret;
}

/******************************************************************************
*
*******************************************************************************/
bool
FrameworkCBThread::
threadLoop()
{
    bool ret;
    callback_data cmd;
    if  ((ret = getCB(cmd)))
    {
        if(cmd.type == 0)
        {
            Thread::requestExit();
            MY_LOGI("Trigger Exit");
            mMyPtr = NULL;
            return false;
        }
        {
            Mutex::Autolock _lock(mCmdQueMtx);
            if( mbStopCB )
            {
                MY_LOGI("drop CB, (type, ext1, ext2)=(%d, %d, %d)", cmd.type, cmd.ext1, cmd.ext2);
                //
                if(cmd.callbackType == CALLBACK_TYPE_DATA)
                {
                    MY_LOGI("drop CB, release pMemory(%p)", cmd.pMemory);
                    cmd.pMemory->release(cmd.pMemory);
                }
                //
                return ret;
            }
        }
        //
        switch(cmd.callbackType)
        {
            case CALLBACK_TYPE_NOTIFY:
            {
                MY_LOGD("NOTIFY:0x%08X",cmd.type);
                mpCamMsgCbInfo->mNotifyCb(
                    cmd.type,
                    cmd.ext1,
                    cmd.ext2,
                    mpCamMsgCbInfo->mCbCookie);
                break;
            }
            case CALLBACK_TYPE_DATA:
            {
                MY_LOGD("DATA:0x%08X",cmd.type);
                mpCamMsgCbInfo->mDataCb(
                        cmd.type,
                        cmd.pMemory,
                        0,
                        cmd.pMetadata,
                        mpCamMsgCbInfo->mCbCookie);
                cmd.pMemory->release(cmd.pMemory);
                break;
            }
            default:
            {
                MY_LOGW("Unknown callbackType %d",cmd.callbackType);
                break;
            }
        }
    }
    return ret;
}

/******************************************************************************
*
*******************************************************************************/
status_t
FrameworkCBThread::
init()
{
    return run("CamAdapter@FrameworkCB");
}

/******************************************************************************
*
*******************************************************************************/
void
FrameworkCBThread::
uninit()
{
    requestExit();
}
/******************************************************************************
*
*******************************************************************************/
IFrameworkCBThread*
IFrameworkCBThread::
createInstance(MINT32 i4OpenId, sp<CamMsgCbInfo> pCamMsgCbInfo)
{
    return  new FrameworkCBThread(i4OpenId, pCamMsgCbInfo);
}
