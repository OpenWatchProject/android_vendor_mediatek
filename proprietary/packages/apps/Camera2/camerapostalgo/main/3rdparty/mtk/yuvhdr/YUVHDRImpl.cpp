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
#include <utils/std/Log.h>
//
#include <stdlib.h>
#include <utils/Errors.h>
#include <utils/List.h>
#include <utils/RefBase.h>
#include <sstream>
#include "BufferUtils.h"
#include "vndk/hardware_buffer.h"
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
#define LOG_DETAILS "debug.camerapostalgo.feature.yuvhdr.logdetails"
static int8_t gLogDetail = ::property_get_bool(LOG_DETAILS, 0);
#define FUNCTION_IN   do { if (gLogDetail) MY_LOGD("%s +", __FUNCTION__); } while(0)
#define FUNCTION_OUT  do { if (gLogDetail) MY_LOGD("%s -", __FUNCTION__); } while(0)
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

/******************************************************************************
*
******************************************************************************/
class YHDRProviderImpl : public MultiFramePlugin::IProvider
{
    typedef MultiFramePlugin::Property Property;
    typedef MultiFramePlugin::Selection Selection;
    typedef MultiFramePlugin::Request::Ptr RequestPtr;
    typedef MultiFramePlugin::RequestCallback::Ptr RequestCallbackPtr;

public:

    virtual void set(MINT32 iOpenId, MINT32 iOpenId2)
    {
        FUNCTION_IN;
        MY_LOGD("set openId:%d openId2:%d", iOpenId, iOpenId2);
        mOpenid = iOpenId;
        FUNCTION_OUT;
    }

    virtual const Property& property()
    {
        FUNCTION_IN;
        static Property prop;
        static bool inited;

        if (!inited) {
            prop.mName = "MTK_YHDR";
            prop.mFeatures = MTK_FEATURE_HDR;
            prop.mFaceData = eFD_None; //eFD_Current;
            prop.mPriority = 3;
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
        sel.mIBufferFull
            .setRequired(MTRUE)
            .addAcceptedFormat(eImgFmt_YV12)
            .addAcceptedSize(eImgSize_Full);

        if(sel.mRequestIndex == 0)
        {
            MY_LOGD("negotiate need mOBufferFull");
            sel.mOBufferFull
                .setRequired(MTRUE)
                .addAcceptedFormat(eImgFmt_YV12)
                .addAcceptedSize(eImgSize_Full);
        }

        sel.mIMetadataDynamic.setRequired(MFALSE);
        sel.mIMetadataApp.setRequired(MFALSE);
        sel.mIMetadataHal.setRequired(MFALSE);
        sel.mOMetadataApp.setRequired(MFALSE);
        sel.mOMetadataHal.setRequired(MFALSE);
        sel.mRequestCount = 3;

        FUNCTION_OUT;
        return OK;
    };

    virtual void init()
    {
        FUNCTION_IN;
        MY_LOGD("init");

        FUNCTION_OUT;
    };


    virtual MERROR process(RequestPtr pRequest, RequestCallbackPtr pCallback =
            nullptr) {
        FUNCTION_IN;
        YHDR_TRACE_CALL();

        MY_LOGD("process index:%d Count:%d",
            pRequest->mRequestIndex, pRequest->mRequestCount);

        if (CC_UNLIKELY( mvRequests.size() != pRequest->mRequestIndex ))
            MY_LOGE("Input sequence of requests from P2A is wrong");

        mvRequests.push_back(pRequest);

        if (pRequest->mRequestIndex == pRequest->mRequestCount - 1)
        {
        MY_LOGD("have collected all requests, do YUVHDRProcess");
            RequestPtr pRequest = mvRequests.at(0);
            if (pRequest->mIBufferFull != nullptr) {
                MY_LOGD_IF(1,
                          "[IN] Full image Size:(%d,%d)", pRequest->mIBufferFull->acquire()->getImgSize().w, pRequest->mIBufferFull->acquire()->getImgSize().h);
            }
            IImageBuffer* pOImgBuffer = nullptr;
            if (pRequest->mOBufferFull != nullptr) {
                pOImgBuffer = pRequest->mOBufferFull->acquire();
                MY_LOGD_IF(1,
                    "[OUT] Full image Size:(%d,%d)", pOImgBuffer->getImgSize().w, pOImgBuffer->getImgSize().h);
            }
            MY_LOGD("copy input content to output");
            // copy input content to output
            BufferUtils::mdpConvertFormatAndResize(
                    pRequest->mIBufferFull->acquire(),
                    pRequest->mOBufferFull->acquire());


            for (auto req : mvRequests) {
                MY_LOGD("callback request(%d/%d) %p",
                        req->mRequestIndex,
                        req->mRequestCount, pCallback.get());
                if (pCallback != nullptr) {
                    pCallback->onCompleted(req, 0);
                }
            }
            mvRequests.clear();
        }

        FUNCTION_OUT;
        return OK;
    };

    virtual void abort(vector<RequestPtr>& pRequests)
    {
        FUNCTION_IN;
        FUNCTION_OUT;
        MY_LOGW("Because we have bg service no need to implement");
        MY_LOGV("0x%p", (void *)&pRequests);
    };

    virtual void uninit()
    {
        FUNCTION_IN;
        FUNCTION_OUT;
    };

    YHDRProviderImpl()
        :mOpenid(-1)
    {
        FUNCTION_IN;
        MY_LOGD("YHDRProviderImpl ctr");

        mEnable = 1;
        //mEnable = ::property_get_int32("vendor.debug.camera.YHDR.enable", 1);

        FUNCTION_OUT;
    };

    virtual ~YHDRProviderImpl()
    {
        FUNCTION_IN;

        FUNCTION_OUT;
    };

private:
    int    mOpenid;
    int    mEnable;

    std::vector<RequestPtr> mvRequests;
};

REGISTER_PLUGIN_PROVIDER(MultiFrame, YHDRProviderImpl);
