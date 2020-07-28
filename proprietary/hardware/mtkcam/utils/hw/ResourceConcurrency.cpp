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
 * MediaTek Inc. (C) 2018. All rights reserved.
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

#define LOG_TAG "MtkCam/HwUtils/ResourceConcurrency"
//
#include <cutils/properties.h>
#include <utils/String8.h>
#include <utils/Condition.h>
#include <utils/Mutex.h>
#include <utils/RWLock.h>
//
#include <mtkcam/utils/std/Log.h>
#include <mtkcam/utils/std/ULog.h>
//
#include <mtkcam/utils/hw/IResourceConcurrency.h>

CAM_ULOG_DECLARE_MODULE_ID(MOD_UTILITY);

using namespace android;
using namespace NSCam;

/******************************************************************************
 *
 ******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_ULOGMV("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_ULOGMD("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_ULOGMI("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_ULOGMW("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_ULOGME("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("[%s] " fmt, __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( (cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( (cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)


/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interface of Resource Concurrency
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class ResourceConcurrency
    : public virtual IResourceConcurrency
{

#ifdef RES_CON_MS_TO_NS
#undef RES_CON_MS_TO_NS
#endif
#define RES_CON_MS_TO_NS(ms) ((MINT64)((ms) * (1000000LL)))

private:

    enum STATE
    {
        STATE_NONE                  = 0,
        STATE_HELD_IDLE,
        STATE_HELD_RES_ACQUIRING,
        STATE_HELD_RES_OCCUPIED,
        STATE_HELD_RES_RELEASING
    };

    class Control
    {
    public:
        Control(const char * name, MINT64 timeout_ns)
            : mStrName(name)
            , mTimeoutNs(timeout_ns)
            , mLock()
            , mCond()
            , mIsWaiting(MFALSE)
            , mIsExiting(MFALSE)
            , mUsingId(CLIENT_HANDLER_NULL)
            {};
        virtual ~Control() {};
        //
        MERROR              acquire(CLIENT_HANDLER id);
        MERROR              release(CLIENT_HANDLER id);
        //
        MVOID               settle();

    public:
        android::String8    mStrName;
        MINT64              mTimeoutNs;

    private:
        CLIENT_HANDLER      getUser() const { return mUsingId; };
        MVOID               setUser(CLIENT_HANDLER id) { mUsingId = id; };

    private:
        mutable Mutex       mLock;
        Condition           mCond;
        MBOOL               mIsWaiting;
        MBOOL               mIsExiting;
        CLIENT_HANDLER      mUsingId;

    };

    class Client
    {
    public:
        Client(CLIENT_HANDLER id, Control * ctrl)
            : mId(id)
            , mOpLock()
            , mStateLock()
            , mState(STATE_NONE)
            , mpCtrl(ctrl)
            {};
        virtual ~Client() {};
        //
        CLIENT_HANDLER      id() const { return mId; };
        //
        CLIENT_HANDLER      obtain(); // obtain this client
        MERROR              cancel(); // cancel this client
        //
        MERROR              acquire(); // acquire the resource
        MERROR              release(); // release the resource
        //
        MBOOL               isApplied() const;

    private:
        STATE               getState() const {
                                RWLock::AutoRLock _l(mStateLock);
                                return mState;
                            };
        MVOID               setState(STATE state) {
                                RWLock::AutoWLock _l(mStateLock);
                                mState = state;
                            };

    private:
        CLIENT_HANDLER      mId;
        mutable Mutex       mOpLock;
        mutable RWLock      mStateLock;
        STATE               mState;
        Control *           mpCtrl;
    };

public:
    ResourceConcurrency(
        const char * name,
        MINT64 timeout_ms
        )
        : mLock()
        , mControl(name, RES_CON_MS_TO_NS(timeout_ms))
        , mClient0(CLIENT_HANDLER_0, &mControl)
        , mClient1(CLIENT_HANDLER_1, &mControl)
        {
            MY_LOGI("name[%s] timeout(%" PRId64 "ns)", mControl.mStrName.string(), mControl.mTimeoutNs);
        };
    virtual ~ResourceConcurrency()
        {
            mControl.settle();
            if (mClient0.isApplied()) {
                MY_LOGE("name[%s] Client0 is in use", mControl.mStrName.string());
            }
            if (mClient1.isApplied()) {
                MY_LOGE("name[%s] Client1 is in use", mControl.mStrName.string());
            }
            MY_LOGI("name[%s] timeout(%" PRId64 "ns)", mControl.mStrName.string(), mControl.mTimeoutNs);
        };
    //
    virtual CLIENT_HANDLER  requestClient();
    virtual MERROR          returnClient(CLIENT_HANDLER id);
    //
    virtual MERROR          acquireResource(CLIENT_HANDLER id);
    virtual MERROR          releaseResource(CLIENT_HANDLER id);

private:
    ResourceConcurrency::Client *
                            getClient(CLIENT_HANDLER id);

private:
    mutable Mutex           mLock;
    Control                 mControl;
    Client                  mClient0;
    Client                  mClient1;
};

}  //namespace NSCam


/******************************************************************************
 *
 ******************************************************************************/
