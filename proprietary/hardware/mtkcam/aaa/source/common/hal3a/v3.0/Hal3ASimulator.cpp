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
 * MediaTek Inc. (C) 2010. All rights reserved.
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

#define LOG_TAG "Hal3ASimulator"
//
#include <sys/prctl.h>
#include <sys/resource.h>
//
#include <cutils/properties.h>
//
#include <vector>
#include <string>
#include <mutex>
#include <pthread.h>
//
#include <mtkcam/utils/std/Log.h>
#include <mtkcam/utils/std/Trace.h>
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>
//
#include <mtkcam/drv/iopipe/CamIO/INormalPipe.h>
#include <mtkcam/aaa/IHal3A.h>
//
using namespace std;
using namespace NSCam;
using namespace NS3Av3;

/******************************************************************************
 *
 ******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_LOGV("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("[%s] " fmt, __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)


/******************************************************************************
 *
 ******************************************************************************/
#ifdef  USING_MTK_LDVT
#define LDVT_TIMING_FACTOR ((MINT64)4) //(1 * 30) for 1sec
#else
#define LDVT_TIMING_FACTOR ((MINT64)1)
#endif

#if (0) // #if (1) // for force enable log
#define HAL3A_SIMULATOR_FORCE_ENABLE_LOG (1)
#else
#define HAL3A_SIMULATOR_FORCE_ENABLE_LOG (0)
#endif

#if (1) // #if (0) // for force disable REAL_SOF
#define HAL3A_SIMULATOR_REAL_SOF (1)
#else
#define HAL3A_SIMULATOR_REAL_SOF (0)
#endif

#if (1) // #if (0) // for disable pass control metadata to result metadata
#define HAL3A_SIMULATOR_PASS_METADATA (1)
#else
#define HAL3A_SIMULATOR_PASS_METADATA (0)
#endif

#ifdef DEF_FRAME_INTERVAL_US
#undef DEF_FRAME_INTERVAL_US
#endif
#define DEF_FRAME_INTERVAL_US ((MINT64)33333)

#ifdef DEF_SHUTTER_DELAY_NUM
#undef DEF_SHUTTER_DELAY_NUM
#endif
#define DEF_SHUTTER_DELAY_NUM (2)

#ifdef INVALID_SOF_INDEX
#undef INVALID_SOF_INDEX
#endif
#define INVALID_SOF_INDEX (0x100)//(256)

/******************************************************************************
 *
 ******************************************************************************/
static auto getNormalPipeModule()
{
    static auto pModule = NSCam::NSIoPipe::NSCamIOPipe::INormalPipeModule::get();
    return pModule;
}

static MUINT32 getNormalPipeModuleVersion(MINT32 sensorIndex)
{
    auto pModule = getNormalPipeModule();
    if  ( ! pModule ) {
        return 0;
    }

    MUINT32 const* version = NULL;
    size_t count = 0;
    int err = pModule->get_sub_module_api_version(&version, &count, sensorIndex);
    if  ( err < 0 || ! count || ! version ) {
        return 0;
    }

    return *(version + count - 1); //Select max. version
}


/******************************************************************************
 *  Metadata Access
 ******************************************************************************/
template <typename T>
static inline MBOOL
tryGetMetadata(
    IMetadata const* pMetadata,
    MUINT32 const tag,
    T & rVal
)
{
    if (pMetadata == NULL) {
        MY_LOGW("pMetadata == NULL");
        return MFALSE;
    }
    //
    IMetadata::IEntry entry = pMetadata->entryFor(tag);
    if(!entry.isEmpty()) {
        rVal = entry.itemAt(0, Type2Type<T>());
        return MTRUE;
    }
    //
    return MFALSE;
}


/******************************************************************************
 *
 ******************************************************************************/
namespace {
class Hal3ASimulator : public IHal3A
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  CallbackThread.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:
    class CallbackThread
    {
    private:
        Hal3ASimulator* mpSim3A;
        pthread_t       mThread;
        bool            mbEnableThread;

    public:
        CallbackThread(Hal3ASimulator* pHal3ASimulatorImp)
            : mpSim3A(pHal3ASimulatorImp)
            , mThread()
            , mbEnableThread(false)
        {}

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    //  Thread Interface.
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    public:
        void run()
        {
            pthread_attr_t attr;
            struct sched_param pthread_param = {
                .sched_priority = -20
            };

            MY_LOGD("readyToRun callback thread");
            pthread_attr_init(&attr);
            pthread_attr_setstack(&attr, NULL, 1024*1024);
            pthread_attr_setguardsize(&attr, 4096);
            pthread_attr_setschedpolicy(&attr, SCHED_OTHER);
            pthread_attr_setschedparam(&attr, &pthread_param);
            MY_LOGD("pthread attr has flags = %d", attr.flags);
            MINT32 result = ::pthread_create(&mThread, &attr, threadLoop, this);
            if(result != 0)
                 MY_LOGE("[%s] result(%d)", __FUNCTION__, result);
            else
                mbEnableThread = true;
            return;
        };

        void requestExit()
        {
            mbEnableThread = false;
        };

        void join()
        {
            pthread_join(mThread, NULL);
        };

    private:
        void changeThreadSetting()
        {
            // set name
            ::prctl(PR_SET_NAME, (unsigned long)"Hal3ASimulator", 0, 0, 0);
            // set normal
            struct sched_param sched_p;
            sched_p.sched_priority = 0;
            ::sched_setscheduler(0, (SCHED_OTHER), &sched_p);
            ::setpriority(PRIO_PROCESS, 0, -20);
                                            //  Note: "priority" is nice value.
            //
            ::sched_getparam(0, &sched_p);
            MY_LOGD(
                "Tid: %d, policy: %d, priority: %d"
                , ::gettid(), ::sched_getscheduler(0)
                , sched_p.sched_priority
            );
            return;
        };

