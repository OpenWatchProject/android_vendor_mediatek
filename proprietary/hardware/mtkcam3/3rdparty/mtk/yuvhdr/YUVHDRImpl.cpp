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
#define LOG_TAG "YUVHDRProvider"
//
#include <mtkcam/utils/std/Log.h>
#include <mtkcam/utils/std/ULog.h>
//
#include <stdlib.h>
#include <utils/Errors.h>
#include <utils/List.h>
#include <utils/RefBase.h>
#include <sstream>
//
#include <mtkcam/utils/metadata/client/mtk_metadata_tag.h>
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>
#include <mtkcam/utils/TuningUtils/FileDumpNamingRule.h>
#include <mtkcam/drv/iopipe/SImager/IImageTransform.h>
//
//
#include <mtkcam/utils/imgbuf/IIonImageBufferHeap.h>
//
#include <mtkcam/drv/IHalSensor.h>
#include <mtkcam/utils/std/Format.h>
//
#include <mtkcam3/pipeline/hwnode/NodeId.h>

#include <mtkcam/utils/metastore/ITemplateRequest.h>
#include <mtkcam/utils/metastore/IMetadataProvider.h>
#include <mtkcam3/3rdparty/plugin/PipelinePlugin.h>
#include <mtkcam3/3rdparty/plugin/PipelinePluginType.h>
//
using namespace NSCam;
using namespace android;
using namespace std;
using namespace NSCam::NSPipelinePlugin;
using namespace NSCam::TuningUtils;
/******************************************************************************
 *
 ******************************************************************************/
CAM_ULOG_DECLARE_MODULE_ID(MOD_LIB_AINR);
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
#define AEE_ASSERT(cond, fmt, arg...)   CAM_ULOG_ASSERT(NSCam::Utils::ULog::MOD_LIB_AINR, cond, fmt, ##arg);
//
#define FUNCTION_IN                 MY_LOGD("%s +", __FUNCTION__)
#define FUNCTION_OUT                MY_LOGD("%s -", __FUNCTION__)
//systrace
#if 1
#ifndef ATRACE_TAG
#define ATRACE_TAG                           ATRACE_TAG_CAMERA
#endif
#include <utils/Trace.h>

#define YHDR_TRACE_CALL()                      ATRACE_CALL()
#define YHDR_TRACE_NAME(name)                  ATRACE_NAME(name)
#define YHDR_TRACE_BEGIN(name)                 ATRACE_BEGIN(name)
#define YHDR_TRACE_END()                       ATRACE_END()
#else
#define YHDR_TRACE_CALL()
#define YHDR_TRACE_NAME(name)
#define YHDR_TRACE_BEGIN(name)
#define YHDR_TRACE_END()
#endif

using NSCam::NSIoPipe::NSSImager::IImageTransform;
/******************************************************************************
*
******************************************************************************/
class YHDRProviderImpl : public YuvPlugin::IProvider
{
    typedef YuvPlugin::Property Property;
    typedef YuvPlugin::Selection Selection;
    typedef YuvPlugin::Request::Ptr RequestPtr;
    typedef YuvPlugin::RequestCallback::Ptr RequestCallbackPtr;

public:

    virtual const Property& property()
    {
        FUNCTION_IN;
        static Property prop;
        static bool inited;

        if (!inited) {
            prop.mName = "MTK_YHDR";
            prop.mFeatures = MTK_FEATURE_YHDR_FOR_AINR;
            prop.mInPlace = MFALSE;
            prop.mFaceData = eFD_Current;
            prop.mPriority = 3;
            prop.mMultiFrame = MTRUE;
            inited = true;
        }
        FUNCTION_OUT;
        return prop;
    };

    virtual MERROR negotiate(Selection& sel)
    {
        FUNCTION_IN;
        if (0 == mEnable) {
            MY_LOGD("Force off YHDR plugin");
            FUNCTION_OUT;
            return -EINVAL;
        }

        // AINR denoised YUV
        if(sel.mMainFrame)
        {
            sel.mIBufferFull
                .setRequired(MTRUE)
                .addAcceptedFormat(eImgFmt_NV21)
                .addAcceptedSize(eImgSize_Full);

            sel.mOBufferFull
                .setRequired(MTRUE)
                .addAcceptedFormat(eImgFmt_NV21)
                .addAcceptedSize(eImgSize_Full);
        }
        else
        {   // Shore exposure YUV
            sel.mIBufferClean
                .setRequired(MTRUE)
                .addAcceptedFormat(eImgFmt_NV21)
                .addAcceptedSize(eImgSize_Full);
        }

        sel.mIMetadataDynamic.setRequired(MTRUE);
        sel.mIMetadataApp.setRequired(MTRUE);
        sel.mIMetadataHal.setRequired(MTRUE);
        sel.mOMetadataApp.setRequired(MTRUE);
        sel.mOMetadataHal.setRequired(MTRUE);

        FUNCTION_OUT;
        return OK;
    };

