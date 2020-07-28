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
#define LOG_TAG "HDRProvider"
//
#include <mtkcam/utils/std/Log.h>
#include <mtkcam/utils/std/ULog.h>
//
#include <stdlib.h>
#include <utils/Errors.h>
#include <utils/List.h>
#include <utils/RefBase.h>
#include <utils/String8.h>
#include <sstream>
//
#include <mtkcam/utils/metadata/client/mtk_metadata_tag.h>
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>
//
//
#include <mtkcam/utils/imgbuf/IIonImageBufferHeap.h>
//
#include <mtkcam/drv/IHalSensor.h>
#include <mtkcam/utils/std/Format.h>
//
#include <mtkcam3/pipeline/hwnode/NodeId.h>

#include <mtkcam/utils/metastore/IMetadataProvider.h>
#include <mtkcam/utils/metastore/ITemplateRequest.h>
#include <mtkcam3/3rdparty/plugin/PipelinePlugin.h>
#include <mtkcam3/3rdparty/plugin/PipelinePluginType.h>
//
// HDR library
#include <mtkcam3/feature/hdr/IHDRProc2.h>
#include <mtkcam3/feature/hdr/HDRDefsCommon.h>
// MTKCAM
#include <mtkcam/aaa/IHal3A.h> // setIsp, CaptureParam_T
//
#include <mtkcam/utils/TuningUtils/FileDumpNamingRule.h> // tuning file naming
#include <sys/stat.h> // mkdir
#include <sys/prctl.h> //prctl set name
#include <mtkcam/drv/iopipe/SImager/ISImager.h>
#include <cutils/properties.h>
//
CAM_ULOG_DECLARE_MODULE_ID(MOD_LIB_HDR);
//
using namespace NSCam;
using namespace android;
using namespace std;
using namespace NSCam::NSPipelinePlugin;
using namespace NSCam::HDR2;
using namespace NS3Av3;
using namespace NSCam::TuningUtils;
using namespace NSIoPipe;
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
//
#define ASSERT(cond, msg)           do { if (!(cond)) { printf("Failed: %s\n", msg); return; } }while(0)
//
#define FUNCTION_IN                 MY_LOGD("%s +", __FUNCTION__)
#define FUNCTION_OUT                MY_LOGD("%s -", __FUNCTION__)
//systrace
#if 1
#ifndef ATRACE_TAG
#define ATRACE_TAG                           ATRACE_TAG_CAMERA
#endif
#include <utils/Trace.h>

#define HDR_TRACE_CALL()                      ATRACE_CALL()
#define HDR_TRACE_NAME(name)                  ATRACE_NAME(name)
#define HDR_TRACE_BEGIN(name)                 ATRACE_BEGIN(name)
#define HDR_TRACE_END()                       ATRACE_END()
#else
#define HDR_TRACE_CALL()
#define HDR_TRACE_NAME(name)
#define HDR_TRACE_BEGIN(name)
#define HDR_TRACE_END()
#endif

#define FUNCTION_SCOPE          auto __scope_logger__ = create_scope_logger(__FUNCTION__)

static std::shared_ptr<char> create_scope_logger(const char* functionName)
{
    bool bEnableLog = !!::property_get_int32("vendor.debug.camera.hdr.log", 0);
    char* pText = const_cast<char*>(functionName);
    MY_LOGD_IF(bEnableLog, "[%s] + ", pText);
    return std::shared_ptr<char>(pText, [bEnableLog](char* p){ MY_LOGD_IF(bEnableLog, "[%s] -", p); });
}

/******************************************************************************
*
******************************************************************************/
class HDRProviderImpl : public MultiFramePlugin::IProvider
{
    typedef MultiFramePlugin::Property Property;
    typedef MultiFramePlugin::Selection Selection;
    typedef MultiFramePlugin::Request::Ptr RequestPtr;
    typedef MultiFramePlugin::RequestCallback::Ptr RequestCallbackPtr;

public:

    virtual const Property& property()
    {
        CAM_ULOGM_APILIFE();
        static Property prop;
        static bool inited;

        if (!inited) {
            prop.mName = "THIRD_PARTY_HDR";
            prop.mFeatures = TP_FEATURE_HDR;
            prop.mThumbnailTiming = eTiming_P2;
            //Feature priority
            prop.mPriority = ePriority_Highest;
            prop.mZsdBufferMaxNum = 3; // maximum frames requirement
            inited = true;
        }
        return prop;
    };

    //if capture number is 4, "negotiate" would be called 4 times
    virtual MERROR negotiate(Selection& sel)
    {
        CAM_ULOGM_APILIFE();

        if(m_enable == 0) {
            MY_LOGD("Force off HDR");
            return BAD_VALUE;
        }

        // HDR is off on low ram devices
        bool isLowRamDevice = ::property_get_bool("ro.config.low_ram", false);
        if( isLowRamDevice ){
            MY_LOGI("low ram deivce, hdr is off.");
            return BAD_VALUE;
        }

        IMetadata* appInMeta = sel.mIMetadataApp.getControl().get();

        MUINT8 sceneMode = 0;
        bool ret = IMetadata::getEntry<MUINT8>(
                    appInMeta, MTK_CONTROL_SCENE_MODE, sceneMode);
        if(CC_UNLIKELY(!ret)){
            MY_LOGE("cannot get control scene mode");
            return BAD_VALUE;
        }
        if(sceneMode != MTK_CONTROL_SCENE_MODE_HDR) {
            if (m_enable == 1) {
                MY_LOGD("Force On HDR");
            } else {
                MY_LOGD("No need to execute HDR");
                return BAD_VALUE;
            }
        }

        if(sel.mRequestIndex == 0) {
            MINT32 sensorId = sel.mState.mSensorId;
            if ( !getCurrentCaptureParam(sensorId, true) ) {
                MY_LOGE("get capture param fail.");
                FUNCTION_OUT;
                return BAD_VALUE;
            }
        }

        sel.mRequestCount = m_captureNum;


        int inputFormat, outputFormat;

        if(m_algoType == YuvDomainHDR) {
            inputFormat = eImgFmt_NV21;
            outputFormat = eImgFmt_NV21;
        } else {
            inputFormat = eImgFmt_BAYER10_UNPAK;
            outputFormat = eImgFmt_BAYER10_UNPAK;
        }

        sel.mIBufferFull
            .setRequired(MTRUE)
            .addAcceptedFormat(inputFormat)
            .addAcceptedSize(eImgSize_Full);

        sel.mIMetadataDynamic.setRequired(MTRUE);
        sel.mIMetadataApp.setRequired(MTRUE);
        sel.mIMetadataHal.setRequired(MTRUE);
        //Only main frame has output buffer
        if (sel.mRequestIndex == 0) {
            sel.mOBufferFull
                .setRequired(MTRUE)
                .addAcceptedFormat(outputFormat)
                .addAcceptedSize(eImgSize_Full);

            sel.mOMetadataApp.setRequired(MTRUE);
            sel.mOMetadataHal.setRequired(MTRUE);
        } else {
            sel.mOBufferFull.setRequired(MFALSE);
            sel.mOMetadataApp.setRequired(MFALSE);
            sel.mOMetadataHal.setRequired(MFALSE);
        }

        // Without control metadata, it's no need to append additional metadata
        if (sel.mIMetadataApp.getControl() != NULL) {
            bool bLastFrame = sel.mRequestIndex == (m_captureNum-1)? 1 : 0;
            auto requestIndex = sel.mRequestIndex;

            MetadataPtr pAppAddtional = make_shared<IMetadata>();
            MetadataPtr pHalAddtional = make_shared<IMetadata>();

            IMetadata* pAppMeta = pAppAddtional.get();
            IMetadata* pHalMeta = pHalAddtional.get();

            const HDRCaptureParam& captureParam(m_CurrentCapParams[requestIndex]);

            IMetadata::Memory capParams;
            capParams.resize(sizeof(CaptureParam_T));
            memcpy(capParams.editArray(), &(captureParam.exposureParam), sizeof(CaptureParam_T));

            IMetadata::setEntry<IMetadata::Memory>(
                    pHalMeta, MTK_3A_AE_CAP_PARAM, capParams);

            // pause AF for (N - 1) frames and resume for the last frame
            IMetadata::setEntry<MUINT8>(
                    pHalMeta, MTK_FOCUS_PAUSE,
                    bLastFrame ? 0 : 1);
            IMetadata::setEntry<MUINT8>(
                    pHalMeta, MTK_HAL_REQUEST_REQUIRE_EXIF, 1);
            IMetadata::setEntry<MUINT8>(
                    pHalMeta, MTK_HAL_REQUEST_DUMP_EXIF, 1);
            if(m_algoType == RawDomainHDR) {
                sel.mDecision.mNeedUnpackRaw = true;
                IMetadata::setEntry<MINT32>(
                        pHalMeta, MTK_HAL_REQUEST_IMG_IMGO_FORMAT, eImgFmt_BAYER10_UNPAK);
            }
            // configure LTM per HDR input frame
       #if 0
            if (CC_LIKELY(captureParam.ltmMode != LTMMode::NONE))
            {
                IMetadata::setEntry<MINT32>(
                        pHalMeta, MTK_3A_ISP_BYPASS_LCE,
                        captureParam.bypassLTM());
            }
       #endif

            sel.mIMetadataApp.setAddtional(pAppAddtional);
            sel.mIMetadataHal.setAddtional(pHalAddtional);
        }

        // TODO: ADD HDR scene mode judge
        return OK;
    };

