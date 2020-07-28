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

#ifndef _MTK_PLATFORM_HARDWARE_INCLUDE_MTKCAM_V3_HWNODE_P2NODE_PROCESSOR_H_
#define _MTK_PLATFORM_HARDWARE_INCLUDE_MTKCAM_V3_HWNODE_P2NODE_PROCESSOR_H_


#include "P2Common.h"
#include "FrameUtils.h"
#if SUPPORT_3RD_PARTY
#include <mtkcam/pipeline/extension/IVendorManager.h>
#include <map>
#endif


#define PROC_TAG(str) "[%s] " str, mName.string()
using NSCam::MERROR;
class Request;

class Processor : virtual public android::RefBase {
public:
    virtual MERROR queueRequest(sp<Request>) = 0;

    virtual MERROR handleRequest(sp<Request>) = 0;

    virtual MVOID flushRequests() = 0;

    virtual MVOID notify(MUINT32 uEvent, MINTPTR iArg1 = 0,
                         MINTPTR iArg2 = 0, MINTPTR iArg3 = 0) = 0;

    virtual MVOID waitForIdle() = 0;

    virtual MVOID setNextProcessor(wp<Processor>) = 0;

    virtual MBOOL isAsyncProcessor() = 0;

    virtual MVOID callbackAsync(sp<Request>, MERROR) = 0;

    virtual MBOOL close() = 0;
};

class Request : public VirtualLightRefBase {
public:
    struct Context {
        Context()
                : is_started(MFALSE),
                  burst_num(0),
                  enable_venc_stream(MFALSE),
                  resized(0),
                  fd_stream_id(0),
                  capture_stream_id(0),
                  captureFrame(false),
                  in_lcso_buffer(NULL),
                  crop_info(NULL),
                  in_mdp_buffer(NULL),
                  in_mdp_crop(MFALSE),
                  in_app_meta(NULL),
                  in_hal_meta(NULL),
                  out_app_meta(NULL),
                  out_hal_meta(NULL),
                  iso(0),
                  run_plugin_raw(MFALSE),
                  run_plugin_yuv(MFALSE),
                  is_yuv_reproc(MFALSE),
                  work_buffer_format(0),
                  work_buffer(NULL),
                  customOption(0),
                  uPass2Count(0),
                  is2run(MFALSE),
                  downscaleRatio(2),
                  downscaleThres(-1),
                  downscaleWidth(-1),
                  downscaleHeight(-1),
                  swnr_thres_temp(-1)
                  #if SUPPORT_3RD_PARTY
                  ,processorMask(0LL)
                  ,userId(0ULL)
                  ,mbSkipNextProcessor(MFALSE)
                  ,mbSkipAllNextProcessor(MFALSE)
                  #endif
        { };

        MBOOL is_started;
        MUINT8 burst_num;
        MBOOL enable_venc_stream;
        MBOOL resized;
        StreamId_T fd_stream_id;
        StreamId_T capture_stream_id;
        MBOOL captureFrame;
        vector<sp<BufferHandle>> in_buffer;
        sp<BufferHandle> in_lcso_buffer;
        sp<Cropper::CropInfo> crop_info;
        vector<sp<BufferHandle>> out_buffers;
        sp<BufferHandle> in_mdp_buffer;
        MBOOL in_mdp_crop;
        sp<MetaHandle> in_app_meta;
        sp<MetaHandle> in_app_ret_meta;
        sp<MetaHandle> in_hal_meta;
        sp<MetaHandle> out_app_meta;
        sp<MetaHandle> out_hal_meta;
        MINT32 iso;
        MBOOL run_plugin_raw;
        MBOOL run_plugin_yuv;
        MBOOL is_yuv_reproc;
        MUINT32 work_buffer_format;
        sp<BufferHandle> work_buffer;
        MINT32 customOption;
        MUINT32 uPass2Count;
        MBOOL is2run;
        MINT32 downscaleRatio;
        MINT32 downscaleThres;
        MINT32 downscaleWidth;
        MINT32 downscaleHeight;
        MINT32 swnr_thres_temp;
#if SUPPORT_3RD_PARTY
        Vector<NSCam::plugin::BufferItem> vWork_buffer;
        MINT64 processorMask;
        MUINT64 userId;
        MBOOL mbSkipNextProcessor;
        MBOOL mbSkipAllNextProcessor;
#endif

    };

