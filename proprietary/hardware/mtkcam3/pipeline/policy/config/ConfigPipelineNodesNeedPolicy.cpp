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

#define LOG_TAG "mtkcam-ConfigPipelineNodesNeedPolicy"

#include <mtkcam3/pipeline/policy/IConfigPipelineNodesNeedPolicy.h>
//
#include "MyUtils.h"


/******************************************************************************
 *
 ******************************************************************************/
using namespace android;
using namespace NSCam::v3::pipeline::policy;


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
 * Make a function target as a policy for Non-P1 - default version
 */
FunctionType_Configuration_PipelineNodesNeedPolicy makePolicy_Configuration_PipelineNodesNeed_Default()
{
    return [](Configuration_PipelineNodesNeed_Params const& params) -> int {
        auto pOut = params.pOut;
        auto pPipelineStaticInfo = params.pPipelineStaticInfo;
        auto pPipelineUserConfiguration = params.pPipelineUserConfiguration;
        auto const& pParsedAppConfiguration   = pPipelineUserConfiguration->pParsedAppConfiguration;
        auto const& pParsedAppImageStreamInfo = pPipelineUserConfiguration->pParsedAppImageStreamInfo;
        auto bIsZslMode = params.bIsZslMode;

        //for (size_t i = 0; i < pPipelineStaticInfo->sensorId.size(); i++) {
        // default pipeline will contain 1 p1node.
        pOut->needP1Node.push_back(true);
        //}

        if (pParsedAppImageStreamInfo->vAppImage_Output_Proc.size() != 0)
        {
            pOut->needP2StreamNode  = true;
        }
        else
        {
            pOut->needP2StreamNode  = false;
        }
        if (pParsedAppImageStreamInfo->vAppImage_Output_Proc.size() != 0 || pParsedAppImageStreamInfo->pAppImage_Jpeg != nullptr)
        {
            pOut->needP2CaptureNode = pParsedAppConfiguration->isConstrainedHighSpeedMode ? false : true;
        }
        else if((bIsZslMode || pPipelineStaticInfo->isNeedP2ProcessRaw) &&
                (!pParsedAppImageStreamInfo->vAppImage_Output_RAW16.empty() || !pParsedAppImageStreamInfo->vAppImage_Output_RAW16_Physical.empty() ||
                  pParsedAppImageStreamInfo->pAppImage_Output_Priv))
        {
            MY_LOGD("bIsZslMode(%d) pParsedAppImageStreamInfo->vAppImage_Output_RAW16.size(%zu) pParsedAppImageStreamInfo->vAppImage_Output_RAW16_Physical.size(%zu) pParsedAppImageStreamInfo->pAppImage_Output_Priv(%p)",
                bIsZslMode, pParsedAppImageStreamInfo->vAppImage_Output_RAW16.size(), pParsedAppImageStreamInfo->vAppImage_Output_RAW16_Physical.size(),pParsedAppImageStreamInfo->pAppImage_Output_Priv.get());
            pOut->needP2CaptureNode = true;
        }
        else
        {
            pOut->needP2CaptureNode = false;
        }
        if (::property_get_int32("vendor.debug.camera.fd.disable", MTKCAM_HAVE_FD_SUPPORT == 0) == 0)
        {
            if (pParsedAppConfiguration->useP1DirectFDYUV)
            {
                pOut->needFDNode = (false == pParsedAppConfiguration->isConstrainedHighSpeedMode);
            }
            else
            {
                pOut->needFDNode = (false == pParsedAppConfiguration->isConstrainedHighSpeedMode)
                                 && pOut->needP2StreamNode;
            }
        }
        else
        {
            pOut->needFDNode        = false;
        }
        pOut->needJpegNode      = pParsedAppImageStreamInfo->pAppImage_Jpeg.get() ? true : false;
        if (pParsedAppConfiguration->useP1DirectAppRaw)
        {
            pOut->needRaw16Node     = false;
        }
        else
        {
            pOut->needRaw16Node     = pParsedAppImageStreamInfo->vAppImage_Output_RAW16.empty() ? false : true;
            pOut->needRaw16Node     = pOut->needRaw16Node || (pParsedAppImageStreamInfo->vAppImage_Output_RAW16_Physical.empty() ? false : true);
            MY_LOGD("pOut->needRaw16Node : %d", pOut->needRaw16Node);
        }
        pOut->needPDENode       = pPipelineStaticInfo->isType3PDSensorWithoutPDE;
        pOut->needP1RawIspTuningDataPackNode = (pParsedAppConfiguration->hasTuningEnable);
        pOut->needP2RawIspTuningDataPackNode = (pParsedAppConfiguration->hasTuningEnable && (bIsZslMode || pPipelineStaticInfo->isNeedP2ProcessRaw));
        pOut->needP2YuvIspTuningDataPackNode = (pParsedAppConfiguration->hasTuningEnable && pOut->needP2CaptureNode);

        return OK;
    };
}


};  //namespace policy
};  //namespace pipeline
};  //namespace v3
};  //namespace NSCam