    virtual void init()
    {
        CAM_ULOGM_APILIFE();
    };

    virtual MERROR process(RequestPtr pRequest,
                           RequestCallbackPtr pCallback)
    {
        CAM_ULOGM_APILIFE();
        //set thread's name
        ::prctl(PR_SET_NAME, "HDRPlugin", 0, 0, 0);

        // restore callback function for abort API
        if(pCallback != nullptr) {
            m_callbackprt = pCallback;
        }

        if (pRequest->mIBufferFull != nullptr) {
            IImageBuffer* pIImgBuffer = pRequest->mIBufferFull->acquire();
            MY_LOGD("[IN] Full image VA: 0x%p size:(%d,%d) fmt: 0x%x planecount: %zu",
                    (void*)pIImgBuffer->getBufVA(0), pIImgBuffer->getImgSize().w, pIImgBuffer->getImgSize().h, pIImgBuffer->getImgFormat(), pIImgBuffer->getPlaneCount());
            if (m_dump){
                // dump input buffer
                String8 fileResultName;
                char    pre_filename[512];

                if (mkdir("/sdcard/camera_dump", 0777) && errno != EEXIST)
                    MY_LOGI("mkdir /sdcard/camera_dump");

                IMetadata* pHalMeta = pRequest->mIMetadataHal->acquire();

                extract(&m_dumpNamingHint, pHalMeta);
                MINT32 sensorId = pRequest->mSensorId;
                m_dumpNamingHint.SensorDev = sensorId;
                m_dumpNamingHint.IspProfile = 2; //EIspProfile_Capture;
                genFileName_TUNING(pre_filename, sizeof(pre_filename), &m_dumpNamingHint);

                fileResultName = String8::format("%s_Input_%d_%dx%d.raw"
                    , pre_filename
                    , pRequest->mRequestIndex
                    , pIImgBuffer->getImgSize().w
                    , pIImgBuffer->getImgSize().h);
                pIImgBuffer->saveToFile(fileResultName);
            }
        }

        if (pRequest->mOBufferFull != nullptr) {
            IImageBuffer* pOImgBuffer = pRequest->mOBufferFull->acquire();
            MY_LOGD("[OUT] Full image VA: 0x%p size:(%d,%d) fmt: 0x%x planecount: %zu",
                    (void*)pOImgBuffer->getBufVA(0), pOImgBuffer->getImgSize().w, pOImgBuffer->getImgSize().h, pOImgBuffer->getImgFormat(), pOImgBuffer->getPlaneCount());

            // throw the same input buffer back
            IImageBuffer* pIImgBuffer = nullptr;
            if (pRequest->mIBufferFull != nullptr) {
                pIImgBuffer = pRequest->mIBufferFull->acquire();
            } else {
                MY_LOGE("[IN] Full image is null!");
                return BAD_VALUE;
            }

            // copy input content to output
            if(m_algoType == YuvDomainHDR) {
                NSSImager::ISImager *pISImager = nullptr;
                pISImager = NSSImager::ISImager::createInstance(pIImgBuffer);
                if(CC_LIKELY(pISImager)) {
                    if (!pISImager->setTargetImgBuffer(pOImgBuffer)) {
                        MY_LOGW("setTargetImgBuffer failed!!!");
                    }
                    if (!pISImager->execute()) {
                        MY_LOGW("execute failed!!!");
                    }
                    pISImager->destroyInstance();
                }
            } else {
                for (size_t i = 0; i < pOImgBuffer->getPlaneCount(); i++)
                {
                    size_t planeBufSize = pOImgBuffer->getBufSizeInBytes(i);
                    MUINT8 *srcPtr = (MUINT8 *)pIImgBuffer->getBufVA(i);
                    void *dstPtr = (void *)pOImgBuffer->getBufVA(i);
                    memcpy(dstPtr, srcPtr, planeBufSize);
                }
            }
            if (m_dump){
                // dump output buffer
                String8 fileResultName;
                char    pre_filename[512];

                IMetadata* pHalMeta = nullptr;
                if (pRequest->mIMetadataHal != nullptr){
                    pHalMeta = pRequest->mIMetadataHal->acquire();
                    extract(&m_dumpNamingHint, pHalMeta);
                }
                MINT32 sensorId = pRequest->mSensorId;
                m_dumpNamingHint.SensorDev = sensorId;
                m_dumpNamingHint.IspProfile = 2; //EIspProfile_Capture;
                genFileName_TUNING(pre_filename, sizeof(pre_filename), &m_dumpNamingHint);

                fileResultName = String8::format("%s_Output_%d_%dx%d.yuv"
                    , pre_filename
                    , pRequest->mRequestIndex
                    , pOImgBuffer->getImgSize().w
                    , pOImgBuffer->getImgSize().h);
                pOImgBuffer->saveToFile(fileResultName);
            }
        }

        if (pRequest->mIMetadataDynamic != nullptr) {
            IMetadata *IMetaDynamic = pRequest->mIMetadataDynamic->acquire();
            if (IMetaDynamic != NULL)
                MY_LOGD("[IN] Dynamic metadata count: %d", IMetaDynamic->count());
            else
                MY_LOGD("[IN] Dynamic metadata Empty");
        }

        if (pRequest->mIMetadataHal != nullptr) {
            IMetadata *IMetahal = pRequest->mIMetadataHal->acquire();
            if (IMetahal != NULL) {
                MY_LOGD("[IN] IMetadataHal count: %d", IMetahal->count());
                if(CC_UNLIKELY(m_enablelog)) {
                    IMetadata::Memory capParams;
                    NS3Av3::CaptureParam_T exposureParam;
                    if (IMetadata::getEntry<IMetadata::Memory>(
                                    IMetahal, MTK_3A_AE_CAP_PARAM, capParams)){
                        memcpy(&exposureParam, capParams.editArray(), sizeof(CaptureParam_T));
                        MY_LOGD("=========RequestID:%d IMetadataHal =========",pRequest->mRequestIndex);
                        MY_LOGD("u4Eposuretime   (%u)", exposureParam.u4Eposuretime);
                        MY_LOGD("u4AfeGain       (%u)", exposureParam.u4AfeGain);
                        MY_LOGD("u4IspGain       (%u)", exposureParam.u4IspGain);
                    }
                }
            }
        }

        mvRequests.push_back(pRequest);
        MY_LOGD("collected request(%d/%d)",
                pRequest->mRequestIndex+1,
                pRequest->mRequestCount);

        if (pRequest->mRequestIndex == pRequest->mRequestCount - 1)
        {
            MY_LOGD("have collected all requests");
            for (auto req : mvRequests) {
                MY_LOGD("callback request(%d/%d) %p",
                        req->mRequestIndex+1,
                        req->mRequestCount, pCallback.get());
                if (pCallback != nullptr) {
                    pCallback->onCompleted(req, 0);
                }
            }
            mvRequests.clear();
        }

        return 0;
    };