    struct Context context;

    typedef enum {
        NR_TYPE_NONE = 0,
        NR_TYPE_MNR,
        NR_TYPE_SWNR
    } NR_TYPE_T;

    Request(sp<FrameLifeHolder> pFrameLifeHolder,
            MUINT32 uUniqueKey,
            MUINT32 mRequestNo,
            MUINT32 uFrameNo,
            MUINT32 uFrameSubNo)
            : mpFrameLifeHolder(pFrameLifeHolder),
              muUniqueKey(uUniqueKey),
              muRequestNo(mRequestNo),
              muFrameNo(uFrameNo),
              muFrameSubNo(uFrameSubNo),
              mbReentry(MFALSE) {
    }

    MUINT32 getUniqueKey() { return muUniqueKey; };

    MUINT32 getRequestNo() { return muRequestNo; };

    MUINT32 getFrameNo() { return muFrameNo; };

    MUINT32 getFrameSubNo() { return muFrameSubNo; };

    MBOOL isReentry() { return mbReentry; };

    MVOID setReentry(MBOOL bReentry) { mbReentry = bReentry; };

    MBOOL isFinished() {
        vector<sp<BufferHandle >>::iterator
                iter = context.out_buffers.begin();
        while (iter != context.out_buffers.end()) {
            if ((*iter).get()) {
                MY_LOGD_IF(1,"remain streamId: %#" PRIx64 "", (*iter)->getStreamId());
                return MFALSE;
            }
            iter++;
        }
        MY_LOGD_IF(P2_DEBUG_LOG,"there no buffer need processing...");
        return MTRUE;
    }

    MVOID setCurrentOwner(wp<Processor> pProcessor) {
        mwpProcessor = pProcessor;
    }

#if SUPPORT_3RD_PARTY
    MVOID setVendorMgr(sp<NSCam::plugin::IVendorManager> pVendorMgr) {
        mpVendor = pVendorMgr;
    }

    sp<NSCam::plugin::IVendorManager> getVendorMgr() {
        return mpVendor;
    };

#endif
    MVOID responseDone(MERROR status) {
        sp<Processor> spProcessor = mwpProcessor.promote();
        if (spProcessor.get()) {
            spProcessor->callbackAsync(this, status);
        }
    }

    MVOID onPartialRequestDone() {
        if (mpFrameLifeHolder.get())
            mpFrameLifeHolder->onPartialFrameDone();
    }

