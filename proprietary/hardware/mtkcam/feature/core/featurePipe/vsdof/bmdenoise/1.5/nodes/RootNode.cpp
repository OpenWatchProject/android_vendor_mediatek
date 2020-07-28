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
#include "RootNode.h"

#define PIPE_MODULE_TAG "BMDeNoise"
#define PIPE_CLASS_TAG "RootNode"
#define PIPE_LOG_TAG PIPE_MODULE_TAG PIPE_CLASS_TAG

#include <PipeLog.h>

#define CHECK_OBJECT(x)  do{\
    if (x == nullptr) {\
        MY_LOGE("Null %s Object", #x);\
        handleData(ERROR_OCCUR_NOTIFY, request);\
        return -MFALSE;}\
} while(0)

using namespace NSCam::NSCamFeature::NSFeaturePipe;
/*******************************************************************************
 *
 ********************************************************************************/
RootNode::
RootNode(const char *name,
    Graph_T *graph,
    MINT32 openId)
    : BMDeNoisePipeNode(name, graph)
    , miOpenId(openId)
{
    MY_LOGD("ctor(%p)", this);
    this->addWaitQueue(&mRequests);
}
/*******************************************************************************
 *
 ********************************************************************************/
RootNode::
~RootNode()
{
    MY_LOGD("dctor(0x%x)", this);
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
RootNode::
onData(
    DataID id,
    PipeRequestPtr &request)
{
    ScopeLogger logger(PIPE_LOG_TAG, __FUNCTION__);
    MBOOL ret = MFALSE;
    switch(id)
    {
        case ROOT_ENQUE:
            mRequests.enque(request);
            ret = MTRUE;
            break;
        default:
            ret = MFALSE;
            MY_LOGE("unknown data id :%d", id);
            break;
    }
    return ret;
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
RootNode::
onInit()
{
    CAM_TRACE_CALL();
    ScopeLogger logger(PIPE_LOG_TAG, __FUNCTION__);
    if(!BMDeNoisePipeNode::onInit()){
        MY_LOGE("BMDeNoisePipeNode::onInit() failed!");
        return MFALSE;
    }
    return MTRUE;
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
RootNode::
onUninit()
{
    ScopeLogger logger(PIPE_LOG_TAG, __FUNCTION__);
    return MTRUE;
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
RootNode::
onThreadStart()
{
    ScopeLogger logger(PIPE_LOG_TAG, __FUNCTION__);

    // query some sensor static info
    {
        IHalSensorList* sensorList = MAKE_HalSensorList();
        int sensorDev_Main1 = sensorList->querySensorDevIdx(mSensorIdx_Main1);

        SensorStaticInfo sensorStaticInfo;
        memset(&sensorStaticInfo, 0, sizeof(SensorStaticInfo));
        sensorList->querySensorStaticInfo(sensorDev_Main1, &sensorStaticInfo);

        mBayerOrder_main1 = sensorStaticInfo.sensorFormatOrder;
    }

    // query some sensor static info
    {
        IHalSensorList* sensorList = MAKE_HalSensorList();
        int sensorDev_Main2 = sensorList->querySensorDevIdx(mSensorIdx_Main2);

        SensorStaticInfo sensorStaticInfo;
        memset(&sensorStaticInfo, 0, sizeof(SensorStaticInfo));
        sensorList->querySensorStaticInfo(sensorDev_Main2, &sensorStaticInfo);

        mBayerOrder_main2 = sensorStaticInfo.sensorFormatOrder;
    }

    return MTRUE;
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
RootNode::
onThreadStop()
{
    ScopeLogger logger(PIPE_LOG_TAG, __FUNCTION__);
    return MTRUE;
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
RootNode::
onThreadLoop()
{
    ScopeLogger logger(PIPE_LOG_TAG, __FUNCTION__);
    PipeRequestPtr pipeRequest = nullptr;

    if( !waitAllQueue() )// block until queue ready, or flush() breaks the blocking state too.
    {
        return MFALSE;
    }
    if( !mRequests.deque(pipeRequest) )
    {
        MY_LOGD("mRequests.deque() failed");
        return MFALSE;
    }

    this->incExtThreadDependency();
    CAM_TRACE_NAME("RootNode::onThreadLoop");

    if(!dispatch(pipeRequest)){
        MY_LOGE("failed to dispatch, please check above errors!");
        this->decExtThreadDependency();
    }

    this->decExtThreadDependency();
    return MTRUE;
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
RootNode::
dispatch(PipeRequestPtr request)
{
    ScopeLogger logger(PIPE_LOG_TAG, __FUNCTION__);

    CHECK_OBJECT(request->getImageBuffer(BID_INPUT_FSRAW_1));
    CHECK_OBJECT(request->getImageBuffer(BID_INPUT_FSRAW_2));
    CHECK_OBJECT(request->getImageBuffer(BID_INPUT_RSRAW_1));
    CHECK_OBJECT(request->getImageBuffer(BID_INPUT_RSRAW_2));
    CHECK_OBJECT(request->getMetadata(BID_META_IN_APP));
    CHECK_OBJECT(request->getMetadata(BID_META_IN_HAL));
    CHECK_OBJECT(request->getMetadata(BID_META_IN_HAL_MAIN2));

    // To PreProcessNode
    {
        sp<ImageBufInfoMap> ImgBufInfo = new ImageBufInfoMap(request);
        ImgBufInfo->addIImageBuffer(BID_INPUT_FSRAW_1, request->getImageBuffer(BID_INPUT_FSRAW_1));
        setRawDumpExtNames(ImgBufInfo, BID_INPUT_FSRAW_1);

        ImgBufInfo->addIImageBuffer(BID_INPUT_FSRAW_2, request->getImageBuffer(BID_INPUT_FSRAW_2));
        setRawDumpExtNames(ImgBufInfo, BID_INPUT_FSRAW_2);

        ImgBufInfo->addMetadata(BID_META_IN_APP, request->getMetadata(BID_META_IN_APP));
        ImgBufInfo->addMetadata(BID_META_IN_HAL, request->getMetadata(BID_META_IN_HAL));
        ImgBufInfo->addMetadata(BID_META_IN_HAL_MAIN2, request->getMetadata(BID_META_IN_HAL_MAIN2));

        handleData(ROOT_TO_PREPROCESS,  ImgBufInfo);
    }

    // To DeNoiseNode
    {
        handleData(ROOT_TO_DENOISE,     request);
    }

    // To P2AFM Node
    {
        sp<ImageBufInfoMap> ImgBufInfo = new ImageBufInfoMap(request);
#if 0
        if( request->getImageBuffer(BID_INPUT_RSRAW_1) != nullptr && request->getImageBuffer(BID_INPUT_RSRAW_2) != nullptr){
            MY_LOGD("P2AFM uses RS raws");
            ImgBufInfo->addIImageBuffer(BID_INPUT_RSRAW_1, request->getImageBuffer(BID_INPUT_RSRAW_1));
            ImgBufInfo->addIImageBuffer(BID_INPUT_RSRAW_2, request->getImageBuffer(BID_INPUT_RSRAW_2));
        }else if( request->getImageBuffer(BID_INPUT_FSRAW_1) != nullptr && request->getImageBuffer(BID_INPUT_FSRAW_2) != nullptr){
            MY_LOGD("P2AFM uses FS raws");
            ImgBufInfo->addIImageBuffer(BID_INPUT_FSRAW_1, request->getImageBuffer(BID_INPUT_FSRAW_1));
            ImgBufInfo->addIImageBuffer(BID_INPUT_FSRAW_2, request->getImageBuffer(BID_INPUT_FSRAW_2));
        }else{
            MY_LOGE("%p,%p,%p,%p, case not supported!",
                request->getImageBuffer(BID_INPUT_RSRAW_1).get(),
                request->getImageBuffer(BID_INPUT_RSRAW_2).get(),
                request->getImageBuffer(BID_INPUT_FSRAW_1).get(),
                request->getImageBuffer(BID_INPUT_FSRAW_2).get()
            );
            return MFALSE;
        }
#else
        MY_LOGD("P2AFM uses FS raws");
        ImgBufInfo->addIImageBuffer(BID_INPUT_FSRAW_1, request->getImageBuffer(BID_INPUT_FSRAW_1));
        setRawDumpExtNames(ImgBufInfo, BID_INPUT_FSRAW_1);

        ImgBufInfo->addIImageBuffer(BID_INPUT_FSRAW_2, request->getImageBuffer(BID_INPUT_FSRAW_2));
        setRawDumpExtNames(ImgBufInfo, BID_INPUT_FSRAW_2);

        ImgBufInfo->addIImageBuffer(BID_INPUT_RSRAW_1, request->getImageBuffer(BID_INPUT_RSRAW_1));
        setRawDumpExtNames(ImgBufInfo, BID_INPUT_RSRAW_1);

        ImgBufInfo->addIImageBuffer(BID_INPUT_RSRAW_2, request->getImageBuffer(BID_INPUT_RSRAW_2));
        setRawDumpExtNames(ImgBufInfo, BID_INPUT_RSRAW_2);
#endif
        ImgBufInfo->addMetadata(BID_META_IN_APP, request->getMetadata(BID_META_IN_APP));
        ImgBufInfo->addMetadata(BID_META_IN_HAL, request->getMetadata(BID_META_IN_HAL));
        ImgBufInfo->addMetadata(BID_META_IN_HAL_MAIN2, request->getMetadata(BID_META_IN_HAL_MAIN2));

        handleData(ROOT_TO_P2AFM,       ImgBufInfo);
    }

    // shutter callback
    handleData(SHUTTER,       request);
    return MTRUE;
}