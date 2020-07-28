/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2018. All rights reserved.
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
#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "Cam3CPUCtrl"
//
#include <dlfcn.h>
#include <fcntl.h>
#include <semaphore.h>
#include <sys/prctl.h>
#include <sys/resource.h>
#include <sys/mman.h>
//
#include <utils/Errors.h>
#include <utils/threads.h>
#include <utils/List.h>
//
#include <cutils/atomic.h>
#include <cutils/properties.h>
//
#include <mtkcam/utils/std/Log.h>

#include <vendor/mediatek/hardware/power/2.0/IPower.h>
#include <vendor/mediatek/hardware/power/2.0/types.h>
using namespace vendor::mediatek::hardware::power::V2_0;

using namespace android;
//
#include <mtkcam/utils/std/Log.h>
#include <mtkcam/utils/std/ULog.h>
#include <mtkcam/utils/std/Trace.h>
#include "Cam3CPUCtrl.h"

CAM_ULOG_DECLARE_MODULE_ID(MOD_CAMDEV3_IMP);
//-----------------------------------------------------------------------------
#define LOG_MSG(fmt, arg...)        CAM_ULOGMD("[%s] " fmt, __FUNCTION__, ##arg)
#define LOG_WRN(fmt, arg...)        CAM_ULOGMW("[%s] " fmt, __FUNCTION__, ##arg)
#define LOG_ERR(fmt, arg...)        CAM_ULOGME("[%s] " fmt, __FUNCTION__, ##arg)
//
#define LOG_MSG_IF(cond, arg...)    if (cond) { LOG_MSG(arg); }
#define LOG_WRN_IF(cond, arg...)    if (cond) { LOG_WRN(arg); }
#define LOG_ERR_IF(cond, arg...)    if (cond) { LOG_ERR(arg); }
//
#define FUNCTION_NAME               LOG_MSG("")
#define FUNCTION_IN                 LOG_MSG("+")
#define FUNCTION_OUT                LOG_MSG("-")

