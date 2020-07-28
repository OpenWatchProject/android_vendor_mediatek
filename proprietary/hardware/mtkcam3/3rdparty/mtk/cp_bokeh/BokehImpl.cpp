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

#define LOG_TAG "PipelinePlugin/TKBokehProvider"

// Standard C header file
#include <stdlib.h>
#include <chrono>
#include <random>
#include <thread>
#include <memory>
// Android system/core header file

// mtkcam custom header file

// mtkcam global header file
#include <mtkcam/utils/std/Log.h>
#include <mtkcam/utils/std/ULog.h>
// Module header file
#include <mtkcam/drv/iopipe/SImager/IImageTransform.h>
#include <mtkcam/utils/metastore/IMetadataProvider.h>
#include <mtkcam3/3rdparty/plugin/PipelinePlugin.h>
#include <mtkcam3/3rdparty/plugin/PipelinePluginType.h>
//
#include <mtkcam/utils/metadata/client/mtk_metadata_tag.h>
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>
// Local header file


/******************************************************************************
 *
 ******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_ULOGMV("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_ULOGMD("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_ULOGMI("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_ULOGMW("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_ULOGME("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)

CAM_ULOG_DECLARE_MODULE_ID(MOD_LIB_BOKEH);

/*******************************************************************************
* MACRO Utilities Define.
********************************************************************************/
namespace { // anonymous namespace for debug MARCO function
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
createAutoTimer(const char* funcName, const char* text) -> AutoObject
{
    using Timing = std::chrono::time_point<std::chrono::high_resolution_clock>;
    using DuationTime = std::chrono::duration<float, std::milli>;

    Timing startTime = std::chrono::high_resolution_clock::now();
    return AutoObject(text, [funcName, startTime](const char* p)
    {
        Timing endTime = std::chrono::high_resolution_clock::now();
        DuationTime duationTime = endTime - startTime;
        CAM_LOGD("[%s] %s, elapsed(ms):%.4f",funcName, p, duationTime.count());
    });
}
#define AUTO_TIMER(TEXT) auto auto_timer = ::createAutoTimer(__FUNCTION__, TEXT)
//
#define UNREFERENCED_PARAMETER(param) (param)
//
} // end anonymous namespace for debug MARCO function

/*******************************************************************************
* Used Namespace.
********************************************************************************/
using namespace std;
//
using namespace android;
//
using namespace NSCam;
using namespace NSCam::NSPipelinePlugin;
using namespace NSCam::NSIoPipe::NSSImager;


/*******************************************************************************
* Type Alias
********************************************************************************/
using Property           = BokehPlugin::Property;
using Selection          = BokehPlugin::Selection;
using RequestPtr         = BokehPlugin::Request::Ptr;
using RequestCallbackPtr = BokehPlugin::RequestCallback::Ptr;
//
template<typename T>
using AutoPtr            = std::unique_ptr<T, std::function<void(T*)>>;
using ImgPtr             = AutoPtr<IImageBuffer>;
using MetaPtr            = AutoPtr<IMetadata>;
using ImageTransformPtr  = AutoPtr<IImageTransform>;


/*******************************************************************************
* Namespace Start.
********************************************************************************/
namespace { // anonymous namespace


/*******************************************************************************
* Class Definition
********************************************************************************/
/**
 * @brief MTK turkey bokeh algo. provider
 */
class MTKTurnkeyBokehProvider final: public BokehPlugin::IProvider
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Instantiation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    MTKTurnkeyBokehProvider();

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  BokehPlugin::IProvider Public Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:

    const Property& property() override;

    MERROR negotiate(Selection& sel) override;

    void init() override;

    MERROR process(RequestPtr requestPtr, RequestCallbackPtr callbackPtr) override;

    void abort(vector<RequestPtr>& requestPtrs) override;

    void uninit() override;

