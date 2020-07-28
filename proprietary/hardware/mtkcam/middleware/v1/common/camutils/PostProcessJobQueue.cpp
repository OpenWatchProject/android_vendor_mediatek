/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2018. All rights reserved.
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
#define LOG_TAG "PostProcQ"

#include <mtkcam/middleware/v1/camutils/PostProcessJobQueue.h>

#include <mtkcam/utils/std/Log.h>
#include <mtkcam/utils/std/JobQueue.h>

#include <mutex>
#include <condition_variable>

#include <time.h>

namespace NSCam {
namespace PostProcessJobQueue {


static NSCam::JobQueue<void()> g_jobQueue(LOG_TAG);
static std::mutex              g_opLock;
static std::condition_variable g_cond;


size_t
addJob(
    std::function<void()>&& job
)
{
    auto _signalCond = [](std::mutex* pMx, std::condition_variable* pCv)
    {
        std::lock_guard<std::mutex> lk(*pMx);
        pCv->notify_all();
    };

    size_t s = 0;

    std::lock_guard<std::mutex> l( g_opLock );

    s = g_jobQueue.getPendingSize() / 2; // update size,

    g_jobQueue.addJob(std::move( job ));
    g_jobQueue.addJob(std::bind(std::move( _signalCond ), &g_opLock, &g_cond));

    return ++s; // including this job;
}


std::cv_status
waitUntilEmpty(
    int timeoutMs /* = -1 */
)
{
    if (timeoutMs < 0) {
        timeoutMs = INT_MAX;
    }

    std::unique_lock<std::mutex> lk( g_opLock );

    while (1) {
        size_t s = g_jobQueue.getPendingSize() / 2; // we always add a condition fire job, hence we divide 2
        if ( s > 0) {
            auto r = g_cond.wait_for(lk, std::chrono::milliseconds(timeoutMs));
            if (r == std::cv_status::timeout) {
                return r;
            }
        }
        else {
            break;
        }
    }
    return std::cv_status::no_timeout;
}


std::cv_status
waitUntilSize(
    size_t theSize,
    int    timeoutMs /* = -1 */
)
{
    if (timeoutMs < 0) {
        timeoutMs = INT_MAX;
    }

    std::unique_lock<std::mutex> lk( g_opLock );

    while (1) {
        size_t s = g_jobQueue.getPendingSize() / 2; // we always add a condition fire job, hence we divide 2
        if ( s > theSize) {
            auto r = g_cond.wait_for(lk, std::chrono::milliseconds(timeoutMs));
            if (r == std::cv_status::timeout) {
                return r;
            }
        }
        else {
            break;
        }
    }
    return std::cv_status::no_timeout;
}


size_t
size()
{
    std::lock_guard<std::mutex> l( g_opLock );
    return g_jobQueue.getPendingSize() / 2; // we always add a condition fire job
}


}; // namespace PostProcessJobQueue
}; // namespace NSCam