ResourceConcurrency::Client *
ResourceConcurrency::getClient(CLIENT_HANDLER id)
{
    if (id == CLIENT_HANDLER_0) {
        return &mClient0;
    }
    if (id == CLIENT_HANDLER_1) {
        return &mClient1;
    }
    return (ResourceConcurrency::Client *)(NULL);
}


/******************************************************************************
 *
 ******************************************************************************/
IResourceConcurrency::CLIENT_HANDLER
ResourceConcurrency::requestClient()
{
    Mutex::Autolock _l(mLock);
    IResourceConcurrency::CLIENT_HANDLER client = CLIENT_HANDLER_NULL;
    client = mClient0.obtain();
    if (client != CLIENT_HANDLER_NULL) {
        return client;
    }
    client = mClient1.obtain();
    if (client != CLIENT_HANDLER_NULL) {
        return client;
    }
    return CLIENT_HANDLER_NULL;
};


/******************************************************************************
 *
 ******************************************************************************/
MERROR
ResourceConcurrency::returnClient(CLIENT_HANDLER id)
{
    Mutex::Autolock _l(mLock);
    Client * pClient = getClient(id);
    if (pClient == NULL) {
        return BAD_VALUE;
    }
    return pClient->cancel();
};


/******************************************************************************
 *
 ******************************************************************************/
MERROR
ResourceConcurrency::acquireResource(CLIENT_HANDLER id)
{
    Client * pClient = getClient(id);
    if (pClient == NULL) {
        return BAD_VALUE;
    }
    return pClient->acquire();
};


/******************************************************************************
 *
 ******************************************************************************/
MERROR
ResourceConcurrency::releaseResource(CLIENT_HANDLER id)
{
    Client * pClient = getClient(id);
    if (pClient == NULL) {
        return BAD_VALUE;
    }
    return pClient->release();
};


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
ResourceConcurrency::Client::isApplied() const
{
    return (getState() != STATE_NONE);
};


/******************************************************************************
 *
 ******************************************************************************/
IResourceConcurrency::CLIENT_HANDLER
ResourceConcurrency::Client::obtain()
{
    if (mpCtrl == NULL) {
        return CLIENT_HANDLER_NULL;
    }
    //
    if (getState() == STATE_NONE) {
        setState(STATE_HELD_IDLE);
        MY_LOGI("[%s][Client-%d] client-requested , state(%d)", mpCtrl->mStrName.string(), id(), getState());
        return id();
    }
    return CLIENT_HANDLER_NULL;
};


/******************************************************************************
 *
 ******************************************************************************/
MERROR
ResourceConcurrency::Client::cancel()
{
    if (mpCtrl == NULL) {
        return BAD_VALUE;
    }
    //
    if (getState() == STATE_HELD_IDLE) {
        setState(STATE_NONE);
        MY_LOGI("[%s][Client-%d] client-returned , state(%d)", mpCtrl->mStrName.string(), id(), getState());
        return NO_ERROR;
    }
    MY_LOGW("[%s][Client-%d] incorrect , state(%d)", mpCtrl->mStrName.string(), id(), getState());
    return INVALID_OPERATION;
};


/******************************************************************************
 *
 ******************************************************************************/
MERROR
ResourceConcurrency::Client::acquire()
{
    if (mpCtrl == NULL) {
        return BAD_VALUE;
    }
    //
    Mutex::Autolock _l(mOpLock);
    if (getState() != STATE_HELD_IDLE) {
        MY_LOGW("[%s][Client-%d] incorrect , state(%d)", mpCtrl->mStrName.string(), id(), getState());
        return INVALID_OPERATION;
    }
    //
    MERROR res = NO_ERROR;
    setState(STATE_HELD_RES_ACQUIRING);
    //
    res = mpCtrl->acquire(id());
    //
    if (res == NO_ERROR) {
        setState(STATE_HELD_RES_OCCUPIED);
    } else {
        setState(STATE_HELD_IDLE);
    }
    return res;
};


