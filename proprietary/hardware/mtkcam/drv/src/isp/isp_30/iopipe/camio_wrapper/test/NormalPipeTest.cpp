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

#define LOG_TAG "NormalPipeTest"

#include <chrono>
#include <thread>
#include <random>
#include <future>
#include <list>
#include <utils/Mutex.h>
#include <utils/List.h>
#include <gtest/gtest.h>
#include <mtkcam/utils/std/Log.h>
#include <mtkcam/utils/std/ULog.h>
#include <mtkcam/utils/module/module.h>
#include <mtkcam/def/common.h>
#include <drv/isp_drv.h>
#include <iopipe/CamIO/CamIoWrapperPublic.h>
#include <CamIoWrapperUtils.h>
#include <NormalPipeWrapper.h>
#include <LmvIrqAdapter.h>

CAM_ULOG_DECLARE_MODULE_ID(MOD_DRV_NORMAL_PIPE);


static const MUINT32 API_VERSION = MTKCAM_MAKE_API_VERSION(1, 0);


using namespace android;
using namespace NSCam;
using namespace NSCam::NSIoPipe;
using namespace NSCam::NSIoPipe::NSCamIOPipe;
using QBufInfoIsp3 = NSCam::NSIoPipeIsp3::NSCamIOPipe::QBufInfo;
using BufInfoIsp3 = NSCam::NSIoPipeIsp3::NSCamIOPipe::BufInfo;


#define TEST_LOGD(fmt, arg...) do { CAM_ULOGMD("[%s]" fmt, __func__, ##arg); } while(0)

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"


namespace NSCam {
namespace NSIoPipe {
namespace NSCamIOPipe {
namespace Wrapper {


class IrqSimulator
{
public:
    static IrqSimulator& get();

    MINT64 signalVsync() {
        Mutex::Autolock lock(signalMutex);
        mVsyncTimeStamp = mClockTimeStamp;
        mVsync.broadcast();
        return mVsyncTimeStamp;
    }

    MINT64 signalP1Done() {
        Mutex::Autolock lock(signalMutex);
        mP1Done.broadcast();
        return mVsyncTimeStamp;
    }

    bool waitForSomebodyWaiting(MUINT timeoutInMs) {
        Mutex::Autolock lock(signalMutex);
        if (mWaitingCount > 0)
            return true;
        mWaitingCond.waitRelative(signalMutex, static_cast<nsecs_t>(timeoutInMs) * 1000000LL);
        return (mWaitingCount > 0);
    }

    MINT64 waitForVsync(MUINT timeoutInMs) {
        Mutex::Autolock lock(signalMutex);
        // Ignore spurious wakeup as this is UT code

        mWaitingCount++;
        mWaitingCond.broadcast();
        mVsync.waitRelative(signalMutex, static_cast<nsecs_t>(timeoutInMs) * 1000000LL);
        mWaitingCount--;

        return mVsyncTimeStamp;
    }

    MINT64 waitForP1Done(MUINT timeoutInMs) {
        Mutex::Autolock lock(signalMutex);

        // UT code don't caure the timeing accuracy
        mWaitingCount++;
        mWaitingCond.broadcast();
        while (mSkipP1Done > 0) {
            mP1Done.waitRelative(signalMutex, static_cast<nsecs_t>(timeoutInMs) * 1000000LL);
            mSkipP1Done--;
        }

        // No blocking waiting
        // Otherwise the LmvIrqThread will not be waken when NormalPipe stop
        mP1Done.waitRelative(signalMutex, static_cast<nsecs_t>(timeoutInMs) * 1000000LL);
        mWaitingCount--;

        return mVsyncTimeStamp;
    }

    void incClockTimeStamp() {
        Mutex::Autolock lock(signalMutex);
        mClockTimeStamp += 33000LL;
    }

    MINT64 getClockTimeStamp() {
        Mutex::Autolock lock(signalMutex);
        return mClockTimeStamp;
    }

    void setSkipP1Done(int count) {
        mSkipP1Done = count;
    }

private:
    IrqSimulator() : mClockTimeStamp(1000000000LL), mVsyncTimeStamp(0), mWaitingCount(0), mSkipP1Done(0) {
    }

