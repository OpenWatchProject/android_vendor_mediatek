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
 * MediaTek Inc. (C) 2016. All rights reserved.
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

#include <cstdlib>
#include <vector>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <memory>
#include <algorithm>
#include <map>
#include <random>
#include <thread>
#include <list>
#include <future>

#define LOG_TAG "ULogTest"
#include <mtkcam/utils/std/Log.h>
#include <mtkcam/utils/std/ULog.h>

CAM_ULOG_DECLARE_MODULE_ID(MOD_CAMERA_HAL_SERVER);

using namespace NSCam::Utils::ULog;


namespace NSCam {
namespace Utils {
namespace ULogTest {


class SleepRandom
{
private:
    std::random_device rd;
    std::mt19937 gen;
    std::uniform_int_distribution<int> dis;

public:

    SleepRandom(int fromMs, int upToMs) :
        rd(), gen(rd()), dis(fromMs, upToMs)
    {
    }

    void operator() () {
        int ms = dis(gen);
        if (ms > 0)
            std::this_thread::sleep_for(std::chrono::milliseconds(ms));
    }
};


struct Request
{
    static std::atomic_int sNextId;

    // Actually request yupr is not bound to request itself, but module
    RequestTypeId requestTypeId;
    int serial;

    Request() = default;
    Request(const Request &) = default;
    Request& operator=(const Request &) = default;

    static std::shared_ptr<Request> makeRequest(RequestTypeId requestTypeId) {
        std::shared_ptr<Request> request = std::make_shared<Request>();
        request->serial = sNextId.fetch_add(1, std::memory_order_relaxed);
        request->requestTypeId = requestTypeId;
        return request;
    }

    RequestTypeId getULogRequestTypeId() const {
        return requestTypeId;
    }
};


std::atomic_int Request::sNextId;

typedef std::shared_ptr<Request> RequestSptr;


class FilterNode
{
public:
    typedef std::shared_ptr<FilterNode> NodeSptr;

    template <typename _NodeT, typename ..._ArgT>
    static std::shared_ptr<_NodeT> createNode(ModuleId moduleId, _ArgT ...args);

    void addSuccessor(NodeSptr successor);
    void start();
    void stop();

    void setSelfDispatch(bool selfDispatch) {
        mSelfDispatch = selfDispatch;
    }

    bool setEntryChild(NodeSptr child) {
        if (child->mParent != this)
            return false;
        mEntryChild = child;
        return true;
    }

    void setMultiChildrenWait(size_t n) {
        mChildrenWait = n;
    }

    // This constructor had better to be private
    // But std::make_shared<> needs it
    FilterNode(ModuleId moduleId, FilterNode *parent) :
        mChildrenWait(1), mModuleId(moduleId), mParent(parent), mEntryChild(nullptr),
        mSelfDispatch(false), mSleepRandom(0, 5),
        mProcessCount(0), mLostRatio(0), mIsHang(false),
        mRd(), mRdGen(mRd())
    {
    }

    virtual ~FilterNode() { }

    ModuleId getULogModuleId() {
        return mModuleId;
    }

    void setLostRatio(int lostRatio) {
        mLostRatio = lostRatio;
    }

    void setHang(bool hang) {
        if (hang) {
            CAM_ULOGD(mModuleId, "SET HANG");
        }
        mIsHang.store(hang, std::memory_order_relaxed);
    }

protected:

    std::vector<NodeSptr> mChildren;
    std::vector<NodeSptr> mSuccessors;
    std::vector<FilterNode*> mPredecessors;
    size_t mChildrenWait;

    void enque(FilterNode *from, RequestSptr request);
    void enqueNoLog(FilterNode *from, RequestSptr request);

    void enqueToEntryChild(RequestSptr request) {
        mEntryChild->enque(this, request);
    }

    bool dispatchToChildSuccessors(FilterNode *child, RequestSptr request);
    virtual void dispatchOut(RequestSptr request);
    virtual void onProcess(RequestSptr request);
    virtual void onChildDone(FilterNode *child, RequestSptr request);

private:

    struct PendingRequest {
        RequestSptr request;
        size_t incomingCount;

        PendingRequest(RequestSptr &t_request, size_t t_incomingCount) :
            request(t_request), incomingCount(t_incomingCount)
        {
        }
    };