    ~Request() {
        MY_LOGD_IF(P2_DEBUG_LOG, "frame[%d]/request[%d]/subNo[%d]", muFrameNo, muRequestNo, muFrameSubNo);

        MBOOL isAllReleased = MTRUE;
        String8 strLog;

#define LOG_UNRELEASED(name) \
    if (!isAllReleased)      \
        strLog += ", ";      \
    strLog += name;          \
    isAllReleased = MFALSE;

        {
            vector<sp<BufferHandle >>::iterator iter = context.in_buffer.begin();
            while (iter != context.in_buffer.end()) {
                if ((*iter).get()) {
                    LOG_UNRELEASED(String8::format("in buffer[%#" PRIxPTR "]", (*iter)->getStreamId()));
                    (*iter).clear();
                }
                iter++;
            }
        }
        if(context.in_lcso_buffer.get())
        {
            context.in_lcso_buffer.clear();
            MY_LOGW("context.in_lcso_buffer not released");
        }
        if (context.in_mdp_buffer.get()) {
            context.in_mdp_buffer.clear();
            LOG_UNRELEASED("mdp buffer");
        }

#if SUPPORT_MNR || SUPPORT_SWNR || SUPPORT_PLUGIN
        if (context.work_buffer.get()) {
            context.work_buffer.clear();
            LOG_UNRELEASED("work buffer");
        }
#endif
        MBOOL isFail = false;
        {
            vector<sp<BufferHandle >>::iterator iter = context.out_buffers.begin();
            while (iter != context.out_buffers.end()) {
                if ((*iter).get()) {
                    if((*iter)->getState() == BufferHandle::STATE_NOT_USED || (*iter)->getState() == BufferHandle::STATE_WRITE_FAIL )
                        isFail = true;
                    LOG_UNRELEASED(String8::format("out buffer[%#" PRIx64 "]", (*iter)->getStreamId()));
                    (*iter).clear();
                }
                iter++;
            }
        }
        if (context.in_app_meta.get()) {
            context.in_app_meta.clear();
        }
        if (context.in_hal_meta.get()) {
            context.in_hal_meta.clear();
        }
        if (context.in_app_ret_meta.get()) {
            context.in_app_ret_meta.clear();
        }
        if (context.out_app_meta.get()) {
            context.out_app_meta->updateState(
                isFail ? MetaHandle::STATE_WRITE_FAIL : MetaHandle::STATE_WRITE_OK
                );
            context.out_app_meta.clear();
            LOG_UNRELEASED("out appmeta");
        }
        if (context.out_hal_meta.get()) {
            context.out_hal_meta.clear();
            LOG_UNRELEASED("out halmeta");
        }
#undef LOG_UNRELEASED
        if (!isAllReleased) {
            MY_LOGW("F[%d]/R[%d]/S[%d] not released: %s", muFrameNo, muRequestNo, muFrameSubNo, strLog.string());
        }
        onPartialRequestDone();
    }

private:
    sp<FrameLifeHolder> mpFrameLifeHolder;
#if SUPPORT_3RD_PARTY
    sp<NSCam::plugin::IVendorManager> mpVendor;
#endif
    wp<Processor> mwpProcessor;
    MUINT32 muUniqueKey;
    MUINT32 muRequestNo;
    MUINT32 muFrameNo;
    MUINT32 muFrameSubNo;
    MBOOL mbReentry;
};

#if SUPPORT_3RD_PARTY
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Multi-Request Handler
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
template<typename TFrameParams>
class FrameHandler
{
public:
                            FrameHandler(MBOOL bEableLog)
                                : mLock()
                                , mbEnableLog(bEableLog)
                            {
                                MY_LOGD_IF(mbEnableLog, "FrameHandler");
                            }

                            ~FrameHandler()
                            {
                                MY_LOGD_IF(mbEnableLog, "~FrameHandler");
                            };

inline  size_t              size() const
                            {
                                return mvCollecting.size();
                            }

typedef vector<pair< sp<Request>, TFrameParams >> vRequestFramePairT;
        MERROR              collect(MUINT32 const frameNo,
                                        sp<Request> const& pRequest,
                                        TFrameParams const& param)
                            {
                                Mutex::Autolock _l(mLock);
                                //ssize_t idx = mvCollecting.indexOfKey(frameNo) ;
                                if(mvCollecting.find(frameNo) == mvCollecting.end()) // not exit
                                {
                                    vRequestFramePairT pair;
                                    pair.push_back(make_pair(pRequest, param));
                                    mvCollecting[frameNo] = pair;
                                    MY_LOGD_IF(0,"collect frameNo:%d mvCollecting size: %zu",frameNo, mvCollecting.size());
                                }
                                else
                                {
                                    mvCollecting[frameNo].push_back(make_pair(pRequest, param));
                                }
                                //vRequestFramePairT vParam = mvCollecting.editValueFor(frameNo);
                                //vParam.push_back(make_pair(pRequest, param));
                                MY_LOGD("collect frameNo:%d, mvCollecting size: %zu, Request size: %zu",frameNo, mvCollecting.size(), mvCollecting[frameNo].size());
                                return OK;
                            }

