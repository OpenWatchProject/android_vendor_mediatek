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
#define LOG_TAG "TP_FBProvider"
//
#if 0
#include <mtkcam/utils/std/Log.h>
//
#include <stdlib.h>
#include <utils/Errors.h>
#include <utils/List.h>
#include <utils/RefBase.h>
#include <sstream>
//
#include <mtkcam/utils/metadata/client/mtk_metadata_tag.h>
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>
//
//
#include <mtkcam/utils/imgbuf/IIonImageBufferHeap.h>
//
#include <mtkcam/drv/IHalSensor.h>
#include <utils/std/Format.h>
//
#include <mtkcam3/pipeline/hwnode/NodeId.h>

#include <mtkcam/utils/metastore/ITemplateRequest.h>
#include <mtkcam/utils/metastore/IMetadataProvider.h>
#include <mtkcam3/3rdparty/plugin/PipelinePlugin.h>
#include <mtkcam3/3rdparty/plugin/PipelinePluginType.h>
#endif
#include <utils/std/Log.h>
//
#include <stdlib.h>
#include <utils/Errors.h>
#include <utils/List.h>
#include <utils/RefBase.h>
#include <sstream>
//
#include <mtk/mtk_platform_metadata_tag.h>

#include <utils/std/Format.h>
#include <plugin/PipelinePlugin.h>
#include <plugin/PipelinePluginType.h>
#include <cutils/properties.h>

//
using namespace NSCam;
using namespace android;
using namespace std;
using namespace NSCam::NSPipelinePlugin;
using namespace com::mediatek::campostalgo::NSFeaturePipe;

/******************************************************************************
 *
 ******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_LOGV("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
//
#define ASSERT(cond, msg)           do { if (!(cond)) { printf("Failed: %s\n", msg); return; } }while(0)
//
#include <cutils/properties.h>
#define LOG_DETAILS "debug.camerapostalgo.feature.tpfb.logdetails"
static int8_t gLogDetail = ::property_get_bool(LOG_DETAILS, 0);
#define FUNCTION_IN()   do { if (gLogDetail) MY_LOGD("%s +", __FUNCTION__); } while(0)
#define FUNCTION_OUT()  do { if (gLogDetail) MY_LOGD("%s -", __FUNCTION__); } while(0)

//systrace
#if 1
#ifndef ATRACE_TAG
#define ATRACE_TAG                           ATRACE_TAG_CAMERA
#endif
#include <utils/Trace.h>

#define FB_TRACE_CALL()                      ATRACE_CALL()
#define FB_TRACE_NAME(name)                  ATRACE_NAME(name)
#define FB_TRACE_BEGIN(name)                 ATRACE_BEGIN(name)
#define FB_TRACE_END()                       ATRACE_END()
#else
#define FB_TRACE_CALL()
#define FB_TRACE_NAME(name)
#define FB_TRACE_BEGIN(name)
#define FB_TRACE_END()
#endif

/******************************************************************************
*
******************************************************************************/
class TPFBProviderImpl : public YuvPlugin::IProvider
{
    typedef YuvPlugin::Property Property;
    typedef YuvPlugin::Selection Selection;
    typedef YuvPlugin::Request::Ptr RequestPtr;
    typedef YuvPlugin::RequestCallback::Ptr RequestCallbackPtr;

public:

    virtual void set(MINT32 iOpenId, MINT32 iOpenId2)
    {
        FUNCTION_IN();
        MY_LOGD("set openId:%d openId2:%d", iOpenId, iOpenId2);
        mOpenid = iOpenId;
        FUNCTION_OUT();
    }

    virtual const Property& property()
    {
        FUNCTION_IN();
        static Property prop;
        static bool inited;

        if (!inited) {
            prop.mName = "THIRD_PARTY_FB";
            prop.mFeatures = TP_FEATURE_FB; //test
            prop.mInPlace = MTRUE;
            prop.mFaceData = eFD_Current;
            prop.mPosition = 1;
            inited = true;
        }
        FUNCTION_OUT();
        return prop;
    };

    virtual MERROR negotiate(Selection& sel)
    {
        FUNCTION_IN();
        if (0 == mEnable) {
            MY_LOGD("Force off TP FB plugin");
            return -EINVAL;
        }
        sel.mIBufferFull
            .setRequired(MTRUE)
            .addAcceptedFormat(eImgFmt_I420)
            .addAcceptedSize(eImgSize_Full);

        sel.mIMetadataDynamic.setRequired(MTRUE);
        sel.mIMetadataApp.setRequired(MTRUE);
        sel.mIMetadataHal.setRequired(MTRUE);
        sel.mOMetadataApp.setRequired(MTRUE);
        sel.mOMetadataHal.setRequired(MTRUE);

        FUNCTION_OUT();
        return OK;
    };

