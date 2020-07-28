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

#define LOG_TAG "mtkcam-ppl_context"
//
#include <mtkcam3/pipeline/pipeline/IPipelineContext.h>
#include "MyUtils.h"
//
#include <atomic>
#include <memory>
#include <sstream>
#include <string>
#include <mtkcam/utils/std/ULog.h>

CAM_ULOG_DECLARE_MODULE_ID(MOD_PIPELINE_CONTEXT);
//
using namespace android;
using namespace NSCam;
using namespace NSCam::v3;
using namespace NSCam::v3::pipeline;
using namespace NSCam::v3::pipeline::NSPipelineContext;


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
#define MY_LOGV_IF(cond, ...)       do { if (            (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if (            (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if (            (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( CC_UNLIKELY(cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( CC_UNLIKELY(cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( CC_UNLIKELY(cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( CC_UNLIKELY(cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)


/******************************************************************************
 *
 ******************************************************************************/
namespace {
class NodeActorImpl : public INodeActor
{
public:
    using INodeOpsActor = implementation::INodeOpsActor;

    enum
    {
        eState_NoInit,
        eState_Initializing,
        eState_Initialized,
        eState_Configuring,
        eState_Configured,
        eState_Reconfiguring,
    };

protected:
    mutable android::Mutex          mLock;
    std::atomic_uint32_t            mState{eState_NoInit};

    std::shared_ptr<INodeOpsActor>  mpNodeOps;

    /**
     * init()/config() order
     *
     * The smaller value, the higher priority (order).
     * -1: don't care (free run)
     */
    int32_t                         mInitOrder = -1;

    /**
     * >=0: a timeout in nanoseconds
     * < 0: no timeout (by default)
     */
    int64_t                         mInitTimeout = -1;

    InitParamsT                     mInitParam;

    /**
     * The elapsed time, in nanoseconds, of init() and config().
     */
    int64_t                         mInitElapsedTime = 0;

public:
                    NodeActorImpl(std::shared_ptr<INodeOpsActor> pNodeOps)
                        : INodeActor()
                        , mpNodeOps(pNodeOps)
                    {
                    }

protected:
    static  auto    stateToString(uint32_t state) -> char const*;
    virtual auto    onInit() -> int     { return mpNodeOps->getNode()->init(mInitParam); }
    virtual auto    onConfig() -> int   { return mpNodeOps->config(mpNodeOps->getConfigParam()); }
    virtual auto    onUninit() -> int   { return mpNodeOps->getNode()->uninit(); }

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  RefBase Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:
    virtual void    onLastStrongRef(const void* id);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  INodeActor Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    virtual auto    init() -> int;
    virtual auto    config() -> int;
    virtual auto    uninit() -> int;
    virtual auto    reconfig() -> int;

public:
    virtual auto    toString() const -> std::string;

public:
    virtual auto    getNode() -> IPipelineNode*         { return mpNodeOps->getNode(); }
    virtual auto    getNodeId() const -> NodeId_T       { return mInitParam.nodeId; }
    virtual auto    getNodeName() const -> char const*  { return mInitParam.nodeName != nullptr ? mInitParam.nodeName : "?"; }
    virtual auto    getInitOrder() const -> int32_t     { return mInitOrder; }
    virtual auto    setInitOrder(int32_t order) -> void { mInitOrder = order; }
    virtual auto    getInitTimeout() const -> int64_t   { return mInitTimeout; }
    virtual auto    setInitTimeout(int64_t t) -> void   { mInitTimeout = t; }
    virtual auto    setInitParam(InitParamsT const& rParam) -> void { mInitParam = rParam; }
    virtual auto    setConfigParam(void const* pConfigParams) -> void { mpNodeOps->setConfigParam(pConfigParams); }

public:
    virtual auto    setInitElapsedTime(int64_t t) -> void { mInitElapsedTime = t; }

};
}; // namespace


/******************************************************************************
 *
 ******************************************************************************/
extern "C" INodeActor* createNodeActor(std::shared_ptr<implementation::INodeOpsActor> pNodeOps)
{
    return new NodeActorImpl(pNodeOps);
}


/******************************************************************************
 *
 ******************************************************************************/
