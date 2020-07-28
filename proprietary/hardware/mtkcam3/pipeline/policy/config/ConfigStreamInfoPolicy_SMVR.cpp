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

#define LOG_TAG "mtkcam-ConfigStreamInfoPolicy_SMVR"

#include <mtkcam3/pipeline/policy/IConfigStreamInfoPolicy.h>
//
#include <mtkcam/utils/metastore/IMetadataProvider.h>
#include <mtkcam/utils/metadata/client/mtk_metadata_tag.h>
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>
#include <mtkcam/utils/std/ULog.h>
//
#include "MyUtils.h"
#include "P1StreamInfoBuilder.h"

CAM_ULOG_DECLARE_MODULE_ID(MOD_PIPELINE_POLICY);


/******************************************************************************
 *
 ******************************************************************************/
using namespace android;
using namespace NSCam;
using namespace NSCam::v3::pipeline::policy;
using namespace NSCam::v3;
using namespace NSCam::v3::Utils;


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
//
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


/**
 * default version
 */
FunctionType_Configuration_StreamInfo_P1 makePolicy_Configuration_StreamInfo_P1_SMVR()
{
    return [](Configuration_StreamInfo_P1_Params const& params) -> int {
        auto pvOut = params.pvOut;
        auto pvP1HwSetting = params.pvP1HwSetting;
        auto pvP1DmaNeed = params.pvP1DmaNeed;
        auto pPipelineNodesNeed = params.pPipelineNodesNeed;
        auto pStreamingFeatureSetting = params.pStreamingFeatureSetting;
        auto pCaptureFeatureSetting = params.pCaptureFeatureSetting;
        auto pPipelineStaticInfo = params.pPipelineStaticInfo;
        auto pPipelineUserConfiguration = params.pPipelineUserConfiguration;
        auto const& pParsedAppConfiguration __unused = pPipelineUserConfiguration->pParsedAppConfiguration;
        auto const& pParsedAppImageStreamInfo   = pPipelineUserConfiguration->pParsedAppImageStreamInfo;

        MINT32 iBurstNum=1;
        {
            sp<IMetadataProvider> metaProvider = NSMetadataProviderManager::valueFor(pPipelineStaticInfo->sensorId[0]);
            if(metaProvider == NULL)
            {
                MY_LOGE("Can not get metadata provider for search burst number!! set burst number to 1");
            }
            else
            {
                IMetadata staMeta = metaProvider->getMtkStaticCharacteristics();
                IMetadata::IEntry entry = staMeta.entryFor(MTK_CONTROL_AVAILABLE_HIGH_SPEED_VIDEO_CONFIGURATIONS);
                if (!entry.isEmpty())
                {
                    MBOOL hit = MFALSE;
                    for ( size_t i=0; i<entry.count(); i+=5 )
                    {
                        if ( pParsedAppImageStreamInfo->videoImageSize.w == entry.itemAt(i,   Type2Type<int32_t>()) &&
                             pParsedAppImageStreamInfo->videoImageSize.h == entry.itemAt(i+1, Type2Type<int32_t>()) )
                        {
                            iBurstNum = entry.itemAt(i+4, Type2Type<MINT32>());
                            hit = MTRUE;
                            break;
                        }
                    }
                    if ( !hit )
                    {
                        MY_LOGW("no matching high speed profile(%dx%d)", pParsedAppImageStreamInfo->videoImageSize.w, pParsedAppImageStreamInfo->videoImageSize.h);
                    }
                }
                else
                {
                    MY_LOGW("no static MTK_CONTROL_AVAILABLE_HIGH_SPEED_VIDEO_CONFIGURATIONS");
                }
            }
        }

        for(size_t i = 0; i < pPipelineNodesNeed->needP1Node.size(); i++)
        {
            ParsedStreamInfo_P1 config;
            if (pPipelineNodesNeed->needP1Node[i])
            {
                //MetaData
                {
                    P1MetaStreamInfoBuilder builder(i);
                    builder.setP1AppMetaDynamic_Default().setMaxBufNum(10*iBurstNum);
                    config.pAppMeta_DynamicP1 = builder.build();
                }
                {
                    P1MetaStreamInfoBuilder builder(i);
                    builder.setP1HalMetaDynamic_Default().setMaxBufNum(10*iBurstNum);
                    config.pHalMeta_DynamicP1 = builder.build();
                }
                {
                    P1MetaStreamInfoBuilder builder(i);
                    builder.setP1HalMetaControl_Default().setMaxBufNum(10*iBurstNum);
                    config.pHalMeta_Control = builder.build();
                }

                std::shared_ptr<IHwInfoHelper> infohelper = IHwInfoHelperManager::get()->getHwInfoHelper(pPipelineStaticInfo->sensorId[i]);
                MY_LOGF_IF(infohelper==nullptr, "getHwInfoHelper");

                P1ImageStreamInfoBuilder::CtorParams const builderCtorParams{
                        .index = i,
                        .pHwInfoHelper = infohelper,
                        .pCaptureFeatureSetting = pCaptureFeatureSetting,
                        .pStreamingFeatureSetting = pStreamingFeatureSetting,
                        .pPipelineStaticInfo = pPipelineStaticInfo,
                        .pPipelineUserConfiguration = pPipelineUserConfiguration,
                    };

                MINT32 P1_streamBufCnt = 8;
                //IMGO
                if ((*pvP1DmaNeed)[i] & P1_IMGO)
                {
                    P1ImageStreamInfoBuilder builder(builderCtorParams);
                    builder.setP1Imgo_Default(P1_streamBufCnt*iBurstNum, (*pvP1HwSetting)[i]);
                    config.pHalImage_P1_Imgo = builder.build();
                }
                //RRZO
                if ((*pvP1DmaNeed)[i] & P1_RRZO)
                {
                    P1ImageStreamInfoBuilder builder(builderCtorParams);
                    builder.setP1Rrzo_Default(P1_streamBufCnt*iBurstNum, (*pvP1HwSetting)[i]);
                    config.pHalImage_P1_Rrzo = builder.build();
                }
                //STT
                if ((*pvP1DmaNeed)[i] & P1_LCSO)
                {
                    P1STTImageStreamInfoBuilder builder(builderCtorParams);
                    builder.setMaxBufNum(P1_streamBufCnt*iBurstNum);
                    builder.setP1Stt_Default(pPipelineStaticInfo->openId);
                    config.pHalImage_P1_Lcso = builder.build();
                }
                //RSSO
                if ((*pvP1DmaNeed)[i] & P1_RSSO)
                {
                    P1ImageStreamInfoBuilder builder(builderCtorParams);
                    builder.setP1Rsso_Default(P1_streamBufCnt*iBurstNum, (*pvP1HwSetting)[i]);
                    config.pHalImage_P1_Rsso = builder.build();
                }
            }
            pvOut->push_back(std::move(config));
        }

        return OK;
    };
}


};  //namespace policy
};  //namespace pipeline
};  //namespace v3
};  //namespace NSCam