        static void* threadLoop(void* arg)
        {
            CallbackThread *_this = reinterpret_cast<CallbackThread*>(arg);

            if (_this->mpSim3A == NULL) {
                MY_LOGD("caller not exist - exit callback thread");
                return NULL;
            }
            _this->changeThreadSetting();

            CAM_TRACE_NAME("Hal3ASim::CT::threadLoop");

            MY_LOGD_IF(_this->mpSim3A->mLogLevel >= 2, "enter callback thread");

            while (_this->mbEnableThread) {
                MY_LOGD_IF(_this->mpSim3A->mLogLevel >= 2, "go-on callback thread");
                MINT64 current_time = 0;
                struct timeval tv;
                //
                MINT32 idx = 0;
                //
                gettimeofday(&tv, NULL);
                current_time = tv.tv_sec * (MINT64)1000000 + tv.tv_usec;
                {
                    std::unique_lock<std::mutex> cblock(_this->mpSim3A->mCallbackLock);
                    if (!_this->mpSim3A->mCallbackEnable) {
                        MY_LOGD("CB wait+");
                        _this->mpSim3A->mCallbackCond.wait(cblock);
                        MY_LOGD("CB wait-");
                        _this->mpSim3A->mLastCallbackTime = current_time;
                        continue;
                    }
                }
                //
                if (_this->mpSim3A->mLastCallbackTime == 0) {
                    MY_LOGD_IF(_this->mpSim3A->mLogLevel >= 2,
                        "CB Thread Time not set %" PRId64 " / %" PRId64 ,
                        _this->mpSim3A->mLastCallbackTime, current_time);
                    _this->mpSim3A->mLastCallbackTime = current_time;
                    continue;
                }
                //
                _this->mpSim3A->waitSuspend();
                //
                #if HAL3A_SIMULATOR_REAL_SOF
                if (_this->mpSim3A->mpCamIO != NULL) {
                    //MY_LOGI("SOF(%d)", _this->mpSim3A->mLastSofIdx);
                    CAM_TRACE_FMT_BEGIN("SOF(%d)", _this->mpSim3A->mLastSofIdx);
                    MBOOL res = MFALSE;
                    MY_LOGD_IF(_this->mpSim3A->mLogLevel >= 2,
                        "CB wait SOF +++ (%" PRId64 ")", current_time);
                    res = _this->mpSim3A->mpCamIO->wait(
                        NSCam::NSIoPipe::NSCamIOPipe::EPipeSignal_SOF,
                        NSCam::NSIoPipe::NSCamIOPipe::EPipeSignal_ClearWait,
                        _this->mpSim3A->mUserKey,
                        _this->mpSim3A->mTimeoutMs);
                    if (_this->mpSim3A->mLogLevel >= 2) {
                        gettimeofday(&tv, NULL);
                        current_time = tv.tv_sec * (MINT64)1000000 + tv.tv_usec;
                    }
                    CAM_TRACE_FMT_END();
                    if (!res) {
                        MY_LOGE("CB wait SOF TIMEOUT:%dms (%" PRId64 ")",
                            _this->mpSim3A->mTimeoutMs, current_time);
                        break;
                    }
                    MY_LOGD_IF(_this->mpSim3A->mLogLevel >= 2,
                        "CB wait SOF --- (%" PRId64 ")", current_time);
                    //
                    if (_this->mpSim3A->mCallbackSkip > 0) {
                        _this->mpSim3A->mCallbackSkip --;
                    } else {
                        _this->mpSim3A->performCallback(_this->mpSim3A->mCallbackCount);
                        _this->mpSim3A->mCallbackCount ++;
                    }
                    //
                }
                #else
                if ((current_time - _this->mpSim3A->mLastCallbackTime) >
                    _this->mpSim3A->mCallbackTimeIntervalUs) {

                    _this->mpSim3A->mLastCallbackTime = current_time;
                    MY_LOGD_IF(_this->mpSim3A->mLogLevel >= 2,
                        "Current CB Thread Time = %lld",
                        _this->mpSim3A->mLastCallbackTime);
                    //
                    if (_this->mpSim3A->mCallbackSkip > 0) {
                        _this->mpSim3A->mCallbackSkip --;
                    } else {
                        _this->mpSim3A->performCallback(_this->mpSim3A->mCallbackCount);
                        _this->mpSim3A->mCallbackCount ++;
                    }
                    //
                } else {
                    MY_LOGD_IF(_this->mpSim3A->mLogLevel >= 2,
                        "CB Thread Time = %lld / %lld",
                        _this->mpSim3A->mLastCallbackTime, current_time);
                    // next time for check, the time interval can be adjusted
                    usleep(_this->mpSim3A->mCallbackTimeIntervalUs >> 4);
                }
                #endif
            }
            MY_LOGD("exit callback thread");
            _this->mpSim3A = NULL;
            return NULL;
        };
    };

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Data Member.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:

