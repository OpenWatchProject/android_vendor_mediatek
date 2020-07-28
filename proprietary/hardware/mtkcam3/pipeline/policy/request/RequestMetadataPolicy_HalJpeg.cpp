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

#define LOG_TAG "mtkcam-RequestMetadataPolicy-HalJpeg"

#include "RequestMetadataPolicy_HalJpeg.h"
//
#include "MyUtils.h"
//
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>
/******************************************************************************
 *
 ******************************************************************************/
using namespace android;


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
namespace requestmetadata {


/******************************************************************************
 *
 ******************************************************************************/
RequestMetadataPolicy_HalJpeg::
RequestMetadataPolicy_HalJpeg(
    CreationParams const& params
)
    :RequestMetadataPolicy_Default(params)
{

}


/******************************************************************************
 *
 ******************************************************************************/
auto
RequestMetadataPolicy_HalJpeg::
evaluateRequest(
    EvaluateRequestParams const& params
) -> int
{
    auto ret = RequestMetadataPolicy_Default::evaluateRequest(params);
    //check for still capture intent
    bool isStillCap = false;

    if ( params.pRequest_ParsedAppMetaControl != nullptr ) {
        MY_LOGD("control_captureIntent = %x" , params.pRequest_ParsedAppMetaControl->control_captureIntent);
        switch (params.pRequest_ParsedAppMetaControl->control_captureIntent)
        {
        case MTK_CONTROL_CAPTURE_INTENT_STILL_CAPTURE:
            isStillCap = true;
            break;
            //
        case static_cast< uint8_t>(-1L):{//invalid parsed cache; use metadata
            IMetadata::IEntry const& CapIntent = params.pRequest_AppControl->entryFor(MTK_CONTROL_CAPTURE_INTENT);
            isStillCap = ( !CapIntent.isEmpty() && MTK_CONTROL_CAPTURE_INTENT_STILL_CAPTURE == CapIntent.itemAt(0, Type2Type<MUINT8>()));
            }
            break;
            //
        default:
            break;
        }
    }
    if ( isStillCap ) {
        MY_LOGD("set MTK_HAL_REQUEST_REQUIRE_EXIF = 1");
        IMetadata::IEntry entry2(MTK_HAL_REQUEST_REQUIRE_EXIF);
        entry2.push_back(1, Type2Type<MUINT8>());
        for(size_t i=0;i<params.pAdditionalHal->size();++i)
        (*params.pAdditionalHal)[i]->update(entry2.tag(), entry2);

        for(size_t i=0;i<params.pAdditionalHal->size();++i)
        {
            IMetadata::IEntry entry3(MTK_HAL_REQUEST_SENSOR_SIZE);
            entry3.push_back((*params.pSensorSize)[i], Type2Type<MSize>());
            (*params.pAdditionalHal)[i]->update(entry3.tag(), entry3);
        }
    }

    return ret;
}


/******************************************************************************
 *
 ******************************************************************************/
auto makePolicy_RequestMetadata_HalJpeg(
    CreationParams const& params
) -> std::shared_ptr<IRequestMetadataPolicy>
{
    return std::make_shared<RequestMetadataPolicy_HalJpeg>(params);
}


};  //namespace requestmetadata
};  //namespace policy
};  //namespace pipeline
};  //namespace v3
};  //namespace NSCam


