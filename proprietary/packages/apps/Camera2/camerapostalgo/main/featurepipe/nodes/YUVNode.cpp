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

#include "core/DebugControl.h"
#define PIPE_CLASS_TAG "YUVNode"
#define PIPE_TRACE TRACE_YUV_NODE
#include <core/PipeLog.h>

#include "YUVNode.h"

#include <sstream>
#include <tuple>

#include <utils/std/Format.h>
#include <pipe/FeaturePlugin.h>
//#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>
//#include <mtkcam/utils/metadata/client/mtk_metadata_tag.h>
//#include <mtkcam/drv/iopipe/SImager/ISImager.h>

using namespace NSCam::NSPipelinePlugin;
using namespace NSCam::Utils::Format;


/*******************************************************************************
* MACRO Utilities Define.
********************************************************************************/
namespace { // anonymous namespace for MACRO function
using AutoObject = std::unique_ptr<const char, std::function<void(const char*)>>;
//
auto
createAutoScoper(const char* funcName) -> AutoObject
{
    CAM_LOGD("[%s] +", funcName);
    return AutoObject(funcName, [](const char* p)
    {
        CAM_LOGD("[%s] -", p);
    });
}
#define SCOPED_TRACER() auto scoped_tracer = ::createAutoScoper(__FUNCTION__)
//
auto
createAutoTimer(const char* funcName, const char* fmt, ...) -> AutoObject
{
    static const MINT32 LENGTH = 512;
    using Timing = std::chrono::time_point<std::chrono::high_resolution_clock>;
    using DuationTime = std::chrono::duration<float, std::milli>;
    //
    char* pBuf = new char[LENGTH];

    va_list ap;
    va_start(ap, fmt);
    vsnprintf(pBuf, LENGTH, fmt, ap);
    va_end(ap);
    //
    Timing startTime = std::chrono::high_resolution_clock::now();
    return AutoObject(pBuf, [funcName, startTime](const char* p)
    {
        Timing endTime = std::chrono::high_resolution_clock::now();
        DuationTime duationTime = endTime - startTime;
        CAM_LOGD("[%s]%s, elapsed(ms):%.4f", funcName, p, duationTime.count());

        delete[] p;
    });
}
#define AUTO_TIMER(FMT, arg...) auto auto_timer = ::createAutoTimer(__FUNCTION__, FMT, ##arg);
//
} // end anonymous namespace for MACRO function

/*******************************************************************************
* Namespace start.
********************************************************************************/
namespace com {
namespace mediatek {
namespace campostalgo {
namespace NSFeaturePipe {


/*******************************************************************************
* Type Alias.
********************************************************************************/
using namespace NSCam::NSPipelinePlugin;
//using namespace NSCam::NSIoPipe::NSSImager;
//
using YuvPlugin = YUVNode::YuvPlugin;
using Selection = YuvPlugin::Selection;
using PluginBufferHandlePtr =  NSCam::NSPipelinePlugin::BufferHandle::Ptr;


/*******************************************************************************
* Type Alias.
********************************************************************************/
static constexpr MINT INVALID_FORMAT = -1;


/*******************************************************************************
* Class Define
*******************************************************************************/
class YuvInterface : public YuvPlugin::IInterface
{
public:
    virtual MERROR offer(YuvPlugin::Selection& sel)
    {
        sel.mIBufferFull
            .addSupportFormat(eImgFmt_NV12)
            .addSupportFormat(eImgFmt_YV12)
            .addSupportFormat(eImgFmt_YUY2)
            .addSupportFormat(eImgFmt_NV21)
            .addSupportFormat(eImgFmt_I420)
            .addSupportFormat(eImgFmt_YUV_P010)
            .addSupportFormat(eImgFmt_MTK_YUV_P010)
            .addSupportFormat(eImgFmt_YUV_P010)
            .addSupportSize(eImgSize_Full);

        sel.mIBufferClean
            .addSupportFormat(eImgFmt_NV12)
            .addSupportFormat(eImgFmt_YV12)
            .addSupportFormat(eImgFmt_YUY2)
            .addSupportFormat(eImgFmt_NV21)
            .addSupportFormat(eImgFmt_I420)
            .addSupportFormat(eImgFmt_YUV_P010)
            .addSupportFormat(eImgFmt_MTK_YUV_P010)
            .addSupportFormat(eImgFmt_YUV_P010)
            .addSupportSize(eImgSize_Full);

        sel.mIBufferDepth
            .addSupportFormat(eImgFmt_Y8)
            .addSupportSize(eImgSize_Specified);

        sel.mOBufferFull
            .addSupportFormat(eImgFmt_NV12)
            .addSupportFormat(eImgFmt_YV12)
            .addSupportFormat(eImgFmt_YUY2)
            .addSupportFormat(eImgFmt_NV21)
            .addSupportFormat(eImgFmt_I420)
            .addSupportFormat(eImgFmt_YUV_P010)
            .addSupportFormat(eImgFmt_MTK_YUV_P010)
            .addSupportFormat(eImgFmt_YUV_P010)
            .addSupportSize(eImgSize_Full);

        sel.mOBufferFull0
            .addSupportFormat(eImgFmt_NV12)
            .addSupportFormat(eImgFmt_YV12)
            .addSupportFormat(eImgFmt_YUY2)
            .addSupportFormat(eImgFmt_NV21)
            .addSupportFormat(eImgFmt_I420)
            .addSupportFormat(eImgFmt_YUV_P010)
            .addSupportFormat(eImgFmt_MTK_YUV_P010)
            .addSupportFormat(eImgFmt_YUV_P010)
            .addSupportSize(eImgSize_Full);

        sel.mOBufferFull1
            .addSupportFormat(eImgFmt_NV12)
            .addSupportFormat(eImgFmt_YV12)
            .addSupportFormat(eImgFmt_YUY2)
            .addSupportFormat(eImgFmt_NV21)
            .addSupportFormat(eImgFmt_I420)
            .addSupportFormat(eImgFmt_YUV_P010)
            .addSupportFormat(eImgFmt_MTK_YUV_P010)
            .addSupportFormat(eImgFmt_YUV_P010)
            .addSupportSize(eImgSize_Full);

        sel.mOBufferFull2
            .addSupportFormat(eImgFmt_NV12)
            .addSupportFormat(eImgFmt_YV12)
            .addSupportFormat(eImgFmt_YUY2)
            .addSupportFormat(eImgFmt_NV21)
            .addSupportFormat(eImgFmt_I420)
            .addSupportFormat(eImgFmt_YUV_P010)
            .addSupportFormat(eImgFmt_MTK_YUV_P010)
            .addSupportFormat(eImgFmt_YUV_P010)
            .addSupportSize(eImgSize_Full);

        sel.mOBufferFull3
            .addSupportFormat(eImgFmt_NV12)
            .addSupportFormat(eImgFmt_YV12)
            .addSupportFormat(eImgFmt_YUY2)
            .addSupportFormat(eImgFmt_NV21)
            .addSupportFormat(eImgFmt_I420)
            .addSupportFormat(eImgFmt_YUV_P010)
            .addSupportFormat(eImgFmt_MTK_YUV_P010)
            .addSupportFormat(eImgFmt_YUV_P010)
            .addSupportSize(eImgSize_Full);

        sel.mOBufferFull4
            .addSupportFormat(eImgFmt_NV12)
            .addSupportFormat(eImgFmt_YV12)
            .addSupportFormat(eImgFmt_YUY2)
            .addSupportFormat(eImgFmt_NV21)
            .addSupportFormat(eImgFmt_I420)
            .addSupportFormat(eImgFmt_YUV_P010)
            .addSupportFormat(eImgFmt_MTK_YUV_P010)
            .addSupportFormat(eImgFmt_YUV_P010)
            .addSupportSize(eImgSize_Full);

        sel.mOBufferFull5
            .addSupportFormat(eImgFmt_NV12)
            .addSupportFormat(eImgFmt_YV12)
            .addSupportFormat(eImgFmt_YUY2)
            .addSupportFormat(eImgFmt_NV21)
            .addSupportFormat(eImgFmt_I420)
            .addSupportFormat(eImgFmt_YUV_P010)
            .addSupportFormat(eImgFmt_MTK_YUV_P010)
            .addSupportFormat(eImgFmt_YUV_P010)
            .addSupportSize(eImgSize_Full);

        sel.mOBufferFull6
            .addSupportFormat(eImgFmt_NV12)
            .addSupportFormat(eImgFmt_YV12)
            .addSupportFormat(eImgFmt_YUY2)
            .addSupportFormat(eImgFmt_NV21)
            .addSupportFormat(eImgFmt_I420)
            .addSupportFormat(eImgFmt_YUV_P010)
            .addSupportFormat(eImgFmt_MTK_YUV_P010)
            .addSupportFormat(eImgFmt_YUV_P010)
            .addSupportSize(eImgSize_Full);

        sel.mOBufferFull7
            .addSupportFormat(eImgFmt_NV12)
            .addSupportFormat(eImgFmt_YV12)
            .addSupportFormat(eImgFmt_YUY2)
            .addSupportFormat(eImgFmt_NV21)
            .addSupportFormat(eImgFmt_I420)
            .addSupportFormat(eImgFmt_YUV_P010)
            .addSupportFormat(eImgFmt_MTK_YUV_P010)
            .addSupportFormat(eImgFmt_YUV_P010)
            .addSupportSize(eImgSize_Full);

        return OK;
    };