    MINT32              mLogLevel;
    MINT32              mSensorIdx;
    IHal3ACb*           mCbSet[IHal3ACb::eID_MSGTYPE_NUM];
    MINT32              mCapacity;
    //
    MINT32              mSubsampleCount;
    //
    mutable std::mutex  mLock;
    MINT32              mLastSofIdx;
    MINT32              mShutterDelayNum;
    //
    mutable std::mutex  mNumQueueLock;
    vector<MINT32> mvNumQueue;
    //
    mutable std::mutex  mMetaQueueLock;
    std::map<MINT32,MetaSet_T> mvMetaQueue;
    //
    mutable std::mutex  mCallbackLock;
    std::condition_variable  mCallbackCond;
    MBOOL               mCallbackEnable;
    MINT32              mCallbackSkip;
    MINT32              mCallbackCount;
    //
    MINT64              mLastCallbackTime;
    MINT64              mCallbackTimeIntervalUs;
    //
    CallbackThread*     mpCallbackThread;
    //
    NSCam::NSIoPipe::NSCamIOPipe::INormalPipe*  mpCamIO;
    char const*         mpCamIOUserName;
    MINT32              mUserKey;
    MINT32              mTimeoutMs;
    //
    mutable std::mutex  mSuspendLock;
    std::condition_variable  mSuspendCond;
    MBOOL               mSuspending;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
                        Hal3ASimulator(MINT32 const i4SensorIdx);

protected:
    MINT32              doConfig(const ConfigInfo_T& rConfigInfo);
    MINT32              doStart(MINT32 i4StartNum = 0);
    MINT32              doStop();
    void                doPause();
    void                doResume(MINT32 MagicNum = 0);
    void                setSuspend(MBOOL suspend);
    MBOOL               getSuspend(void);
    void                waitSuspend(void);
    void                performCallback(MINT32 count);
    virtual MINT32      setIspDefault(MINT32 flowType,
                            const MetaSet_T& control,
                            TuningParam* pTuningBuf,
                            MetaSet_T* pResult);
    MINT32              setRequest(const vector<MetaSet_T*>& controls);
    MINT32              setMetaList(const vector<MetaSet_T*>& controls);
    void                getMetaSet(MINT32 num, MetaSet_T & meta);
    void                enQueueMeta(MINT32 num, MetaSet_T const & meta);
    MetaSet_T           deQueueMeta(MINT32 num);
    void                enQueueNum(MINT32 num);
    MINT32              deQueueNum(void);
    MINT32              getQueueNum(MUINT32 index = 0);
    void                dumpQueueNum(void);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:

    virtual MVOID       destroyInstance(const char* /*strUser*/) {}

    virtual MINT32      config(const ConfigInfo_T& rConfigInfo);
    virtual MINT32      config(MINT32 i4SubsampleCount);
    virtual MINT32      start(MINT32 i4StartNum = 0);
    virtual MINT32      stop();

    virtual MVOID       stopStt()
    {
        MY_LOGD_IF(mLogLevel >= 1, "");
        return;
    }

    virtual MVOID      pause()
    {
        MY_LOGD_IF(mLogLevel >= 1, "3A simulator pause");
        CAM_TRACE_NAME("Hal3ASim::pause");
        std::lock_guard<std::mutex> lock(mLock);
        return doPause();
    }

    virtual MVOID      resume(MINT32)
    {
        MY_LOGD_IF(mLogLevel >= 1, "3A simulator resume");
        CAM_TRACE_NAME("Hal3ASim::resume");
        std::lock_guard<std::mutex> lock(mLock);
        return doResume();
    }

    /* Add for Previous Compatibility */

    virtual MINT32      startCapture(const std::list<MetaSet_T>& controls, MINT32 i4StartNum=0, MINT32 i4RequestQSize = -1);

    virtual MINT32      startRequestQ(const std::list<MetaSet_T>& requestQ, MINT32 i4RequestQSize = -1);

    virtual MINT32      set(const std::list<MetaSet_T>& controls, MINT32 i4RequestQSize = -1);


    virtual MINT32      set(const vector<MetaSet_T*>& requestQ)
    {
        MY_LOGD_IF(mLogLevel >= 1, "");
        std::lock_guard<std::mutex> lock(mLock);
        return setMetaList(requestQ);
    }

    virtual MINT32      preset(const vector<MetaSet_T*>& requestQ)
    {
        return 0;
    }

    virtual MINT32      startRequestQ(const vector<MetaSet_T*>& requestQ);

    virtual MINT32      startCapture(
                            const vector<MetaSet_T*>& controls,
                            MINT32 i4StartNum = 0);

    virtual MINT32      setIsp(MINT32 flowType, const MetaSet_T& control,
                            TuningParam* pTuningBuf, MetaSet_T* pResult);

    virtual MINT32      get(MUINT32 frmId, MetaSet_T& result)
    {
        std::lock_guard<std::mutex> lock(mLock);
        MY_LOGD_IF(mLogLevel >= 1, "frmId(%d)", frmId);
        getMetaSet(frmId, result);
        return 0;
    }

    virtual MINT32      getCur(MUINT32 frmId, MetaSet_T& result)
    {
        std::lock_guard<std::mutex> lock(mLock);
        MY_LOGD_IF(mLogLevel >= 1, "frmId(%d)", frmId);
        getMetaSet(frmId, result);
        return 0;
    }

    virtual MINT32      attachCb(IHal3ACb::ECb_T eId, IHal3ACb* pCb);
    virtual MINT32      detachCb(IHal3ACb::ECb_T eId, IHal3ACb* pCb);

    virtual MINT32      getDelay(IMetadata& /*delay_info*/) const
    {
        std::lock_guard<std::mutex> lock(mLock);
        MY_LOGD_IF(mLogLevel >= 1, "");
        return 0;
    }

    virtual MINT32      getDelay(MUINT32 tag) const
    {
        std::lock_guard<std::mutex> lock(mLock);
        MY_LOGD_IF(mLogLevel >= 1, "tag(%d)", tag);
        return 0;
    }

    virtual MINT32      getCapacity() const
    {
        std::lock_guard<std::mutex> lock(mLock);
        MY_LOGD_IF(mLogLevel >= 1, "(%d)", mCapacity);
        return mCapacity;
    }

    virtual MINT32      send3ACtrl(
                            E3ACtrl_T e3ACtrl,
                            MINTPTR i4Arg1,
                            MINTPTR i4Arg2)
    {
        MY_LOGD_IF(mLogLevel >= 1, "(0x%x) 0x%zx 0x%zx",
            e3ACtrl, i4Arg1, i4Arg2);
        return 0;
    }

    virtual MVOID       setSensorMode(MINT32 i4SensorMode)
    {
        std::lock_guard<std::mutex> lock(mLock);
        MY_LOGD_IF(mLogLevel >= 1, "(0x%x)", i4SensorMode);
        return;
    }
#if (CAM3_3A_IP_BASE)
    virtual MVOID       notifyP1Done(MINT32 i4MagicNum, MVOID* pvArg)
    {
        MY_LOGD_IF(mLogLevel >= 1, "(%d) %p", i4MagicNum, pvArg);
        return;
    }
#else
    virtual MVOID       notifyP1Done(MUINT32 u4MagicNum, MVOID* pvArg)
    {
        MY_LOGD_IF(mLogLevel >= 1, "(%d) %p", u4MagicNum, pvArg);
        return;
    }
#endif

