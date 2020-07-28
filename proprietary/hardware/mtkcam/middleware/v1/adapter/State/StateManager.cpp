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

#define LOG_TAG "MtkCam/CamAdapter"
//
#include <inc/CamUtils.h>
using namespace android;
using namespace MtkCamUtils;
//
#include <inc/IState.h>
#include "State.h"
using namespace NSCamState;
//


/******************************************************************************
*
*******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_LOGV("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( (cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( (cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)

namespace android {
namespace NSCamState {
/*******************************************************************************
*   StateManager
*******************************************************************************/
class StateManager : public IStateManager
{
public:     ////            Instantiation.
    virtual void            destroyInstance();

public:
    virtual bool            init();
    virtual bool            uninit();

public:     ////            Attributes.
    virtual IState*         getCurrentState() const { return mpCurrState; }
    virtual ENState         getCurrentStateEnum() const
    {
        Mutex::Autolock _lock(mStateLock);
        return mpCurrState->getEnum();
    }

    virtual bool            isState(ENState const eState);

public:     ////            Operations.
#if 0
    //
    //  eState:
    //      [in] the state to wait.
    //
    //  timeout:
    //      [in] the timeout to wait in nanoseconds. -1 indicates no timeout.
    //
    virtual status_t        waitState(ENState const eState, nsecs_t const timeout = -1);
#endif
    virtual status_t        transitState(ENState const eNewState);

    virtual bool            registerOneShotObserver(IObserver* pObserver);
    virtual void            unregisterObserver(IObserver* pObserver);

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
private:    ////            Instantiation.
    IState*                 getStateInst(ENState const eState);
    friend class            StateObserver;

public:     ////            Instantiation.
                            StateManager();

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
protected:  ////            Data Members.
    mutable Mutex           mStateLock;
    Condition               mStateCond;
    IState* volatile        mpCurrState;        //  Pointer to the current state.
    //
    typedef List<IObserver*>ObserverList_t;
    Mutex                   mObserverLock;
    ObserverList_t          mObserverList;
    //
#define STATE_TO_ADD_STATE(_name_) \
    IState*                 mpState##_name_;
    //
    STATE_TO_ADD_STATE(Idle);
    STATE_TO_ADD_STATE(Preview);
    STATE_TO_ADD_STATE(PreCapture);
    STATE_TO_ADD_STATE(Recording);
    STATE_TO_ADD_STATE(NormalCapture);
    STATE_TO_ADD_STATE(ZSLCapture);
    STATE_TO_ADD_STATE(VideoSnapshot);
};

}; // namespace NSMtkEngCamAdapter
}; // namespace android

/*******************************************************************************
 *
 ******************************************************************************/
IState*
StateManager::
getStateInst(ENState const eState)
{
    switch  (eState)
    {
#define STATE_ENUM_TO_INST(_name_)\
    case IState::eState_##_name_:\
        {\
            return  mpState##_name_;\
        }

    STATE_ENUM_TO_INST(Idle);
    STATE_ENUM_TO_INST(Preview);
    STATE_ENUM_TO_INST(PreCapture);
    STATE_ENUM_TO_INST(Recording);
    STATE_ENUM_TO_INST(NormalCapture);
    STATE_ENUM_TO_INST(ZSLCapture);
    STATE_ENUM_TO_INST(VideoSnapshot);
    default:
            MY_LOGW("bad eState(%d)", eState);
            break;
    };
    return  NULL;
}


/*******************************************************************************
 *
 ******************************************************************************/
IStateManager*
IStateManager::
createInstance()
{
    return new StateManager;
}


/*******************************************************************************
 *
 ******************************************************************************/
void
StateManager::
destroyInstance()
{
    MY_LOGD("(%p)", this);
    delete this;
}


/*******************************************************************************
 *
 ******************************************************************************/
StateManager::
StateManager()
    : IStateManager()
    , mStateLock()
    , mStateCond()
    , mpCurrState(NULL)
    //
    , mObserverLock()
    , mObserverList()
    //
{
#define STATE_TO_NULL(_name_) \
    mpState##_name_ = NULL;

    STATE_TO_NULL(Idle);
    STATE_TO_NULL(Preview);
    STATE_TO_NULL(PreCapture);
    STATE_TO_NULL(Recording);
    STATE_TO_NULL(NormalCapture);
    STATE_TO_NULL(ZSLCapture);
    STATE_TO_NULL(VideoSnapshot);


    MY_LOGD("(%p)", this);
}


/*******************************************************************************
 *
 ******************************************************************************/
bool
StateManager::
init()
{

#define STATE_TO_INIT(_name_)\
   mpState##_name_  = new State##_name_ (IState::eState_##_name_, this);

    STATE_TO_INIT(Idle);
    STATE_TO_INIT(Preview);
    STATE_TO_INIT(PreCapture);
    STATE_TO_INIT(Recording);
    STATE_TO_INIT(NormalCapture);
    STATE_TO_INIT(ZSLCapture);
    STATE_TO_INIT(VideoSnapshot);
    //
    mpCurrState          = getStateInst(IState::eState_Idle);
    //
    return true;
}


/*******************************************************************************
 *
 ******************************************************************************/
bool
StateManager::
uninit()
{
#define STATE_TO_UNINIT(_name_) \
    delete mpState##_name_; \
    mpState##_name_ = NULL;

    STATE_TO_UNINIT(Idle);
    STATE_TO_UNINIT(Preview);
    STATE_TO_UNINIT(PreCapture);
    STATE_TO_UNINIT(Recording);
    STATE_TO_UNINIT(NormalCapture);
    STATE_TO_UNINIT(ZSLCapture);
    STATE_TO_UNINIT(VideoSnapshot);
    //
    mpCurrState          = getStateInst(IState::eState_Idle);

    return true;
}