        size_t              sizeOfRequest(MUINT32 const frameNo)
                            {
                                Mutex::Autolock _l(mLock);
                                //ssize_t idx = mvCollecting.indexOfKey(frameNo) ;
                                //MY_LOGD_IF(0,"mvCollecting.indexOfKey(%d) %d mvCollecting.size: %d",frameNo, idx, mvCollecting.size());
                                return mvCollecting[frameNo].size();
                            }

        /*size_t              sizeOfRequestByIndex(size_t idx)
                            {
                                Mutex::Autolock _l(mLock);
                                return mvCollecting.valueAt(idx).size();
                            }

        sp<Request>         getRequestByIndex(size_t i, size_t idx)
                            {
                                Mutex::Autolock _l(mLock);
                                return mvCollecting.valueAt(i).itemAt(idx).first; //sp<Request>
                            }*/
        sp<Request>         getRequest(MUINT32 const frameNo, size_t idx)
                            {
                                Mutex::Autolock _l(mLock);
                                return mvCollecting[frameNo].at(idx).first; //sp<Request>
                            }

        TFrameParams        getFrameParam(MUINT32 const frameNo, size_t idx)
                            {
                                Mutex::Autolock _l(mLock);
                                //MY_LOGD_IF(0,"getFrameParam frameNo:%d idx: %d size(%zu)",frameNo,  mvCollecting.indexOfKey(frameNo),  mvCollecting.valueFor(frameNo).size());
                                return mvCollecting[frameNo].at(idx).second; //sp<Request>
                            }

        TFrameParams&        getFrameParamAndEdit(MUINT32 const frameNo, size_t idx)
                            {
                                Mutex::Autolock _l(mLock);
                                //MY_LOGD_IF(0,"getFrameParam frameNo:%d idx: %d size(%zu)",frameNo,  mvCollecting.indexOfKey(frameNo),  mvCollecting.valueFor(frameNo).size());
                                return mvCollecting[frameNo].at(idx).second; //sp<Request>
                            }

        MERROR              remove(MUINT32 const frameNo)
                            {
                                Mutex::Autolock _l(mLock);
                                mvCollecting.erase(frameNo);
                                MY_LOGD_IF(1,"[FrameHandler] remove frameNo: %d",frameNo);
                                return OK;
                            }
        MVOID               dump()
                            {
                                for (auto const &iter: mvCollecting)
                                {
                                    MY_LOGI("Dump mvCollecting frameNo: %d", iter.first);
                                    for(size_t i = 0;i< iter.second.size(); i++)
                                    {
                                        MY_LOGI("requestNo: %d frameNo:%d subNo: %d", iter.second[i].first->getRequestNo(),
                                            iter.second[i].first->getFrameNo(),
                                            iter.second[i].first->getFrameSubNo());
                                    }
                                }
                            }
//-----------------------------------
//| frameNo |  Request , FrameParam |
//|----------------------------------
private:
    mutable Mutex mLock;
    MBOOL const mbEnableLog;
    std::map< MUINT32, vRequestFramePairT > mvCollecting; //<frameNo, mvRequestFramePair>

};
#endif

template<typename TProcedure>
struct ProcedureTraits {
    typedef typename TProcedure::CreateParams TCreateParams;
    typedef typename TProcedure::FrameParams TProcParams;

    typedef MERROR (TProcedure::*TOnPullParams)(sp<Request>, TProcParams &);

    typedef MERROR (TProcedure::*TOnExecute)(sp<Request>, TProcParams const &);

    typedef MERROR (TProcedure::*TOnFinish)(TProcParams const &, MBOOL const);

    typedef MVOID  (TProcedure::*TOnFlush)();

    typedef MVOID  (TProcedure::*TOnNotify)(MUINT, MINTPTR, MINTPTR, MINTPTR);

    static constexpr TOnPullParams fnOnPullParams = &TProcedure::onPullParams;
    static constexpr TOnExecute fnOnExecute = &TProcedure::onExecute;
    static constexpr TOnFinish fnOnFinish = &TProcedure::onFinish;
    static constexpr TOnFlush fnOnFlush = &TProcedure::onFlush;
    static constexpr TOnNotify fnOnNotify = &TProcedure::onNotify;