    virtual MBOOL       notifyPwrOn()
    {
        MY_LOGD_IF(mLogLevel >= 1, "");
        return MFALSE;
    }

    virtual MBOOL       notifyPwrOff()
    {
        MY_LOGD_IF(mLogLevel >= 1, "");
        return MFALSE;
    }

    virtual MBOOL       notifyP1PwrOn()
    {
        MY_LOGD_IF(mLogLevel >= 1, "");
        return MFALSE;
    }

    virtual MBOOL       notifyP1PwrOff()
    {
        MY_LOGD_IF(mLogLevel >= 1, "");
        return MFALSE;
    }

    virtual MBOOL       checkCapFlash()
    {
        MY_LOGD_IF(mLogLevel >= 1, "");
        return MFALSE;
    }

    virtual MVOID       setFDEnable(MBOOL fgEnable)
    {
        MY_LOGD_IF(mLogLevel >= 1, "(%d)", fgEnable);
        return;
    }

    virtual MBOOL       setFDInfo(MVOID* prFaces)
    {
        MY_LOGD_IF(mLogLevel >= 1, "(%p)", prFaces);
        return MFALSE;
    }

    virtual MBOOL       setFDInfoOnActiveArray(MVOID* prFaces)
    {
        MY_LOGD_IF(mLogLevel >= 1, "(%p)", prFaces);
        return MFALSE;
    }

    virtual MBOOL       setOTInfo(MVOID* prOT)
    {
        MY_LOGD_IF(mLogLevel >= 1, "(%p)", prOT);
        return MFALSE;
    }
    virtual MINT32      dumpIsp(MINT32 flowType, const MetaSet_T& control, TuningParam* pTuningBuf, MetaSet_T* pResult);

}; // class
}; // namespace


/******************************************************************************
 *
 ******************************************************************************/
Hal3ASimulator::
Hal3ASimulator(MINT32 const i4SensorIdx)
    : IHal3A()
    , mLogLevel(0)
    , mSensorIdx(i4SensorIdx)
    , mCbSet{0}
    , mCapacity(3)
    , mSubsampleCount(1)
    //
    , mLock()
    , mLastSofIdx(INVALID_SOF_INDEX)
    , mShutterDelayNum(DEF_SHUTTER_DELAY_NUM)
    //
    , mNumQueueLock()
    , mCallbackLock()
    , mCallbackCond()
    , mCallbackEnable(MFALSE)
    , mCallbackSkip(0)
    , mCallbackCount(0)
    , mLastCallbackTime(0)
    , mCallbackTimeIntervalUs(DEF_FRAME_INTERVAL_US * LDVT_TIMING_FACTOR)
    //
    , mpCamIO(NULL)
    , mpCamIOUserName(NULL)
    , mUserKey(0)
    , mTimeoutMs(0)
    //
    , mSuspendLock()
    , mSuspendCond()
    , mSuspending(MFALSE)
{
    mLogLevel = ::property_get_int32("vendor.debug.camera.log", 0);
    if (mLogLevel < 2) {
        MINT32 level = ::property_get_int32("vendor.debug.camera.log.sim3a", 0);
        mLogLevel = (mLogLevel > level) ? mLogLevel : level;
    }
    #if HAL3A_SIMULATOR_FORCE_ENABLE_LOG
    #warning "[FIXME] force enable Hal3ASimulator log"
    mLogLevel = 2;
    #endif
    //
    mvNumQueue.clear();
    mvMetaQueue.erase(mvMetaQueue.begin(), mvMetaQueue.end());
    MY_LOGI("Hal3ASimulator(%d)", i4SensorIdx);
};


/******************************************************************************
 *
 ******************************************************************************/
MINT32
Hal3ASimulator::
doConfig(const ConfigInfo_T& rConfigInfo)
{
    MY_LOGD_IF(mLogLevel >= 1, "(%d)", rConfigInfo.i4SubsampleCount);
    mSubsampleCount = (rConfigInfo.i4SubsampleCount > 0) ?
        rConfigInfo.i4SubsampleCount : 1;
    return 0;
}


/******************************************************************************
 *
 ******************************************************************************/
MINT32
Hal3ASimulator::
config(const ConfigInfo_T& rConfigInfo)
{
    MY_LOGD_IF(mLogLevel >= 1, "3A simulator ConfigInfo.SubsampleCount(%d)",
        rConfigInfo.i4SubsampleCount);
    CAM_TRACE_NAME("Hal3ASim::configInfo");
    std::lock_guard<std::mutex> lock(mLock);
    return doConfig(rConfigInfo);
}


/******************************************************************************
 *
 ******************************************************************************/
MINT32
Hal3ASimulator::
config(MINT32 i4SubsampleCount)
{
    MY_LOGD_IF(mLogLevel >= 1, "3A simulator SubsampleCount(%d)",
        i4SubsampleCount);
    CAM_TRACE_NAME("Hal3ASim::configCnt");
    std::lock_guard<std::mutex> lock(mLock);
    ConfigInfo_T info;
    info.i4SubsampleCount = i4SubsampleCount;
    return doConfig(info);
}


/******************************************************************************
 *
 ******************************************************************************/
MINT32
Hal3ASimulator::
start(MINT32 i4StartNum)
{
    MY_LOGD_IF(mLogLevel >= 1, "3A simulator start (%d)", i4StartNum);
    CAM_TRACE_NAME("Hal3ASim::start");
    std::lock_guard<std::mutex> lock(mLock);
    return doStart(i4StartNum);
}


/******************************************************************************
 *
 ******************************************************************************/