    virtual void init()
    {
        FUNCTION_IN;
        MY_LOGD("init");

        FUNCTION_OUT;
    };

    MBOOL dumpRequestYUV(MINT32 sensorId, RequestPtr pRequest, IImageBuffer* pYUVBuffer, const char* userString)
    {
        IMetadata *pIMetataHAL = pRequest->mIMetadataHal->acquire();
        const int DUMP_FILE_NAME_SIZE = 1024;
        char writepath[DUMP_FILE_NAME_SIZE] = {0};
        FILE_DUMP_NAMING_HINT hint;
        extract(&hint, pIMetataHAL);
        extract_by_SensorOpenId(&hint, sensorId);
        extract(&hint, pYUVBuffer);

        genFileName_YUV(writepath, DUMP_FILE_NAME_SIZE, &hint, TuningUtils::YUV_PORT_UNDEFINED, userString);
        pYUVBuffer->saveToFile(writepath);
        return MTRUE;
    }

    virtual MERROR process(RequestPtr pRequest,
                           RequestCallbackPtr pCallback = nullptr)
    {
        FUNCTION_IN;
        YHDR_TRACE_CALL();

        MY_LOGD("R/F:%d/%d isMainFram:%d", pRequest->mRequestNo, pRequest->mFrameNo, pRequest->mMainFrame);

        MBOOL bCheck = this->miOnGoingRequestNo != -1 && pRequest->mRequestNo != this->miOnGoingRequestNo;
        AEE_ASSERT(!bCheck, "YUVHDRProvider: Critical!The input RequestNo(%d) is DIFFERENT with the plugin's on-going request(%d)",
                        pRequest->mRequestNo, this->miOnGoingRequestNo);
        // record the current request
        this->miOnGoingRequestNo = pRequest->mRequestNo;
        //
        if(pRequest->mMainFrame)
        {
            // AINR-denoised YUV
            if (pRequest->mIBufferFull == nullptr)
            {
                MY_LOGE("Cannot find the AINR denoise YUV!");
                pCallback->onAborted(pRequest);
                return BAD_VALUE;
            }

            // output YUV
            if (pRequest->mOBufferFull == nullptr)
            {
                MY_LOGE("Cannot find the output YUV!");
                pCallback->onAborted(pRequest);
                return BAD_VALUE;
            }

            // FD Information only exist in the main frame
            IMetadata *pIMetataHAL = pRequest->mIMetadataHal->acquire();
            IMetadata::IEntry entryFaceRects = pIMetataHAL->entryFor(MTK_FEATURE_FACE_RECTANGLES);
            for (size_t i = 0; i < entryFaceRects.count(); i++) {
                MRect faceRect = entryFaceRects.itemAt(i, Type2Type<MRect>());
                MY_LOGD("Detected Face Rect[%zd]: (xmin, ymin, xmax, ymax) => (%d, %d, %d, %d)",
                    i,
                    faceRect.p.x,
                    faceRect.p.y,
                    faceRect.s.w,
                    faceRect.s.h);
            }
            //
            MY_LOGD("[IN] AINR-denoised YUV request arrived!");
            mAINRDenoiseYUVReady = true;
            mpAINRYUVRequest = pRequest;
            mpAINRYUVCallback = pCallback;
        }
        else
        {
            // Short exposure frame
            if (pRequest->mIBufferClean == nullptr) {

                MY_LOGE("Cannot find the Short Exposure YUV!");
                pCallback->onAborted(pRequest);
                return BAD_VALUE;

            }
            MY_LOGD("[IN] Short-Exposure frame arrived.");
            mShortExposureYUVReady = true;
            mpShortExpYUVRequest = pRequest;
            mpShortExpYUVCallback = pCallback;
        }

        if(mShortExposureYUVReady && mAINRDenoiseYUVReady)
        {
            MY_LOGD("All Frame is ready!");
            // AINR-denoised YUV
            IImageBuffer* pImgBuf_AINRDenoisYUV = mpAINRYUVRequest->mIBufferFull->acquire();
            // output buffer
            IImageBuffer* pImgBuf_Out = mpAINRYUVRequest->mOBufferFull->acquire();
            // Short exposure YUV
            IImageBuffer* pImgBuf_ShortExpYUV = mpShortExpYUVRequest->mIBufferClean->acquire();
            MINT32 sensorId = pRequest->mSensorId;
            dumpRequestYUV(sensorId, mpAINRYUVRequest, pImgBuf_AINRDenoisYUV, "AINR_DENOISED_YUV");
            dumpRequestYUV(sensorId, mpShortExpYUVRequest, pImgBuf_ShortExpYUV, "SE_FRAME_YUV");
            // put algo here
            // =====================================================================
            // use image transform to copy buffer for testing solution
            IImageTransform * transformer = IImageTransform::createInstance("YUVHDRProvider", sensorId);
            MRect roi;
            roi.p = MPoint(0,0);
            roi.s = pImgBuf_AINRDenoisYUV->getImgSize();
            MBOOL ret = transformer->execute(pImgBuf_AINRDenoisYUV, pImgBuf_Out, NULL, roi, 0 , 0xFFFFFFFF);
            if(!ret)
            {
                MY_LOGE("Failed to copy output buffer");
            }
            // =====================================================================
            // release all the buffer & meta
            #define RELEASE_DATA(buf)\
                if(buf != nullptr)\
                    buf->release();
            auto releaseReq = [](const RequestPtr& pReq)
            {
                RELEASE_DATA(pReq->mIBufferFull);
                RELEASE_DATA(pReq->mIBufferClean);
                RELEASE_DATA(pReq->mOBufferFull);
                RELEASE_DATA(pReq->mIMetadataDynamic);
                RELEASE_DATA(pReq->mIMetadataApp);
                RELEASE_DATA(pReq->mIMetadataHal);
                RELEASE_DATA(pReq->mOMetadataApp);
                RELEASE_DATA(pReq->mOMetadataHal);
            };
            releaseReq(mpAINRYUVRequest);
            releaseReq(mpShortExpYUVRequest);
            // finish and call onCompleted callback
            mpAINRYUVCallback->onCompleted(mpAINRYUVRequest, OK);
            mpShortExpYUVCallback->onCompleted(mpShortExpYUVRequest, OK);
            // reset variables
            mAINRDenoiseYUVReady = false;
            mpAINRYUVRequest = nullptr;
            mpAINRYUVCallback = nullptr;
            mShortExposureYUVReady = false;
            mpShortExpYUVRequest = nullptr;
            mpShortExpYUVCallback = nullptr;
            miOnGoingRequestNo = -1;
        }

        FUNCTION_OUT;
        return OK;
    };