    static constexpr MBOOL isAsync = TProcedure::isAsync;
};


template<typename TProcedure, typename TProcedureTraits = ProcedureTraits<TProcedure>>
class ProcessorBase : public Processor, public Thread {
    typedef typename TProcedureTraits::TCreateParams TCreateParams;
    typedef typename TProcedureTraits::TProcParams TProcParams;

    typedef typename TProcedureTraits::TOnPullParams TOnPullParams;
    typedef typename TProcedureTraits::TOnExecute TOnExecute;
    typedef typename TProcedureTraits::TOnFinish TOnFinish;
    typedef typename TProcedureTraits::TOnFlush TOnFlush;
    typedef typename TProcedureTraits::TOnNotify TOnNotify;

protected:
    sp<TProcedure> mspProcedure;
    TProcedure *mpProcedure;

public:
    ProcessorBase(MUINT32 uOpenId,
                  TCreateParams const &createParams,
                  char const *name)
            : Thread(false),
              mbRequestDrained(MTRUE),
              mbExit(MFALSE),
              mName(name),
              muOpenId(uOpenId)
    {
        mspProcedure = new TProcedure(createParams);
        mpProcedure = mspProcedure.get();

        char threadname[10];
        sprintf(threadname, "Cam@%s", name);
        muSyncframe = ::property_get_int32("debug.camera.dump.p2.syncframe", 0);
        run(threadname);
    }

    virtual MBOOL close() {
        MY_LOGDO_IF(P2_DEBUG_LOG, PROC_TAG("close processor"));

        requestExit();
        join();
        waitForIdle();
        return MTRUE;
    }

    ~ProcessorBase() {
        MY_LOGDO_IF(P2_DEBUG_LOG, PROC_TAG("destroy processor"));
    }

    virtual MERROR queueRequest(sp<Request> pRequest) {
        Mutex::Autolock _l(mRequestLock);
        if (pRequest->context.is_started && pRequest->isFinished())
            return OK;
#if SUPPORT_3RD_PARTY
        if(pRequest->context.mbSkipNextProcessor)
        {
            MY_LOGDO_IF(P2_DEBUG_LOG, "frame[%d-%02d]/request[%d] skip , queue next processor",
                      pRequest->getFrameNo(), pRequest->getFrameSubNo(), pRequest->getRequestNo());
            pRequest->context.mbSkipNextProcessor = MFALSE;
            sp<Processor> pNextProcessor = mwpNextProcessor.promote();
            if(pNextProcessor.get())
                pNextProcessor->queueRequest(pRequest);
            return OK;
        }
        if( pRequest->context.is_started && pRequest->context.mbSkipAllNextProcessor ) return OK;
#endif
        pRequest->context.is_started = MTRUE;
        //
        pRequest->setCurrentOwner(this);
        // Make sure the request with a smaller frame number has a higher priority.
        vector<sp<Request >>::iterator it = mvPending.end();
        for (; it != mvPending.begin();) {
            --it;
            if (0 <= (MINT32)(pRequest->getFrameNo() - (*it)->getFrameNo())) {
                ++it; //insert(): insert before the current node
                break;
            }
        }
        mvPending.insert(it, pRequest);
        MY_LOGDO_IF(mvPending.size() > 1, PROC_TAG("frame[%d-%02d]/request[%d] queued, pending:%zu, running:%zu"),
                   pRequest->getFrameNo(), pRequest->getFrameSubNo(), pRequest->getRequestNo(),
                   mvPending.size(), mvRunning.size());

        mRequestQueueCond.signal();
        return OK;
    }