MINT32
Hal3ASimulator::
doStart(MINT32 i4StartNum)
{
    MY_LOGD_IF(mLogLevel >= 1, "doStart (%d)", i4StartNum);
    CAM_TRACE_NAME("Hal3ASim::doStart");
    #if HAL3A_SIMULATOR_REAL_SOF
    if (mpCamIO == NULL) {
        #ifdef USING_MTK_LDVT //[FIXME]
        mpCamIOUserName = "iopipeUseTM";
        #else
        mpCamIOUserName = "Hal3ASimulator";
        #endif

        getNormalPipeModule()->createSubModule(
            mSensorIdx, mpCamIOUserName, getNormalPipeModuleVersion(mSensorIdx),
            (MVOID**)&mpCamIO);
        //
        if (mpCamIO == NULL) {
            MY_LOGW("CamIO create fail");
            return 0;
        }
        mUserKey = mpCamIO->attach("S3A_CB");
        mTimeoutMs = mCallbackTimeIntervalUs / ((MINT64)50);
        // driver default timeout = AvgFrameTimeIntervalMs x 20
        //                        = (mCallbackTimeIntervalUs / 1000) * 20;
        //                        = mCallbackTimeIntervalUs / 50;
    }
    #endif
    {
        std::lock_guard<std::mutex> lock(mNumQueueLock);
        mvNumQueue.clear();
    }
    //
    mShutterDelayNum = DEF_SHUTTER_DELAY_NUM;
    #if 0 // assign shutter-delay-num by sensor define
    MY_LOGI("ShutterDelayNum(%d)(%d)", mShutterDelayNum, DEF_SHUTTER_DELAY_NUM);
    #endif
    //
    mLastSofIdx = INVALID_SOF_INDEX;
    mLastCallbackTime = 0;
    mpCallbackThread = new CallbackThread(this);
    mpCallbackThread->run();
    MY_LOGD_IF(mLogLevel >= 1, "3A simulator thread start (%d)", i4StartNum);
    return 0;
}


/******************************************************************************
 *
 ******************************************************************************/
MINT32
Hal3ASimulator::
stop()
{
    MY_LOGD_IF(mLogLevel >= 1, "3A simulator stop");
    CAM_TRACE_NAME("Hal3ASim::stop");
    std::lock_guard<std::mutex> lock(mLock);
    return doStop();
}


/******************************************************************************
 *
 ******************************************************************************/
MINT32
Hal3ASimulator::
doStop()
{
    MY_LOGD_IF(mLogLevel >= 1, "doStop");
    CAM_TRACE_NAME("Hal3ASim::doStop");
    {
        std::lock_guard<std::mutex> lock(mCallbackLock);
        mCallbackEnable = MFALSE;
        mCallbackSkip = 0;
        mCallbackCount = 0;
    }
    setSuspend(MFALSE);
    mpCallbackThread->requestExit();
    #if HAL3A_SIMULATOR_REAL_SOF
    if (mpCamIO != NULL) {
        MY_LOGD_IF(mLogLevel >= 1, "doStop - stop signal");
        mpCamIO->signal(
            NSCam::NSIoPipe::NSCamIOPipe::EPipeSignal_SOF,
            mUserKey);
    }
    #endif
    mpCallbackThread->join();
    free(mpCallbackThread);
    MY_LOGD_IF(mLogLevel >= 1, "doStop - thread stop");
    //
    #if HAL3A_SIMULATOR_REAL_SOF
    if (mpCamIO != NULL) {
        mpCamIO->destroyInstance(mpCamIOUserName);
        mpCamIO = NULL;
    }
    #endif
    return 0;
}

/*******************************************************************************************
*******************************For Previous Compatibility***********************************
********************************************************************************************/

MINT32
Hal3ASimulator::
startCapture(const std::list<MetaSet_T>&, MINT32, MINT32 )
{
    // Only for ISP4.X
    return 0;
}

MINT32
Hal3ASimulator::
startRequestQ(const std::list<MetaSet_T>&, MINT32)
{
    // Only for ISP4.x
    return 0;
}

MINT32
Hal3ASimulator::
set(const std::list<MetaSet_T>&, MINT32)
{
    // Only for ISP4.x
    return 0;
}
/******************************************************************************
 *
 ******************************************************************************/
MINT32
Hal3ASimulator::
startRequestQ(const vector<MetaSet_T*>& requestQ)
{
    MY_LOGD_IF(mLogLevel >= 1, "3A simulator startRequest List(%zu)", requestQ.size());
    CAM_TRACE_NAME("Hal3ASim::startRequestQ");
    std::lock_guard<std::mutex> lock(mLock);
    doStart(0);
    setRequest(requestQ);
    return MFALSE;
}


/******************************************************************************
 *
 ******************************************************************************/
MINT32
Hal3ASimulator::
startCapture(const vector<MetaSet_T*>& requestQ, MINT32 i4StartNum)
{
    MY_LOGD_IF(mLogLevel >= 1, "3A simulator startCapture (%d)", i4StartNum);
    CAM_TRACE_NAME("Hal3ASim::startCapture");
    std::lock_guard<std::mutex> lock(mLock);
    doStart(i4StartNum);
    setMetaList(requestQ);
    return ESTART_CAP_NORMAL;//ESTART_CAP_MANUAL;
}


/******************************************************************************
 *
 ******************************************************************************/
MINT32
Hal3ASimulator::
setIsp(
    MINT32 flowType,
    const MetaSet_T& control,
    TuningParam* pRegBuf,
    MetaSet_T* pResult
)
{
    MY_LOGD_IF(mLogLevel >= 1,
        "3A simulator setIsp type(%d) reg(%p) result(%p)",
        flowType, pRegBuf, (void*)pResult);
    CAM_TRACE_NAME("Hal3ASim::setIsp");
    std::lock_guard<std::mutex> lock(mLock);
    return setIspDefault(flowType, control, pRegBuf, pResult);
}

/******************************************************************************
 *
 ******************************************************************************/
MINT32
Hal3ASimulator::
dumpIsp(MINT32 flowType, const MetaSet_T& control, TuningParam* pTuningBuf, MetaSet_T* pResult)

{
    return MTRUE;
}


/******************************************************************************
 *
 ******************************************************************************/
