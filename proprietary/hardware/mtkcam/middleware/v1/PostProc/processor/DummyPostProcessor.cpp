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
 * MediaTek Inc. (C) 2017. All rights reserved.
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

#define LOG_TAG "DummyPostProcessor"

// Standard C header file
#include <sys/prctl.h>
#include <sys/resource.h>
// Android system/core header file
#include <utils/List.h>
#include <utils/Mutex.h>
#include <utils/Condition.h>
#include <utils/Thread.h>
// mtkcam custom header file

// mtkcam global header file
#include <mtkcam/def/BuiltinTypes.h>
#include <mtkcam/def/Errors.h>
#include <mtkcam/utils/std/Log.h>
#include <mtkcam/utils/std/Misc.h>
// Module header file

// Local header file
#include "DummyPostProcessor.h"

/******************************************************************************
 *
 ******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_LOGV(LOG_TAG "(%d)[%s] " fmt "\n", ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD(LOG_TAG "(%d)[%s] " fmt "\n", ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI(LOG_TAG "(%d)[%s] " fmt "\n", ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW(LOG_TAG "(%d)[%s] " fmt "\n", ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE(LOG_TAG "(%d)[%s] " fmt "\n", ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA(LOG_TAG "(%d)[%s] " fmt "\n", ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF(LOG_TAG "(%d)[%s] " fmt "\n", ::gettid(), __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( (cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( (cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)

#define FUNCTION_LOG_START          MY_LOGD_IF(1<=1, " +");
#define FUNCTION_LOG_END            MY_LOGD_IF(1<=1, " -");

using namespace android;
using namespace android::NSPostProc;

#define PROCESSOR_POLICY     (SCHED_OTHER)
#define PROCESSOR_PRIORITY   (0)
/******************************************************************************
 *
 ******************************************************************************/
DummyPostProcessor::
DummyPostProcessor(
    PostProcessorType type
)
: ImagePostProcessorBase(type)
{
    FUNCTION_LOG_START
    if(run(LOG_TAG) != OK)
    {
        MY_LOGE("create post process thread fail.");
        return;
    }
    FUNCTION_LOG_END
}
/******************************************************************************
 *
 ******************************************************************************/
DummyPostProcessor::
~DummyPostProcessor()
{
    FUNCTION_LOG_START
    mQueue.clear();
    FUNCTION_LOG_END
}
/******************************************************************************
 *
 ******************************************************************************/
status_t
DummyPostProcessor::
doPostProc(
    sp<PostProcRequestSetting> setting
)
{
    FUNCTION_LOG_START
    Mutex::Autolock _l(mQueueLock);
    mQueue.push_back(setting);
    mQueueCond.signal();
    FUNCTION_LOG_END
    return OK;
}
/******************************************************************************
 *
 ******************************************************************************/
status_t
DummyPostProcessor::
onResultReceived(
    MUINT32    const /*requestNo*/,
    StreamId_T const /*streamId*/,
    MBOOL      const /*errorBuffer*/,
    android::sp<IImageBuffer>& /*pBuffer*/
)
{
    FUNCTION_LOG_START
    FUNCTION_LOG_END
    return OK;
}
/******************************************************************************
 *
 ******************************************************************************/
void
DummyPostProcessor::
requestExit()
{
    FUNCTION_LOG_START;
    Thread::requestExit();
    mbExit = MTRUE;
    FUNCTION_LOG_END
}
/******************************************************************************
 *
 ******************************************************************************/
status_t
DummyPostProcessor::
readyToRun()
{
    //
    //  thread policy & priority
    //  Notes:
    //      Even if pthread_create() with SCHED_OTHER policy, a newly-created thread
    //      may inherit the non-SCHED_OTHER policy & priority of the thread creator.
    //      And thus, we must set the expected policy & priority after a thread creation.
    struct sched_param sched_p;
    ::sched_getparam(0, &sched_p);
    if (PROCESSOR_POLICY == SCHED_OTHER) {
        sched_p.sched_priority = 0;
        ::sched_setscheduler(0, PROCESSOR_POLICY, &sched_p);
        ::setpriority(PRIO_PROCESS, 0, PROCESSOR_PRIORITY);   //  Note: "priority" is nice value.
    } else {
        sched_p.sched_priority = PROCESSOR_PRIORITY;          //  Note: "priority" is real-time priority.
        ::sched_setscheduler(0, PROCESSOR_POLICY, &sched_p);
    }

    MY_LOGD("tid(%d) policy(%d) priority(%d)", ::gettid(), PROCESSOR_POLICY, PROCESSOR_PRIORITY);

    return OK;
}
/******************************************************************************
 *
 ******************************************************************************/
bool
DummyPostProcessor::
threadLoop()
{
    MY_LOGD("+");
    {
        Mutex::Autolock _l(mQueueLock);
        while (mQueue.empty() && !exitPending())
        {
            status_t status = mQueueCond.wait(mQueueLock);
            if  ( OK != status ) {
                MY_LOGW(
                    "wait status:%d:%s, mQueue.size:%zu",
                    status, ::strerror(-status), mQueue.size()
                );
            }
        }
    }

    if  ( exitPending() ) {
        MY_LOGD("exit unpack thread");
        return false;
    }

    sp<PostProcRequestSetting> setting = nullptr;
    {
        Mutex::Autolock _l(mQueueLock);
        if (!mQueue.empty())
        {
            setting = *mQueue.begin();
            mQueue.erase(mQueue.begin());
            MY_LOGD("Queue size %zu", mQueue.size());
        }
    }

    if (setting != nullptr)
    {
//        sleep(1);
        sp<IPostProcImageCallback> pCB = setting->mpImageCallback;
        pCB->onCB_CompressedImage_packed(PostProcessorType::DUMMY, setting->miTimeStamp, 0, NULL, 0, true);
    }

    MY_LOGD("-");
    return true;
}
