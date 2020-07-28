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

// Standard C header file
#include <stdarg.h>
#include <chrono>
#include <future>
#include <functional>
// Android system/core header file
#include <cutils/properties.h>
#include <utils/String8.h>
// mtkcam custom header file

// mtkcam global header file

// Module header file
#include <mtkcam/include/mtkcam/utils/metadata/IMetadata.h>
#include <mtkcam/utils/metadata/client/mtk_metadata_tag.h>
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>
#include <mtkcam/drv/iopipe/SImager/IImageTransform.h>
#include <mtkcam/aaa/ICaptureNR.h>
// Local header file
#include "../CaptureFeaturePlugin.h"
#include "DepthNode.h"

// Logging
#define PIPE_CLASS_TAG "DepthNode"
#define PIPE_TRACE TRACE_DEPTH_NODE
#include <featurePipe/core/include/PipeLog.h>
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>

CAM_ULOG_DECLARE_MODULE_ID(MOD_CAPTURE_DEPTH);


/*******************************************************************************
* MACRO Utilities Define.
********************************************************************************/
namespace { // anonymous namespace for MACRO function
using CamThread      = NSCam::NSCamFeature::NSFeaturePipe::CamThread;
using AutoObject     = std::unique_ptr<const char, std::function<void(const char*)>>;
using AutoDependency = std::unique_ptr<CamThread, std::function<MVOID(CamThread*)>>;
//
auto
createAutoScoper(const char* funcName) -> AutoObject
{
    CAM_ULOGMD("[%s] +", funcName);
    return AutoObject(funcName, [](const char* p)
    {
        CAM_ULOGMD("[%s] -", p);
    });
}
#define SCOPED_TRACER() auto scoped_tracer = ::createAutoScoper(__FUNCTION__)
//
auto
createAutoProfiler(const char* funcName, const char* fmt, ...) -> AutoObject
{
    static const MINT32 LENGTH = 512;

    char* pBuf = new char[LENGTH];

    va_list ap;
    va_start(ap, fmt);
    vsnprintf(pBuf, LENGTH, fmt, ap);
    va_end(ap);

    CAM_TRACE_BEGIN(pBuf);
    CAM_ULOGMD("[%s]%s +", funcName, pBuf);

    return AutoObject(pBuf, [funcName](const char* p)
    {
        CAM_ULOGMD("[%s]%s -", funcName, p);
        CAM_TRACE_END();

        delete[] p;
    });
}
#define AUTO_PROFILER(FMT, arg...) auto auto_profiler = ::createAutoProfiler(__FUNCTION__, FMT, ##arg);
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
        CAM_ULOGMD("[%s]%s, elapsed(ms):%.4f", funcName, p, duationTime.count());

        delete[] p;
    });
}
#define AUTO_TIMER(FMT, arg...) auto auto_timer = ::createAutoTimer(__FUNCTION__, FMT, ##arg);
//
#define UNREFERENCED_PARAMETER(param) (param)
#define TO_STRING(val) (#val)
//
} // end anonymous namespace for MACRO function