    static constexpr size_t MAX_QUEUE_SIZE = 3;
    ModuleId mModuleId;
    FilterNode *mParent;
    NodeSptr mEntryChild;
    bool mSelfDispatch;
    SleepRandom mSleepRandom;
    std::thread mThread;
    std::mutex mMutex;
    std::atomic_bool mContinueWorking;
    std::condition_variable mQueueCond;
    std::list<PendingRequest> mPartialEnqueRequests;
    std::list<PendingRequest> mPartialChildrenDone;
    std::list<RequestSptr> mRequestQueue;
    int mProcessCount;
    int mLostRatio;
    std::atomic_bool mIsHang;

    void addChild(NodeSptr child);
    void run();

protected:
    std::random_device mRd;
    std::mt19937 mRdGen;
};


template <typename _NodeT, typename ..._ArgT>
std::shared_ptr<_NodeT> FilterNode::createNode(ModuleId moduleId, _ArgT ...args)
{
    std::shared_ptr<_NodeT> node = std::make_shared<_NodeT>(moduleId, std::forward<_ArgT>(args)...);
    if (node->mParent != nullptr)
        node->mParent->addChild(node);
    return node;
}


void FilterNode::addSuccessor(NodeSptr successor)
{
    mSuccessors.emplace_back(successor);
    successor->mPredecessors.emplace_back(this);
}


void FilterNode::addChild(NodeSptr child)
{
    mChildren.emplace_back(child);
    if (mEntryChild == nullptr)
        mEntryChild = child;
}


void FilterNode::start()
{
    mContinueWorking = true;

    for (NodeSptr &child : mChildren)
        child->start();

    mThread = std::thread(&FilterNode::run, this);
}


void FilterNode::stop()
{
    if (!mContinueWorking)
        return;

    for (NodeSptr &child : mChildren)
        child->stop();

    {
        std::unique_lock<std::mutex> lock(mMutex);
        mContinueWorking = false;
        mQueueCond.notify_all();
    }

    mThread.join();
}


void FilterNode::enqueNoLog(FilterNode *from, RequestSptr request)
{
    CAM_ULOG_FUNCLIFE(mModuleId);

    if (request->serial & 0x1) {
        CAM_ULOG_MODIFY_GUARD(this, request, request->serial, 20000);
    }

    if (mPredecessors.size() > 1) {
        CAM_ULOG_PATHJOIN(mModuleId, request, request->serial, from->mModuleId);

        // request can go through only if all partial request arrives
        std::unique_lock<std::mutex> lock(mMutex);
        auto pendingIter = std::find_if(mPartialEnqueRequests.begin(), mPartialEnqueRequests.end(),
            [&request] (PendingRequest &pending) {
                return (request->serial == pending.request->serial);
            }
        );

        if (pendingIter == mPartialEnqueRequests.end()) {
            mPartialEnqueRequests.emplace_back(request, 1);
            return;
        } else {
            pendingIter->incomingCount++;
            if (pendingIter->incomingCount != mPredecessors.size())
                return;
            mPartialEnqueRequests.erase(pendingIter);
        }
    }

    std::unique_lock<std::mutex> lock(mMutex);
    while (mRequestQueue.size() >= MAX_QUEUE_SIZE) {
        mQueueCond.wait(lock);
    }

    mRequestQueue.push_back(request);
    mQueueCond.notify_all();
}



void FilterNode::enque(FilterNode *from, RequestSptr request)
{
    CAM_ULOG_APILIFE(mModuleId);
    CAM_ULOG_ENTER(mModuleId, request, request->serial);
    enqueNoLog(from, request);
}


void FilterNode::run()
{
    while (mContinueWorking) {
        RequestSptr request = nullptr;

        {
            std::unique_lock<std::mutex> lock(mMutex);
            while (mRequestQueue.size() == 0 &&
                    mContinueWorking)
            {
                mQueueCond.wait(lock);
            }

            if (!mContinueWorking)
                break;

            request = std::move(mRequestQueue.front());
            mRequestQueue.pop_front();
            mQueueCond.notify_all();
        }

        onProcess(request);
    }
}


void FilterNode::onProcess(RequestSptr request)
{
    CAM_ULOG_FUNCLIFE_GUARD(this);

    mSleepRandom();

    while (mIsHang.load(std::memory_order_relaxed)) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    mProcessCount++;
    if (mLostRatio > 0 &&
        (mProcessCount % mLostRatio) == (mLostRatio - 1))
    {
        // Lost one request
        CAM_ULOGE(this, "Request lost %u:%u", request->getULogRequestTypeId(), request->serial);
        return;
    }

    if (mEntryChild != nullptr)
        enqueToEntryChild(request);
    else
        dispatchOut(request);
}


void FilterNode::dispatchOut(RequestSptr request)
{
    CAM_ULOG_EXIT(this, request, request->serial);

    if (mSelfDispatch && mSuccessors.size() > 0) {
        if (mSuccessors.size() > 1) {
            ULogVector<ModuleId> divModuleList;
            for (NodeSptr &successor : mSuccessors) {
                divModuleList.push_back(successor->mModuleId);
            }
            CAM_ULOG_PATHDIV(this, request, request->serial, divModuleList);
        }

        // Well.. if I can choose, I will remove self-dispatch
        // As it is difficult to control
        for (NodeSptr &successor : mSuccessors) {
            successor->enque(this, request);
        }
    } else {
        mParent->onChildDone(this, request);
    }
}


bool FilterNode::dispatchToChildSuccessors(FilterNode *child, RequestSptr request)
{
    if (child->mSuccessors.size() > 0) {
        if (mSuccessors.size() > 1) {
            ULogVector<ModuleId> divModuleList;
            for (NodeSptr &successor : mSuccessors) {
                divModuleList.push_back(successor->mModuleId);
            }
            CAM_ULOG_PATHDIV(this, request, request->serial, divModuleList);
        }

        for (NodeSptr &successor : child->mSuccessors) {
            successor->enque(child, request);
        }
        return true;
    }

    return false;
}


void FilterNode::onChildDone(FilterNode *child, RequestSptr request)
{
    CAM_ULOG_DECKEY(this, "%s:%u Child %x done", getULogReqName(request), request->serial, child->mModuleId);

    if (!dispatchToChildSuccessors(child, request)) {
        if (mChildrenWait > 1) {
            CAM_ULOG_PATHJOIN(mModuleId, request, request->serial, child->mModuleId);

            // request can go through only if all partial request arrives
            std::unique_lock<std::mutex> lock(mMutex);
            auto pendingIter = std::find_if(mPartialChildrenDone.begin(), mPartialChildrenDone.end(),
                [&request] (PendingRequest &pending) {
                    return (request->serial == pending.request->serial);
                }
            );

            if (pendingIter == mPartialChildrenDone.end()) {
                mPartialChildrenDone.emplace_back(request, 1);
                return;
            } else {
                pendingIter->incomingCount++;
                if (pendingIter->incomingCount != mChildrenWait)
                    return;
                mPartialEnqueRequests.erase(pendingIter);
            }
        }

        dispatchOut(request);
    }
}


class SubrequestNode : public FilterNode
{
private:
    struct Admission { };

public:
    static std::shared_ptr<SubrequestNode> createNode(ModuleId moduleId, FilterNode *parent, RequestTypeId subReqType,
        int numOfSubMin, int numOfSubMax)
    {
        return FilterNode::createNode<SubrequestNode>(moduleId, parent, subReqType, numOfSubMin, numOfSubMax, Admission());
    }