#define DISPLAY_IDLE_MS (100)
//-----------------------------------------------------------------------------
class Cam3CPUCtrlImp : public Cam3CPUCtrl
                     , public android::hardware::hidl_death_recipient
{
    public:
        Cam3CPUCtrlImp();
        virtual ~Cam3CPUCtrlImp();
        //
        virtual void    destroyInstance(void);
        virtual MBOOL   init(void);
        virtual MBOOL   uninit(void);
        virtual MBOOL   enable(CPU_CTRL_INFO &cpuInfo);
        virtual MBOOL   enable(CPU_CTRL_RANGE_INFO &cpuRangeInfo);
        virtual MBOOL   enableBoost(MINT timeout);
        virtual MBOOL   enablePreferIdle(void);
        virtual MBOOL   disable(void);
        virtual MBOOL   cpuPerformanceMode(MINT timeOut);
        virtual MBOOL   resetTimeout(MINT timeout);
        virtual void    serviceDied(uint64_t cookie, const android::wp<android::hidl::base::V1_0::IBase>& who) override;
        //
    private:
        void            __initPowerHAL();
    private:
        mutable Mutex           mLock;
        volatile MINT32         mUser;
        MBOOL                   mEnable;
        //
        MINT32                  mPowerHalHandle;
        sp<IPower>              mPowerHalService;
        CPU_CTRL_INFO           mCpuCtrlInfo;
        CPU_CTRL_RANGE_INFO     mCpuCtrlRangeInfo;
};
//-----------------------------------------------------------------------------
Cam3CPUCtrlImp::
Cam3CPUCtrlImp()
{
    FUNCTION_NAME;
    mUser = 0;
    mEnable = MFALSE;
    mPowerHalHandle = -1;
    mPowerHalService = NULL;
    ::memset(&mCpuCtrlInfo, 0, sizeof(mCpuCtrlInfo));
    ::memset(&mCpuCtrlRangeInfo, 0, sizeof(mCpuCtrlRangeInfo));
}
//----------------------------------------------------------------------------
Cam3CPUCtrlImp::
~Cam3CPUCtrlImp()
{
    mPowerHalService = NULL;
    FUNCTION_NAME;
}
//-----------------------------------------------------------------------------
Cam3CPUCtrl*
Cam3CPUCtrl::
createInstance(void)
{
    FUNCTION_NAME;
    //
    return new Cam3CPUCtrlImp();
}
//----------------------------------------------------------------------------
MVOID
Cam3CPUCtrlImp::
destroyInstance(void)
{
    FUNCTION_IN;
    //
    delete this;
    //
    FUNCTION_OUT;
}
//----------------------------------------------------------------------------
MBOOL
Cam3CPUCtrlImp::
enable(CPU_CTRL_INFO &cpuInfo)
{
    CAM_TRACE_NAME("Cam3CPUCtrl::enable");
    FUNCTION_IN;
    //
    MBOOL ret = MTRUE;
    //
    {
        Mutex::Autolock templock(mLock);
        //
        if(mUser <= 0)
        {
            LOG_WRN("No user");
            ret = MFALSE;
            FUNCTION_OUT;
            return ret;
        }
    }
    //
    if(mEnable)
    {
        disable();
    }
    //
    Mutex::Autolock lock(mLock);
    __initPowerHAL();
    //
    LOG_MSG("big(%d/%d),little(%d/%d),timeout(%d)",
            cpuInfo.bigCore,
            cpuInfo.bigFreq,
            cpuInfo.littleCore,
            cpuInfo.littleFreq,
            cpuInfo.timeout);
    //
    mCpuCtrlInfo = cpuInfo;
    //
    if(mPowerHalService != NULL)
    {
        mPowerHalHandle = mPowerHalService->scnReg();
        LOG_MSG("mPowerHalHandle:(%d)",mPowerHalHandle);
    }
    else
    {
        LOG_WRN("mPowerHalService == NULL !!!");
        ret = MFALSE;
        goto EXIT;
    }

    // set the core number
    mPowerHalService->scnConfig(mPowerHalHandle, MtkPowerCmd::CMD_SET_CLUSTER_CPU_CORE_MIN, 0, cpuInfo.littleCore, 0, 0);
    mPowerHalService->scnConfig(mPowerHalHandle, MtkPowerCmd::CMD_SET_CLUSTER_CPU_CORE_MIN, 1, cpuInfo.bigCore, 0, 0);
    mPowerHalService->scnConfig(mPowerHalHandle, MtkPowerCmd::CMD_SET_CLUSTER_CPU_CORE_MAX, 0, cpuInfo.littleCore, 0, 0);
    mPowerHalService->scnConfig(mPowerHalHandle, MtkPowerCmd::CMD_SET_CLUSTER_CPU_CORE_MAX, 1, cpuInfo.bigCore, 0, 0);

    // set the frequency
    mPowerHalService->scnConfig(mPowerHalHandle, MtkPowerCmd::CMD_SET_CLUSTER_CPU_FREQ_MIN, 0, cpuInfo.littleFreq, 0, 0);
    mPowerHalService->scnConfig(mPowerHalHandle, MtkPowerCmd::CMD_SET_CLUSTER_CPU_FREQ_MIN, 1, cpuInfo.bigFreq, 0, 0);
    mPowerHalService->scnConfig(mPowerHalHandle, MtkPowerCmd::CMD_SET_CLUSTER_CPU_FREQ_MAX, 0, cpuInfo.littleFreq, 0, 0);
    mPowerHalService->scnConfig(mPowerHalHandle, MtkPowerCmd::CMD_SET_CLUSTER_CPU_FREQ_MAX, 1, cpuInfo.bigFreq, 0, 0);
    if(mCpuCtrlInfo.timeout > 0)
    {
        mPowerHalService->scnEnable(mPowerHalHandle, mCpuCtrlInfo.timeout*1000);
    }
    else
    {
        mPowerHalService->scnEnable(mPowerHalHandle, 0);
    }
    mEnable = MTRUE;
    //
    EXIT:
    FUNCTION_OUT;
    return ret;
}
//----------------------------------------------------------------------------
MBOOL
Cam3CPUCtrlImp::
enable(CPU_CTRL_RANGE_INFO &cpuRangeInfo)
{
    CAM_TRACE_NAME("Cam3CPUCtrl::enable");
    FUNCTION_IN;
    //
    MBOOL ret = MTRUE;
    //
    {
        Mutex::Autolock templock(mLock);
        //
        if(mUser <= 0)
        {
            LOG_WRN("No user");
            ret = MFALSE;
            FUNCTION_OUT;
            return ret;
        }
    }
    //
    if(mEnable)
    {
        disable();
    }
    //
    Mutex::Autolock lock(mLock);
    __initPowerHAL();
    //
    LOG_MSG("MIN CORE:big/little(%d/%d), MAX CORE:big/little(%d/%d)",
            cpuRangeInfo.bigCoreMin,
            cpuRangeInfo.littleCoreMin,
            cpuRangeInfo.bigCoreMax,
            cpuRangeInfo.littleCoreMax);
    LOG_MSG("MIN FREQ:big/little(%d/%d), MAX FREQ:big/little(%d/%d)",
            cpuRangeInfo.bigFreqMin,
            cpuRangeInfo.bigFreqMax,
            cpuRangeInfo.littleFreqMin,
            cpuRangeInfo.littleFreqMax);
    LOG_MSG("timeout(%d)",cpuRangeInfo.timeout);
    //
    mCpuCtrlRangeInfo = cpuRangeInfo;
    //
    if(mPowerHalService != NULL)
    {
        mPowerHalHandle = mPowerHalService->scnReg();
        LOG_MSG("mPowerHalHandle:(%d)",mPowerHalHandle);
    }
    else
    {
        LOG_WRN("IPower::getService() == NULL !!!");
        ret = MFALSE;
        goto EXIT;
    }
    //
    if( mCpuCtrlRangeInfo.bigCoreMin >= 0 &&
        mCpuCtrlRangeInfo.littleCoreMin >= 0)
    {
        LOG_MSG("CMD_SET_CPU_CORE_BIG_LITTLE_MIN");
        mPowerHalService->scnConfig(mPowerHalHandle, MtkPowerCmd::CMD_SET_CLUSTER_CPU_CORE_MIN, 0, mCpuCtrlRangeInfo.littleCoreMin, 0, 0);
        mPowerHalService->scnConfig(mPowerHalHandle, MtkPowerCmd::CMD_SET_CLUSTER_CPU_CORE_MIN, 1, mCpuCtrlRangeInfo.bigCoreMin, 0, 0);
    }
    if( mCpuCtrlRangeInfo.bigCoreMax >= 0 &&
        mCpuCtrlRangeInfo.littleCoreMax >= 0)
    {
        LOG_MSG("CMD_SET_CPU_CORE_BIG_LITTLE_MAX");
        mPowerHalService->scnConfig(mPowerHalHandle, MtkPowerCmd::CMD_SET_CLUSTER_CPU_CORE_MAX, 0, mCpuCtrlRangeInfo.littleCoreMax, 0, 0);
        mPowerHalService->scnConfig(mPowerHalHandle, MtkPowerCmd::CMD_SET_CLUSTER_CPU_CORE_MAX, 1, mCpuCtrlRangeInfo.bigCoreMax, 0, 0);
    }
    if( mCpuCtrlRangeInfo.bigFreqMin >= 0 &&
        mCpuCtrlRangeInfo.littleFreqMin >= 0)
    {
        LOG_MSG("CMD_SET_CPU_FREQ_BIG_LITTLE_MIN");
        mPowerHalService->scnConfig(mPowerHalHandle, MtkPowerCmd::CMD_SET_CLUSTER_CPU_FREQ_MIN, 0, mCpuCtrlRangeInfo.littleFreqMin, 0, 0);
        mPowerHalService->scnConfig(mPowerHalHandle, MtkPowerCmd::CMD_SET_CLUSTER_CPU_FREQ_MIN, 1, mCpuCtrlRangeInfo.bigFreqMin, 0, 0);
    }
    if( mCpuCtrlRangeInfo.bigFreqMax >= 0 &&
        mCpuCtrlRangeInfo.littleFreqMax >= 0)
    {
        LOG_MSG("CMD_SET_CPU_FREQ_BIG_LITTLE_MAX");
        mPowerHalService->scnConfig(mPowerHalHandle, MtkPowerCmd::CMD_SET_CLUSTER_CPU_FREQ_MAX, 0, mCpuCtrlRangeInfo.littleFreqMax, 0, 0);
        mPowerHalService->scnConfig(mPowerHalHandle, MtkPowerCmd::CMD_SET_CLUSTER_CPU_FREQ_MAX, 1, mCpuCtrlRangeInfo.bigFreqMax, 0, 0);
    }

    // enable settings and set timeout(if there's one)
    if(mCpuCtrlRangeInfo.timeout > 0)
    {
        mPowerHalService->scnEnable(mPowerHalHandle, mCpuCtrlRangeInfo.timeout*1000);
    }
    else
    {
        mPowerHalService->scnEnable(mPowerHalHandle, 0);
    }
    mEnable = MTRUE;
    //
    EXIT:
    FUNCTION_OUT;
    return ret;
}
//----------------------------------------------------------------------------
MBOOL
Cam3CPUCtrlImp::
enableBoost(MINT timeout)
{
    CAM_TRACE_NAME("Cam3CPUCtrl::enableBoost");
    FUNCTION_IN;
    //
    MBOOL ret = MTRUE;
    //
    {
        Mutex::Autolock templock(mLock);
        //
        if(mUser <= 0)
        {
            LOG_WRN("No user");
            ret = MFALSE;
            FUNCTION_OUT;
            return ret;
        }
    }
    //
    if(mEnable)
    {
        disable();
    }
    //
    Mutex::Autolock lock(mLock);
    //
    int clusterNum = mPowerHalService->querySysInfo(MtkQueryCmd::CMD_GET_CLUSTER_NUM,0);
    for(int i = 0; i < clusterNum; ++i)
    {
        // query the maximum frenquency of the current cluster
        int maxFreq = mPowerHalService->querySysInfo(MtkQueryCmd::CMD_GET_CLUSTER_CPU_FREQ_MAX, i);

        // set both the min and max frequency of current cluster to the maximum frequency
        mPowerHalService->scnConfig(mPowerHalHandle, MtkPowerCmd::CMD_SET_CLUSTER_CPU_FREQ_MIN, i, maxFreq, 0, 0);
        mPowerHalService->scnConfig(mPowerHalHandle, MtkPowerCmd::CMD_SET_CLUSTER_CPU_FREQ_MAX, i, maxFreq, 0, 0);

        LOG_MSG("modify CPU frequency: %d", maxFreq);
    }
    //
    if(timeout > 0)
    {
        mPowerHalService->scnEnable(mPowerHalHandle, timeout*1000);
    }
    else
    {
        mPowerHalService->scnEnable(mPowerHalHandle, 0);
    }
    //
    mEnable = MTRUE;
    //
    FUNCTION_OUT;
    return ret;
}
//----------------------------------------------------------------------------
MBOOL
Cam3CPUCtrlImp::
enablePreferIdle()
{
    if(::property_get_bool("vendor.cam3dev.cpupreferidle", true) == false) {
        LOG_MSG("CPU prefer idle is disabled");
        return false;
    }

    CAM_TRACE_NAME("Cam3CPUCtrl::enablePreferIdle");
    FUNCTION_IN;
    //
    MBOOL ret = MTRUE;
    //
    {
        Mutex::Autolock templock(mLock);
        //
        if(mUser <= 0)
        {
            LOG_WRN("No user");
            ret = MFALSE;
            FUNCTION_OUT;
            return ret;
        }
    }
    //
    if(mEnable)
    {
        disable();
    }
    //
    Mutex::Autolock lock(mLock);
    __initPowerHAL();
    //
    if(mPowerHalService != NULL)
    {
        mPowerHalHandle = mPowerHalService->scnReg();
        LOG_MSG("mPowerHalHandle:(%d)",mPowerHalHandle);
    }
    else
    {
        LOG_WRN("mPowerHalService == NULL !!!");
        ret = MFALSE;
        goto EXIT;
    }

    //
    LOG_MSG("mPowerHalHandle=%d Cmd=%d", mPowerHalHandle, MtkPowerCmd::CMD_SET_SCHED_BOOST);
    mPowerHalService->scnConfig(mPowerHalHandle, MtkPowerCmd::CMD_SET_SCHED_BOOST, 1, 0, 0, 0);
    mPowerHalService->scnEnable(mPowerHalHandle, 0);

    //
    mEnable = MTRUE;
    //
    EXIT:
    FUNCTION_OUT;
    return ret;
}
//----------------------------------------------------------------------------
MBOOL
Cam3CPUCtrlImp::
disable()
{
    CAM_TRACE_NAME("Cam3CPUCtrl::disable");
    FUNCTION_IN;
    //
    MBOOL ret = MTRUE;
    //
    Mutex::Autolock lock(mLock);
    //
    if(mUser <= 0)
    {
        LOG_WRN("No user");
        ret = MFALSE;
        goto EXIT;
    }
    //
    if(mEnable)
    {
        if(mPowerHalHandle != -1)
            mPowerHalService->scnDisable(mPowerHalHandle);
        mEnable = MFALSE;
    }
    else
    {
        LOG_MSG("Not thing to disable");
    }
    //
    EXIT:
    FUNCTION_OUT;
    return ret;
}
//----------------------------------------------------------------------------
MBOOL
Cam3CPUCtrlImp::
init(void)
{
    CAM_TRACE_NAME("Cam3CPUCtrl::init");
    FUNCTION_IN;
    //

    // mPowerHalService = IPower::getService();
    __initPowerHAL();

    MBOOL ret = MTRUE;
    //
    Mutex::Autolock lock(mLock);
    //
    if(mUser == 0)
    {
        LOG_MSG("First user");
    }
    else
    {
        LOG_MSG("More user(%d)",mUser);
        android_atomic_inc(&mUser);
        goto EXIT;
    }
    //
    mEnable = MFALSE;
    mPowerHalHandle = -1;
    //
    android_atomic_inc(&mUser);
    //
    EXIT:
    FUNCTION_OUT;
    return ret;
}
//----------------------------------------------------------------------------
MBOOL
Cam3CPUCtrlImp::
uninit(void)
{
    CAM_TRACE_NAME("Cam3CPUCtrl::uninit");
    FUNCTION_IN;
    //
    MBOOL ret = MTRUE;
    //
    {
        Mutex::Autolock templock(mLock);
        //
        if(mUser <= 0)
        {
            LOG_WRN("No user(%d)",mUser);
            FUNCTION_OUT;
            return ret;
        }
    }
    //
    if(mEnable)
    {
        disable();
    }
    //
    Mutex::Autolock lock(mLock);
    //
    android_atomic_dec(&mUser);
    //
    if(mUser == 0)
    {
        LOG_MSG("Last user(%d)",mUser);
    }
    else
    {
        LOG_MSG("More user(%d)",mUser);
        goto EXIT;
    }
    //
    if(mPowerHalService != NULL)
    {
        mPowerHalService->scnUnreg(mPowerHalHandle);
        mPowerHalService->scnDisable(mPowerHalHandle);
        mPowerHalService->unlinkToDeath(this);
        mPowerHalHandle = -1;
    }
    //
    EXIT:
    FUNCTION_OUT;
    return ret;
}
//----------------------------------------------------------------------------
MBOOL
Cam3CPUCtrlImp::
cpuPerformanceMode(MINT timeOut)
{
    CAM_TRACE_NAME("Cam3CPUCtrl::cpuPerformanceMode");
    FUNCTION_IN;
    //
    MBOOL ret = MTRUE;
    //
    {
        Mutex::Autolock templock(mLock);
        //
        if(mUser <= 0)
        {
            LOG_WRN("No user");
            ret = MFALSE;
            FUNCTION_OUT;
            return ret;
        }
    }
    //
    if(mEnable)
    {
        disable();
    }
    //
    Mutex::Autolock lock(mLock);
    __initPowerHAL();
    //
    if(mPowerHalService != NULL)
    {
        mPowerHalHandle = mPowerHalService->scnReg();
        LOG_MSG("mPowerHalHandle:(%d)",mPowerHalHandle);
    }
    else
    {
        LOG_WRN("mPowerHalService == NULL !!!");
        ret = MFALSE;
        goto EXIT;
    }
    // enter cpu performance mode
    mPowerHalService->scnConfig(mPowerHalHandle, MtkPowerCmd::CMD_SET_CPU_PERF_MODE, 1, 0, 0, 0);

    if(timeOut > 0)
    {
        mPowerHalService->scnEnable(mPowerHalHandle, timeOut * 1000);
    }
    else
    {
        mPowerHalService->scnEnable(mPowerHalHandle, (int)MtkHintOp::MTK_HINT_ALWAYS_ENABLE);
    }
    mEnable = MTRUE;
    //
    EXIT:
    FUNCTION_OUT;
    return ret;
}
//----------------------------------------------------------------------------
MBOOL
Cam3CPUCtrlImp::
resetTimeout(MINT timeOut)
{
    CAM_TRACE_NAME("Cam3CPUCtrl::resetTimeout");
    FUNCTION_IN;
    //
    MBOOL ret = MTRUE;
    //
    {
        Mutex::Autolock templock(mLock);
        //
        if(mUser <= 0)
        {
            LOG_WRN("No user");
            ret = MFALSE;
            FUNCTION_OUT;
            return ret;
        }
    }
    //
    Mutex::Autolock lock(mLock);
    //
    if(mPowerHalHandle == -1)
    {
        LOG_WRN("mPowerHalHandle is invalid !!!");
        ret = MFALSE;
        goto EXIT;
    }

    // reset timeout
    if(timeOut > 0)
    {
        mPowerHalService->scnEnable(mPowerHalHandle, timeOut * 1000);
    }
    else
    {
        mPowerHalService->scnEnable(mPowerHalHandle, (int)MtkHintOp::MTK_HINT_ALWAYS_ENABLE);
    }
    //
    EXIT:
    FUNCTION_OUT;
    return ret;
}
//----------------------------------------------------------------------------
void
Cam3CPUCtrlImp::
serviceDied(uint64_t cookie __unused, const wp<hidl::base::V1_0::IBase>& who __unused)
{
    LOG_WRN("IPower hidl service died");
    Mutex::Autolock lock(mLock);
    mPowerHalService = nullptr;
    mPowerHalHandle  = -1;
}
//----------------------------------------------------------------------------
void
Cam3CPUCtrlImp::
__initPowerHAL()
{
    CAM_TRACE_NAME("Cam3CPUCtrl::__initPowerHAL");
    FUNCTION_IN;

    if (mPowerHalService == nullptr)
    {
        LOG_MSG("Loaded power HAL service +");
        mPowerHalService = IPower::getService();
        LOG_MSG("Loaded power HAL service -");
        if (mPowerHalService != nullptr) {
            //Work around since linkToDeath will call decStrong
            this->incStrong(this);
            android::hardware::Return<bool> linked = mPowerHalService->linkToDeath(this, 0);

            // this->decStrong(this);
            if (!linked.isOk()) {
                LOG_ERR("Transaction error in linking to PowerHAL death: %s",
                linked.description().c_str());
            } else if (!linked) {
                LOG_WRN("Unable to link to PowerHAL death notifications");
            } else {
               // LOG_MSG("Link to death notification successful");
            }
        } else {
            LOG_WRN("Couldn't load power HAL service");
        }
    }

    FUNCTION_OUT;
}