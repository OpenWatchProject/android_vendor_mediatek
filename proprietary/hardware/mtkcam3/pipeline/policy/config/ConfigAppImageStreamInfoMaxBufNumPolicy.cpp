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

#define LOG_TAG "mtkcam-ConfigAppImageStreamInfoMaxBufNumPolicy"

#include <mtkcam3/pipeline/policy/IConfigAppImageStreamInfoMaxBufNumPolicy.h>
//
#include <mtkcam3/pipeline/stream/IStreamInfo.h>
//
#include "MyUtils.h"

#if MTKCAM_MIN_LOW_MEM
    #define APPYUV_MAX_SIZE (7)
    #define APPLARGEYUV_MAX_SIZE (5)
#else
    #define APPYUV_MAX_SIZE (12)
#endif
/******************************************************************************
 *
 ******************************************************************************/
using namespace android;
using namespace NSCam;
using namespace NSCam::v3::pipeline::policy;
using namespace NSCam::v3;


/******************************************************************************
 *
 ******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_LOGV("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("[%s] " fmt, __FUNCTION__, ##arg)


/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {
namespace v3 {
namespace pipeline {
namespace policy {


/**
 * Make a function target as a policy - default version
 */
FunctionType_Configuration_AppImageStreamInfoMaxBufNumPolicy makePolicy_Configuration_AppImageStreamInfoMaxBufNum_Default()
{
    return [](
        ParsedAppImageStreamInfo* pInOut,
        StreamingFeatureSetting const* pStreamingFeatureSetting,
        CaptureFeatureSetting const* pCaptureFeatureSetting,
        PipelineStaticInfo const* pPipelineStaticInfo __unused,
        PipelineUserConfiguration const* pPipelineUserConfiguration
    ) -> int {

        auto const& pParsedAppConfiguration __unused = pPipelineUserConfiguration->pParsedAppConfiguration;
        uint32_t const maxJpegNum = ( pCaptureFeatureSetting )?
                                    pCaptureFeatureSetting->maxAppJpegStreamNum : 1;
        auto const maxRaw16OutputBufNum = ( pCaptureFeatureSetting ) ?
                                    pCaptureFeatureSetting->maxAppRaw16OutputBufferNum : 1;

        if (pInOut->pAppImage_Input_RAW16.get()) {
            pInOut->pAppImage_Input_RAW16->setMaxBufNum(1);
        }
        if (pInOut->pAppImage_Jpeg.get()) {
            pInOut->pAppImage_Jpeg->setMaxBufNum(maxJpegNum);
        }
        if (pInOut->pAppImage_Input_Yuv.get()) {
            pInOut->pAppImage_Input_Yuv->setMaxBufNum(2);
        }
        if (pInOut->pAppImage_Input_Priv.get()) {
            pInOut->pAppImage_Input_Priv->setMaxBufNum(2);
        }
        if (pInOut->pAppImage_Output_Priv.get()) {
            pInOut->pAppImage_Output_Priv->setMaxBufNum(6);
        }
        if (pInOut->pAppImage_Output_IspTuningData_Raw.get()) {
            pInOut->pAppImage_Output_IspTuningData_Raw->setMaxBufNum(12);
        }
        if (pInOut->pAppImage_Output_IspTuningData_Yuv.get()) {
            pInOut->pAppImage_Output_IspTuningData_Yuv->setMaxBufNum(12);
        }
        for(auto &data : pInOut->vAppImage_Output_IspTuningData_Raw_Physical)
        {
            data.second->setMaxBufNum(12);
        }
        for(auto &data : pInOut->vAppImage_Output_IspTuningData_Yuv_Physical)
        {
            data.second->setMaxBufNum(12);
        }

        sp<IImageStreamInfo> pStreamInfo;
        for( const auto& n : pInOut->vAppImage_Output_Proc )
        {
            if  ( (pStreamInfo = n.second).get() )
            {
                if ( pStreamInfo->getUsageForConsumer() & GRALLOC_USAGE_HW_VIDEO_ENCODER )
                {
                    pStreamInfo->setMaxBufNum(APPYUV_MAX_SIZE + pStreamingFeatureSetting->eisExtraBufNum);
                }
                else
                {
                    #if MTKCAM_MIN_LOW_MEM
                    if ( pStreamInfo->getImgSize().w > 1920 )
                    {
                        MY_LOGD("low mem : set app buffer count = %d", APPLARGEYUV_MAX_SIZE);
                        pStreamInfo->setMaxBufNum(APPLARGEYUV_MAX_SIZE);
                    }
                    else
                    {
                        MY_LOGD("low mem : set app buffer count = %d", APPYUV_MAX_SIZE);
                        pStreamInfo->setMaxBufNum(APPYUV_MAX_SIZE);
                    }
                    #else
                    pStreamInfo->setMaxBufNum(APPYUV_MAX_SIZE);
                    #endif
                }
            }
        }
        for( const auto& yuv : pInOut->vAppImage_Output_Proc_Physical )
        {
            for( const auto& n : yuv.second )
            {
                if  ( (pStreamInfo = n).get() )
                {
                    if ( pStreamInfo->getUsageForConsumer() & GRALLOC_USAGE_HW_VIDEO_ENCODER )
                    {
                        pStreamInfo->setMaxBufNum(8 + pStreamingFeatureSetting->eisExtraBufNum);
                    }
                    else
                    {
                        pStreamInfo->setMaxBufNum(APPYUV_MAX_SIZE);
                    }
                }
            }
        }
        for( const auto& n : pInOut->vAppImage_Output_RAW16 )
        {
            if  ( (pStreamInfo = n.second).get() )
            {
                pStreamInfo->setMaxBufNum(maxRaw16OutputBufNum);
            }
        }

        for( const auto& raw16 : pInOut->vAppImage_Output_RAW16_Physical )
        {
            for( const auto &n : raw16.second ) {
                if  ( (pStreamInfo = n).get() )
                {
                    pStreamInfo->setMaxBufNum(maxRaw16OutputBufNum);
                }
            }
        }

        return OK;
    };
}