    SubrequestNode(ModuleId moduleId, FilterNode *parent, RequestTypeId subReqType, int numOfSubMin, int numOfSubMax, Admission&&)
        : FilterNode(moduleId, parent), mSubReqType(subReqType),
        mNumOfSubMin(numOfSubMin), mNumOfSubMax(numOfSubMax), mSubRandom(numOfSubMin, numOfSubMax)

    {
    }

private:

    struct MainRequest {
        int numOfSubsRemain;
        RequestSptr request;

        MainRequest(int t_numOfSubsRemain, RequestSptr t_request) :
            numOfSubsRemain(t_numOfSubsRemain), request(t_request)
        {
        }
    };

    RequestTypeId mSubReqType;
    int mNumOfSubMin, mNumOfSubMax;
    std::mutex mReqMutex;
    std::map<int,int> mSubReqToMain; // subReqId -> mainReqId
    std::map<int,MainRequest> mMainReqRemain; // mainReqId -> MainRequest
    std::uniform_int_distribution<int> mSubRandom;

    virtual void onProcess(RequestSptr request);
    virtual void onChildDone(FilterNode *child, RequestSptr request);
};


void SubrequestNode::onProcess(RequestSptr request)
{
    CAM_ULOG_FUNCLIFE(this);

    if (mChildren.size() != 0) {
        int numOfSubs = mNumOfSubMin;
        if (mNumOfSubMax > mNumOfSubMin)
            numOfSubs = mSubRandom(mRdGen);

        std::vector<RequestSptr> subReqs;
        {
            std::unique_lock<std::mutex> lock(mReqMutex);

            mMainReqRemain.emplace(
                std::piecewise_construct,
                std::forward_as_tuple(request->serial),
                std::forward_as_tuple(numOfSubs * mChildrenWait, request));

            for (int i = 0; i < numOfSubs; i++) {
                RequestSptr subReq = Request::makeRequest(mSubReqType);
                mSubReqToMain[subReq->serial] = request->serial;
                CAM_ULOG_SUBREQS(this, request, request->serial, subReq, subReq->serial);
                subReqs.emplace_back(subReq);
            }
        }

        for (auto &subReq : subReqs)
            enqueToEntryChild(subReq);
    } else {
        dispatchOut(request);
    }
}


void SubrequestNode::onChildDone(FilterNode *child, RequestSptr subReq)
{
    CAM_ULOG_TAGLIFE(this, "SubrequestNode::onChildDone");

    if (!dispatchToChildSuccessors(child, subReq)) {
        RequestSptr mainReq = nullptr;

        {
            std::unique_lock<std::mutex> lock(mReqMutex);

            auto subToMain = mSubReqToMain.find(subReq->serial);
            if (subToMain == mSubReqToMain.end()) {
                CAM_LOGE("SubReq %u:%u not found", subReq->requestTypeId, subReq->serial);
                return;
            }

            int mainId = subToMain->second;
            auto mainReqIt = mMainReqRemain.find(mainId);
            if (mainReqIt == mMainReqRemain.end()) {
                CAM_LOGE("MainReq of %u:%u not found", subReq->requestTypeId, subReq->serial);
                return;
            }

            mainReqIt->second.numOfSubsRemain--;
            CAM_ULOGD(this, "%s:%u subRemain = %d; recv sub %u:%u",
                getULogReqName(mainReqIt->second.request->requestTypeId), mainReqIt->second.request->serial,
                mainReqIt->second.numOfSubsRemain, subReq->requestTypeId, subReq->serial);
            if (mainReqIt->second.numOfSubsRemain == 0) {
                mainReq = std::move(mainReqIt->second.request);
                mMainReqRemain.erase(mainReqIt);
            }
        }

        if (mainReq != nullptr) {
            CAM_ULOG_TAGLIFE(this, "SubrequestNode::onChildDone -> dispatchOut");
            dispatchOut(mainReq);
        }
    }
}


class RootNode : public FilterNode
{
private:
    struct Admission { };

public:
    static std::shared_ptr<RootNode> createNode(ModuleId moduleId) {
        return FilterNode::createNode<RootNode>(moduleId, Admission());
    }