    Mutex signalMutex;
    MINT64 mClockTimeStamp;
    MINT64 mVsyncTimeStamp;
    int mWaitingCount;
    Condition mVsync;
    Condition mP1Done;
    Condition mWaitingCond;
    int mSkipP1Done;
};


IrqSimulator& IrqSimulator::get()
{
    static IrqSimulator singleton;
    return singleton;
}


class IspDrvMock : public IspDrv
{
    public:
        IspDrvMock(): IspDrv() { }
        virtual ~IspDrvMock() {}
    //
    public:
        virtual void    destroyInstance(void) { }
        virtual MINT32 isp_fd_open(const char* userName) { return 0; }
        virtual MINT32 isp_fd_close(MINT32 mIspFd) { return 0; }
        virtual MBOOL   init(const char* userName) { return MTRUE; }
        virtual MBOOL   uninit(const char* userName) { return MTRUE; }
        virtual MBOOL   waitIrq(ISP_DRV_WAIT_IRQ_STRUCT* pWaitIrq) override;
        virtual MINT32   registerIrq(const char* userName) { return 0; }
        virtual MBOOL   markIrq(ISP_DRV_WAIT_IRQ_STRUCT Irqinfo) { return MTRUE; }
        virtual MBOOL   flushIrq(ISP_DRV_WAIT_IRQ_STRUCT Irqinfo) { return MTRUE; }
        virtual MBOOL   queryirqtimeinfo(ISP_DRV_WAIT_IRQ_STRUCT* Irqinfo) { return MTRUE; }
        virtual MBOOL   readIrq(ISP_DRV_READ_IRQ_STRUCT* pReadIrq) { return MTRUE; }
        virtual MBOOL   checkIrq(ISP_DRV_CHECK_IRQ_STRUCT CheckIrq) { return MTRUE; }
        virtual MBOOL   clearIrq(ISP_DRV_CLEAR_IRQ_STRUCT ClearIrq) { return MTRUE; }
    virtual MBOOL   unregisterIrq(ISP_DRV_WAIT_IRQ_STRUCT WaitIrq) { return MTRUE; }
        virtual MBOOL   reset(MINT32 rstpath) { return MTRUE; }
        virtual MBOOL   resetBuf(void) { return MTRUE; }
    virtual MBOOL   checkCQBufAllocated(void) { return MTRUE; }
        //wrapper
        virtual MBOOL   readRegs(
            ISP_DRV_REG_IO_STRUCT*  pRegIo,
            MUINT32                 Count,
            MINT32                  caller) { return MTRUE; }
        virtual MUINT32 readReg(MUINT32 Addr,MINT32 caller) { return 0; }
        virtual MBOOL   writeRegs(
            ISP_DRV_REG_IO_STRUCT*  pRegIo,
            MUINT32                 Count,
            MINT32                  userEnum,
            MINT32                  caller) { return MTRUE; }
        virtual MBOOL   writeReg(
            MUINT32     Addr,
            unsigned long     Data,
            MINT32      userEnum,
            MINT32      caller) { return MTRUE; }
        //
        virtual MBOOL   holdReg(MBOOL En) { return MTRUE; }
        virtual MBOOL   dumpReg(void) { return MTRUE; }
        virtual MBOOL   checkTopReg(MUINT32 Addr) { return MTRUE; }
        virtual isp_reg_t*  getCurHWRegValues() { return NULL; }
        virtual MUINT32*  getRegAddr(void) { return NULL; }
        //
        //commandQ

        virtual IspDrv* getCQInstance(MINT32 cq) { return NULL; }
        virtual MBOOL   cqAddModule(MINT32 cq,MINT32 burstQIdx,MINT32 dupCqIdx, CAM_MODULE_ENUM moduleId) { return MTRUE; }
        virtual MBOOL   cqDelModule(MINT32 cq,MINT32 burstQIdx,MINT32 dupCqIdx, CAM_MODULE_ENUM moduleId) { return MTRUE; }
        virtual int getCqModuleInfo(MINT32 cq,MINT32 burstQIdx,MINT32 dupCqIdx, CAM_MODULE_ENUM moduleId) { return 0; }
        virtual MUINT32* getCQDescBufPhyAddr(MINT32 cq,MINT32 burstQIdx,MINT32 dupCqIdx) { return NULL; }
        virtual MUINT32* getCQDescBufVirAddr(MINT32 cq,MINT32 burstQIdx,MINT32 dupCqIdx) { return NULL; }
        virtual MUINT32* getCQVirBufVirAddr(MINT32 cq,MINT32 burstQIdx,MINT32 dupCqIdx) { return NULL; }
        virtual MINT32  getRealCQIndex(MINT32 cqBaseEnum,MINT32 burstQIdx, MINT32 dupCqIdx) { return 0; }
        virtual MBOOL   setCQTriggerMode(
            ISP_DRV_CQ_ENUM cq,
            ISP_DRV_CQ_TRIGGER_MODE_ENUM mode,
            ISP_DRV_CQ_TRIGGER_SOURCE_ENUM trig_src) { return MTRUE; }
        //
        virtual MBOOL rtBufCtrl(void *pBuf_ctrl) { return MTRUE; }

        virtual MUINT32 pipeCountInc(EIspDrvPipePath ePipePath) { return 0; }
        virtual MUINT32 pipeCountDec(EIspDrvPipePath ePipePath) { return 0; }
        //
        virtual MBOOL   ISPWakeLockCtrl(MBOOL WakeLockEn) { return MTRUE; }

