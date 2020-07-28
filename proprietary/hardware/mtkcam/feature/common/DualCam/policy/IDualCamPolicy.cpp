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
 * MediaTek Inc. (C) 2016. All rights reserved.
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

#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>
#include <mtkcam/utils/metadata/IMetadata.h>


#ifdef LOG_TAG
    #undef LOG_TAG
#endif

#define LOG_TAG "MtkCam/DualCamPolicy"

#include <mtkcam/feature/DualCam/IDualCamPolicy.h>

#if (MTKCAM_HAVE_DUAL_ZOOM_SUPPORT==1)
#include "DualCamZoomPolicy.h"
#endif

#if (MTKCAM_HAVE_DUALCAM_DENOISE_SUPPORT == 1)
#include "DualCamDenoisePolicy.h"
#endif

#if (MTKCAM_HAVE_STEREO_CAMERA_SUPPORT == 1)
#include "DualCamBokehPolicy.h"
#endif

#undef MY_LOGV
#undef MY_LOGD
#undef MY_LOGI
#undef MY_LOGW
#undef MY_LOGE
#undef MY_LOGA
#undef MY_LOGF

#define MY_LOGV(fmt, arg...)        CAM_LOGV("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("[%s] " fmt, __FUNCTION__, ##arg)

namespace NSCam
{
IDualCamPolicy* IDualCamPolicy::createInstance(MINT32 scenario)
{
    IDualCamPolicy* policy = NULL;

    switch(scenario)
    {
#if (MTKCAM_HAVE_DUAL_ZOOM_SUPPORT==1)
        case DUAL_CAM_POLICY_MTK_TK:
            MY_LOGD("DUAL_CAM_POLICY_MTK_TK");
            policy = new DualCamZoomPolicy();
            break;

        case DUAL_CAM_POLICY_3RD_PARTY_CP:
            // *****ToDo *****
            // policy = new DualCamCPPolicy();
            break;
#endif
#if (MTKCAM_HAVE_DUALCAM_DENOISE_SUPPORT == 1)
        case DUAL_CAM_POLICY_DENOISE:
            MY_LOGD("DUAL_CAM_POLICY_DENOISE");
            policy = new DualCamDenoisePolicy();
            break;
#endif
#if (MTKCAM_HAVE_STEREO_CAMERA_SUPPORT == 1)
#if (MTKCAM_HAVE_VSDOF_SUPPORT == 1)
        case DUAL_CAM_POLICY_BOKEH:
            MY_LOGD("DUAL_CAM_POLICY_BOKEH");
            policy = new DualCamBokehPolicy();
            break;
#endif
#endif
    }

    return policy;
}

}; // NSCam