    virtual ~YuvInterface() {};
};

REGISTER_PLUGIN_INTERFACE(Yuv, YuvInterface);

/******************************************************************************
*
******************************************************************************/
class YuvCallback : public YuvPlugin::RequestCallback
{
public:
    YuvCallback(YUVNode* pNode)
        : mpNode(pNode)
    {
    }

    virtual void onAborted(YuvPlugin::Request::Ptr pPlgRequest)
    {
        MY_LOGD("onAborted request: %p", pPlgRequest.get());
        onCompleted(pPlgRequest, UNKNOWN_ERROR);
    }

    virtual void onCompleted(YuvPlugin::Request::Ptr pPlgRequest, MERROR result)
    {
        RequestPtr pCapRequest = mpNode->findRequest(pPlgRequest);

        if (pCapRequest == NULL) {
            MY_LOGE("unknown request happened: %p, result %d", pPlgRequest.get(), result);
            return;
        }

        *pPlgRequest = YuvPlugin::Request();
        MY_LOGD("onCompleted request:%p, result:%d", pPlgRequest.get(), result);

        if (result != OK) {
            pCapRequest->addParameter(PID_FAILURE, 1);
        }

        MBOOL bRepeat = mpNode->onRequestRepeat(pCapRequest);
        // no more repeating
        if (bRepeat) {
            mpNode->onRequestProcess(pCapRequest);
        } else {
            mpNode->onRequestFinish(pCapRequest);
        }
    }

    virtual void onNextCapture(YuvPlugin::Request::Ptr pPlgRequest)
    {
        RequestPtr pRequest = mpNode->findRequest(pPlgRequest);

        if (pRequest == NULL) {
            MY_LOGE("unknown request happened: %p", pPlgRequest.get());
            return;
        }

        const MUINT32 frameCount = pRequest->getActiveFrameCount();
        const MUINT32 frameIndex = pRequest->getActiveFrameIndex();
        MY_LOGW("not support next capture at (%dI%d)", frameIndex, frameCount);
    }

    virtual ~YuvCallback() { };
private:
    YUVNode* mpNode;
};

#if 0
/******************************************************************************
*
******************************************************************************/
/**
 * @brief implemented class of the NSCam::NSPipelinePlugin::BufferHandle
 */
class PluginSmartBufferHandle final: public NSCam::NSPipelinePlugin::BufferHandle
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Type Alias.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    using Ptr = std::shared_ptr<PluginSmartBufferHandle>;
    using OnReleasingCallback = std::function<MVOID(IImageBuffer*)>;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Instantiation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    PluginSmartBufferHandle(const char* name, const android::sp<IIBuffer>& pSmartImageBuffer);

    ~PluginSmartBufferHandle();

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  NSCam::NSPipelinePlugin::BufferHandle Public Operators.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    IImageBuffer* acquire(MINT usage) override;

    MVOID release() override;

    MVOID dump(std::ostream& os) const override;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  PluginSmartBufferHandle Public Operators.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    android::sp<IIBuffer> getSmartImageBuffer();

    MVOID setOnReleasingCallback(OnReleasingCallback onReleasingCB);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  PluginBufferHandle Private Data Members.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    std::string             mName;
    android::sp<IIBuffer>   mSmartImageBuffer;
    OnReleasingCallback     mOnReleasingCallback;
};
using PluginSmartBufferHandlePtr = PluginSmartBufferHandle::Ptr;

#endif
/**
 * @brief the class, for caching the partial result of selection, that had Negotiated.
 *
 * the class provides the guarantee of thread-safe
 */
class YUVNode::NegotiatedCacher final
{
public:
    NegotiatedCacher();

public:
    MBOOL getIsFeatureIdExisting(FeatureID_T featureId) const;

    MVOID add(FeatureID_T featureId, const std::vector<MINT>& iFmts, const std::vector<MINT>& oFmts);

    MBOOL getIsSupportedInputBufferFmt(FeatureID_T featureId, MINT fmt) const;

    MBOOL getSupportedInputBufferFmt(FeatureID_T featureId, MINT& fmt) const;

    MBOOL getIsSupportedOutputBufferFmt(FeatureID_T featureId, MINT fmt) const;

    MBOOL getSupportedOutputBufferFmt(FeatureID_T featureId, MINT& fmt) const;

private:
    struct NegotiatedInfo
    {
    public:
        inline const std::string toString() const;

    public:
        std::vector<MINT> mIFmts;
        std::vector<MINT> mOFmts;
    };

private:
    MBOOL innerGetIsFeatureIdExisting(FeatureID_T featureId) const;

private:
    using NegotiatedInfoTable = std::map<FeatureID_T, const NegotiatedInfo>;

private:
    mutable std::mutex  mLocker;
    NegotiatedInfoTable mNegotiatedInfoTable;
};

#if 0
/**
 * @brief helper class used by YuvNode
 */
class YuvNodeHelper final
{
public:
    YuvNodeHelper() = delete;

    static MINT32 msiSensorIdx;

public:
    static MBOOL existNodeBuffer(const sp<FeatureNodeRequest>& pNodeReq, TypeID_T typeId, Direction dir);
    static MVOID setSensorIdx(MINT32 iSensorIdx) {msiSensorIdx = iSensorIdx;}
    static MINT getImageFormat(const sp<FeatureNodeRequest>& pNodeReq, TypeID_T typeId, Direction dir);

    static MSize getImageSize(const sp<FeatureNodeRequest>& pNodeReq, TypeID_T typeId, Direction dir);

    static MBOOL formatConverter(IImageBuffer *pSrcImg, IImageBuffer *pDstImg);

    static MBOOL tryGetTheSameBufferFmt(const Selection& selection, MINT& iFmt, MINT& oFmt);

    static MBOOL getIsDumpImage(NodeID_T nodeId);

    static inline PluginSmartBufferHandlePtr createSmartBuffer(const char* name, MINT fmt, const sp<FeatureNodeRequest>& pNodeReq, TypeID_T typeId, Direction dire, const sp<WorkingBufferPool>& pBufferPool);

    static inline PluginSmartBufferHandlePtr createSmartBuffer(const char* name, MINT fmt, MSize size, const sp<WorkingBufferPool>& pBufferPool);