        // load default setting
        virtual MBOOL loadInitSetting(void) { return MTRUE; }
        // debug information
        virtual MBOOL dumpCQTable(ISP_DRV_CQ_ENUM cq,MINT32 burstQIdx, MUINT32 dupCqIdx, ISP_DRV_CQ_CMD_DESC_STRUCT* cqDesVa, MUINT32* cqVirVa) { return MTRUE; }
        //idx0:0 for p1 , 1 for p1_d , 2 for camsv, 3 for camsv_d
        //idx1:debug flag init.    MTRUE/FALSE
        //idx2:prt log.                MTRUE/FALSE
        virtual MBOOL dumpDBGLog(MUINT32* P1,IspDumpDbgLogP2Package* pP2Packages) { return MTRUE; }
        // for turning update
        virtual MBOOL   getCqModuleInf(CAM_MODULE_ENUM moduleId, MUINT32 &addrOffset, MUINT32 &moduleSize) { return MTRUE; }
        virtual MBOOL   bypassTuningQue(ESoftwareScenario softScenario, MINT32 magicNum) { return MTRUE; }
        virtual MBOOL   deTuningQue(ETuningQueAccessPath path, ESoftwareScenario softScenario, MINT32 magicNum) { return MTRUE; }
        virtual MBOOL   enTuningQue(ETuningQueAccessPath path, ESoftwareScenario softScenario, MINT32 magicNum, EIspTuningMgrFunc updateFuncBit) { return MTRUE; }
        virtual MBOOL   deTuningQueByCq(ETuningQueAccessPath path, ISP_DRV_CQ_ENUM eCq, MINT32 magicNum) { return MTRUE; }
        virtual MBOOL   enTuningQueByCq(ETuningQueAccessPath path, ISP_DRV_CQ_ENUM eCq, MINT32 magicNum, EIspTuningMgrFunc updateFuncBit) { return MTRUE; }
        virtual MBOOL   setP2TuningStatus(ISP_DRV_CQ_ENUM cq, MBOOL en) { return MTRUE; }
        virtual MBOOL   getP2TuningStatus(ISP_DRV_CQ_ENUM cq) { return MTRUE; }
        virtual MUINT64 getTuningUpdateFuncBit(ETuningQueAccessPath ePath, MINT32 magicNum, MINT32 cq,MUINT32 drvScenario) { return 0; }
        virtual MUINT32* getTuningBuf(ETuningQueAccessPath ePath, MINT32 cq) { return NULL; }
        virtual MUINT32 getTuningTop(ETuningQueAccessPath ePath, ETuningTopEn top, MINT32 cq, MUINT32 magicNum) { return 0; }
        virtual MBOOL   getCqInfoFromScenario(ESoftwareScenario softScenario, ISP_DRV_CQ_ENUM &cq) { return MTRUE; }
        virtual MBOOL   getP2cqInfoFromScenario(ESoftwareScenario softScenario, ISP_DRV_P2_CQ_ENUM &p2Cq) { return MTRUE; }
        virtual MBOOL   getTuningTpipeFiled(ISP_DRV_P2_CQ_ENUM p2Cq, MUINT32* pTuningBuf, stIspTuningTpipeFieldInf &pTuningField) { return MTRUE; }
        virtual MBOOL   mapCqToP2Cq(ISP_DRV_CQ_ENUM cq, ISP_DRV_P2_CQ_ENUM &p2Cq) { return MTRUE; }
        // slow motion feature, support burst Queue control
        virtual MBOOL  cqTableControl(ISP_DRV_CQTABLE_CTRL_ENUM cmd,int burstQnum) { return MTRUE; }
        virtual MINT32 cqNumInfoControl(ISP_DRV_CQNUMINFO_CTRL_ENUM cmd,int newValue) { return 0; }
        //
        // enqueue/dequeue control in ihalpipewarpper
        virtual MBOOL   enqueP2Frame(MUINT32 callerID,MINT32 p2burstQIdx,MINT32 p2dupCQIdx,MINT32 frameNum) { return MTRUE; }
        virtual MBOOL   waitP2Deque() { return MTRUE; }
        virtual MBOOL   dequeP2FrameSuccess(MUINT32 callerID,MINT32 p2dupCQIdx) { return MTRUE; }
        virtual MBOOL   dequeP2FrameFail(MUINT32 callerID,MINT32 p2dupCQIdx) { return MTRUE; }
        virtual MBOOL   waitP2Frame(MUINT32 callerID,MINT32 p2dupCQIdx,MINT32 timeoutUs) { return MTRUE; }
        virtual MBOOL   wakeP2WaitedFrames() { return MTRUE; }
        virtual MBOOL   freeAllP2Frames() { return MTRUE; }
        //
        //update/query register Scenario
        virtual MBOOL   updateScenarioValue(MUINT32 value) { return MTRUE; }
        virtual MBOOL   queryScenarioValue(MUINT32& value) { return MTRUE; }
        //
        //temp remove later
        virtual isp_reg_t* getRegAddrMap(void) { return NULL; }

        virtual MBOOL getIspCQModuleInfo(CAM_MODULE_ENUM eModule,ISP_DRV_CQ_MODULE_INFO_STRUCT &outInfo) { return MTRUE; }
        virtual MBOOL SetFPS(MUINT32 _fps) { return MTRUE; }
        virtual MBOOL updateCq0bRingBuf(void *pOBval) { return MTRUE; }
    protected:
        virtual MBOOL dumpP2DebugLog(IspDumpDbgLogP2Package* pP2Package) { return MTRUE; }
        virtual MBOOL dumpP1DebugLog(MUINT32* P1) { return MTRUE; }
};


MBOOL IspDrvMock::waitIrq(ISP_DRV_WAIT_IRQ_STRUCT* pWaitIrq)
{
    LOG_FUNCTION_LIFE();

    if (pWaitIrq->Clear != ISP_DRV_IRQ_CLEAR_STATUS &&
        pWaitIrq->UserInfo.Type == ISP_DRV_IRQ_TYPE_INT_P1_ST)
    {
        switch (pWaitIrq->UserInfo.Status) {
        case CAM_CTL_INT_P1_STATUS_VS1_INT_ST:
            {
                MINT64 timeStamp = IrqSimulator::get().waitForVsync(pWaitIrq->Timeout);
                pWaitIrq->EisMeta.tLastSOF2P1done_sec = timeStamp / 1000000000LL;
                pWaitIrq->EisMeta.tLastSOF2P1done_usec = (timeStamp % 1000000000LL) / 1000LL;
                TEST_LOGD("P1_STATUS_VS1_INT_ST: timeStamp = %" PRId64, timeStamp);
                return MTRUE;
            }

        case CAM_CTL_INT_P1_STATUS_PASS1_DON_ST:
            {
                MINT64 timeStamp = IrqSimulator::get().waitForP1Done(pWaitIrq->Timeout);
                pWaitIrq->EisMeta.tLastSOF2P1done_sec = timeStamp / 1000000000LL;
                pWaitIrq->EisMeta.tLastSOF2P1done_usec = (timeStamp % 1000000000LL) / 1000LL;
                TEST_LOGD("PASS1_DON_ST: timeStamp = %" PRId64, timeStamp);
                return MTRUE;
            }
        }
    }

    return MTRUE;
}


// We don't care OO design principles here because this is UT code
class NormalPipeIsp3Mock : public INormalPipeIsp3
{
public:
    enum StateEnum {
        STATE_NONE     = 0,
        STATE_INITED   = (1 << 1),
        STATE_STARTED  = (1 << 2),
        STATE_STOPPED  = (1 << 3),
        STATE_UNINITED = (1 << 4)
    };
    typedef unsigned int State;

