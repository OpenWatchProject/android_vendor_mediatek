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

#define LOG_TAG "mtkcam-RequestMetadataPolicy_SMVRBatch"

#include "RequestMetadataPolicy.h"
//
#include "MyUtils.h"
//
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>
#include <mtkcam/utils/metastore/IMetadataProvider.h>
#include <mtkcam/utils/std/ULog.h>

CAM_ULOG_DECLARE_MODULE_ID(MOD_PIPELINE_POLICY);
/******************************************************************************
 *
 ******************************************************************************/
using namespace android;


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

#define MY_LOGV_IF(cond, ...)       do { if (            (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if (            (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if (            (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( CC_UNLIKELY(cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( CC_UNLIKELY(cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( CC_UNLIKELY(cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( CC_UNLIKELY(cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)


/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {
namespace v3 {
namespace pipeline {
namespace policy {
namespace requestmetadata {

class RequestMetadataPolicy_SMVRBatch : public IRequestMetadataPolicy
{
public:
    virtual auto    evaluateRequest(
                        EvaluateRequestParams const& params
                    ) -> int;

public:
    // RequestMetadataPolicy Interfaces.
    RequestMetadataPolicy_SMVRBatch(CreationParams const& params);
    virtual ~RequestMetadataPolicy_SMVRBatch() { }

private:
    CreationParams mPolicyParams;
    MINT32         mUniqueKey;
};


/******************************************************************************
 *
 ******************************************************************************/
RequestMetadataPolicy_SMVRBatch::
RequestMetadataPolicy_SMVRBatch(
    CreationParams const& params
)
    :mPolicyParams(params)
{
    mUniqueKey = NSCam::Utils::TimeTool::getReadableTime();
}


/******************************************************************************
 *
 ******************************************************************************/
auto
RequestMetadataPolicy_SMVRBatch::
evaluateRequest(
    EvaluateRequestParams const& params
) -> int
{

    // update request unique key
    if (params.pAdditionalHal->size() > 0)
    {
        IMetadata::IEntry entry(MTK_PIPELINE_UNIQUE_KEY);
        IMetadata::IEntry entry1 = (*params.pAdditionalHal)[0]->entryFor(MTK_PIPELINE_UNIQUE_KEY);
        if (entry1.isEmpty())
        {
            entry.push_back(mUniqueKey, Type2Type<MINT32>());
        }
        else
        {
            entry = entry1;
        }
        for(size_t i=0;i<params.pAdditionalHal->size();++i){
            (*params.pAdditionalHal)[i]->update(entry.tag(), entry);
        }
    }

    // update request id
    {
        IMetadata::IEntry entry(MTK_PIPELINE_REQUEST_NUMBER);
        entry.push_back(params.requestNo, Type2Type<MINT32>());
        for(size_t i=0;i<params.pAdditionalHal->size();++i){
            (*params.pAdditionalHal)[i]->update(entry.tag(), entry);
        }
    }


    MUINT8 bRepeating = (MUINT8) params.pRequest_ParsedAppMetaControl->repeating;
    {
        IMetadata::IEntry entry(MTK_HAL_REQUEST_REPEAT);
        entry.push_back(bRepeating, Type2Type< MUINT8 >());
        for(size_t i=0;i<params.pAdditionalHal->size();++i)
            (*params.pAdditionalHal)[i]->update(entry.tag(), entry);
        //MY_LOGD("Control AppMetadata is repeating(%d)", bRepeating);
    }
    {

        if ( // params.isZSLMode || /* SMVRBatch: no need for ZSL */
             params.pRequest_AppImageStreamInfo->pAppImage_Jpeg != nullptr
             // || params.pRequest_AppImageStreamInfo->pAppImage_Output_Priv != nullptr /* SMVRBatch: no need for private/Paque reproc */
             //|| params.needExif /* SMVRBatch: no need for Exif */
           )
        {
            //MY_LOGD("set MTK_HAL_REQUEST_REQUIRE_EXIF = 1");
            IMetadata::IEntry entry2(MTK_HAL_REQUEST_REQUIRE_EXIF);
            entry2.push_back(1, Type2Type<MUINT8>());
            for(size_t i=0;i<params.pAdditionalHal->size();++i)
                (*params.pAdditionalHal)[i]->update(entry2.tag(), entry2);
        }

        for(size_t i=0;i<params.pAdditionalHal->size();++i)
        {
            IMetadata::IEntry entry3(MTK_HAL_REQUEST_SENSOR_SIZE);
            entry3.push_back((*params.pSensorSize)[i], Type2Type<MSize>());
            (*params.pAdditionalHal)[i]->update(entry3.tag(), entry3);
        }
    }

    //!!NOTES:
    // In SMVRBatch case, RRZO size should be decided by streams output size
    // Setting MTK_P1NODE_SENSOR_CROP_REGION will cause P1Node to re-calculate RRZO size, which should not be applied
    auto pParsedAppConfiguration = mPolicyParams.pPipelineUserConfiguration->pParsedAppConfiguration;
    auto pParsedSMVRBatchInfo = (pParsedAppConfiguration != nullptr) ? pParsedAppConfiguration->pParsedSMVRBatchInfo : nullptr;
    if (pParsedSMVRBatchInfo != nullptr)
    {
        MY_LOGD_IF(2 <= pParsedSMVRBatchInfo->logLevel, "SMVRBatch: no need to change rrzo size");
    }

    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
auto makePolicy_RequestMetadata_SMVRBatch(
    CreationParams const& params
) -> std::shared_ptr<IRequestMetadataPolicy>
{
    return std::make_shared<RequestMetadataPolicy_SMVRBatch>(params);
}


};  //namespace requestmetadata
};  //namespace policy
};  //namespace pipeline
};  //namespace v3
};  //namespace NSCam

