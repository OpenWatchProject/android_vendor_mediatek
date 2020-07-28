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
#define LOG_TAG "YUVEffectHal"
#define EFFECT_NAME "YUVEffect"
#define MAJOR_VERSION 0
#define MINOR_VERSION 1



#define UNUSED(x) (void)x

#include <cutils/log.h>
#include <cutils/properties.h>

#define MY_LOGV(fmt, arg...)        CAM_LOGV("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("[%s] " fmt, __FUNCTION__, ##arg)
 //
#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( (cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( (cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)
//
#include <mtkcam/feature/effectHalBase/IEffectHal.h>
#include <mtkcam/feature/effectHalBase/EffectHalBase.h>
//
#include <mtkcam/feature/stereo/effecthal/YUVEffectHal.h>
#include <mtkcam/feature/stereo/hal/stereo_setting_provider.h>
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>
#include <mtkcam/feature/eis/eis_ext.h>
//
#include <mtkcam/utils/std/Log.h>
#include <mtkcam/utils/std/Misc.h>
#include <mtkcam/utils/std/Trace.h>
#include <mtkcam/drv/def/Dip_Notify_datatype.h>
using namespace NSCam;
using namespace NSCam::NSIoPipe;
using namespace StereoHAL;

/******************************************************************************
 *
 ******************************************************************************/
#define FUNCTION_LOG_START
#define FUNCTION_LOG_END

#define EIS_REGION_META_MV_X_LOC 6
#define EIS_REGION_META_MV_Y_LOC 7
#define EIS_REGION_META_FROM_RRZ_LOC 8

#define RETRIEVE_OFRAMEINFO_IMGBUF_ERROR_RETURN(request, rFrameInfo, BufID, rImgBuf) \
if (request->vOutputFrameInfo.indexOfKey(BufID) >= 0) \
{ \
    rFrameInfo = request->vOutputFrameInfo.valueFor(BufID);\
    rFrameInfo->getFrameBuffer(rImgBuf); \
}\
else\
{\
    MY_LOGE("Cannot find the frameBuffer in the effect request, frameID=%d! Return!", BufID); \
    return MFALSE; \
}

//  ============================  updateEntry  ============================
template <typename T>
inline MVOID
updateEntry(
    IMetadata* pMetadata,
    MUINT32 const tag,
    T const& val
)
{
    if( pMetadata == NULL ) {
        MY_LOGW("pMetadata == NULL");
        return;
    }

    IMetadata::IEntry entry(tag);
    entry.push_back(val, Type2Type<T>());
    pMetadata->update(tag, entry);
}

//  ============================  tryGetMetadata  ============================
template <typename T>
inline MBOOL
tryGetMetadata(
    IMetadata* pMetadata,
    MUINT32 const tag,
    T & rVal
)
{
    if( pMetadata == NULL ) {
        MY_LOGW("pMetadata == NULL");
        return MFALSE;
    }
    //
    IMetadata::IEntry entry = pMetadata->entryFor(tag);
    if( !entry.isEmpty() ) {
        rVal = entry.itemAt(0, Type2Type<T>());
        return MTRUE;
    }
    return MFALSE;
}

