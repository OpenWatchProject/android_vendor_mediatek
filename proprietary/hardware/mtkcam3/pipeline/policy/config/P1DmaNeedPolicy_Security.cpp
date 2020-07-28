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

#define LOG_TAG "mtkcam-P1DmaNeedPolicy-Security"

#include <mtkcam3/pipeline/policy/IConfigP1DmaNeedPolicy.h>
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
 * Make a function target - Security version
 */
FunctionType_Configuration_P1DmaNeedPolicy makePolicy_Configuration_P1DmaNeed_Security()
{
    return [](
        std::vector<uint32_t>* pvOut,
        std::vector<P1HwSetting> const* pP1HwSetting,
        StreamingFeatureSetting const* pStreamingFeatureSetting,
        PipelineStaticInfo const* pPipelineStaticInfo,
        PipelineUserConfiguration const* pPipelineUserConfiguration __unused
    ) -> int{

        bool needLcso = false;
        bool needRsso = false;  /* pStreamingFeatureSetting->bNeedRSS */
        bool needFDYUV = (pStreamingFeatureSetting->bNeedP1FDYUV && ((*pP1HwSetting)[0].fdyuvSize.size() > 0));
        MY_LOGD("pStreamingFeatureSetting->bNeedP1FDYUV(%d),(*pP1HwSetting)[0].fdyuvSize.size(%zu),needFDYUV(%d)",
            pStreamingFeatureSetting->bNeedP1FDYUV, (*pP1HwSetting)[0].fdyuvSize.size(),needFDYUV);
        //
#if MTKCAM_LTM_SUPPORT
        needLcso = true;
#else
        needLcso = false;
#endif
        //
        for (size_t idx = 0; idx < pPipelineStaticInfo->sensorId.size(); idx++)
        {
            uint32_t setting = 0;
            if ((*pP1HwSetting)[idx].imgoDefaultRequest.imageSize.size()) {
                setting |= P1_IMGO;
            }
            if ((*pP1HwSetting)[idx].rrzoDefaultRequest.imageSize.size()) {
                setting |= P1_RRZO;
            }
            if (needLcso) {
                setting |= P1_LCSO;
            }
            if (needRsso) {
                setting |= P1_RSSO;
            }

#ifdef MTKCAM_HAVE_SECURE_FD_SUPPORT
            if (needFDYUV && idx == 0) {
                setting |= P1_FDYUV; // for isp6.0
            }
#endif
            MY_LOGD("setting(0x%x)",setting);
            pvOut->push_back(setting);
        }
        return OK;

    };
}


};  //namespace policy
};  //namespace pipeline
};  //namespace v3
};  //namespace NSCam