    virtual void abort(vector<RequestPtr>& pRequests)
    {
        CAM_ULOGM_APILIFE();
        bool babort = false;
        if(m_callbackprt == nullptr)
            MY_LOGW("callbackptr is null");
        for (auto req : pRequests){
            babort = false;
            for (std::vector<RequestPtr>::iterator it = mvRequests.begin() ; it != mvRequests.end(); it++){
                if((*it) == req){
                    mvRequests.erase(it);
                    m_callbackprt->onAborted(req);
                    babort = true;
                    break;
                }
            }
            if (!babort){
                MY_LOGW("Desire abort request[%d] is not found", req->mRequestIndex);
            }
        }
        if(mvRequests.empty()) {
            MY_LOGD("abort() cleans all the requests");
        } else {
            MY_LOGW("abort() does not clean all the requests");
        }
    };

    virtual void uninit()
    {
        CAM_ULOGM_APILIFE();
    };

    HDRProviderImpl()
        : mvCapParam()
        , m_CurrentCapParams()
        , m_DelayCapParams()
        , m_captureNum(0)
        , m_delayFrame(0)
        , m_dump(0)
        , m_enable(-1)
        , m_enablelog(0)
        , m_algoType(0)
        , m_callbackprt()
        , m_dumpNamingHint()
        , mvRequests()
    {
        CAM_ULOGM_APILIFE();
        // enable HDR plugin
        m_enable = ::property_get_int32("vendor.debug.camera.hdr.enable", -1);
        // enable input/output dump buffer
        m_dump = ::property_get_int32("vendor.debug.camera.hdr.dump", 0);
        // enable print debug log
        m_enablelog = ::property_get_int32("vendor.debug.camera.hdr.log", 0);
        // HDR algo format, 0: YuvDomainHDR, 1: RawDomainHDR
        m_algoType = ::property_get_int32("vendor.camera.hdr.type", 0);
    };