    ~MTKTurnkeyBokehProvider();
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  MTKTurnkeyBokehProvider Private Operator.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    MERROR processDone(const RequestPtr& requestPtr, const RequestCallbackPtr& callbackPtr, MERROR status);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  MTKTurnkeyBokehProvider Private Data Members.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    struct FlowControl
    {
        MINT8  mEnable;
        MBOOL  mIsNeedCurrentFD;
        MBOOL  mIsSupportThumbnail;
        MUINT8 mInitPhase;
        MBOOL  mIsDrawDepth;

        FlowControl();
    };

private:
    FlowControl mFlowControl;

};
REGISTER_PLUGIN_PROVIDER(Bokeh, MTKTurnkeyBokehProvider);

/**
 * @brief utility class
 */
class TKBokehUtility final
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Instantiation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    TKBokehUtility() = delete;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  TKBokehUtility Public Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    static inline ImgPtr createImgPtr(BufferHandle::Ptr& hangle);

    static inline MetaPtr createMetaPtr(MetadataHandle::Ptr& hangle);

    static inline ImageTransformPtr createImageTransformPtr();

    static inline MBOOL tryGetDoFLevel(const IMetadata& appMetadata, MINT32& dofLevel);

    static inline MBOOL tryGetAFStatus(const IMetadata& appMetadata, MUINT8& afStatus);

    static inline MBOOL tryGetAFRoi(const IMetadata& appMetadata, MRect& afRoi);

    static inline MVOID dump(const IImageBuffer* pImgBuf, const std::string& dumpName);

    static inline MVOID dump(IMetadata* pMetaData, const std::string& dumpName);
};


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  TKBokehUtility implementation.
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
ImgPtr
TKBokehUtility::
createImgPtr(BufferHandle::Ptr& hangle)
{
    return ImgPtr(hangle->acquire(), [hangle](IImageBuffer* p)
    {
        UNREFERENCED_PARAMETER(p);
        hangle->release();
    });
}

MetaPtr
TKBokehUtility::
createMetaPtr(MetadataHandle::Ptr& hangle)
{
    return MetaPtr(hangle->acquire(), [hangle](IMetadata* p)
    {
        UNREFERENCED_PARAMETER(p);
        hangle->release();
    });
}

ImageTransformPtr
TKBokehUtility::
createImageTransformPtr()
{
    return ImageTransformPtr(IImageTransform::createInstance(), [](IImageTransform *p)
    {
        p->destroyInstance();
    });
}

MBOOL
TKBokehUtility::
tryGetDoFLevel(const IMetadata& appMetadata, MINT32& dofLevel)
{
    MBOOL bRet = MFALSE;
    IMetadata::IEntry entry = appMetadata.entryFor(MTK_STEREO_FEATURE_DOF_LEVEL);
    if( !entry.isEmpty() )
    {
        dofLevel = entry.itemAt(0, Type2Type<MINT32>());
        bRet = MTRUE;
    }
    else
    {
        dofLevel = 0;
        MY_LOGD("failed to get app metadata: MTK_STEREO_FEATURE_DOF_LEVEL, and set as default value:%d", dofLevel);
    }
    return bRet;
}

MBOOL
TKBokehUtility::
tryGetAFStatus(const IMetadata& appMetadata, MUINT8& afStatus)
{
    MBOOL bRet = MFALSE;
    IMetadata::IEntry entry = appMetadata.entryFor(MTK_CONTROL_AF_STATE);
    if( !entry.isEmpty() )
    {
        afStatus = entry.itemAt(0, Type2Type<MINT32>());
        bRet = MTRUE;
    }
    else
    {
        afStatus = 0;
        MY_LOGD("failed to get app metadata: MTK_CONTROL_AF_STATE, and set as default value:%u", afStatus);
    }
    return bRet;
}

MBOOL
TKBokehUtility::
tryGetAFRoi(const IMetadata& appMetadata, MRect& afRoi)
{
    MBOOL bRet = MFALSE;
    IMetadata::IEntry entry = appMetadata.entryFor(MTK_3A_FEATURE_AF_ROI);
    if( !entry.isEmpty() )
    {
        // TODO: extract the value
        afRoi = MRect(0, 0);
        bRet = MTRUE;
    }
    else
    {
        afRoi = MRect(0, 0);
        MY_LOGD("failed to get app metadata: MTK_3A_FEATURE_AF_ROI, and set as default value:(%d, %d, %d, %d)",
            afRoi.p.x, afRoi.p.y, afRoi.s.w, afRoi.s.h);
    }
    return bRet;
}

