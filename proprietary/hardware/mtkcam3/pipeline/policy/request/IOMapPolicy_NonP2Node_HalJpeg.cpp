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

#define LOG_TAG "mtkcam-NonP2NodeIOMapPolicy-HalJpeg"

#include <mtkcam3/pipeline/policy/IIOMapPolicy.h>
#include <mtkcam3/pipeline/hwnode/NodeId.h>
#include <mtkcam3/pipeline/hwnode/StreamId.h>

//
#include "MyUtils.h"


/******************************************************************************
 *
 ******************************************************************************/
using namespace android;
using namespace NSCam::v3::pipeline;
using namespace NSCam::v3::pipeline::NSPipelineContext;

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


/******************************************************************************
 * HAL Jpeg version - decorator design pattern
 ******************************************************************************/
FunctionType_IOMapPolicy_NonP2Node makePolicy_IOMap_NonP2Node_HalJpeg(
        FunctionType_IOMapPolicy_NonP2Node f) {
    auto p= [](
            FunctionType_IOMapPolicy_NonP2Node fDecoratedPolicy,
            iomap::RequestOutputParams const& out,
            iomap::RequestInputParams const& in
    ) -> int
    {
        auto err =fDecoratedPolicy(out, in);
        if (in.pRequest_PipelineNodesNeed->needJpegNode && in.pConfiguration_StreamInfo_NonP1->pHalImage_Jpeg) {
            MY_LOGI("Enabled: needJpegNode=1");
            IOMap JpegMap;
            JpegMap.addOut(in.pConfiguration_StreamInfo_NonP1->pHalImage_Jpeg->getStreamId());

            android::sp<IImageStreamInfo> pInYuv_Main;
            for( const auto& n : in.pRequest_AppImageStreamInfo->vAppImage_Output_Proc ) {
                android::sp<IImageStreamInfo> pImageStreamInfo = n.second;

                if (pInYuv_Main == nullptr) {
                    pInYuv_Main = pImageStreamInfo;
                    continue;
                }

                //max. size?
                if (pInYuv_Main->getImgSize().w * pInYuv_Main->getImgSize().h
                        < pImageStreamInfo->getImgSize().w * pImageStreamInfo->getImgSize().h )
                {
                    pInYuv_Main = pImageStreamInfo;
                }
            }
            if (pInYuv_Main != nullptr ) {
                MY_LOGD("add inYuv %x", pInYuv_Main->getStreamId());
                JpegMap.addIn(pInYuv_Main->getStreamId());
            }

            //
            (*out.pNodeIOMapImage)[eNODEID_JpegNode] = IOMapSet().add(JpegMap);

            IOMap jpegMetaMap = IOMap();

            if (in.pConfiguration_StreamInfo_NonP1->pAppMeta_Control) {
                jpegMetaMap.addIn(in.pConfiguration_StreamInfo_NonP1->pAppMeta_Control->getStreamId());
            }
            if (in.pConfiguration_StreamInfo_NonP1->pHalMeta_DynamicP2CaptureNode) {
                jpegMetaMap.addIn(in.pConfiguration_StreamInfo_NonP1->pHalMeta_DynamicP2CaptureNode->getStreamId());
            }

            (*out.pNodeIOMapMeta) [eNODEID_JpegNode] = IOMapSet().add(jpegMetaMap);
        }
        return err;
    };
    MY_LOGA_IF(f == nullptr, "decoraated policy == nullptr");
    return std::bind(p, f, std::placeholders::_1/*out*/, std::placeholders::_2/*in*/);
}


};  //namespace policy
};  //namespace pipeline
};  //namespace v3
};  //namespace NSCam


