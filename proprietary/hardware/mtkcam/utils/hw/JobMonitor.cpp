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

#define LOG_TAG "MtkCam/JobMonitor"

#include <sys/prctl.h>
#include <sys/resource.h>

#include <mtkcam/utils/std/Log.h>
#include <mtkcam/utils/std/ULog.h>
#include <mtkcam/utils/hw/IScenarioControl.h>
#include <mtkcam/utils/hw/JobMonitor.h>
#include <libladder.h>

CAM_ULOG_DECLARE_MODULE_ID(MOD_UTILITY);

//log defines
#define MY_LOGV(fmt, arg...)        CAM_ULOGMV("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_ULOGMD("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_ULOGMI("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_ULOGMW("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_ULOGME("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("[%s] " fmt, __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( (cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( (cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)

#define FUNC_START                  MY_LOGD("+")
#define FUNC_END                    MY_LOGD("-")

//thread defines
#define JOB_MONITOR_THREAD_NAME       ("Cam@jobMonitor")
#define JOB_MONITOR_THREAD_POLICY     (SCHED_OTHER)
#define JOB_MONITOR_THREAD_PRIORITY   (ANDROID_PRIORITY_BACKGROUND)

#define TIME_STEP_MS 10

namespace NSCam {
namespace Utils {

static android::Mutex jobMonitorLock;
static sp<JobMonitor> jobMonitor = NULL; //singleton


sp<JobMonitor>
JobMonitor::
getInstance()
{
    Mutex::Autolock lock(jobMonitorLock);
    if( jobMonitor == NULL )
    {
        jobMonitor = new JobMonitor();
    }
    return jobMonitor;
}


JobMonitor::
JobMonitor()
    : mIdCounter(0)
{
    FUNC_START;
    {
        Mutex::Autolock lock(mJobLock);
        mvJob.clear();
    }
    Thread::run(LOG_TAG);
    FUNC_END;
}


JobMonitor::
~JobMonitor()
{
}


void
JobMonitor::
onLastStrongRef(const void* /*id*/)
{
    FUNC_START;
    {
        Mutex::Autolock lock(mJobLock);
        Thread::requestExit();
        mJobCond.signal();
    }
    Thread::join();
    {
        Mutex::Autolock lock(mJobLock);
        MY_LOGV("job size:%zu", mvJob.size());
        mvJob.clear();
    }
    FUNC_END;
}


MUINT64
JobMonitor::
registerJob(struct Job job)
{
    Mutex::Autolock lock(mJobLock);
    MUINT64 usingId = mIdCounter;
    mIdCounter++;
    //
    struct Job newJob = job;
    newJob.timePassedMs = 0;
    mvJob.add(usingId, newJob);
    MY_LOGV("size:%zu, id:%" PRId64 ", job=( %s, %d, %p, 0x%x, %d)"
            , mvJob.size(), usingId, newJob.tag.string(), newJob.timeoutMs, newJob.cb.promote().get(), newJob.flag, newJob.timePassedMs);
    mJobCond.signal();
    return usingId;
}


MVOID
JobMonitor::
unregisterJob(MUINT64 jobId, MBOOL printResult)
{
    Mutex::Autolock lock(mJobLock);
    if( mvJob.indexOfKey(jobId) >= 0 )
    {
        struct Job job = mvJob.editValueFor(jobId);
        if( printResult )
        {
            MY_LOGI("size:%zu, id:%" PRId64 ", job=( %s, %d, %p, 0x%x, %d)"
                    , mvJob.size(), jobId, job.tag.string(), job.timeoutMs, job.cb.promote().get(), job.flag, job.timePassedMs);
        }
        else
        {
            MY_LOGV("size:%zu, id:%" PRId64 ", job=( %s, %d, %p, 0x%x, %d)"
                    , mvJob.size(), jobId, job.tag.string(), job.timeoutMs, job.cb.promote().get(), job.flag, job.timePassedMs);
        }
        mvJob.removeItem(jobId);
    }
    else
    {
        MY_LOGW("trying to remove unexisting job %" PRId64, jobId);
    }
}


status_t
JobMonitor::
readyToRun()
{
    // set name
    ::prctl(PR_SET_NAME, JOB_MONITOR_THREAD_NAME, 0, 0, 0);

    // set normal
    struct sched_param sched_p;
    sched_p.sched_priority = 0;
    ::sched_setscheduler(0, JOB_MONITOR_THREAD_POLICY, &sched_p);
    ::setpriority(PRIO_PROCESS, 0, JOB_MONITOR_THREAD_PRIORITY);
    //
    ::sched_getparam(0, &sched_p);

    MY_LOGD("policy(%d) priority(%d)", ::sched_getscheduler(0), sched_p.sched_priority);
    return OK;
}


bool
JobMonitor::
threadLoop()
{
    MUINT32 jobSize = 0;
    MINT32 i = 0;
    while( MTRUE )
    {
        Mutex::Autolock lock(mJobLock);
        if( exitPending() )
        {
            break;
        }
        jobSize = mvJob.size();
        if( jobSize > 0 )
        {
            mJobLock.unlock();
            usleep(TIME_STEP_MS*1000);
            mJobLock.lock();
            for( i = 0 ; i < mvJob.size() ; i++ )
            {
                mvJob.editValueAt(i).timePassedMs += TIME_STEP_MS;
            }
            for( i = 0 ; i < mvJob.size() ; i++ )
            {
                if( mvJob.valueAt(i).timePassedMs >= mvJob.valueAt(i).timeoutMs )
                {
                    struct Job job = mvJob.valueAt(i);
                    mJobLock.unlock();
                    timeoutHandling(job);
                    unregisterJob(mvJob.keyAt(i));
                    mJobLock.lock();
                    break;
                }
            }
        }
        else
        {
            MY_LOGV("wait signal +");
            mJobCond.wait(mJobLock);
            MY_LOGV("wait signal -");
        }
    }
    {
        Mutex::Autolock lock(mJobLock);
        jobSize = mvJob.size();
    }
    MY_LOGI("exit monitor thread, remain %u job", jobSize);
    return false;
}


MVOID
JobMonitor::
timeoutHandling(struct Job& job)
{
    FUNC_START;
    MY_LOGW("job=( %s, %d, %p, 0x%x, %d)"
            , job.tag.string(), job.timeoutMs, job.cb.promote().get(), job.flag, job.timePassedMs);
    if( job.flag & FLAG_DUMP_STACK )
    {
        dumpStack();
    }
    sp<JobMonitor::Callback> cb = job.cb.promote();
    if( (job.flag & FLAG_TIMEOUT_CB) && cb != NULL )
    {
        cb->onTimeout();
    }
    if( job.flag & FLAG_KILL_PROCESS )
    {
        doBeforeKillCb();
        killProcess();
    }
    FUNC_END;
}


MVOID
JobMonitor::
dumpStack()
{
    std::string callstack;
    UnwindCurProcessBT(&callstack);
    __android_log_write(ANDROID_LOG_INFO, "MTKCAM_TIMEOUT", callstack.c_str());
}


MVOID
JobMonitor::
doBeforeKillCb()
{
    //TODO : call back to registered job
}


MVOID
JobMonitor::
killProcess()
{
    //run exit scenario
    for( int openId = 0 ; openId < 3 ; openId++ )
    {
        sp<IScenarioControl> pScenarioCtrl = IScenarioControl::create(openId);
        if (pScenarioCtrl != NULL){
            pScenarioCtrl->exitScenario();
            pScenarioCtrl = NULL;
        }
    }
    // run NE flow
    MY_LOGE("suicide: kill camerahalserver - raise(SIGINT)");
    ::raise(SIGINT);
    MY_LOGE("suicide: kill camerahalserver - raise(SIGTERM)");
    ::raise(SIGTERM);
    MY_LOGE("suicide: kill camerahalserver - raise(SIGKILL)");
    ::raise(SIGKILL);
}


/******************************************************************************
 *
 ******************************************************************************/
JobMonitor::Helper::
Helper(String8 tag, MINT32 timeoutMs, wp<JobMonitor::Callback> cb, MUINT32 flag)
{
    sp<JobMonitor> spMonitor = JobMonitor::getInstance();
    mJobId = spMonitor->registerJob(JobMonitor::Job(tag, timeoutMs, cb, flag));
    mwpMonitor = spMonitor;
}


JobMonitor::Helper::
~Helper()
{
}


MVOID
JobMonitor::Helper::
onLastStrongRef(const void* /*id*/)
{
    sp<JobMonitor> spMonitor = mwpMonitor.promote();
    if( spMonitor != NULL )
    {
        spMonitor->unregisterJob(mJobId);
    }
}

}; // namespace NSCam::Utils
}; // namespace NSCam