    virtual void abort(vector<RequestPtr>& pRequests __unused)
    {
        FUNCTION_IN;
        FUNCTION_OUT;
        MY_LOGW("Because we have bg service no need to implement");
    };

    virtual void uninit()
    {
        FUNCTION_IN;
        FUNCTION_OUT;
    };

    YHDRProviderImpl()
    {
        FUNCTION_IN;
        MY_LOGD("YHDRProviderImpl ctr");

        mEnable = ::property_get_int32("vendor.debug.camera.YHDR.enable", 1);

        FUNCTION_OUT;
    };

    virtual ~YHDRProviderImpl()
    {
        FUNCTION_IN;

        FUNCTION_OUT;
    };

private:
    int    mEnable;

    bool   mAINRDenoiseYUVReady = false;
    RequestPtr mpAINRYUVRequest = nullptr;
    RequestCallbackPtr mpAINRYUVCallback = nullptr;

    bool   mShortExposureYUVReady = false;
    RequestPtr mpShortExpYUVRequest = nullptr;
    RequestCallbackPtr mpShortExpYUVCallback = nullptr;
    // record the ongoing request no
    MINT32 miOnGoingRequestNo = -1;
};

REGISTER_PLUGIN_PROVIDER(Yuv, YHDRProviderImpl);

