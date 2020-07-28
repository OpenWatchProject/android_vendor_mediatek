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
#define LOG_TAG "RawHDRProvider"
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
    CAM_LOGD_IF(bEnableLog, "[%s] + ", pText);
    return std::shared_ptr<char>(pText, [bEnableLog](char* p){ CAM_LOGD_IF(bEnableLog, "[%s] -", p); });
}

template<typename T>
static MBOOL GET_ENTRY_ARRAY(const NSCam::IMetadata& metadata, MINT32 entry_tag, T* array, MUINT32 size)
{
    NSCam::IMetadata::IEntry entry = metadata.entryFor(entry_tag);
    if (entry.tag() != NSCam::IMetadata::IEntry::BAD_TAG &&
        entry.count() == size)
    {
        for (MUINT32 i = 0; i < size; i++)
        {
            *array++ = entry.itemAt(i, NSCam::Type2Type< T >());
        }
        return MTRUE;
    }
    return MFALSE;
}

/******************************************************************************
*
******************************************************************************/
class RawHDRPlugImpl : public MultiFramePlugin::IProvider
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
            prop.mName = "THIRD_PARTY_RawHDR";
            prop.mFeatures = TP_FEATURE_RAW_HDR;
            prop.mThumbnailTiming = eTiming_P2;
            prop.mFaceData = eFD_Cache;
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
        MBOOL result = MFALSE;

        result = IMetadata::getEntry<MUINT8>(
                    appInMeta, MTK_CONTROL_SCENE_MODE, sceneMode);

        if(sceneMode != MTK_CONTROL_SCENE_MODE_HDR) {
            if (m_enable == 1) {
                MY_LOGD("Force On HDR");
            } else {
                MY_LOGD("No need to execute HDR");
                return BAD_VALUE;
            }
        }

        MINT32 sensorId = sel.mState.mSensorId;
        mHal3A = std::unique_ptr< IHal3A, std::function<void(IHal3A*)> >
        (
             MAKE_Hal3A(sensorId, "RawHDRProvider"),
             [](IHal3A* p){ if (p) p->destroyInstance("RawHDRProvider"); }
        );

        if(sel.mRequestIndex == 0) {
            if ( !getCurrentCaptureParam(true) ) {
                MY_LOGE("get capture param fail.");
                FUNCTION_OUT;
                return BAD_VALUE;
            }
        }

        sel.mRequestCount = m_captureNum;


        int inputFormat, outputFormat;


        inputFormat  = eImgFmt_BAYER10_UNPAK;
        outputFormat = eImgFmt_BAYER10_UNPAK;


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
            {
                // Indicate it is raw HDR
                sel.mDecision.mNeedUnpackRaw = true;
                IMetadata::setEntry<MINT32>(
                        pHalMeta, MTK_HAL_REQUEST_IMG_IMGO_FORMAT, eImgFmt_BAYER10_UNPAK);
            }
            // Need to bypass lce
            {
                IMetadata::setEntry<MINT32>(
                        pHalMeta, MTK_3A_ISP_BYPASS_LCE,
                        1);
            }


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
        ::prctl(PR_SET_NAME, "RawHDRPlugin", 0, 0, 0);
        /*
        * Be aware of that metadata and buffer should acquire one time
        */
        IImageBuffer* pIImgBuffer = nullptr;
        IImageBuffer* pOImgBuffer = nullptr;
        // Get out metadata
        IMetadata* pAppMeta = pRequest->mIMetadataApp->acquire();
        IMetadata* IMetahal = pRequest->mIMetadataHal->acquire();
        IMetadata* IMetaDynamic = pRequest->mIMetadataDynamic->acquire();

        if (pRequest->mIMetadataApp != nullptr) {
            pAppMeta = pRequest->mIMetadataApp->acquire();
        }

        if (pRequest->mIMetadataHal != nullptr) {
            IMetahal = pRequest->mIMetadataHal->acquire();
        }

        if (pRequest->mIMetadataDynamic != nullptr) {
            IMetaDynamic = pRequest->mIMetadataDynamic->acquire();
        }

        // Check metadata
        if(CC_UNLIKELY(pAppMeta == nullptr)
            || CC_UNLIKELY(IMetahal == nullptr)
            || CC_UNLIKELY(IMetaDynamic == nullptr)) {
            MY_LOGE("one of metdata is null idx(%d)!!!", pRequest->mRequestIndex);
            return BAD_VALUE;
        }

        // restore callback function for abort API
        if(pCallback != nullptr) {
            m_callbackprt = pCallback;
        }

        if (pRequest->mIBufferFull != nullptr) {
            pIImgBuffer = pRequest->mIBufferFull->acquire();
            if(CC_UNLIKELY(pIImgBuffer == nullptr)) {
                MY_LOGE("Input buffer is null idx(%d)!!!", pRequest->mRequestIndex);
                return BAD_VALUE;
            }
            MY_LOGD("[IN] Full image VA: 0x%p fmt: 0x%x", (void*)pIImgBuffer->getBufVA(0), pIImgBuffer->getImgFormat());
            if(m_dump){
                // dump input buffer
                MINT32 sensorId = pRequest->mSensorId;
                bufferDump(sensorId, IMetahal, pIImgBuffer, pRequest->mRequestIndex, "input");
            }
        }

        if (pRequest->mOBufferFull != nullptr) {
            pOImgBuffer = pRequest->mOBufferFull->acquire();
            if(CC_UNLIKELY(pOImgBuffer == nullptr)) {
                MY_LOGE("Output buffer is null idx(%d)!!!", pRequest->mRequestIndex);
                return BAD_VALUE;
            }
            MY_LOGD("[OUT] Full image VA: 0x%p fmt: 0x%x",(void*)pOImgBuffer->getBufVA(0), pOImgBuffer->getImgFormat());

            // copy input content to output
            if(pIImgBuffer && pOImgBuffer) {
                for (size_t i = 0; i < pOImgBuffer->getPlaneCount(); i++)
                {
                    size_t planeBufSize = pOImgBuffer->getBufSizeInBytes(i);
                    MUINT8 *srcPtr = (MUINT8 *)pIImgBuffer->getBufVA(i);
                    void *dstPtr = (void *)pOImgBuffer->getBufVA(i);
                    memcpy(dstPtr, srcPtr, planeBufSize);
                }
            }

            if (m_dump){
                // dump input buffer
                MINT32 sensorId = pRequest->mSensorId;
                bufferDump(sensorId, IMetahal, pOImgBuffer, pRequest->mRequestIndex, "output");
            }

        }

        // Parsing algo Meta
        parseHalMeta(IMetaDynamic, IMetahal);
        // Get 3a Info
        get3aInfo();

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

    RawHDRPlugImpl()
    {
        CAM_ULOGM_APILIFE();

        m_enable = ::property_get_int32("vendor.debug.camera.hdr.enable", -1);
        m_dump = ::property_get_int32("vendor.debug.camera.hdr.dump", 0);
        // Need to set up
        m_captureNum = 0;
        m_delayFrame = 0;

        // enable print debug log
        m_enablelog = ::property_get_int32("vendor.debug.camera.hdr.log", 0);

    };

    virtual ~RawHDRPlugImpl()
    {
        CAM_ULOGM_APILIFE();
    };