    virtual MERROR handleRequest(sp<Request> pRequest) {
        TProcParams params;
        TOnPullParams fnOnPullParams = TProcedureTraits::fnOnPullParams;
        TOnExecute fnOnExecute = TProcedureTraits::fnOnExecute;
        TOnFinish fnOnFinish = TProcedureTraits::fnOnFinish;
        // reset reentry flag
        pRequest->setReentry(MFALSE);

        MERROR ret;
        if (OK == (ret = (mpProcedure->*fnOnPullParams)(pRequest.get(), params))) {
            MERROR ret = (mpProcedure->*fnOnExecute)(pRequest, params);
            if (isAsyncProcessor() && ret == OK) {
                // do aync processing
                Mutex::Autolock _l(mCallbackLock);
                mvRunning.push_back(make_pair(pRequest, params));
                MY_LOGDO_IF(P2_DEBUG_LOG, PROC_TAG("frame[%d-%02d]/request[%d] executed, pending:%zu, running:%zu"),
                           pRequest->getFrameNo(), pRequest->getFrameSubNo(), pRequest->getRequestNo(),
                           mvPending.size(), mvRunning.size());
                if(muSyncframe) //for debug
                {
                    MY_LOGD("wait deque...");
                    mCallbackCond.wait(mCallbackLock);
                }
                else {
                    params = TProcParams();
                    pRequest = nullptr;
                    mCallbackCond.signal();
                }
            }
            else {
                (mpProcedure->*fnOnFinish)(params, ret == OK);
                // trigger to release buffer
                params = TProcParams();
                pRequest->onPartialRequestDone();

                sp<Processor> spProcessor = (pRequest->isReentry()) ? this : mwpNextProcessor.promote();
                if (spProcessor.get()) {
                    spProcessor->queueRequest(pRequest);
                }
            }
            return ret;
        }
        else if (ret == NOT_ENOUGH_DATA){
            sp<Processor> spProcessor = (pRequest->isReentry()) ? this : mwpNextProcessor.promote();
            if (spProcessor.get()) {
                spProcessor->queueRequest(pRequest);
            }
        }

        return OK;
    }

    virtual MVOID callbackAsync(sp<Request> pRequest, MERROR status) {
        if (isAsyncProcessor()) {
            Mutex::Autolock _l1(mAsyncLock);
            Mutex::Autolock _l2(mCallbackLock);
            MY_LOGDO_IF(P2_DEBUG_LOG, PROC_TAG("frame[%d-%02d]/request[%d] callbacked, pending:%zu, running:%zu"),
                       pRequest->getFrameNo(), pRequest->getFrameSubNo(), pRequest->getRequestNo(),
                       mvPending.size(), mvRunning.size());
            TOnFinish fnOnFinish = TProcedureTraits::fnOnFinish;

            MBOOL isFound = MFALSE;
            do {
                typename vector<pair<sp<Request>, TProcParams> >::iterator
                        iter = mvRunning.begin();
                for (; iter != mvRunning.end(); iter++) {
                    if ((*iter).first == pRequest) {
                        (mpProcedure->*fnOnFinish)((*iter).second, status == OK);
                        mvRunning.erase(iter);
                        isFound = MTRUE;
                        if(muSyncframe) //for debug
                        {
                            MY_LOGD("deque signal");
                            mCallbackCond.signal();
                        }
                        break;
                    }
                }
                if (!isFound) {
                    MY_LOGWO_IF(1, PROC_TAG("frame[%d-%02d]/request[%d] callback faster than execution finished"),
                               pRequest->getFrameNo(), pRequest->getFrameSubNo(), pRequest->getRequestNo());

                    #if 0 // July28,2017 - Rynn Wu <rynn.wu@mediatek.com>
                          // Unknown deadlock may happened here, for some reasons,
                          // we're not able to fix this problem, but invoke a
                          // null pointer dereference exception to avoid deadlock
                          // problem.

                    mCallbackCond.wait(mCallbackLock);
                    #else
                    const nsecs_t TIMEOUTNS = 3000000000LL; // nanosecond
                    auto _result = mCallbackCond.waitRelative(mCallbackLock, TIMEOUTNS);
                    if (__builtin_expect( _result != OK , false )) {
                        MY_LOGE("%s: mCallbackCond wait timed out(%" PRId64 "), err=%#x",
                                __FUNCTION__, TIMEOUTNS, _result);
                        *((volatile int*)(0x0)) = 0xDEADFEED; // invoke NE
                    }
                    #endif
                }
            } while (!isFound);

            MY_LOGDO_IF(status!=OK, PROC_TAG("request callback async, status:%d"), status);
            if (!pRequest->context.burst_num)
                pRequest->onPartialRequestDone();

            mAsyncCond.signal();
            sp<Processor> spProcessor = (pRequest->isReentry()) ? this : mwpNextProcessor.promote();
            if (spProcessor.get()) {
                spProcessor->queueRequest(pRequest);
            }
        }
        return;
    }

