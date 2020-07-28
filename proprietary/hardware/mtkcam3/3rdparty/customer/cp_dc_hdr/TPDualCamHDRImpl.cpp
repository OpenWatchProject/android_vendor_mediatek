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

#define LOG_TAG "PipelinePlugin/TPDualCamHDRPrivider"

// Standard C header file
#include <stdlib.h>
#include <chrono>
#include <random>
#include <thread>
#include <vector>
#include <functional>
#include <mutex>
#include <sstream>
#include <iostream>
#include <algorithm>
// Android system/core header file

// mtkcam custom header file

// mtkcam global header file
#include <mtkcam/utils/std/Log.h>
#include <mtkcam/utils/std/ULog.h>
// Module header file
#include <mtkcam/aaa/IHal3A.h>
#include <mtkcam/drv/iopipe/SImager/IImageTransform.h>
#include <mtkcam/utils/metastore/IMetadataProvider.h>
#include <mtkcam3/3rdparty/plugin/PipelinePlugin.h>
#include <mtkcam3/3rdparty/plugin/PipelinePluginType.h>
//
#include <mtkcam/utils/metadata/client/mtk_metadata_tag.h>
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>

CAM_ULOG_DECLARE_MODULE_ID(MOD_LIB_HDR);
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
#define MY_LOGV_IF(cond, ...)       do { if( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)


#define TO_ONE_BASED_NUMBERING(X) ((X) + 1)
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
* Used Namespace.
********************************************************************************/
using namespace std;
//
using namespace android;
//
using namespace NSCam;
using namespace NS3Av3;
using namespace NSCam::NSPipelinePlugin;
using namespace NSCam::NSIoPipe::NSSImager;


/*******************************************************************************
* Type Alias.
********************************************************************************/
using Property = MultiFramePlugin::Property;
using Selection = MultiFramePlugin::Selection;
using RequestPtr = MultiFramePlugin::Request::Ptr;
using RequestCallbackPtr = MultiFramePlugin::RequestCallback::Ptr;
//
template<typename T>
using AutoPtr             = std::unique_ptr<T, std::function<void(T*)>>;
//
using IHal3APtr           = AutoPtr<IHal3A>;
using ImgPtr              = AutoPtr<IImageBuffer>;
using MetaPtr             = AutoPtr<IMetadata>;
using ImageTransformPtr   = AutoPtr<IImageTransform>;


/*******************************************************************************
* Namespace Start.
********************************************************************************/
namespace { // anonymous namespace

/*******************************************************************************
* Global Variales.
********************************************************************************/
constexpr MUINT32   FRAME_COUNT                          = 2;

/*******************************************************************************
* Global Functions.
********************************************************************************/
inline ostream& operator<<(std::ostream& out, const CaptureParam_T& cp)
{
    return out << "CaptureParam_T: " << std::endl
               << "================================" << std::endl
               << "exposureMode: "  << cp.u4ExposureMode   << std::endl
               << "eposuretime: "   << cp.u4Eposuretime    << std::endl
               << "afeGain: "       << cp.u4AfeGain        << std::endl
               << "ispGain: "       << cp.u4IspGain        << std::endl
               << "realISO: "       << cp.u4RealISO        << std::endl
               << "flareOffset: "   << cp.u4FlareOffset    << std::endl
               << "flareGain: "     << cp.u4FlareGain      << std::endl
               << "lightValue_x10: "<< cp.i4LightValue_x10;
}

inline ostream& operator<<(std::ostream& out, const RequestPtr& requestPtr)
{
    out << "RequestPtr: " << std::endl
        << "================================" << std::endl;

    if( requestPtr.get() == nullptr )
    {
        return out << "address: nullptr";
    }
    return out << "address: "           << requestPtr.get() << std::endl
               << "requestIndex: "      << TO_ONE_BASED_NUMBERING(requestPtr->mRequestIndex) << std::endl
               << "requestCount: "      << static_cast<MINT32>(requestPtr->mRequestCount) << std::endl
               << "inBufferFull: "      << requestPtr->mIBufferFull.get()<< std::endl
               << "inBufferSpecified: " << requestPtr->mIBufferSpecified.get() << std::endl
               << "inBufferLCS: "       << requestPtr->mIBufferLCS.get() << std::endl
               << "inputBufferFull: "   << requestPtr->mOBufferFull.get() << std::endl
               << "inMetadataDynamic: " << requestPtr->mIMetadataDynamic.get() << std::endl
               << "inMetadataApp: "     << requestPtr->mIMetadataApp.get() << std::endl
               << "inMetadataHal: "     << requestPtr->mIMetadataHal.get() << std::endl
               << "outMetadataApp: "    << requestPtr->mOMetadataApp.get() << std::endl
               << "outMetadataHal: "    << requestPtr->mOMetadataHal.get();
}


/*******************************************************************************
* Class Definition
********************************************************************************/
/**
 * @brief third party dualcam hdr algo. provider
 */
class ThirdPartyDualCamHDRProvider final: public MultiFramePlugin::IProvider
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Instantiation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    ThirdPartyDualCamHDRProvider();

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  MultiFramePlugin::IProvider Public Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:

    const Property& property() override;

    MERROR negotiate(Selection& sel) override;

    void init() override;

    MERROR process(RequestPtr requestPtr, RequestCallbackPtr callbackPtr) override;

    void abort(vector<RequestPtr>& requestPtrs) override;

    void uninit() override;

    ~ThirdPartyDualCamHDRProvider();

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  ThirdPartyDualCamHDRProvider Private Type.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    struct MyRequest
    {
        RequestPtr          mRequestPtr;
        RequestCallbackPtr  mCallbackPtr;
        MERROR              mStatus;
    };

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  ThirdPartyDualCamHDRProvider Private Operator.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    MERROR processDone(const RequestPtr& requestPtr, const RequestCallbackPtr& callbackPtr, MERROR status);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  ThirdPartyDualCamHDRProvider Private Data Members.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    struct FlowControl
    {
        MINT8  mEnable;
        MBOOL  mIsNeedCurrentFD;
        MBOOL  mIsSupportThumbnail;
        MUINT8 mInitPhase;
        MBOOL  mIsDrawFace;
        MBOOL  mIsDrawDepth;

        FlowControl();
    };

private:
    FlowControl mFlowControl;

    //
    std::vector<MyRequest>      mCollectedRequests;
    std::vector<CaptureParam_T> mCaptureParams;
};
REGISTER_PLUGIN_PROVIDER(MultiFrame, ThirdPartyDualCamHDRProvider);
/**
 * @brief utility class
 */
class TPHDRImpUtility final
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Instantiation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    TPHDRImpUtility() = delete;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  MyUtilityr Public Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    static inline ImageTransformPtr createImageTransformPtr();

    static inline ImgPtr createImgPtr(BufferHandle::Ptr& hangle);

    static inline MetaPtr createMetaPtr(MetadataHandle::Ptr& hangle);

    static inline MVOID getCaptureParams(const IMetadata& matadata, CaptureParam_T& captureParam);

    static inline MVOID setCaptureParams(const CaptureParam_T& captureParam, IMetadata& matadata);

    static MBOOL getCurrentCaptureParams(MINT32 openId, MBOOL isZSLFlow, std::vector<CaptureParam_T>& captureParams);

    static inline MVOID dump(const CaptureParam_T& captureParam, const char* title);

    static inline MVOID dump(const RequestPtr& captureParam, const char* title);

    static inline MVOID drawRect(const ImgPtr& imptr, const MPoint& leftTop, const MPoint& rightBottom);
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  MyUtilityr Private Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    static inline IHal3APtr getHAL3APtr(MINT32 openId);

    static MBOOL isContaining(const MSize& region, const MPoint& leftTop, const MPoint& rightBottom);
};


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  TPHDRImpUtility implementation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
IHal3APtr
TPHDRImpUtility::
getHAL3APtr(MINT32 openId)
{
    return IHal3APtr(MAKE_Hal3A(openId, LOG_TAG), [](IHal3A* p)
        {
            p->destroyInstance(LOG_TAG);
        });
}

ImageTransformPtr
TPHDRImpUtility::
createImageTransformPtr()
{
    return ImageTransformPtr(IImageTransform::createInstance(), [](IImageTransform *p)
    {
        p->destroyInstance();
    });
}

ImgPtr
TPHDRImpUtility::
createImgPtr(BufferHandle::Ptr& hangle)
{
    return ImgPtr(hangle->acquire(), [hangle](IImageBuffer* p)
    {
        UNREFERENCED_PARAMETER(p);
        hangle->release();
    });
};

MetaPtr
TPHDRImpUtility::
createMetaPtr(MetadataHandle::Ptr& hangle)
{
    return MetaPtr(hangle->acquire(), [hangle](IMetadata* p)
    {
        UNREFERENCED_PARAMETER(p);
        hangle->release();
    });
};