/******************************************************************************
 *
 ******************************************************************************/
MERROR
ResourceConcurrency::Client::release()
{
    if (mpCtrl == NULL) {
        return BAD_VALUE;
    }
    //
    Mutex::Autolock _l(mOpLock);
    if (getState() != STATE_HELD_RES_OCCUPIED) {
        MY_LOGW("[%s][Client-%d] incorrect , state(%d)", mpCtrl->mStrName.string(), id(), getState());
        return INVALID_OPERATION;
    }
    //
    MERROR res = NO_ERROR;
    setState(STATE_HELD_RES_RELEASING);
    //
    res = mpCtrl->release(id());
    //
    setState(STATE_HELD_IDLE);
    return res;
};


/******************************************************************************
 *
 ******************************************************************************/
MERROR
ResourceConcurrency::Control::acquire(CLIENT_HANDLER id)
{
    MERROR res = NO_ERROR;
    if (id >= CLIENT_HANDLER_NULL){
        return INVALID_OPERATION;
    } else {
        Mutex::Autolock _l(mLock);
        if (mIsExiting) {
            MY_LOGE("[%s][Client-%d] acquire but exiting (%" PRId64 "ns)", mStrName.string(), id, mTimeoutNs);
            return INVALID_OPERATION;
        }
        if (getUser() == CLIENT_HANDLER_NULL) {
            MY_LOGI("[%s][Client-%d] acquire resource directly", mStrName.string(), id);
            setUser(id);
            res = NO_ERROR;
        } else if (getUser() != id) {
            MY_LOGI("[%s][Client-%d] wait resource (%" PRId64 "ns)", mStrName.string(), id, mTimeoutNs);
            mIsWaiting = MTRUE;
            res = mCond.waitRelative(mLock, (nsecs_t)(mTimeoutNs));
            mIsWaiting = MFALSE;
            if (res == NO_ERROR) {
                setUser(id);
                MY_LOGI("[%s][Client-%d] got resource (%" PRId64 "ns)", mStrName.string(), id, mTimeoutNs);
            } else {
                // timeout
                MY_LOGW("[%s][Client-%d] wait timeout (%" PRId64 "ns) res(%d)", mStrName.string(), id, mTimeoutNs, res);
            }
        } else {
            MY_LOGW("[%s][Client-%d] has this resource already", mStrName.string(), id);
            res = INVALID_OPERATION;
        }
    }
    return res;
};


/******************************************************************************
 *
 ******************************************************************************/
MERROR
ResourceConcurrency::Control::release(CLIENT_HANDLER id)
{
    MERROR res = NO_ERROR;
    if (id >= CLIENT_HANDLER_NULL){
        return INVALID_OPERATION;
    } else {
        Mutex::Autolock _l(mLock);
        if (mIsExiting) {
            MY_LOGE("[%s][Client-%d] release but exiting (%" PRId64 "ns)", mStrName.string(), id, mTimeoutNs);
            return INVALID_OPERATION;
        }
        if (getUser() == id) {
            MY_LOGI("[%s][Client-%d] free resource (%" PRId64 "ns)", mStrName.string(), id, mTimeoutNs);
            setUser(CLIENT_HANDLER_NULL);
            mCond.broadcast();
        } else {
            MY_LOGW("[%s][Client-%d] NOT has this resource", mStrName.string(), id);
            res = INVALID_OPERATION;
        }
    }
    return res;
};


/******************************************************************************
 *
 ******************************************************************************/
MVOID
ResourceConcurrency::Control::settle()
{
    {
        Mutex::Autolock _l(mLock);
        mIsExiting = MTRUE;
        if (mIsWaiting) {
            MY_LOGE("[%s][Client-%d] waiting as exiting", mStrName.string(), getUser());
            mCond.broadcast();
        }
    }
    MY_LOGD("[%s][Client-%d] settle done", mStrName.string(), getUser());
    return;
};


/******************************************************************************
 *
 ******************************************************************************/
android::sp<IResourceConcurrency>
IResourceConcurrency::
createInstance(const char * name, MINT64 timeout_ms)
{
    return (new ResourceConcurrency(name, timeout_ms));
};