    virtual MVOID setNextProcessor(wp<Processor> pProcessor) {
        mwpNextProcessor = pProcessor;
    }

    virtual MBOOL isAsyncProcessor() {
        return TProcedureTraits::isAsync;
    }


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Thread Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    // Ask this object's thread to exit. This function is asynchronous, when the
    // function returns the thread might still be running. Of course, this
    // function can be called from a different thread.
    virtual void requestExit() {
        Mutex::Autolock _l(mRequestLock);
        mbExit = MTRUE;
        mRequestQueueCond.signal();
    }

    // Good place to do one-time initializations
    virtual status_t readyToRun() {
        //
        //  thread policy & priority
        //  Notes:
        //      Even if pthread_create() with SCHED_OTHER policy, a newly-created thread
        //      may inherit the non-SCHED_OTHER policy & priority of the thread creator.
        //      And thus, we must set the expected policy & priority after a thread creation.
        MINT tid;
        struct sched_param sched_p;
        ::sched_getparam(0, &sched_p);
        if (P2_THREAD_POLICY == SCHED_OTHER) {
            sched_p.sched_priority = 0;
            ::sched_setscheduler(0, P2_THREAD_POLICY, &sched_p);
            ::setpriority(PRIO_PROCESS, 0, P2_THREAD_PRIORITY);   //  Note: "priority" is nice value.
        } else {
            sched_p.sched_priority = P2_THREAD_PRIORITY;          //  Note: "priority" is real-time priority.
            ::sched_setscheduler(0, P2_THREAD_POLICY, &sched_p);
        }

        MY_LOGDO_IF(1, PROC_TAG("tid(%d) policy(%d) priority(%d)"), ::gettid(), P2_THREAD_POLICY, P2_THREAD_PRIORITY);

        return OK;

    }

private:

    // Derived class must implement threadLoop(). The thread starts its life
    // here. There are two ways of using the Thread object:
    // 1) loop: if threadLoop() returns true, it will be called again if
    //          requestExit() wasn't called.
    // 2) once: if threadLoop() returns false, the thread will exit upon return.
    virtual bool threadLoop() {
        while (!exitPending() && OK == onWaitRequest()) {

            sp<Request> pRequest = NULL;
            {
                Mutex::Autolock _l(mRequestLock);
                if (mvPending.size() == 0) {
                    MY_LOGWO_IF(1, PROC_TAG("no request"));
                    return true;
                }

                pRequest = mvPending.front();
                mvPending.erase(mvPending.begin());
            }

            MY_LOGEO_IF(handleRequest(pRequest) != OK, "request execute failed");

            return true;
        }
        MY_LOGDO_IF(P2_DEBUG_LOG, PROC_TAG("exit thread"));
        return false;
    }

public:
    virtual MVOID flushRequests() {
        FUNC_START;

        Mutex::Autolock _l(mRequestLock);

        mvPending.clear();

        if (!mbRequestDrained) {
            MY_LOGDO_IF(1, PROC_TAG("wait for request drained"));
            mRequestDrainedCond.wait(mRequestLock);
        }

        TOnFlush fnOnFlush = TProcedureTraits::fnOnFlush;
        (mpProcedure->*fnOnFlush)();

        FUNC_END;
        return;
    }

