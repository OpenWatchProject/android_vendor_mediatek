/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2017. All rights reserved.
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
#include "MfllFeaturePerf.h"

#define LOG_TAG "MtkCam/Mfll/Perfserv"

#include "MfllLog.h"

// MFNR
#include <MfllProperty.h>

// MTK
#include <mtkcam/utils/std/Misc.h>

// Performance Service
#include <perfservicenative/PerfServiceNative.h>
#include <cutils/properties.h>

#if 0 // no need
// Affinity
#include <linux/mt_sched.h>
#endif

// AOSP
#include <cutils/compiler.h>

// STL
#include <memory>
#include <mutex>

/* Define this to 1 for enable performance service */
#ifndef MFLL_ENABLE_PERFSERV
#define MFLL_ENABLE_PERFSERV    0
#endif

/* define __DEBUG for enable debug message */
//#define __DEBUG

#define __FUNCTION__IN__    mfllLogD("%s [+]", __FUNCTION__)
#define __FUNCTION__OUT__   mfllLogD("%s [-]", __FUNCTION__)


using namespace mfll;


static std::vector<enum EventType> EVENTS_TO_LISTEN_INITIALIZER(void)
{
    std::vector<enum EventType> v;
    #define LISTEN(x) v.push_back(x)
    LISTEN(mfll::EventType_Bss);
    LISTEN(mfll::EventType_MotionCompensation);
    #undef LISTEN
    return v;
}
static vector<enum EventType> g_eventsToListen = EVENTS_TO_LISTEN_INITIALIZER();


// ----------------------------------------------------------------------------
// PerfImpl for MFNR
// ----------------------------------------------------------------------------
namespace __mfll_feature_perf__
{
class MfllPerfImpl : public MfllFeaturePerf
{
public:
    // constructor
    MfllPerfImpl()
        : MfllFeaturePerf()
        , mSceneHandle(-1)
    {
        mProperyDisable =
            (CC_UNLIKELY( MfllProperty::readProperty(Property_PerfServ) == 0 ))
            ? true : false;
    }

    // destructor
    ~MfllPerfImpl()
    {
        disablePerfServ();
    }

//
// method
//
public:
    // By PerfServ, enable all CPU and set maximum frequency for every cluster
    int enablePerfServ()
    {
        __FUNCTION__IN__;
        if (CC_UNLIKELY( mProperyDisable )) {
            mfllLogD("%s: disable PerfServ by property", __FUNCTION__);
        }
        else {
            std::lock_guard<std::mutex> _locker(mPerfLock);

            // has been init already.
            if (mSceneHandle >= 0) {
                __FUNCTION__OUT__;
                return 0;
            }

            mSceneHandle = PerfServiceNative_userRegScn();
            if (mSceneHandle == -1) {
                mfllLogE("register PerfService scenario failed");
                __FUNCTION__OUT__;
                return -1;
            }
            // disable EAS+ schedule
            PerfServiceNative_userRegScnConfig(mSceneHandle,CMD_SET_SCHED_MODE, 0, 0, 0, 0);

            // get cluster number
            int numOfCluster = PerfServiceNative_getClusterInfo(CMD_GET_CLUSTER_NUM, 0);

#ifdef __DEBUG
            mfllLogD("%s: cluster num = %d", __FUNCTION__, numOfCluster);
#endif

            // for each cluster, get CPU num and freq, and adjust to maximum
            for (int i = 0; i < numOfCluster; i++) {
                // get CPU number
                int cpuNum = PerfServiceNative_getClusterInfo(CMD_GET_CLUSTER_CPU_NUM, i);
                // get the maximum frequency of the cluster
                int freq   = PerfServiceNative_getClusterInfo(CMD_GET_CLUSTER_CPU_FREQ_MAX, i);

#ifdef __DEBUG
                mfllLogD("%s: cluster(%d) has %d CPUs", __FUNCTION__, i, cpuNum);
                mfllLogD("%s: cluster(%d) freq = %d", __FUNCTION__, i, freq);
#endif
                // set CPU enable count and frequency to this cluster
                PerfServiceNative_userRegScnConfig(
                        mSceneHandle, CMD_SET_CLUSTER_CPU_CORE_MIN, i, cpuNum, 0, 0);
                PerfServiceNative_userRegScnConfig(
                        mSceneHandle, CMD_SET_CLUSTER_CPU_CORE_MAX, i, cpuNum, 0, 0);
                PerfServiceNative_userRegScnConfig(
                        mSceneHandle, CMD_SET_CLUSTER_CPU_FREQ_MIN, i, freq,   0, 0);
                PerfServiceNative_userRegScnConfig(
                        mSceneHandle, CMD_SET_CLUSTER_CPU_FREQ_MAX, i, freq,   0, 0);
            } // for-loop

            //PerfServiceNative_userEnable(mSceneHandle);

        }
        __FUNCTION__OUT__;
        return 0;
    }


