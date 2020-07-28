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

#define LOG_TAG "mtkcam-P1DmaNeedPolicy"

#include <mtkcam3/pipeline/policy/IConfigP1DmaNeedPolicy.h>
//
#include <mtkcam/utils/LogicalCam/IHalLogicalDeviceList.h>
#include <camera_custom_eis.h>
#include <mtkcam3/feature/3dnr/3dnr_defs.h>
#include <mtkcam/utils/std/ULog.h>
//
#include "MyUtils.h"

CAM_ULOG_DECLARE_MODULE_ID(MOD_PIPELINE_POLICY);


/******************************************************************************
 *
 ******************************************************************************/
using namespace android;
using namespace NSCam::v3::pipeline::policy;
using namespace NSCam::NR3D;


/******************************************************************************
 *
 ******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_ULOGMV("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_ULOGMD("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_ULOGMI("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_ULOGMW("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_ULOGME("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("[%s] " fmt, __FUNCTION__, ##arg)


/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {
namespace v3 {
namespace pipeline {
namespace policy {

bool
p1DmaOutputQuery(
    bool &needLcso,
    bool &needRsso,
    bool &needFDYUV,
    StreamingFeatureSetting const* pStreamingFeatureSetting
)
{
    // LCSO
#if MTKCAM_LTM_SUPPORT
    MBOOL normalLTM = MTRUE;
#else
    MBOOL normalLTM = MFALSE;
#endif
    MBOOL mHDR = ((pStreamingFeatureSetting->hdrSensorMode == SENSOR_VHDR_MODE_MVHDR));
    needLcso = (mHDR || normalLTM);

    // RSSO
    needRsso = false;
    if( pStreamingFeatureSetting->bNeedRSS )
    {
        MY_LOGD("use RSSO");
        needRsso = true;
    }

    needFDYUV = pStreamingFeatureSetting->bNeedP1FDYUV;

    return true;
}

/**
 * Make a function target - default version
 */
FunctionType_Configuration_P1DmaNeedPolicy makePolicy_Configuration_P1DmaNeed_Default()
{
    return [](
        std::vector<uint32_t>* pvOut,
        std::vector<P1HwSetting> const* pP1HwSetting,
        StreamingFeatureSetting const* pStreamingFeatureSetting,
        PipelineStaticInfo const* pPipelineStaticInfo,
        PipelineUserConfiguration const* pPipelineUserConfiguration __unused
    ) -> int{

        bool needLcso, needRsso;
        bool needFDYUV = false;
        p1DmaOutputQuery(needLcso, needRsso, needFDYUV, pStreamingFeatureSetting);
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
            if (needFDYUV && idx == 0 && ((*pP1HwSetting)[idx].fdyuvSize.size() != 0)) {
                setting |= P1_FDYUV; // for isp6.0
            }

            pvOut->push_back(setting);
        }
        return OK;

    };
}

/**
 * Make a function target - multi-cam version
 */
FunctionType_Configuration_P1DmaNeedPolicy makePolicy_Configuration_P1DmaNeed_MultiCam()
{
    return [](
        std::vector<uint32_t>* pvOut,
        std::vector<P1HwSetting> const* pP1HwSetting,
        StreamingFeatureSetting const* pStreamingFeatureSetting,
        PipelineStaticInfo const* pPipelineStaticInfo,
        PipelineUserConfiguration const* pPipelineUserConfiguration __unused
    ) -> int{

        bool needLcso, needRsso;
        bool needFDYUV = false;
        p1DmaOutputQuery(needLcso, needRsso, needFDYUV, pStreamingFeatureSetting);
        //
        for (size_t idx = 0; idx < pPipelineStaticInfo->sensorId.size(); idx++)
        {
            uint32_t setting = 0;

            if ((*pP1HwSetting)[idx].imgoDefaultRequest.imageSize.size()) {
                setting |= P1_IMGO;
            }
            // if usingCamSV is true, it cannot set rrzo.
            if ((*pP1HwSetting)[idx].rrzoDefaultRequest.imageSize.size() && !(*pP1HwSetting)[idx].usingCamSV) {
                setting |= P1_RRZO;
            }
            if (needLcso) {
                setting |= P1_LCSO;
            }
            if (needRsso) {
                setting |= P1_RSSO;
            }
            if (needFDYUV && idx == 0 && ((*pP1HwSetting)[idx].fdyuvSize.size() != 0)) {
                setting |= P1_FDYUV; // for isp6.0
            }
            if ((*pP1HwSetting)[idx].canSupportScaledYuv) {
                setting |= P1_SCALEDYUV; // for isp6.0
            }

            pvOut->push_back(setting);
        }
        return OK;

    };
}

};  //namespace policy
};  //namespace pipeline
};  //namespace v3
};  //namespace NSCam