MVOID
TPHDRImpUtility::
getCaptureParams(const IMetadata& matadata, CaptureParam_T& captureParam)
{
    IMetadata::Memory captureMemory;
    captureMemory.resize(sizeof(CaptureParam_T));
    IMetadata::getEntry<IMetadata::Memory>(&matadata, MTK_3A_AE_CAP_PARAM, captureMemory);
    memcpy(&captureParam, captureMemory.editArray(), sizeof(CaptureParam_T));
}

MVOID
TPHDRImpUtility::
setCaptureParams(const CaptureParam_T& captureParam, IMetadata& matadata)
{
    IMetadata::Memory captureMemory;
    captureMemory.resize(sizeof(CaptureParam_T));
    memcpy(captureMemory.editArray(), &captureParam, sizeof(CaptureParam_T));
    IMetadata::setEntry<IMetadata::Memory>(&matadata, MTK_3A_AE_CAP_PARAM, captureMemory);
}

MBOOL
TPHDRImpUtility::
getCurrentCaptureParams(MINT32 openId, MBOOL isZSLFlow, std::vector<CaptureParam_T>& captureParams)
{
    SCOPED_TRACER();

    auto hal3APtr = TPHDRImpUtility::getHAL3APtr(openId);
    if( hal3APtr == nullptr )
    {
        MY_LOGE("falied to get hal 3A");
        return MFALSE;
    }
    //
    {
        static std::mutex __locker;
        std::lock_guard<std::mutex> __l(__locker);

        // clean output argument
        captureParams.clear();
        // get exposure setting from 3A
        ExpSettingParam_T expSetting;
        hal3APtr->send3ACtrl(E3ACtrl_GetExposureInfo, reinterpret_cast<MINTPTR>(&expSetting), 0);
        // query the current 3A information
        CaptureParam_T captureParam;
        hal3APtr->send3ACtrl(E3ACtrl_GetExposureParam, reinterpret_cast<MINTPTR>(&captureParam), 0);
        dump(captureParam, "original captureParam");
        //
        const MUINT32 inputFrameCount = FRAME_COUNT;
        MUINT32 delayedFrameCount = 0;
        if( isZSLFlow && (FRAME_COUNT > 1) )
        {
            hal3APtr->send3ACtrl(E3ACtrl_GetCaptureDelayFrame, reinterpret_cast<MINTPTR>(&delayedFrameCount), 0);
        }
        MY_LOGD("inputFrameCount:%d, delayedFrameCount:%u", inputFrameCount, delayedFrameCount);
        //
        for(MUINT32 i = 0; i < inputFrameCount; i++)
        {
            CaptureParam_T temp = captureParam;
            // TODO: set the desired 3a parameters
            // temp.u4Eposuretime  = captureParam.u4Eposuretime;
            // temp.u4AfeGain      = captureParam.u4AfeGain;
            // temp.u4IspGain      = captureParam.u4IspGain;
            // temp.u4FlareOffset  = captureParam.u4FlareOffset;
            // TODO: could we need to update ISP conditions?
            // temp.ltmMode = LTMMode::OFF;
            //
            // MY_LOGD("update captureParams, frame#:%d", i);
            // dump(captureParam, "new captureParam");
            captureParams.push_back(temp);
        }
    }
    return MTRUE;
}

MVOID
TPHDRImpUtility::
drawRect(const ImgPtr& imptr, const MPoint& leftTop, const MPoint& rightBottom)
{
    static const char intensity = 255;
    char* pBufferVa = reinterpret_cast<char*>(imptr->getBufVA(0));
    const MSize imgSize = imptr->getImgSize();
    const MUINT32 stride = imptr->getBufStridesInBytes(0);

    if(!isContaining(imgSize, leftTop, rightBottom))
    {
        MY_LOGW("retangle doesn't include in the image region, imageRegion:(0, 0, %d, %d), leftTop:(%d, %d), rightBottom:(%d, %d)",
            imgSize.w, imgSize.h, leftTop.x, leftTop.y, rightBottom.x, rightBottom.y);
    }
    else
    {
        // draw line from top-left to top-right
        memset(pBufferVa + stride*leftTop.y + leftTop.x, intensity, rightBottom.x - leftTop.x);
        // draw line from bottom-left to bottom-right
        memset(pBufferVa + stride*rightBottom.y + leftTop.x, intensity, rightBottom.x - leftTop.x);
        // draw left and right vertical line
        for (MINT32 j = leftTop.y; j < rightBottom.y ; j++)
        {
            *(pBufferVa + stride*j + leftTop.x) = intensity;
            *(pBufferVa + stride*j + rightBottom.x) = intensity;
        }
    }
}