MINT32
Hal3ASimulator::
setIspDefault(
    MINT32 flowType,
    const MetaSet_T& control,
    TuningParam* pRegBuf,
    MetaSet_T* pResult
)
{
    if (pResult != NULL) {
        pResult->appMeta = control.appMeta;
        pResult->halMeta = control.halMeta;
        MY_LOGD_IF(mLogLevel >= 1, "3A simulator setIsp - assign control "
            "metadata to result type(%d) [%p]", flowType, (void*)pRegBuf);
    } else {
        MY_LOGD_IF(mLogLevel >= 1, "3A simulator setIsp - ignore control "
            "metadata to result type(%d) [%p]", flowType, (void*)pRegBuf);
    }
    #if 0 // clear tuning and return 0
    if ((pRegBuf != NULL) && (pRegBuf->pRegBuf != NULL)){
        char* name = "3A-simulator-setIsp";
        NSIoPipe::NSPostProc::INormalStream* pipe = NULL;
        pipe = NSIoPipe::NSPostProc::INormalStream::createInstance(mSensorIdx);
        if (pipe == NULL) {
            MY_LOGE("normal stream - create failed");
            return (-1);
        }
        if (!pipe->init(name)) {
            MY_LOGE("normal stream - init failed");
            if (!pipe->uninit(name)) {
                MY_LOGE("normal stream - uninit failed");
            }
            pipe->destroyInstance();
            return (-1);
        }
        unsigned int tuningsize = pipe->getRegTableSize();//sizeof(dip_x_reg_t);
        MY_LOGD_IF(mLogLevel >= 1, "3A simulator setIsp - "
            "get tuning RegBuf Size(%d)", tuningsize);
        ::memset((unsigned char*)(pRegBuf->pRegBuf), 0, tuningsize);
        MY_LOGD_IF(mLogLevel >= 1, "3A simulator setIsp - "
            "clear tuning RegBuf");
        if (!pipe->uninit(name)) {
            MY_LOGE("normal stream - uninit failed");
        }
        pipe->destroyInstance();
        return (0);
    }
    #endif
    return (-1);
}


/******************************************************************************
 *
 ******************************************************************************/
MINT32
Hal3ASimulator::
attachCb(IHal3ACb::ECb_T eId, IHal3ACb* pCb)
{
    std::lock_guard<std::mutex> lock(mLock);
    std::lock_guard<std::mutex> cblock(mCallbackLock);
    if (eId < IHal3ACb::eID_MSGTYPE_NUM) {
        mCbSet[eId] = pCb;
        MY_LOGD_IF(mLogLevel >= 2, "attachCb %p at [%d]", pCb, eId);
    } else {
        MY_LOGW("3A simulator can not attachCb at [%d]", eId);
    }
    return 0;
}


/******************************************************************************
 *
 ******************************************************************************/
MINT32
Hal3ASimulator::
detachCb(IHal3ACb::ECb_T eId, IHal3ACb* pCb)
{
    std::lock_guard<std::mutex> lock(mLock);
    std::lock_guard<std::mutex> cblock(mCallbackLock);
    if (eId < IHal3ACb::eID_MSGTYPE_NUM) {
        if (mCbSet[eId] == pCb) {
            mCbSet[eId] = NULL;
            MY_LOGD_IF(mLogLevel >= 2, "detachCb %p at [%d]", pCb, eId);
        } else {
            MY_LOGW("3A simulator can not find detachCb %p at [%d]", pCb, eId);
        }
    } else {
        MY_LOGW("3A simulator can not detachCb at [%d]", eId);
    }
    return 0;
}


/******************************************************************************
 *
 ******************************************************************************/
void
Hal3ASimulator::
doPause(void)
{
    MY_LOGD_IF(mLogLevel >= 1, "doPause");
    setSuspend(MTRUE);
    return;
}


/******************************************************************************
 *
 ******************************************************************************/
void
Hal3ASimulator::
doResume(MINT32)
{
    MY_LOGD_IF(mLogLevel >= 1, "doResume");
    setSuspend(MFALSE);
    return;

}


/******************************************************************************
 *
 ******************************************************************************/
