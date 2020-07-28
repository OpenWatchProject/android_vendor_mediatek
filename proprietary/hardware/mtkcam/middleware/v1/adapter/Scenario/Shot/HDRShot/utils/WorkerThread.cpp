/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2015. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

#define DEBUG_LOG_TAG "WorkerThread"

#include "WorkerThread.h"

#include <mtkcam/def/Modes.h>

#include <sys/resource.h>

#include <mtkcam/feature/hdr/utils/Debug.h>

using namespace android;
using namespace NSCam;
using namespace NSShot;

// ---------------------------------------------------------------------------

LifeSpanObserver::LifeSpanObserver(const wp<ICapture>& object)
    :mObject(object)
{
    sp <ICapture> capture = mObject.promote();
    if (capture.get())
        capture->onCaptureBegin();
}

LifeSpanObserver::~LifeSpanObserver()
{
    sp <ICapture> capture = mObject.promote();
    if (capture.get())
        capture->onCaptureEnd();
}

// ---------------------------------------------------------------------------

WorkerThread::WorkerThread(
        const char* name, const uint32_t shotMode, const sp<ICapture>& shot)
    : mShot(shot)
    , mShotMode(shotMode)
    , mStatus(NO_ERROR)
{
    snprintf(mThreadName, sizeof(mThreadName), "%s", name);
}

status_t WorkerThread::readyToRun()
{
    //  set thread policy & priority
    //  NOTE:
    //  Even if pthread_create() with SCHED_OTHER policy, a newly-created thread
    //  may inherit the non-SCHED_OTHER policy & priority of the thread creator.
    //  And thus, we must set the expected policy & priority after a thread creation.

    // retrieve the parameters of the calling process
    struct sched_param schedParam;
    sched_getparam(0, &schedParam);

    if (setpriority(PRIO_PROCESS, 0, ANDROID_PRIORITY_NORMAL))
    {
        HDR_LOGW("set priority failed(%s)", strerror(errno));
    }

    schedParam.sched_priority = 0;
    sched_setscheduler(0, SCHED_OTHER, &schedParam);

    // enable CPU affinity
    if (mShot.get() && mShot->getPerf().get())
        mShot->getPerf()->enableAffinity(gettid());

    return OK;
}

bool WorkerThread::threadLoop()
{
    HDR_TRACE_CALL();

    Mutex::Autolock _l(mLock);

    if (!mShot.get())
    {
        HDR_LOGE("shot is not set");
        mStatus = NO_INIT;
        return false;
    }

    LifeSpanObserver observer(mShot.get());

    if (eShotMode_ZsdHdrShot == mShotMode)
    {
        mStatus = mShot->zsdCapture() ? NO_ERROR : UNKNOWN_ERROR;
    }
    else
    {
        mStatus = mShot->normalCapture() ? NO_ERROR : UNKNOWN_ERROR;
    }

    // disable CPU affinity
    if (mShot->getPerf().get())
        mShot->getPerf()->disableAffinity(gettid());

    // one-shot execution
    return false;
}