namespace NSCam{
//************************************************************************
//
//************************************************************************
YUVEffectHal::
YUVEffectHal()
: miSensorIdx_Main1(-1),
  miSensorIdx_Main2(-1),
  mp3AHal_Main1(NULL),
  mp3AHal_Main2(NULL),
  mpINormalStream(NULL)
{
    char cLogLevel[PROPERTY_VALUE_MAX];
    ::property_get("vendor.debug.yuveffect.dump", cLogLevel, "0");
    miDump = ::atoi(cLogLevel);
    if(miDump>0)
    {
        msFilename = std::string("/sdcard/vsdof/Cap/yuveffectout/");
        NSCam::Utils::makePath(msFilename.c_str(), 0660);
    }

    ::property_get("vendor.debug.camera.log.yuveffecthal", cLogLevel, "0");
    mLogLevel = atoi(cLogLevel);
    if ( mLogLevel == 2 ) {
        mbDebugLog = MTRUE;
    }

    ::property_get("vendor.debug.yuveffecthal.off3Ainfo", cLogLevel, "0");
    miOff3AInfo = ::atoi(cLogLevel);
}
//************************************************************************
//
//************************************************************************
YUVEffectHal::
~YUVEffectHal()
{

}
//************************************************************************
//
//************************************************************************
bool
YUVEffectHal::
allParameterConfigured()
{

    return MTRUE;
}
//************************************************************************
//
//************************************************************************
MVOID
YUVEffectHal::
cleanUp()
{
    MY_LOGD("+");
    if(mpINormalStream != NULL)
    {
        mpINormalStream->uninit(LOG_TAG);
        mpINormalStream->destroyInstance();
        mpINormalStream = NULL;
    }

    if(mp3AHal_Main1)
    {
        mp3AHal_Main1->destroyInstance("YUV_EFFECTHAL_3A_MAIN1");
        mp3AHal_Main1 = NULL;
    }

    if(mp3AHal_Main2)
    {
        mp3AHal_Main2->destroyInstance("YUV_EFFECTHAL_3A_MAIN2");
        mp3AHal_Main2 = NULL;
    }

    MY_LOGD("-");
}
//************************************************************************
//
//************************************************************************
status_t
YUVEffectHal::
initImpl()
{
    FUNCTION_LOG_START;
    CAM_TRACE_BEGIN("YUVEffectHal::createInstance+init");

    // get sensor index
    if (!StereoSettingProvider::getStereoSensorIndex(miSensorIdx_Main1, miSensorIdx_Main2))
    {
        MY_LOGE("StereoSettingProvider getStereoSensorIndex failed!");
        return MFALSE;
    }

    mpINormalStream = NSCam::NSIoPipe::NSPostProc::INormalStream::createInstance(miSensorIdx_Main1);
    if (mpINormalStream == NULL)
    {
        MY_LOGE("mpINormalStream create instance for YUVEffectHal failed!");
        cleanUp();
        return MFALSE;
    }
    else if(!mpINormalStream->init(LOG_TAG))
    {
        MY_LOGE("mpINormalStream init for YUVEffectHal failed!");
        cleanUp();
        return MFALSE;
    }
    CAM_TRACE_END();

    // 3A: create instance
    // UT does not test 3A
    CAM_TRACE_BEGIN("YUVEffectHal::create_3A_instance");
    #ifndef GTEST
    mp3AHal_Main1 = MAKE_Hal3A(miSensorIdx_Main1, "YUV_EffectHal_3A_MAIN1");
    mp3AHal_Main2 = MAKE_Hal3A(miSensorIdx_Main2, "YUV_EffectHal_3A_MAIN2");
    MY_LOGD("3A create instance, Main1: %x Main2: %x", mp3AHal_Main1, mp3AHal_Main2);
    #endif
    CAM_TRACE_END();

    FUNCTION_LOG_END;
    return OK;
}
//************************************************************************
//
//************************************************************************
status_t
YUVEffectHal::
uninitImpl()
{
    FUNCTION_LOG_START;
    CAM_TRACE_NAME("YUVEffectHal::uninitImpl");
    cleanUp();
    FUNCTION_LOG_END;
    return OK;
}
//************************************************************************
//
//************************************************************************
status_t
YUVEffectHal::
prepareImpl()
{
    FUNCTION_LOG_START;
    FUNCTION_LOG_END;
    return OK;
}
//************************************************************************
//
//************************************************************************
status_t
YUVEffectHal::
releaseImpl()
{
    FUNCTION_LOG_START;

    FUNCTION_LOG_END;
    return OK;
}
//************************************************************************
//
//************************************************************************
status_t
YUVEffectHal::
getNameVersionImpl(
    EffectHalVersion &nameVersion) const
{
    FUNCTION_LOG_START;

    nameVersion.effectName = EFFECT_NAME;
    nameVersion.major = MAJOR_VERSION;
    nameVersion.minor = MINOR_VERSION;

    FUNCTION_LOG_END;
    return OK;
}
//************************************************************************
//
//************************************************************************
status_t
YUVEffectHal::
getCaptureRequirementImpl(
    EffectParameter *inputParam,
    Vector<EffectCaptureRequirement> &requirements) const
{
    FUNCTION_LOG_START;
    FUNCTION_LOG_END;
    return OK;
}
//************************************************************************
//
//************************************************************************
status_t
YUVEffectHal::
setParameterImpl(
    String8 &key,
    String8 &object)
{
    FUNCTION_LOG_START;
    UNUSED(key);
    UNUSED(object);
    FUNCTION_LOG_END;
    return OK;
}
//************************************************************************
//
//************************************************************************
status_t
YUVEffectHal::
setParametersImpl(sp<EffectParameter> parameter)
{
    FUNCTION_LOG_START;

    FUNCTION_LOG_END;
    return OK;
}
//************************************************************************
//
//************************************************************************
status_t
YUVEffectHal::
startImpl(
    uint64_t *uid)
{
    FUNCTION_LOG_START;
    UNUSED(uid);
    FUNCTION_LOG_END;
    return OK;
}
//************************************************************************
//
//************************************************************************
status_t
YUVEffectHal::
abortImpl(
    EffectResult &result,
    EffectParameter const *parameter)
{
    FUNCTION_LOG_START;
    UNUSED(result);
    UNUSED(parameter);
    //
    onFlush();
    //
    FUNCTION_LOG_END;
    return OK;
}
//************************************************************************
//
//************************************************************************
status_t
YUVEffectHal::
updateEffectRequestImpl(const EffectRequestPtr request)
{
    FUNCTION_LOG_START;
    Mutex::Autolock _l(mLock);
    MBOOL ret = MTRUE;
    //
    if (request == NULL)
    {
        MY_LOGE("Effect Request is NULL");
        return MFALSE;
    }

    EffectRequestPtr req = (EffectRequestPtr)request;

    //prepare enque parameter
    QParams enqueParams, dequeParams;
    //EnquedData *enquedData = new EnquedData(request, this);

    // get current state
    sp<EffectParameter> pReqParam = request->getRequestParameter();
    DualYUVNodeOpState eState = (DualYUVNodeOpState) pReqParam->getInt(DUAL_YUV_REQUEST_STATE_KEY);
    MY_LOGD_IF(mLogLevel >= 1,"+ begin reqID=%d eState=%d", request->getRequestNo(), eState);

    MBOOL bRet;
    if(eState == STATE_CAPTURE)
        bRet = buildQParams_CAP(req, enqueParams);
    else if (eState == STATE_THUMBNAIL)
        bRet = buildQParams_THUMBNAIL(req, enqueParams);
    else if (eState == STATE_NORMAL)
        bRet = buildQParams_NORMAL(req, enqueParams);
    else
    {
        MY_LOGE("Wrong Request op state.");
        goto lbExit;
    }

    debugQParams(enqueParams);
    if(!bRet)
    {
        MY_LOGE("Failed to build P2 enque parametes.");
        goto lbExit;
    }
    // callback
    //enqueParams.mpfnCallback = onP2Callback;
    //enqueParams.mpfnEnQFailCallback = onP2FailedCallback;
    //enqueParams.mpCookie = (MVOID*) enquedData;
    enqueParams.mpfnCallback = NULL;
    enqueParams.mpfnEnQFailCallback = NULL;
    enqueParams.mpCookie = NULL;

    MY_LOGD_IF(mLogLevel >= 1,"mpINormalStream enque start! reqID=%d", request->getRequestNo());
    CAM_TRACE_BEGIN("YUVEffectHal::NormalStream::enque");
    bRet = mpINormalStream->enque(enqueParams);

    if(!bRet)
    {
        MY_LOGE("mpINormalStream enque failed! reqID=%d", request->getRequestNo());
        goto lbExit;
    }

    bRet = mpINormalStream->deque(dequeParams);
    CAM_TRACE_END();
    MY_LOGD_IF(mLogLevel >= 1,"mpINormalStream deque end! reqID=%d", request->getRequestNo());

    if(bRet)
    {
        //handleP2Done(enqueParams, enquedData);
        if(miDump>0)
        {
            FrameInfoPtr pFramePtr;
            if (eState == STATE_NORMAL)
            {
                sp<IImageBuffer> frameBuf_MV_F, frameBuf_FD;
                RETRIEVE_OFRAMEINFO_IMGBUF_ERROR_RETURN(req, pFramePtr, BID_OUT_FD, frameBuf_FD);
                RETRIEVE_OFRAMEINFO_IMGBUF_ERROR_RETURN(req, pFramePtr, BID_OUT_MV_F, frameBuf_MV_F);

                std::string saveFileName = msFilename + std::string("/FD_") + std::string(".yuv");
                frameBuf_FD->saveToFile(saveFileName.c_str());
                saveFileName = msFilename + std::string("/MV_F_") + std::string(".yuv");
                frameBuf_MV_F->saveToFile(saveFileName.c_str());
            }
            else if (eState == STATE_CAPTURE)
            {
                sp<IImageBuffer> frameBuf_MV_F_Cap_main1, frameBuf_MV_F_Cap_main2;
                RETRIEVE_OFRAMEINFO_IMGBUF_ERROR_RETURN(req, pFramePtr, BID_OUT_MV_F_CAP_MAIN1, frameBuf_MV_F_Cap_main1);
                RETRIEVE_OFRAMEINFO_IMGBUF_ERROR_RETURN(req, pFramePtr, BID_OUT_MV_F_CAP_MAIN2, frameBuf_MV_F_Cap_main2);

                std::string saveFileName = msFilename + std::string("/MV_F_Cap1") + std::string(".yuv");
                frameBuf_MV_F_Cap_main1->saveToFile(saveFileName.c_str());
                saveFileName = msFilename + std::string("/MV_F_Cap2") + std::string(".yuv");
                frameBuf_MV_F_Cap_main2->saveToFile(saveFileName.c_str());
            }
            else if (eState == STATE_THUMBNAIL)
            {
                sp<IImageBuffer> frameBuf_Thumbnail_Cap;
                RETRIEVE_OFRAMEINFO_IMGBUF_ERROR_RETURN(req, pFramePtr, BID_OUT_THUMBNAIL_CAP, frameBuf_Thumbnail_Cap);

                std::string saveFileName = msFilename + std::string("/Thumbnail_Cap") + std::string(".yuv");
                frameBuf_Thumbnail_Cap->saveToFile(saveFileName.c_str());
            }

        }

    }
    else
    {
        MY_LOGE("mpINormalStream deque failed! reqID=%d", request->getRequestNo());
        goto lbExit;
    }


    MY_LOGD_IF(mLogLevel >= 1,"- end reqID=%d", request->getRequestNo());
    //delete enquedData;
    ReleaseQParam(enqueParams);
    ReleaseMain2HalMeta(request);

    return MTRUE;
lbExit:
    //handleP2Failed(enqueParams, enquedData);
    //delete enquedData;
    ReleaseQParam(enqueParams);
    ReleaseMain2HalMeta(request);
    return MFALSE;
}
//************************************************************************
//
//************************************************************************
MVOID
YUVEffectHal::
onP2Callback(QParams& rParams)
{
    EnquedData* pEnqueData = (EnquedData*) (rParams.mpCookie);
    YUVEffectHal* pYUVEffectHal = (YUVEffectHal*) (pEnqueData->mpYUVEffectHal);
    pYUVEffectHal->handleP2Done(rParams, pEnqueData);
}
//************************************************************************
//
//************************************************************************
MVOID
YUVEffectHal::
onP2FailedCallback(QParams& rParams)
{
    MY_LOGE("YUVEffectHal operations failed!!Check the following log:");
    EnquedData* pEnqueData = (EnquedData*) (rParams.mpCookie);
    YUVEffectHal* pYUVEffectHal = (YUVEffectHal*) (pEnqueData->mpYUVEffectHal);
    pYUVEffectHal->handleP2Failed(rParams, pEnqueData);
}
//************************************************************************
//
//************************************************************************
MVOID
YUVEffectHal::
handleP2Done(QParams& rParams, EnquedData* pEnqueData)
{
    CAM_TRACE_NAME("YUVEffectHal::handleP2Done");
    EffectRequestPtr request = pEnqueData->mRequest;
    MY_LOGD_IF(mLogLevel >= 1,"+ :reqID=%d", request->getRequestNo());

    if(request->mpOnRequestProcessed!=nullptr)
    {
        request->mpOnRequestProcessed(request->mpTag, String8("Done"), request);
    }
}
//************************************************************************
//
//************************************************************************
MVOID
YUVEffectHal::
ReleaseQParam(QParams& rParams)
{
    CAM_TRACE_NAME("YUVEffectHal::ReleaseQParam");
    //Free rParams
    for (int i=0; i < rParams.mvFrameParams.size(); i++)
    {
        rParams.mvFrameParams.editItemAt(i).mvIn.clear();
        rParams.mvFrameParams.editItemAt(i).mvOut.clear();
        rParams.mvFrameParams.editItemAt(i).mvCropRsInfo.clear();
        rParams.mvFrameParams.editItemAt(i).mvModuleData.clear();
        MVOID* pTuningBuffer = rParams.mvFrameParams[i].mTuningData;
        free(pTuningBuffer);
    }
    rParams.mvFrameParams.clear();
}
//************************************************************************
//
//************************************************************************
MVOID
YUVEffectHal::
ReleaseMain2HalMeta(const EffectRequestPtr request)
{
    sp<EffectFrameInfo> pFrame_HalMetaMain2 = request->vInputFrameInfo.valueFor(BID_META_IN_HAL_MAIN2);
    sp<EffectParameter> pEffParam = pFrame_HalMetaMain2->getFrameParameter();
    IMetadata* pMeta = reinterpret_cast<IMetadata*>(pEffParam->getPtr("Metadata"));
    delete pMeta;
}
//************************************************************************
//
//************************************************************************
MVOID
YUVEffectHal::
handleP2Failed(QParams& rParams, EnquedData* pEnqueData)
{
    CAM_TRACE_NAME("YUVEffectHal::handleP2Failed");
    EffectRequestPtr request = pEnqueData->mRequest;
    MY_LOGD_IF(mLogLevel >= 1,"+ :reqID=%d", request->getRequestNo());
    debugQParams(rParams);
    //
    if(request->mpOnRequestProcessed!=nullptr)
    {
        request->mpOnRequestProcessed(request->mpTag, String8("Failed"), request);
    }
}
//************************************************************************
//
//************************************************************************
MBOOL
YUVEffectHal::
buildQParams_CAP(EffectRequestPtr& rEffReqPtr, QParams& rEnqueParam)
{
    FUNCTION_LOG_START;

    MY_LOGD_IF(mLogLevel >= 1,"+, reqID=%d", rEffReqPtr->getRequestNo());
    // Get the input/output buffer inside the request
    FrameInfoPtr framePtr_inMain1FSRAW = rEffReqPtr->vInputFrameInfo.valueFor(BID_DualYUV_IN_FSRAW1);
    FrameInfoPtr framePtr_inMain2FSRAW = rEffReqPtr->vInputFrameInfo.valueFor(BID_DualYUV_IN_FSRAW2);
    FrameInfoPtr pFramePtr;
    MVOID* pVATuningBuffer_Main1 = NULL;
    MVOID* pVATuningBuffer_Main2 = NULL;

    NSCam::NSIoPipe::FrameParams frameParams;
    NSCam::NSIoPipe::FrameParams frameParams2;
    MUINT iFrameNum = 0;
    FrameInfoPtr framePtr_inAppMeta = rEffReqPtr->vInputFrameInfo.valueFor(BID_META_IN_APP);
    //--> frame 0
    {
        // input StreamTag
        frameParams.mStreamTag = ENormalStreamTag_Normal;

        iFrameNum = 0;
        FrameInfoPtr framePtr_inHalMeta_Main2 = rEffReqPtr->vInputFrameInfo.valueFor(BID_META_IN_HAL_MAIN2);
        // Apply tuning data
        pVATuningBuffer_Main2 = (MVOID*) malloc(mpINormalStream->getRegTableSize());
        memset(pVATuningBuffer_Main2, 0, mpINormalStream->getRegTableSize());

        sp<IImageBuffer> frameBuf_MV_F_Main2;
        // Apply ISP tuning
        ISPTuningConfig ispConfig = {framePtr_inAppMeta, framePtr_inHalMeta_Main2, mp3AHal_Main2, MFALSE};
        TuningParam rTuningParam = applyISPTuning(pVATuningBuffer_Main2, ispConfig);
        // insert tuning data
        frameParams.mTuningData = pVATuningBuffer_Main2;

        // UT does not test 3A
        #ifndef GTEST
        if(rTuningParam.pLsc2Buf != NULL)
        {
            // input: LSC2 buffer (for tuning)
            IImageBuffer* pLSC2Src = static_cast<IImageBuffer*>(rTuningParam.pLsc2Buf);

            Input input_DEPI;
            input_DEPI.mPortID = PORT_DEPI;
            input_DEPI.mPortID.group = iFrameNum;
            input_DEPI.mBuffer = pLSC2Src;
            frameParams.mvIn.push_back(input_DEPI);
        }
        else
        {
            MY_LOGE("LSC2 buffer from 3A is NULL!!");
            return MFALSE;
        }
        #endif

        // make sure the output is 16:9, get crop size& point
        MSize cropSize;
        MPoint startPoint;
        calCropForScreen(framePtr_inMain2FSRAW, startPoint, cropSize);

        // input: Main1 Fullsize RAW
        {
            sp<IImageBuffer> pImgBuf;
            framePtr_inMain2FSRAW->getFrameBuffer(pImgBuf);

            Input input_IMGI;
            input_IMGI.mPortID = PORT_IMGI;
            input_IMGI.mPortID.group = iFrameNum;
            input_IMGI.mBuffer = pImgBuf.get();
            frameParams.mvIn.push_back(input_IMGI);
        }

        // output : MV_F_CAP
        {
            RETRIEVE_OFRAMEINFO_IMGBUF_ERROR_RETURN(rEffReqPtr, pFramePtr, BID_OUT_MV_F_CAP_MAIN2, frameBuf_MV_F_Main2);

            // insert output
            Output output_WDMA;
            output_WDMA.mPortID = PORT_WDMAO;
            output_WDMA.mPortID.group = iFrameNum;
            output_WDMA.mTransform = 0;
            output_WDMA.mBuffer = frameBuf_MV_F_Main2.get();
            frameParams.mvOut.push_back(output_WDMA);

            // setCrop
            MCrpRsInfo cropInfo;
            cropInfo.mGroupID = (MUINT32) eCROP_WDMA;
            cropInfo.mCropRect.p_fractional.x=0;
            cropInfo.mCropRect.p_fractional.y=0;
            cropInfo.mCropRect.p_integral.x=startPoint.x;
            cropInfo.mCropRect.p_integral.y=startPoint.y;
            cropInfo.mCropRect.s=cropSize;
            cropInfo.mResizeDst=frameBuf_MV_F_Main2->getImgSize();
            cropInfo.mFrameGroup = iFrameNum;
            frameParams.mvCropRsInfo.push_back(cropInfo);
        }
        rEnqueParam.mvFrameParams.push_back(frameParams);
    }

    //--> frame 1
    {
        // input StreamTag
        frameParams2.mStreamTag = ENormalStreamTag_Normal;

        iFrameNum = 1;
        FrameInfoPtr framePtr_inHalMeta_Main1 = rEffReqPtr->vInputFrameInfo.valueFor(BID_META_IN_HAL);
        // Apply tuning data
        pVATuningBuffer_Main1 = (MVOID*) malloc(mpINormalStream->getRegTableSize());
        memset(pVATuningBuffer_Main1, 0, mpINormalStream->getRegTableSize());

        sp<IImageBuffer> frameBuf_MV_F_Main1;
        // Apply ISP tuning
        ISPTuningConfig ispConfig = {framePtr_inAppMeta, framePtr_inHalMeta_Main1, mp3AHal_Main1, MFALSE};
        TuningParam rTuningParam = applyISPTuning(pVATuningBuffer_Main1, ispConfig);
        // insert tuning data
        frameParams2.mTuningData = pVATuningBuffer_Main1;

        // UT does not test 3A
        #ifndef GTEST
        if(rTuningParam.pLsc2Buf != NULL)
        {
            // input: LSC2 buffer (for tuning)
            IImageBuffer* pLSC2Src = static_cast<IImageBuffer*>(rTuningParam.pLsc2Buf);

            Input input_DEPI;
            input_DEPI.mPortID = PORT_DEPI;
            input_DEPI.mPortID.group = iFrameNum;
            input_DEPI.mBuffer = pLSC2Src;
            frameParams2.mvIn.push_back(input_DEPI);
        }
        else
        {
            MY_LOGE("LSC2 buffer from 3A is NULL!!");
            return MFALSE;
        }
        #endif

        // make sure the output is 16:9, get crop size& point
        MSize cropSize;
        MPoint startPoint;
        calCropForScreen(framePtr_inMain1FSRAW, startPoint, cropSize);

        // input: Main1 Fullsize RAW
        {
            sp<IImageBuffer> pImgBuf;
            framePtr_inMain1FSRAW->getFrameBuffer(pImgBuf);

            Input input_IMGI;
            input_IMGI.mPortID = PORT_IMGI;
            input_IMGI.mPortID.group = iFrameNum;
            input_IMGI.mBuffer = pImgBuf.get();
            frameParams2.mvIn.push_back(input_IMGI);
        }

        // output : MV_F_CAP
        {
            RETRIEVE_OFRAMEINFO_IMGBUF_ERROR_RETURN(rEffReqPtr, pFramePtr, BID_OUT_MV_F_CAP_MAIN1, frameBuf_MV_F_Main1);

            // insert output
            Output output_WDMA;
            output_WDMA.mPortID = PORT_WDMAO;
            output_WDMA.mPortID.group = iFrameNum;
            output_WDMA.mTransform = 0;
            output_WDMA.mBuffer = frameBuf_MV_F_Main1.get();
            frameParams2.mvOut.push_back(output_WDMA);

            // setCrop
            MCrpRsInfo cropInfo;
            cropInfo.mGroupID = (MUINT32) eCROP_WDMA;
            cropInfo.mCropRect.p_fractional.x=0;
            cropInfo.mCropRect.p_fractional.y=0;
            cropInfo.mCropRect.p_integral.x=startPoint.x;
            cropInfo.mCropRect.p_integral.y=startPoint.y;
            cropInfo.mCropRect.s=cropSize;
            cropInfo.mResizeDst=frameBuf_MV_F_Main1->getImgSize();
            cropInfo.mFrameGroup = iFrameNum;
            frameParams2.mvCropRsInfo.push_back(cropInfo);
        }
        rEnqueParam.mvFrameParams.push_back(frameParams2);
    }


    MY_LOGD_IF(mLogLevel >= 1,"-, reqID=%d", rEffReqPtr->getRequestNo());
    FUNCTION_LOG_END;
    return MTRUE;
}
//************************************************************************
//
//************************************************************************
MBOOL
YUVEffectHal::
buildQParams_THUMBNAIL(EffectRequestPtr& rEffReqPtr, QParams& rEnqueParam)
{
    FUNCTION_LOG_START;

    MY_LOGD_IF(mLogLevel >= 1,"+, reqID=%d", rEffReqPtr->getRequestNo());
    // Get the input/output buffer inside the request
    FrameInfoPtr framePtr_inMain1FSRAW = rEffReqPtr->vInputFrameInfo.valueFor(BID_DualYUV_IN_FSRAW1);
    FrameInfoPtr pFramePtr;
    MVOID* pVATuningBuffer_Main1 = NULL;

    MUINT iFrameNum = 0;
    NSCam::NSIoPipe::FrameParams frameParams;
    FrameInfoPtr framePtr_inAppMeta = rEffReqPtr->vInputFrameInfo.valueFor(BID_META_IN_APP);
    IMetadata* pMeta_InApp  = getMetadataFromFrameInfoPtr(framePtr_inAppMeta);
    //--> frame 0
    {
        // input StreamTag
        frameParams.mStreamTag = ENormalStreamTag_Normal;

        FrameInfoPtr framePtr_inHalMeta_Main1 = rEffReqPtr->vInputFrameInfo.valueFor(BID_META_IN_HAL);
        // Apply tuning data
        pVATuningBuffer_Main1 = (MVOID*) malloc(mpINormalStream->getRegTableSize());
        memset(pVATuningBuffer_Main1, 0, mpINormalStream->getRegTableSize());

        sp<IImageBuffer> frameBuf_Thumbnail_Cap;
        // Apply ISP tuning
        ISPTuningConfig ispConfig = {framePtr_inAppMeta, framePtr_inHalMeta_Main1, mp3AHal_Main1, MFALSE};
        TuningParam rTuningParam = applyISPTuning(pVATuningBuffer_Main1, ispConfig);
        // insert tuning data
        frameParams.mTuningData = pVATuningBuffer_Main1;

        // UT does not test 3A
        #ifndef GTEST
        if(rTuningParam.pLsc2Buf != NULL)
        {
            // input: LSC2 buffer (for tuning)
            IImageBuffer* pLSC2Src = static_cast<IImageBuffer*>(rTuningParam.pLsc2Buf);

            Input input_DEPI;
            input_DEPI.mPortID = PORT_DEPI;
            input_DEPI.mPortID.group = iFrameNum;
            input_DEPI.mBuffer = pLSC2Src;
            frameParams.mvIn.push_back(input_DEPI);
        }
        else
        {
            MY_LOGE("LSC2 buffer from 3A is NULL!!");
            return MFALSE;
        }
        #endif

        // make sure the output is 16:9, get crop size& point
        MSize cropSize;
        MPoint startPoint;
        calCropForScreen(framePtr_inMain1FSRAW, startPoint, cropSize);

        // input: Main1 Fullsize RAW
        {
            sp<IImageBuffer> pImgBuf;
            framePtr_inMain1FSRAW->getFrameBuffer(pImgBuf);

            Input input_IMGI;
            input_IMGI.mPortID = PORT_IMGI;
            input_IMGI.mPortID.group = iFrameNum;
            input_IMGI.mBuffer = pImgBuf.get();
            frameParams.mvIn.push_back(input_IMGI);
        }

        // output : MV_F_CAP
        {
            RETRIEVE_OFRAMEINFO_IMGBUF_ERROR_RETURN(rEffReqPtr, pFramePtr, BID_OUT_THUMBNAIL_CAP, frameBuf_Thumbnail_Cap);
            MINT32 eTransJpeg = getJpegRotation(pMeta_InApp);

            // insert output
            Output output_WROT;
            output_WROT.mPortID = PORT_WROTO;
            output_WROT.mPortID.group = iFrameNum;
            output_WROT.mTransform = eTransJpeg;
            output_WROT.mBuffer = frameBuf_Thumbnail_Cap.get();
            frameParams.mvOut.push_back(output_WROT);

            //StereoArea fullraw_crop;
            //fullraw_crop = sizePrvider->getBufferSize(E_BM_PREPROCESS_FULLRAW_CROP_1);

            // setCrop
            MCrpRsInfo cropInfo;
            cropInfo.mGroupID = (MUINT32) eCROP_WROT;
            cropInfo.mCropRect.p_fractional.x=0;
            cropInfo.mCropRect.p_fractional.y=0;
            cropInfo.mCropRect.p_integral.x=startPoint.x;
            cropInfo.mCropRect.p_integral.y=startPoint.y;
            cropInfo.mCropRect.s=cropSize;
            cropInfo.mResizeDst=frameBuf_Thumbnail_Cap->getImgSize();
            cropInfo.mFrameGroup = iFrameNum;
            frameParams.mvCropRsInfo.push_back(cropInfo);
        }
        rEnqueParam.mvFrameParams.push_back(frameParams);
    }


    MY_LOGD_IF(mLogLevel >= 1,"-, reqID=%d", rEffReqPtr->getRequestNo());
    FUNCTION_LOG_END;
    return MTRUE;
}
/*******************************************************************************
 *
 ********************************************************************************/
MINT32
YUVEffectHal::
DegreeToeTransform(MINT32 degree)
{
    switch(degree)
    {
        case 0:
            return 0;
        case 90:
            return eTransform_ROT_90;
        case 180:
            return eTransform_ROT_180;
        case 270:
            return eTransform_ROT_270;
        default:
            MY_LOGE("Not support degree =%d", degree);
            return -1;
    }
}

/*******************************************************************************
 *
 ********************************************************************************/
MINT32
YUVEffectHal::
getJpegRotation(IMetadata* pMeta)
{
    MINT32 jpegRotationApp = 0;
    if(!tryGetMetadata<MINT32>(pMeta, MTK_JPEG_ORIENTATION, jpegRotationApp))
    {
        MY_LOGE("Get jpegRotationApp failed!");
    }

    MINT32 rotDegreeJpeg = jpegRotationApp;
    if(rotDegreeJpeg < 0){
        rotDegreeJpeg = rotDegreeJpeg + 360;
    }

    MY_LOGD("jpegRotationApp:%d, rotDegreeJpeg:%d", jpegRotationApp, rotDegreeJpeg);

    return DegreeToeTransform(rotDegreeJpeg);;
}
//************************************************************************
//
//************************************************************************
MBOOL
YUVEffectHal::
calCropForScreen(FrameInfoPtr& pFrameInfo, MPoint &rCropStartPt, MSize& rCropSize )
{
    sp<IImageBuffer> pImgBuf;
    pFrameInfo->getFrameBuffer(pImgBuf);
    MSize srcSize = pImgBuf->getImgSize();

    // calculate the required image hight according to image ratio
    int iHeight;
    switch(StereoSettingProvider::imageRatio())
    {
        case eRatio_4_3:
            iHeight = ((srcSize.w * 3 / 4) >> 1 ) <<1;
            break;
        case eRatio_16_9:
        default:
            iHeight = ((srcSize.w * 9 / 16) >> 1 ) <<1;
            break;
    }

    if(abs(iHeight-srcSize.h) == 0)
    {
        rCropStartPt = MPoint(0, 0);
        rCropSize = srcSize;
    }
    else
    {
        rCropStartPt.x = 0;
        rCropStartPt.y = (srcSize.h - iHeight)/2;
        rCropSize.w = srcSize.w;
        rCropSize.h = iHeight;
    }

    MY_LOGD_IF(mLogLevel >= 1,"calCropForScreen rCropStartPt: (%d, %d), \
                    rCropSize: %dx%d ", rCropStartPt.x, rCropStartPt.y, rCropSize.w, rCropSize.h);

    return MTRUE;

}
//************************************************************************
//
//************************************************************************
TuningParam
YUVEffectHal::
applyISPTuning(MVOID* pVATuningBuffer, const ISPTuningConfig& ispConfig)
{
    CAM_TRACE_NAME("YUVEffectHal::applyISPTuning");
    MY_LOGD_IF(mLogLevel >= 1,"+, reqID=%d bIsResized=%d", ispConfig.pInAppMetaFrame->getRequestNo(), ispConfig.bInputResizeRaw);

    TuningParam tuningParam;
    tuningParam.pRegBuf = reinterpret_cast<void*>(pVATuningBuffer);

    MetaSet_T inMetaSet;
    IMetadata* pMeta_InApp  = getMetadataFromFrameInfoPtr(ispConfig.pInAppMetaFrame);
    IMetadata* pMeta_InHal  = getMetadataFromFrameInfoPtr(ispConfig.pInHalMetaFrame);

    inMetaSet.appMeta = *pMeta_InApp;
    inMetaSet.halMeta = *pMeta_InHal;

    // USE resize raw-->set PGN 0
    if(ispConfig.bInputResizeRaw)
        updateEntry<MUINT8>(&(inMetaSet.halMeta), MTK_3A_PGN_ENABLE, 0);
    else
        updateEntry<MUINT8>(&(inMetaSet.halMeta), MTK_3A_PGN_ENABLE, 1);

    // UT do not test setIsp
    #ifndef GTEST
    {
        MetaSet_T resultMeta;
        ispConfig.p3AHAL->setIsp(0, inMetaSet, &tuningParam, miOff3AInfo ? NULL : &resultMeta);

        // DO NOT write ISP resultMeta back into input hal Meta since there are other node doing this concurrently
        // (*pMeta_InHal) += resultMeta.halMeta;
    }
    #endif

    MY_LOGD_IF(mLogLevel >= 1,"-, reqID=%d", ispConfig.pInAppMetaFrame->getRequestNo());
    return tuningParam;
}
//************************************************************************
//
//************************************************************************
IMetadata*
YUVEffectHal::
getMetadataFromFrameInfoPtr(sp<EffectFrameInfo> pFrameInfo)
{
    IMetadata* result;
    sp<EffectParameter> effParam = pFrameInfo->getFrameParameter();
    result = reinterpret_cast<IMetadata*>(effParam->getPtr(EFFECT_PARAMS_KEY));
    return result;
}
//************************************************************************
//
//************************************************************************
MBOOL
YUVEffectHal::
buildQParams_NORMAL(EffectRequestPtr& rEffReqPtr, QParams& rEnqueParam)
{
    FUNCTION_LOG_START;
    CAM_TRACE_NAME("YUVEffectHal::buildQParams_NORMAL");
    MY_LOGD_IF(mLogLevel >= 1,"+, reqID=%d", rEffReqPtr->getRequestNo());

    // Get the input/output buffer inside the request
    NSCam::NSIoPipe::FrameParams frameParams;
    FrameInfoPtr framePtr_inMain1RSRAW = rEffReqPtr->vInputFrameInfo.valueFor(BID_DualYUV_IN_RSRAW1);
    FrameInfoPtr pFramePtr;
    sp<IImageBuffer> frameBuf_RSRAW1, frameBuf_FD;

    framePtr_inMain1RSRAW->getFrameBuffer(frameBuf_RSRAW1);
    MY_LOGD_IF(mLogLevel >= 1,"RSRAW1=%d", frameBuf_RSRAW1->getImgSize());

    // Make sure the ordering inside the mvIn mvOut
    int mvInIndex = 0, mvOutIndex = 0;
    MUINT iFrameNum = 0;
    MPoint zeroPos(0,0);
    MVOID* pVATuningBuffer = NULL;

    FrameInfoPtr framePtr_inAppMeta = rEffReqPtr->vInputFrameInfo.valueFor(BID_META_IN_APP);
    IMetadata* pMeta_InApp = getMetadataFromFrameInfoPtr(framePtr_inAppMeta);
    FrameInfoPtr framePtr_inHalMeta;
    //--> frame 0
    {
        // input StreamTag
        frameParams.mStreamTag = ENormalStreamTag_Normal;
        frameParams.mSensorIdx = miSensorIdx_Main1;

        iFrameNum = 0;
        FrameInfoPtr framePtr_inHalMeta_Main1 = rEffReqPtr->vInputFrameInfo.valueFor(BID_META_IN_HAL);
        // Apply tuning data
        pVATuningBuffer = (MVOID*) malloc(mpINormalStream->getRegTableSize());
        memset(pVATuningBuffer, 0, mpINormalStream->getRegTableSize());

        sp<IImageBuffer> frameBuf_MV_F;
        // Apply ISP tuning
        ISPTuningConfig ispConfig = {framePtr_inAppMeta, framePtr_inHalMeta_Main1, mp3AHal_Main1, MTRUE};
        TuningParam rTuningParam = applyISPTuning(pVATuningBuffer, ispConfig);
        frameParams.mTuningData = pVATuningBuffer;

        // input: Main1 RSRAW
        {
            sp<IImageBuffer> pImgBuf;
            framePtr_inMain1RSRAW->getFrameBuffer(pImgBuf);

            Input input_IMGI;
            input_IMGI.mPortID = PORT_IMGI;
            input_IMGI.mPortID.group = iFrameNum;
            input_IMGI.mBuffer = pImgBuf.get();
            frameParams.mvIn.push_back(input_IMGI);
        }

        // output FD
        if(rEffReqPtr->vOutputFrameInfo.indexOfKey(BID_OUT_FD) >= 0)
        {
             // output: FD
            RETRIEVE_OFRAMEINFO_IMGBUF_ERROR_RETURN(rEffReqPtr, pFramePtr, BID_OUT_FD, frameBuf_FD);
            // insert FD output
            Output output_IMG2O;
            output_IMG2O.mPortID = PORT_IMG2O;
            output_IMG2O.mPortID.group = iFrameNum;
            output_IMG2O.mTransform = 0;
            output_IMG2O.mBuffer = frameBuf_FD.get();
            frameParams.mvOut.push_back(output_IMG2O);

            // setCrop
            MCrpRsInfo cropInfo;
            cropInfo.mGroupID = (MUINT32) eCROP_CRZ;
            cropInfo.mCropRect.p_fractional.x=0;
            cropInfo.mCropRect.p_fractional.y=0;
            cropInfo.mCropRect.p_integral.x=zeroPos.x;
            cropInfo.mCropRect.p_integral.y=zeroPos.y;
            cropInfo.mCropRect.s=frameBuf_RSRAW1->getImgSize();
            cropInfo.mResizeDst=frameBuf_FD->getImgSize();
            cropInfo.mFrameGroup = iFrameNum;
            frameParams.mvCropRsInfo.push_back(cropInfo);
        }

        // output: MV_F
        RETRIEVE_OFRAMEINFO_IMGBUF_ERROR_RETURN(rEffReqPtr, pFramePtr, BID_OUT_MV_F, frameBuf_MV_F);
        if(frameBuf_MV_F == nullptr)
        {
            MY_LOGE("frameBuf_MV_F is null request(0x%x)", rEffReqPtr.get());
        }
        // check EIS on/off
        if (isEISOn(pMeta_InApp))
        {
            IMetadata* pMeta_InHal = getMetadataFromFrameInfoPtr(framePtr_inHalMeta_Main1);
            eis_region region;
            // set MV_F crop for EIS
            if(queryEisRegion(pMeta_InHal, region, rEffReqPtr))
            {
                MY_LOGD_IF(mLogLevel >= 1,"queryEisRegion IN");
                // setCrop
                MCrpRsInfo cropInfo;
                cropInfo.mGroupID = (MUINT32) eCROP_WDMA;
                cropInfo.mCropRect.p_fractional.x=0;
                cropInfo.mCropRect.p_fractional.y=0;
                cropInfo.mCropRect.p_integral.x=region.x_int;
                cropInfo.mCropRect.p_integral.y=region.y_int;
                cropInfo.mCropRect.s=region.s;
                cropInfo.mResizeDst=frameBuf_MV_F->getImgSize();
                cropInfo.mFrameGroup = iFrameNum;
                frameParams.mvCropRsInfo.push_back(cropInfo);
            }
            else
            {
                MY_LOGE("Query EIS Region Failed! reqID=%d.", rEffReqPtr->getRequestNo());
                return MFALSE;
            }
        }
        else
        {
            // MV_F crop
            MY_LOGD_IF(mLogLevel >= 1,"No EIS Crop IN");

            // setCrop
            MCrpRsInfo cropInfo;
            cropInfo.mGroupID = (MUINT32) eCROP_WDMA;
            cropInfo.mCropRect.p_fractional.x=0;
            cropInfo.mCropRect.p_fractional.y=0;
            cropInfo.mCropRect.p_integral.x=zeroPos.x;
            cropInfo.mCropRect.p_integral.y=zeroPos.y;
            cropInfo.mCropRect.s=frameBuf_RSRAW1->getImgSize();
            cropInfo.mResizeDst=frameBuf_MV_F->getImgSize();
            cropInfo.mFrameGroup = iFrameNum;
            frameParams.mvCropRsInfo.push_back(cropInfo);
        }

        // insert output
        Output output_WDMA;
        output_WDMA.mPortID = PORT_WDMAO;
        output_WDMA.mPortID.group = iFrameNum;
        output_WDMA.mTransform = 0;
        output_WDMA.mBuffer = frameBuf_MV_F.get();
        frameParams.mvOut.push_back(output_WDMA);

        rEnqueParam.mvFrameParams.push_back(frameParams);
    }


    FUNCTION_LOG_END;
    return MTRUE;
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
YUVEffectHal::
isEISOn(IMetadata* const inApp)
{
    MUINT8 eisMode = MTK_CONTROL_VIDEO_STABILIZATION_MODE_OFF;
    if( !tryGetMetadata<MUINT8>(inApp, MTK_CONTROL_VIDEO_STABILIZATION_MODE, eisMode) ) {
        MY_LOGD_IF(mLogLevel >= 1,"no MTK_CONTROL_VIDEO_STABILIZATION_MODE");
    }

    return eisMode == MTK_CONTROL_VIDEO_STABILIZATION_MODE_ON;
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
YUVEffectHal::
queryEisRegion(
    IMetadata* const inHal,
    eis_region& region,
    EffectRequestPtr request)
{
    IMetadata::IEntry entry = inHal->entryFor(MTK_EIS_REGION);

#if SUPPORT_EIS_MV
    // get EIS's motion vector
    if (entry.count() > 8)
    {
        MINT32 x_mv         = entry.itemAt(EIS_REGION_META_MV_X_LOC, Type2Type<MINT32>());
        MINT32 y_mv         = entry.itemAt(EIS_REGION_META_MV_Y_LOC, Type2Type<MINT32>());
        region.is_from_zzr  = entry.itemAt(EIS_REGION_META_FROM_RRZ_LOC, Type2Type<MINT32>());
        MBOOL x_mv_negative = x_mv >> 31;
        MBOOL y_mv_negative = y_mv >> 31;
        // convert to positive for getting parts of int and float if negative
        if (x_mv_negative) x_mv = ~x_mv + 1;
        if (y_mv_negative) y_mv = ~y_mv + 1;
        //
        region.x_mv_int   = (x_mv & (~0xFF)) >> 8;
        region.x_mv_float = (x_mv & (0xFF)) << 31;
        if(x_mv_negative){
            region.x_mv_int   = ~region.x_mv_int + 1;
            region.x_mv_float = ~region.x_mv_float + 1;
        }
        region.y_mv_int   = (y_mv& (~0xFF)) >> 8;
        region.y_mv_float = (y_mv& (0xFF)) << 31;
        if(y_mv_negative){
            region.y_mv_int   = ~region.y_mv_int + 1;
            region.y_mv_float = ~region.x_mv_float + 1;
        }
        // calculate x_int/y_int from mv
        FrameInfoPtr sourceFrameInfo;
        sp<IImageBuffer> pSrcImgBuf;
        DepthMapBufferID srcBID = BID_DualYUV_IN_RSRAW1; // main1 resize raw for preview
        // get frame buffer
        if(request->vInputFrameInfo.indexOfKey(srcBID) >= 0)
        {
            FrameInfoPtr frameInfo = request->vInputFrameInfo.valueFor(BID_DualYUV_IN_RSRAW1);
            frameInfo->getFrameBuffer(pSrcImgBuf);
        }
        else
        {
            MY_LOGE("Cannot find input frame info, BID=%d", srcBID);
            return MFALSE;
        }

        MSize sourceSize = pSrcImgBuf->getImgSize();
         // eisCenterStart is the left-up position of eis crop region when no eis offset, that is the eis crop region is located in the center.
        MPoint eisCenterStart;
        eisCenterStart.x = sourceSize.w / (EIS_FACTOR/100.0) * (EIS_FACTOR-100)/100 * 0.5;
        eisCenterStart.y = sourceSize.h / (EIS_FACTOR/100.0) * (EIS_FACTOR-100)/100 * 0.5;

        region.x_int = eisCenterStart.x + mv_x;
        region.y_int = eisCenterStart.y + mv_y;
        region.s = sourceSize / (EIS_FACTOR/100.0);

        return MTRUE;
    }
#else
    // get EIS's region
    if (entry.count() > 5)
    {
        region.x_int        = entry.itemAt(0, Type2Type<MINT32>());
        region.x_float      = entry.itemAt(1, Type2Type<MINT32>());
        region.y_int        = entry.itemAt(2, Type2Type<MINT32>());
        region.y_float      = entry.itemAt(3, Type2Type<MINT32>());
        region.s.w          = entry.itemAt(4, Type2Type<MINT32>());
        region.s.h          = entry.itemAt(5, Type2Type<MINT32>());

       return MTRUE;
    }
#endif
    MY_LOGE("wrong eis region count %zu", entry.count());
    return MFALSE;
}
/*******************************************************************************
 *
 ********************************************************************************/
MVOID
YUVEffectHal::
debugQParams(const QParams& rInputQParam)
{
    if(!mbDebugLog)
        return;

    MY_LOGD_IF(mLogLevel >= 2,"debugQParams start");
    MY_LOGD_IF(mLogLevel >= 2,"Size of mvFrameParams=%d\n",rInputQParam.mvFrameParams.size());


    MY_LOGD_IF(mLogLevel >= 2,"mvStreamTag section");
    for(size_t i=0;i<rInputQParam.mvFrameParams.size(); i++)
    {
        MY_LOGD_IF(mLogLevel >= 2,"Index = %d mvStreamTag = %d", i, rInputQParam.mvFrameParams[i].mStreamTag);

        MY_LOGD_IF(mLogLevel >= 2,"Size of mvIn=%d mvOut=%d, mvCropRsInfo=%d, mvModuleData=%d\n",
                    rInputQParam.mvFrameParams[i].mvIn.size(), rInputQParam.mvFrameParams[i].mvOut.size(),
                    rInputQParam.mvFrameParams[i].mvCropRsInfo.size(), rInputQParam.mvFrameParams[i].mvModuleData.size());

    }

    MY_LOGD_IF(mLogLevel >= 2,"mvTuningData section");

    for(size_t i=0;i<rInputQParam.mvFrameParams.size(); i++)
    {
#if 0
        dip_x_reg_t* data = (dip_x_reg_t*) rInputQParam.mvFrameParams[i].mTuningData;
        MY_LOGD_IF(mLogLevel >= 2,"========\nIndex = %d", i);

        MY_LOGD_IF(mLogLevel >= 2,"DIP_X_FE_CTRL1.Raw = %x", data->DIP_X_FE_CTRL1.Raw);
        MY_LOGD_IF(mLogLevel >= 2,"DIP_X_FE_IDX_CTRL.Raw = %x", data->DIP_X_FE_IDX_CTRL.Raw);
        MY_LOGD_IF(mLogLevel >= 2,"DIP_X_FE_CROP_CTRL1.Raw = %x", data->DIP_X_FE_CROP_CTRL1.Raw);
        MY_LOGD_IF(mLogLevel >= 2,"DIP_X_FE_CROP_CTRL2.Raw = %x", data->DIP_X_FE_CROP_CTRL2.Raw);
        MY_LOGD_IF(mLogLevel >= 2,"DIP_X_FE_CTRL2.Raw = %x", data->DIP_X_FE_CTRL2.Raw);
        MY_LOGD_IF(mLogLevel >= 2,"DIP_X_FM_SIZE.Raw = %x", data->DIP_X_FM_SIZE.Raw);
        MY_LOGD_IF(mLogLevel >= 2,"DIP_X_FM_TH_CON0.Raw = %x", data->DIP_X_FM_TH_CON0.Raw);
#endif
        MY_LOGD_IF(mLogLevel >= 2,"mvIn section");
        for(size_t j=0;j<rInputQParam.mvFrameParams[i].mvIn.size(); j++)
        {
            Input data = rInputQParam.mvFrameParams[i].mvIn[j];
            MY_LOGD_IF(mLogLevel >= 2,"========\nIndex = %d", j);

            MY_LOGD_IF(mLogLevel >= 2,"mvIn.PortID.index = %d", data.mPortID.index);
            MY_LOGD_IF(mLogLevel >= 2,"mvIn.PortID.type = %d", data.mPortID.type);
            MY_LOGD_IF(mLogLevel >= 2,"mvIn.PortID.inout = %d", data.mPortID.inout);
            MY_LOGD_IF(mLogLevel >= 2,"mvIn.PortID.group = %d", data.mPortID.group);

            MY_LOGD_IF(mLogLevel >= 2,"mvIn.mBuffer=%x", data.mBuffer);
            if(data.mBuffer !=NULL)
            {
                MY_LOGD_IF(mLogLevel >= 2,"mvIn.mBuffer->getImgSize = %dx%d", data.mBuffer->getImgSize().w,
                                                    data.mBuffer->getImgSize().h);

                MY_LOGD_IF(mLogLevel >= 2,"mvIn.mBuffer->getImgFormat = %x", data.mBuffer->getImgFormat());
                MY_LOGD_IF(mLogLevel >= 2,"mvIn.mBuffer->getPlaneCount = %d", data.mBuffer->getPlaneCount());
                for(int k=0;k<data.mBuffer->getPlaneCount();k++)
                {
                    MY_LOGD_IF(mLogLevel >= 2,"mvIn.mBuffer->getBufVA(%d) = %X", k, data.mBuffer->getBufVA(k));
                    MY_LOGD_IF(mLogLevel >= 2,"mvIn.mBuffer->getBufStridesInBytes(%d) = %d", k, data.mBuffer->getBufStridesInBytes(k));
                }
            }
            else
            {
                MY_LOGD_IF(mLogLevel >= 2,"mvIn.mBuffer is NULL!!");
            }


            MY_LOGD_IF(mLogLevel >= 2,"mvIn.mTransform = %d", data.mTransform);
        }

        MY_LOGD_IF(mLogLevel >= 2,"mvOut section");
        for(size_t j=0;j<rInputQParam.mvFrameParams[i].mvOut.size(); j++)
        {
            Output data = rInputQParam.mvFrameParams[i].mvOut[j];
            MY_LOGD_IF(mLogLevel >= 2,"========\nIndex = %d", j);

            MY_LOGD_IF(mLogLevel >= 2,"mvOut.PortID.index = %d", data.mPortID.index);
            MY_LOGD_IF(mLogLevel >= 2,"mvOut.PortID.type = %d", data.mPortID.type);
            MY_LOGD_IF(mLogLevel >= 2,"mvOut.PortID.inout = %d", data.mPortID.inout);
            MY_LOGD_IF(mLogLevel >= 2,"mvOut.PortID.group = %d", data.mPortID.group);

            MY_LOGD_IF(mLogLevel >= 2,"mvOut.mBuffer=%x", data.mBuffer);
            if(data.mBuffer != NULL)
            {
                MY_LOGD_IF(mLogLevel >= 2,"mvOut.mBuffer->getImgSize = %dx%d", data.mBuffer->getImgSize().w,
                                                    data.mBuffer->getImgSize().h);

                MY_LOGD_IF(mLogLevel >= 2,"mvOut.mBuffer->getImgFormat = %x", data.mBuffer->getImgFormat());
                MY_LOGD_IF(mLogLevel >= 2,"mvOut.mBuffer->getPlaneCount = %d", data.mBuffer->getPlaneCount());
                for(size_t k=0;k<data.mBuffer->getPlaneCount();k++)
                {
                    MY_LOGD_IF(mLogLevel >= 2,"mvOut.mBuffer->getBufVA(%d) = %X", k, data.mBuffer->getBufVA(k));
                    MY_LOGD_IF(mLogLevel >= 2,"mvOut.mBuffer->getBufStridesInBytes(%d) = %d", k, data.mBuffer->getBufStridesInBytes(k));
                }
            }
            else
            {
                MY_LOGD_IF(mLogLevel >= 2,"mvOut.mBuffer is NULL!!");
            }
            MY_LOGD_IF(mLogLevel >= 2,"mvOut.mTransform = %d", data.mTransform);
        }

        MY_LOGD_IF(mLogLevel >= 2,"mvCropRsInfo section");
        for(size_t j=0;j<rInputQParam.mvFrameParams[i].mvCropRsInfo.size(); j++)
        {
            MCrpRsInfo data = rInputQParam.mvFrameParams[i].mvCropRsInfo[j];
            MY_LOGD_IF(mLogLevel >= 2,"========\nIndex = %d", j);

            MY_LOGD_IF(mLogLevel >= 2,"CropRsInfo.mGroupID=%d", data.mGroupID);
            MY_LOGD_IF(mLogLevel >= 2,"CropRsInfo.mFrameGroup=%d", data.mFrameGroup);
            MY_LOGD_IF(mLogLevel >= 2,"CropRsInfo.mResizeDst=%dx%d", data.mResizeDst.w, data.mResizeDst.h);
            MY_LOGD_IF(mLogLevel >= 2,"CropRsInfo.mCropRect.p_fractional=(%d,%d) ", data.mCropRect.p_fractional.x, data.mCropRect.p_fractional.y);
            MY_LOGD_IF(mLogLevel >= 2,"CropRsInfo.mCropRect.p_integral=(%d,%d) ", data.mCropRect.p_integral.x, data.mCropRect.p_integral.y);
            MY_LOGD_IF(mLogLevel >= 2,"CropRsInfo.mCropRect.s=%dx%d ", data.mCropRect.s.w, data.mCropRect.s.h);
        }

        MY_LOGD_IF(mLogLevel >= 2,"mvModuleData section");
        for(size_t j=0;i<rInputQParam.mvFrameParams[i].mvModuleData.size(); j++)
        {
            ModuleInfo data = rInputQParam.mvFrameParams[i].mvModuleData[j];
            MY_LOGD_IF(mLogLevel >= 2,"========\nIndex = %d", j);

            MY_LOGD_IF(mLogLevel >= 2,"ModuleData.moduleTag=%d", data.moduleTag);
            MY_LOGD_IF(mLogLevel >= 2,"ModuleData.frameGroup=%d", data.frameGroup);

            _SRZ_SIZE_INFO_ *SrzInfo = (_SRZ_SIZE_INFO_ *) data.moduleStruct;
            MY_LOGD_IF(mLogLevel >= 2,"SrzInfo->in_w=%d", SrzInfo->in_w);
            MY_LOGD_IF(mLogLevel >= 2,"SrzInfo->in_h=%d", SrzInfo->in_h);
            MY_LOGD_IF(mLogLevel >= 2,"SrzInfo->crop_w=%d", SrzInfo->crop_w);
            MY_LOGD_IF(mLogLevel >= 2,"SrzInfo->crop_h=%d", SrzInfo->crop_h);
            MY_LOGD_IF(mLogLevel >= 2,"SrzInfo->crop_x=%d", SrzInfo->crop_x);
            MY_LOGD_IF(mLogLevel >= 2,"SrzInfo->crop_y=%d", SrzInfo->crop_y);
            MY_LOGD_IF(mLogLevel >= 2,"SrzInfo->crop_floatX=%d", SrzInfo->crop_floatX);
            MY_LOGD_IF(mLogLevel >= 2,"SrzInfo->crop_floatY=%d", SrzInfo->crop_floatY);
            MY_LOGD_IF(mLogLevel >= 2,"SrzInfo->out_w=%d", SrzInfo->out_w);
            MY_LOGD_IF(mLogLevel >= 2,"SrzInfo->out_h=%d", SrzInfo->out_h);
        }

    }
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
YUVEffectHal::
onFlush()
{
    FUNCTION_LOG_START;
    FUNCTION_LOG_END;
    return true;
}

};


