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
 * MediaTek Inc. (C) 2017. All rights reserved.
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
#define LOG_TAG "ContextBuilder/P2NodeDataMain2"
//
#include <utils/StrongPointer.h>
//
#include <mtkcam/utils/std/Log.h>
#include <mtkcam/middleware/v1/LegacyPipeline/stereo/ContextBuilder/MetaStreamManager.h>
#include <mtkcam/middleware/v1/LegacyPipeline/stereo/ContextBuilder/ImageStreamManager.h>
#include <mtkcam/middleware/v1/LegacyPipeline/stereo/ContextBuilder/StereoBasicParameters.h>
//
#include <mtkcam/pipeline/hwnode/P2Node.h>
//
#include <mtkcam/middleware/v1/LegacyPipeline/StreamId.h>
//
#include "StereoP2NodeMain2ConfigData.h"
#if (MTKCAM_STEREO_DENOISE_SUPPORT == '1')
//
    #include <mtkcam/feature/stereo/StereoCamEnum.h>
#endif
#include <mtkcam/utils/hw/HwInfoHelper.h>
//
#include <camera_custom_stereo.h>
using namespace android;
using namespace NSCam;
using namespace v3;
#if (MTKCAM_STEREO_DENOISE_SUPPORT == '1')
using namespace NSCam::v1::Stereo;
#endif
//
/******************************************************************************
 *
 ******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_LOGV("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)

//
typedef P2Node                          NodeT;
typedef NodeActor< NodeT >              MyNodeActorT;
//
StereoP2NodeMain2ConfigData::
StereoP2NodeMain2ConfigData(
    MINT32 openId,
    NodeId_T nodeId,
    char const* nodeName)
    : INodeConfigBase(openId, nodeId, nodeName)
{
    initParam.openId = mOpenId;
    initParam.nodeId = mNodeId;
    initParam.nodeName = mNodeName;
}
//
StereoP2NodeMain2ConfigData::
~StereoP2NodeMain2ConfigData()
{
    MY_LOGD("dcot(%x)", this);
}
//
sp<INodeActor>
StereoP2NodeMain2ConfigData::
getNode()
{
    MY_LOGD("+");
    //
    sp<MyNodeActorT> pNode = new MyNodeActorT( NodeT::createInstance() );
    //
    pNode->setInitParam(initParam);
    pNode->setConfigParam(cfgParam);
    //
    MY_LOGD("-");
    return pNode;
}
//
void
StereoP2NodeMain2ConfigData::
configNode(
    MetaStreamManager* metaManager,
    ImageStreamManager* imageManager,
    StereoBasicParameters* userParams,
    PipelineContext* pipeContext)
{
#define DIN     MTRUE
#define DOUT    MFALSE

    MY_LOGD("+");
    //
    sp<IImageStreamInfo> spStrmInfo = NULL;
    //
    // ==========================
    // Meta Stream
    // ==========================
    addStreamBegin(MTRUE);
    if(metaManager != NULL)
    {
        //
        cfgParam.pInAppMeta = metaManager->getStreamInfoByStreamId(eSTREAMID_META_HAL_DYNAMIC_STEREOROOT);
        addStream(cfgParam.pInAppMeta, DIN);
        //
        cfgParam.pInHalMeta = metaManager->getStreamInfoByStreamId(eSTREAMID_META_HAL_DYNAMIC_P1_MAIN2);
        addStream(cfgParam.pInHalMeta, DIN);
        //
        cfgParam.pOutAppMeta = metaManager->getStreamInfoByStreamId(eSTREAMID_META_APP_DYNAMIC_P2_MAIN2);
        addStream(cfgParam.pOutAppMeta, DOUT);
        //
        cfgParam.pOutHalMeta = metaManager->getStreamInfoByStreamId(eSTREAMID_META_HAL_DYNAMIC_P2_MAIN2);
        addStream(cfgParam.pOutHalMeta, DOUT);
    }
    addStreamEnd(MTRUE);
    // ==========================
    // Image Stream
    // ==========================
    if(imageManager != NULL)
    {
        addStreamBegin(MFALSE);
        // rrzo
        spStrmInfo = imageManager->getStreamInfoByStreamId(eSTREAMID_IMAGE_PIPE_RAW_RESIZER_01);
        if( spStrmInfo!= NULL)
        {
            cfgParam.pInResizedRaw = spStrmInfo;
            addStream(spStrmInfo, DIN);
        }
        // resize yuv
        spStrmInfo = imageManager->getStreamInfoByStreamId(eSTREAMID_IMAGE_PIPE_THIRDPARTY_RESIZE_YUV_1);
        if(spStrmInfo != NULL)
        {
            cfgParam.vOutImage.push_back(spStrmInfo);
            addStream(spStrmInfo, DOUT);
        }
        addStreamEnd(MFALSE);

        if(::supportMain2FullRaw())
        {
            addStreamBegin_keepIOSet(MFALSE); //we need 2 IO maps, so we do not clear IOMapSet
            // imgo
            spStrmInfo = imageManager->getStreamInfoByStreamId(eSTREAMID_IMAGE_PIPE_RAW_OPAQUE_01);
            if(spStrmInfo != NULL)
            {
                cfgParam.pvInFullRaw.push_back(spStrmInfo);
                addStream(spStrmInfo, DIN);
            }
            // lcso
            spStrmInfo = imageManager->getStreamInfoByStreamId(eSTREAMID_IMAGE_PIPE_RAW_LCSO_01);
            if(spStrmInfo != NULL)
            {
                cfgParam.pInLcsoRaw = spStrmInfo;
                addStream(spStrmInfo, DIN);
            }
            // full yuv
            spStrmInfo = imageManager->getStreamInfoByStreamId(eSTREAMID_IMAGE_PIPE_THIRDPARTY_FULLSIZE_YUV_1);
            if(spStrmInfo != NULL)
            {
                cfgParam.vOutImage.push_back(spStrmInfo);
                addStream(spStrmInfo, DOUT);
            }
            addStreamEnd(MFALSE);
        }

    }
    spStrmInfo = NULL;
    MY_LOGD("-");
}
//
void
StereoP2NodeMain2ConfigData::
dump()
{
    MY_LOGD("Dump P2Node Config Data +");
    //
    MY_LOGD("InitParam - openId(%d), NodeID(%d), NodeName=%s", initParam.openId, initParam.nodeId, initParam.nodeName);
    //
    MY_LOGD("StreamIOSet +");
    for(MUINT32 i=0;i<mvImageIOSet.size();++i)
    {
        for(MUINT32 j=0;j<mvImageIOSet[i].vIn.size();++j)
        {
            MY_LOGD("InImageIOMap StreamId(%d)", mvImageIOSet[i].vIn[j]);
        }
        for(MUINT32 j=0;j<mvImageIOSet[i].vOut.size();++j)
        {
            MY_LOGD("OutImageIOMap StreamId(%d)", mvImageIOSet[i].vOut[j]);
        }
    }
    MY_LOGD("StreamIOSet -");
    //
    MY_LOGD("MetaIOSet +");
    for(MUINT32 i=0;i<mvMetaIOSet.size();++i)
    {
        for(MUINT32 j=0;j<mvMetaIOSet[i].vIn.size();++j)
        {
            MY_LOGD("InMetaIOMap StreamId(%d)", mvMetaIOSet[i].vIn[j]);
        }
        for(MUINT32 j=0;j<mvMetaIOSet[i].vOut.size();++j)
        {
            MY_LOGD("OutMetaIOMap StreamId(%d)", mvMetaIOSet[i].vOut[j]);
        }
    }
    MY_LOGD("MetaIOSet -");
    MY_LOGD("Dump P2Node Config Data -");
}
//
void
StereoP2NodeMain2ConfigData::
destroy()
{
    cfgParam.pInAppMeta = NULL;
    cfgParam.pInHalMeta = NULL;
    cfgParam.pOutAppMeta = NULL;
    cfgParam.pOutHalMeta = NULL;
    cfgParam.pInResizedRaw = NULL;
    cfgParam.pvInFullRaw.clear();

    mvInStreamSet.clear();
    mvOutStreamSet.clear();
    mvStreamUsageSet.clear();
}