private:
    inline void setCaptureNum(int c) noexcept { m_captureNum = c; }
    inline void setDelayFrameNum(MINT32 d) noexcept { m_delayFrame = d; }

    bool getCurrentCaptureParam(bool bZsdFlow)
    {
        CAM_ULOGM_FUNCLIFE();

        {
            static std::mutex __locker;
            std::lock_guard<std::mutex> __l(__locker);
        }

        // get exposure setting from 3A
        ExpSettingParam_T rExpSetting;
        mHal3A->send3ACtrl(
                E3ACtrl_GetExposureInfo, reinterpret_cast<MINTPTR>(&rExpSetting), 0);

        // Remove hard code
        int inputFrameCount = 3;

        // query the current 3A information
        HDRCaptureParam tmpCap3AParam;
        mHal3A->send3ACtrl(
                E3ACtrl_GetExposureParam,
                reinterpret_cast<MINTPTR>(&tmpCap3AParam.exposureParam), 0);

        MUINT32 delayedFrames = 0;
        if ( bZsdFlow && inputFrameCount>1 ) {
            mHal3A->send3ACtrl(
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

        if (msg) MY_LOGD("========= %s =========", msg);
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

    void bufferDump(MINT32 sensorId, IMetadata *halMeta, IImageBuffer* buff, int index, const char* ioStr){
        // dump input buffer
        String8                   fileResultName;
        char                      pre_filename[512];
        char*                     algoTypeStr = "raw";
        FILE_DUMP_NAMING_HINT     dumpNamingHint;

        if(!halMeta) {
            MY_LOGE("HalMeta is nullptr, dump fail");
            return;
        }

        extract(&dumpNamingHint, halMeta);
        dumpNamingHint.SensorDev = sensorId;
        dumpNamingHint.IspProfile = 2; //EIspProfile_Capture;
        genFileName_TUNING(pre_filename, sizeof(pre_filename), &dumpNamingHint);

        fileResultName = String8::format("%s_%s_%d_%dx%d.%s"
            , pre_filename
            , ioStr
            , index
            , buff->getImgSize().w
            , buff->getImgSize().h
            , algoTypeStr);
        buff->saveToFile(fileResultName);
    }
    //
    void parseHalMeta(IMetadata *appMeta, IMetadata *halMeta) {
        //Get faceInfo
        CAM_ULOGM_FUNCLIFE();
        IMetadata::IEntry entryFaceRects = halMeta->entryFor(MTK_FEATURE_FACE_RECTANGLES);
        const size_t faceDataCount = entryFaceRects.count();
        for(size_t i = 0; i < faceDataCount; i++)
        {
            MRect faceRect = entryFaceRects.itemAt(i, Type2Type<MRect>());
            MY_LOGD("face rectangle, index:%zu/%zu, leftTop:(%d, %d), rightBottom:(%d, %d)",
                i, faceDataCount, faceRect.p.x, faceRect.p.y, faceRect.s.w, faceRect.s.h);
            mvFaceRect.push_back(std::move(faceRect));
        }

        MRational NeutralColorPt[3];
        HDR_AWB_Gain awbGain;

        GET_ENTRY_ARRAY(*appMeta, MTK_SENSOR_NEUTRAL_COLOR_POINT, NeutralColorPt, 3);

        awbGain.rGain = NeutralColorPt[0].denominator;
        awbGain.gGain = NeutralColorPt[1].denominator;
        awbGain.bGain = NeutralColorPt[2].denominator;

        MY_LOGD("AWB gain(%d, %d, %d)" , NeutralColorPt[0].denominator
                                       , NeutralColorPt[1].denominator
                                       , NeutralColorPt[2].denominator);
        mAWB[0] = (float)awbGain.rGain / 512;
        mAWB[1] = (float)awbGain.gGain / 512; //G is 1
        mAWB[2] = (float)awbGain.bGain / 512;
    }

    void get3aInfo() {

        CAM_ULOGM_FUNCLIFE();
        ASDInfo_T ASDInfo;
        MINT32    ae_lv = 0;
        double    temp = 0;

        mHal3A->send3ACtrl( NS3Av3::E3ACtrl_GetAsdInfo,
            reinterpret_cast<MINTPTR>(&ASDInfo), 0);
        ae_lv = ASDInfo.i4AELv_x10;
        temp = ae_lv/10 - 3;
        // BV convert to lux index, lux index = 2^ (i4AELv_x10/10 -3)
        mLuxIndex = pow(2, temp);
        MY_LOGD("Lux index(%f), Scene(%d)", mLuxIndex, mScene);

        //Get Black level (OB offest) nvram range is 0~8192
        MINT32 tmp[4] = {0, 0, 0, 0};
        mHal3A->send3ACtrl(E3ACtrl_GetOBOffset, reinterpret_cast<MINTPTR>(tmp), 0);
        for(int i = 0; i < 4; i++)
        {
            mBlackLevel[i] = (float)(8192 - tmp[i]) / 4;
        }
        MY_LOGD("Black level(%d, %d, %d, %d)", mBlackLevel[0], mBlackLevel[1], mBlackLevel[2], mBlackLevel[3]);
    }
    std::vector<HDRCaptureParam> mvCapParam;
    // capture param for hdr capture
    std::vector<HDRCaptureParam> m_CurrentCapParams;
    std::vector<HDRCaptureParam> m_DelayCapParams;
    int                          m_captureNum;
    int                          m_delayFrame;
    int                          m_dump;
    int                          m_enable;
    int                          m_enablelog;
    RequestCallbackPtr           m_callbackprt;
    // file dump hint
    FILE_DUMP_NAMING_HINT        m_dumpNamingHint;
    // collect request
    std::vector<RequestPtr>      mvRequests;
    //
    std::vector<MRect>           mvFaceRect;
    double                      mLuxIndex = 0;
    MINT32                      mScene = 0;
    float                       mAWB[3] = {0}; // 0:R, 1:G, 2:B
    MINT32                      mBlackLevel[4] = {0};
    // Hal3A
    std::unique_ptr< NS3Av3::IHal3A, std::function<void(NS3Av3::IHal3A*)> >
        mHal3A;
};

REGISTER_PLUGIN_PROVIDER(MultiFrame, RawHDRPlugImpl);

