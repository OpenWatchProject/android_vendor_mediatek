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

#define LOG_TAG "PipelinePlugin/TPFusionProvider"

// Standard C header file
#include <stdlib.h>
#include <chrono>
#include <random>
#include <thread>
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

CAM_ULOG_DECLARE_MODULE_ID(MOD_LIB_FUSION);
// Local header file


using namespace NSCam;
using namespace android;
using namespace std;
using namespace NSCam::NSPipelinePlugin;
/******************************************************************************
 *
 ******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_ULOGMV("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_ULOGMD("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_ULOGMI("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_ULOGMW("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_ULOGME("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, ...)       do { if( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)


/*******************************************************************************
* MACRO Utilities Define.
********************************************************************************/
namespace { // anonymous namespace for debug MARCO function
using AutoObject = std::unique_ptr<const char, std::function<void(const char*)>>;
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
createAutoTimer(const char* funcName, const char* text) -> AutoObject
{
    using Timing = std::chrono::time_point<std::chrono::high_resolution_clock>;
    using DuationTime = std::chrono::duration<float, std::milli>;

    Timing startTime = std::chrono::high_resolution_clock::now();
    return AutoObject(text, [funcName, startTime](const char* p)
    {
        Timing endTime = std::chrono::high_resolution_clock::now();
        DuationTime duationTime = endTime - startTime;
        CAM_ULOGMD("[%s] %s, elapsed(ms):%.4f",funcName, p, duationTime.count());
    });
}
#define AUTO_TIMER(TEXT) auto auto_timer = ::createAutoTimer(__FUNCTION__, TEXT)
//
#define UNREFERENCED_PARAMETER(param) (param)
//
} // end anonymous namespace for debug MARCO function


/*******************************************************************************
* Alias.
********************************************************************************/
using namespace NSCam;
using namespace NSCam::NSPipelinePlugin;
using namespace NSCam::NSIoPipe::NSSImager;


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Type Alias..
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
using Property = FusionPlugin::Property;
using Selection = FusionPlugin::Selection;
using RequestPtr = FusionPlugin::Request::Ptr;
using RequestCallbackPtr = FusionPlugin::RequestCallback::Ptr;
//
template<typename T>
using AutoPtr             = std::unique_ptr<T, std::function<void(T*)>>;
//
using ImgPtr              = AutoPtr<IImageBuffer>;
using MetaPtr             = AutoPtr<IMetadata>;
using ImageTransformPtr   = AutoPtr<IImageTransform>;


/*******************************************************************************
* Namespace Start.
********************************************************************************/
namespace { // anonymous namespace


/*******************************************************************************
* Class Definition
********************************************************************************/
/**
 * @brief third party fusion algo. provider
 */
class ThirdPartyFusionProvider final: public FusionPlugin::IProvider
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Instantiation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    ThirdPartyFusionProvider();

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  FusionPlugin::IProvider Public Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:

    const Property& property() override;

    MERROR negotiate(Selection& sel) override;

    void init() override;

    MERROR process(RequestPtr requestPtr, RequestCallbackPtr callbackPtr) override;

    void abort(vector<RequestPtr>& requestPtrs) override;

    void uninit() override;

    ~ThirdPartyFusionProvider();
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  ThirdPartyFusionProvider Private Operator.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    MERROR processDone(const RequestPtr& requestPtr, const RequestCallbackPtr& callbackPtr, MERROR status);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  ThirdPartyFusionProvider Private Data Members.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    MINT32 mEnable;

};
REGISTER_PLUGIN_PROVIDER(Fusion, ThirdPartyFusionProvider);
/**
 * @brief utility class
 */
class TPFusionUtility final
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Instantiation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    TPFusionUtility() = delete;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  TPFusionUtility Public Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    static inline ImageTransformPtr createImageTransformPtr();

    static inline ImgPtr createImgPtr(BufferHandle::Ptr& hangle);

    static inline MetaPtr createMetaPtr(MetadataHandle::Ptr& hangle);

    static inline MVOID dump(const IImageBuffer* pImgBuf, const std::string& dumpName);

    static inline MVOID dump(IMetadata* pMetaData, const std::string& dumpName);
};


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  TPFusionUtility implementation.
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
ImageTransformPtr
TPFusionUtility::
createImageTransformPtr()
{
    return ImageTransformPtr(IImageTransform::createInstance(), [](IImageTransform *p)
    {
        p->destroyInstance();
    });
}

