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

#define LOG_TAG "mtkcam-P2NodeDecisionPolicy-AppRaw16Reprocess"

#include <mtkcam3/pipeline/policy/IP2NodeDecisionPolicy.h>
#include <mtkcam/utils/std/ULog.h>

//
#include "MyUtils.h"

CAM_ULOG_DECLARE_MODULE_ID(MOD_PIPELINE_POLICY);


/******************************************************************************
 *
 ******************************************************************************/
using namespace android;
using namespace NSCam;
using namespace NSCam::v3;
using namespace NSCam::v3::pipeline::policy;
using namespace NSCam::v3::pipeline::policy::p2nodedecision;

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
 * App Raw16 reprocessing version
 */
FunctionType_P2NodeDecisionPolicy
makePolicy_P2NodeDecision_AppRaw16Reprocess(FunctionType_P2NodeDecisionPolicy f)
{
    auto p = [](
        FunctionType_P2NodeDecisionPolicy fDecoratedPolicy,
        p2nodedecision::RequestOutputParams& out,
        p2nodedecision::RequestInputParams const& in
    ) -> int {
        auto pRequest_AppImageStreamInfo = in.pRequest_AppImageStreamInfo;

        if (!(in.hasP2CaptureNode) && !(in.hasP2StreamNode))
        {
            out.needP2CaptureNode = false;
            out.needP2StreamNode = false;
            MY_LOGD("didn't have p2 node.....");
            return OK;
        }
        /**
         * [A] Input RAW16
         *
         * Use P2CaptureNode to output all P2 streams.
         */
        if ( pRequest_AppImageStreamInfo->pAppImage_Input_RAW16 != nullptr ) {
            CAM_ULOGMD("[requestNo:%u] Input RAW16: P2CaptureNode outputs all P2 streams", in.requestNo);

            CAM_LOGD_IF((!in.hasP2CaptureNode || !in.hasP2StreamNode),
                    "[requestNo:%u] hasP2CaptureNode:%d hasP2StreamNode:%d",
                    in.requestNo, in.hasP2CaptureNode, in.hasP2StreamNode);

            p2nodedecision::RequestInputParams tempIn = in;
            tempIn.hasP2CaptureNode = true;
            tempIn.hasP2StreamNode = false;

            return fDecoratedPolicy(out, tempIn);
        }

        /**
         * [B] Output RAW16 (like Preview scenario + Output RAW16)
         *
         * Use P2StreamingNode to output all P2 streams, regardless of capture intents.
         */
        if ( pRequest_AppImageStreamInfo->vAppImage_Output_RAW16.size() != 0 ||
             pRequest_AppImageStreamInfo->vAppImage_Output_RAW16_Physical.size() != 0 )
        {
            CAM_ULOGMD("[requestNo:%u] Output RAW16: P2StreamingNode outputs all P2 streams", in.requestNo);

            CAM_LOGD_IF((!in.hasP2CaptureNode || !in.hasP2StreamNode),
                    "[requestNo:%u] hasP2CaptureNode:%d hasP2StreamNode:%d",
                    in.requestNo, in.hasP2CaptureNode, in.hasP2StreamNode);

            p2nodedecision::RequestInputParams tempIn = in;
            tempIn.hasP2CaptureNode = false;
            tempIn.hasP2StreamNode = true;

            return fDecoratedPolicy(out, tempIn);
        }

        /**
         * [C] Output priv raw (like Preview scenario + Output priv raw)
         *
         * Use P2StreamingNode to output all P2 streams, regardless of capture intents.
         */
        if ( pRequest_AppImageStreamInfo->pAppImage_Output_Priv.get() != nullptr )
        {
            CAM_LOGD("[requestNo:%u] Output Priv RAW: P2StreamingNode outputs all P2 streams", in.requestNo);

            CAM_LOGD_IF((!in.hasP2CaptureNode || !in.hasP2StreamNode),
                    "[requestNo:%u] hasP2CaptureNode:%d hasP2StreamNode:%d",
                    in.requestNo, in.hasP2CaptureNode, in.hasP2StreamNode);

            p2nodedecision::RequestInputParams tempIn = in;
            tempIn.hasP2CaptureNode = false;
            tempIn.hasP2StreamNode = true;

            return fDecoratedPolicy(out, tempIn);
        }

        /**
         * [D] No Input/Output RAW16 (like Preview scenario)
         *
         * As usual, use default policy for serving P2 streams.
         */
        return fDecoratedPolicy(out, in);
    };

    MY_LOGA_IF(f==nullptr, "decoraated policy == nullptr");
    return std::bind(p, f, std::placeholders::_1, std::placeholders::_2);
}


};  //namespace policy
};  //namespace pipeline
};  //namespace v3
};  //namespace NSCam

