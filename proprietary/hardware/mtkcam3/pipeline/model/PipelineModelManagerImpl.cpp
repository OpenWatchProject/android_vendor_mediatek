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

#include <mtkcam3/pipeline/model/IPipelineModelManager.h>

#include <mtkcam/utils/debug/debug.h>

#include "PipelineModelImpl.h"
#include "EventLog.h"
#include <mtkcam/utils/std/ULog.h>
#include "MyUtils.h"

#include <map>

CAM_ULOG_DECLARE_MODULE_ID(MOD_PIPELINE_MODEL);

/******************************************************************************
 *
 ******************************************************************************/
using namespace android;
using namespace NSCam;
using namespace NSCam::v3::pipeline::model;
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
class PipelineModelManagerImpl
    : public IPipelineModelManager
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Definitions.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////
    struct  MyDebuggee : public IDebuggee
    {
        static const std::string            mName;
        std::shared_ptr<IDebuggeeCookie>    mCookie = nullptr;
        android::wp<PipelineModelManagerImpl>mContext = nullptr;

                        MyDebuggee(PipelineModelManagerImpl* p) : mContext(p) {}
        virtual auto    debuggeeName() const -> std::string { return mName; }
        virtual auto    debug(
                            android::Printer& printer,
                            const std::vector<std::string>& options
                        ) -> void override;
    };

    struct  PipelineModelInfo
    {
        android::sp<PipelineModelImpl>      mPipelineModel;
        std::shared_ptr<EventLogPrinter>    mErrorState;
        std::shared_ptr<EventLogPrinter>    mWarningState;
        std::shared_ptr<EventLogPrinter>    mDebugState;
    };

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Data Members.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////
    std::shared_ptr<MyDebuggee>             mDebuggee = nullptr;
    mutable std::map<int32_t, std::shared_ptr<PipelineModelInfo>>
                                            mPipelineMap;
    mutable android::RWLock                 mPipelineLock;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////    Instantiation.
                    PipelineModelManagerImpl(IDebuggeeManager* pDbgMgr);
                    ~PipelineModelManagerImpl();

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IPipelineModelManager Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////    Interfaces.

    virtual auto    getPipelineModel(
                        int32_t const openId
                        ) const -> android::sp<IPipelineModel> override;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IDebuggee Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////    Interfaces.

    virtual auto    debug(
                        android::Printer& printer __unused,
                        const std::vector<std::string>& options __unused
                        ) -> void;

};


/******************************************************************************
 *
 ******************************************************************************/
auto
IPipelineModelManager::get() -> android::sp<IPipelineModelManager>
{
    //Make sure IDebuggeeManager singleton is fully constructed before this singleton.
    //So that it's safe to access IDebuggeeManager instance from this singleton's destructor.
    static auto pDbgMgr = IDebuggeeManager::get();
    static android::sp<PipelineModelManagerImpl> singleton = new PipelineModelManagerImpl(pDbgMgr);
    return singleton;
}


/******************************************************************************
 *
 ******************************************************************************/
PipelineModelManagerImpl::
PipelineModelManagerImpl(IDebuggeeManager* pDbgMgr)
{
    mDebuggee = std::make_shared<MyDebuggee>(this);
    if ( pDbgMgr ) {
        mDebuggee->mCookie = pDbgMgr->attach(mDebuggee, 1);
    }
}


/******************************************************************************
 *
 ******************************************************************************/
PipelineModelManagerImpl::
~PipelineModelManagerImpl()
{
    MY_LOGD("+ mDebuggee:%p", mDebuggee.get());
    if ( mDebuggee != nullptr ) {
        if ( auto pDbgMgr = IDebuggeeManager::get() ) {
            pDbgMgr->detach(mDebuggee->mCookie);
        }
        mDebuggee = nullptr;
    }
    MY_LOGD("-");
}


/******************************************************************************
 *
 ******************************************************************************/
auto
PipelineModelManagerImpl::
getPipelineModel(
    int32_t const openId __unused
) const -> android::sp<IPipelineModel>
{
    android::RWLock::AutoWLock _l(mPipelineLock);

    auto it = mPipelineMap.find(openId);
    if ( CC_LIKELY(it != mPipelineMap.end()) ) {
        return it->second->mPipelineModel;
    }


    // first time.
    auto pPipelineModelInfo = mPipelineMap[openId] = std::make_shared<PipelineModelInfo>();
    if  ( CC_UNLIKELY(pPipelineModelInfo==nullptr) ) {
        MY_LOGE("openId:%d: Fail on std::make_shared<PipelineModelInfo>", openId);
        return nullptr;
    }
    {
        auto errorPrinter   = pPipelineModelInfo->mErrorState   = std::make_shared<EventLogPrinter>(15, 25);
        auto warningPrinter = pPipelineModelInfo->mWarningState = std::make_shared<EventLogPrinter>(25, 15);
        auto debugPrinter   = pPipelineModelInfo->mDebugState   = std::make_shared<EventLogPrinter>();
        if ( CC_UNLIKELY(errorPrinter==nullptr || warningPrinter==nullptr || debugPrinter==nullptr) ) {
            MY_LOGE("openId:%d: Fail on std::make_shared<EventLogPrinter>", openId);
            return nullptr;
        }

        auto pPipelineModel = pPipelineModelInfo->mPipelineModel =
                                PipelineModelImpl::createInstance(
                                    PipelineModelImpl::CreationParams{
                                        .openId         = openId,
                                        .errorPrinter   = errorPrinter,
                                        .warningPrinter = warningPrinter,
                                        .debugPrinter   = debugPrinter,
                                });
        if ( CC_UNLIKELY(pPipelineModel==nullptr) ) {
            MY_LOGE("openId:%d: Fail on PipelineModelImpl::createInstance", openId);
            return nullptr;
        }
    }
    return pPipelineModelInfo->mPipelineModel;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
PipelineModelManagerImpl::
debug(
    android::Printer& printer __unused,
    const std::vector<std::string>& options __unused
) -> void
{
    android::RWLock::AutoRLock _l(mPipelineLock);

    printer.printFormatLine("## IPipelineModelManager: has # %zu IPipelineModel\n", mPipelineMap.size());

    for (auto const& v : mPipelineMap) {

        printer.printFormatLine("## IPipelineModel [%02x]", v.first);

        auto pInfo = v.second;

        pInfo->mPipelineModel->dumpState(printer, options);

        printer.printLine("\n== error state (most recent at bottom) ==");
        pInfo->mErrorState->print(printer);

        printer.printLine("\n== warning state (most recent at bottom) ==");
        pInfo->mWarningState->print(printer);

        printer.printLine("\n== debug state (most recent at bottom) ==");
        pInfo->mDebugState->print(printer);

        printer.printLine("\n");
    }
}


/******************************************************************************
 *
 ******************************************************************************/
const std::string PipelineModelManagerImpl::MyDebuggee::mName{"NSCam::v3::pipeline::model::IPipelineModelManager"};
auto PipelineModelManagerImpl::MyDebuggee::debug(android::Printer& printer, const std::vector<std::string>& options) -> void
{
    auto p = mContext.promote();
    if ( CC_LIKELY(p != nullptr) ) {
        p->debug(printer, options);
    }
}