ImgPtr
TPFusionUtility::
createImgPtr(BufferHandle::Ptr& hangle)
{
    return ImgPtr(hangle->acquire(), [hangle](IImageBuffer* p)
    {
        UNREFERENCED_PARAMETER(p);
        hangle->release();
    });
};

MetaPtr
TPFusionUtility::
createMetaPtr(MetadataHandle::Ptr& hangle)
{
    return MetaPtr(hangle->acquire(), [hangle](IMetadata* p)
    {
        UNREFERENCED_PARAMETER(p);
        hangle->release();
    });
};

MVOID
TPFusionUtility::
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
TPFusionUtility::
dump(IMetadata* pMetaData, const std::string& dumpName)
{
    MY_LOGD("dump meta info, dumpName:%s, addr::%p, count:%u",
        dumpName.c_str(), pMetaData, pMetaData->count());
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  ThirdPartyFusionProvider implementation.
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
ThirdPartyFusionProvider::
ThirdPartyFusionProvider()
: mEnable(-1)
{
    MY_LOGD("ctor:%p", this);
    // on:1/off:0/auto:-1
    mEnable = ::property_get_int32("vendor.debug.camera.tp.fusion.enable", mEnable);
}

const Property&
ThirdPartyFusionProvider::
property()
{
    static const Property prop = []() -> const Property
    {
        Property ret;
        ret.mName = "MTK_TP_FUSION";
        ret.mFeatures = TP_FEATURE_FUSION;
        ret.mFaceData = eFD_Cache;
        ret.mBoost = eBoost_CPU;
        ret.mInitPhase = ePhase_OnRequest;
        return ret;
    }();
    return prop;
}

MERROR
ThirdPartyFusionProvider::
negotiate(Selection& sel)
{
    SCOPED_TRACER();

    if( mEnable == 0 )
    {
        MY_LOGD("force off tp fusion");
        return BAD_VALUE;
    }
    // INPUT
    {
        sel.mIBufferFull
            .setRequired(MTRUE)
            .addAcceptedFormat(eImgFmt_NV21)
            .addAcceptedSize(eImgSize_Full);

        sel.mIBufferFull2
            .setRequired(MTRUE)
            .addAcceptedFormat(eImgFmt_NV21)
            .addAcceptedSize(eImgSize_Full);

        sel.mIMetadataApp.setRequired(MTRUE);
        sel.mIMetadataHal.setRequired(MTRUE);
        sel.mIMetadataHal2.setRequired(MTRUE);
        sel.mIMetadataDynamic.setRequired(MTRUE);
        sel.mIMetadataDynamic2.setRequired(MTRUE);
    }
    // OUTPUT
    {
        sel.mOBufferFull
            .setRequired(MTRUE)
            .addAcceptedFormat(eImgFmt_NV21)
            .addAcceptedSize(eImgSize_Full);

        sel.mOMetadataApp.setRequired(MTRUE);
        sel.mOMetadataHal.setRequired(MTRUE);
    }
    return OK;
}

void
ThirdPartyFusionProvider::
init()
{
    SCOPED_TRACER();
    ::srand(time(nullptr));
}

MERROR
ThirdPartyFusionProvider::
process(RequestPtr requestPtr, RequestCallbackPtr callbackPtr)
{
    SCOPED_TRACER();

    auto isValidInput = [](const RequestPtr& requestPtr) -> MBOOL
    {
        const MBOOL ret = requestPtr->mIBufferFull != nullptr
                    && requestPtr->mIBufferFull2 != nullptr
                    && requestPtr->mIMetadataApp != nullptr
                    && requestPtr->mIMetadataHal != nullptr
                    && requestPtr->mIMetadataHal2 != nullptr;
        if( !ret )
        {
            MY_LOGE("invalid request with input, req:%p, inFullImg:%p, inFullImg2:%p, inAppMeta:%p, inHalMeta:%p, inHalMeta2:%p",
                requestPtr.get(),
                requestPtr->mIBufferFull.get(),
                requestPtr->mIBufferFull2.get(),
                requestPtr->mIMetadataApp.get(),
                requestPtr->mIMetadataHal.get(),
                requestPtr->mIMetadataHal2.get());
        }
        return ret;
    };

    auto isValidOutput = [](const RequestPtr& requestPtr) -> MBOOL
    {
        const MBOOL ret = requestPtr->mOBufferFull != nullptr
                    && requestPtr->mOMetadataApp != nullptr
                    && requestPtr->mOMetadataHal != nullptr;
        if( !ret )
        {
            MY_LOGE("invalid request with input, req:%p, outFullImg:%p, outAppMeta:%p, outHalMeta:%p",
                requestPtr.get(),
                requestPtr->mOBufferFull.get(),
                requestPtr->mOMetadataApp.get(),
                requestPtr->mOMetadataHal.get());
        }
        return ret;
    };

    MY_LOGD("process, reqAdrr:%p", requestPtr.get());

    if( !isValidInput(requestPtr) )
    {
        return processDone(requestPtr, callbackPtr, BAD_VALUE);
    }

    if( !isValidOutput(requestPtr) )
    {
        return processDone(requestPtr, callbackPtr, BAD_VALUE);
    }
    //
    //
    {
        // note: we can just call createXXXXPtr one time for a specified handle
        ImgPtr inMainImgPtr = TPFusionUtility::createImgPtr(requestPtr->mIBufferFull);
        ImgPtr inSubImgPtr = TPFusionUtility::createImgPtr(requestPtr->mIBufferFull2);
        ImgPtr outFSImgPtr = TPFusionUtility::createImgPtr(requestPtr->mOBufferFull);
        //
        MetaPtr inAppMetaPtr = TPFusionUtility::createMetaPtr(requestPtr->mIMetadataApp);
        MetaPtr inMainHalMetaPtr = TPFusionUtility::createMetaPtr(requestPtr->mIMetadataHal);
        MetaPtr inSubHalMetaPtr = TPFusionUtility::createMetaPtr(requestPtr->mIMetadataHal2);
        MetaPtr outAppMetaPtr = TPFusionUtility::createMetaPtr(requestPtr->mOMetadataApp);
        MetaPtr outHalMetaPtr = TPFusionUtility::createMetaPtr(requestPtr->mOMetadataHal);
        // dump info
        {
            TPFusionUtility::dump(inMainImgPtr.get(), "inputMainImg");
            TPFusionUtility::dump(inSubImgPtr.get(), "inputSubImg");
            TPFusionUtility::dump(outFSImgPtr.get(), "outFSImg");
            //
            TPFusionUtility::dump(inAppMetaPtr.get(), "inAppMeta");
            TPFusionUtility::dump(inMainHalMetaPtr.get(), "inMainHalMeta");
            TPFusionUtility::dump(inSubHalMetaPtr.get(), "inSubHalMeta");
            TPFusionUtility::dump(outAppMetaPtr.get(), "outAppMeta");
            TPFusionUtility::dump(outHalMetaPtr.get(), "outHalMeta");
        }
        // copy input yuv to output yuv
        {
            AUTO_TIMER("proces fusion algo.");

            ImageTransformPtr transformPtr = TPFusionUtility::createImageTransformPtr();
            if(transformPtr.get() != nullptr)
            {
                AUTO_TIMER("process copy(mdp) inputYuv to outputYuv");

                const MRect crop = MRect(inMainImgPtr->getImgSize().w, inMainImgPtr->getImgSize().h);
                if( transformPtr->execute( inMainImgPtr.get(), outFSImgPtr.get(), nullptr, crop, 0, 3000) )
                {
                    MY_LOGD("success to execute image transform");
                    outFSImgPtr->syncCache(eCACHECTRL_FLUSH);
                }
                else
                {
                    MY_LOGE("failed to execute image transform");
                    return processDone(requestPtr, callbackPtr, BAD_VALUE);
                }
            }
            else
            {
                MY_LOGE("IImageTransform is nullptr, cannot generate output");
                return processDone(requestPtr, callbackPtr, BAD_VALUE);
            }
        }
    }
    //
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
ThirdPartyFusionProvider::
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
ThirdPartyFusionProvider::
abort(vector<RequestPtr>& requestPtrs)
{
    SCOPED_TRACER();

    for(auto& item : requestPtrs)
    {
        MY_LOGD("abort request, reqAddr:%p", item.get());
    }
}

void
ThirdPartyFusionProvider::
uninit()
{
    SCOPED_TRACER();
}

ThirdPartyFusionProvider::
~ThirdPartyFusionProvider()
{
    MY_LOGD("dtor:%p", this);
}

}  // anonymous namespace
