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
#ifndef _MTK_CAMERA_UTILS_STD_JOBMONITOR_H_
#define _MTK_CAMERA_UTILS_STD_JOBMONITOR_H_

#include <utils/RefBase.h>
#include <utils/String8.h>
#include <utils/Mutex.h>
#include <utils/Condition.h>
#include <utils/Thread.h>
#include <utils/KeyedVector.h>

#include <mtkcam/def/common.h>

using namespace android;

namespace NSCam {
namespace Utils {

class JobMonitor : public virtual RefBase
                 , public Thread
{
    enum
    {
        //bit mask flag
        FLAG_DUMP_STACK = 0x1,
        FLAG_KILL_PROCESS = 0x2,
        FLAG_TIMEOUT_CB = 0x4,
        FLAG_BEFORE_KILL_CB = 0x8,
        FLAG_ALL = 0xF
    };

public:
    class Callback : public virtual RefBase
    {
    public:
        virtual MVOID   onTimeout(){}
        virtual MVOID   onBeforeKillProcess(){}
    };

public:
    class Helper : public virtual RefBase
    {
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    //  RefBase Interface.
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    public:
        virtual MVOID   onLastStrongRef( const void* /*id*/);

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    //  Implementations.
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    public:
                        Helper(String8 tag, MINT32 timeoutMs, wp<JobMonitor::Callback> cb = NULL, MUINT32 flag = JobMonitor::FLAG_ALL);
                        ~Helper();
    private:
        wp<JobMonitor>  mwpMonitor;
        MUINT64         mJobId;
    };

public:
    struct Job
    {
        String8 tag;
        MINT32 timeoutMs;
        wp<JobMonitor::Callback> cb;
        MUINT32 flag;
        MINT32 timePassedMs;

        Job()
            : tag(String8(""))
            , timeoutMs(0)
            , cb(NULL)
            , flag(0)
            , timePassedMs(0)
        {
        }

        Job(String8 _tag, MINT32 _timeoutMs, wp<JobMonitor::Callback> _cb, MUINT32 _flag = JobMonitor::FLAG_ALL)
            : tag(_tag)
            , timeoutMs(_timeoutMs)
            , cb(_cb)
            , flag(_flag)
            , timePassedMs(0)
        {
        }
    };
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  RefBase Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    virtual MVOID       onLastStrongRef( const void* /*id*/);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Thread Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
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
//  Implementations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    static sp<JobMonitor>   getInstance();

                            JobMonitor();
                            ~JobMonitor();

    MUINT64                 registerJob(struct Job job);
    MVOID                   unregisterJob(MUINT64 jobId, MBOOL printResult = MFALSE);

private:
    MVOID                   timeoutHandling(struct Job& job);
    MVOID                   dumpStack();
    MVOID                   doBeforeKillCb();
    MVOID                   killProcess();

private:
    Mutex                   mJobLock;
    Condition               mJobCond;
    KeyedVector<MUINT64, JobMonitor::Job> mvJob;
    MUINT64                 mIdCounter;
};

}; // namespace NSCam::Utils
}; // namespace NSCam
#endif // _MTK_CAMERA_UTILS_STD_JOBMONITOR_H_