/*******************************************************************************
* Namespace start.
********************************************************************************/
namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {
namespace NSCapture {


/*******************************************************************************
* Used namespace.
********************************************************************************/
using namespace NSCam::NSPipelinePlugin;
using namespace NSIoPipe::NSSImager;


/*******************************************************************************
* Type Alias.
********************************************************************************/
//
using NodeRequestPtr    = sp<CaptureFeatureNodeRequest>;
//
using PluginPtr         = DepthNode::PluginPtr;
using PipeRequestPtr    = DepthNode::PipeRequestPtr;
using PluginRequestPtr  = DepthNode::PluginRequestPtr;
using BufferHandlePtr   = DepthNode::BufferHandlePtr;
using MetadataHandlePtr = DepthNode::MetadataHandlePtr;
//
using String8 = android::String8;


/*******************************************************************************
* Global Function.
*******************************************************************************/
static inline auto toString(IImageBuffer* pImgBuf) -> std::string;


/*******************************************************************************
* Class Define.
*******************************************************************************/
/**
 * @brief the depth offer interface
 */
class DepthInterface final: public DepthPlugin::IInterface
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  DepthPlugin::IInterface Public Operators.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    auto offer(DepthPlugin::Selection& sel) -> MERROR override;

};
REGISTER_PLUGIN_INTERFACE(Depth, DepthInterface);
/**
 * @brief the depth offer interface
 */
class SwnrProcessor final
{
public:
    using Ptr = UniquePtr<SwnrProcessor>;

public:
    static inline auto createInstance(const std::string& processName, MINT32 openId) -> Ptr;

public:
    auto execute(IImageBuffer* pImgBuf, IMetadata* pIMetadata, sp<CaptureBufferPool> poolPtr) -> MERROR;

    auto waitExecuteDone() -> MERROR;

    ~SwnrProcessor();

private:
    SwnrProcessor(const std::string& name, MINT32 openId);

    SwnrProcessor(const SwnrProcessor& other) = delete;

private:
    auto onExecute(IImageBuffer* pImgBuf, IMetadata* pIMetadata, sp<CaptureBufferPool> poolPtr) -> MERROR;

private:
    using ImageTransformPtr  = UniquePtr<IImageTransform>;
    using ISwNRPtr           = UniquePtr<ISwNR>;

private:
    static inline auto getIsoValue(IMetadata* pIMetadata) -> MINT32;

    static inline auto createImageISwNRPtr(MINT32 openId) -> ISwNRPtr;

    static inline auto createImageTransformPtr() -> ImageTransformPtr;

    static inline auto imgCopy(IImageBuffer* pSrcImg, IImageBuffer* pDstImg) -> MBOOL;

private:
    const std::string       mProcessName;
    const MINT32            mOpenId;
    //
    std::future<MERROR>     mFuture;
    MBOOL                   mEnableDump;

private:
    // SWNR need this type of image format
    static const MUINT32 sSupportedImgFmt = eImgFmt_YV12;
};
/**
 * @brief utility class of the depth node
 */
class DepthNodeUtility final
{
public:
    DepthNodeUtility() = delete;

public:
    static auto createPluginRequestPtr(NodeID_T nodeId, MINT32 sensorIndex, const vector<MINT>& inputFmts, sp<CaptureBufferPool> bufPool, PluginPtr& pluginPtr, PipeRequestPtr& pipeRequestPtr) -> PluginRequestPtr;

    static auto dump(RequestPtr& pipeRequestPtr, NodeID_T nodeId, TypeID_T typeId, Direction dir, const char* fileName) -> MBOOL;

    static auto dump(IImageBuffer* pImgBuf, const char* fileName, MINT64 uniqueKey) -> MBOOL;

    static auto dump(IImageBuffer* pImgBuf, const char* fileName, MINT32 reqNum, MINT32 frameNum) -> MBOOL;

    static auto tryGetFeatureWarning(RequestPtr& pipeRequestPtr, MINT32& featureWarning) -> MBOOL;

    static auto getIsForceByPassFlow() -> MBOOL;

    static auto processNextCapture(const RequestPtr& pipeRequestPtr, MUINT32 timing) -> MVOID;

private:
    static auto dump(IImageBuffer* pImgBuf, const char* relatedFilePath, const char* fileName) -> MBOOL;
};


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Global Function Implementation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
auto
toString(IImageBuffer* pImgBuf) -> std::string
{
    String8 ret = String8::format("[a:%p, si:%dx%d, st:%zu, f:%#09x, va:%p]",
        pImgBuf,
        pImgBuf->getImgSize().w, pImgBuf->getImgSize().h,
        pImgBuf->getBufStridesInBytes(0),
        pImgBuf->getImgFormat(),
        reinterpret_cast<void*>(pImgBuf->getBufVA(0)));
    return std::string(ret.string());
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  DepthInterface Implementation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
auto
DepthInterface::
offer(DepthPlugin::Selection& sel) -> MERROR
{
    sel.mIBufferFull
        .addSupportFormat(eImgFmt_NV21)
        .addSupportFormat(eImgFmt_NV12)
        .addSupportFormat(eImgFmt_I422)
        .addSupportSize(eImgSize_Full);

    sel.mIBufferFull2
        .addSupportFormat(eImgFmt_NV21)
        .addSupportFormat(eImgFmt_NV12)
        .addSupportFormat(eImgFmt_I422)
        .addSupportSize(eImgSize_Full);

    sel.mIBufferResized
        .addSupportFormat(eImgFmt_NV21)
        .addSupportFormat(eImgFmt_NV12)
        .addSupportFormat(eImgFmt_I422)
        .addSupportSize(eImgSize_Resized);

    sel.mIBufferResized2
        .addSupportFormat(eImgFmt_NV21)
        .addSupportFormat(eImgFmt_NV12)
        .addSupportFormat(eImgFmt_I422)
        .addSupportSize(eImgSize_Resized);

    sel.mOBufferDepth
        .addSupportFormat(eImgFmt_Y8)
        .addSupportSize(eImgSize_Specified);

    sel.mOBufferThumbnail
        .addSupportSize(eImgSize_Arbitrary);

    return OK;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  SwnrProcessor Implementation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
auto
SwnrProcessor::
createInstance(const std::string& processName, MINT32 openId) -> Ptr
{
    return Ptr(new SwnrProcessor(processName, openId), [](SwnrProcessor *p) -> MVOID
    {
        delete p;
    });
}

SwnrProcessor::
SwnrProcessor(const std::string& processName, MINT32 openId)
: mProcessName(processName)
, mOpenId(openId)
, mEnableDump(0)
{
    mEnableDump = ::property_get_int32("vendor.debug.camera.depthnode.preprocess.dump", mEnableDump);
    MY_LOGD("ctor, processName:%s, openId:%d, enableDump:%d, addr:%p",
        mProcessName.c_str(), mOpenId, mEnableDump, this);
}

SwnrProcessor::
~SwnrProcessor()
{
    MY_LOGD("dtor, processName:%s, openId:%d, addr:%p", mProcessName.c_str(), mOpenId, this);
    if ( mFuture.valid() )
    {
        MY_LOGW("some process is executing, wait done, processName:%s", mProcessName.c_str());
        MERROR ret = mFuture.get();
        MY_LOGW("legacy executed process wait done, processName:%s, ret:%d", mProcessName.c_str(), ret);
    }
}

auto
SwnrProcessor::
execute(IImageBuffer* pImgBuf, IMetadata* pIMetadata, sp<CaptureBufferPool> poolPtr) -> MERROR
{
    if ( mFuture.valid() )
    {
        MY_LOGW("please check, some process is executing, processName:%s", mProcessName.c_str());
        return INVALID_OPERATION;
    }
    //
    static MBOOL isSyncMode = MTRUE;
    //
    auto task = std::bind(&SwnrProcessor::onExecute, this, pImgBuf, pIMetadata, poolPtr);
    const std::launch lunchPolicy = (isSyncMode ? std::launch::async : std::launch::deferred);
    mFuture = std::async(lunchPolicy, task);
    return OK;
}

auto
SwnrProcessor::
waitExecuteDone() -> MERROR
{
    if ( !mFuture.valid() )
    {
        MY_LOGW("please check, there is no executed process to be waited, processName:%s", mProcessName.c_str());
        return INVALID_OPERATION;
    }
    return mFuture.get();
}

auto
SwnrProcessor::
onExecute(IImageBuffer* pImgBuf, IMetadata* pIMetadata, sp<CaptureBufferPool> poolPtr) -> MERROR
{
    SCOPED_TRACER();

    AUTO_PROFILER("preprocess swnr, processName:%s", mProcessName.c_str());
    AUTO_TIMER("profile swnr, processName:%s", mProcessName.c_str());

    const MSize imgSize = pImgBuf->getImgSize();
    const MINT inputImgFmt = pImgBuf->getImgFormat();
    const MBOOL isUsingWorkingBuf = (inputImgFmt != sSupportedImgFmt);
    // TODO: determine is there data race in modifying input buffers
    static MBOOL isForceUseWorkingBuf = MTRUE;
    MY_LOGD("decide buffer used, processName:%s, isForceUseWorkingBuf:%d, isUsingWorkingBuf:%d, inputImgFmt:%#09x, desirdImgFmt:%#09x",
        mProcessName.c_str(), isForceUseWorkingBuf, isUsingWorkingBuf, inputImgFmt, sSupportedImgFmt);
    //
    const MINT64 uniqueKey = reinterpret_cast<MINT64>(pImgBuf);
    if ( mEnableDump )
    {
        const String8 dumpName = String8::format("before_swnr_%s", mProcessName.c_str());
        DepthNodeUtility::dump(pImgBuf, dumpName.string(), uniqueKey);
    }
    //
    android::sp<IIBuffer> workingImgBufPtr = nullptr;
    if ( isForceUseWorkingBuf || isUsingWorkingBuf )
    {
        workingImgBufPtr = poolPtr->getImageBuffer(imgSize, sSupportedImgFmt);
        if ( workingImgBufPtr == nullptr )
        {
            MY_LOGW("failed to get workingBuf, processName:%s, fmt:%#09x", mProcessName.c_str(), inputImgFmt);
            return BAD_VALUE;
        }

        if ( !imgCopy(pImgBuf, workingImgBufPtr->getImageBufferPtr()) )
        {
            MY_LOGW("failed to copy image(inputBuf to workingBuf), processName:%s", mProcessName.c_str());
            return UNKNOWN_ERROR;
        }
    }
    // execute swnr
    {
        const MINT32 isoValue = getIsoValue(pIMetadata);
        //
        ISwNRPtr swnrPtr = createImageISwNRPtr(mOpenId);
        // TODO: let the swnrParam is correct
        ISwNR::SWNRParam swnrParam;
        swnrParam.isDualCam = MTRUE,
        swnrParam.iso = isoValue,
        swnrParam.isMfll = MFALSE;
        //
        IImageBuffer* pTmpImgBuf = (isUsingWorkingBuf ? workingImgBufPtr->getImageBufferPtr() : pImgBuf);
        MY_LOGW("execute swnr, processName:%s, iso:%d", mProcessName.c_str(), isoValue);
        if ( !swnrPtr->doSwNR(swnrParam, pTmpImgBuf) )
        {
             MY_LOGW("failed to execute swnr, processName:%s", mProcessName.c_str());
             return UNKNOWN_ERROR;
        }
    }
    //
    if ( isUsingWorkingBuf && !imgCopy(workingImgBufPtr->getImageBufferPtr(), pImgBuf) )
    {
        MY_LOGW("failed to copy image(workingBuf to inputBuf), processName:%s", mProcessName.c_str());
        return UNKNOWN_ERROR;
    }
    //
    if ( mEnableDump )
    {
        const String8 dumpName = String8::format("after_swnr_%s", mProcessName.c_str());
        DepthNodeUtility::dump(pImgBuf, dumpName.string(), uniqueKey);
    }
    return OK;
}

auto
SwnrProcessor::
getIsoValue(IMetadata* pMetaData) -> MINT32
{
    static const MINT32 defaultIsoValue = 100;
    IMetadata exifMeta;
    if ( !IMetadata::getEntry<IMetadata>(pMetaData, MTK_3A_EXIF_METADATA, exifMeta) )
    {
        MY_LOGW("failed to get 3A exif, tag:%d(%s)", MTK_3A_EXIF_METADATA, TO_STRING(MTK_3A_EXIF_METADATA));
        return defaultIsoValue;
    }
    //
    MINT32 ret = defaultIsoValue;
    if ( !IMetadata::getEntry<MINT32>(&exifMeta, MTK_3A_EXIF_AE_ISO_SPEED, ret) )
    {
        MY_LOGW("failed to get iso, tag:%d(%s)", MTK_3A_EXIF_AE_ISO_SPEED, TO_STRING(MTK_3A_EXIF_AE_ISO_SPEED));
        return defaultIsoValue;
    }
    return ret;
}

auto
SwnrProcessor::
createImageISwNRPtr(MINT32 openId) -> ISwNRPtr
{
    return ISwNRPtr(MAKE_SwNR(openId), [](ISwNR *p)
    {
        delete p;
    });
}

auto
DepthNodeUtility::
tryGetFeatureWarning(RequestPtr& pipeRequestPtr, MINT32& featureWarning) -> MBOOL
{
    const static MINT32 defaultValue = 0;
    featureWarning = defaultValue;
    if( ::property_get_int32("vendor.debug.camera.vsdof.disable.featurnWarning", 0) != 0 )
    {
        MY_LOGD("force disable feature warning");
        return MTRUE;
    }
    //
    MINT32 featureWarningFromP1 = 0;
    {
        sp<MetadataHandle> inDynamicMetaPtr = pipeRequestPtr->getMetadata(MID_MAN_IN_P1_DYNAMIC);
        IMetadata* pInDynamicMeta = (inDynamicMetaPtr != nullptr) ? inDynamicMetaPtr->native() : nullptr;
        if( pInDynamicMeta == nullptr )
        {
            MY_LOGW("failed to get featureWarning, in-dynamic metadata is not existing");
            return MFALSE;
        }

        IMetadata::IEntry entry = pInDynamicMeta->entryFor(MTK_STEREO_FEATURE_WARNING);
        if( entry.isEmpty() )
        {
            MY_LOGW("failed to get metadata from in-dynamic, tag:%#09x(%s), and set as default value:%d",
                MTK_STEREO_FEATURE_WARNING, TO_STRING(MTK_STEREO_FEATURE_WARNING), defaultValue);
            return MFALSE;
        }
        featureWarningFromP1 = entry.itemAt(0, Type2Type<MINT32>());
    }
    featureWarning |= featureWarningFromP1;
    //
    MINT32 featureWarningFromApp = 0;
    {
        sp<MetadataHandle> inAppMetaPtr = pipeRequestPtr->getMetadata(MID_MAN_IN_APP);
        IMetadata* pInAppMeta = (inAppMetaPtr != nullptr) ? inAppMetaPtr->native() : nullptr;
        if( pInAppMeta == nullptr )
        {
            MY_LOGW("failed to get featureWarning, in-app metadata is not existing");
            return MFALSE;
        }

        IMetadata::IEntry entry = pInAppMeta->entryFor(MTK_VSDOF_FEATURE_CAPTURE_WARNING_MSG);
        if( entry.isEmpty() )
        {
            MY_LOGW("failed to get metadata from in-app, tag:%#09x(%s), and set as default value:%d",
                MTK_VSDOF_FEATURE_CAPTURE_WARNING_MSG, TO_STRING(MTK_VSDOF_FEATURE_CAPTURE_WARNING_MSG), defaultValue);
            return MFALSE;
        }
        featureWarningFromApp = entry.itemAt(0, Type2Type<MINT32>());
    }
    MY_LOGD("get featureWarning, featureWarningFromP1:%#09x, featureWarningFromApp:%#09x", featureWarningFromP1, featureWarningFromApp);
    featureWarning |= featureWarningFromApp;
    return MTRUE;
}

auto
SwnrProcessor::
createImageTransformPtr() -> ImageTransformPtr
{
    return ImageTransformPtr(IImageTransform::createInstance(), [](IImageTransform *p) -> MVOID
    {
        p->destroyInstance();
    });
}

auto
SwnrProcessor::
imgCopy(IImageBuffer* pSrcImg, IImageBuffer* pDstImg) -> MBOOL
{
    MY_LOGD("image copy, srcImgInfo:%s, dstImgInfo:%s", toString(pSrcImg).c_str(), toString(pDstImg).c_str());
    ImageTransformPtr imageTransformPtr = createImageTransformPtr();
    if ( imageTransformPtr == nullptr )
    {
        MY_LOGW("failed to get imageTransform");
        return MFALSE;
    }
    //
    const MSize imgSize = pSrcImg->getImgSize();
    const MRect cropRect = MRect(imgSize.w, imgSize.h);
    if ( !imageTransformPtr->execute(pSrcImg, pDstImg, nullptr, cropRect, 0, 3000) )
    {
        MY_LOGW("failed to execute image transform, srcImg:%p, dstImg:%p", pSrcImg, pDstImg);
        return MFALSE;
    }
    return MTRUE;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  DepthNodeUtility Implementation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
auto
DepthNodeUtility::
createPluginRequestPtr(NodeID_T nodeId, MINT32 sensorIndex, const vector<MINT>& inputFmts, sp<CaptureBufferPool> bufPool, PluginPtr& pluginPtr, PipeRequestPtr& pipeRequestPtr) -> PluginRequestPtr
{
    NodeRequestPtr nodeRequestPtr = pipeRequestPtr->getNodeRequest(nodeId);
    //
    const BufferID_T mainBufId = nodeRequestPtr->mapBufferID(TID_MAN_FULL_YUV, INPUT);
    const BufferID_T subBufId = nodeRequestPtr->mapBufferID(TID_SUB_FULL_YUV, INPUT);
    const IImageBuffer* pMainImgBuf = nodeRequestPtr->acquireBuffer(mainBufId);
    const IImageBuffer* pSubImgBuf = nodeRequestPtr->acquireBuffer(subBufId);
    const MINT mainFmt = pMainImgBuf->getImgFormat();
    const MINT subFmt = pSubImgBuf->getImgFormat();
    PluginSmartBufferHandlePtr mainBufHadlePtr = nullptr;
    if ( mainFmt != subFmt )
    {
        if ( std::find(inputFmts.begin(), inputFmts.end(), mainFmt) == inputFmts.end() )
        {
            MY_LOGW("cannot align the main and sub input buffer format, main:%s, sub%s", ImgFmt2Name(mainFmt), ImgFmt2Name(subFmt));
        }
        MY_LOGD("convert main input buffer format as sub, main:%s -> %s", ImgFmt2Name(mainFmt), ImgFmt2Name(subFmt));

        mainBufHadlePtr = PluginSmartBufferHandle::createInstance("TID_MAN_FULL_YUV_COMVERTED", subFmt, sensorIndex, pMainImgBuf, bufPool);
        if ( getIsDumpImage<NID_DEPTH>() )
        {
            pipeRequestPtr->dump("inMainWorkingBuf", nodeId, mainBufHadlePtr->getSmartImageBuffer()->getImageBufferPtr());
        }
        nodeRequestPtr->releaseBuffer(mainBufId);
    }
    //
    PluginRequestPtr ret = pluginPtr->createRequest();
    {
        // in/out buffer
        ret->mIBufferFull       = (mainBufHadlePtr == nullptr) ? PluginHelper::CreateBuffer(nodeRequestPtr, TID_MAN_FULL_YUV, INPUT) : std::move(mainBufHadlePtr);
        ret->mIBufferFull2      = PluginHelper::CreateBuffer(nodeRequestPtr, TID_SUB_FULL_YUV, INPUT);
        ret->mIBufferResized    = PluginHelper::CreateBuffer(nodeRequestPtr, TID_MAN_RSZ_YUV, INPUT);
        ret->mIBufferResized2   = PluginHelper::CreateBuffer(nodeRequestPtr, TID_SUB_RSZ_YUV, INPUT);
        ret->mIBufferLCS        = PluginHelper::CreateBuffer(nodeRequestPtr, TID_MAN_LCS, INPUT);
        ret->mIBufferLCS2       = PluginHelper::CreateBuffer(nodeRequestPtr, TID_SUB_LCS, INPUT);
        ret->mOBufferDepth      = PluginHelper::CreateBuffer(nodeRequestPtr, TID_MAN_DEPTH, OUTPUT);
        ret->mOBufferThumbnail   = PluginHelper::CreateBuffer(nodeRequestPtr, TID_POSTVIEW, OUTPUT);
        // in/out metadata
        ret->mIMetadataHal      = PluginHelper::CreateMetadata(nodeRequestPtr, MID_MAN_IN_HAL);
        ret->mIMetadataHal2     = PluginHelper::CreateMetadata(nodeRequestPtr, MID_SUB_IN_HAL);
        ret->mIMetadataApp      = PluginHelper::CreateMetadata(nodeRequestPtr, MID_MAN_IN_APP);
        ret->mIMetadataDynamic  = PluginHelper::CreateMetadata(nodeRequestPtr, MID_MAN_IN_P1_DYNAMIC);
        ret->mIMetadataDynamic2 = PluginHelper::CreateMetadata(nodeRequestPtr, MID_SUB_IN_P1_DYNAMIC);
        ret->mOMetadataApp      = PluginHelper::CreateMetadata(nodeRequestPtr, MID_MAN_OUT_APP);
        ret->mOMetadataHal      = PluginHelper::CreateMetadata(nodeRequestPtr, MID_MAN_OUT_HAL);
        // main sensorIndex
        ret->mSensorId          = sensorIndex;
    }
    return ret;
}

auto
DepthNodeUtility::
dump(RequestPtr& pipeRequestPtr, NodeID_T nodeId, TypeID_T typeId, Direction dir, const char* fileName) -> MBOOL
{
    MBOOL ret = MFALSE;
    NodeRequestPtr nodeRequestPtr = pipeRequestPtr->getNodeRequest(nodeId);
    const BufferID_T bufId = nodeRequestPtr->mapBufferID(typeId, dir);
    if ( bufId != NULL_BUFFER )
    {
        const MINT32 reqNum = pipeRequestPtr->getRequestNo();
        const MINT32 frameNum = pipeRequestPtr->getFrameNo();
        sp<BufferHandle> bufferHandle = pipeRequestPtr->getBuffer(bufId);
        IImageBuffer* pImgBuf = bufferHandle->native();
        ret = dump(pImgBuf, fileName, reqNum, frameNum);
    }
    return ret;
}

auto
DepthNodeUtility::
dump(IImageBuffer* pImgBuf, const char* fileName, MINT32 reqNum, MINT32 frameNum) -> MBOOL
{
     const String8 relatedFilePath = String8::format("%08d_%08d", reqNum, frameNum);
     return dump(pImgBuf, relatedFilePath.string(), fileName);
}

auto
DepthNodeUtility::
dump(IImageBuffer* pImgBuf, const char* fileName, MINT64 uniqueKey) -> MBOOL
{
    const String8 relatedFilePath = String8::format("%" PRId64, uniqueKey);
    return dump(pImgBuf, relatedFilePath.string(), fileName);
}

auto
DepthNodeUtility::
dump(IImageBuffer* pImgBuf, const char* relatedFilePath, const char* fileName) -> MBOOL
{
    const MSize imgSize = pImgBuf->getImgSize();
    const size_t imgStrides = pImgBuf->getBufStridesInBytes(0);
    const MINT imgFmt = pImgBuf->getImgFormat();
    MY_LOGD("dump image info, relatedFilePath:%s, fileName:%s, addr:%p, size:(%dx%d), fmt:%#09x, stride:%zu",
        relatedFilePath, fileName, pImgBuf, imgSize.w, imgSize.h, imgFmt, imgStrides);

    {
        // TODO: support image format for file extension
    }

    static const MINT32 writePermission = 0660;
    static const std::string directoryName = "/sdcard/vsdof/depthNode";

    const String8 directoryPath = String8::format("%s/%s", directoryName.c_str(), relatedFilePath);
    const MBOOL isSuccessCreateDirectory = NSCam::Utils::makePath(directoryPath.string(), writePermission);
    if ( !isSuccessCreateDirectory )
    {
        MY_LOGW("failed to create directory, fullFilePath:%s", directoryPath.string());
        return MFALSE;
    }
    //
    const String8 fullFilePath = String8::format("%s/%s_%dx%d_%zu_%#09x.yuv",
        directoryPath.string(), fileName, imgSize.w, imgSize.h, imgStrides, imgFmt);
    pImgBuf->saveToFile(fullFilePath.string());
    MY_LOGD("save image file, fullFilePath:%s", fullFilePath.string());
    return MTRUE;
}

auto
DepthNodeUtility::
getIsForceByPassFlow() -> MBOOL
{
    MUINT8 flowByPass = 0;
    flowByPass = ::property_get_int32("vendor.debug.camera.depthnode.force.flowbypass", flowByPass);
    if( flowByPass != 0 )
    {
        MY_LOGD("force flow bypass, val:%d", flowByPass);
        return MTRUE;
    }
    return MFALSE;
}

auto
DepthNodeUtility::
processNextCapture(const RequestPtr& pipeRequestPtr, MUINT32 timing) -> MVOID
{
    const MBOOL isEnableNextCapture = (pipeRequestPtr->getParameter(PID_ENABLE_NEXT_CAPTURE) > 0);
    const MBOOL hasTiming = pipeRequestPtr->hasParameter(PID_THUMBNAIL_TIMING);
    if (!isEnableNextCapture || !hasTiming)
    {
        MY_LOGD("failed to processor next capure, isEnableNextCapture:%d, hasTiming:%d",
            isEnableNextCapture, hasTiming);
        return;
    }

    if (pipeRequestPtr->getParameter(PID_THUMBNAIL_TIMING) == timing)
    {
        const MINT32 requestNo = pipeRequestPtr->getRequestNo();
        const MINT32 frameNo = pipeRequestPtr->getFrameNo();
        const MINT32 sensorId = pipeRequestPtr->getMainSensorIndex();
        if (pipeRequestPtr->mpCallback != nullptr)
        {
            MY_LOGD("Notify next capture, R/F/S Num: %d/%d/%d", requestNo, frameNo, sensorId);
            pipeRequestPtr->mpCallback->onContinue(pipeRequestPtr);
        }
        else
        {
            MY_LOGW("have no request callback instance!");
        }
    }
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  DepthNode Implementation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
DepthNode::
DepthNode(NodeID_T nodeId, const char* name, MINT32 policy, MINT32 priority)
: CamNodeULogHandler(Utils::ULog::MOD_CAPTURE_DEPTH)
, CaptureFeatureNode(nodeId, name, 0, policy, priority)
// note: we have to use the only one callback object to advoid leak
, mRequestCallbackPtr(this, [](RequestCallback* ptr){ UNREFERENCED_PARAMETER(ptr); })
, mForcePreprocess(0)
{
    mForcePreprocess = ::property_get_int32("vendor.debug.camera.depthnode.force.preprocess", mForcePreprocess);
    this->addWaitQueue(&mRequests);
    MY_LOGD("ctor:%p, forcePreprocess:%d", this, mForcePreprocess);
}

DepthNode::
~DepthNode()
{
    MY_LOGI("(%p) dtor", this);
}

MVOID
DepthNode::
setBufferPool(const sp<CaptureBufferPool> &pool)
{
    mpBufferPool = pool;
    MY_LOGD("set buffer pool, addr:%p", pool.get());
}

MBOOL
DepthNode::
onData(DataID dataId, const RequestPtr& pRequest)
{
    SCOPED_TRACER();

    MY_LOGD("request arrived, req#:%d, dataId:%d(%s)",
        pRequest->getRequestNo(), dataId, ID2Name(dataId));

    if ( pRequest->isReadyFor(mNodeId) )
    {
        mRequests.enque(pRequest);
    }
    else
    {
        MY_LOGD("not ready to enque, request is not satisfied, req#:%d, dataId:%d(%s)",
            pRequest->getRequestNo(), dataId, ID2Name(dataId));
    }
    return MTRUE;
}

MVOID
DepthNode::
onFlush()
{
    SCOPED_TRACER();
    CaptureFeatureNode::onFlush();

    MY_LOGD("extThreadDependency:%d, requestSize:%zu", this->getExtThreadDependency(), mRequests.size());
    RequestPtr pipeRequestPtr;
    while ( mRequests.deque(pipeRequestPtr) )
    {
        // TODO: should we need dispatch the request?
        // dispatch(pipeRequestPtr);
    }
}

MBOOL
DepthNode::
onInit()
{
    SCOPED_TRACER();
    CaptureFeatureNode::onInit();
    MY_LOGI("(%p) uniqueKey:%d", this, mUsageHint.mPluginUniqueKey);

    mPluginPtr = Plugin::getInstance(mUsageHint.mPluginUniqueKey);

    auto& providers = mPluginPtr->getProviders();
    auto interface = mPluginPtr->getInterface();

    const MUINT64 supportedScenarioFeatures = mUsageHint.mSupportedScenarioFeatures;
    for( auto& provider : providers )
    {
        const Property& rProperty =  provider->property();
        if( !(supportedScenarioFeatures & rProperty.mFeatures) )
        {
            MY_LOGD("not in supportedScenarioFeatures(%#012" PRIx64 "), skip the provider, name:%s, feature:%#012" PRIx64,
                supportedScenarioFeatures, rProperty.mName, rProperty.mFeatures);
            continue;
        }

        FeatureID_T featureId = NULL_FEATURE;
        if ( rProperty.mFeatures & MTK_FEATURE_DEPTH )
        {
            featureId = FID_DEPTH;
        }
        else if ( rProperty.mFeatures & TP_FEATURE_DEPTH )
        {
            featureId = FID_DEPTH_3RD_PARTY;
        }
        else
        {
            MY_LOGW("can't match the supported features, skip the provider, name:%s, feature:%#012" PRIx64,
                rProperty.mName, rProperty.mFeatures);
            continue;
        }

        SelectionPtr selectionPtr = mPluginPtr->createSelection();
        Selection& rSelection = *selectionPtr.get();
        interface->offer(rSelection);
        provider->negotiate(rSelection);
        //
        std::launch initPolicy = std::launch::deferred;
        if(rProperty.mInitPhase == ePhase_OnPipeInit)
        {
            initPolicy = std::launch::async;
        }
        //
        auto getFmtStr = [](const std::vector<MINT>& fmts) -> String8
        {
            String8 str;
            for(MINT fmt : fmts)
                str += String8::format("%s ", ImgFmt2Name(fmt));

            return std::move(str);
        };

        std::vector<MINT> mainFmt = rSelection.mIBufferFull.getFormats();
        std::vector<MINT> subFmt = rSelection.mIBufferFull2.getFormats();
        std::sort(mainFmt.begin(), mainFmt.end());
        std::sort(subFmt.begin(), subFmt.end());
        std::vector<MINT> intersectionFmts;
        std::set_intersection(mainFmt.begin(), mainFmt.end(),
                              subFmt.begin(), subFmt.end(),
                              std::back_inserter(intersectionFmts));
        if ( intersectionFmts.size() == 0 )
        {
            MY_LOGW("no intersection of the input buffer format");
        }
        else
        {
            MY_LOGD("intersectionFmts:%s, inputFmts:%s, subFmts:%s",
                getFmtStr(intersectionFmts).string(), getFmtStr(mainFmt).string(), getFmtStr(subFmt).string());
        }
        //
        ProviderInfo providerInfo =
        {
            .mProviderPtr  = provider,
            .mSelectionPtr = selectionPtr,
            .mSupportInputFmts = std::move(intersectionFmts),
            .mInitJob = std::async(initPolicy, [provider, initPolicy]()
            {
                const Property& rProperty = provider->property();
                AUTO_TIMER("init provider, name:%s, features:%#012" PRIx64 ", initPolicy:%d", rProperty.mName, rProperty.mFeatures, initPolicy);
                provider->init();
            })
        };
        mProviderInfoTable.insert(FeaturePairItem(featureId, std::move(providerInfo)));
        MY_LOGD("insert item table, featureId:%d(%s), name:%s, features:%#012" PRIx64 ", initPhase:%d, initPolicy:%d",
                featureId, FeatID2Name(featureId),
                rProperty.mName, rProperty.mFeatures,
                rProperty.mInitPhase, static_cast<MINT32>(initPolicy));
    }
    return MTRUE;
}

MBOOL
DepthNode::
onUninit()
{
    SCOPED_TRACER();
    MY_LOGI("(%p) uniqueKey:%d", this, mUsageHint.mPluginUniqueKey);

    for ( auto& pair : mProviderInfoTable )
    {
        const FeatureID_T featureId = pair.first;
        ProviderInfo& item = pair.second;
        // TODO: we need the supportScenarioFeatures to advoid init non-used provider
        if ( item.mInitJob.valid() )
        {
            const Property& rProperty = item.mProviderPtr->property();
            MY_LOGD("wait for init job completion, featureId:%d(%s), name:%s, features:%#012" PRIx64 ", initPhase:%d",
                featureId, FeatID2Name(featureId), rProperty.mName, rProperty.mFeatures, rProperty.mInitPhase);
            item.mInitJob.get();
        }
        //
        auto& providerPtr = item.mProviderPtr;
        providerPtr = nullptr;
    }
    mProviderInfoTable.clear();

    return MTRUE;
}

MBOOL
DepthNode::
onThreadStart()
{
    return MTRUE;
}

MBOOL
DepthNode::
onThreadStop()
{
    return MTRUE;
}

MBOOL
DepthNode::
onThreadLoop()
{
    SCOPED_TRACER();

    if ( !waitAllQueue() )
    {
        return MFALSE;
    }

    RequestPtr pipeRequestPtr;
    if ( !mRequests.deque(pipeRequestPtr) )
    {
        MY_LOGE("failed to deque");
        return MFALSE;
    }

    if ( pipeRequestPtr == nullptr )
    {
        MY_LOGE("pipeRequestPtr is nullptr");
        return MFALSE;
    }
    //
    const MINT32 reqNum = pipeRequestPtr->getRequestNo();
    //
    auto foundItem = mProviderInfoTable.end();
    MBOOL isFlowByPass = DepthNodeUtility::getIsForceByPassFlow();
    {
        if ( !isFlowByPass )
        {
            MINT32 featureWarning = 0;
            DepthNodeUtility::tryGetFeatureWarning(pipeRequestPtr, featureWarning);
            if ( featureWarning != 0 )
            {
                MY_LOGD("feature warning occur, req#:%d, featureWarning:%d", reqNum, featureWarning);
                isFlowByPass = MTRUE;
            }
        }
        //
        if ( !isFlowByPass )
        {
            auto item = mProviderInfoTable.begin();
            while( item != mProviderInfoTable.end() )
            {
                if( pipeRequestPtr->hasFeature(item->first) )
                {
                    break;
                }
                ++item;
            }
            //
            if ( item != mProviderInfoTable.end() )
            {
                foundItem = item;
            }
            else
            {
                MY_LOGD("failed to find provider, req#:%d", reqNum);
                isFlowByPass = MTRUE;
            }
        }
    }
    //
    if ( isFlowByPass )
    {
        MY_LOGD("something happen and by pass flow, req#:%d", reqNum);
        dispatch(pipeRequestPtr);
    }
    else
    {
        const FeatureID_T featureId = foundItem->first;
        ProviderPtr& providerPtr = foundItem->second.mProviderPtr;
        const Property& rProperty = providerPtr->property();
        //
        InitJob& initJob = foundItem->second.mInitJob;
        if ( initJob.valid() )
        {
            MY_LOGD("wait for init job completion, req#:%d, featureId:%d(%s), name:%s, features:%#012" PRIx64 ", initPhase:%d",
                reqNum, featureId, FeatID2Name(featureId), rProperty.mName, rProperty.mFeatures, rProperty.mInitPhase);
            initJob.get();
        }
        //
        MY_LOGD("execute provider, req#:%d, featureId:%d(%s), name:%s, features:%#012" PRIx64,
            reqNum, featureId, FeatID2Name(featureId), rProperty.mName, rProperty.mFeatures);
        //
        preprocess(providerPtr, pipeRequestPtr);
        process(providerPtr, pipeRequestPtr);
    }
    return MTRUE;
}

MVOID
DepthNode::
process(ProviderPtr& providerPtr, RequestPtr& pipeRequestPtr)
{
    const MINT32 reqNum = pipeRequestPtr->getRequestNo();
    const MINT32 frameNum = pipeRequestPtr->getFrameNo();
    const MINT32 masterId = pipeRequestPtr->getMainSensorIndex();
    AUTO_PROFILER("process depth request, req#:%d, fram#:%d", reqNum, frameNum);
    //
    vector<MINT> inputFmts;
    auto item = mProviderInfoTable.begin();
    while ( item != mProviderInfoTable.end() )
    {
        if ( item->second.mProviderPtr.get() == providerPtr.get() )
        {
            inputFmts = item->second.mSupportInputFmts;
            break;
        }
        ++item;
    }
    //
    auto pluginRequestPtr = DepthNodeUtility::createPluginRequestPtr(getNodeID(), masterId, inputFmts, mpBufferPool, mPluginPtr, pipeRequestPtr);
    {
        std::lock_guard<std::mutex> guard(mRequestPairLock);
        mRequestPairTable.insert(RequestPairItem(pluginRequestPtr.get(),
        {
            .mPipeRequest   = pipeRequestPtr,
            .mPluginRequest = pluginRequestPtr
        }));
        // mark on-going-request start
        this->incExtThreadDependency();
        MY_LOGI("(%p) insert requestPair into table, req#:%d, pluginRequest:%p, dependencyCount:%d",
            this, reqNum, pluginRequestPtr.get(), this->getExtThreadDependency());
    }
    //
    if ( getIsDumpImage<NID_DEPTH>() )
    {
        pipeRequestPtr->dump("mainFSYuv", getNodeID(), TID_MAN_FULL_YUV, INPUT);
        pipeRequestPtr->dump("subFSYuv", getNodeID(), TID_SUB_FULL_YUV, INPUT);
    }
    providerPtr->process(pluginRequestPtr, mRequestCallbackPtr);
}

MVOID
DepthNode::
preprocess(ProviderPtr& providerPtr, RequestPtr& pipeRequestPtr)
{
    const MINT32 reqNum = pipeRequestPtr->getRequestNo();
    AUTO_PROFILER("preprocess depthRequest, req#:%d", reqNum);
    AUTO_TIMER("profile preprocess, req#:%d", reqNum);

    const Property& property = providerPtr->property();
    const MBOOL isExecutSWNR = ((property.mPreprocess == ePreProcess_NR) || (mForcePreprocess & ePreProcess_NR));
    MY_LOGD("swnr preprocess, req#:%d, isExecutSWNR:%d", reqNum, isExecutSWNR);
    if ( isExecutSWNR )
    {
        AUTO_PROFILER("swnr preprocess for depth input, req#:%d", reqNum);
        NodeRequestPtr nodeRequestPtr = pipeRequestPtr->getNodeRequest(getNodeID());
        //
        const BufferID_T mainInputBufId = nodeRequestPtr->mapBufferID(TID_MAN_FULL_YUV, INPUT);
        const BufferID_T subInputBufId = nodeRequestPtr->mapBufferID(TID_SUB_FULL_YUV, INPUT);
        sp<BufferHandle> mianFSYuvImgBufPtr = pipeRequestPtr->getBuffer(mainInputBufId);
        sp<BufferHandle> subFSYuvImgBufPtr = pipeRequestPtr->getBuffer(subInputBufId);
        //
        sp<MetadataHandle> mainHalMetadataPtr = pipeRequestPtr->getMetadata(MID_MAN_IN_HAL);
        sp<MetadataHandle> subHalMetadataPtr = pipeRequestPtr->getMetadata(MID_SUB_IN_HAL);
        const MBOOL readyToPrepareImgBuf = (mianFSYuvImgBufPtr != nullptr) && (subFSYuvImgBufPtr != nullptr);
        const MBOOL readyToPrepareMetaData = (mainHalMetadataPtr != nullptr) && (subHalMetadataPtr != nullptr);
        if ( (!readyToPrepareImgBuf) || (!readyToPrepareMetaData) )
        {
            MY_LOGW("invalid parameters for NR process, req#:%d, mianFSYuvImgBufPtr:%p, subFSYuvImgBufPtr:%p. mainHalMetadataPtr:%p, subHalMetadataPtr,%p",
                reqNum,
                mianFSYuvImgBufPtr.get(), subFSYuvImgBufPtr.get(),
                mainHalMetadataPtr.get(), subHalMetadataPtr.get());
        }
        else
        {
            SwnrProcessor::Ptr mainSwnrProcessorPtr = SwnrProcessor::createInstance("mainFullYuv", pipeRequestPtr->getMainSensorIndex());
            SwnrProcessor::Ptr subSwnrProcessorPtr = SwnrProcessor::createInstance("subFullYuv", pipeRequestPtr->getSubSensorIndex());
            //
            IImageBuffer* pMinFSYuvImgBuf = mianFSYuvImgBufPtr->native();
            IImageBuffer* pSubFSYuvImgBuf = subFSYuvImgBufPtr->native();
            IMetadata* pMainHalMetadata = mainHalMetadataPtr->native();
            IMetadata* pSubHalMetadata = subHalMetadataPtr->native();
            if ( (mainSwnrProcessorPtr->execute(pMinFSYuvImgBuf, pMainHalMetadata, mpBufferPool) != OK)
                || (subSwnrProcessorPtr->execute(pSubFSYuvImgBuf, pSubHalMetadata, mpBufferPool) != OK) )
            {
                 MY_LOGW("failed to execute swnr preprocess, req#:%d", reqNum);
            }
            else
            {
                MERROR mainSwnrRet = mainSwnrProcessorPtr->waitExecuteDone();
                MERROR subSwnrRet = subSwnrProcessorPtr->waitExecuteDone();
                MY_LOGD("swnr preprocess result, req#:%d, mainRet:%d, subRet:%d", reqNum, mainSwnrRet, subSwnrRet);
            }
        }
    }
}

MERROR
DepthNode::
evaluate(NodeID_T nodeId, CaptureFeatureInferenceData& rInfer)
{
    SCOPED_TRACER();

    UNREFERENCED_PARAMETER(nodeId);

    const MINT8 reqtIndex = rInfer.getRequestIndex();
    const MUINT8 reqCount = rInfer.getRequestCount();
    const MBOOL isMainFrame = rInfer.isMainFrame();

    MY_LOGD("evaluate info, reqIndex:%d, reqCount:%u, isMainFrame:%d",
        reqtIndex, reqCount, isMainFrame);

    if ( !isMainFrame )
        return OK;

    auto& rSrcData = rInfer.getSharedSrcData();
    auto& rDstData = rInfer.getSharedDstData();
    auto& rFeatures = rInfer.getSharedFeatures();
    auto& rMetadatas = rInfer.getSharedMetadatas();

    MBOOL isEvaluated = MFALSE;
    MERROR status = OK;
    for (auto& item : mProviderInfoTable)
    {
        const FeatureID_T featureId = item.first;
        if ( !rInfer.hasFeature(featureId) )
        {
            continue;
        }
        else if ( isEvaluated )
        {
            MY_LOGE("duplicated feature:%d(%s)", featureId, FeatID2Name(featureId));
            continue;
        }

        const auto& fmts = item.second.mSupportInputFmts;
        auto setFormat = [&fmts](CaptureFeatureInferenceData::SrcData& data)
        {
            if(fmts.size() > 0)
                data.setFormat(fmts[0]);
        };

        const Selection& rSel = *(item.second.mSelectionPtr);
        MBOOL isOK = MTRUE;

        if ( isOK && rSel.mIBufferFull.getRequired() )
        {
            const TypeID_T typeId = TID_MAN_FULL_YUV;
            const MBOOL hasType = rInfer.hasType(typeId);
            const MBOOL isValid = rSel.mIBufferFull.isValid();

            isOK = (isValid && hasType);
            if ( isOK )
            {
                auto& srcTemp = rSrcData.editItemAt(rSrcData.add());
                srcTemp.mTypeId = typeId;
                srcTemp.mSizeId = rSel.mIBufferFull.getSizes()[0];
                setFormat(srcTemp);
                srcTemp.addSupportFormats(rSel.mIBufferFull.getFormats());
                srcTemp.setAllFmtSupport(MTRUE);
            }
            else
            {
                MY_LOGW("failed to add src, hasType:%d, typeId:%d(%s), featureId:%d(%s)",
                    hasType, typeId, TypeID2Name(typeId), featureId, FeatID2Name(featureId));
            }
        }

        if ( isOK && rSel.mIBufferFull2.getRequired() )
        {
            const TypeID_T typeId = TID_SUB_FULL_YUV;
            const MBOOL hasType = rInfer.hasType(typeId);
            const MBOOL isValid = rSel.mIBufferFull2.isValid();

            isOK = ( isValid && hasType );
            if ( isOK )
            {
                auto& srcTemp = rSrcData.editItemAt(rSrcData.add());
                srcTemp.mTypeId = typeId;
                srcTemp.mSizeId = rSel.mIBufferFull2.getSizes()[0];
                setFormat(srcTemp);
                srcTemp.addSupportFormats(rSel.mIBufferFull2.getFormats());
            }
            else
            {
                MY_LOGW("failed to add src, hasType:%d, typeId:%d(%s), featureId:%d(%s)",
                    hasType, typeId, TypeID2Name(typeId), featureId, FeatID2Name(featureId));
            }
        }

        if ( isOK && rSel.mIBufferResized.getRequired() )
        {
            const TypeID_T typeId =  TID_MAN_RSZ_YUV;
            const MBOOL hasType = rInfer.hasType(typeId);
            const MBOOL isValid = rSel.mIBufferResized.isValid();

            isOK = ( isValid && hasType );
            if ( isOK )
            {
                auto& srcTemp = rSrcData.editItemAt(rSrcData.add());
                srcTemp.mTypeId = typeId;
                srcTemp.mSizeId = rSel.mIBufferResized.getSizes()[0];
                srcTemp.addSupportFormats(rSel.mIBufferResized.getFormats());
            }
            else
            {
                MY_LOGW("failed to add src, hasType:%d, typeId:%d(%s), featureId:%d(%s)",
                    hasType, typeId, TypeID2Name(typeId), featureId, FeatID2Name(featureId));
            }
        }

        if ( isOK && rSel.mIBufferResized2.getRequired() )
        {
            const TypeID_T typeId = TID_SUB_RSZ_YUV;
            const MBOOL hasType = rInfer.hasType(typeId);
            const MBOOL isValid = rSel.mIBufferResized2.isValid();

            isOK = ( isValid && hasType );
            if ( isOK )
            {
                auto& srcTemp = rSrcData.editItemAt(rSrcData.add());
                srcTemp.mTypeId = typeId;
                srcTemp.mSizeId = rSel.mIBufferResized2.getSizes()[0];
                srcTemp.addSupportFormats(rSel.mIBufferResized2.getFormats());
            }
            else
            {
                MY_LOGW("failed to add src, hasType:%d, typeId:%d(%s), featureId:%d(%s)",
                    hasType, typeId, TypeID2Name(typeId), featureId, FeatID2Name(featureId));
            }
        }

        if ( isOK && rSel.mIBufferLCS.getRequired() )
        {
            const TypeID_T typeId = TID_MAN_LCS;
            const MBOOL hasType = rInfer.hasType(typeId);
            const MBOOL isValid = MTRUE;

            isOK = ( isValid && hasType );
            if ( isOK )
            {
                auto& srcTemp = rSrcData.editItemAt(rSrcData.add());
                srcTemp.mTypeId = typeId;
            }
            else
            {
                MY_LOGW("failed to add src, hasType:%d, typeId:%d(%s), featureId:%d(%s)",
                    hasType, typeId, TypeID2Name(typeId), featureId, FeatID2Name(featureId));
            }
        }

        if ( isOK && rSel.mIBufferLCS2.getRequired() )
        {
            const TypeID_T typeId = TID_SUB_LCS;
            const MBOOL hasType = rInfer.hasType(typeId);
            const MBOOL isValid = MTRUE;

            isOK = ( isValid && hasType );
            if ( isOK )
            {
                auto& srcTemp = rSrcData.editItemAt(rSrcData.add());
                srcTemp.mTypeId = typeId;
            }
            else
            {
                MY_LOGW("failed to add src, hasType:%d, typeId:%d(%s), featureId:%d(%s)",
                    hasType, typeId, TypeID2Name(typeId), featureId, FeatID2Name(featureId));
            }
        }

        const Property& rProperty =  item.second.mProviderPtr->property();
        if ( rProperty.mFaceData == eFD_Current )
        {
            auto& srcTemp = rSrcData.editItemAt(rSrcData.add());
            srcTemp.mTypeId = TID_MAN_FD;
            srcTemp.mSizeId = NULL_SIZE;
            rInfer.markFaceData(eFD_Current);
        }
        else if ( rProperty.mFaceData == eFD_Cache )
        {
            rInfer.markFaceData(eFD_Cache);
        }
        else if ( rProperty.mFaceData == eFD_None )
        {
            rInfer.markFaceData(eFD_None);
        }
        else
        {
            MY_LOGW("unknow faceDateType:%#09x", rInfer.mFaceDateType.value);
        }
        //
        if( rProperty.mBoost != 0 )
        {
            MY_LOGD("append boostType, value:%#09x", rProperty.mBoost);
            rInfer.appendBootType(rProperty.mBoost);
        }
        //
        if ( isOK && rSel.mOBufferDepth.getRequired() )
        {
            const TypeID_T typeId = TID_MAN_DEPTH;
            isOK = rSel.mOBufferDepth.isValid();

            if ( isOK )
            {
                auto& dstTemp = rDstData.editItemAt(rDstData.add());
                dstTemp.mTypeId = typeId;
                dstTemp.mSizeId = rSel.mOBufferDepth.getSizes()[0];
                dstTemp.mSize = rSel.mOBufferDepth.getSpecifiedSize();
                dstTemp.addSupportFormats(rSel.mOBufferDepth.getFormats());
            }
            else
            {
                MY_LOGW("failed to add dst, typeId:%d(%s), featureId:%d(%s)",
                    typeId, TypeID2Name(typeId), featureId, FeatID2Name(featureId));
            }
        }
        //
        if ( isOK )
        {
            rInfer.markThumbnailTiming(rProperty.mThumbnailTiming);
        }
        //
        if ( isOK && rSel.mOBufferThumbnail.getRequired() )
        {
            auto& dstTemp = rDstData.editItemAt(rDstData.add());
            dstTemp.mTypeId = TID_POSTVIEW;
        }
        //
        if ( isOK )
        {
            if (rSel.mIMetadataDynamic.getRequired())
                rMetadatas.push_back(MID_MAN_IN_P1_DYNAMIC);
            if (rSel.mIMetadataApp.getRequired())
                rMetadatas.push_back(MID_MAN_IN_APP);
            if (rSel.mIMetadataHal.getRequired())
                rMetadatas.push_back(MID_MAN_IN_HAL);
            if (rSel.mOMetadataApp.getRequired())
                rMetadatas.push_back(MID_MAN_OUT_APP);
            if (rSel.mOMetadataHal.getRequired())
                rMetadatas.push_back(MID_MAN_OUT_HAL);
            if (rSel.mIMetadataDynamic2.getRequired())
                rMetadatas.push_back(MID_SUB_IN_P1_DYNAMIC);
            if (rSel.mIMetadataHal2.getRequired())
                rMetadatas.push_back(MID_SUB_IN_HAL);
        }

        if ( isOK )
        {
            isEvaluated = MTRUE;
            rFeatures.push_back(featureId);
            if(!rInfer.addNodeIO(NID_DEPTH, rSrcData, rDstData, rMetadatas, rFeatures))
            {
                status = BAD_VALUE;
                break;
            }
            else
            {
                MY_LOGD("add feature, featureId:%d(%s)", featureId, FeatID2Name(featureId));
            }
        }
    }
    return status;
}

MVOID
DepthNode::
dispatch(const RequestPtr& pRequest, NodeID_T nodeId)
{
    UNREFERENCED_PARAMETER(nodeId);

    DepthNodeUtility::processNextCapture(pRequest, NSPipelinePlugin::eTiming_Depth);
    //
    CaptureFeatureNode::dispatch(pRequest);
}

void
DepthNode::
onAborted(PluginRequestPtr pluginRequestPtr)
{
    MY_LOGD("aborted requestAddr:%p", pluginRequestPtr.get());
}

void
DepthNode::
onCompleted(PluginRequestPtr pluginRequestPtr, MERROR status)
{
    MY_LOGD("completed pluginRequest, addr:%p, status:%d", pluginRequestPtr.get(), status);

    const auto key = pluginRequestPtr.get();
    RequestPtr pipeRequestPtr = nullptr;
    {
        std::lock_guard<std::mutex> guard(mRequestPairLock);

        auto foundItem = mRequestPairTable.find(key);
        if ( foundItem != mRequestPairTable.end() )
        {
            pipeRequestPtr = foundItem->second.mPipeRequest;
            mRequestPairTable.erase(foundItem);
            MY_LOGI("(%p) remove requestPair from table, req#:%d, pluginRequest:%p, RequestPairTableCount:%zu",
                this, pipeRequestPtr->getRequestNo(), pluginRequestPtr.get(), mRequestPairTable.size());
        }
        *pluginRequestPtr = Plugin::Request();
    }
    if ( pipeRequestPtr == nullptr )
    {
        MY_LOGE("failed to find pipeRequest from pluginRequest, addr:%p", key);
        return;
    }
    //
    if ( getIsDumpImage<NID_DEPTH>() )
    {
        pipeRequestPtr->dump("depthmap", getNodeID(), TID_MAN_DEPTH, OUTPUT);
    }
    //
    dispatch(pipeRequestPtr);
    //
    this->decExtThreadDependency();
    MY_LOGD("finish request, req#:%d, dependence:%d",
        pipeRequestPtr->getRequestNo(), this->getExtThreadDependency());
}

void
DepthNode::
onNextCapture(PluginRequestPtr pluginRequestPtr)
{
    MY_LOGD("no support next capture, requestAddr:%p", pluginRequestPtr.get());
}

} // NSCapture
} // namespace NSFeaturePipe
} // namespace NSCamFeature
} // namespace NSCam
