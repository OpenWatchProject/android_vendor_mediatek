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

#include <atomic>
#include <vector>
#include <memory>
#include <deque>
#include <utils/List.h>
#include <utils/Mutex.h>
#include <utils/Thread.h>
#include <utils/Condition.h>
#include <mtkcam/def/common.h>
#include <mtkcam/drv/iopipe/CamIO/INormalPipe.h>
#include <iopipe/CamIO/CamIoWrapperPublic.h>
#include "CamIoDummyBuffer.h"
#include "LmvIrqAdapter.h"


namespace NSCam {
namespace NSIoPipe {
namespace NSCamIOPipe {
namespace Wrapper {


class NormalPipeWrapper : public INormalPipe
{
public:
    static INormalPipe *createInstance(
        MUINT32 sensorIndex, char const* szCallerName, MUINT32 apiVersion,
        android::sp<IDriverFactory> driverFactory);

    virtual MVOID   destroyInstance(char const* szCallerName);

    virtual MBOOL   start();
    virtual MBOOL   stop(MBOOL bNonblocking);
    virtual MBOOL   abort();
    virtual MBOOL   init(MBOOL EnableSec = MFALSE);
    virtual MBOOL   uninit() ;
    virtual MBOOL   enque(QBufInfo const& rQBuf);
    virtual MBOOL   deque(QPortID& rQPort, QBufInfo& rQBuf, MUINT32 u4TimeoutMs);
    virtual MBOOL   reset();
    virtual MBOOL   configPipe(QInitParam const& vInPorts, MINT32 burstQnum);
    virtual MBOOL   sendCommand(MINT32 cmd, MINTPTR arg1, MINTPTR arg2, MINTPTR arg3);
    virtual MINT32  attach(const char* UserName);
    virtual MBOOL   wait(EPipeSignal eSignal, EPipeSignalClearType eClear, const MINT32 mUserKey,
                            MUINT32 TimeoutMsF, SignalTimeInfo *pTime);
    virtual MBOOL   signal(EPipeSignal eType, const MINT32 mUserKey);
    virtual MBOOL   abortDma(PortID port, char const* szCallerName);

#if 0
    // There are default implmentations in IHalCamIO
    virtual MBOOL   suspend(E_SUSPEND_TPYE etype);
    virtual MBOOL   resume(QBufInfo const *, E_SUSPEND_TPYE etype);
    virtual MBOOL   resume(MUINT64, E_SUSPEND_TPYE etype);
#endif

    virtual MUINT32 getIspReg(MUINT32 RegAddr, MUINT32 RegCount, MUINT32 RegData[], MBOOL bPhysical);
    virtual MUINT32 getIspReg(IspP1RegIo *pRegs, MUINT32 RegCount, MBOOL bPhysical);
    virtual MUINT32 getIspUniReg(IspP1RegIo *pRegs, MUINT32 RegCount, MBOOL bPhysical);

private:
    using QBufInfoIsp3 = NSIoPipeIsp3::NSCamIOPipe::QBufInfo;
    using BufInfoIsp3 = NSIoPipeIsp3::NSCamIOPipe::BufInfo;

    struct Statistics {
        int numOfUserEnqued;
        int numOfUserDequed;
        int numOfDummysGen;
        int numOfCompEnqued;
        int numOfCompDequed;
        int numOfDummyDequed;
        std::atomic_int numOfDropped;

        Statistics() :
            numOfUserEnqued(0), numOfUserDequed(0), numOfDummysGen(0),
            numOfCompEnqued(0), numOfCompDequed(0), numOfDummyDequed(0),
            numOfDropped(0)
        {
        }

        void reset() {
            numOfUserEnqued = 0;
            numOfUserDequed = 0;
            numOfDummysGen = 0;
            numOfCompEnqued = 0;
            numOfCompDequed = 0;
            numOfDummyDequed = 0;
            numOfDropped.store(0, std::memory_order_relaxed);
        }
    };

    struct InitBufInfo {
        bool isValid;
        MSize mDstSize;
        MRect mCropRect;
        MUINT32 mRawOutFmt;
        MUINT32 mSOFidx;

        InitBufInfo() :
            isValid(false), mDstSize(), mCropRect(),  mRawOutFmt(0), mSOFidx(0)
        {
        }
    };

    struct DummyEnque {
        bool isCompensation;
        MUINT32 magicNum;
        android::sp<IImageBuffer> imgoBuffer;
        android::sp<IImageBuffer> rrzoBuffer;

        DummyEnque(bool _isCompensation, MUINT32 _magicNum,
                android::sp<IImageBuffer> &_imgoBuffer, android::sp<IImageBuffer> &_rrzoBuffer) :
            isCompensation(_isCompensation), magicNum(_magicNum), imgoBuffer(_imgoBuffer), rrzoBuffer(_rrzoBuffer)
        {
        }
    };

    struct DropCallback {
        fp_DropCB m_DropCB;   //call back for drop enque request
        void*     m_returnCookie;

        DropCallback() : m_DropCB(NULL), m_returnCookie(NULL) { }
    };