    bool checkState(State shouldSet, State shouldClear = STATE_NONE, State toSet = STATE_NONE, State toClear = STATE_NONE) {
        if ((mState & shouldSet) != shouldSet || (mState & shouldClear) != 0) {
            CAM_ULOGME("State not match: 0x%x, shouldSet = 0x%x, shouldClear = 0x%x", mState, shouldSet, shouldClear);
            return false;
        }

        State prev = mState;
        mState |= toSet;
        mState &= ~toClear;
        if (prev != mState) {
            TEST_LOGD("State changed: 0x%x -> 0x%x", prev, mState);
        }

        return true;
    }

    virtual MBOOL sendCommand(MINT32 cmd, MINTPTR arg1, MINTPTR arg2, MINTPTR arg3) override;

    virtual MBOOL DummyFrame(QBufInfoIsp3 const& rQBuf) {
        Mutex::Autolock lock(mMutex);

        EXPECT_TRUE(checkState(STATE_INITED));
        mDummyBuf.push_back(rQBuf);
        TEST_LOGD("mDummyBuf.size() = %zu", mDummyBuf.size());

        return MTRUE;
    }

    virtual MVOID   destroyInstance(char const* szCallerName) override {
        Mutex::Autolock lock(mMutex);
        TEST_LOGD("%s, createdCount = %d -> %d", szCallerName, mCreatedCount, mCreatedCount - 1);
        mCreatedCount--;
    }

    virtual MBOOL   start() override {
        Mutex::Autolock lock(mMutex);

        TEST_LOGD("");
        EXPECT_TRUE(checkState(STATE_INITED, STATE_NONE, STATE_STARTED));
        EXPECT_EQ(3U, mDummyBuf.size());

        return MTRUE;
    }

    virtual MBOOL   stop() override {
        Mutex::Autolock lock(mMutex);

        TEST_LOGD("");
        EXPECT_TRUE(checkState(STATE_INITED | STATE_STARTED, STATE_NONE, STATE_STOPPED, STATE_STARTED));
        mDummyBuf.clear();
        mEnquedBuf.clear();

        return MTRUE;
    }

    virtual MBOOL   init() override;

    virtual MBOOL   uninit() override {
        Mutex::Autolock lock(mMutex);

        TEST_LOGD("");
        EXPECT_TRUE(checkState(STATE_INITED, STATE_NONE, STATE_UNINITED, STATE_INITED));

        return MTRUE;
    }

    virtual MBOOL   enque(QBufInfoIsp3 const& rQBuf) override;

    virtual MBOOL   deque(QBufInfoIsp3& rQBuf, MUINT32 u4TimeoutMs) override;

    virtual MBOOL   Reset() override {
        Mutex::Autolock lock(mMutex);
        mResetCalled++;
        TEST_LOGD("state = 0x%x, mResetCalled+1 = %d", mState, mResetCalled);
        return MTRUE;
    }

    //one-time conifg
    virtual MBOOL   configPipe(NSCam::NSIoPipeIsp3::NSCamIOPipe::QInitParam const& vInPorts) override {
        Mutex::Autolock lock(mMutex);
        TEST_LOGD("state = 0x%x", mState);
        return MTRUE;
    }

    //run-time config
    virtual MBOOL   configFrame(NSCam::NSIoPipeIsp3::NSCamIOPipe::QFrameParam const& rQParam) override {
        TEST_LOGD();
        return MTRUE;
    }

    //replace buffer
    virtual MBOOL   replace(BufInfoIsp3 const& bufOld, BufInfoIsp3 const& bufNew) override {
        TEST_LOGD();
        return MTRUE;
    }

public:
    virtual MVOID   attach(NSCam::NSIoPipeIsp3::NSCamIOPipe::ISignalConsumer *pConsumer,
            MINT32 sensorIdx, NSCam::NSIoPipeIsp3::NSCamIOPipe::EPipeSignal eType) override
    {
        TEST_LOGD();
    }

    virtual MBOOL   wait(NSCam::NSIoPipeIsp3::NSCamIOPipe::ISignalConsumer *pConsumer,
            MINT32 sensorIdx, NSCam::NSIoPipeIsp3::NSCamIOPipe::EPipeSignal eType, uint64_t ns_timeout = 0xFFFFFFFF) override
    {
        TEST_LOGD();
        return MTRUE;
    }

public:
    NormalPipeIsp3Mock() : mState(STATE_NONE), mCreatedCount(0),
        mResetCalled(0), mForceDequeFail(0), mForceDequeDummy(0)
    {
    }

    virtual ~NormalPipeIsp3Mock() { }

    State mState;
    Mutex mMutex;
    List<QBufInfoIsp3> mDummyBuf;
    List<QBufInfoIsp3> mEnquedBuf;
    int mCreatedCount;
    int mResetCalled;
    int mForceDequeFail;
    int mForceDequeDummy;

    void setForceDequeFail(int forceDequeFail) {
        mForceDequeFail = forceDequeFail;
    }

