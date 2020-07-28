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

#define LOG_TAG "MtkCam/P1NodeImp"
//
#include "P1DeliverMgr.h"
#include "P1NodeImp.h"


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
using namespace std;
using namespace android;
using namespace NSCam;
using namespace NSCam::v3;
using namespace NSCam::v3::NSP1Node;
using namespace NSCam::Utils::Sync;
using namespace NSCam::NSIoPipe;
using namespace NSCam::NSIoPipe::NSCamIOPipe;
using namespace NS3Av3;


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
namespace NSCam {
namespace v3 {
namespace NSP1Node {


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

/******************************************************************************
 *
 ******************************************************************************/
P1DeliverMgr::
P1DeliverMgr()
    : mspP1NodeImp(NULL)
    , mOpenId(-1)
    , mLogLevel(0)
    , mLogLevelI(0)
    , mBurstNum(1)
    , mLoopRunning(MFALSE)
    , mLoopState(LOOP_STATE_INIT)
    , mSentNum(0)
    , mNumList()
    , mActQueue()
{
    mNumList.clear();
    mActQueue.clear();
};


/******************************************************************************
 *
 ******************************************************************************/
P1DeliverMgr::
~P1DeliverMgr()
{
    mNumList.clear();
    mActQueue.clear();
};


/******************************************************************************
 *
 ******************************************************************************/
void
P1DeliverMgr::
init(sp<P1NodeImp> pP1NodeImp)
{
    mspP1NodeImp = pP1NodeImp;
    config();
};


/******************************************************************************
 *
 ******************************************************************************/
void
P1DeliverMgr::
uninit()
{
    exit();
    if (mspP1NodeImp != NULL) {
        mspP1NodeImp = NULL;
    }
};


/******************************************************************************
 *
 ******************************************************************************/
void
P1DeliverMgr::
config()
{
    Mutex::Autolock _l(mDeliverLock);
    mNumList.clear();
    mActQueue.clear();
    if (mspP1NodeImp != NULL) {
        mOpenId = mspP1NodeImp->getOpenId();
        mLogLevel = mspP1NodeImp->mLogLevel;
        mLogLevelI = mspP1NodeImp->mLogLevelI;
        if (mspP1NodeImp->mBurstNum > 1) {
            mBurstNum = mspP1NodeImp->mBurstNum;
        }
        size_t const res_size = (size_t)(mBurstNum * P1NODE_DEF_QUEUE_DEPTH);
        mNumList.reserve(res_size);
        mNumList.clear();
        mActQueue.reserve(res_size);
        mActQueue.clear();
        MY_LOGI2("ActQueue.Capacity[%d]", (MUINT32)mActQueue.capacity());
    }
};


/******************************************************************************
 *
 ******************************************************************************/
void
P1DeliverMgr::
runningSet(MBOOL bRunning)
{
    Mutex::Autolock _l(mDeliverLock);
    mLoopRunning = bRunning;
    return;
};


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
P1DeliverMgr::
runningGet()
{
    Mutex::Autolock _l(mDeliverLock);
    return mLoopRunning;
};


/******************************************************************************
 *
 ******************************************************************************/
void
P1DeliverMgr::
exit()
{
    MY_LOGD2("DeliverMgr loop stop");
    if (mspP1NodeImp != NULL && mspP1NodeImp->mpTimingCheckerMgr != NULL) {
        U_if (mspP1NodeImp->mpTimingCheckerMgr->getEnable()) {
            MY_LOGI0("TimingCheckerMgr-stopping");
            mspP1NodeImp->mpTimingCheckerMgr->setEnable(MFALSE);
        }
    }
    //
    MY_LOGD2("DeliverMgr loop exit");
    Thread::requestExit();
    trigger();
    MY_LOGD2("DeliverMgr loop join");
    Thread::join();
    MY_LOGD2("DeliverMgr loop finish");
};


/******************************************************************************
 *
 ******************************************************************************/
status_t
P1DeliverMgr::
readyToRun()
{
    MY_LOGD2("readyToRun DeliverMgr thread");

    // set name
    ::prctl(PR_SET_NAME, (unsigned long)"CAM_P2", 0, 0, 0);
    // set normal
    struct sched_param sched_p;
    sched_p.sched_priority = 0;
    ::sched_setscheduler(0, P1THREAD_POLICY, &sched_p);
    ::setpriority(PRIO_PROCESS, 0, P1THREAD_PRIORITY);
    //  Note: "priority" is nice value.
    //
    ::sched_getparam(0, &sched_p);
    MY_LOGD2(
        "Tid: %d, policy: %d, priority: %d"
        , ::gettid(), ::sched_getscheduler(0)
        , ::getpriority(PRIO_PROCESS, 0)
    );
    //
    return OK;
};


/******************************************************************************
 *
 ******************************************************************************/
bool
P1DeliverMgr::
threadLoop()
{
    if (exitPending()) {
        MY_LOGD0("DeliverMgr try to leave");

        Mutex::Autolock _l(mDeliverLock);
        if (mActQueue.size() > 0) {
            MY_LOGI0("the deliver queue is not empty, go-on the loop");
        } else {
            MY_LOGI0("DeliverMgr Leaving");
            return MFALSE;
        }
    }

    return deliverLoop();
};


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
P1DeliverMgr::
deliverLoop()
{
    MBOOL res = MTRUE;
    std::vector< P1FrameAct > outQueue;
    outQueue.clear();
    outQueue.reserve((size_t)(mBurstNum * P1NODE_DEF_QUEUE_DEPTH));
    //
    if (mspP1NodeImp != NULL && mspP1NodeImp->mpTimingCheckerMgr != NULL) {
        mspP1NodeImp->mpTimingCheckerMgr->onCheck();
    }
    //
    if (mspP1NodeImp != NULL) { // check the DropQ
        mspP1NodeImp->onProcessDropFrame(MFALSE);
    }
    //
    {
        Mutex::Autolock _l(mDeliverLock);
        MINT32 currentNum = 0;
        //
        mLoopState = LOOP_STATE_INIT;
        currentNum = (mActQueue.empty()) ? (P1_FRM_NUM_NULL) :
            (mActQueue.at(mActQueue.size() - 1).frmNum);
        if (!exitPending()) {
            //MY_LOGD0("deliverLoop Num(%d == %d)", currentNum, mSentNum);
            if (currentNum == mSentNum) {
                mLoopState = LOOP_STATE_WAITING;
                MY_LOGD2("deliverLoop wait ++");
                #if 0 // wait forever
                mDeliverCond.wait(mDeliverLock);
                #else // force to check the delivery periodically
                status_t status = mDeliverCond.waitRelative(mDeliverLock,
                    P1_DELIVERY_CHECK_INV_NS);
                if (status != OK) {
                    MY_LOGI0("Delivery(%lld) NumList[%zu] NodeQueue[%zu]",
                        P1_DELIVERY_CHECK_INV_NS,
                        mNumList.size(), mActQueue.size());
                    if ((!mNumList.empty()) || (!mActQueue.empty())) {
                        dumpNumList(MFALSE);
                        dumpActQueue(MFALSE);
                    }
                    if (mspP1NodeImp != NULL) {
                        MBOOL needInspection = MTRUE;
                        if ((mspP1NodeImp->mpHwStateCtrl != NULL) &&
                            (mspP1NodeImp->mpHwStateCtrl->checkPausing())) {
                            MY_LOGI0("NoDelivery - SuspendDone");
                            needInspection = MFALSE;
                        }
                        if (mspP1NodeImp->syncHelperStandbyState()) {
                            MY_LOGI0("NoDelivery - SyncHelperStandby");
                            needInspection = MFALSE;
                        }
                        if (mspP1NodeImp->mLongExp.get() == MTRUE) {
                            MY_LOGI0("NoDelivery - LongExposure");
                            needInspection = MFALSE;
                        }
                        //
                        if (needInspection) {
                            mspP1NodeImp->mLogInfo.inspect(
                                LogInfo::IT_NO_DELIVERY);
                        }
                    }
                }
                #endif
                MY_LOGD2("deliverLoop wait --");
            } // else , there is new coming node need to check
        } else {
            MY_LOGD0("deliverLoop need to exit");
        }
        //
        mLoopState = LOOP_STATE_PROCESSING;
        mSentNum = currentNum;
        //
        //dumpNumList();
        //dumpActQueue();
        //
        NumList_T::iterator it_list = mNumList.begin();
        ActQueue_T::iterator it_act = mActQueue.begin();
        MBOOL isFound = MFALSE;
        size_t i = mNumList.size();
        //MY_LOGI0("NumList(%zu) ActQueue(%zu) OutQueue(%zu) +++", mNumList.size(), mActQueue.size(), outQueue.size());
        for (; it_list != mNumList.end() && i > 0; i--) {
            isFound = MFALSE;
            it_act = mActQueue.begin();
            for(; it_act != mActQueue.end(); ) {
                if (it_act->frmNum == (*it_list)) {
                    outQueue.push_back(*it_act);
                    it_act = mActQueue.erase(it_act);
                    isFound = MTRUE;
                    break;
                } else {
                    it_act++;
                }
            }
            if (isFound) { // this number of list is found in the ActQueue, go-on to find the next number of list to output
                it_list = mNumList.erase(it_list);
            } else { // this number of list is not found in the ActQueue, stop the searching and wait for the next trigger (ex: sendActQueue)
                break;
            }
        }
        // check the BYPASS act
        if (!mActQueue.empty()) { // in the most case, while BYPASS act exist, the BYPASS act is the first and the only one act in the ActQueue
            //MY_LOGI0("ActQueue(%zu) outQueue(%zu) +++", mActQueue.size(), outQueue.size());
            ActQueue_T::iterator it_check = mActQueue.begin();
            for(; it_check != mActQueue.end(); ) { // the bypass acts should be in-order in the ActQueue
                if (it_check->reqType == REQ_TYPE_ZSL
                    || it_check->reqType == REQ_TYPE_REDO
                    || it_check->reqType == REQ_TYPE_YUV
                    || it_check->reqType == REQ_TYPE_RAWIN) {
                    outQueue.push_back(*it_check);
                    it_check = mActQueue.erase(it_check); // go-on
                } else {
                    it_check++;
                }
            }
            //MY_LOGI0("ActQueue(%zu) outQueue(%zu) ---", mActQueue.size(), outQueue.size());
        }
        //MY_LOGI0("NumList(%zu) ActQueue(%zu) OutQueue(%zu) ---", mNumList.size(), mActQueue.size(), outQueue.size());
    }
    //
    if (outQueue.empty()) {
        //MY_LOGD2("there is no node need to deliver");
    } else {
        std::vector< P1FrameAct >::iterator it = outQueue.begin();
        //MY_LOGD2("outQueue.size(%d)", outQueue.size());
        for(; it != outQueue.end(); it++) {
            //MY_LOGD2("it->magicNum : (%d)", it->magicNum);
            mspP1NodeImp->releaseFrame(*it);
            /* DO NOT use this P1QueAct after releaseAction() */
        }
        outQueue.clear();
    }

    {
        Mutex::Autolock _l(mDeliverLock);
        mLoopState = LOOP_STATE_DONE;
        mDoneCond.broadcast();
    }

    return res;
};


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
P1DeliverMgr::
isActListEmpty() {
    Mutex::Autolock _l(mDeliverLock);
    U_if (!mLoopRunning) {
        return MTRUE;
    }
    return (mNumList.empty());
};


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
P1DeliverMgr::
registerActList(MINT32 num)
{
    Mutex::Autolock _l(mDeliverLock);
    U_if (!mLoopRunning) {
        return MFALSE;
    }
    mNumList.push_back(num);
    return (!mNumList.empty());
};


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
P1DeliverMgr::
sendActQueue(P1QueAct & rAct, MBOOL needTrigger)
{
    P1Act act = GET_ACT_PTR(act, rAct, MFALSE);
    P1FrameAct frameAct(rAct);
    L_if (frameAct.ready()) {
        Mutex::Autolock _l(mDeliverLock);
        mActQueue.push_back(frameAct);
    } else {
        MY_LOGE("FrameAct not ready to deliver - "
            P1INFO_ACT_STR, P1INFO_ACT_VAR(*act));
    }
    if (needTrigger) {
        trigger();
    };
    //
    return MTRUE;
};


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
P1DeliverMgr::
waitFlush(MBOOL needTrigger)
{
    size_t queueSize = 0;
    LOOP_STATE loopState = LOOP_STATE_INIT;
    MBOOL isListEmpty = MFALSE;
    if (!runningGet()) {
        return MTRUE;
    };
    //
    {
        Mutex::Autolock _l(mDeliverLock);
        queueSize = mActQueue.size();
        isListEmpty = mNumList.empty();//(mNumList.size() > 0) ? (MFALSE) : (MTRUE);
        loopState = mLoopState;
    }
    //MY_LOGI0("QueSize(%d) ListEmpty(%d) - start", queueSize, isListEmpty);
    status_t status = OK;
    while ((queueSize > 0) || (loopState == LOOP_STATE_PROCESSING) ||
        (status != OK)) {
        if (needTrigger) {
            trigger();
        }
        {
            Mutex::Autolock _l(mDeliverLock);
            if ((mActQueue.size() > 0 && mLoopState == LOOP_STATE_WAITING) ||
                (mLoopState == LOOP_STATE_PROCESSING)) {
                MY_LOGD0("doneLoop wait ++");
                status = mDoneCond.waitRelative(mDeliverLock,
                    P1_COMMON_CHECK_INV_NS);
                MY_LOGD0("doneLoop wait --");
                if (status != OK) {
                    MY_LOGI0("WaitFlushStatus(%d) LoopState(%d) "
                        "NumList[%zu] NodeQueue[%zu]", status, mLoopState,
                        mNumList.size(), mActQueue.size());
                    dumpNumList(MFALSE);
                    dumpActQueue(MFALSE);
                    if (mspP1NodeImp != NULL) {
                        mspP1NodeImp->mLogInfo.inspect(
                            LogInfo::IT_FLUSH_BLOCKING);
                    }
                }
            } else {
                status = OK;
            }
            queueSize = mActQueue.size();
            isListEmpty = mNumList.empty();//(mNumList.size() > 0) ? (MFALSE) : (MTRUE);
            loopState = mLoopState;
        }
        //MY_LOGI0("QueSize(%d) ListEmpty(%d) - next", queueSize, isListEmpty);
    };
    //MY_LOGI0("QueSize(%d) ListEmpty(%d) - end", queueSize, isListEmpty);
    //
    U_if (!isListEmpty) {
        MY_LOGW("ListEmpty(%d)", isListEmpty);
        dumpInfo();
        //dumpNumList(MTRUE);
        //dumpActQueue(MTRUE);
        return MFALSE;
    }
    return MTRUE;
};


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
P1DeliverMgr::
trigger(void)
{
    Mutex::Autolock _l(mDeliverLock);
    /*if (!mActQueue.empty())*/
    if (mLoopRunning) {
        MY_LOGD2("DeliverMgr trigger (%zu)", mActQueue.size());
        mDeliverCond.broadcast();
    }
    return MTRUE;
};


/******************************************************************************
 *
 ******************************************************************************/
MVOID
P1DeliverMgr::
dumpInfo(void)
{
    Mutex::Autolock _l(mDeliverLock);
    MY_LOGI0("DeliverMgr - Burst(%d) LoopRunning(%d) LoopState(%d)",
        mBurstNum, mLoopRunning, mLoopState);
    dumpNumList(MFALSE);
    dumpActQueue(MFALSE);
}


/******************************************************************************
 *
 ******************************************************************************/
void
P1DeliverMgr::
dumpNumList(MBOOL isLock)
{
    String8 str("");
    size_t size = 0;
    //
    NEED_LOCK(isLock, mDeliverLock);
    //
    size= mNumList.size();
    NumList_T::iterator it = mNumList.begin();
    for (; it != mNumList.end(); it++) {
        str += String8::format("%d ", *it);
    }
    //
    NEED_UNLOCK(isLock, mDeliverLock);
    //
    MY_LOGI0("dump NumList[%zu] = {%s}", size, str.string());
};


/******************************************************************************
 *
 ******************************************************************************/
void
P1DeliverMgr::
dumpActQueue(MBOOL isLock)
{
    String8 str("");
    size_t size = 0;
    //
    NEED_LOCK(isLock, mDeliverLock);
    //
    size = mActQueue.size();
    ActQueue_T::iterator it = mActQueue.begin();
    for (; it != mActQueue.end(); it++) {
        str += String8::format("%d ", (*it).queId);
    }
    //
    NEED_UNLOCK(isLock, mDeliverLock);
    //
    MY_LOGI0("dump ActQueue[%zu] = {%s}", size, str.string());
};

};//namespace NSP1Node
};//namespace v3
};//namespace NSCam