MBOOL
TPHDRImpUtility::
isContaining(const MSize& region, const MPoint& leftTop, const MPoint& rightBottom)
{
    MRect regionRect(MPoint(0, 0), region);
    MRect checkedRect(leftTop, rightBottom);

    return (regionRect.p.x <= checkedRect.p.x) &&
           ((checkedRect.p.x + checkedRect.s.w) <= (regionRect.p.x + regionRect.s.w)) &&
           (regionRect.p.y <= checkedRect.p.y) &&
           ((checkedRect.p.y + checkedRect.s.h) <= (regionRect.p.y + regionRect.s.h));
}

MVOID
TPHDRImpUtility::
dump(const CaptureParam_T& captureParam, const char* title)
{
    std::stringstream ss;
    ss << "title: " << title << std::endl
       << "********************************" << std::endl
       << captureParam << std::endl
       << "********************************" << std::endl;

    MY_LOGD("%s", ss.str().c_str());
}

MVOID
TPHDRImpUtility::
dump(const RequestPtr& captureParam, const char* title)
{
    std::stringstream ss;
    ss << "title: " << title << std::endl
       << "********************************" << std::endl
       << captureParam << std::endl
       << "********************************" << std::endl;

    MY_LOGD("%s", ss.str().c_str());
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  ThirdPartyDualCamHDRProvider implementation.
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
ThirdPartyDualCamHDRProvider::FlowControl::
FlowControl()
: mEnable(-1)
, mIsNeedCurrentFD(MFALSE)
, mIsSupportThumbnail(MFALSE)
, mInitPhase(ePhase_OnRequest)
, mIsDrawFace(MFALSE)
, mIsDrawDepth(MFALSE)
{
    // on:1/off:0/auto:-1
    mEnable = ::property_get_int32("vendor.debug.camera.tp.dchdr.enable", mEnable);
    mIsNeedCurrentFD = ::property_get_bool("vendor.debug.camera.tp.dchdr.currentfd", mIsNeedCurrentFD);
    mIsSupportThumbnail = ::property_get_bool("vendor.debug.camera.tp.dchdr.thumbnail", mIsSupportThumbnail);
    mInitPhase = ::property_get_int32("vendor.debug.camera.tp.dchdr.initphase", mInitPhase);
    mIsDrawFace = ::property_get_bool("vendor.debug.camera.tp.dchdr.draw.face", mIsDrawFace);
    mIsDrawDepth = ::property_get_bool("vendor.debug.camera.tp.dchdr.draw.depth", mIsDrawDepth);
    MY_LOGD("enable:%d, needCurrentFD:%d, supportThumbnail:%d, initPhase:%u, drawFace:%d, drawDepth:%d",
        mEnable, mIsNeedCurrentFD, mIsSupportThumbnail, mInitPhase, mIsDrawFace, mIsDrawDepth);
}

ThirdPartyDualCamHDRProvider::
ThirdPartyDualCamHDRProvider()
{
    MY_LOGD("ctor:%p", this);
}

const Property&
ThirdPartyDualCamHDRProvider::
property()
{
    static const Property prop = [this]() -> const Property
    {
        Property ret;
        ret.mName = "TP_DC_HDR";
        ret.mFeatures = TP_FEATURE_HDR_DC;
        ret.mThumbnailTiming = (mFlowControl.mIsSupportThumbnail ? eTiming_MultiFrame : eTiming_P2);
        ret.mFaceData = (mFlowControl.mIsNeedCurrentFD ? eFD_Current : eFD_Cache);
        ret.mInitPhase = (mFlowControl.mInitPhase == ePhase_OnRequest ? ePhase_OnRequest : ePhase_OnPipeInit);
        ret.mPriority = ePriority_Highest;
        // maximum frames requirement
        ret.mZsdBufferMaxNum = FRAME_COUNT;
        return ret;
    }();
    return prop;
}

MERROR
ThirdPartyDualCamHDRProvider::
negotiate(Selection& sel)
{
    SCOPED_TRACER();

    IMetadata* appInMeta = sel.mIMetadataApp.getControl().get();
    if( appInMeta == nullptr )
    {
        MY_LOGE("failed to get app in metadata");
        return BAD_VALUE;
    }
    //
    if( mFlowControl.mEnable == 0 )
    {
        MY_LOGD("force off tp dualcam hdr off");
        return BAD_VALUE;
    }
    else if( mFlowControl.mEnable == 1 )
    {
        MY_LOGD("force tp dualcam hdr on");
    }
    else
    {
        MUINT8 sceneMode = 0;
        bool ret = IMetadata::getEntry<MUINT8>(appInMeta, MTK_CONTROL_SCENE_MODE, sceneMode);
        if( !ret ){
            MY_LOGE("cannot get control scene mode");
            return BAD_VALUE;
        }
        if( sceneMode != MTK_CONTROL_SCENE_MODE_HDR ) {
            MY_LOGD("not dualcam hdr mode(%d), no need to trigger, current scene mode:%d", MTK_CONTROL_SCENE_MODE_HDR, sceneMode);
            return BAD_VALUE;
        }
    }
    //
    const MUINT8 frameIndex = sel.mRequestIndex;
    if( frameIndex == 0 )
    {
        MY_LOGD("reset captureParams container, size:%zu", mCaptureParams.size());
        mCaptureParams.clear();

        // get capture parameters for each sub-frame
        MINT32 sensorId = sel.mState.mSensorId;
        if( !TPHDRImpUtility::getCurrentCaptureParams(sensorId, MTRUE, mCaptureParams) )
        {
            MY_LOGE("failed to set capture params");
            return BAD_VALUE;
        }
    }
    //
    sel.mRequestCount = FRAME_COUNT;
    // INPUT
    {
        sel.mIBufferFull
            .setRequired(MTRUE)
            .addAcceptedFormat(eImgFmt_NV21)
            .addAcceptedSize(eImgSize_Full);

        sel.mIMetadataApp.setRequired(MTRUE);
        sel.mIMetadataHal.setRequired(MTRUE);
        sel.mIMetadataDynamic.setRequired(MTRUE);
    }
    // OUTPUT
    {
        // note: only main frame has output buffer
        if( frameIndex == 0 )
        {
            sel.mOBufferFull
                .setRequired(MTRUE)
                .addAcceptedFormat(eImgFmt_NV21)
                .addAcceptedSize(eImgSize_Full);

            sel.mOMetadataApp.setRequired(MTRUE);
            sel.mOMetadataHal.setRequired(MTRUE);
        }
        else
        {
            sel.mOBufferFull.setRequired(MFALSE);
            sel.mOMetadataApp.setRequired(MFALSE);
            sel.mOMetadataHal.setRequired(MFALSE);
        }
    }
    //
    {
        if( frameIndex < mCaptureParams.size() )
        {
            const MBOOL isLastFrame = (frameIndex == (FRAME_COUNT - 1));
            MetadataPtr appAddtionalPtr = make_shared<IMetadata>();
            MetadataPtr halAddtionalPtr = make_shared<IMetadata>();
            IMetadata* pHalMeta = halAddtionalPtr.get();
            // set 3a capture params
            const CaptureParam_T captureParams = mCaptureParams[frameIndex];
            TPHDRImpUtility::setCaptureParams(captureParams, *pHalMeta);
            // is not the last frame, we pause the focus
            IMetadata::setEntry<MUINT8>(pHalMeta, MTK_FOCUS_PAUSE, !isLastFrame);
            IMetadata::setEntry<MUINT8>(pHalMeta, MTK_HAL_REQUEST_REQUIRE_EXIF, 1);
            IMetadata::setEntry<MUINT8>(pHalMeta, MTK_HAL_REQUEST_DUMP_EXIF, 1);
            // TODO: we need to do it? configure LTM per HDR input frame
            // if(captureParams.ltmMode != LTMMode::NONE)
            // {
            //     IMetadata::setEntry<MINT32>(pHalMeta, MTK_3A_ISP_BYPASS_LCE, captureParam.bypassLTM());
            // }
            //
            sel.mIMetadataApp.setAddtional(appAddtionalPtr);
            sel.mIMetadataHal.setAddtional(halAddtionalPtr);
        }
        else
        {
            MY_LOGE("captureParams is not existing, frameIndex:%d, captureParamsSize:%zu",
                frameIndex, mCaptureParams.size());
            return BAD_VALUE;
        }
    }

    return OK;
}

void
ThirdPartyDualCamHDRProvider::
init()
{
    SCOPED_TRACER();
    ::srand(time(NULL));
}

MERROR
ThirdPartyDualCamHDRProvider::
process(RequestPtr requestPtr, RequestCallbackPtr callbackPtr)
{
    SCOPED_TRACER();

    const MUINT8 frameIndex = requestPtr->mRequestIndex;
    const MUINT8 frameCount = requestPtr->mRequestCount;
    const MBOOL isFristFrame = (frameIndex == 0);
    const MBOOL isLastFrame = ((frameIndex + 1) == frameCount);
    const size_t collectedCount = mCollectedRequests.size();

    mCollectedRequests.push_back({.mRequestPtr = requestPtr, .mCallbackPtr = callbackPtr, .mStatus = OK});
    MY_LOGD("frameIndex:%d, frameCount:%d, reqAddr:%p, callbackAddr:%p, collectedCount:%zu/%d",
        TO_ONE_BASED_NUMBERING(frameIndex), frameCount, requestPtr.get(), callbackPtr.get(), collectedCount, frameCount);

    auto isValidInput = [](const RequestPtr& requestPtr) -> MBOOL
    {
        const MBOOL ret = (requestPtr->mIBufferFull != nullptr)
                    && (requestPtr->mIMetadataApp != nullptr)
                    && (requestPtr->mIMetadataHal != nullptr)
                    && (requestPtr->mIMetadataDynamic != nullptr);
        if( !ret )
        {
             MY_LOGE("invalid request with input data, reqAddr:%p, inFullImg:%p, inAppMeta:%p, inHalMeta:%p, indyMeta:%p",
                requestPtr.get(),
                requestPtr->mIBufferFull.get(),
                requestPtr->mIMetadataApp.get(),
                requestPtr->mIMetadataHal.get(),
                requestPtr->mIMetadataDynamic.get());
        }
        return ret;
    };
    //
    auto isValidOutput = [isFristFrame](const RequestPtr& requestPtr) -> MBOOL
    {
        if( !isFristFrame )
            return MTRUE;

        const MBOOL ret = (requestPtr->mOBufferFull != nullptr)
                        && (requestPtr->mOMetadataApp != nullptr)
                        && (requestPtr->mOMetadataHal != nullptr);
        if( !ret )
        {
            MY_LOGE("invalid request with output data, reqAddr:%p, isFristFrame:%d, outFullImg:%p, outAppMeta:%p, outHalMeta:%p",
                    requestPtr.get(),
                    isFristFrame,
                    requestPtr->mOBufferFull.get(),
                    requestPtr->mOMetadataApp.get(),
                    requestPtr->mOMetadataHal.get());
        }
        return ret;
    };
    //
    auto tryGetRequestByFrameIndex = [](const std::vector<MyRequest> requests, MINT32 frameIndex, RequestPtr& retRequest) -> MBOOL
    {
        retRequest = nullptr;
        auto foundItem = std::find_if(requests.begin(), requests.end(), [frameIndex](MyRequest item) -> MBOOL
        {
            return (item.mRequestPtr->mRequestIndex == frameIndex);
        });

        if( foundItem != requests.end() )
        {
            retRequest = foundItem->mRequestPtr;
        }
        return (retRequest != nullptr);
    };
    //
    //
    if( !isValidInput(requestPtr) || !isValidOutput(requestPtr))
    {
        return processDone(requestPtr, callbackPtr, BAD_VALUE);
    }
    //
    {
        if( !isLastFrame )
        {
            MY_LOGD("is not last frame, no need to trigger process, addr:%p, frameIndex:%d/%d",
                requestPtr.get(), TO_ONE_BASED_NUMBERING(frameIndex), frameCount);
        }
        else
        {
            const MINT32 mainFrameIndex = 0;
            const MINT32 subFrameIndex = 1;
            RequestPtr mainRequestPtr = nullptr;
            RequestPtr subRequestPtr = nullptr;
            if( !tryGetRequestByFrameIndex(mCollectedRequests, mainFrameIndex, mainRequestPtr) )
            {
                MY_LOGE("failed to get mainRequestPt");
                return processDone(requestPtr, callbackPtr, BAD_VALUE);
            }
            if( !tryGetRequestByFrameIndex(mCollectedRequests, subFrameIndex, subRequestPtr) )
            {
                MY_LOGE("failed to get subRequestPtr");
                return processDone(requestPtr, callbackPtr, BAD_VALUE);
            }
            // process for output image
            // note: we can just call createXXXXPtr one time for a specified handle
            ImgPtr mainInputImgPtr = TPHDRImpUtility::createImgPtr(mainRequestPtr->mIBufferFull);
            ImgPtr mainOutputImgPtr = TPHDRImpUtility::createImgPtr(mainRequestPtr->mOBufferFull);
            MetaPtr mainInputHalMetaPtr = TPHDRImpUtility::createMetaPtr(mainRequestPtr->mIMetadataHal);
            MetaPtr subInputHalMetaPtr = TPHDRImpUtility::createMetaPtr(mainRequestPtr->mIMetadataHal);
            {
                AUTO_TIMER("process tp dualcam hdr");

                ImageTransformPtr imageTransformPtr = TPHDRImpUtility::createImageTransformPtr();
                if( imageTransformPtr.get() == nullptr )
                {
                    MY_LOGE("failed to get image transform");
                    return processDone(requestPtr, callbackPtr, BAD_VALUE);
                }
                //
                const MRect crop = MRect(mainInputImgPtr->getImgSize().w, mainInputImgPtr->getImgSize().h);
                if( !imageTransformPtr->execute(mainInputImgPtr.get(), mainOutputImgPtr.get(), nullptr, crop, 0, 3000) )
                {
                    MY_LOGE("failed to execute image transform(full)");
                    return processDone(requestPtr, callbackPtr, BAD_VALUE);
                }
                // make debug effect
                if( mFlowControl.mIsDrawDepth )
                {
                    const size_t outBufStrides = mainOutputImgPtr->getBufStridesInBytes(0);
                    const MSize& outBufSize = mainOutputImgPtr->getImgSize();
                    char* pOutBufVA = reinterpret_cast<char*>(mainOutputImgPtr->getBufVA(0));
                    for(MINT32 i = 0; i < outBufSize.h/2; i++)
                    {
                        ::memset(pOutBufVA, 125, outBufStrides);
                        pOutBufVA += outBufStrides;
                    }
                }
                mainOutputImgPtr->syncCache(eCACHECTRL_FLUSH);
            }
            // dump captureParams for debug
            {
                CaptureParam_T captureParam;
                if( mainInputHalMetaPtr.get() != nullptr )
                {
                    MY_LOGD("dump captureParams info., mainInHalAddr:%p", mainInputHalMetaPtr.get());
                    IMetadata::IEntry entryFaceRects = mainInputHalMetaPtr->entryFor(MTK_3A_AE_CAP_PARAM);
                    TPHDRImpUtility::getCaptureParams(*mainInputHalMetaPtr.get(), captureParam);
                    TPHDRImpUtility::dump(captureParam, "get params form mainInputHalMetaPtr in mainFrame");
                }
                if( subInputHalMetaPtr.get() != nullptr )
                {
                    MY_LOGD("dump captureParams info., subInHalAddr:%p", subInputHalMetaPtr.get());
                    IMetadata::IEntry entryFaceRects = subInputHalMetaPtr->entryFor(MTK_3A_AE_CAP_PARAM);
                    TPHDRImpUtility::getCaptureParams(*subInputHalMetaPtr.get(), captureParam);
                    TPHDRImpUtility::dump(captureParam, "get params form subInputHalMetaPtr in subFrame");
                }
            }
            // face data
            if( mFlowControl.mIsDrawFace )
            {
                IMetadata::IEntry entryFaceRects = mainInputHalMetaPtr->entryFor(MTK_FEATURE_FACE_RECTANGLES);
                const size_t faceDataCount = entryFaceRects.count();
                for(size_t i = 0; i < faceDataCount; i++)
                {
                    MRect faceRect = entryFaceRects.itemAt(i, Type2Type<MRect>());
                    MY_LOGD("face rectangle, index:%zu/%zu, leftTop:(%d, %d), rightBottom:(%d, %d)",
                        i, faceDataCount, faceRect.p.x, faceRect.p.y, faceRect.s.w, faceRect.s.h);
                    TPHDRImpUtility::drawRect(mainOutputImgPtr, faceRect.p, MPoint(faceRect.s.w, faceRect.s.h));
                }
                // pose oriens
                IMetadata::IEntry entryPoseOriens = mainInputHalMetaPtr->entryFor(MTK_FEATURE_FACE_POSE_ORIENTATIONS);
                const size_t poseOriensCount = entryPoseOriens.count();
                static const MINT32 poseOriensDataLength = 3;
                if(poseOriensCount%poseOriensDataLength != 0)
                {
                    MY_LOGW("invaild facePoseOriensCount, count:%zu, dataLenth:%d", poseOriensCount, poseOriensDataLength);
                }
                else
                {
                    const MINT32 dataSize = poseOriensCount/poseOriensDataLength;
                    for(MINT32 index = 0; index < dataSize; ++index)
                    {
                        const MINT32 poseX = entryPoseOriens.itemAt(index*poseOriensDataLength + 0, Type2Type<MINT32>());
                        const MINT32 poseY = entryPoseOriens.itemAt(index*poseOriensDataLength + 1, Type2Type<MINT32>());
                        const MINT32 poseZ = entryPoseOriens.itemAt(index*poseOriensDataLength + 2, Type2Type<MINT32>());
                        MY_LOGD("face pose, index:%d/%d, (x, y, z):(%d, %d, %d)",
                            index, dataSize, poseX, poseY, poseZ);
                    }
                }
            }
            //
            {
                std::default_random_engine generator;
                std::uniform_int_distribution<int> distribution(0, 500);
                const int randomTime = distribution(generator);
                const int baseTime = 500;
                auto sleepTime = std::chrono::microseconds(baseTime + randomTime);
                MY_LOGD("sleep for %lld ms", sleepTime.count());
                {
                    std::this_thread::sleep_for(sleepTime);
                }
                MY_LOGD("wait up");
            }
        }
    }
    return processDone(requestPtr, callbackPtr, OK);
}

MERROR
ThirdPartyDualCamHDRProvider::
processDone(const RequestPtr& requestPtr, const RequestCallbackPtr& callbackPtr, MERROR status)
{
    SCOPED_TRACER();

    const MUINT8 frameIndex = requestPtr->mRequestIndex;
    const MUINT8 frameCount = requestPtr->mRequestCount;
    MY_LOGD("request process done, reqAddr:%p, callbackPtr:%p, status:%d, frameIndex:%d, frameCount:%d",
        requestPtr.get(), callbackPtr.get(), status, TO_ONE_BASED_NUMBERING(frameIndex), frameCount);

    auto foundItem = std::find_if(mCollectedRequests.begin(), mCollectedRequests.end(), [&requestPtr](MyRequest item) -> MBOOL
    {
        return (item.mRequestPtr.get() == requestPtr.get());
    });

    if( foundItem != mCollectedRequests.end() )
    {
        foundItem->mStatus = status;
    }
    else
    {
        MY_LOGW("failed to find request to set status, reqAddr:%p", requestPtr.get());
    }

    const size_t collectedCount = mCollectedRequests.size();
    const MBOOL isLastFrame = ((frameIndex + 1) == frameCount);
    if( isLastFrame )
    {
        MY_LOGD("all process had be done, call complete for all frame, collectedCount:%zu", collectedCount);
        while( mCollectedRequests.begin() != mCollectedRequests.end() )
        {
            const MyRequest& item = mCollectedRequests.front();
            const RequestPtr& tmpReqPtr = item.mRequestPtr;
            const RequestCallbackPtr& cbReqPtr = item.mCallbackPtr;
            const MERROR status = item.mStatus;

            MY_LOGD("frame completed, frameIndex:%d, reqAddr:%p, status:%d",
                TO_ONE_BASED_NUMBERING(tmpReqPtr->mRequestIndex), tmpReqPtr.get(), status);

            if( cbReqPtr != nullptr )
            {
                cbReqPtr->onCompleted(tmpReqPtr, status);
            }
            mCollectedRequests.erase(mCollectedRequests.begin());
        }
    }
    // Note:
    // we always return OK in this function, and keep the status of frame in the container member of this class,
    // when collect all frame we will call the callback function to return all frames.
    return OK;
}

void
ThirdPartyDualCamHDRProvider::
abort(vector<RequestPtr>& requestPtrs)
{
    SCOPED_TRACER();

    bool babort = false;

    if (mCollectedRequests.size() > 0) {
        for(auto req : requestPtrs) {
            babort = false;
            auto foundItem = std::find_if(mCollectedRequests.begin(), mCollectedRequests.end(), [&req](MyRequest item) -> MBOOL
            {
                return (item.mRequestPtr.get() == req.get());
            });

            if( foundItem != mCollectedRequests.end() )
            {
                foundItem->mCallbackPtr->onAborted(foundItem->mRequestPtr);
                mCollectedRequests.erase(foundItem);
                babort = true;
            }
            if (!babort){
                MY_LOGE("Desire abort request[%d] is not found", req->mRequestIndex);
            }
        }
        if(mCollectedRequests.empty()) {
            MY_LOGD("abort() cleans all the requests");
        } else {
            MY_LOGW("abort() does not clean all the requests");
        }
    }
}

void
ThirdPartyDualCamHDRProvider::
uninit()
{
    SCOPED_TRACER();
}

ThirdPartyDualCamHDRProvider::
~ThirdPartyDualCamHDRProvider()
{
    MY_LOGD("dtor:%p", this);
}


}  // anonymous namespace