    void setForceDequeDummy(int forceDequeDummy) {
        mForceDequeDummy = forceDequeDummy;
    }
};


MBOOL NormalPipeIsp3Mock::init()
{
    Mutex::Autolock lock(mMutex);

    TEST_LOGD("");
    EXPECT_TRUE(checkState(STATE_NONE, STATE_INITED | STATE_STARTED, STATE_INITED, STATE_UNINITED));
    mEnquedBuf.clear();
    mDummyBuf.clear();
    mResetCalled = 0;
    mForceDequeFail = 0;
    mForceDequeDummy = 0;

    return MTRUE;
}


MBOOL NormalPipeIsp3Mock::enque(QBufInfoIsp3 const& rQBuf)
{
    Mutex::Autolock lock(mMutex);

    EXPECT_TRUE(checkState(STATE_INITED));
    mEnquedBuf.push_back(rQBuf);
    TEST_LOGD("mEnquedBuf.size() = %zu", mEnquedBuf.size());

    return MTRUE;
}


MBOOL NormalPipeIsp3Mock::deque(QBufInfoIsp3& rQBuf, MUINT32 u4TimeoutMs)
{
    Mutex::Autolock lock(mMutex);

    EXPECT_TRUE(checkState(STATE_INITED | STATE_STARTED, STATE_STOPPED | STATE_UNINITED));

    if (mForceDequeFail > 0) {
        mForceDequeFail--;
        return MFALSE;
    }

    IrqSimulator &irq = IrqSimulator::get();
    irq.incClockTimeStamp();
    MINT64 timeStamp = irq.signalVsync();
    irq.signalP1Done();

    if (mForceDequeDummy > 0) {
        rQBuf = *mDummyBuf.begin();
        TEST_LOGD("Deque dummy");
    } else {
        rQBuf = *mEnquedBuf.begin();
    }

    for (auto &bufInfo : rQBuf.mvOut) {
        bufInfo.mMetaData.mTimeStamp = timeStamp;
        bufInfo.mMetaData.mMagicNum_hal = bufInfo.FrameBased.mMagicNum_tuning;
    }

    if (mForceDequeDummy > 0) {
        mForceDequeDummy--;
    } else {
        mEnquedBuf.erase(mEnquedBuf.begin());
    }

    TEST_LOGD("timeStamp = %" PRId64 ", mEnquedBuf.size() = %zu", timeStamp, mEnquedBuf.size());

    return MTRUE;
}


MBOOL NormalPipeIsp3Mock::sendCommand(MINT32 cmd, MINTPTR arg1, MINTPTR arg2, MINTPTR arg3)
{
    TEST_LOGD("%d: 0x%" PRIxPTR "(%" PRIdPTR "), 0x%" PRIxPTR ", 0x%" PRIxPTR, cmd, arg1, arg1, arg2, arg3);

    switch (cmd) {
    case NSImageioIsp3::NSIspio::EPIPECmd_GET_MODULE_HANDLE:
        if (arg1 == NSImageioIsp3::NSIspio::EModule_EISO) {
            *reinterpret_cast<MINTPTR*>(arg2) = LmvIrqAdapter::UT_MODULE_HANDLE;
        }
        break;
    case NSImageioIsp3::NSIspio::EPIPECmd_SET_MODULE_CFG_DONE:
        EXPECT_TRUE(arg1 == LmvIrqAdapter::UT_MODULE_HANDLE);
        break;
    case NSImageioIsp3::NSIspio::EPIPECmd_RELEASE_MODULE_HANDLE:
        EXPECT_TRUE(arg1 == LmvIrqAdapter::UT_MODULE_HANDLE);
        break;
    }

    return MTRUE;
}


class UTDriverFactory : public IDriverFactory
{
public:
    virtual INormalPipeIsp3 *createNormalPipeIsp3(MUINT32 sensorIndex, char const* szCallerName, MINT32 burstQnum) override;
    virtual IspDrv *createIspDrvIsp3() override { return &mIspDrv; }
    virtual ~UTDriverFactory() { }

    NormalPipeIsp3Mock mNormalPipeIsp3;
    IspDrvMock mIspDrv;
};


INormalPipeIsp3 *UTDriverFactory::createNormalPipeIsp3(
        MUINT32 sensorIndex, char const* szCallerName, MINT32 burstQnum)
{
    Mutex::Autolock lock(mNormalPipeIsp3.mMutex);

    TEST_LOGD("sensorIndex = %u, createdCount = %d -> %d", sensorIndex,
        mNormalPipeIsp3.mCreatedCount, mNormalPipeIsp3.mCreatedCount + 1);

    mNormalPipeIsp3.mCreatedCount++;

    return &mNormalPipeIsp3;
}

#pragma clang diagnostic pop


namespace NormalPipeTest {
    constexpr int IMGO_W = 1920, IMGO_H = 1080, RRZO_W = 960, RRZO_H = 540;
    constexpr int IMGO_SIZE = IMGO_W * IMGO_H, RRZO_SIZE = RRZO_W * RRZO_H;
    constexpr int EISO_SIZE = 256;
}

using namespace NormalPipeTest;

class ImageHeapSingleton
{
public:
    sp<IImageBuffer> createImgoBuffer() {
        if (!prepareHeap(mImgoHeap, IMGO_SIZE))
            return NULL;
        return mImgoHeap->createImageBuffer();
    }

    sp<IImageBuffer> createRrzoBuffer() {
        if (!prepareHeap(mRrzoHeap, RRZO_SIZE))
            return NULL;
        return mRrzoHeap->createImageBuffer();
    }

    sp<IImageBuffer> createEisoBuffer() {
        if (!prepareHeap(mEisoHeap, EISO_SIZE))
            return NULL;
        return mEisoHeap->createImageBuffer();
    }

    void release() {
        mImgoHeap = NULL;
        mRrzoHeap = NULL;
        mEisoHeap = NULL;
    }

    static ImageHeapSingleton &get() {
        return sInstance;
    }

private:
    static ImageHeapSingleton sInstance;

    sp<IImageBufferHeap> mImgoHeap;
    sp<IImageBufferHeap> mRrzoHeap;
    sp<IImageBufferHeap> mEisoHeap;