MVOID
TKBokehUtility::
dump(const IImageBuffer* pImgBuf, const std::string& dumpName)
{
    MY_LOGD("dump image info, dumpName:%s, info:[a:%p, si:%dx%d, st:%zu, f:0x%x, va:%p]",
        dumpName.c_str(), pImgBuf,
        pImgBuf->getImgSize().w, pImgBuf->getImgSize().h,
        pImgBuf->getBufStridesInBytes(0),
        pImgBuf->getImgFormat(),
        reinterpret_cast<void*>(pImgBuf->getBufVA(0)));
}

MVOID
TKBokehUtility::
dump(IMetadata* pMetaData, const std::string& dumpName)
{
    MY_LOGD("dump meta info, dumpName:%s, info:[a:%p, c:%u]",
        dumpName.c_str(), pMetaData, pMetaData->count());
    pMetaData->dump();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  MTKTurnkeyBokehProvider implementation.
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MTKTurnkeyBokehProvider::FlowControl::
FlowControl()
: mEnable(-1)
, mIsNeedCurrentFD(MFALSE)
, mIsSupportThumbnail(MFALSE)
, mInitPhase(ePhase_OnPipeInit)
, mIsDrawDepth(MFALSE)
{
    mEnable = ::property_get_int32("vendor.debug.camera.tk.bokeh.enable", mEnable);
    mIsNeedCurrentFD = ::property_get_bool("vendor.debug.camera.tk.bokeh.currentfd", mIsNeedCurrentFD);
    mIsSupportThumbnail = ::property_get_bool("vendor.debug.camera.tk.bokeh.thumbnail", mIsSupportThumbnail);
    mInitPhase = ::property_get_int32("vendor.debug.camera.tk.bokeh.initphase", mInitPhase);
    mIsDrawDepth = ::property_get_bool("vendor.debug.camera.tk.bokeh.draw.depth", mIsDrawDepth);
    MY_LOGD("enable:%d, needCurrentFD:%d, supportThumbnail:%d, initPhase:%u, drawDepth:%d",
        mEnable, mIsNeedCurrentFD, mIsSupportThumbnail, mInitPhase, mIsDrawDepth);
}

MTKTurnkeyBokehProvider::
MTKTurnkeyBokehProvider()
{
    SCOPED_TRACER();
    MY_LOGD("ctor:%p", this);
}

const Property&
MTKTurnkeyBokehProvider::
property()
{
    static const Property prop = [this]() -> const Property
    {
        Property ret;
        ret.mName = "MTK_BOKEH";
        ret.mFeatures = MTK_FEATURE_BOKEH;
        ret.mThumbnailTiming = (mFlowControl.mIsSupportThumbnail ? eTiming_Bokeh : eTiming_P2);
        ret.mFaceData = (mFlowControl.mIsNeedCurrentFD ? eFD_Current : eFD_Cache);
        ret.mBoost = eBoost_CPU;
        ret.mInitPhase = (mFlowControl.mInitPhase == ePhase_OnPipeInit ? ePhase_OnPipeInit : ePhase_OnRequest);
        return ret;
    }();
    return prop;
}

MERROR
MTKTurnkeyBokehProvider::
negotiate(Selection& sel)
{
    SCOPED_TRACER();

    if( mFlowControl.mEnable == 0 )
    {
        MY_LOGD("force off tk bokeh");
        return BAD_VALUE;
    }
    // INPUT
    {
        sel.mIBufferFull
            .setRequired(MTRUE)
            .addAcceptedFormat(eImgFmt_NV21)
            .addAcceptedFormat(eImgFmt_NV12)
            .addAcceptedSize(eImgSize_Full);

        sel.mIBufferDepth
            .setRequired(MTRUE)
            .addAcceptedFormat(eImgFmt_Y8)
            .addAcceptedSize(eImgSize_Specified);

        sel.mIMetadataApp.setRequired(MTRUE);
        sel.mIMetadataHal.setRequired(MTRUE);
        sel.mIMetadataDynamic.setRequired(MTRUE);
    }
    // OUTPUT
    {
        sel.mOBufferFull
            .setRequired(MTRUE)
            .addAcceptedFormat(eImgFmt_NV21)
            .addAcceptedSize(eImgSize_Full);

        if ( mFlowControl.mIsSupportThumbnail )
        {
            sel.mOBufferThumbnail
                .setRequired(MTRUE);
        }

        sel.mOMetadataApp.setRequired(MTRUE);
        sel.mOMetadataHal.setRequired(MTRUE);
    }
    return OK;
}

void
MTKTurnkeyBokehProvider::
init()
{
    SCOPED_TRACER();
    ::srand(time(NULL));
}

MERROR
MTKTurnkeyBokehProvider::
process(RequestPtr requestPtr, RequestCallbackPtr callbackPtr)
{
    SCOPED_TRACER();

    auto isValidInput = [](const RequestPtr& requestPtr) -> MBOOL
    {
        const MBOOL ret = (requestPtr->mIBufferFull != nullptr)
                        && (requestPtr->mIBufferDepth != nullptr)
                        && (requestPtr->mIMetadataApp != nullptr)
                        && (requestPtr->mIMetadataHal != nullptr);
        if( !ret )
        {
            MY_LOGE("invalid request with input, reqAddr:%p, inFullImg:%p, inDepthImg:%p, inAppMeta:%p, inHalMeta:%p",
                requestPtr.get(),
                requestPtr->mIBufferFull.get(),
                requestPtr->mIBufferDepth.get(),
                requestPtr->mIMetadataApp.get(),
                requestPtr->mIMetadataHal.get());
        }
        return ret;
    };

    auto isValidOutput = [this](const RequestPtr& requestPtr) -> MBOOL
    {
        const MBOOL ret = (requestPtr->mOBufferFull != nullptr)
                        && (mFlowControl.mIsSupportThumbnail ? (requestPtr->mOBufferThumbnail != nullptr) : MTRUE)
                        && (requestPtr->mOMetadataApp != nullptr)
                        && (requestPtr->mOMetadataHal != nullptr);
        if( !ret )
        {
            MY_LOGE("invalid request with output, reqAddr:%p, outFullImg:%p, outThumbnail:%p, outAppMeta:%p, outHalMeta:%p",
                requestPtr.get(),
                requestPtr->mOBufferFull.get(),
                requestPtr->mOBufferThumbnail.get(),
                requestPtr->mOMetadataApp.get(),
                requestPtr->mOMetadataHal.get());
        }
        return ret;
    };
    MY_LOGD("process request, reqAddr:%p", requestPtr.get());

    if( !isValidInput(requestPtr) )
    {
        return processDone(requestPtr, callbackPtr, BAD_VALUE);
    }

    if( !isValidOutput(requestPtr) )
    {
        return processDone(requestPtr, callbackPtr, BAD_VALUE);
    }

    {
         // note: we can just call createXXXXPtr one time for a specified handle
        ImgPtr inFSImgPtr = TKBokehUtility::createImgPtr(requestPtr->mIBufferFull);
        ImgPtr inDepthImgPtr = TKBokehUtility::createImgPtr(requestPtr->mIBufferDepth);
        ImgPtr outFSImgPtr = TKBokehUtility::createImgPtr(requestPtr->mOBufferFull);
        //
        MetaPtr inAppMetaPtr = TKBokehUtility::createMetaPtr(requestPtr->mIMetadataApp);
        MetaPtr inHalMetaPtr = TKBokehUtility::createMetaPtr(requestPtr->mIMetadataHal);
        MetaPtr outAppMetaPtr = TKBokehUtility::createMetaPtr(requestPtr->mOMetadataApp);
        MetaPtr outHalMetaPtr = TKBokehUtility::createMetaPtr(requestPtr->mOMetadataHal);
        //
        ImageTransformPtr transformPtr = TKBokehUtility::createImageTransformPtr();
        const MRect crop = MRect(inFSImgPtr->getImgSize().w, inFSImgPtr->getImgSize().h);
        // dump info
        {
            TKBokehUtility::dump(inFSImgPtr.get(), "inFSImg");
            TKBokehUtility::dump(inDepthImgPtr.get(), "inDepthImg");
            TKBokehUtility::dump(outFSImgPtr.get(), "outFSImg");
            //
            TKBokehUtility::dump(inAppMetaPtr.get(), "inAppMeta");
            TKBokehUtility::dump(inHalMetaPtr.get(), "inHalMeta");
            TKBokehUtility::dump(outAppMetaPtr.get(), "outAppMeta");
            TKBokehUtility::dump(outHalMetaPtr.get(), "outHalMeta");
        }
        // get parameters
        {
            //
            MINT32 dofLevel = 0;
            TKBokehUtility::tryGetDoFLevel(*inAppMetaPtr, dofLevel);
            MUINT8 afStatus = 0;
            TKBokehUtility::tryGetAFStatus(*inAppMetaPtr, afStatus);
            MRect afRoi = MRect(0, 0);
            TKBokehUtility::tryGetAFRoi(*inAppMetaPtr, afRoi);

            MY_LOGD("get parameters, dofLevel:%d, afStatus:%u, afRoi:(%d, %d, %d, %d)",
                dofLevel, afStatus, afRoi.p.x, afRoi.p.y, afRoi.s.w, afRoi.s.h);
            //
            // face data
            IMetadata::IEntry entryFaceRects = inHalMetaPtr->entryFor(MTK_FEATURE_FACE_RECTANGLES);
            IMetadata::IEntry entryPoseOriens = inHalMetaPtr->entryFor(MTK_FEATURE_FACE_POSE_ORIENTATIONS);
            static const MINT32 poseOriensDataLength = 3;

            const size_t faceDataCount = entryFaceRects.count();
            const size_t poseOriensCount = entryPoseOriens.count();
            const MBOOL isValidFaceData = (poseOriensCount%poseOriensDataLength == 0) && (poseOriensCount/poseOriensDataLength == faceDataCount);
            if( !isValidFaceData )
            {
                MY_LOGW("invaild faceData, faceDataCount:%zu, poseOriensCount:%zu, poseOriensDataLength;%d",
                    faceDataCount, poseOriensCount, poseOriensDataLength);
            }
            else
            {
                for( size_t index = 0; index < faceDataCount; ++index )
                {
                    const MRect faceRect = entryFaceRects.itemAt(index, Type2Type<MRect>());
                    const MINT32 poseX = entryPoseOriens.itemAt(index*poseOriensDataLength + 0, Type2Type<MINT32>());
                    const MINT32 poseY = entryPoseOriens.itemAt(index*poseOriensDataLength + 1, Type2Type<MINT32>());
                    const MINT32 poseZ = entryPoseOriens.itemAt(index*poseOriensDataLength + 2, Type2Type<MINT32>());
                    MY_LOGD("face data, index:%zu/%zu, rectangle(leftTop-rightBottom):(%d, %d)-(%d, %d), pose(x, y, z):(%d, %d, %d)",
                        index, faceDataCount,
                        faceRect.p.x, faceRect.p.y, faceRect.s.w, faceRect.s.h,
                        poseX, poseY, poseZ);
                }
            }
        }
        // be filled with output
        {
            // 1. copy input yuv to output yuv
            {
                AUTO_TIMER("process copy(mdp) inputYuv to output");

                if( (transformPtr.get() == nullptr) || (!transformPtr->execute( inFSImgPtr.get(), outFSImgPtr.get(), nullptr, crop, 0, 3000)) )
                {
                    MY_LOGE("failed to generate output imgage, transformPtr:%p", transformPtr.get());
                    return processDone(requestPtr, callbackPtr, BAD_VALUE);
                }
                outFSImgPtr->syncCache(eCACHECTRL_FLUSH);
            }
            // 2. process thumbnail
            {
                if ( mFlowControl.mIsSupportThumbnail )
                {
                    ImgPtr outThumbnailImgPtr = TKBokehUtility::createImgPtr(requestPtr->mOBufferThumbnail);
                    if( (transformPtr.get() == nullptr) || (!transformPtr->execute( inFSImgPtr.get(), outThumbnailImgPtr.get(), nullptr, crop, 0, 3000)) )
                    {
                        MY_LOGE("failed to generate output thumbnail, transformPtr:%p", transformPtr.get());
                        return processDone(requestPtr, callbackPtr, BAD_VALUE);
                    }
                    outThumbnailImgPtr->syncCache(eCACHECTRL_FLUSH);
                }
            }
            // 3. copy depth result to output yuv
            if( mFlowControl.mIsDrawDepth )
            {
                AUTO_TIMER("copy depth result to output buffer");

                const MUINT32 depthBufStrides = inDepthImgPtr->getBufStridesInBytes(0);
                const MUINT32 outputBufStrides = outFSImgPtr->getBufStridesInBytes(0);
                const MSize& depthBufSize = inDepthImgPtr->getImgSize();
                const MSize& outpuBuftSize = outFSImgPtr->getImgSize();

                const MUINT32 copyStrides = (depthBufStrides > outputBufStrides) ? outputBufStrides : depthBufStrides;
                const MINT32 copyHeight = (depthBufSize.h > outpuBuftSize.h) ? outpuBuftSize.h : depthBufSize.h;
                // offset on the output buf
                const MINT32 xOffest = (outpuBuftSize.w > depthBufSize.w) ? (outpuBuftSize.w - depthBufSize.w)/2 : 0;
                const MINT32 yOffest = (outpuBuftSize.h > depthBufSize.h) ? (outpuBuftSize.h - depthBufSize.h)/2 : 0;

                MY_LOGW("depthBufSize:%dx%d, outpuBuftSize:%dx%d, offset:%dx%d",
                    depthBufSize.w, depthBufSize.h, outpuBuftSize.w, outpuBuftSize.h, xOffest, yOffest);

                char* pDepthBufVA = reinterpret_cast<char*>(inDepthImgPtr->getBufVA(0));
                char* pOutputBufBufVA = reinterpret_cast<char*>(outFSImgPtr->getBufVA(0)) + yOffest*outputBufStrides;
                for(int i = 0; i < copyHeight; i++)
                {
                    ::memcpy(pOutputBufBufVA + xOffest, pDepthBufVA, copyStrides);
                    pDepthBufVA += depthBufStrides;
                    pOutputBufBufVA += outputBufStrides;
                }
                outFSImgPtr->syncCache(eCACHECTRL_FLUSH);
            }
        }
    }
    // sleep a random time
    {
        std::default_random_engine generator;
        std::uniform_int_distribution<int> distribution(0, 500);
        const int randomTime = distribution(generator);
        const int baseTime = 1000;
        auto sleepTime = std::chrono::microseconds(baseTime + randomTime);
        MY_LOGD("sleep for %lld ms", sleepTime.count());
        {
            std::this_thread::sleep_for(sleepTime);
        }
        MY_LOGD("wait up");
    }
    return processDone(requestPtr, callbackPtr, OK);
}

MERROR
MTKTurnkeyBokehProvider::
processDone(const RequestPtr& requestPtr, const RequestCallbackPtr& callbackPtr, MERROR status)
{
    SCOPED_TRACER();

    MY_LOGD("process done, call complete, reqAddr:%p, callbackPtr:%p, status:%d",
        requestPtr.get(), callbackPtr.get(), status);

    if( callbackPtr != nullptr )
    {
        callbackPtr->onCompleted(requestPtr, status);
    }
    return OK;
}

void
MTKTurnkeyBokehProvider::
abort(vector<RequestPtr>& requestPtrs)
{
    SCOPED_TRACER();

    for(auto& item : requestPtrs)
    {
        MY_LOGD("abort request, reqAddr:%p", item.get());
    }
}

void
MTKTurnkeyBokehProvider::
uninit()
{
    SCOPED_TRACER();
}

MTKTurnkeyBokehProvider::
~MTKTurnkeyBokehProvider()
{
    SCOPED_TRACER();
    MY_LOGD("dtor:%p", this);
}


}  // anonymous namespace
