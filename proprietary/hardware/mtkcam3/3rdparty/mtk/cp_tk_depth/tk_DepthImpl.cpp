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

#define LOG_TAG "PipelinePlugin/CPTKDepthProvider"

// Standard C header file
#include <stdlib.h>
#include <chrono>
#include <random>
#include <thread>
#include <bitset>
#include <climits>
// Android system/core header file

// mtkcam custom header file
#include <camera_custom_stereo.h>

// mtkcam global header file
#include <mtkcam/utils/std/Log.h>
#include <mtkcam/utils/std/ULog.h>
// Module header file
#include <mtkcam/utils/metastore/IMetadataProvider.h>
#include <mtkcam3/3rdparty/plugin/PipelinePlugin.h>
#include <mtkcam3/3rdparty/plugin/PipelinePluginType.h>
#include <mtkcam3/feature/stereo/pipe/DepthPipeHolder.h>
#include <mtkcam3/feature/stereo/pipe/IDualFeatureRequest.h>
//
#include <mtkcam3/feature/stereo/pipe/IDepthMapPipe.h>
#include <mtkcam3/feature/stereo/pipe/IDepthMapEffectRequest.h>
#include <mtkcam3/feature/stereo/pipe/IDualFeatureRequest.h>
#include <mtkcam3/feature/stereo/hal/stereo_size_provider.h>
//
#include <mtkcam/utils/metadata/client/mtk_metadata_tag.h>
#include <mtkcam/utils/metadata/IMetadata.h>
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>
//
#include <mtkcam3/feature/stereo/hal/stereo_setting_provider.h>
// Local header file


/******************************************************************************
 *
 ******************************************************************************/
#define MY_LOGD(fmt, arg...)        CAM_ULOGMD("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_ULOGMI("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_ULOGME("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
//
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)

CAM_ULOG_DECLARE_MODULE_ID(MOD_LIB_DEPTH);

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
#define UNREFERENCED_PARAMETER(a) (a)
//
} // end anonymous namespace for debug MARCO function


/*******************************************************************************
* Used namespace.
********************************************************************************/
using namespace std;
//
using namespace android;
//
using namespace NSCam;
using namespace NSCam::NSPipelinePlugin;
/*******************************************************************************
* Type Alias.
********************************************************************************/
using Property           = DepthPlugin::Property;
using Selection          = DepthPlugin::Selection;
using RequestPtr         = DepthPlugin::Request::Ptr;
using RequestCallbackPtr = DepthPlugin::RequestCallback::Ptr;
//
/*******************************************************************************
* Namespace Start.
********************************************************************************/
namespace NSCam { //
namespace NSCamFeature {
namespace NSFeaturePipe {

using namespace NSDualFeature;
using namespace NSFeaturePipe_DepthMap;
using NSFeaturePipe_DepthMap::IDepthMapEffectRequest;

/*******************************************************************************
* Class Definition
********************************************************************************/
/**
 * @brief MTK turkey depth algo. provider
 */
class cp_tk_DepthProvider final: public DepthPlugin::IProvider
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Instantiation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    cp_tk_DepthProvider();

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  DepthPlugin::IProvider Public Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:

    const Property& property() override;

    MERROR negotiate(Selection& sel) override;

    void init() override;

    MERROR process(RequestPtr requestPtr, RequestCallbackPtr callbackPtr) override;

    void abort(vector<RequestPtr>& requestPtrs) override;

    void uninit() override;

    ~cp_tk_DepthProvider();
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  cp_tk_DepthProvider Private Operator.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    struct inputImgData{
        NodeBufferSetting    param;
        NSCam::IImageBuffer* buf;
    };
    struct inputMetaData{
        NodeBufferSetting param;
        IMetadata*        meta;
    };
    //
    MERROR  processDone(const RequestPtr& requestPtr,
                        const RequestCallbackPtr& callbackPtr,
                        MERROR status);

    MUINT16 setOutputData(sp<IDepthMapEffectRequest> pDepMapReq,
                          RequestPtr requestPtr);

    MUINT16 setInputData(sp<IDepthMapEffectRequest> pDepMapReq,
                         RequestPtr requestPtr);

    static MVOID onPipeReady(MVOID* tag, NSDualFeature::ResultState state,
                       sp<IDualFeatureRequest>& request);

    MERROR _copy2Target(IImageBuffer* pSrc,IImageBuffer* target);
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  cp_tk_DepthProvider Private Data Members.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    MINT32 mEnable;
    bool mBypass = 0;
    bool mDebug  = 0;

    // capture request start at 3000
    MINT32 mReqID = 0;
    MUINT32 mStereoModeType = 0;

