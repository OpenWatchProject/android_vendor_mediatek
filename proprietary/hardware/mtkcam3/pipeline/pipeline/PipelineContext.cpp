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

#define LOG_TAG "MtkCam/ppl_context"
//
#include "MyUtils.h"
#include "PipelineContextImpl.h"
//
#include <list>
#include <memory>
#include <string>
//
#include <mtkcam/utils/debug/debug.h>
#include <mtkcam/utils/std/ULog.h>

CAM_ULOG_DECLARE_MODULE_ID(MOD_PIPELINE_CONTEXT);
//
using namespace android;
using namespace NSCam;
using namespace NSCam::v3;
using namespace NSCam::v3::pipeline;
using namespace NSCam::v3::pipeline::NSPipelineContext;
using namespace NSCam::Utils::ULog;

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


#if 1
#define FUNC_START     MY_LOGD("+")
#define FUNC_END       MY_LOGD("-")
#else
#define FUNC_START
#define FUNC_END
#endif


/******************************************************************************
 *
 ******************************************************************************/
namespace {
class PipelineContextManager
    : public IPipelineContextManager
    , public IDebuggee
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Definitions.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////
    struct  ContextInfo
    {
        android::wp<PipelineContext>    pPipelineContext;
        struct timespec                 timestampCreated;
        std::string                     name;
    };
    using ContextInfoListT = std::list<std::shared_ptr<ContextInfo>>;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Data Members.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////
    static const std::string            mDebuggeeName;
    std::shared_ptr<IDebuggeeCookie>    mDebuggeeCookie = nullptr;

    std::timed_mutex                    mContextInfoListLock;
    ContextInfoListT                    mContextInfoList;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////    Instantiation.
                    PipelineContextManager();
                    ~PipelineContextManager();

protected:  ////    Operations.
    auto            print(
                        android::Printer& printer,
                        const std::vector<std::string>& options,
                        int32_t level = 0
                    ) -> void;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IPipelineContextManager Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////    Interfaces.
    virtual auto    create(char const* name) -> android::sp<IPipelineContext>;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IDebuggee Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////    Interfaces.
    virtual auto    debuggeeName() const -> std::string { return mDebuggeeName; }
    virtual auto    debug(
                        android::Printer& printer,
                        const std::vector<std::string>& options
                    ) -> void;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////    Interfaces.
    static auto     get() -> std::shared_ptr<PipelineContextManager>;
    auto            attach(android::sp<PipelineContext> pPipelineContext) -> void;
    auto            detach(PipelineContext const* pPipelineContext) -> void;

};
}
const std::string PipelineContextManager::mDebuggeeName{"NSCam::v3::NSPipelineContext::IPipelineContextManager"};


/******************************************************************************
 *
 ******************************************************************************/
auto
IPipelineContextManager::get() -> std::shared_ptr<IPipelineContextManager>
{
    return PipelineContextManager::get();
}


/******************************************************************************
 *
 ******************************************************************************/
auto
PipelineContextManager::get() -> std::shared_ptr<PipelineContextManager>
{
    static auto singleton = [](){
        //Make sure IDebuggeeManager singleton is fully constructed before this singleton.
        //So that it's safe to access IDebuggeeManager instance from this singleton's destructor.
        auto pDbgMgr = IDebuggeeManager::get();
        auto inst = std::make_shared<PipelineContextManager>();
        MY_LOGF_IF(inst == nullptr, "Fail on std::make_shared<PipelineContextManager>()");
        if (CC_LIKELY( pDbgMgr != nullptr )) {
            inst->mDebuggeeCookie = pDbgMgr->attach(inst, 0);
        }
        return inst;
    }();
    return singleton;
}


/******************************************************************************
 *
 ******************************************************************************/
PipelineContextManager::
PipelineContextManager()
{
}


/******************************************************************************
 *
 ******************************************************************************/
PipelineContextManager::
~PipelineContextManager()
{
    MY_LOGI("+ mDebuggeeCookie:%p", mDebuggeeCookie.get());
    if (auto pDbgMgr = IDebuggeeManager::get()) {
        pDbgMgr->detach(mDebuggeeCookie);
    }
    mDebuggeeCookie = nullptr;
    MY_LOGI("-");
}


/******************************************************************************
 *
 ******************************************************************************/