    virtual MVOID notify(MUINT32 uEvent, MINTPTR iArg1 = 0, MINTPTR iArg2 = 0, MINTPTR iArg3 = 0) {
        MY_LOGDO_IF(1, PROC_TAG("received event[%d] arg1[%#" PRIxPTR "] arg2[%#" PRIxPTR "] arg3[%#" PRIxPTR "]"),
                   uEvent, iArg1, iArg2, iArg3);
        TOnNotify fnOnNotify = TProcedureTraits::fnOnNotify;
        (mpProcedure->*fnOnNotify)(uEvent, iArg1, iArg2, iArg3);
        return;
    }

    virtual MVOID waitForIdle() {
        if (isAsyncProcessor())
            return;

        Mutex::Autolock _l(mAsyncLock);
        while (mvRunning.size()) {
            MY_LOGDO_IF(1, PROC_TAG("wait request done %zu"), mvRunning.size());
            mAsyncCond.wait(mAsyncLock);
        }

        return;
    }

    virtual MERROR onWaitRequest() {
        Mutex::Autolock _l(mRequestLock);
        while (!mvPending.size() && !mbExit) {
            // set drained flag
            mbRequestDrained = MTRUE;
            mRequestDrainedCond.signal();

            status_t status = mRequestQueueCond.wait(mRequestLock);
            if (OK != status) {
                MY_LOGWO_IF(1, PROC_TAG("wait status:%d:%s, request size:%zu, exit:%d"),
                           status, ::strerror(-status), mvPending.size(), mbExit
                );
            }
        }

        if (mbExit) {
            MY_LOGWO_IF(mvPending.size(), PROC_TAG("existed mvPending.size:%zu"), mvPending.size());
            return DEAD_OBJECT;
        }

        mbRequestDrained = MFALSE;
        return OK;
    }

    MUINT32 getOpenId() {return muOpenId;};

protected:
    vector<sp<Request>> mvPending;
    // for async request
    vector<pair<sp<Request>, TProcParams>> mvRunning;
    wp<Processor> mwpNextProcessor;
    mutable Mutex mRequestLock;
    mutable Condition mRequestQueueCond;
    mutable Condition mRequestDrainedCond;
    MBOOL mbRequestDrained;
    MBOOL mbExit;
    mutable Mutex mAsyncLock;
    mutable Condition mAsyncCond;
    mutable Mutex mCallbackLock;
    mutable Condition mCallbackCond;
    String8 mName;
    MUINT32 muOpenId;
    MUINT32 muSyncframe;
};


#define DECLARE_PROC_TRAIT(proc) \
template<> \
struct ProcedureTraits<proc##Procedure> { \
    typedef typename proc##Procedure::CreateParams  TCreateParams; \
    typedef typename proc##Procedure::FrameParams TProcParams; \
    typedef MERROR (proc##Procedure::*TOnPullParams)(sp<Request>, TProcParams&);  \
    typedef MERROR (proc##Procedure::*TOnExecute)(sp<Request>, TProcParams const&); \
    typedef MERROR (proc##Procedure::*TOnFinish)(TProcParams const&, MBOOL const); \
    typedef MVOID  (proc##Procedure::*TOnFlush)(); \
    typedef MVOID  (proc##Procedure::*TOnNotify)(MUINT32 const, MINTPTR const, MINTPTR const, MINTPTR const); \
    static constexpr TOnPullParams  fnOnPullParams  = &proc##Procedure::on##proc##PullParams;   \
    static constexpr TOnExecute     fnOnExecute     = &proc##Procedure::on##proc##Execute;      \
    static constexpr TOnFinish      fnOnFinish      = &proc##Procedure::on##proc##Finish;       \
    static constexpr TOnFlush       fnOnFlush       = &proc##Procedure::on##proc##Flush;        \
    static constexpr TOnNotify      fnOnNotify      = &proc##Procedure::on##proc##Notify;       \
    static constexpr MBOOL  isAsync                 = proc##Procedure::isAsync;                 \
};

#endif