    android::Mutex mLock;

    android::sp<SmartDepthMapPipe> mpDepthMapPipe;

    struct CapDepthPlugPackage {
        sp<IDepthMapEffectRequest>  depthReq;
        RequestPtr                  enqueReq;
        RequestCallbackPtr          callback;
    };
    KeyedVector<MUINT32, CapDepthPlugPackage> mvCapDepthPlugPack;
};
REGISTER_PLUGIN_PROVIDER(Depth, cp_tk_DepthProvider);
/**
 * @brief utility class
 */
class TKDepthUtility final
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Instantiation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    TKDepthUtility() = delete;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  TKDepthUtility Public Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    static inline MVOID dump(const IImageBuffer* pImgBuf, const std::string& dumpName);

    static inline MVOID dump(IMetadata* pMetaData, const std::string& dumpName);
};


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  TKDepthUtility implementation.
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
TKDepthUtility::
dump(const IImageBuffer* pImgBuf, const std::string& dumpName)
{
    MY_LOGD("dump image info, dumpName:%s, info: [a:%p, si:%dx%d, st:%zu, f:0x%x, va:%p]",
        dumpName.c_str(), pImgBuf,
        pImgBuf->getImgSize().w, pImgBuf->getImgSize().h,
        pImgBuf->getBufStridesInBytes(0),
        pImgBuf->getImgFormat(),
        reinterpret_cast<void*>(pImgBuf->getBufVA(0)));
}

MVOID
TKDepthUtility::
dump(IMetadata* pMetaData, const std::string& dumpName)
{
    MY_LOGD("dump meta info, dumpName:%s, info:[a:%p, c:%u]",
        dumpName.c_str(), pMetaData, pMetaData->count());
    pMetaData->dump();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  cp_tk_DepthProvider implementation.
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
cp_tk_DepthProvider::cp_tk_DepthProvider() :
    mEnable(1)
{
    mEnable = ::property_get_int32("vendor.debug.camera.tk.depth.enable",
                                   mEnable);// on:1/off:0/auto:-1
    mBypass = ::property_get_bool("vendor.debug.camera.tk.depth.bypass", 0);
    //
    mStereoModeType = getStereoModeType();// 0: tk | 1: pure 3rd | 2: tk depth + 3rd
    MY_LOGD("ctor:%p, RunningMode:%d StereoMode:%d", this,mEnable, mStereoModeType);
}

const Property& cp_tk_DepthProvider::property()
{
    static const Property prop = []() -> const Property
    {
        Property ret;
        ret.mName       = "CP_TK_DEPTH";
        ret.mFeatures   = MTK_FEATURE_DEPTH;
        ret.mFaceData   = eFD_Cache;
        ret.mPreprocess = ePreProcess_None;
        ret.mBoost      = eBoost_CPU;
        ret.mInitPhase  = ePhase_OnPipeInit;
        return ret;
    }();
    return prop;
}

MERROR cp_tk_DepthProvider::negotiate(Selection& sel)
{
    SCOPED_TRACER();

    if (mEnable == 0)
    {
        MY_LOGD("force off tk depth");
        return BAD_VALUE;
    }
    if (mStereoModeType != 0 && mStereoModeType != 2)
    {
        MY_LOGD("Not Support the pure 3rdParty");
        return BAD_VALUE;
    }

    {// INPUT
        sel.mIBufferFull
            .setRequired(MTRUE)
            .addAcceptedFormat(eImgFmt_NV21)
            .addAcceptedFormat(eImgFmt_NV12)
            .addAcceptedSize(eImgSize_Full);

        sel.mIBufferFull2
            .setRequired(MTRUE)
            .addAcceptedFormat(eImgFmt_NV21)
            .addAcceptedFormat(eImgFmt_NV12)
            .addAcceptedSize(eImgSize_Full);

        sel.mIBufferResized
            .setRequired(MTRUE)
            .addAcceptedFormat(eImgFmt_NV21)
            .addAcceptedSize(eImgSize_Resized);

        sel.mIBufferResized2
            .setRequired(MTRUE)
            .addAcceptedFormat(eImgFmt_NV21)
            .addAcceptedSize(eImgSize_Resized);

        sel.mIBufferLCS
            .setRequired(MTRUE)
            .addAcceptedFormat(eImgFmt_NV21)
            .addAcceptedSize(eImgSize_Resized);

        sel.mIBufferLCS2
            .setRequired(MTRUE)
            .addAcceptedFormat(eImgFmt_NV21)
            .addAcceptedSize(eImgSize_Resized);

        sel.mIMetadataApp.setRequired(MTRUE);
        sel.mIMetadataHal.setRequired(MTRUE);
        sel.mIMetadataHal2.setRequired(MTRUE);
        sel.mIMetadataDynamic.setRequired(MTRUE);
    }
    // OUTPUT
    {
        MTK_DEPTHMAP_INFO_T depthInfo = StereoSettingProvider::getDepthmapInfo();
        sel.mOBufferDepth.setRequired(MTRUE)
            .addAcceptedFormat(depthInfo.format)
            .addAcceptedSize(eImgSize_Specified)
            .setSpecifiedSize(depthInfo.size);

        sel.mOMetadataApp.setRequired(MTRUE);
        sel.mOMetadataHal.setRequired(MTRUE);
    }
    return OK;
}

void cp_tk_DepthProvider::init()
{
    MY_LOGD("+");
    SCOPED_TRACER();
    ::srand(time(nullptr));

    mReqID = 0;

    if (mStereoModeType == 0 || mStereoModeType == 2)
        mpDepthMapPipe = DepthPipeHolder::waitInstance();
    else
        MY_LOGE("3rdParty Mode(%d). No Need to init this plugin", mStereoModeType);

    mDebug = ::property_get_bool("vendor.debug.cp_tkDepth.debugLog", 0);
    MY_LOGD("-");
}

MERROR cp_tk_DepthProvider::process(RequestPtr requestPtr, RequestCallbackPtr callbackPtr)
{
    SCOPED_TRACER();

    if (mBypass) {
        MERROR ret = OK;
        ret = _copy2Target(requestPtr->mIBufferFull->acquire(),
                           requestPtr->mOBufferDepth->acquire());
        MY_LOGI_IF(ret == OK, "ByPass DepthMapPipe on Demand!!");
        processDone(requestPtr, callbackPtr, ret);
        return OK;
    }

    auto isValidInput = [](const RequestPtr& requestPtr) -> MBOOL
    {
        const MBOOL ret =  requestPtr->mIBufferFull     != nullptr &&
                           requestPtr->mIBufferFull2    != nullptr &&
                           //
                           requestPtr->mIBufferResized  != nullptr &&
                           requestPtr->mIBufferResized2 != nullptr &&
                           requestPtr->mIBufferLCS      != nullptr &&
                           requestPtr->mIBufferLCS2     != nullptr &&
                           //
                           requestPtr->mIMetadataApp  != nullptr &&
                           requestPtr->mIMetadataHal  != nullptr &&
                           requestPtr->mIMetadataHal2 != nullptr &&
                           requestPtr->mIMetadataDynamic != nullptr;
        if (!ret) {
            MY_LOGE("invalid request with INPUT, reqAddr:%p, inFullImg:%p,inFullImg2:%p,"
                    "inBufRRz:%p, inBufRRz2:%p, inBufLCS:%p, inBufLCS2:%p"
                    "inAppMeta:%p, inHalMeta:%p, inMalMetaDyn:%p, inHalMeta2:%p",
                    requestPtr.get(),
                    requestPtr->mIBufferFull.get()     , requestPtr->mIBufferFull2.get(),
                    requestPtr->mIBufferResized.get()  , requestPtr->mIBufferResized2.get(),
                    requestPtr->mIBufferLCS.get()      , requestPtr->mIBufferLCS2.get(),
                    requestPtr->mIMetadataApp.get()    , requestPtr->mIMetadataHal.get(),
                    requestPtr->mIMetadataDynamic.get(), requestPtr->mIMetadataHal2.get());
        }
        return ret;
    };

    auto isValidOutput = [](const RequestPtr& requestPtr) -> MBOOL
    {
        const MBOOL ret = requestPtr->mOBufferDepth != nullptr &&
                          requestPtr->mOMetadataApp != nullptr &&
                          requestPtr->mOMetadataHal != nullptr;
        if (!ret) {
            MY_LOGE("invalid request with OUTPUT, reqAddr:%p,"
                    "outDepthImg:%p, outAppMeta:%p, outHalMeta:%p",
                    requestPtr.get(), requestPtr->mOBufferDepth.get(),
                    requestPtr->mOMetadataApp.get(), requestPtr->mOMetadataHal.get());
        }
        return ret;
    };

    if (!isValidInput(requestPtr) || !isValidOutput(requestPtr))
    {
        return processDone(requestPtr, callbackPtr, BAD_VALUE);
    }
    else
    {
        android::Mutex::Autolock lock(mLock);
        mReqID = (static_cast<MINT32>(NSCam::Utils::TimeTool::getReadableTime()));
        MY_LOGD("process, reqID = %u, reqAddr:%p", mReqID, requestPtr.get());
    }
    //
    // create Request
    sp<IDepthMapEffectRequest> pDepMapReq = IDepthMapEffectRequest::createInstance(
                                                            mReqID, onPipeReady, this);

    if (!setInputData(pDepMapReq, requestPtr) && !setOutputData(pDepMapReq, requestPtr))
    {
        {
            android::Mutex::Autolock lock(mLock);
            CapDepthPlugPackage pack;
            pack.depthReq = pDepMapReq;
            pack.enqueReq = requestPtr;
            pack.callback = callbackPtr;
            mvCapDepthPlugPack.add(pDepMapReq->getRequestNo(), pack);
            MY_LOGD("reqID:%u [enque]", pDepMapReq->getRequestNo());
        }
        mpDepthMapPipe->enque(pDepMapReq);
    }
    else
    {
        MY_LOGE("ByPass DepthMapPipe!! Something Wrong");
    }
    return OK;
}

MERROR cp_tk_DepthProvider::processDone(const RequestPtr& requestPtr,
                                    const RequestCallbackPtr& callbackPtr, MERROR status)
{
    SCOPED_TRACER();

    MY_LOGD("process done, call complete, reqAddr:%p, callbackPtr:%p, status:%d",
            requestPtr.get(), callbackPtr.get(), status);

    if (callbackPtr != nullptr)
    {
        callbackPtr->onCompleted(requestPtr, status);
    }

    return OK;
}

void cp_tk_DepthProvider::abort(vector<RequestPtr>& requestPtrs)
{
    SCOPED_TRACER();

    for(auto& item : requestPtrs)
    {
        MY_LOGD("abort request, reqAddr:%p", item.get());
    }
}

void cp_tk_DepthProvider::uninit()
{
    SCOPED_TRACER();
    if (0 != mvCapDepthPlugPack.size()) {
        MY_LOGE("CapDepthPlugPack should be release before uninit");
        mvCapDepthPlugPack.clear();
    }
}

cp_tk_DepthProvider::~cp_tk_DepthProvider()
{
    MY_LOGD("dtor:%p", this);
}

MUINT16 cp_tk_DepthProvider::setOutputData(sp<IDepthMapEffectRequest> pDepMapReq,
                                          RequestPtr requestPtr)
{
    MY_LOGD_IF(mDebug > 0, "+");
    //
    vector<inputImgData> vImgs {
        {{PBID_OUT_DEPTHMAP, eBUFFER_IOTYPE_OUTPUT}, requestPtr->mOBufferDepth->acquire()},//0x01
    };
    //
    vector<inputMetaData> vMetas {
        {{PBID_OUT_APP_META, eBUFFER_IOTYPE_OUTPUT}, requestPtr->mIMetadataApp->acquire()},//0x02
        {{PBID_OUT_HAL_META, eBUFFER_IOTYPE_OUTPUT}, requestPtr->mIMetadataHal->acquire()},//0x04
    };
    //
    std::bitset<3> state;
    //
    for (MUINT i = 0; i < vImgs.size(); i++) {
        if (!pDepMapReq->pushRequestImageBuffer(
                    {vImgs[i].param.bufferID, vImgs[i].param.ioType}, vImgs[i].buf))
            state.set(i);
    }
    //
    for (MUINT i = 0; i < vMetas.size(); i++) {
        if (!pDepMapReq->pushRequestMetadata(
                    {vMetas[i].param.bufferID, vMetas[i].param.ioType}, vMetas[i].meta))
            state.set(i + vImgs.size());
    }
    MY_LOGE_IF(true == state.any(), "set Output with Error, state(%#lx)", state.to_ulong());
    //
    MY_LOGD_IF(mDebug > 0, "-");
    return state.to_ulong();
}

MUINT16 cp_tk_DepthProvider::setInputData(sp<IDepthMapEffectRequest> pDepMapReq,
                                          RequestPtr requestPtr)
{
    MY_LOGD_IF(mDebug > 0, "+");
    //
    vector<inputImgData> vImgs {
        {{PBID_IN_YUV1, eBUFFER_IOTYPE_INPUT}, requestPtr->mIBufferResized->acquire()},//0x01
        {{PBID_IN_YUV2, eBUFFER_IOTYPE_INPUT}, requestPtr->mIBufferFull2->acquire()},//0x02
    };
    //
    vector<inputMetaData> vMetas {
        {{PBID_IN_APP_META      , eBUFFER_IOTYPE_INPUT}, requestPtr->mIMetadataApp->acquire()},
        {{PBID_IN_HAL_META_MAIN1, eBUFFER_IOTYPE_INPUT}, requestPtr->mIMetadataHal->acquire()},
        {{PBID_IN_P1_RETURN_META, eBUFFER_IOTYPE_INPUT}, requestPtr->mIMetadataDynamic->acquire()},

        {{PBID_IN_HAL_META_MAIN2, eBUFFER_IOTYPE_INPUT}, requestPtr->mIMetadataHal2->acquire()},
    };
    //
    std::bitset<10> state;//vImgs.size+vMetas.size
    //
    for (MUINT i = 0; i < vImgs.size(); i++) {
        if (!pDepMapReq->pushRequestImageBuffer(
                    {vImgs[i].param.bufferID, vImgs[i].param.ioType}, vImgs[i].buf))
            state.set(i);
    }
    //
    for (MUINT i = 0; i < vMetas.size(); i++) {
        if (!pDepMapReq->pushRequestMetadata(
                    {vMetas[i].param.bufferID, vMetas[i].param.ioType}, vMetas[i].meta))
            state.set(i + vImgs.size());
    }
    MY_LOGE_IF(true == state.any(), "set input with Error, state(%#lx)", state.to_ulong());
    //
    MY_LOGD_IF(mDebug > 0, "-");
    return state.to_ulong();
}

MVOID cp_tk_DepthProvider::onPipeReady(MVOID* tag, NSDualFeature::ResultState state,
                                      sp<IDualFeatureRequest>& request)
{
    cp_tk_DepthProvider* pDepProvider  = (cp_tk_DepthProvider*)tag;
    sp<IDepthMapEffectRequest> pDepReq = (IDepthMapEffectRequest*) request.get();

    MY_LOGD("+,reqID=%u state=%d:%s", pDepProvider->mReqID, state, ResultState2Name(state));
    //
    CapDepthPlugPackage pack;
    if (state == eRESULT_COMPLETE || state == eRESULT_DEPTH_NOT_READY ||
        state == eRESULT_FLUSH)
    {
        ssize_t idx = -1;
        {
            android::Mutex::Autolock lock(pDepProvider->mLock);
            idx = pDepProvider->mvCapDepthPlugPack.indexOfKey(pDepReq->getRequestNo());
            if (idx < 0) {
                    MY_LOGE("[DepthNode]idx=%zu,reqID=%u is missing, might be released!!"
                            "size=%zu", idx, pDepReq->getRequestNo(),
                            pDepProvider->mvCapDepthPlugPack.size());
                return;
            }
            pack = pDepProvider->mvCapDepthPlugPack.valueAt(idx);
            pDepProvider->mvCapDepthPlugPack.removeItem(pDepReq->getRequestNo());
        }
        pDepProvider->processDone(pack.enqueReq, pack.callback, OK);
    }
    else
        pDepProvider->processDone(pack.enqueReq, pack.callback, BAD_VALUE);
    //
    MY_LOGD("-");
}

MERROR cp_tk_DepthProvider::_copy2Target(IImageBuffer* pSrc,IImageBuffer* target)
{
    MERROR ret = OK;

    if (pSrc == nullptr || target == nullptr) {
        ret = BAD_VALUE;
        return ret;
    }

    unsigned char *outBuf = nullptr, *src1Buf = nullptr;
    size_t src1BufSize = 0, src1Strides = 0, outBufStrides = 0;
    size_t outBufSize = 0, planeCnt = 0, strides_diff = 0;

    for (size_t i = 0; i < planeCnt; i++) {
        src1Buf = (unsigned char*)pSrc->getBufVA(i);
        outBuf  = (unsigned char*)target->getBufVA(i);

        src1Strides   = pSrc->getBufStridesInBytes(i);
        outBufStrides = target->getBufStridesInBytes(i);

        src1BufSize   = pSrc->getBufSizeInBytes(i);
        outBufSize    = target->getBufSizeInBytes(i);

        if (src1Strides == outBufStrides)
            memcpy(outBuf, src1Buf, std::min(src1BufSize, outBufSize));
        else {
            unsigned stride = std::min(src1Strides, outBufStrides);
            unsigned height = std::min(src1BufSize/src1Strides, outBufSize/outBufStrides);
            for( unsigned y = 0; y < height; ++y )
                memcpy(outBuf+y*outBufStrides, src1Buf+y*src1Strides, stride);
        }
    }
    MY_LOGD_IF(ret != OK, "ret = %d", ret);
    return ret;
}

}  // namespace NSFeaturePipe
}  // namespace NSCamFeature
}  // namespace NSCam
