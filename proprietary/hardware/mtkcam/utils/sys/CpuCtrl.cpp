#define LOG_TAG "MtkCam/CpuCtrl"
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
#include <cutils/log.h>
#include <cutils/properties.h>
//

#include <vendor/mediatek/hardware/power/2.0/IPower.h>
#include <vendor/mediatek/hardware/power/2.0/types.h>
using namespace vendor::mediatek::hardware::power::V2_0;

using namespace android;
//
#include <mtkcam/utils/std/Log.h>
#include <mtkcam/utils/std/ULog.h>
#include <mtkcam/utils/std/Trace.h>
#include <mtkcam/utils/sys/CpuCtrl.h>
//-----------------------------------------------------------------------------
CAM_ULOG_DECLARE_MODULE_ID(MOD_UTILITY);
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
class CpuCtrlImp : public CpuCtrl
{
    public:
        CpuCtrlImp();
        ~CpuCtrlImp();
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
        //
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
CpuCtrlImp::
CpuCtrlImp()
{
    FUNCTION_NAME;
    mUser = 0;
    mEnable = MFALSE;
    mPowerHalHandle = -1;
    mPowerHalService = NULL;
}
//----------------------------------------------------------------------------
CpuCtrlImp::
~CpuCtrlImp()
{
    mPowerHalService = NULL;
    FUNCTION_NAME;
}
//-----------------------------------------------------------------------------
CpuCtrl*
CpuCtrl::
createInstance(void)
{
    FUNCTION_NAME;
    //
    CpuCtrlImp* pInstance = new CpuCtrlImp();
    return pInstance;
}
//----------------------------------------------------------------------------
MVOID
CpuCtrlImp::
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
CpuCtrlImp::
enable(CPU_CTRL_INFO &cpuInfo)
{
    CAM_TRACE_NAME("CpuCtrl::enable");
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
CpuCtrlImp::
enable(CPU_CTRL_RANGE_INFO &cpuRangeInfo)
{
    CAM_TRACE_NAME("CpuCtrl::enable");
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
CpuCtrlImp::
enableBoost(MINT timeout)
{
    CAM_TRACE_NAME("CpuCtrl::enableBoost");
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
CpuCtrlImp::
enablePreferIdle()
{
    CAM_TRACE_NAME("CpuCtrl::enablePreferIdle");
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
    LOG_MSG("mPowerHalHandle=%d Cmd=%d,%d", mPowerHalHandle, MtkPowerCmd::CMD_SET_MTK_PREFER_IDLE, MtkPowerCmd::CMD_SET_DISP_IDLE_TIME);
    mPowerHalService->scnConfig(mPowerHalHandle, MtkPowerCmd::CMD_SET_MTK_PREFER_IDLE, 1, 0, 0, 0);
    mPowerHalService->scnConfig(mPowerHalHandle, MtkPowerCmd::CMD_SET_DISP_IDLE_TIME, DISPLAY_IDLE_MS, 0, 0, 0);
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
CpuCtrlImp::
disable()
{
    CAM_TRACE_NAME("CpuCtrl::disable");
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
CpuCtrlImp::
init(void)
{
    CAM_TRACE_NAME("CpuCtrl::init");
    FUNCTION_IN;
    //

    mPowerHalService = IPower::getService();

    MBOOL ret = MTRUE;
    //
    Mutex::Autolock lock(mLock);
    //
    if(mUser == 0)
    {
        LOG_MSG("First user(%d)",mUser);
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
CpuCtrlImp::
uninit(void)
{
    CAM_TRACE_NAME("CpuCtrl::uninit");
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
        mPowerHalHandle = -1;
    }
    //
    EXIT:
    FUNCTION_OUT;
    return ret;
}
//----------------------------------------------------------------------------
MBOOL
CpuCtrlImp::
cpuPerformanceMode(MINT timeOut)
{
    CAM_TRACE_NAME("CpuCtrl::cpuPerformanceMode");
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
CpuCtrlImp::
resetTimeout(MINT timeOut)
{
    CAM_TRACE_NAME("CpuCtrl::resetTimeout");
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