    void enque(RequestSptr request);
    RequestSptr deque();

    RootNode(ModuleId moduleId, Admission&&) : FilterNode(moduleId, nullptr) { }

protected:
    using FilterNode::enque;

private:

    std::mutex mMutex;
    std::condition_variable mCompleteCond;
    std::list<RequestSptr> mCompletedRequests;
    virtual void dispatchOut(RequestSptr request);
};


void RootNode::enque(RequestSptr request)
{
    CAM_ULOG_APILIFE_GUARD(this);
    CAM_ULOGMI("New request enqued: request %u:%d", request->requestTypeId, request->serial);
    CAM_ULOG_ENTER_GUARD(this, request, request->serial);
    FilterNode::enqueNoLog(this, request);
}


RequestSptr RootNode::deque()
{
    CAM_ULOG_APILIFE_GUARD(this, 8000);
    std::unique_lock<std::mutex> lock(mMutex);

    while (mCompletedRequests.empty()) {
        mCompleteCond.wait(lock);
    }

    RequestSptr request = std::move(mCompletedRequests.front());
    mCompletedRequests.pop_front();

    CAM_ULOG_EXIT_GUARD(this, request, request->serial);

    return request;
}


void RootNode::dispatchOut(RequestSptr request)
{
    std::unique_lock<std::mutex> lock(mMutex);
    mCompletedRequests.emplace_back(request);
    mCompleteCond.notify_one();
}


void testCase1()
{
    CAM_ULOG_ENTER(MOD_CAMERA_HAL_SERVER, REQ_APP_REQUEST, 0);
        CAM_ULOG_ENTER(MOD_CAMERA_DEVICE, REQ_APP_REQUEST, 0);
        CAM_ULOG_SUBREQS(MOD_CAMERA_DEVICE, REQ_APP_REQUEST, 0, REQ_PIPELINE_FRAME, 0);
            CAM_ULOG_ENTER(MOD_DEFAULT_PIPELINE_MODEL, REQ_PIPELINE_FRAME, 0);
                CAM_ULOG_ENTER(MOD_P1_NODE, REQ_PIPELINE_FRAME, 0);
                CAM_ULOG_EXIT(MOD_P1_NODE, REQ_PIPELINE_FRAME, 0);
                CAM_ULOG_ENTER(MOD_P2_CAP_NODE, REQ_PIPELINE_FRAME, 0);
                CAM_ULOG_EXIT(MOD_P2_CAP_NODE, REQ_PIPELINE_FRAME, 0);
                CAM_ULOG_ENTER(MOD_P2_STR_NODE, REQ_PIPELINE_FRAME, 0);
                    CAM_ULOG_SUBREQS(MOD_P2_STR_NODE, REQ_PIPELINE_FRAME, 0, REQ_P2_STR_REQUEST, 0);
                    CAM_ULOG_ENTER(MOD_P2_STR_PROC, REQ_P2_STR_REQUEST, 0);
                        {
                        ULogVector<RequestSerial> subReqs{1, 2};
                        CAM_ULOG_SUBREQS(MOD_P2_STR_PROC, REQ_P2_STR_REQUEST, 0, REQ_STR_FPIPE_REQUEST, subReqs);
                        CAM_ULOG_ENTER(MOD_FPIPE_STREAMING, REQ_STR_FPIPE_REQUEST, 1);
                        ModuleList subMod{MOD_STREAMING_RSC, MOD_STREAMING_EIS};
                        CAM_ULOG_PATHDIV(MOD_STREAMING_P2A, REQ_STR_FPIPE_REQUEST, 1, subMod);
                        CAM_ULOG_PATHJOIN(MOD_STREAMING_EIS_WARP, REQ_STR_FPIPE_REQUEST, 1, subMod);
                        CAM_ULOG_EXIT(MOD_FPIPE_STREAMING, REQ_STR_FPIPE_REQUEST, 1);
                        CAM_ULOG_ENTER(MOD_FPIPE_STREAMING, REQ_STR_FPIPE_REQUEST, 2);
                        CAM_ULOG_EXIT(MOD_FPIPE_STREAMING, REQ_STR_FPIPE_REQUEST, 2);
                        }
                        {
                        ULogVector<RequestSerial> subReqs{3, 4};
                        CAM_ULOGD(MOD_P2_STR_PROC, "Generate subrequest %s size=%zu", getULogReqName(REQ_STR_FPIPE_REQUEST), subReqs.size());
                        CAM_ULOG_SUBREQS(MOD_P2_STR_PROC, REQ_P2_STR_REQUEST, 0, REQ_STR_FPIPE_REQUEST, subReqs);
                        CAM_ULOG_ENTER(MOD_FPIPE_STREAMING, REQ_STR_FPIPE_REQUEST, 3);
                        CAM_ULOG_EXIT(MOD_FPIPE_STREAMING, REQ_STR_FPIPE_REQUEST, 3);
                        CAM_ULOG_ENTER(MOD_FPIPE_STREAMING, REQ_STR_FPIPE_REQUEST, 4);
                        CAM_ULOG_DISCARD(MOD_FPIPE_STREAMING, REQ_STR_FPIPE_REQUEST, 4);
                        }
                    CAM_ULOG_EXIT(MOD_P2_STR_PROC, REQ_P2_STR_REQUEST, 0);
                CAM_ULOG_EXIT(MOD_P2_STR_NODE, REQ_PIPELINE_FRAME, 0);
            CAM_ULOG_EXIT(MOD_DEFAULT_PIPELINE_MODEL, REQ_PIPELINE_FRAME, 0);
        CAM_ULOG_EXIT(MOD_CAMERA_DEVICE, REQ_APP_REQUEST, 0);
    CAM_ULOG_EXIT(MOD_CAMERA_HAL_SERVER, REQ_APP_REQUEST, 0);
}


void testCase2(int numOfReqs, int intervalMs, int lostRatio, bool hangFinally)
{
    std::shared_ptr<RootNode> camDevice = RootNode::createNode(MOD_CAMERA_DEVICE);

    FilterNode::NodeSptr
        defaultPipelineModel = SubrequestNode::createNode(MOD_DEFAULT_PIPELINE_MODEL, camDevice.get(), REQ_PIPELINE_FRAME, 1, 1);

    FilterNode::NodeSptr
        p1Node = FilterNode::createNode<FilterNode>(MOD_P1_NODE, defaultPipelineModel.get()),
        p2CapNode = SubrequestNode::createNode(MOD_P2_CAP_NODE, defaultPipelineModel.get(), REQ_P2_CAP_REQUEST, 1, 1),
        p2StrNode = SubrequestNode::createNode(MOD_P2_STR_NODE, defaultPipelineModel.get(), REQ_P2_STR_REQUEST, 1, 1);
    p1Node->addSuccessor(p2CapNode);
    p1Node->addSuccessor(p2StrNode);
    defaultPipelineModel->setMultiChildrenWait(2);

    FilterNode::NodeSptr
        streamingFeaturePipe = SubrequestNode::createNode(MOD_FPIPE_STREAMING, p2StrNode.get(), REQ_STR_FPIPE_REQUEST, 1, 3);
    FilterNode::NodeSptr
        rootNode = FilterNode::createNode<FilterNode>(MOD_STREAMING_ROOT, streamingFeaturePipe.get()),
        p2aNode = FilterNode::createNode<FilterNode>(MOD_STREAMING_P2A, streamingFeaturePipe.get()),
        rscNode = FilterNode::createNode<FilterNode>(MOD_STREAMING_RSC, streamingFeaturePipe.get()),
        eisNode = FilterNode::createNode<FilterNode>(MOD_STREAMING_EIS, streamingFeaturePipe.get()),
        eisWarpNode = FilterNode::createNode<FilterNode>(MOD_STREAMING_EIS_WARP, streamingFeaturePipe.get()),
        helperNode = FilterNode::createNode<FilterNode>(MOD_STREAMING_HELPER, streamingFeaturePipe.get());
    rootNode->addSuccessor(p2aNode);
    rootNode->addSuccessor(rscNode);
    p2aNode->addSuccessor(eisWarpNode);
    rscNode->addSuccessor(eisNode);
    eisNode->addSuccessor(eisWarpNode);
    eisWarpNode->addSuccessor(helperNode);
    eisNode->setLostRatio(lostRatio);

    camDevice->start();

    auto asyncDeque = std::async(std::launch::async,
        [&camDevice, numOfReqs] () -> void {
            for (int i = 0; i < numOfReqs; i++) {
                camDevice->deque();
            }
        }
    );

    for (int i = 0; i < numOfReqs; i++) {
        RequestSptr request = Request::makeRequest(REQ_APP_REQUEST);
        if (hangFinally && i == numOfReqs - 1)
            rscNode->setHang(true);
        camDevice->enque(request);
        if (intervalMs > 0)
            std::this_thread::sleep_for(std::chrono::milliseconds(intervalMs));
    }

    asyncDeque.get();

    camDevice->stop();
}


void testFunc2(int arg1, void *arg2)
{
    CAM_ULOG_FUNCLIFE_ARGS(MOD_CAMERA_HAL_SERVER, arg1, arg2);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
}


void testFunc3(long arg)
{
    CAM_ULOG_APILIFE_ARGS(MOD_DEFAULT_PIPELINE_MODEL, arg);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
}


void testFunc()
{
    CAM_ULOGMD("testfunc begin(), CAM_ULOGMD works!");

    CAM_ULOG_FUNCLIFE(MOD_CAMERA_HAL_SERVER);

    auto func2Thread = [] {
        for (int i = 0; i < 100; i++) {
            int v = 0;
            CAM_ULOG_DTAG_BEGIN(MOD_CAMERA_HAL_SERVER, i == 2, "Call testFunc2 %d %d", i, v);
            testFunc2(5, &v);
            CAM_ULOG_DTAG_END();
        }
    };

    auto func3Thread = [] {
        for (int i = 0; i < 100; i++) {
            CAM_ULOG_TAG_BEGIN(MOD_CAMERA_HAL_SERVER, "Call testFunc3");
            testFunc3(10);
            CAM_ULOG_TAG_END();
        }
    };

    auto t2 = std::async(std::launch::async, func2Thread);
    auto t3 = std::async(std::launch::async, func3Thread);

    t2.get();
    t3.get();
}


class FinalizerTest : public ULogGuard::IFinalizer
{
public:
    virtual void onTimeout(const ULogGuard::Status &) {
        CAM_ULOGMD("Finalizer works!");
    }
};


void testTimeBomb(bool triggerTimeout)
{
    {
        android::sp<ULogTimeBomb> tb1 = CAM_ULOG_TIMEBOMB_CREATE(MOD_PIPELINE_MODEL, 0, "TimeBombTest", 20000, 40000);
    }

    {
        std::list<android::sp<ULogTimeBomb>> timeBombs;
        for (int i = 0; i < 200; i++) {
            android::sp<ULogTimeBomb> tb = CAM_ULOG_TIMEBOMB_CREATE(MOD_PIPELINE_MODEL, 0, "TimeBombTest", 10000, 20000);
            timeBombs.emplace_back(tb);
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(5000));
    }

    std::list<android::sp<ULogTimeBomb>> timeBombs;
    for (int i = 0; i < 3; i++) {
        android::sp<ULogTimeBomb> tb = CAM_ULOG_TIMEBOMB_CREATE(MOD_PIPELINE_MODEL, i, "TimeBombTest", 10000, 20000);
        timeBombs.emplace_back(tb);
        android::sp<ULogTimeBomb> tbv = CAM_ULOG_TIMEBOMB_VERBOSE_CREATE(MOD_PIPELINE_MODEL, "TimeBombTest", 10000, 20000, 30000,
            "Verbose %d", i);
        timeBombs.emplace_back(tbv);
    }

    (*timeBombs.begin())->extendTime(2000, 1000);

    if (triggerTimeout) {
        std::this_thread::sleep_for(std::chrono::milliseconds(30000));
    }
}


void testOvertimeFormat(bool triggerTimeout)
{
    CAM_ULOG_TAG_OVERTIMEF(MOD_UNIFIED_LOG, "testOvertimeFormat", 2000, 0, 60,
        "triggerTimeout = %d", (triggerTimeout ? 1 : 0));

    if (triggerTimeout) {
        std::this_thread::sleep_for(std::chrono::milliseconds(30000));
    }
}


void testOvertimeKeyValue(bool triggerTimeout)
{
    CAM_ULOGM_TAG_OVERTIME("testOvertimeKeyValue", 2000, 0, triggerTimeout);

    if (triggerTimeout) {
        std::this_thread::sleep_for(std::chrono::milliseconds(30000));
    }
}



}
}
}