    static bool prepareHeap(sp<IImageBufferHeap> &heap, size_t bufferSize) {
        if (heap == NULL) {
            IImageBufferAllocator::ImgParam imgParam(bufferSize, 0);
            heap = IIonImageBufferHeap::create(LOG_TAG, imgParam);
        }

        return (heap != NULL);
    }
};


ImageHeapSingleton ImageHeapSingleton::sInstance;



static bool generateBasicQBufInfo(QBufInfo &eBufInfo,
    sp<IImageBuffer> &imgoBuffer, sp<IImageBuffer> &rrzoBuffer, MUINT32 magicNum)
{
    imgoBuffer = ImageHeapSingleton::get().createImgoBuffer();
    rrzoBuffer = ImageHeapSingleton::get().createRrzoBuffer();

    if (imgoBuffer == NULL || rrzoBuffer == NULL) {
        CAM_ULOGME("generateBasicQBufInfo create buffers failed");
        return false;
    }

    eBufInfo.mvOut.push_back(
        BufInfo(
            NSCam::NSIoPipe::PORT_IMGO,
            imgoBuffer.get(),
            imgoBuffer->getImgSize(),
            MRect(MPoint(0, 0), imgoBuffer->getImgSize()),
            magicNum,
            0)
    );
    eBufInfo.mvOut.push_back(
        BufInfo(
            NSCam::NSIoPipe::PORT_RRZO,
            rrzoBuffer.get(),
            rrzoBuffer->getImgSize(),
            MRect(MPoint(0, 0), rrzoBuffer->getImgSize()),
            magicNum,
            0)
    );

    return true;
}


static QInitParam genQInitParamBasic()
{
    std::vector<IHalSensor::ConfigParam> sensorConfig;
    std::vector<portInfo> rPortInfo;
    rPortInfo.emplace_back(
        PORT_IMGO,
        eImgFmt_RAW_OPAQUE,
        MSize(IMGO_W, IMGO_H),
        MRect(MPoint(0, 0), MSize(IMGO_W, IMGO_H)),
        IMGO_W,
        0,
        0);
    rPortInfo.emplace_back(
        PORT_RRZO,
        eImgFmt_RAW_OPAQUE,
        MSize(RRZO_W, RRZO_H),
        MRect(MPoint(0, 0), MSize(RRZO_W, RRZO_H)),
        RRZO_W,
        0,
        0);

    MUINT32 bitDepth = 10;
    QInitParam initParam(0, bitDepth, sensorConfig, rPortInfo);

    return initParam;
}


}
}
}
}


using namespace NSCam::NSIoPipe::NSCamIOPipe::Wrapper;

TEST(TestNormalPipeWrapper, Basic)
{
    sp<UTDriverFactory> driverFactory = new UTDriverFactory;
    INormalPipe *normalPipe = NormalPipeWrapper::createInstance(0, LOG_TAG, API_VERSION, driverFactory);
    ASSERT_NE(nullptr, normalPipe);

    MBOOL isInited = normalPipe->init();
    ASSERT_TRUE(isInited);

    QInitParam initParam = genQInitParamBasic();
    MBOOL isConfigured = normalPipe->configPipe(initParam);

    QBufInfo eBufInfo;
    sp<IImageBuffer> imgoBuffer, rrzoBuffer;
    bool isBufInfoGen = generateBasicQBufInfo(eBufInfo, imgoBuffer, rrzoBuffer, 1);
    ASSERT_TRUE(isBufInfoGen);

    MBOOL isEnqued = normalPipe->enque(eBufInfo);
    EXPECT_TRUE(isEnqued);

    MBOOL isStarted = normalPipe->start();
    ASSERT_TRUE(isStarted);
    EXPECT_EQ(3U, driverFactory->mNormalPipeIsp3.mDummyBuf.size());
    EXPECT_EQ(3U, driverFactory->mNormalPipeIsp3.mEnquedBuf.size());

    QBufInfo dBufInfo;
    QPortID dequePortId = { { NSCam::NSIoPipe::PORT_IMGO, NSCam::NSIoPipe::PORT_RRZO } };
    MBOOL isDequed = normalPipe->deque(dequePortId, dBufInfo);
    EXPECT_TRUE(isDequed);

    MBOOL isStopped = normalPipe->stop();
    EXPECT_TRUE(isStopped);

    MBOOL isUninited = normalPipe->uninit();
    EXPECT_TRUE(isUninited);

    normalPipe->destroyInstance(LOG_TAG);
    EXPECT_EQ(0, driverFactory->mNormalPipeIsp3.mCreatedCount);
}


