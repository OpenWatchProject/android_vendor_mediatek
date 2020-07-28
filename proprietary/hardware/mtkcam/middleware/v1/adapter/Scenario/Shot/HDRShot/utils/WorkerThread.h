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

#ifndef _MTK_CAMERA_CAMADAPTER_SCENARIO_SHOT_HDRSHOT_UTILS_WORKERTHREAD_H_
#define _MTK_CAMERA_CAMADAPTER_SCENARIO_SHOT_HDRSHOT_UTILS_WORKERTHREAD_H_

#include <utils/Thread.h>

#include <mtkcam/feature/hdr/Platform.h>

#include <memory>

// ---------------------------------------------------------------------------

namespace android {
namespace NSShot {

// ---------------------------------------------------------------------------

class ICapture : public virtual RefBase
{
public:
    virtual ~ICapture() = default;

    virtual bool normalCapture() = 0;
    virtual bool zsdCapture() = 0;

    virtual std::shared_ptr<NSCam::IPerf> getPerf() const = 0;

    virtual void onCaptureBegin() = 0;
    virtual void onCaptureEnd() = 0;
};

// ---------------------------------------------------------------------------

class LifeSpanObserver final
{
public:
    LifeSpanObserver(const wp<ICapture>& object);
    virtual ~LifeSpanObserver();

private:
    wp<ICapture> mObject;
};

// ---------------------------------------------------------------------------

class WorkerThread final : public Thread
{
public:

    WorkerThread(const char* name, const uint32_t shotMode, const sp<ICapture>& shot);

    status_t queryResult() const { Mutex::Autolock _l(mLock); return mStatus; }

private:
    mutable Mutex mLock;

    char mThreadName[256];

    sp<ICapture> mShot;

    const uint32_t mShotMode;

    // used to report the execution status
    status_t mStatus;

    // good place to do one-time initializations
    status_t readyToRun();

    bool threadLoop();
};

} // namespace NSShot
} // namespace android

#endif // _MTK_CAMERA_CAMADAPTER_SCENARIO_SHOT_HDRSHOT_UTILS_WORKERTHREAD_H_