auto
PipelineContextManager::
create(char const* name) -> android::sp<IPipelineContext>
{
    android::sp<PipelineContext> pPipelineContext = new PipelineContext(name);
    attach(pPipelineContext);
    return pPipelineContext;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
PipelineContextManager::
attach(android::sp<PipelineContext> pPipelineContext) -> void
{
    auto pContextInfo = std::make_shared<ContextInfo>();
    if (CC_UNLIKELY( pContextInfo == nullptr )) {
        MY_LOGW("Fail on make_shared<ContextInfo>() - %s (%p)",
            pPipelineContext->getName(), pPipelineContext.get());
        return;
    }

    pContextInfo->pPipelineContext = pPipelineContext;
    pContextInfo->name = pPipelineContext->getName();
    if ( auto pLogTool = NSCam::Utils::LogTool::get() ) {
        pLogTool->getCurrentLogTime(&pContextInfo->timestampCreated);
    }

    {
        std::lock_guard<std::timed_mutex> _l(mContextInfoListLock);
        mContextInfoList.push_back(pContextInfo);
    }
}


/******************************************************************************
 *
 ******************************************************************************/
auto
PipelineContextManager::
detach(PipelineContext const* pPipelineContext) -> void
{
    bool needDump = false;
    {
        std::lock_guard<std::timed_mutex> _l(mContextInfoListLock);
        for (auto it = mContextInfoList.cbegin(); it != mContextInfoList.cend(); it++) {
            if ( pPipelineContext == (*it)->pPipelineContext.unsafe_get() ) {
                mContextInfoList.erase(it);
                needDump = ! mContextInfoList.empty(); // need dump if not empty
                break;
            }
        }
    }
    if (CC_UNLIKELY( needDump )) {
        ULogPrinter logPrinter(__ULOG_MODULE_ID, LOG_TAG, DetailsType::DETAILS_WARNING, "[detach] ");
        print(logPrinter, {});
    }
}


/******************************************************************************
 *
 ******************************************************************************/
auto
PipelineContextManager::
debug(
    android::Printer& printer,
    const std::vector<std::string>& options
) -> void
{
    print(printer, options, 2);
}


/******************************************************************************
 *
 ******************************************************************************/
auto
PipelineContextManager::
print(
    android::Printer& printer,
    const std::vector<std::string>& options,
    int32_t level
) -> void
{
    ContextInfoListT list;
    if ( mContextInfoListLock.try_lock_for(std::chrono::milliseconds(100)) ) {
        list = mContextInfoList;
        mContextInfoListLock.unlock();
    }

    if ( ! list.empty() ) {

        auto pLogTool = NSCam::Utils::LogTool::get();
        MY_LOGE_IF(pLogTool==nullptr, "NSCam::Utils::LogTool::get()");

        for (auto const& v : list) {
            android::String8 os;
            os += pLogTool->convertToFormattedLogTime(&v->timestampCreated).c_str();
            os += " ";
            auto pPipelineContext = v->pPipelineContext.promote();
            if ( pPipelineContext != nullptr ) {
                os += android::String8::format("#strong:%d-1", pPipelineContext->getStrongCount());
            }
            else {
                os += "not exist";
            }
            os += android::String8::format(" <%s>", v->name.c_str());
            os += android::String8::format(" (%p)", v->pPipelineContext.unsafe_get());
            printer.printLine("");
            printer.printLine(os.c_str());
        }

        if ( level >= 2 ) {
            printer.printLine("");
            for (auto const& v : list) {
                auto pPipelineContext = v->pPipelineContext.promote();
                if ( pPipelineContext != nullptr ) {
                    pPipelineContext->dumpState(printer, options);
                }
            }
        }

    }
}


/******************************************************************************
 *
 ******************************************************************************/
auto
PipelineContext::
magicName() -> std::string
{
    return std::string("PPLCTX");//PipelineContext
}


/******************************************************************************
 *
 ******************************************************************************/
auto
PipelineContext::
cast(IPipelineContext* pContext) -> PipelineContext*
{
    if (CC_UNLIKELY( pContext == nullptr )) {
        MY_LOGE("pContext == nullptr");
        return nullptr;
    }

    if (CC_UNLIKELY( magicName() != pContext->getNativeName() )) {
        MY_LOGE("the given native name:\"%s\" != magic name:\"%s\"",
            pContext->getNativeName().c_str(), magicName().c_str());
        return nullptr;
    }

    return reinterpret_cast<PipelineContext*>(pContext->getNativeHandle());
}


/******************************************************************************
 *
 ******************************************************************************/
PipelineContext::
PipelineContext(char const* name)
    : mpImpl( new PipelineContextImpl(name) )
{
    MY_LOGI("<%s> (%p)", name, this);
}


/******************************************************************************
 *
 ******************************************************************************/
PipelineContext::
~PipelineContext()
{
}


/******************************************************************************
 *
 ******************************************************************************/
void
PipelineContext::
onLastStrongRef(const void* /*id*/)
{
    String8 name(mpImpl->getName());
    MY_LOGI("+ <%s> (%p)", name.c_str(), this);
    PipelineContextManager::get()->detach(this);
    mpImpl = nullptr;
    MY_LOGI("- <%s>", name.c_str());
}


/******************************************************************************
 *
 ******************************************************************************/
char const*
PipelineContext::
getName() const
{
    return mpImpl->getName();
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
PipelineContext::
beginConfigure(
    android::sp<IPipelineContext> oldContext
)
{
    FUNC_START;
    // if old == current
    if( oldContext.get() == this ) {
        MY_LOGW("context: old == this");
        return INVALID_OPERATION;
    }
    waitUntilDrained();
    //
    // if( oldContext.get() ) {
    //     String8 name(getName());
    //     MY_LOGD( "context: old(%s:%p), current(%s:%p)",
    //              oldContext->getName(), oldContext.get(),
    //              getName(), this );
    //     //
    //     oldContext->waitUntilDrained();
    // }
    //
    if ( oldContext == nullptr ) {
        mpOldContext = nullptr;
    }
    else {
        mpOldContext = static_cast<PipelineContext*>(oldContext.get());
    }

    FUNC_END;
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
PipelineContext::
endConfigure(MBOOL parallelConfig, MBOOL isMultiThread)
{
    MERROR err;
    //
    if(1)
    {
        CAM_TRACE_NAME("endConfigure dump");
        dump();
    }
    //
    if( OK != (err = getImpl()->config(mpOldContext.get() ? mpOldContext->getImpl() : NULL, parallelConfig, isMultiThread)) )
        return err;
    // release old context
    mpOldContext = NULL;
    //
    // TODO: add status check!
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
PipelineContext::
queue(
    android::sp<IPipelineFrame>const& frame
) -> int
{
    return getImpl()->submitFrame({frame});
}


/******************************************************************************
 *
 ******************************************************************************/
auto
PipelineContext::
queue(
    std::vector<android::sp<IPipelineFrame>>const& frames
) -> int
{
    return getImpl()->submitFrame(frames);
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
PipelineContext::
kick(
    android::sp<IPipelineFrame> const &pFrame
)
{
    return getImpl()->kick(pFrame);
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
PipelineContext::
beginFlush()
{
    String8 name(mpImpl->getName());
    MY_LOGI("+ <%s>", name.c_str());
    //
    getImpl()->beginFlush();
    //
    MY_LOGI("- <%s>", name.c_str());
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
PipelineContext::
endFlush()
{
    String8 name(mpImpl->getName());
    MY_LOGI("+ <%s>", name.c_str());
    //
    getImpl()->waitUntilDrained();
    //
    getImpl()->endFlush();
    //
    MY_LOGI("- <%s>", name.c_str());
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
PipelineContext::
waitUntilDrained()
{
    FUNC_START;
    //
    getImpl()->waitUntilDrained();
    //
    FUNC_END;
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
PipelineContext::
waitUntilRootNodeDrained() -> int
{
    return getImpl()->waitUntilRootNodeDrained();
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
PipelineContext::
waitUntilNodeDrained(NodeId_T const nodeId)
{
    FUNC_START;
    //
    getImpl()->waitUntilNodeDrained(nodeId);
    //
    FUNC_END;
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
PipelineContext::
setDataCallback(
    android::wp<IDataCallback> pCallback
)
{
    return getImpl()->setDataCallback(pCallback);
}


/******************************************************************************
 *
 ******************************************************************************/
auto
PipelineContext::
setInitFrameCount(uint32_t count) -> void
{
    return getImpl()->setInitFrameCount(count);
}


/******************************************************************************
 *
 ******************************************************************************/
sp<INodeActor>
PipelineContext::
queryINodeActor(NodeId_T const nodeId) const
{
    return getImpl()->queryNode(nodeId);
}


/******************************************************************************
 *
 ******************************************************************************/
auto
PipelineContext::
dumpState(
    android::Printer& printer __unused,
    const std::vector<std::string>& options __unused
) -> void
{
    printer.printFormatLine("<%s> (%p)", getName(), this);
    mpImpl->dumpState(printer, options);
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
PipelineContext::
dump()
{
    ULogPrinter logPrinter(__ULOG_MODULE_ID, LOG_TAG, DetailsType::DETAILS_INFO, "[dump] ");
    mpImpl->dumpState(logPrinter, {});
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
PipelineContext::
setMultiCamSyncHelper(
    sp<MultiCamSyncHelperT>& helper
)
{
    MY_LOGD("set sync helper (%p)", helper.get());
    mpSyncHelper = helper;
}

/******************************************************************************
 *
 ******************************************************************************/
auto
PipelineContext::
getMultiCamSyncHelper(
) -> sp<MultiCamSyncHelperT>
{
    return mpSyncHelper;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
PipelineContext::
setSessionLifeHolder(
    std::shared_ptr<ISessionLifeHolder> const& mgr
) -> void
{
    mpSessionLifeHolder = mgr;
}

/******************************************************************************
 *
 ******************************************************************************/
auto
PipelineContext::
getSessionLifeHolder(
) const -> std::shared_ptr<ISessionLifeHolder>
{
    return mpSessionLifeHolder;
}

/******************************************************************************
 *
 ******************************************************************************/
auto
PipelineContext::
getHistoryBufferProvider() const -> std::shared_ptr<IHistoryBufferProvider>
{
    return getImpl()->getFrameBufferManager();
}


/******************************************************************************
 *
 ******************************************************************************/
PipelineContext::PipelineContextImpl*
PipelineContext::
getImpl() const
{
    return mpImpl.get();
}