TEST(TestNormalPipeWrapper, MultiEnqueDeque)
{
    sp<UTDriverFactory> driverFactory = new UTDriverFactory;
    INormalPipe *normalPipe = NormalPipeWrapper::createInstance(0, LOG_TAG, API_VERSION, driverFactory);
    ASSERT_NE(nullptr, normalPipe);

    QInitParam initParam = genQInitParamBasic();

    static constexpr int NUM_OF_ENQUES = 3;
    MUINT32 magicNum = 4000;
    QBufInfo eBufInfo[NUM_OF_ENQUES];
    sp<IImageBuffer> imgoBuffer[NUM_OF_ENQUES], rrzoBuffer[NUM_OF_ENQUES];

    bool isBufInfoGen = generateBasicQBufInfo(eBufInfo[0], imgoBuffer[0], rrzoBuffer[0], magicNum);
    ASSERT_TRUE(isBufInfoGen);

    MBOOL isStarted =
        normalPipe->init() &&
        normalPipe->configPipe(initParam) &&
        normalPipe->enque(eBufInfo[0]) &&
        normalPipe->start();
    ASSERT_TRUE(isStarted);

    QBufInfo dBufInfo[NUM_OF_ENQUES];
    QPortID portId = { { PORT_IMGO, PORT_RRZO } };

    MBOOL isSuccess = MFALSE;
    isBufInfoGen = generateBasicQBufInfo(eBufInfo[1], imgoBuffer[1], rrzoBuffer[1], magicNum + 1);
    ASSERT_TRUE(isBufInfoGen);
    isSuccess = normalPipe->enque(eBufInfo[1]);
    EXPECT_TRUE(isSuccess);

    isSuccess = normalPipe->deque(portId, dBufInfo[0]);
    EXPECT_TRUE(isSuccess);

    isBufInfoGen = generateBasicQBufInfo(eBufInfo[2], imgoBuffer[2], rrzoBuffer[2], magicNum + 2);
    ASSERT_TRUE(isBufInfoGen);
    isSuccess = normalPipe->enque(eBufInfo[2]);
    EXPECT_TRUE(isSuccess);

    isSuccess = normalPipe->deque(portId, dBufInfo[1]);
    EXPECT_TRUE(isSuccess);
    isSuccess = normalPipe->deque(portId, dBufInfo[2]);
    EXPECT_TRUE(isSuccess);

    for (int i = 0; i < NUM_OF_ENQUES; i++) {
        EXPECT_TRUE(dBufInfo[i].mvOut[0].mBuffer == eBufInfo[i].mvOut[0].mBuffer);
        EXPECT_TRUE(dBufInfo[i].mvOut[1].mBuffer == eBufInfo[i].mvOut[1].mBuffer);
        EXPECT_TRUE(dBufInfo[i].mvOut[1].mMetaData.mMagicNum_hal == eBufInfo[i].mvOut[1].FrameBased.mMagicNum_tuning);
    }

    MBOOL isUninited =
        normalPipe->stop() &&
        normalPipe->uninit();
    EXPECT_TRUE(isUninited);

    normalPipe->destroyInstance(LOG_TAG);
}


TEST(TestNormalPipeWrapper, LMV)
{
    sp<UTDriverFactory> driverFactory = new UTDriverFactory;
    INormalPipe *normalPipe = NormalPipeWrapper::createInstance(0, LOG_TAG, API_VERSION, driverFactory);
    ASSERT_NE(nullptr, normalPipe);

    QInitParam initParam = genQInitParamBasic();
    initParam.mPortInfo.emplace_back(
        PORT_EISO,
        eImgFmt_BLOB,
        MSize(256, 1),
        MRect(MPoint(0, 0), MSize(256, 1)),
        256, 0, 0
    );

    MUINT32 magicNum = 4000;
    QBufInfo eBufInfo;
    sp<IImageBuffer> imgoBuffer, rrzoBuffer, eisoBuffer;

    bool isBufInfoGen = generateBasicQBufInfo(eBufInfo, imgoBuffer, rrzoBuffer, magicNum);
    ASSERT_TRUE(isBufInfoGen);
    eisoBuffer = ImageHeapSingleton::get().createEisoBuffer();
    ASSERT_NE(nullptr, eisoBuffer.get());
    MUINT const usage = (GRALLOC_USAGE_SW_READ_OFTEN |
                        GRALLOC_USAGE_SW_WRITE_OFTEN |
                        GRALLOC_USAGE_HW_CAMERA_READ |
                        GRALLOC_USAGE_HW_CAMERA_WRITE);
    eisoBuffer->lockBuf(LOG_TAG, usage);

    eBufInfo.mvOut.emplace_back(PORT_EISO, eisoBuffer.get());

    MBOOL isStarted =
        normalPipe->init() &&
        normalPipe->configPipe(initParam) &&
        normalPipe->enque(eBufInfo) &&
        normalPipe->start();
    EXPECT_TRUE(isStarted);

    QBufInfo dBufInfo;
    QPortID portId = { { PORT_IMGO, PORT_RRZO, PORT_EISO } };

    MBOOL isSuccess;
    isSuccess = normalPipe->deque(portId, dBufInfo);
    EXPECT_TRUE(isSuccess);
    EXPECT_TRUE(dBufInfo.mvOut.size() == portId.mvPortId.size());

    // Lmv waiting is asynchronized
    // The easiest way to verify the behavior is to check probability
    bool validExist = false, invalidExist = false;
    for (int i = 0; i < 100; i++) {
        if ((i & 0x1) == 0) {
            IrqSimulator::get().setSkipP1Done(1);
        }

        IrqSimulator::get().waitForSomebodyWaiting(5);
        isBufInfoGen = generateBasicQBufInfo(eBufInfo, imgoBuffer, rrzoBuffer, magicNum + 1 + i);
        ASSERT_TRUE(isBufInfoGen);
        isSuccess = normalPipe->enque(eBufInfo);
        EXPECT_TRUE(isSuccess);
        isSuccess = normalPipe->deque(portId, dBufInfo);
        EXPECT_TRUE(isSuccess);

        ASSERT_TRUE(dBufInfo.mvOut.size() > 2);
        if (dBufInfo.mvOut[2].mSize > 0)
            validExist = true;
        else
            invalidExist = true;

        if (validExist && invalidExist)
            break;
    }
    EXPECT_TRUE(validExist && invalidExist);

    MBOOL isUninited =
        normalPipe->stop() &&
        normalPipe->uninit();
    EXPECT_TRUE(isUninited);

    eisoBuffer->unlockBuf(LOG_TAG);
    normalPipe->destroyInstance(LOG_TAG);
}