auto
NodeActorImpl::
stateToString(uint32_t state) -> char const*
{
    switch (state)
    {
    case eState_NoInit:         return "NO-INIT";
    case eState_Initializing:   return "INIT-ING";
    case eState_Initialized:    return "INITIALIZED";
    case eState_Configuring:    return "CONFIG-ING";
    case eState_Configured:     return "CONFIGURED";
    case eState_Reconfiguring:  return "RECONFIGURED";
    default: break;
    }
    return "BAD STATE";
}


/******************************************************************************
 *
 ******************************************************************************/
auto
NodeActorImpl::
toString() const -> std::string
{
    // For example: "nodeId=0x1 (P1Node) CONFIGURED"
    std::ostringstream oss;
    oss << "nodeId=0x" << std::hex << getNodeId() << std::dec
        << " (" << getNodeName() << ")"
        << " " << stateToString(mState.load())
        ;
    if ( mInitElapsedTime != 0 ) {
        oss << "(" << mInitElapsedTime/1000000 << "ms)";
    }
    return oss.str();
}


/******************************************************************************
 *
 ******************************************************************************/
void
NodeActorImpl::
onLastStrongRef(const void* /*id*/)
{
    android::Mutex::Autolock _l(mLock);
    //
    if( mState.load() != eState_NoInit ) {
        onUninit();
    }
}


/******************************************************************************
 *
 ******************************************************************************/
#define PRECHECK_STATE( _status_, _skip_st_, _expected_ )                       \
    do {                                                                        \
        if (CC_UNLIKELY( _status_ >= _skip_st_ )) {                             \
            MY_LOGD("%s already in state %d", getNodeName(), _status_.load());  \
            return OK;                                                          \
        }                                                                       \
        else if (CC_UNLIKELY( _status_ != _expected_ )) {                       \
            MY_LOGE("%s current state %d is bad; expected state %d",            \
                getNodeName(), _status_.load(), _expected_);                    \
            return INVALID_OPERATION;                                           \
        }                                                                       \
    } while(0)

#define UPDATE_STATE_IF_OK( _status_, _newstatus_, _err_ )                      \
    do {                                                                        \
        if (CC_LIKELY( 0 == _err_ )) {                                          \
            _status_ = _newstatus_;                                             \
        }                                                                       \
        else {                                                                  \
            MY_LOGE("%s err=%d", getNodeName(), _err_);                         \
        }                                                                       \
    } while(0)


/******************************************************************************
 *
 ******************************************************************************/
int
NodeActorImpl::
init()
{
    android::Mutex::Autolock _l(mLock);

    PRECHECK_STATE(     mState, eState_Initializing, eState_NoInit);
    UPDATE_STATE_IF_OK( mState, eState_Initializing, OK);

    int const ret = onInit();

    PRECHECK_STATE(     mState, eState_Initialized, eState_Initializing);
    UPDATE_STATE_IF_OK( mState, eState_Initialized, ret);

    return ret;
}


/******************************************************************************
 *
 ******************************************************************************/
int
NodeActorImpl::
config()
{
    android::Mutex::Autolock _l(mLock);

    PRECHECK_STATE(     mState, eState_Configuring, eState_Initialized);
    UPDATE_STATE_IF_OK( mState, eState_Configuring, OK);

    int const ret = onConfig();

    PRECHECK_STATE(     mState, eState_Configured, eState_Configuring);
    UPDATE_STATE_IF_OK( mState, eState_Configured, ret);

    return ret;
}


/******************************************************************************
 *
 ******************************************************************************/
int
NodeActorImpl::
reconfig()
{
    android::Mutex::Autolock _l(mLock);

    PRECHECK_STATE(     mState, eState_Reconfiguring, eState_Configured);
    UPDATE_STATE_IF_OK( mState, eState_Reconfiguring, OK);

    int const ret = onConfig();

    UPDATE_STATE_IF_OK( mState, eState_Configured, ret);

    return ret;
}


/******************************************************************************
 *
 ******************************************************************************/
int
NodeActorImpl::
uninit()
{
    android::Mutex::Autolock _l(mLock);
    if( mState.load() == eState_NoInit ) {
        MY_LOGD("already uninit or not init");
        return OK;
    }
    //
    int const ret = onUninit();
    if( ret != OK ) {
        MY_LOGE("uninit failed");
    }
    // always update
    mState = eState_NoInit;
    return ret;
}