int main(int argc, char *argv[])
{
    ULogInitializer ulogInit; // The constructor will init ULog
    android::sp<NSCam::Utils::ULogTest::FinalizerTest> finalizer = new NSCam::Utils::ULogTest::FinalizerTest;

    ULogGuard::registerFinalizer(finalizer);

    ULogPrinter printer(MOD_UNIFIED_LOG, LOG_TAG);

    unsigned int flags = 0xff;
    if (argc > 1)
        flags = static_cast<unsigned int>(strtol(argv[1], NULL, 0));

    printer.printFormatLine("Go: %x", flags);

    if (flags & 0x1)
        NSCam::Utils::ULogTest::testCase1();

    if (flags & 0x2) {
        if (argc <= 4) {
            NSCam::Utils::ULogTest::testCase2(1000, 0, 0, false);
        } else {
            bool hangFinally = argv[2][0] != '0';
            int numOfReqs = atoi(argv[3]);
            int intervalMs = atoi(argv[4]);
            NSCam::Utils::ULogTest::testCase2(numOfReqs, intervalMs, 0, hangFinally);
        }
    }

    if (flags & 0x4) {
        NSCam::Utils::ULogTest::testFunc();
    }

    if (flags & 0x8) {
        bool triggerTimeout = (argc > 2) && (argv[2][0] != '0');
        NSCam::Utils::ULogTest::testTimeBomb(triggerTimeout);
    }

    if (flags & 0x10) {
        bool triggerTimeout = (argc > 2) && (argv[2][0] != '0');
        NSCam::Utils::ULogTest::testOvertimeFormat(triggerTimeout);
    }

    if (flags & 0x20) {
        bool triggerTimeout = (argc > 2) && (argv[2][0] != '0');
        NSCam::Utils::ULogTest::testOvertimeKeyValue(triggerTimeout);
    }


    printer.printLine("End");

    return 0;
}