/**
 * Make a function target as a policy - SMVR version
 */
FunctionType_Configuration_AppImageStreamInfoMaxBufNumPolicy makePolicy_Configuration_AppImageStreamInfoMaxBufNum_SMVR()
{
    return [](
        ParsedAppImageStreamInfo* pInOut,
        StreamingFeatureSetting const* pStreamingFeatureSetting __unused,
        CaptureFeatureSetting const* pCaptureFeatureSetting __unused,
        PipelineStaticInfo const* pPipelineStaticInfo __unused,
        PipelineUserConfiguration const* pPipelineUserConfiguration
    ) -> int {

        auto const& pParsedAppConfiguration __unused = pPipelineUserConfiguration->pParsedAppConfiguration;
        auto const &pParsedSMVRBatchInfo = (pParsedAppConfiguration != nullptr) ? pParsedAppConfiguration->pParsedSMVRBatchInfo : nullptr;

        if (pInOut->pAppImage_Jpeg.get()) {
            pInOut->pAppImage_Jpeg->setMaxBufNum(1);
        }
        if (pInOut->pAppImage_Input_Yuv.get()) {
            pInOut->pAppImage_Input_Yuv->setMaxBufNum(2);
        }
        if (pInOut->pAppImage_Input_Priv.get()) {
            pInOut->pAppImage_Input_Priv->setMaxBufNum(2);
        }
        if (pInOut->pAppImage_Output_Priv.get()) {
            pInOut->pAppImage_Output_Priv->setMaxBufNum(6);
        }

        sp<IImageStreamInfo> pStreamInfo;
        for( const auto& n : pInOut->vAppImage_Output_Proc )
        {
            if  ( (pStreamInfo = n.second).get() )
            {
                if ( (pStreamInfo->getUsageForConsumer() & GRALLOC_USAGE_HW_VIDEO_ENCODER )
                    && pParsedSMVRBatchInfo == nullptr /* smvrBatch: no need w/ as many as 52  */)

                {
                    pStreamInfo->setMaxBufNum(52);
                }
                else
                {
                    pStreamInfo->setMaxBufNum(APPYUV_MAX_SIZE);
                }
            }
        }
        for( const auto& n : pInOut->vAppImage_Output_RAW16 )
        {
            if  ( (pStreamInfo = n.second).get() )
            {
                pStreamInfo->setMaxBufNum(1);
            }
        }
        for( const auto& raw16 : pInOut->vAppImage_Output_RAW16_Physical )
        {
            for( const auto &n : raw16.second ) {
                if  ( (pStreamInfo = n).get() )
                {
                    pStreamInfo->setMaxBufNum(1);
                }
            }
        }

        return OK;
    };
}


};  //namespace policy
};  //namespace pipeline
};  //namespace v3
};  //namespace NSCam