    static inline PluginSmartBufferHandlePtr createSmartBuffer(const char* name, const android::sp<IIBuffer>& smartImgBuffer);
};


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  PluginSmartBufferHandle Implementation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
PluginSmartBufferHandle::
PluginSmartBufferHandle(const char* name, const android::sp<IIBuffer>& pSmartImageBuffer)
    : mName(name)
    , mSmartImageBuffer(pSmartImageBuffer)
{
    if (mSmartImageBuffer == NULL) {
        MY_LOGW("construct an invalid handle, name:%s", mName.c_str());
    }
}

PluginSmartBufferHandle::
~PluginSmartBufferHandle()
{
    if (mSmartImageBuffer != NULL) {
        MY_LOGW("did NOT release plugin buffer, name:%s", mName.c_str());
        release();
    }
}

IImageBuffer*
PluginSmartBufferHandle::
acquire(MINT usage)
{
    (void) usage;
    if (mSmartImageBuffer == NULL)
        return NULL;

    return mSmartImageBuffer->getImageBufferPtr();
}

MVOID
PluginSmartBufferHandle::
release()
{
    if (mSmartImageBuffer != NULL) {
        if (mOnReleasingCallback != NULL) {
            mOnReleasingCallback(acquire(eBUFFER_USAGE_HW_CAMERA_READWRITE | eBUFFER_USAGE_SW_READ_OFTEN));
        }
        mSmartImageBuffer = NULL;
    }
}

MVOID
PluginSmartBufferHandle::
dump(std::ostream& os) const
{
    os << "{ not implementation }" << std::endl;
}

android::sp<IIBuffer>
PluginSmartBufferHandle::
getSmartImageBuffer()
{
    return mSmartImageBuffer;
}

MVOID
PluginSmartBufferHandle::
setOnReleasingCallback(OnReleasingCallback onReleasingCB)
{
    mOnReleasingCallback = onReleasingCB;
}
#endif

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  NegotiatedCacher Implementation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
YUVNode::NegotiatedCacher::
NegotiatedCacher()
{

}

MBOOL
YUVNode::NegotiatedCacher::
getIsFeatureIdExisting(FeatureID_T featureId) const
{
    std::lock_guard<std::mutex> lock(mLocker);
    return innerGetIsFeatureIdExisting(featureId);
}

MVOID
YUVNode::NegotiatedCacher::
add(FeatureID_T featureId, const std::vector<MINT>& iFmts, const std::vector<MINT>& oFmts)
{
    std::lock_guard<std::mutex> lock(mLocker);
    if (innerGetIsFeatureIdExisting(featureId)) {
        MY_LOGW("invalid operator, featureId is existing, id:%s, value:%s",
            FeatID2Name(featureId), mNegotiatedInfoTable[featureId].toString().c_str());
        return;
    }
    //
    NegotiatedInfo info;
    info.mIFmts = iFmts;
    info.mOFmts = oFmts;
    auto ret = mNegotiatedInfoTable.insert(std::pair<FeatureID_T, NegotiatedInfo>(featureId, info));
    MY_LOGD("insert new item into negotiated table, id:%s, value:%s", FeatID2Name(featureId), mNegotiatedInfoTable[featureId].toString().c_str());
}

MBOOL
YUVNode::NegotiatedCacher::
getIsSupportedInputBufferFmt(FeatureID_T featureId, MINT fmt) const
{
    std::lock_guard<std::mutex> lock(mLocker);
    MBOOL ret = MFALSE;
    if (!innerGetIsFeatureIdExisting(featureId)) {
        MY_LOGW("the feature(%s) is not existing", FeatID2Name(featureId));
        return ret;
    }
    //
    const std::vector<MINT>& fmts = mNegotiatedInfoTable.at(featureId).mIFmts;
    if (std::find(fmts.begin(), fmts.end(), fmt) != fmts.end()) {
        ret = MTRUE;
    }
    return ret;
}

MBOOL
YUVNode::NegotiatedCacher::
getSupportedInputBufferFmt(FeatureID_T featureId, MINT& fmt) const
{
    std::lock_guard<std::mutex> lock(mLocker);
    MBOOL ret = MFALSE;
    if (!innerGetIsFeatureIdExisting(featureId)) {
        MY_LOGW("the feature(%s) is not existing", FeatID2Name(featureId));
        return ret;
    }
    //
    const std::vector<MINT>& fmts = mNegotiatedInfoTable.at(featureId).mIFmts;
    if (!fmts.empty()) {
        fmt = fmts.front();
        ret = MTRUE;
    }
    return ret;
}

MBOOL
YUVNode::NegotiatedCacher::
getIsSupportedOutputBufferFmt(FeatureID_T featureId, MINT fmt) const
{
    std::lock_guard<std::mutex> lock(mLocker);
    MBOOL ret = MFALSE;
    if (!innerGetIsFeatureIdExisting(featureId)) {
        MY_LOGW("the featureId:%s is not existing", FeatID2Name(featureId));
        return ret;
    }
    //
    const std::vector<MINT>& fmts = mNegotiatedInfoTable.at(featureId).mOFmts;
    if(std::find(fmts.begin(), fmts.end(), fmt) != fmts.end()) {
        ret = MTRUE;
    }
    return ret;
}

MBOOL
YUVNode::NegotiatedCacher::
getSupportedOutputBufferFmt(FeatureID_T featureId, MINT& fmt) const
{
    std::lock_guard<std::mutex> lock(mLocker);
    MBOOL ret = MFALSE;
    if (!innerGetIsFeatureIdExisting(featureId)) {
        MY_LOGW("the feature(%s) is not existing", FeatID2Name(featureId));
        return ret;
    }
    //
    const std::vector<MINT>& fmts = mNegotiatedInfoTable.at(featureId).mOFmts;
    if (!fmts.empty()) {
        fmt = fmts.front();
        ret = MTRUE;
    }
    return ret;
}

const std::string
YUVNode::NegotiatedCacher::NegotiatedInfo::
toString() const
{
    auto toString = [](const std::string& name, const std::vector<MINT>& items)
    {
        std::ostringstream out;

        out << "{"
            << name << ":";

        const int last = items.size() - 1;
        for(int i = 0; i <= last; i++) {
            out << std::hex << std::showbase <<  queryImageFormatName(items[i])
                                             << "("
                                             << items[i]
                                             << ")";
            if(i != last)
                out << ", ";
        }
        out << "}";
        return out.str();
    };
    //
    std::ostringstream ret;
    ret << "{";
    ret << toString("iFmts", mIFmts);
    ret << ", ";
    ret << toString("oFmts", mOFmts);
    ret << "}";
    return ret.str();
}

MBOOL
YUVNode::NegotiatedCacher::
innerGetIsFeatureIdExisting(FeatureID_T featureId) const
{
    return (mNegotiatedInfoTable.count(featureId) > 0);
}

#if 0
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  YuvNodeHelper Implementation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

MINT32 YuvNodeHelper::msiSensorIdx;

MBOOL
YuvNodeHelper::
existNodeBuffer(const sp<FeatureNodeRequest>& pNodeReq, TypeID_T typeId, Direction dir)
{
    BufferID_T bufId = pNodeReq->mapBufferID(typeId, dir);
    IImageBuffer* pImgBuffer = pNodeReq->acquireBuffer(bufId);
    return (pImgBuffer != nullptr);
}

MINT
YuvNodeHelper::
getImageFormat(const sp<FeatureNodeRequest>& pNodeReq, TypeID_T typeId, Direction dir)
{
    // NOTE: this function DOESN'T call releaseBuffer to decrease the buffer reference count
    BufferID_T bufId = pNodeReq->mapBufferID(typeId, dir);
    IImageBuffer* pImgBuffer = pNodeReq->acquireBuffer(bufId);
    return pImgBuffer->getImgFormat();
}

MSize
YuvNodeHelper::
getImageSize(const sp<FeatureNodeRequest>& pNodeReq, TypeID_T typeId, Direction dir)
{
    // NOTE: this function DOESN'T call releaseBuffer to decrease the buffer reference count
    BufferID_T bufId = pNodeReq->mapBufferID(typeId, dir);
    IImageBuffer* pImgBuffer = pNodeReq->acquireBuffer(bufId);
    return pImgBuffer->getImgSize();
}

MBOOL
YuvNodeHelper::
formatConverter(IImageBuffer *pSrcImg, IImageBuffer *pDstImg)
{
    SCOPED_TRACER();

    AUTO_TIMER("format transfer, srcSize:%dx%d, srcFmt:%s, dstSize:%dx%d, dstFmt:%s",
        pSrcImg->getImgSize().w,  pSrcImg->getImgSize().h, ImgFmt2Name(pSrcImg->getImgFormat()),
        pDstImg->getImgSize().w,  pDstImg->getImgSize().h, ImgFmt2Name(pDstImg->getImgFormat()));

    using NSIoPipe::NSSImager::ISImager;
    auto pSImager = UniquePtr<ISImager>(ISImager::createInstance(pSrcImg, msiSensorIdx), [](ISImager* p)
    {
        if(p != nullptr)
            p->destroyInstance();
    });

    if(pSImager == nullptr) {
        MY_LOGE("ISImager::createInstance() failed!!!");
        return MFALSE;
    }

    if (!pSImager->setTargetImgBuffer(pDstImg)) {
        MY_LOGE("setTargetImgBuffer failed!!!");
        return MFALSE;
    }

    if (!pSImager->execute()) {
        MY_LOGE("execute failed!!!");
        return MFALSE;
    }

    return MTRUE;
}

MBOOL
YuvNodeHelper::
tryGetTheSameBufferFmt(const Selection& selection, MINT& iFmt, MINT& oFmt)
{
    const std::vector<MINT>& iFmts = selection.mIBufferFull.getFormats();
    const std::vector<MINT>& oFmts = selection.mOBufferFull.getFormats();
    for(MINT fmt : iFmts) {
        if(std::find(oFmts.begin(), oFmts.end(), fmt) != oFmts.end()) {
            iFmt = fmt;
            oFmt = fmt;
            return MTRUE;
        }
    }
    return MFALSE;
}

MBOOL
YuvNodeHelper::
getIsDumpImage(NodeID_T nodeId)
{
    if (nodeId == NID_YUV) {
        return NSCam::NSCamFeature::NSFeaturePipe::NSCapture::getIsDumpImage<NID_YUV>();
    }
    else if (nodeId == NID_YUV_R1) {
        return NSCam::NSCamFeature::NSFeaturePipe::NSCapture::getIsDumpImage<NID_YUV_R1>();
    }
    else if (nodeId == NID_YUV_R2) {
        return NSCam::NSCamFeature::NSFeaturePipe::NSCapture::getIsDumpImage<NID_YUV_R2>();
    }
    else if (nodeId == NID_YUV_R3) {
        return NSCam::NSCamFeature::NSFeaturePipe::NSCapture::getIsDumpImage<NID_YUV_R3>();
    }
    else if (nodeId == NID_YUV_R4) {
        return NSCam::NSCamFeature::NSFeaturePipe::NSCapture::getIsDumpImage<NID_YUV_R4>();
    }
    else if (nodeId == NID_YUV_R5) {
        return NSCam::NSCamFeature::NSFeaturePipe::NSCapture::getIsDumpImage<NID_YUV_R5>();
    }
    else if (nodeId == NID_YUV2) {
        return  NSCam::NSCamFeature::NSFeaturePipe::NSCapture::getIsDumpImage<NID_YUV2>();
    }
    else if (nodeId == NID_YUV2_R1) {
        return  NSCam::NSCamFeature::NSFeaturePipe::NSCapture::getIsDumpImage<NID_YUV2_R1>();
    }
    else if (nodeId == NID_YUV2_R2) {
        return  NSCam::NSCamFeature::NSFeaturePipe::NSCapture::getIsDumpImage<NID_YUV2_R2>();
    }
     else if (nodeId == NID_YUV2_R3) {
        return  NSCam::NSCamFeature::NSFeaturePipe::NSCapture::getIsDumpImage<NID_YUV2_R3>();
    }
    else if (nodeId == NID_YUV2_R4) {
        return  NSCam::NSCamFeature::NSFeaturePipe::NSCapture::getIsDumpImage<NID_YUV2_R4>();
    }
     else if (nodeId == NID_YUV2_R5) {
        return  NSCam::NSCamFeature::NSFeaturePipe::NSCapture::getIsDumpImage<NID_YUV2_R5>();
    }
    else {
        MY_LOGW("not supported nodeId, node:%d(%s)", nodeId, NodeID2Name(nodeId));
    }
    return MFALSE;
}

PluginSmartBufferHandlePtr
YuvNodeHelper::
createSmartBuffer(const char* name, MINT fmt, const sp<FeatureNodeRequest>& pNodeReq, TypeID_T typeId, Direction dir, const sp<WorkingBufferPool>& pBufferPool)
{
    MY_LOGD("create smart image buffer, name:%s , fmt:%s, typeId:%s, dir:%d, poolAddr:%p",
        name, ImgFmt2Name(fmt), TypeID2Name(typeId), dir, pBufferPool.get());

    // NOTE: this function will decrease the buffer reference count of current node
    const BufferID_T srcBufId = pNodeReq->mapBufferID(typeId, dir);
    UniquePtr<IImageBuffer> pSrcImgBuffer = UniquePtr<IImageBuffer>(pNodeReq->acquireBuffer(srcBufId), [pNodeReq, srcBufId](IImageBuffer* ptr)
    {
        pNodeReq->releaseBuffer(srcBufId);
    });
    //
    //
    const MINT srcImgFmt = pSrcImgBuffer->getImgFormat();
    const MSize srcImgSize = pSrcImgBuffer->getImgSize();
    // query alignment
    MSize alignment = HwStrideAlignment::queryFormatAlignment(fmt);
    android::sp<IIBuffer> dstSmartImgBuffer = pBufferPool->getImageBuffer(srcImgSize, fmt, alignment);
    const MINT dstImgFmt = dstSmartImgBuffer->getImageBuffer()->getImgFormat();
    const MSize dstImgSize = dstSmartImgBuffer->getImageBuffer()->getImgSize();
    //
    if (!YuvNodeHelper::formatConverter(pSrcImgBuffer.get(), dstSmartImgBuffer->getImageBufferPtr()))
    {
        MY_LOGE("failed to execute format convert, name:%s src:%p, dst:%p", name, pSrcImgBuffer.get(), dstSmartImgBuffer->getImageBufferPtr());
        return NULL;
    }
    return make_shared<PluginSmartBufferHandle>(name, dstSmartImgBuffer);
}

PluginSmartBufferHandlePtr
YuvNodeHelper::
createSmartBuffer(const char* name, MINT fmt, MSize size, const sp<WorkingBufferPool>& pBufferPool)
{
    MY_LOGE("create smart image buffer, name:%s fmt:%s, size:%dx%d, poolAddr:%p",
        name, ImgFmt2Name(fmt),size.w, size.h, pBufferPool.get());

    // query alignment
    MSize alignment = HwStrideAlignment::queryFormatAlignment(fmt);
    android::sp<IIBuffer> smartImgBuffer = pBufferPool->getImageBuffer(size, fmt, alignment);
    return createSmartBuffer(name, smartImgBuffer);
}

PluginSmartBufferHandlePtr
YuvNodeHelper::
createSmartBuffer(const char* name, const android::sp<IIBuffer>& smartImgBuffer)
{
    return make_shared<PluginSmartBufferHandle>(name, smartImgBuffer);
}
#endif

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  YUVNode Implementation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
YUVNode::YUVNode(NodeID_T nid, const char *name, MINT32 policy, MINT32 priority, MBOOL hasTwinNodes)
    :FeatureNode(nid, name, policy, priority)
    , mpNegotiatedCacher(new NegotiatedCacher())
    , mHasTwinNodes(hasTwinNodes)
{
    TRACE_FUNC_ENTER();
    MY_LOGI("(%p) ctor", this);
    this->addWaitQueue(&mRequests);
    TRACE_FUNC_EXIT();
}

YUVNode::~YUVNode()
{
    TRACE_FUNC_ENTER();
    MY_LOGI("(%p) dtor", this);
    TRACE_FUNC_EXIT();
}

MVOID YUVNode::setBufferPool(const android::sp<WorkingBufferPool> &pool)
{
    TRACE_FUNC_ENTER();
    mpBufferPool = pool;
    TRACE_FUNC_EXIT();
}

MBOOL YUVNode::onData(DataID id, const RequestPtr& pRequest)
{
    TRACE_FUNC_ENTER();
    MY_LOGD_IF(mLogLevel, "R/T:%d/%d I/C:%d/%d %s arrived",
                pRequest->getRequestNo(), pRequest->getTimestamp(),
                pRequest->getActiveFrameIndex(), pRequest->getActiveFrameCount(),
                PathID2Name(id));
    MBOOL ret = MTRUE;

    if (pRequest->isReadyFor(mNodeId)) {
        pRequest->addParameter(PID_REQUEST_REPEAT, 0);
        MY_LOGI("ready for yuv, enque!");
        mRequests.enque(pRequest);
    }
    else {
        MY_LOGI("not ready for yuv");
    }

    TRACE_FUNC_EXIT();
    return ret;
}

MBOOL YUVNode::onInit()
{
    TRACE_FUNC_ENTER();
    FeatureNode::onInit();
    MY_LOGI("(%p)(%d) uniqueKey:%d", this, mNodeId, mUsageHint.mPluginUniqueKey);
    mSensorIndex =-1;
    mSensorIndex2 = -1;

    mPlugin = YuvPlugin::getInstance(mUsageHint.mPluginUniqueKey, {mSensorIndex, mSensorIndex2});

    FeatureID_T featId;
    auto& vpProviders = mPlugin->getProviders();
    mpInterface = mPlugin->getInterface();

    std::vector<ProviderPtr> vSortedProvider = vpProviders;

    std::sort(vSortedProvider.begin(), vSortedProvider.end(),
            [] (const ProviderPtr& p1, const ProviderPtr& p2) {
                return p1->property().mPosition < p2->property().mPosition ||
                       p1->property().mPriority > p2->property().mPriority;
            });

    for (auto& pProvider : vSortedProvider) {
        const YuvPlugin::Property& rProperty =  pProvider->property();
        featId = NULL_FEATURE;

        if (mHasTwinNodes) {
            if (mNodeId == NID_YUV && rProperty.mPosition != 0)
                continue;
            /*
            if (mNodeId == NID_YUV2 && rProperty.mPosition != 1)
                continue;
            */
        }

        if (rProperty.mFeatures & MTK_FEATURE_NR)
            featId = FID_NR;
        else if (rProperty.mFeatures & MTK_FEATURE_FB)
            featId = FID_FB;
        else if (rProperty.mFeatures & MTK_FEATURE_ABF)
            featId = FID_ABF;
        else if (rProperty.mFeatures & TP_FEATURE_FB)
            featId = FID_FB_3RD_PARTY;
        else if (rProperty.mFeatures & MTK_FEATURE_AINR_YUV)
            featId = FID_AINR_YUV;
        else if (rProperty.mFeatures & TP_FEATURE_RELIGHTING)
            featId = FID_RELIGHTING_3RD_PARTY;
        else if(rProperty.mFeatures & MTK_FEATURE_YHDR_FOR_AINR)
            featId = FID_AINR_YHDR;
        else if(rProperty.mFeatures & MTK_FEATURE_FILTER_PREVIEW)
            featId = FID_FILTER_PREVIEW;
        else if(rProperty.mFeatures & MTK_FEATURE_FILTER_CAPTURE)
            featId = FID_FILTER_CAPTURE;
        else if(rProperty.mFeatures & MTK_FEATURE_FILTER_MATRIX)
            featId = FID_FILTER_MATRIX;
        else if(rProperty.mFeatures & MTK_FEATURE_PORTRAIT)
            featId = FID_PORTRAIT;
        else if (rProperty.mFeatures & MTK_FEATURE_AUTORAMA)
            featId = FID_AUTOPANORAMA;
        else if (rProperty.mFeatures & MTK_FEATURE_VFB_PREVIEW)
            featId = FID_VFB_PREVIEW;
        else if (rProperty.mFeatures & MTK_FEATURE_VFB_CAPTURE)
            featId = FID_VFB_CAPTURE;
        if (featId != NULL_FEATURE) {
            MY_LOGD_IF(mLogLevel, "%s finds plugin:%s, priority:%d",
                    NodeID2Name(mNodeId), FeatID2Name(featId), rProperty.mPriority);
            auto& item = mProviderPairs.editItemAt(mProviderPairs.add());
            item.mFeatureId = featId;
            item.mpProvider = pProvider;

            if (rProperty.mInitPhase == ePhase_OnPipeInit && mInitMap.count(featId) <= 0)
            {
                std::function<void()> func = [featId, pProvider]() {
                    MY_LOGD("Init Plugin: %s on init phase +", FeatID2Name(featId));
                    pProvider->init();
                    MY_LOGD("Init Plugin: %s on init phase -", FeatID2Name(featId));
                };

                mInitMap[featId] = mpTaskQueue->addFutureTask(func);
            }
        }

    }

    mpCallback = make_shared<YuvCallback>(this);
    TRACE_FUNC_EXIT();
    return MTRUE;
}

MBOOL YUVNode::onUninit()
{
    TRACE_FUNC_ENTER();
    MY_LOGI("(%p)(%d) uniqueKey:%d, sensorIndex:%d, sensorIndex2:%d", this, mNodeId, mUsageHint.mPluginUniqueKey, mSensorIndex, mSensorIndex2);
    for (ProviderPair& p : mProviderPairs) {
        ProviderPtr pProvider = p.mpProvider;
        FeatureID_T featId = p.mFeatureId;

        if (mInitMap.count(featId) > 0) {
            if (!mInitFeatures.hasBit(featId)) {
                MY_LOGD("Wait for initilizing + Feature: %s", FeatID2Name(featId));
                mInitMap[featId].wait();
                MY_LOGD("Wait for initilizing - Feature: %s", FeatID2Name(featId));
                mInitFeatures.markBit(featId);
            }
            pProvider->uninit();
        }
    }

    mProviderPairs.clear();
    mInitMap.clear();
    mInitFeatures.clear();
    mpCurProvider = NULL;
    TRACE_FUNC_EXIT();
    return MTRUE;
}

MBOOL YUVNode::onThreadStart()
{
    TRACE_FUNC_ENTER();
    MBOOL ret = MTRUE;
    //YuvNodeHelper::setSensorIdx(mSensorIndex);
    TRACE_FUNC_EXIT();
    return ret;
}

MBOOL YUVNode::onThreadStop()
{
    TRACE_FUNC_ENTER();
    TRACE_FUNC_EXIT();
    return MTRUE;
}


MBOOL YUVNode::onThreadLoop()
{
    TRACE_FUNC_ENTER();
    if (!waitAllQueue()) {
        TRACE_FUNC("Wait all queue exit");
        return MFALSE;
    }

    RequestPtr pRequest;
    if (!mRequests.deque(pRequest)) {
        MY_LOGE("Request deque out of sync");
        return MFALSE;
    } else if (pRequest == NULL) {
        MY_LOGE("Request out of sync");
        return MFALSE;
    }

    onRequestProcess(pRequest);

    TRACE_FUNC_EXIT();
    return MTRUE;

}

MBOOL YUVNode::onRequestRepeat(RequestPtr& pRequest)
{
    MINT32 repeat = pRequest->getParameter(PID_REQUEST_REPEAT);
    repeat++;
    sp<FeatureNodeRequest> pNodeReq = pRequest->getNodeRequest(mNodeId + repeat);

    // no more repeating
    if (pNodeReq == NULL)
        return MFALSE;

    MY_LOGD("onRequestRepeat request:%d, repeat:%d",
            pRequest->getRequestNo(), repeat);

    {
        Mutex::Autolock _l(mPairLock);
        auto it = mRequestPairs.begin();
        for (; it != mRequestPairs.end(); it++) {
            if ((*it).mPipe == pRequest) {
                mRequestPairs.erase(it);
                break;
            }
        }
        if (mRequestPairs.empty()) {
            mpCurProvider = NULL;
        }
    }

    pRequest->addParameter(PID_REQUEST_REPEAT, repeat);

    return MTRUE;
}

MBOOL YUVNode::onRequestProcess(RequestPtr& pRequest)
{

    TRACE_FUNC_ENTER();
    MINT32 repeat = pRequest->getParameter(PID_REQUEST_REPEAT);
    if (repeat == 0)
        incExtThreadDependency();

    NodeID_T nodeId = mNodeId + repeat;

    if (pRequest->isCancelled()) {
        //pRequest->decNodeReference(nodeId);
        onRequestFinish(pRequest);
        return MFALSE;
    }

    MINT32 requestNo = pRequest->getRequestNo();
    MINT32 timestamp = pRequest->getTimestamp();
    FeatureID_T featureId = NULL_FEATURE;
    //CAM_TRACE_FMT_BEGIN("yuv(%d):process|r%df%d",mNodeId, requestNo, timestamp);
    MY_LOGI("(%p)(%d) +, R/T Num: %d/%d", this, mNodeId, requestNo, timestamp);

    sp<FeatureNodeRequest> pNodeReq = pRequest->getNodeRequest(nodeId);
    if (pNodeReq == NULL) {
        MY_LOGE("should not be here if no node request");
        pRequest->addParameter(PID_FAILURE, 1);
        onRequestFinish(pRequest);
        return MFALSE;
    }
    // pick a provider
    ProviderPtr pProvider = NULL;
    for (ProviderPair& p : mProviderPairs) {
        FeatureID_T featId = p.mFeatureId;
        MBOOL bSupportMultiFrame = p.mpProvider->property().mMultiFrame;
        MY_LOGD_IF(mLogLevel, "hasFeature:%d MultiFrame:%d, isMainFrame:%d R/T:%d/%d",
        pRequest->hasFeature(featId), bSupportMultiFrame, pRequest->isMainFrame(), pRequest->getRequestNo(), pRequest->getTimestamp());
        if (pRequest->hasFeature(featId)
            // check the plugin's allow multiframe config
            && (bSupportMultiFrame || pRequest->isMainFrame())
        )
        {
            if (repeat > 0) {
                repeat--;
                continue;
            }
            pProvider = p.mpProvider;
            featureId = featId;
            break;
        }
    }


    if (pProvider == NULL) {
        MY_LOGE("do not execute a plugin");
        dispatch(pRequest);
        return MFALSE;
    }

    if (!mInitFeatures.hasBit(featureId) && (mInitMap.count(featureId) > 0)) {
        MY_LOGD("Wait for initilizing + Feature: %s", FeatID2Name(featureId));
        mInitMap[featureId].wait();
        MY_LOGD("Wait for initilizing - Feature: %s", FeatID2Name(featureId));
        mInitFeatures.markBit(featureId);
    }

#if 0

    PluginSmartBufferHandlePtr iBufferFullHandle = NULL;
    PluginSmartBufferHandlePtr oBufferFullHandle = NULL;
    const MBOOL bExistIBuf = YuvNodeHelper::existNodeBuffer(pNodeReq, TID_MAN_FULL_YUV, INPUT);
    const MBOOL bExistOBuf = YuvNodeHelper::existNodeBuffer(pNodeReq, TID_MAN_FULL_YUV, OUTPUT);
    const MBOOL isInPlace = pProvider->property().mInPlace;
    const MINT iFmt = bExistIBuf ? YuvNodeHelper::getImageFormat(pNodeReq, TID_MAN_FULL_YUV, INPUT) : -1;
    const MINT oFmt = bExistOBuf ? YuvNodeHelper::getImageFormat(pNodeReq, TID_MAN_FULL_YUV, OUTPUT) : -1;
    MY_LOGD("get in/out full buffer format, R/F Num: %d/%d, feature:%s, iFmt:%s, oFmt:%s, isInPlace:%d",
        requestNo, frameNo,
        FeatID2Name(featureId),
        bExistIBuf ? ImgFmt2Name(iFmt) : "no-buffer-exist",
        bExistOBuf ? ImgFmt2Name(oFmt) : "no-buffer-exist",
        isInPlace);
    // get compatible input/output BufferHandle
    {
        // input buffer
        MINT workingIFmt = iFmt;
        if (bExistIBuf && !mpNegotiatedCacher->getIsSupportedInputBufferFmt(featureId, iFmt)
                && mpNegotiatedCacher->getSupportedInputBufferFmt(featureId, workingIFmt)) {
            MY_LOGD("input buffer is not support, iFmt:%s, workingIFmt:%s", ImgFmt2Name(iFmt), ImgFmt2Name(workingIFmt));
            iBufferFullHandle = YuvNodeHelper::createSmartBuffer("INPUT_TID_MAN_FULL_YUV", workingIFmt, pNodeReq, TID_MAN_FULL_YUV, INPUT, mpBufferPool);

            if (YuvNodeHelper::getIsDumpImage(mNodeId)) {
                const String8 inRealBufferName = String8::format("inRealBuffer_%s_%s_%03d", NodeID2Name(mNodeId), FeatID2Name(featureId), repeat);
                pRequest->dump(inRealBufferName.string(), mNodeId, TID_MAN_FULL_YUV, INPUT);

                const String8 inWorkBufferName = String8::format("inWorkingBuffer_%s_%s_%03d", NodeID2Name(mNodeId), FeatID2Name(featureId), repeat);
                pRequest->dump(inWorkBufferName.string(), mNodeId, iBufferFullHandle->getSmartImageBuffer()->getImageBufferPtr());
            }

            if (isInPlace) {
                oBufferFullHandle = YuvNodeHelper::createSmartBuffer("INPUT_TID_MAN_FULL_YUV_INPLACE", iBufferFullHandle->getSmartImageBuffer());
            }
        }
        //
        MINT workingOFmt = oFmt;
        if (bExistOBuf && !isInPlace) {
            if (!mpNegotiatedCacher->getIsSupportedOutputBufferFmt(featureId, oFmt)) {
                MY_LOGD("output buffer is not support, oFmt:%s", ImgFmt2Name(oFmt));
                if (mpNegotiatedCacher->getIsSupportedOutputBufferFmt(featureId, workingIFmt)) {
                    MY_LOGD("align the output buffer format to input, oFmt:%s -> %s",
                        ImgFmt2Name(oFmt), ImgFmt2Name(workingOFmt));
                    workingOFmt = workingIFmt;
                }
                else if (!mpNegotiatedCacher->getSupportedOutputBufferFmt(featureId, workingOFmt)) {
                    workingOFmt = INVALID_FORMAT;
                }

                if (workingOFmt != INVALID_FORMAT) {
                    MSize oBufferSize = YuvNodeHelper::getImageSize(pNodeReq, TID_MAN_FULL_YUV, OUTPUT);
                    oBufferFullHandle = YuvNodeHelper::createSmartBuffer("OUTPUT_TID_MAN_FULL_YUV_INPLACE", workingOFmt, oBufferSize, mpBufferPool);
                }
            }
        }
        // last, set the callback for copy the result to real output image buffer when handle is releasing
        if (oBufferFullHandle != NULL) {
            PluginBufferHandlePtr realOBufferFullHandle = PluginHelper::CreateBuffer(pNodeReq, TID_MAN_FULL_YUV, OUTPUT);
            auto params = std::make_tuple(pRequest, mNodeId, featureId, repeat);
            auto onReleasingCB = [realOBufferFullHandle, params](IImageBuffer* pImgBuffer)
            {
                const NodeID_T nodeId = std::get<1>(params);
                const FeatureID_T featureId = std::get<2>(params);
                const MINT32 repeat = std::get<3>(params);
                RequestPtr pRequest = std::get<0>(params);

                IImageBuffer* dstImgBuffer = realOBufferFullHandle->acquire();
                if (dstImgBuffer != NULL) {
                    YuvNodeHelper::formatConverter(pImgBuffer, dstImgBuffer);
                    //
                    if (YuvNodeHelper::getIsDumpImage(nodeId)) {
                        const String8 outWorkingName = String8::format("outWorkingBuffer_%s_%s_%03d", NodeID2Name(nodeId), FeatID2Name(featureId), repeat);
                        pRequest->dump(outWorkingName.string(), nodeId, pImgBuffer);
                        const String8 outRealName = String8::format("outRealBuffer_%s_%s_%03d", NodeID2Name(nodeId), FeatID2Name(featureId), repeat);
                        pRequest->dump(outRealName.string(), nodeId, dstImgBuffer);
                    }
                    //
                    realOBufferFullHandle->release();
                }
                else {
                    const MINT32 requestNo = pRequest->getRequestNo();
                    const MINT32 frameNo = pRequest->getFrameNo();
                    MY_LOGE("failed to acquire buffer from output buffer handle, R/F Num:%d/%d node:%s, feature:%s, repret:%d",
                        requestNo, frameNo, NodeID2Name(nodeId), FeatID2Name(featureId), repeat);
                }
            };
            oBufferFullHandle->setOnReleasingCallback(onReleasingCB);
        }
    }
#endif

    auto pPlgRequest = mPlugin->createRequest();

    pPlgRequest->mMainFrame = pRequest->isMainFrame();
    pPlgRequest->mRequestNo = pRequest->getRequestNo();
    pPlgRequest->mFrameNo = 0; //pRequest->getFrameNo();
    pPlgRequest->mIBufferFull  = PluginHelper::CreateBuffer(pNodeReq, TID_MAN_FULL_YUV, INPUT);
    pPlgRequest->mIBufferClean = PluginHelper::CreateBuffer(pNodeReq, TID_MAN_FULL_PURE_YUV, INPUT);
    pPlgRequest->mIBufferDepth = PluginHelper::CreateBuffer(pNodeReq, TID_MAN_DEPTH, INPUT);
    pPlgRequest->mOBufferFull  = PluginHelper::CreateBuffer(pNodeReq, TID_OUT_YUV, OUTPUT);
    pPlgRequest->mOBufferFull0  = PluginHelper::CreateBuffer(pNodeReq, TID_OUT_YUV0, OUTPUT);
    pPlgRequest->mOBufferFull1  = PluginHelper::CreateBuffer(pNodeReq, TID_OUT_YUV1, OUTPUT);
    pPlgRequest->mOBufferFull2  = PluginHelper::CreateBuffer(pNodeReq, TID_OUT_YUV2, OUTPUT);
    pPlgRequest->mOBufferFull3  = PluginHelper::CreateBuffer(pNodeReq, TID_OUT_YUV3, OUTPUT);
    pPlgRequest->mOBufferFull4  = PluginHelper::CreateBuffer(pNodeReq, TID_OUT_YUV4, OUTPUT);
    pPlgRequest->mOBufferFull5  = PluginHelper::CreateBuffer(pNodeReq, TID_OUT_YUV5, OUTPUT);
    pPlgRequest->mOBufferFull6  = PluginHelper::CreateBuffer(pNodeReq, TID_OUT_YUV6, OUTPUT);
    pPlgRequest->mOBufferFull7  = PluginHelper::CreateBuffer(pNodeReq, TID_OUT_YUV7, OUTPUT);

    pPlgRequest->mIMetadataDynamic = PluginHelper::CreateMetadata(pNodeReq, MID_MAN_IN_P1_DYNAMIC);
    pPlgRequest->mIMetadataApp = PluginHelper::CreateMetadata(pNodeReq, MID_MAN_IN_APP);
    pPlgRequest->mIMetadataHal = PluginHelper::CreateMetadata(pNodeReq, MID_MAN_IN_HAL);
    pPlgRequest->mOMetadataApp = PluginHelper::CreateMetadata(pNodeReq, MID_MAN_OUT_APP);
    pPlgRequest->mOMetadataHal = PluginHelper::CreateMetadata(pNodeReq, MID_MAN_OUT_HAL);

    MBOOL ret = MFALSE;
    {
        Mutex::Autolock _l(mPairLock);
        auto& rPair = mRequestPairs.editItemAt(mRequestPairs.add());
        rPair.mPipe = pRequest;
        rPair.mPlugin = pPlgRequest;
        mpCurProvider = pProvider;
    }

    pProvider->process(pPlgRequest, mpCallback);
    ret = MTRUE;

    MY_LOGI("(%p)(%d) -, R/T Num: %d/%d", this, mNodeId, requestNo, timestamp);
    TRACE_FUNC_EXIT();
    return ret;
}


RequestPtr YUVNode::findRequest(PluginRequestPtr& pPlgRequest)
{
    Mutex::Autolock _l(mPairLock);
    for (const auto& rPair : mRequestPairs) {
        if (pPlgRequest == rPair.mPlugin) {
            return rPair.mPipe;
        }
    }

    return NULL;
}

MBOOL YUVNode::onRequestFinish(RequestPtr& pRequest)
{
    MINT32 requestNo = pRequest->getRequestNo();
    MINT32 timestamp = pRequest->getTimestamp();
    //CAM_TRACE_FMT_BEGIN("yuv(%d):finish|r%df%d",mNodeId, requestNo, timestamp);
    MY_LOGI("(%p)(%d), R/T Num: %d/%d", this, mNodeId, requestNo, timestamp);

    {
        Mutex::Autolock _l(mPairLock);
        auto it = mRequestPairs.begin();
        for (; it != mRequestPairs.end(); it++) {
            if ((*it).mPipe == pRequest) {
                mRequestPairs.erase(it);
                break;
            }
        }
        if (mRequestPairs.empty()) {
            mpCurProvider = NULL;
        }
    }

    dispatch(pRequest);

    decExtThreadDependency();
    //CAM_TRACE_FMT_END();
    return MTRUE;
}

MERROR YUVNode::evaluate(NodeID_T nodeId, FeatureInferenceData& rInfer)
{
    (void) nodeId;
    MERROR status = OK;

    // skip not main frame and not yuv process
    if(!rInfer.isMainFrame() && !rInfer.isYUVProcess())
        return OK;

    MBOOL isValid;
    MUINT8 uRepeatCount = 0;

    // Foreach all loaded plugin
    for (ProviderPair& p : mProviderPairs) {
        FeatureID_T featId = p.mFeatureId;

        if (!rInfer.hasFeature(featId)) {
            continue;
        } else if (uRepeatCount >= MAX_YUV_REPEAT_NUM) {
            MY_LOGE("over max repeating count(%d), ignore feature: %s", MAX_YUV_REPEAT_NUM, FeatID2Name(featId));
            continue;
        }

        auto& rSrcData = rInfer.getSharedSrcData();
        auto& rDstData = rInfer.getSharedDstData();
        auto& rFeatures = rInfer.getSharedFeatures();
        auto& rMetadatas = rInfer.getSharedMetadatas();

        isValid = MTRUE;

        ProviderPtr pProvider = p.mpProvider;
        const YuvPlugin::Property& rProperty =  pProvider->property();
        // only allow provider with the MultiFrame property = true
        if(!rProperty.mMultiFrame && !rInfer.isMainFrame())
            continue;

        Selection sel;
        sel.mIsMultiCamVSDoFMode = HasFeatureVSDoF(rInfer);
        mpInterface->offer(sel);
        sel.mMainFrame = rInfer.isMainFrame();

        auto HasFeatureVSDoF = [] (FeatureInferenceData& t) -> bool
        {
            const MBOOL hasDepth = t.hasFeature(FID_DEPTH) || t.hasFeature(FID_DEPTH_3RD_PARTY);
            const MBOOL hasBokeh = t.hasFeature(FID_BOKEH) || t.hasFeature(FID_BOKEH_3RD_PARTY);
            return (hasDepth && hasBokeh) || t.hasFeature(FID_PUREBOKEH_3RD_PARTY);
        };

        if(HasFeatureVSDoF(rInfer)) {
            //sel.mMultiCamFeatureMode = MTK_MULTI_CAM_FEATURE_MODE_VSDOF;
        }

        auto HasFeatureAinr = [] (FeatureInferenceData& t) -> bool
        {
            return t.hasFeature(FID_AINR) || t.hasFeature(FID_AINR_YHDR);
        };

        if(HasFeatureAinr(rInfer)) {
            sel.mIsAinr = MTRUE;
        }

        sel.mIMetadataHal.setControl(rInfer.mpIMetadataHal);
        sel.mIMetadataApp.setControl(rInfer.mpIMetadataApp);
        sel.mIMetadataDynamic.setControl(rInfer.mpIMetadataDynamic);
        if (pProvider->negotiate(sel) != OK) {
            MY_LOGD("bypass %s after negotiation", FeatID2Name(featId));
            rInfer.clearFeature(featId);
            continue;
        }

        MY_LOGD("evaluate featureid:%s", FeatID2Name(featId));

        if (pProvider->property().mInitPhase == ePhase_OnRequest &&
            mInitMap.count(featId) <= 0)
        {
            std::function<void()> func = [featId, pProvider]() {
                MY_LOGD("Init Plugin: %s on evaluate phase +", FeatID2Name(featId));
                pProvider->init();
                MY_LOGD("Init Plugin: %s on evaluate phase -", FeatID2Name(featId));
            };

            mInitMap[featId] = mpTaskQueue->addFutureTask(func);
        }
/*
        MINT iFmt;
        MINT oFmt;
        if (!YuvNodeHelper::tryGetTheSameBufferFmt(sel, iFmt, oFmt))
        {
            iFmt = INVALID_FORMAT;
            oFmt = INVALID_FORMAT;
        }
        MY_LOGD("input and outout format align: iFmt:%s, Fmt:%s",
            iFmt == INVALID_FORMAT ? "invaild" : ImgFmt2Name(iFmt),
            oFmt == INVALID_FORMAT ? "invaild" : ImgFmt2Name(oFmt));
*/

        // full size input
        if (sel.mIBufferFull.getRequired()) {
            if (sel.mIBufferFull.isValid()) {
                auto& src_0 = rSrcData.editItemAt(rSrcData.add());

                src_0.mTypeId = TID_MAN_FULL_YUV;
                if (!rInfer.hasType(TID_MAN_FULL_YUV))
                    isValid = MFALSE;

                src_0.mSizeId = sel.mIBufferFull.getSizes()[0];
                src_0.setAllFmtSupport(MTRUE);
                // although can support all fmt, but still if same format the better
                src_0.addSupportFormats(sel.mIBufferFull.getFormats());

                // In-place processing must add a output
                if (rProperty.mInPlace) {
                    auto& dst_0 = rDstData.editItemAt(rDstData.add());
                    dst_0.mTypeId = TID_MAN_FULL_YUV;
                    dst_0.mSizeId = src_0.mSizeId;
                    dst_0.mSize = rInfer.getSize(TID_MAN_FULL_YUV);
                    dst_0.setAllFmtSupport(MTRUE);
                    // inplace no need to set dst formats
                    dst_0.mInPlace = true;
                }
            } else
                isValid = MFALSE;
        }

        if (sel.mIBufferClean.getRequired()) {
            if (sel.mIBufferClean.isValid()) {
                auto& src_1 = rSrcData.editItemAt(rSrcData.add());

                src_1.mTypeId = TID_MAN_FULL_PURE_YUV;
                if (!rInfer.hasType(TID_MAN_FULL_PURE_YUV))
                    isValid = MFALSE;

                src_1.mSizeId = sel.mIBufferClean.getSizes()[0];
                src_1.addSupportFormats(sel.mIBufferClean.getFormats());

            } else
                isValid = MFALSE;
        }

        if (sel.mIBufferDepth.getRequired()) {
            if (sel.mIBufferDepth.isValid()) {
                auto& src_1 = rSrcData.editItemAt(rSrcData.add());

                src_1.mTypeId = TID_MAN_DEPTH;
                if (!rInfer.hasType(TID_MAN_DEPTH))
                    isValid = MFALSE;

                src_1.mSizeId = sel.mIBufferDepth.getSizes()[0];

            } else
                isValid = MFALSE;
        }

        // face data
        if (rProperty.mFaceData == eFD_Current) {
            auto& src_2 = rSrcData.editItemAt(rSrcData.add());
            src_2.mTypeId = TID_MAN_FD;
            src_2.mSizeId = NULL_SIZE;
            rInfer.markFaceData(eFD_Current);
        }
        else if (rProperty.mFaceData == eFD_Cache) {
            rInfer.markFaceData(eFD_Cache);
        }
        else if (rProperty.mFaceData == eFD_None) {
            rInfer.markFaceData(eFD_None);
        }
        else {
            MY_LOGW("unknow faceDateType:%x", rInfer.mFaceDateType.value);
        }

        // full size output
        if (!rProperty.mInPlace) {
            //
            if (sel.mOBufferFull.getRequired() && sel.mOBufferFull.isValid()) {
                auto& dst_0 = rDstData.editItemAt(rDstData.add());
                dst_0.mTypeId = TID_OUT_YUV;
                dst_0.mSizeId = sel.mOBufferFull.getSizes()[0];
                dst_0.setAllFmtSupport(MTRUE);
                dst_0.addSupportFormats(sel.mOBufferFull.getFormats());
            }
            else {
                isValid = MFALSE;
            }
            if(featId == FID_FILTER_MATRIX) {
                //  0
                if (isValid && sel.mOBufferFull0.getRequired() && sel.mOBufferFull0.isValid()) {
                    auto& dst_0 = rDstData.editItemAt(rDstData.add());
                    dst_0.mTypeId = TID_OUT_YUV0;
                    dst_0.mSizeId = sel.mOBufferFull0.getSizes()[0];
                    dst_0.setAllFmtSupport(MTRUE);
                    dst_0.addSupportFormats(sel.mOBufferFull0.getFormats());
                }
                else {
                    isValid = MFALSE;
                }
                //   1
                if (isValid && sel.mOBufferFull1.getRequired() && sel.mOBufferFull1.isValid()) {
                    auto& dst_0 = rDstData.editItemAt(rDstData.add());
                    dst_0.mTypeId = TID_OUT_YUV1;
                    dst_0.mSizeId = sel.mOBufferFull1.getSizes()[0];
                    dst_0.setAllFmtSupport(MTRUE);
                    dst_0.addSupportFormats(sel.mOBufferFull1.getFormats());
                }
                else {
                    isValid = MFALSE;
                }
                //   2
                if (isValid && sel.mOBufferFull2.getRequired() && sel.mOBufferFull2.isValid()) {
                    auto& dst_0 = rDstData.editItemAt(rDstData.add());
                    dst_0.mTypeId = TID_OUT_YUV2;
                    dst_0.mSizeId = sel.mOBufferFull2.getSizes()[0];
                    dst_0.setAllFmtSupport(MTRUE);
                    dst_0.addSupportFormats(sel.mOBufferFull2.getFormats());
                }
                else {
                    isValid = MFALSE;
                }
                //   3
                if (isValid && sel.mOBufferFull3.getRequired() && sel.mOBufferFull3.isValid()) {
                    auto& dst_0 = rDstData.editItemAt(rDstData.add());
                    dst_0.mTypeId = TID_OUT_YUV3;
                    dst_0.mSizeId = sel.mOBufferFull3.getSizes()[0];
                    dst_0.setAllFmtSupport(MTRUE);
                    dst_0.addSupportFormats(sel.mOBufferFull3.getFormats());
                }
                else {
                    isValid = MFALSE;
                }
                //   4
                if (isValid && sel.mOBufferFull4.getRequired() && sel.mOBufferFull4.isValid()) {
                    auto& dst_0 = rDstData.editItemAt(rDstData.add());
                    dst_0.mTypeId = TID_OUT_YUV4;
                    dst_0.mSizeId = sel.mOBufferFull4.getSizes()[0];
                    dst_0.setAllFmtSupport(MTRUE);
                    dst_0.addSupportFormats(sel.mOBufferFull4.getFormats());
                }
                else {
                    isValid = MFALSE;
                }
                //   5
                if (isValid && sel.mOBufferFull5.getRequired() && sel.mOBufferFull5.isValid()) {
                    auto& dst_0 = rDstData.editItemAt(rDstData.add());
                    dst_0.mTypeId = TID_OUT_YUV5;
                    dst_0.mSizeId = sel.mOBufferFull5.getSizes()[0];
                    dst_0.setAllFmtSupport(MTRUE);
                    dst_0.addSupportFormats(sel.mOBufferFull5.getFormats());
                }
                else {
                    isValid = MFALSE;
                }
                //   6
                if (isValid && sel.mOBufferFull6.getRequired() && sel.mOBufferFull6.isValid()) {
                    auto& dst_0 = rDstData.editItemAt(rDstData.add());
                    dst_0.mTypeId = TID_OUT_YUV6;
                    dst_0.mSizeId = sel.mOBufferFull6.getSizes()[0];
                    dst_0.setAllFmtSupport(MTRUE);
                    dst_0.addSupportFormats(sel.mOBufferFull6.getFormats());
                }
                else {
                    isValid = MFALSE;
                }
                //   7
                if (isValid && sel.mOBufferFull7.getRequired() && sel.mOBufferFull7.isValid()) {
                    auto& dst_0 = rDstData.editItemAt(rDstData.add());
                    dst_0.mTypeId = TID_OUT_YUV7;
                    dst_0.mSizeId = sel.mOBufferFull7.getSizes()[0];
                    dst_0.setAllFmtSupport(MTRUE);
                    dst_0.addSupportFormats(sel.mOBufferFull7.getFormats());
                }
                else {
                    isValid = MFALSE;
                }
            }
        }

        if (sel.mIMetadataDynamic.getRequired())
            rMetadatas.push_back(MID_MAN_IN_P1_DYNAMIC);

        if (sel.mIMetadataApp.getRequired())
            rMetadatas.push_back(MID_MAN_IN_APP);

        if (sel.mIMetadataHal.getRequired())
            rMetadatas.push_back(MID_MAN_IN_HAL);

        if (sel.mOMetadataApp.getRequired())
            rMetadatas.push_back(MID_MAN_OUT_APP);

        if (sel.mOMetadataHal.getRequired())
            rMetadatas.push_back(MID_MAN_OUT_HAL);

        if (isValid) {
            // we need to know what are the provider supported input/output buffer form section, that had negotiated,
            // so we cache the "first" mentioned negotiate result of each provider.
            // please note, cache the "first one" means we ASSERT it will not change in next negotiation.
            if (!mpNegotiatedCacher->getIsFeatureIdExisting(featId)) {
                mpNegotiatedCacher->add(featId, sel.mIBufferFull.getFormats(), sel.mOBufferFull.getFormats());
            }

            rFeatures.push_back(featId);
            // check force yuv
            MBOOL bForceRunYUV = MFALSE;
            if(rInfer.isYUVProcess())
            {
                MY_LOGD("YUV Process request found, force run YUVNode.");
                bForceRunYUV = MTRUE;
            }

            if(!rInfer.addNodeIO(mNodeId + uRepeatCount, rSrcData, rDstData, rMetadatas, rFeatures, bForceRunYUV))
            {
                status = BAD_VALUE;
                break;
            }
            uRepeatCount++;
        } else
            MY_LOGW("%s has an invalid evaluation: %s", NodeID2Name(nodeId), FeatID2Name(featId));
    }

    return status;
}

std::string YUVNode::getStatus(std::string& strDispatch)
{
    //Mutex::Autolock _l(mPairLock);
    String8 str;
    if (mRequestPairs.size() > 0 && mpCurProvider) {
        const YuvPlugin::Property& rProperty = mpCurProvider->property();
        str = String8::format("(name:%s/algo:0x%016" PRIx64 ")", rProperty.mName, rProperty.mFeatures);
        if (strDispatch.size() == 0) {
            String8 strTmp = String8::format(" NOT Finish Provider: %s", rProperty.mName);
            strDispatch = strTmp.string();
        }
    }
    return str.string();
}
} // NSFeaturePipe
} // campostalgo
} // mediatek
} // com