    virtual ~HDRProviderImpl()
    {
        CAM_ULOGM_APILIFE();
    };

private:
    inline void setCaptureNum(int c) noexcept { m_captureNum = c; }
    inline void setDelayFrameNum(MINT32 d) noexcept { m_delayFrame = d; }

    bool getCurrentCaptureParam(MINT32 sensorId, bool bZsdFlow)
    {
        CAM_ULOGM_FUNCLIFE();
        std::unique_ptr <
                        IHal3A,
                        std::function<void(IHal3A*)>
                        > hal3a
                (
                    MAKE_Hal3A(sensorId, "HDR provider"),
                    [](IHal3A* p){ if (p) p->destroyInstance("HDR provider"); }
                );

        if (hal3a.get() == nullptr) {
            MY_LOGE("create IHal3A instance failed");
            return false;
        }

        {
            static std::mutex __locker;
            std::lock_guard<std::mutex> __l(__locker);
        }

        // get exposure setting from 3A
        ExpSettingParam_T rExpSetting;
        hal3a->send3ACtrl(
                E3ACtrl_GetExposureInfo, reinterpret_cast<MINTPTR>(&rExpSetting), 0);

        // Remove hard code
        int inputFrameCount = 3;

        // query the current 3A information
        HDRCaptureParam tmpCap3AParam;
        hal3a->send3ACtrl(
                E3ACtrl_GetExposureParam,
                reinterpret_cast<MINTPTR>(&tmpCap3AParam.exposureParam), 0);

        MUINT32 delayedFrames = 0;
        if ( bZsdFlow && inputFrameCount>1 ) {
            hal3a->send3ACtrl(
                    E3ACtrl_GetCaptureDelayFrame,
                    reinterpret_cast<MINTPTR>(&delayedFrames), 0);
        }


        MY_LOGD("HDR input frames(%d) delayed frames(%u)",
            inputFrameCount, delayedFrames);

        dumpCaptureParam(tmpCap3AParam, "Original ExposureParam");

        // update 3A information with information from HDR proc
        m_CurrentCapParams.resize(inputFrameCount);
        for (MINT32 i = 0; i < inputFrameCount; i++)
        {
            HDRCaptureParam& captureParam(m_CurrentCapParams.at(i));

            // copy original capture parameter
            captureParam = tmpCap3AParam;
        #if 0
            // adjust exposure settings
            CaptureParam_T& modifiedCapExpParam(captureParam.exposureParam);
            const CaptureParam_T& capExpParam(vCapParam.at(i).exposureParam);
            modifiedCapExpParam.u4Eposuretime  = capExpParam.u4Eposuretime;
            modifiedCapExpParam.u4AfeGain      = capExpParam.u4AfeGain;
            modifiedCapExpParam.u4IspGain      = capExpParam.u4IspGain;
            modifiedCapExpParam.u4FlareOffset  = capExpParam.u4FlareOffset;
        #endif

            // update ISP conditions
            captureParam.ltmMode = LTMMode::OFF;//vCapParam.at(i).ltmMode;

            String8 str;
            str.appendFormat("Modified ExposureParam[%d]", i);
            dumpCaptureParam(captureParam, str.string());

            m_CurrentCapParams.push_back(captureParam);
        }
        for (MUINT32 i = 0; i < delayedFrames; i++)
            m_DelayCapParams.push_back(tmpCap3AParam);

        setCaptureNum(inputFrameCount);
        setDelayFrameNum(delayedFrames);

        return true;
    }
    void dumpCaptureParam(
        const HDRCaptureParam& captureParam, const char* msg)
    {
        const NS3Av3::CaptureParam_T& capExpParam(captureParam.exposureParam);
        if (!msg) msg = "";
        MY_LOGD("========= %s =========", msg);
        MY_LOGD("u4ExposureMode  (%u)", capExpParam.u4ExposureMode);
        MY_LOGD("u4Eposuretime   (%u)", capExpParam.u4Eposuretime);
        MY_LOGD("u4AfeGain       (%u)", capExpParam.u4AfeGain);
        MY_LOGD("u4IspGain       (%u)", capExpParam.u4IspGain);
        MY_LOGD("u4RealISO       (%u)", capExpParam.u4RealISO);
        MY_LOGD("u4FlareGain     (%u)", capExpParam.u4FlareGain);
        MY_LOGD("u4FlareOffset   (%u)", capExpParam.u4FlareOffset);
        MY_LOGD("i4LightValue_x10(%d)", capExpParam.i4LightValue_x10);
        MY_LOGD("bypassLTM       (%d)", captureParam.bypassLTM());
    }
    //
    std::vector<HDRCaptureParam> mvCapParam;
    // capture param for hdr capture
    std::vector<HDRCaptureParam> m_CurrentCapParams;
    std::vector<HDRCaptureParam> m_DelayCapParams;
    int                          m_captureNum;
    int                          m_delayFrame;
    int                          m_dump;
    int                          m_enable;
    int                          m_enablelog;
    int                          m_algoType;
    RequestCallbackPtr           m_callbackprt;
    // file dump hint
    FILE_DUMP_NAMING_HINT        m_dumpNamingHint;
    // collect request
    std::vector<RequestPtr>      mvRequests;
};

REGISTER_PLUGIN_PROVIDER(MultiFrame, HDRProviderImpl);

