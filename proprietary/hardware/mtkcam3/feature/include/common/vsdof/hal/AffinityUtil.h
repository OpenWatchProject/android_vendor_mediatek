/********************************************************************************************
 *     LEGAL DISCLAIMER
 *
 *     (Header of MediaTek Software/Firmware Release or Documentation)
 *
 *     BY OPENING OR USING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *     THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE") RECEIVED
 *     FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON AN "AS-IS" BASIS
 *     ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED,
 *     INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR
 *     A PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY
 *     WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK
 *     ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
 *     NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION
 *     OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 *
 *     BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE LIABILITY WITH
 *     RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION,
 *     TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/
#ifndef _AFFINITY_UTIL_H_
#define _AFFINITY_UTIL_H_

#include <bitset>
#include <sstream>
#include <mtkcam/utils/std/Log.h>
#include <cutils/properties.h>

#include <sched.h>
#include <sys/syscall.h>
#include <sys/sysinfo.h>

// To correctly use Log.h, please do followed setup in your modules:
// (1) Include mtkcam.mk in your Android.mk
// (2) Add MTKCAM_CFLAGS to CFLAGS in your Android.mk
// If you do not meet the requirements, Log LEVEL will be strict.

//CAM_LOG_LEVEL is defined at:
//  vendor/mediatek/proprietary/hardware/mtkcam/common/include/Log.h
//  64: #define CAM_LOG_LEVEL       2
#define AFFINITY_LOGV(fmt, arg...)    do{ if(__LOG_ENABLED) __android_log_print(ANDROID_LOG_VERBOS, __logTAG.c_str(), fmt, ##arg); } while(0)
#define AFFINITY_LOGD(fmt, arg...)    do{ if(__LOG_ENABLED) __android_log_print(ANDROID_LOG_DEBUG, __logTAG.c_str(), fmt, ##arg); } while(0)
#define AFFINITY_LOGI(fmt, arg...)    do{ if(__LOG_ENABLED) __android_log_print(ANDROID_LOG_INFO, __logTAG.c_str(), fmt, ##arg); } while(0)
#define AFFINITY_LOGW(fmt, arg...)    do{ if(CAM_LOG_LEVEL >= 1) __android_log_print(ANDROID_LOG_WARN, __logTAG.c_str(), fmt, ##arg); } while(0)
#define AFFINITY_LOGE(fmt, arg...)    __android_log_print(ANDROID_LOG_ERROR, __logTAG.c_str(), "(%s){%s:%d}" fmt, __FUNCTION__, __FILE__, __LINE__, ##arg)

enum ENUM_CPU_CORE
{
    CPUCoreLL = 0,      //LL: 0~3
    CPUCoreL  = 0x10,   //L : 4~7
    CPUCoreB  = 0x100   //B : 8~9
};

typedef unsigned int MASK_T;

#define ALL_CORE 0x3FF  //10 cores

class CPUMask
{
public:
    CPUMask()
    {
        setAllCPUCore();
    }

    CPUMask(MASK_T mask)
    {
        __mask = mask & ALL_CORE;  //10 cores
    }

    CPUMask(ENUM_CPU_CORE core, MASK_T count=4)
    {
        reset();
        setCore(core, count);
    }

    ~CPUMask() {}

    void reset()
    {
        __mask = 0;
    }

    void setAllCPUCore()
    {
        __mask = ALL_CORE;
    }

    void setCore(ENUM_CPU_CORE core, MASK_T count=4)
    {
        int shift = 0;
        switch(core) {
        case CPUCoreLL:
            if(count > 4) count = 4;
            __mask &= ~0xF;     //Clear bit 0~3
            break;
        case CPUCoreL:
            shift = 4;
            if(count > 4) count = 4;
            __mask &= ~0xF0;    //Clear bit 4~7
            break;
        case CPUCoreB:
            shift = 8;
            if(CPU_CORE_COUNT > 8) {
                if(count > 2) count = 2;
                __mask &= ~0x300;   //Clear bit 8, 9
            }
        default:
            break;
        }

        for(MASK_T n = 0; n < count; n++) {
            __mask |= (1 << (n+shift));
        }
    }

    MASK_T getMask()
    {
        return __mask;
    }

    void setMask(MASK_T mask)
    {
        __mask = mask;
    }

private:
    MASK_T __mask;
    const int CPU_CORE_COUNT = get_nprocs();
};

class CPUAffinity
{
public:
    CPUAffinity(const char *LOGTAG=NULL)
    {
        __logTAG = (LOGTAG) ? std::string(LOGTAG) : "CPUAffinityUtil";
    }

    virtual ~CPUAffinity()
    {
        disable();
    }

    virtual void enable(CPUMask &cpuMask, bool threadOnly=true)
    {
        __threadOnly = threadOnly;
        MASK_T cpu_msk = cpuMask.getMask();
        if(cpu_msk==0) {
            cpu_msk = 1;
        }

        if(__threadOnly) {
            __setAffinityToThread(cpu_msk);
        } else {
            cpu_set_t cpuset;
            CPU_ZERO(&cpuset);

            for(MASK_T Msk=1, cpu_no=0; Msk <= cpu_msk; Msk<<=1, cpu_no++)
            {
                if(Msk&cpu_msk)
                {
                    CPU_SET(cpu_no, &cpuset);
                    // AFFINITY_LOGD("Set CPU %d(%s)", cpu_no, __cpusetToString(cpuset).c_str());
                }
            }

            __setAffinityToProcess(cpuset);
        }
    }

    virtual void disable()
    {
        if(!__isEnabled) {
            return;
        }

        // clear affinity
        int s = 0;
        if(__threadOnly) {
            MASK_T currentMask = 0;
            s = syscall(__NR_sched_getaffinity, gettid(), sizeof(int), &currentMask);
            if(s < 0) {
                AFFINITY_LOGE("Get current affinity fail, pid %d, err: %d(%s)", getpid(), s, strerror(errno));
            } else if(currentMask != __newCPUMask) {
                AFFINITY_LOGD("Affnity changed, ignore to reset to old affinity, old mask: %s, current %s",
                              __maskToString(__newCPUMask).c_str(), __maskToString(currentMask).c_str());
            } else {
                s = syscall(__NR_sched_setaffinity, gettid(), sizeof(int), &__oldCPUMask);
                if(s < 0) {
                    AFFINITY_LOGE("Reset affinity to %s failed, err: %d(%s)", __maskToString(__oldCPUMask).c_str(), s, strerror(errno));
                }
            }
        } else {
            cpu_set_t cpuset;
            s = sched_getaffinity(getpid(), sizeof(cpu_set_t), &cpuset);
            if(s < 0) {
                AFFINITY_LOGE("Get current affinity fail, pid %d, err: %d(%s)", getpid(), s, strerror(errno));
            } else if(0 == CPU_EQUAL(&cpuset,  &__newCPUSet)) {
                AFFINITY_LOGD("Affnity changed, ignore to reset to old affinity, old set: %s, current %s",
                              __cpusetToString(__newCPUSet).c_str(), __cpusetToString(cpuset).c_str());
            } else {
                s = sched_setaffinity(getpid(), sizeof(cpu_set_t), &__oldCPUSet);
                if (s < 0) {
                    AFFINITY_LOGE("Reset affinity to %s failed, err: %d(%s)", __cpusetToString(__oldCPUSet).c_str(), s, strerror(errno));
                }
            }
        }

        __isEnabled = false;
    }
private:
    bool __setAffinityToThread(MASK_T &mask)
    {
        AFFINITY_LOGD("Set CPU Affinity %s to thread %d(in pid %d)\n", __maskToString(mask).c_str(), gettid(), getpid());

        __isEnabled = false;
        pid_t tid = gettid();
        int s = syscall(__NR_sched_getaffinity, tid, sizeof(int), &__oldCPUMask);
        if(s < 0) {
            AFFINITY_LOGE("Get current affinity fail, tid %d, err: %d(%s)", tid, s, strerror(errno));
            return false;
        }

        s = syscall(__NR_sched_setaffinity, tid, sizeof(int), &mask);
        if (s < 0) {
            AFFINITY_LOGE("Set affinity fail, tid %d, mask %s, err: %d(%s)",
                          tid, __maskToString(mask).c_str(), s, strerror(errno));
            return false;
        }
        else
        {
            //Double check
            MASK_T currentMask = 0;
            s = syscall(__NR_sched_getaffinity, tid, sizeof(int), &currentMask);
            if(s >= 0) {
                if(currentMask == mask) {
                    __isEnabled = true;
                    __newCPUMask = mask;
                    AFFINITY_LOGD("Change affinity of tid %d from %s to %s", tid, __maskToString(__oldCPUMask).c_str(), __maskToString(mask).c_str());
                } else {
                    AFFINITY_LOGE("Change affinity failed, tid %d, current %s, target %s", tid, __maskToString(currentMask).c_str(), __maskToString(mask).c_str());
                }
            } else {
                AFFINITY_LOGE("Get new affinity fail, err: %d(%s)", s, strerror(errno));
            }
        }

        return __isEnabled;
    }

    bool __setAffinityToProcess(cpu_set_t &cpuset)
    {
        AFFINITY_LOGD("Set CPU Affinity %s to process %d(from tid %d)\n", __cpusetToString(cpuset).c_str(), getpid(), gettid());

        __isEnabled = false;
        pid_t pid = getpid();
        int s = sched_getaffinity(pid, sizeof(cpu_set_t), &__oldCPUSet);
        if(s < 0) {
            AFFINITY_LOGE("Get current affinity fail, err: %d(%s)", s, strerror(errno));
            return false;
        }

        s = sched_setaffinity(pid, sizeof(cpu_set_t), &cpuset);
        if (s < 0) {
            AFFINITY_LOGE("Set affinity fail, err: %d(%s)", s, strerror(errno));
            return false;
        }
        else
        {
            //Double check
            cpu_set_t currentCPUSet;
            s = sched_getaffinity(pid, sizeof(cpu_set_t), &currentCPUSet);
            if(s >= 0) {
                if(0 == CPU_EQUAL(&cpuset, &currentCPUSet)) {
                    __isEnabled = true;
                    __newCPUSet = cpuset;
                    AFFINITY_LOGD("Change affinity of PID %d from %s to %s",
                                  pid, __cpusetToString(cpuset).c_str(), __cpusetToString(__oldCPUSet).c_str());
                } else {
                    AFFINITY_LOGD("Change affinity of PID %d failed, current %s, target %s",
                                  pid, __cpusetToString(currentCPUSet).c_str(), __cpusetToString(cpuset).c_str());
                }
            } else {
                AFFINITY_LOGE("Get new affinity fail, err: %d(%s)", s, strerror(errno));
            }
        }

        return __isEnabled;
    }

    //For debugging
    const std::string __cpusetToString(cpu_set_t &cpuset)
    {
        std::ostringstream oss("");
        for(int i = 10-1; i >= 0; i--) {
            oss << CPU_ISSET(i, &cpuset);
        }

        return oss.str();
    }

    const std::string __maskToString(MASK_T mask)
    {
        if(CPU_CORE_COUNT <= 8) {
            return std::bitset<8>(mask).to_string();
        }

        return std::bitset<10>(mask).to_string();
    }

private:
    cpu_set_t   __oldCPUSet;    //For process
    cpu_set_t   __newCPUSet;    //For process
    MASK_T      __oldCPUMask;   //For thread
    MASK_T      __newCPUMask;   //For thread

    std::string __logTAG;
    const bool __LOG_ENABLED = ::property_get_int32("vendor.affinity.log", 0);
    bool __isEnabled = false;
    bool __threadOnly = false;
    const int CPU_CORE_COUNT = get_nprocs();
};

class AutoCPUAffinity : public CPUAffinity
{
public:
    AutoCPUAffinity(CPUMask &cpuMask, const char *LOGTAG=NULL)
    {
        enable(cpuMask, LOGTAG);
    }

    ~AutoCPUAffinity()
    {
        disable();
    }
};

#endif