    enum StateEnum {
        STATE_NONE     = 0,
        STATE_INITED   = 0x1,
        STATE_STARTED  = 0x2,
        STATE_STOPPED  = 0x4,
        STATE_UNINITED = 0x8,
        STATE_DEQUE_ERROR = 0x10
    };
    typedef unsigned int State; // bit set of StateEnum

    static const MUINT32 MAGIC_NUM_PREFIX = 0x1fff;

    static android::Mutex sInstanceLock;
    static NormalPipeWrapper* spInstance[EPIPE_Sensor_RSVD];
    static int sNextInstanceId;

    int mInstanceId;
    static constexpr int VERI_SLOT_SIZE = 4;
    NormalPipeWrapper *mVeriSlot[VERI_SLOT_SIZE];

    const MUINT32 mSensorIndex;
    int mUsersCreated;
    int mUsersInited;
    State mState;
    std::atomic_bool mIsActive;
    unsigned int mDebugFlag;
    android::Mutex mDequeStopMutex;
    android::Mutex mOpMutex;
    android::List<QBufInfo> mEnquedBufferQueue;
    bool mIsDummyInfoInited;
    android::List<DummyEnque> mDummyEnques;
    DummyBuffer mDummyBufferImgo;
    DummyBuffer mDummyBufferRrzo;
    MUINT32 mInitMagicNum;
    InitBufInfo mInitInfoImgo, mInitInfoRrzo;
    Statistics mStatistics;
    bool mIsLmvEnabled;
    android::sp<LmvIrqAdapter> mLmvAdapter;
    android::sp<IDriverFactory> mDriverFactory;
    INormalPipeIsp3 *mNormalPipeIsp3;
    IspDrv *mIspDrv;
    MINT32 mIspDrvUserKey;
    DropCallback mDropCallback;
    std::vector<MUINT32> mDroppedMagicNums;
    android::Mutex mDroppedMutex;
    android::Mutex mDequeListMutex;
    std::vector<NSIoPipeIsp3::PortID> mConfiguredPorts;
    std::deque<std::unique_ptr<QBufInfoIsp3>> mDequedList;
    android::Condition mDequeCond;

    // Call createInstance() instead
    NormalPipeWrapper(
        int instanceId,
        MUINT32 sensorIndex,
        android::sp<IDriverFactory> &driverFactory);

    // Call destroyInstance() instead
    ~NormalPipeWrapper();

    bool checkState(const char *tag, State shouldSet, State shouldClear = STATE_NONE);

    bool isStateSet(State state) {
        return (mState & state) != 0;
    }

    void setState(const char *tag, State toSet, State toClear = STATE_NONE);
    void logQueueEvent(const char *event, BufInfoIsp3 *imgoBufInfo, BufInfoIsp3 *rrzoBufInfo);

    MBOOL stopInternal();
    static void cloneBufInfo(const BufInfoIsp3 &src, BufInfo &dest);
    static NSCam::NSIoPipeIsp3::PortID transPortIdToIsp3(const PortID &portId);
    void initDummyInfo(QBufInfo const& rQBuf);
    bool generateDummyQBufInfo(QBufInfoIsp3 &enBuf, const char *caller, bool isCompensation, MUINT32 magicNum);
    void releaseDummy(DummyEnque &dummyEnque);
    void compensatorillyEnque(MUINT32 magicNum);
    void enqueDummy(MUINT32 magicNum);
    bool isCompensationAndDiscarded(BufInfoIsp3 *imgoBufInfo, BufInfoIsp3 *rrzoBufInfo);
    bool dequeExceptCompensation(std::unique_ptr<QBufInfoIsp3> &deBuf, bool &maybeDelayed);
    void broadcastDequedEvent(BufInfoIsp3 *imgoBufInfoIsp3, BufInfoIsp3 *rrzoBufInfoIsp3);
    auto getEnquedBufferIter(BufInfoIsp3 *imgoBufInfoIsp3, BufInfoIsp3 *rrzoBufInfoIsp3);
    void waitForSOFIfDropped();
    static void onDrvDropped(MUINT32 magic, void* cookie);
    void removeDropped();
    void abortDequeThread();

    template <typename _Ret, typename _QBufInfo, typename _PortID>
    static _Ret findBufInfo(_QBufInfo &rQBuf, _PortID portId);

    friend class DequeThread;
    class DequeThread : public android::Thread {
    public:
        DequeThread(NormalPipeWrapper &wrapper);

    private:
        NormalPipeWrapper &mWrapper;
        virtual bool threadLoop() override;
        bool dequeExceptDummy(QBufInfoIsp3 &deBuf, MUINT32 timeoutInMs);
    };

    android::sp<DequeThread> mDequeThread;
};


template <typename _Ret, typename _QBufInfo, typename _PortID>
_Ret NormalPipeWrapper::findBufInfo(_QBufInfo &rQBuf, _PortID portId)
{
    for (auto bufIt = rQBuf.mvOut.begin(); bufIt != rQBuf.mvOut.end(); bufIt++) {
        if (bufIt->mPortID == portId)
            return &(*bufIt);
    }

    return nullptr;
}


}
}
}
}