void
Hal3ASimulator::
setSuspend(MBOOL suspend)
{
    std::lock_guard<std::mutex> lock(mSuspendLock);
    MBOOL broadcast = ((mSuspending) && (!suspend)) ? MTRUE : MFALSE;
    mSuspending = suspend;
    if (broadcast) {
        mSuspendCond.notify_all();
    }
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
Hal3ASimulator::
getSuspend(void)
{
    std::lock_guard<std::mutex> lock(mSuspendLock);
    return mSuspending;
}


/******************************************************************************
 *
 ******************************************************************************/
void
Hal3ASimulator::
waitSuspend(void)
{
    std::unique_lock<std::mutex> lock(mSuspendLock);
    if (mSuspending) {
        CAM_TRACE_NAME("Hal3ASim::waitSuspend");
        MY_LOGI("waitSuspend +++");
        mSuspendCond.wait(lock);
        MY_LOGI("waitSuspend ---");
    }
    return;
}


/******************************************************************************
 *
 ******************************************************************************/
void
Hal3ASimulator::
performCallback(MINT32 count)
{
    MY_LOGD_IF(mLogLevel >= 2, "CB CNT(%d)", count);
    CAM_TRACE_NAME("Hal3ASim::performCallback");
    // callback if exist
    MINT32 msgType = 0;
    //
    msgType = IHal3ACb::eID_NOTIFY_VSYNC_DONE;
    if (getSuspend()) {
        MY_LOGD_IF(mLogLevel >= 2, "skip notify (%d) [%d]", count, msgType);
        return;
    }
    if (mCbSet[msgType] != NULL) {
        IHal3ACb* pCb = mCbSet[msgType];
        if (count > 0) {
            MINTPTR ext1 = (MINTPTR)(NULL);
            MINTPTR ext2 = (MINTPTR)(NULL);
            MINTPTR ext3 = (MINTPTR)(NULL);
            MY_LOGD_IF(mLogLevel >= 2,
                "do notify (%d) [%d] +++", count, msgType);
            //
            CAM_TRACE_FMT_BEGIN("Hal3ASim::CB(%d)[%d]", count, msgType);
            pCb->doNotifyCb(msgType, ext1, ext2, ext3);
            CAM_TRACE_FMT_END();
            //
            MY_LOGD_IF(mLogLevel >= 2,
                "do notify (%d) [%d] ---", count, msgType);
        }
    }
    //
    msgType = IHal3ACb::eID_NOTIFY_3APROC_FINISH;
    if (getSuspend()) {
        MY_LOGD_IF(mLogLevel >= 2, "skip notify (%d) [%d]", count, msgType);
        return;
    }
    if (mCbSet[msgType] != NULL) {
        IHal3ACb* pCb = mCbSet[msgType];
        //
        #if 1
        // after the SOF arrival,
        // it might need to take some time for this calculation
        // then, it can perform callback
        // for simulation, adjust the sleep time
        usleep(mCallbackTimeIntervalUs >> 4);
        #endif
        MINT32 idx = 0;
        MINT32 num = 0;
        RequestSet_T reqSet;
        CapParam_T cap_param;
        cap_param.i8ExposureTime = DEF_FRAME_INTERVAL_US * ((MINT64)1000); // ns
        //
        #if HAL3A_SIMULATOR_REAL_SOF
        mpCamIO->sendCommand(
            NSCam::NSIoPipe::NSCamIOPipe::ENPipeCmd_GET_CUR_SOF_IDX,
            (MINTPTR)&idx, 0, 0);
        #endif
        //
        mLastSofIdx = idx;
        //
        MINTPTR ext1 = (MINTPTR)(NULL);
        MINTPTR ext2 = (MINTPTR)(idx);
        MINTPTR ext3 = (MINTPTR)(NULL);
        MY_LOGD_IF(mLogLevel >= 2, "NumQueue[%zu]:(%d) %d %d @ %d",
            mvNumQueue.size(), getQueueNum(),
            mShutterDelayNum, mSubsampleCount, mLastSofIdx);
        if (mvNumQueue.size() >= (size_t)((mShutterDelayNum -
            DEF_SHUTTER_DELAY_NUM + 1) * mSubsampleCount)) {
            for (MINT32 i = 0; i < mSubsampleCount; i++) {
                num = deQueueNum();
                if (num > 0) {
                    reqSet.vNumberSet.push_back(num);
                } else {
                    MY_LOGW("cannot find the num at %d", i);
                    dumpQueueNum();
                    break;
                }
            }
        } else {
            dumpQueueNum();
        }
        ext1 = (MINTPTR)(&reqSet);
        ext3 = (MINTPTR)(&cap_param);
        //
        MY_LOGD_IF(mLogLevel >= 2,
            "do notify (%d) [%d] (%d,%d) +++", count, msgType, num, idx);
        //
        CAM_TRACE_FMT_BEGIN("Hal3ASim::CB(%d)[%d](%d,%d)",
            count, msgType, num, idx);
        pCb->doNotifyCb(msgType, ext1, ext2, ext3);
        CAM_TRACE_FMT_END();
        //
        MY_LOGD_IF(mLogLevel >= 2,
            "do notify (%d) [%d] (%d,%d) ---", count, msgType, num, idx);
    }
}


/******************************************************************************
 *
 ******************************************************************************/
MINT32
Hal3ASimulator::
setRequest(const vector<MetaSet_T*>& controls)
{
    MINT32 nListSize = controls.size();
    MINT32 nListSet = ((nListSize > mSubsampleCount) && (mSubsampleCount > 0)) ?
        (nListSize / mSubsampleCount) : 0;
    MINT32 nMagicNum = 0;
    MINT32 nCount = 0;
    CAM_TRACE_NAME("Hal3ASim::setRequest");

    if (nListSet < (mShutterDelayNum + 1)) {
        MY_LOGW("Set Request Meta List size not enough : %d=(%d/%d) < (%d+1)",
            nListSet, nListSize, mSubsampleCount, mShutterDelayNum);
        return nMagicNum;
    }

    #if 1 // refine shutter-delay-num by control list size
    MY_LOGI("ListSet(%d) > (%d)", nListSet, (DEF_SHUTTER_DELAY_NUM + 1));
    if (nListSet > (DEF_SHUTTER_DELAY_NUM + 1)) {
        mShutterDelayNum = (nListSet - 1);
        MY_LOGI("ShutterDelayNum(%d)", mShutterDelayNum);
    }
    #endif

    for (auto it : controls) {
        if (!tryGetMetadata<MINT32>(&(it->halMeta),
            MTK_P1NODE_PROCESSOR_MAGICNUM, nMagicNum)) {
            MY_LOGW("Set Meta List fail");
            break;
        }
        nCount ++;
        if (nCount > (nListSize - mSubsampleCount)) {
            enQueueNum(nMagicNum);
        }
        enQueueMeta(nMagicNum, *it);
    }

    MY_LOGI("Set Request Meta List size(%d/%d) sub(%d)",
        nCount, nListSize, mSubsampleCount);

    if (mLogLevel >= 2) {
        dumpQueueNum();
    }

    {
        std::lock_guard<std::mutex> cblock(mCallbackLock);
        if (!mCallbackEnable) {
            #if 0
            // after the first set,
            // it might need to take some time for the first calculation
            // then, it can start to callback
            // for simulation, adjust the sleep time
            usleep(mCallbackTimeIntervalUs / 2);
            #endif
            mCallbackEnable = MTRUE;
            mCallbackSkip = 0; // it does not need to skip in setRequest flow
            mCallbackCount = 0;
            mCallbackCond.notify_all();
        }
    }

    return nMagicNum;
}


/******************************************************************************
 *
 ******************************************************************************/
MINT32
Hal3ASimulator::
setMetaList(const vector<MetaSet_T*>& controls)
{
    CAM_TRACE_NAME("Hal3ASim::setMetaList");
    MINT32 nListSize = controls.size();
    MINT32 nMagicNum = 0;

    #ifdef HAL3A_SIMULATOR_REQ_PROC_KEY
    #undef HAL3A_SIMULATOR_REQ_PROC_KEY
    #endif
    #define HAL3A_SIMULATOR_REQ_PROC_KEY 2


    vector<MetaSet_T*>::const_iterator it = controls.begin();

    if (nListSize >= (HAL3A_SIMULATOR_REQ_PROC_KEY + 1) * mSubsampleCount) {
        for (int i = 0;
            i < ((HAL3A_SIMULATOR_REQ_PROC_KEY + 1) * mSubsampleCount) &&
                it != controls.end(); i++, it++) {
            //MY_LOGI("setMetaList(%d/%d)", i, nListSize);
            if (i >= (HAL3A_SIMULATOR_REQ_PROC_KEY * mSubsampleCount)) {
                if (!tryGetMetadata<MINT32>(&((*it)->halMeta),
                    MTK_P1NODE_PROCESSOR_MAGICNUM, nMagicNum)) {
                    MY_LOGW("Set Meta List fail");
                    break;
                }
                //MY_LOGI("setMetaList(%d/%d) = %d", i, nListSize, nMagicNum);
                enQueueNum(nMagicNum);
                #if HAL3A_SIMULATOR_PASS_METADATA
                enQueueMeta(nMagicNum, **it);
                #endif
            }
        }
    } else {
        MY_LOGI("Set Meta List size not enough : %d < (%d * %d)",
            nListSize, mSubsampleCount, (HAL3A_SIMULATOR_REQ_PROC_KEY + 1));
    }
    if (mLogLevel >= 2) {
        dumpQueueNum();
    }

    #undef HAL3A_SIMULATOR_REQ_PROC_KEY

    {
        std::lock_guard<std::mutex> cblock(mCallbackLock);
        if (!mCallbackEnable) {
            #if 0
            // after the first set,
            // it might need to take some time for the first calculation
            // then, it can start to callback
            // for simulation, adjust the sleep time
            usleep(mCallbackTimeIntervalUs / 2);
            #endif
            mCallbackEnable = MTRUE;
            mCallbackSkip = DEF_SHUTTER_DELAY_NUM; // to simulate 3A stable frame delay
            mCallbackCount = 0;
            mCallbackCond.notify_all();
        }
    }

    return nMagicNum;
}


/******************************************************************************
 *
 ******************************************************************************/
void
Hal3ASimulator::
getMetaSet(MINT32 num, MetaSet_T & meta)
{
    MY_LOGD_IF(mLogLevel >= 1, "Get Meta num(%d) to (%p)", num, &meta);
    CAM_TRACE_NAME("Hal3ASim::getMetaSet");
    #if HAL3A_SIMULATOR_PASS_METADATA
    meta = deQueueMeta(num);
    #endif
    return;
}


/******************************************************************************
 *
 ******************************************************************************/
void
Hal3ASimulator::
enQueueMeta(MINT32 num, MetaSet_T const & meta)
{
    std::lock_guard<std::mutex> lock(mMetaQueueLock);
    mvMetaQueue[num] = meta;
    return;
}


/******************************************************************************
 *
 ******************************************************************************/
MetaSet_T
Hal3ASimulator::
deQueueMeta(MINT32 num)
{
    std::lock_guard<std::mutex> lock(mMetaQueueLock);
    MetaSet_T resultMetaSet;
    std::map<MINT32,MetaSet_T>::iterator iter;
    iter = mvMetaQueue.find(num);
    if (iter == mvMetaQueue.end()) {
        MY_LOGW("Cannot find the MetaSet : %d",num);
        return resultMetaSet;
    }
    resultMetaSet = iter->second;
    mvMetaQueue.erase(iter);
    return resultMetaSet;
}


/******************************************************************************
 *
 ******************************************************************************/
void
Hal3ASimulator::
enQueueNum(MINT32 num)
{
    std::lock_guard<std::mutex> lock(mNumQueueLock);
    mvNumQueue.push_back(num);
    return;
}


/******************************************************************************
 *
 ******************************************************************************/
MINT32
Hal3ASimulator::
deQueueNum(void)
{
    std::lock_guard<std::mutex> lock(mNumQueueLock);
    MINT32 num = -1;
    if (mvNumQueue.size() > 0) {
        vector<MINT32>::iterator it = mvNumQueue.begin();
        num = *(it);
        mvNumQueue.erase(it);
    }
    return num;
}


/******************************************************************************
 *
 ******************************************************************************/
MINT32
Hal3ASimulator::
getQueueNum(MUINT32 index)
{
    std::lock_guard<std::mutex> lock(mNumQueueLock);
    MINT32 num = -1;
    if ((mvNumQueue.size() > 0) && (index < mvNumQueue.size())) {
        num = mvNumQueue[index];
    }
    return num;
}


/******************************************************************************
 *
 ******************************************************************************/
void
Hal3ASimulator::
dumpQueueNum(void)
{
    std::lock_guard<std::mutex> lock(mNumQueueLock);
    std::string str =
        std::string("Q[%d] = { ", (int)(mvNumQueue.size()));
    vector<MINT32>::iterator it = mvNumQueue.begin();
    for(; it != mvNumQueue.end(); it++) {
        str += std::string(" %d ", (*it));
    }
    str += std::string(" }");
    MY_LOGD("%s", str.c_str());
    return;
}


/******************************************************************************
 *
 ******************************************************************************/
extern "C"
NS3Av3::IHal3A*
createInstance_Hal3ASimulator(MINT32 const i4SensorIdx, const char* strUser)
{
    MY_LOGD("[%d] %s", i4SensorIdx, strUser);
    switch (i4SensorIdx)
    {
    case 0:{
        static Hal3ASimulator inst(i4SensorIdx);
        return &inst;
        }break;

    case 1:{
        static Hal3ASimulator inst(i4SensorIdx);
        return &inst;
        }break;

    default:
        return NULL;
    }

    return NULL;
}