    virtual void init()
    {
        FUNCTION_IN();

        FUNCTION_OUT();
    };

    virtual MERROR process(RequestPtr pRequest,
                           RequestCallbackPtr pCallback = nullptr)
    {
        FUNCTION_IN();
        FB_TRACE_CALL();

        IImageBuffer* pIBufferFull = NULL;
        /*IMetadata *pIMetataHAL = NULL;*/

        if (pRequest->mIBufferFull != nullptr) {
            pIBufferFull = pRequest->mIBufferFull->acquire();
            MY_LOGD("[IN] Full image VA: 0x%p", (void *)pIBufferFull->getBufVA(0));
        }

        if (pRequest->mOBufferFull != nullptr) {
            IImageBuffer* pImgBuffer = pRequest->mOBufferFull->acquire();
            MY_LOGD("[OUT] Full image VA: 0x%p", (void *)pImgBuffer->getBufVA(0));
        }

        if (pRequest->mIMetadataDynamic != nullptr) {
            IMetadata *meta = pRequest->mIMetadataDynamic->acquire();
            if (meta != NULL)
                MY_LOGD("[IN] Dynamic metadata count: %d", meta->count());
            else
                MY_LOGD("[IN] Dynamic metadata empty");
        }

        /*if (pRequest->mIMetadataHal != nullptr) {
            pIMetataHAL = pRequest->mIMetadataHal->acquire();
            if (pIMetataHAL != NULL)
                MY_LOGD("[IN] HAL metadata count: %d", pIMetataHAL->count());
            else
                MY_LOGD("[IN] HAL metadata empty");
        }*/

        if (pIBufferFull != NULL /*&& pIMetataHAL != NULL*/)
        {
            MRect faceRect;
            char* pBufferVa = (char *) (pIBufferFull->getBufVA(0));
            MUINT32 stride = pIBufferFull->getBufStridesInBytes(0);
            /*IMetadata::IEntry entryFaceRects = pIMetataHAL->entryFor(MTK_FEATURE_FACE_RECTANGLES);
            for (size_t i = 0; i < entryFaceRects.count(); i++)*/
            {
                /*faceRect = entryFaceRects.itemAt(i, Type2Type<MRect>());*/
                faceRect.p.x = 400;
                faceRect.p.y = 900;
                faceRect.s.w = 700;
                faceRect.s.h = 1200;
                MY_LOGD("Detected Face Rect: (xmin, ymin, xmax, ymax) => (%d, %d, %d, %d)",
                    faceRect.p.x,
                    faceRect.p.y,
                    faceRect.s.w,
                    faceRect.s.h);

                // draw rectangles to output buffer
                memset(
                    pBufferVa + stride * faceRect.p.y + faceRect.p.x,
                    255, faceRect.s.w - faceRect.p.x + 1);

                memset(
                    pBufferVa + stride * faceRect.s.h + faceRect.p.x,
                    255, faceRect.s.w - faceRect.p.x + 1);

                for (auto j = faceRect.p.y + 1; j < faceRect.s.h ; j++) {
                    *(pBufferVa + stride * j + faceRect.p.x) = 255;
                    *(pBufferVa + stride * j + faceRect.s.w) = 255;
                }
            }
        }
        if (pCallback != nullptr) {
            MY_LOGD("callback request");
            pCallback->onCompleted(pRequest, 0);
        }
        FUNCTION_OUT();
        return 0;
    };

    virtual void abort(vector<RequestPtr>& pRequests)
    {
        FUNCTION_IN();
        MY_LOGV("0x%p", (void *)&pRequests);
        FUNCTION_OUT();
    };

    virtual void uninit()
    {
        FUNCTION_IN();

        FUNCTION_OUT();
    };

    TPFBProviderImpl()
        :mOpenid(-1)
    {
        FUNCTION_IN();
        mEnable = ::property_get_int32("vendor.debug.camera.tpfb.enable", 0);

        FUNCTION_OUT();
    };

    virtual ~TPFBProviderImpl()
    {
        FUNCTION_IN();

        FUNCTION_OUT();
    };

private:
    int    mOpenid;
    int    mEnable;
};

REGISTER_PLUGIN_PROVIDER(Yuv, TPFBProviderImpl);