/*******************************************************************************
 *
 ******************************************************************************/
bool
StateManager::
isState(ENState const eState)
{
    Mutex::Autolock _lock(mStateLock);
    //
#define IS_STATE(_state_,_case_) \
    if( _state_&_case_) \
    { \
        IState*const pWaitedState = getStateInst(_case_); \
        if (pWaitedState != NULL)   \
        {   \
            if  ( pWaitedState != mpCurrState ) \
            { \
                MY_LOGD("current/waited=%s/%s", mpCurrState->getName(), pWaitedState->getName()); \
            } \
            else \
            { \
                return true; \
            } \
        }   \
        else    \
        {   \
            MY_LOGW("pWaitedState is NULL (%d,%d)",_state_,_case_);    \
        }   \
    }
    //
    IS_STATE(eState,IState::eState_Idle);
    IS_STATE(eState,IState::eState_Preview);
    IS_STATE(eState,IState::eState_PreCapture);
    IS_STATE(eState,IState::eState_NormalCapture);
    IS_STATE(eState,IState::eState_ZSLCapture);
    IS_STATE(eState,IState::eState_Recording);
    IS_STATE(eState,IState::eState_VideoSnapshot);
    //
    return  false;

}

/*******************************************************************************
 *
 ******************************************************************************/
status_t
StateManager::
transitState(ENState const eNewState)
{
    IState*const pNewState = getStateInst(eNewState);
    if  ( ! pNewState )
    {
        MY_LOGW("pNewState==NULL (eNewState:%d)", eNewState);
        return  INVALID_OPERATION;
    }
    //
    {
        Mutex::Autolock _lock(mStateLock);
        MY_LOGI("%s --> %s", mpCurrState->getName(), pNewState->getName());
        mpCurrState = pNewState;
        mStateCond.broadcast();
    }
    //
    {
        Mutex::Autolock _lock(mObserverLock);
        if(mObserverList.size()==0)
        {
            MY_LOGW("mObserverList.size()==0");
        }
        for (ObserverList_t::iterator it = mObserverList.begin(); it != mObserverList.end(); it++)
        {
            (*it)->notify(eNewState);
        }
        mObserverList.clear();
    }
    return  OK;
}


/*******************************************************************************
 *
 ******************************************************************************/
bool
StateManager::
registerOneShotObserver(IObserver* pObserver)
{
    MY_LOGD("+");
    if  ( pObserver == 0 ) {
        MY_LOGW("pObserver is NULL");
        return  false;
    }
    //
    Mutex::Autolock _lock(mObserverLock);
    pObserver->notify(getCurrentState()->getEnum());
    mObserverList.push_back(pObserver);
    MY_LOGD("-");
    return  true;
}


/*******************************************************************************
 *
 ******************************************************************************/
void
StateManager::
unregisterObserver(IObserver* pObserver)
{
    Mutex::Autolock _lock(mObserverLock);
    //
    MY_LOGD("+");
    for (ObserverList_t::iterator it = mObserverList.begin(); it != mObserverList.end(); it++)
    {
        if  ( pObserver == (*it) )
        {
            MY_LOGD("(%p)", (*it));
            mObserverList.erase(it);
            break;
        }
    }
    MY_LOGD("-");
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  StateObserver
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


/*******************************************************************************
 *
 ******************************************************************************/
IStateManager::
StateObserver::
StateObserver(IStateManager* pStateManager)
    : mpStateManager(pStateManager)
    , mLock()
    , mCond()
    , meCurrState(pStateManager->getCurrentState()->getEnum())
{
}


/*******************************************************************************
 *
 ******************************************************************************/
IStateManager::
StateObserver::
~StateObserver()
{
    mpStateManager->unregisterObserver(this);
}


/*******************************************************************************
 *
 ******************************************************************************/
void
IStateManager::
StateObserver::
notify(ENState eNewState)
{
    Mutex::Autolock _lock(mLock);
    MY_LOGD("notify: meCurrState(%d) --> eNewState(%d)", meCurrState, eNewState);
    meCurrState = eNewState;
    mCond.broadcast();
}


/*******************************************************************************
 *
 ******************************************************************************/
status_t
IStateManager::
StateObserver::
waitState(ENState eState, nsecs_t const timeout)
{
    status_t status = OK;
    //
    Mutex::Autolock _lock(mLock);
    //
    ENState eInitState = meCurrState;
    //
    if  ( eState != meCurrState )
    {
        MY_LOGW("<StateObserver> + now/current/waited=%d/%d/%d, timeout(%lld)",
             mpStateManager->getCurrentState()->getEnum(),
            meCurrState,
            eState,
            (long long)timeout
            );
        switch  (timeout)
        {
        case 0:     //  not wait.
            status = TIMED_OUT;
            break;
        case -1:    //  wait without timeout.
            status = mCond.wait(mLock);
            break;
        default:    //  wait with a given timeout.
            status = mCond.waitRelative(mLock, timeout);
            break;
        }
        //
        if  ( eState != meCurrState )
        {
            status = FAILED_TRANSACTION;
        }
        //
        if  ( OK != status )
        {
            MY_LOGW(
                "<StateObserver> Timeout: now/current/waited/init=%d/%d/%d/%d, status[%s(%d)]",
                mpStateManager->getCurrentState()->getEnum(),
                meCurrState,
                eState,
                eInitState,
                ::strerror(-status), -status
            );
        }
    }
    //
    return  status;
}