TEST(TestNormalPipeWrapper, MultiUser)
{
    sp<UTDriverFactory> driverFactory = new UTDriverFactory;

    auto user = [driverFactory] {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<int> dis(0, 5);

        auto sleepRandom = [&gen, &dis] () {
            int ms = dis(gen);
            if (ms > 0)
                std::this_thread::sleep_for(std::chrono::milliseconds(ms));
        };

        for (int i = 0; i < 100; i++) {
            INormalPipe *normalPipe = NormalPipeWrapper::createInstance(0, LOG_TAG, API_VERSION, driverFactory);
            ASSERT_NE(nullptr, normalPipe);

            sleepRandom();

            MBOOL isStarted = normalPipe->init();
            EXPECT_TRUE(isStarted);

            sleepRandom();

            NormalPipe_EIS_Info eisInfo;
            MBOOL result = normalPipe->sendCommand(ENPipeCmd_GET_EIS_INFO, reinterpret_cast<MINTPTR>(&eisInfo), 0, 0);
            EXPECT_TRUE(result);

            sleepRandom();

            MBOOL isUninited = normalPipe->uninit();
            EXPECT_TRUE(isUninited);

            sleepRandom();

            normalPipe->destroyInstance(LOG_TAG);

            sleepRandom();
        }
    };

    std::list<std::future<void>> asyncTasks;
    for (int i = 0; i < 4; i++) {
        asyncTasks.emplace_back(std::async(std::launch::async, user));
    }

    for (auto &i : asyncTasks) {
        i.get();
    }

    EXPECT_EQ(0, driverFactory->mNormalPipeIsp3.mCreatedCount);
}


TEST(TestNormalPipeWrapper, DequeFail)
{
    sp<UTDriverFactory> driverFactory = new UTDriverFactory;
    INormalPipe *normalPipe = NormalPipeWrapper::createInstance(0, LOG_TAG, API_VERSION, driverFactory);
    ASSERT_NE(nullptr, normalPipe);

    QInitParam initParam = genQInitParamBasic();

    static constexpr int NUM_OF_ENQUES = 3;
    MUINT32 magicNum = 4000;
    QBufInfo eBufInfo[NUM_OF_ENQUES];
    sp<IImageBuffer> imgoBuffer[NUM_OF_ENQUES], rrzoBuffer[NUM_OF_ENQUES];

    bool isBufInfoGen = generateBasicQBufInfo(eBufInfo[0], imgoBuffer[0], rrzoBuffer[0], magicNum);
    ASSERT_TRUE(isBufInfoGen);

    MBOOL isStarted =
        normalPipe->init() &&
        normalPipe->configPipe(initParam) &&
        normalPipe->enque(eBufInfo[0]) &&
        normalPipe->start();
    ASSERT_TRUE(isStarted);

    QBufInfo dBufInfo[NUM_OF_ENQUES];
    QPortID portId = { { PORT_IMGO, PORT_RRZO } };

    MBOOL isSuccess = MFALSE;
    isBufInfoGen = generateBasicQBufInfo(eBufInfo[1], imgoBuffer[1], rrzoBuffer[1], magicNum + 1);
    ASSERT_TRUE(isBufInfoGen);
    isSuccess = normalPipe->enque(eBufInfo[1]);
    EXPECT_TRUE(isSuccess);
    isBufInfoGen = generateBasicQBufInfo(eBufInfo[2], imgoBuffer[2], rrzoBuffer[2], magicNum + 2);
    ASSERT_TRUE(isBufInfoGen);
    isSuccess = normalPipe->enque(eBufInfo[2]);
    EXPECT_TRUE(isSuccess);

    isSuccess = normalPipe->deque(portId, dBufInfo[0]);
    EXPECT_TRUE(isSuccess);

    // Test where reset() will be called when deque failed
    driverFactory->mNormalPipeIsp3.setForceDequeFail(2);
    isSuccess = normalPipe->deque(portId, dBufInfo[1]);
    EXPECT_TRUE(isSuccess);
    EXPECT_EQ(2, driverFactory->mNormalPipeIsp3.mResetCalled);

    driverFactory->mNormalPipeIsp3.setForceDequeFail(3);
    isSuccess = normalPipe->deque(portId, dBufInfo[2]);
    EXPECT_FALSE(isSuccess); // only reset twice
    isSuccess = normalPipe->deque(portId, dBufInfo[2]);
    EXPECT_TRUE(isSuccess);

    // Test dummy will not be dequed out
    isSuccess = normalPipe->enque(eBufInfo[1]);
    EXPECT_TRUE(isSuccess);
    driverFactory->mNormalPipeIsp3.setForceDequeDummy(1);
    isSuccess = normalPipe->deque(portId, dBufInfo[1]);
    EXPECT_TRUE(isSuccess); // dummy was covered in wrapper

    for (int i = 0; i < NUM_OF_ENQUES; i++) {
        EXPECT_TRUE(dBufInfo[i].mvOut[0].mBuffer == eBufInfo[i].mvOut[0].mBuffer);
        EXPECT_TRUE(dBufInfo[i].mvOut[1].mBuffer == eBufInfo[i].mvOut[1].mBuffer);
        EXPECT_TRUE(dBufInfo[i].mvOut[1].mMetaData.mMagicNum_hal == eBufInfo[i].mvOut[1].FrameBased.mMagicNum_tuning);
    }

    MBOOL isUninited =
        normalPipe->stop() &&
        normalPipe->uninit();
    EXPECT_TRUE(isUninited);

    normalPipe->destroyInstance(LOG_TAG);
}



int main(int argc, char *argv[])
{
    LOG_FUNCTION_LIFE();

    testing::InitGoogleTest(&argc, argv);
    int ret = RUN_ALL_TESTS();

    // We have to release the heap before main() returns
    // Otherwise the test program will halt
    ImageHeapSingleton::get().release();

    return ret;
}