    int disablePerfServ()
    {
        __FUNCTION__IN__;
        if (CC_UNLIKELY( mProperyDisable )) {
            mfllLogD("%s: disable PerfServ by property", __FUNCTION__);
        }
        else {
            // lock mutex w/o exception, if failed, ignore it.
            std::unique_lock<std::mutex> _locker(mPerfLock, std::defer_lock);
            NSCam::Utils::lockNoexcept<std::mutex>(_locker);

            if (mSceneHandle != -1) {
                PerfServiceNative_userDisable(mSceneHandle);
                PerfServiceNative_userUnreg(mSceneHandle);
                mSceneHandle = -1;
            }
        }
        __FUNCTION__OUT__;
        return 0;
    }


    int enableAffinity(pid_t pid, int cpu_select_bit, int cpu_num)
    {
        __FUNCTION__IN__;
        int status = 0;
#if 0 // no need
        size_t cpu_set_size = 0;
        cpu_set_t* cpuset = nullptr;


        if (CC_UNLIKELY( cpu_num <= 0 )) {
            mfllLogE("cpu_num <= 0, ignore CPU affinity");
            status = -1;
            goto lbExit;
        }

        // allocate cpu_set_t
        cpuset = CPU_ALLOC(cpu_num);
        if (CC_UNLIKELY( cpuset == nullptr )) {
            mfllLogE("CPU_ALLOC failed");
            status = -1;
            goto lbExit;
        }

        // get chunk size and fill up with 0
        cpu_set_size = CPU_ALLOC_SIZE(cpu_num);
        CPU_ZERO_S(cpu_set_size, cpuset);

        // for all CPU, check if enable, if yes, set it
        for (int i = 0; i < cpu_num; i++) {
            if (cpu_select_bit & (1 << i)) {
                CPU_SET(i, cpuset);
            }
        }

        {
            std::lock_guard<std::mutex> _locker(mAffineLock);
            status = mt_sched_setaffinity(pid, sizeof(cpu_set_t), cpuset);
        }

        if (CC_UNLIKELY( status != 0 )) {
            mfllLogW("mt_sched_setaffinity() failed: status(%d)", status);
        }

lbExit:
        if (CC_LIKELY( cpuset != nullptr )) {
            CPU_FREE(cpuset); // clear memory
        }
#endif
        __FUNCTION__OUT__;
        return status;
    }


    int disableAffinity(pid_t pid)
    {
        __FUNCTION__IN__;

        int status = 0;
#if 0 // no need
        {
            std::lock_guard<std::mutex> _locker(mAffineLock);
            status = mt_sched_exitaffinity(pid);
        }

        if (CC_UNLIKELY( status != 0 )) {
            mfllLogD("mt_sched_exitaffinity() failed: status(%d)", status);
        }

        __FUNCTION__OUT__;
#endif
        return status;
    }


//
// Re-implementation
//
public:
    void onEvent(
            enum EventType      t,
            MfllEventStatus_t&  /* status */,
            void*               /* mfllCore */,
            void*               /* param1 */,
            void*               /* param2 */
            )
    {
        // enable PerfService since BSS starting
        if (t == EventType_Bss)
            enablePerfServ();
    }


    void doneEvent(
            enum EventType      t,
            MfllEventStatus_t&  /* status */,
            void*               mfllCore,
            void*               param1,
            void*               /* param2 */
            )
    {
        // disable PerfService until the last MC done
        if (t == EventType_MotionCompensation) {
            if (CC_LIKELY( mfllCore != nullptr )) {
                IMfllCore* c = static_cast<IMfllCore*>(mfllCore);
                /* getCaptureFrameNum is frame number, param1 is index, hence we need minus 1 */
                if ( (c->getCaptureFrameNum()-1) == (unsigned int)(long long)(param1)) {
                    disablePerfServ();
                }
            }
        }
    }


    vector<enum EventType> getListenedEventTypes(void)
    {
        return g_eventsToListen;
    }


//
// Attributes
//
private:
    int mSceneHandle;
    bool mProperyDisable;
    std::mutex mPerfLock;
    std::mutex mAffineLock;


}; // class MfllPerfImpl
}; // namespace __mfll_feature_perf__


// ----------------------------------------------------------------------------
// Factory
// ----------------------------------------------------------------------------
MfllFeaturePerf* MfllFeaturePerf::createInstance()
{
#if MFLL_ENABLE_PERFSERV
    return new __mfll_feature_perf__::MfllPerfImpl;
#else
    return new MfllFeaturePerf;
#endif
}

// ----------------------------------------------------------------------------
// MfllFeaturePerf default
// ----------------------------------------------------------------------------
MfllFeaturePerf::MfllFeaturePerf(void)
{
}


MfllFeaturePerf::~MfllFeaturePerf(void)
{
}


void MfllFeaturePerf::onEvent(enum EventType, MfllEventStatus_t&, void*, void*, void*)
{
}


void MfllFeaturePerf::doneEvent(enum EventType, MfllEventStatus_t&, void*, void*, void*)
{
}


vector<enum EventType> MfllFeaturePerf::getListenedEventTypes(void)
{
    return g_eventsToListen;
}
