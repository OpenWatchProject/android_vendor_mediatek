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
#include "RootHRNode.h"

#define PIPE_MODULE_TAG "MFHR"
#define PIPE_CLASS_TAG "RootHRNode"
#define PIPE_LOG_TAG PIPE_MODULE_TAG PIPE_CLASS_TAG

#include <PipeLog.h>

#define CHECK_OBJECT(x)  do{                                            \
    if (x == nullptr) { MY_LOGE("Null %s Object", #x); return -MFALSE;} \
} while(0)

using namespace NSCam::NSCamFeature::NSFeaturePipe;
/*******************************************************************************
 *
 ********************************************************************************/
RootHRNode::
RootHRNode(const char *name,
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
RootHRNode::
~RootHRNode()
{
    MY_LOGD("dctor(0x%x)", this);
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
RootHRNode::
onData(
    DataID id,
    PipeRequestPtr &request)
{
    FUNC_START;
    TRACE_FUNC_ENTER();

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

    TRACE_FUNC_EXIT();
    FUNC_END;
    return ret;
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
RootHRNode::
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
RootHRNode::
onUninit()
{
    ScopeLogger logger(PIPE_LOG_TAG, __FUNCTION__);
    return MTRUE;
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
RootHRNode::
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
RootHRNode::
onThreadStop()
{
    return MTRUE;
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
RootHRNode::
onThreadLoop()
{
    FUNC_START;
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

    CAM_TRACE_NAME("RootHRNode::onThreadLoop");

    if(!dispatch(pipeRequest)){
        MY_LOGE("failed to dispatch, please check above errors!");
    }

    FUNC_END;
    return MTRUE;
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
RootHRNode::
isFrameSync(PipeRequestPtr request)
{
    MBOOL ret = MFALSE;
    ScopeLogger logger(PIPE_LOG_TAG, __FUNCTION__);
    AutoProfileUtil proflie(PIPE_LOG_TAG, __func__);

    IMetadata* pMetadata_main1 = request->getMetadata(BID_META_IN_HAL);
    IMetadata* pMetadata_main2 = request->getMetadata(BID_META_IN_HAL_MAIN2);

    if(pMetadata_main1 == nullptr){
        MY_LOGE("cant get pMetadata_main1!");
        return MTRUE;
    }

    if(pMetadata_main2 == nullptr){
        MY_LOGE("cant get pMetadata_main2!");
        return MTRUE;
    }

    MINT64 timestamp_main1 = -1;
    MINT64 timestamp_main2 = -1;

MY_LOGD("temp always return true"); // since MTK_SENSOR_TIMESTAMP is not saved in p1 hal meta, we can't get it right now
return MTRUE;

    IMetadata::IEntry entry;

    MY_LOGD("main1");
    entry = pMetadata_main1->entryFor(MTK_SENSOR_TIMESTAMP);
    if (!entry.isEmpty())
            timestamp_main1 = entry.itemAt(0, Type2Type<MINT64>());

    MY_LOGD("main2");
    entry = pMetadata_main2->entryFor(MTK_SENSOR_TIMESTAMP);
    if (!entry.isEmpty())
            timestamp_main2 = entry.itemAt(0, Type2Type<MINT64>());

    if(timestamp_main1 == -1 || timestamp_main2 == -1){
        MY_LOGE("cant get timesamp %d/%d! return true", timestamp_main1, timestamp_main2);
        return MTRUE;
    }

    int timestamp_main1_ms = timestamp_main1/1000000;
    int timestamp_main2_ms = timestamp_main2/1000000;
    int timestamp_diff     = timestamp_main1_ms - timestamp_main2_ms;

    if(abs(timestamp_diff) <= SYNC_THRESHOLD_MS){
        ret = MTRUE;
    }else{
        ret = MFALSE;
    }
    MY_LOGD("%d-%d, diff:%d threshold:%d, ret:%d", timestamp_main1_ms, timestamp_main2_ms, timestamp_diff, SYNC_THRESHOLD_MS, ret);
    return ret;
}
/*******************************************************************************
 *
 ********************************************************************************/
MVOID
RootHRNode::
markFrameSync(PipeRequestPtr request, MBOOL syncOK)
{
    if(syncOK){
        request->addParam(BMDeNoiseParamID::PID_SYNC_FAILED, 1);
    }
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
RootHRNode::
dispatch(PipeRequestPtr request)
{
    ScopeLogger logger(PIPE_LOG_TAG, __FUNCTION__);

    MY_LOGD("reqNo:%d", request->getRequestNo());

    markFrameSync(
        request,
        isFrameSync(request)
    );

    CHECK_OBJECT(request->getImageBuffer(BID_INPUT_FSRAW_1));
    CHECK_OBJECT(request->getImageBuffer(BID_INPUT_FSRAW_2));
    CHECK_OBJECT(request->getImageBuffer(BID_INPUT_RSRAW_1));
    CHECK_OBJECT(request->getImageBuffer(BID_INPUT_RSRAW_2));

    // To BayerPreProcessNode
    {
        sp<ImageBufInfoMap> ImgBufInfo = new ImageBufInfoMap(request);
        ImgBufInfo->addIImageBuffer(BID_INPUT_FSRAW_1, request->getImageBuffer(BID_INPUT_FSRAW_1));
        setRawDumpExtNames(ImgBufInfo, BID_INPUT_FSRAW_1);
        ImgBufInfo->addIImageBuffer(BID_INPUT_FSRAW_2, request->getImageBuffer(BID_INPUT_FSRAW_2));
        setRawDumpExtNames(ImgBufInfo, BID_INPUT_FSRAW_2);
        ImgBufInfo->addMetadata(BID_META_IN_APP, request->getMetadata(BID_META_IN_APP));
        ImgBufInfo->addMetadata(BID_META_IN_HAL, request->getMetadata(BID_META_IN_HAL));
        ImgBufInfo->addMetadata(BID_META_IN_HAL_MAIN2, request->getMetadata(BID_META_IN_HAL_MAIN2));

        handleData(ROOT_HR_TO_BAYER_PREPROCESS,  ImgBufInfo);
    }

    // To Fusion Node
    {
        handleData(ROOT_HR_TO_FUSION,     request);
    }

    // To BSS Node
    {
        sp<ImageBufInfoMap> ImgBufInfo = new ImageBufInfoMap(request);

        ImgBufInfo->addIImageBuffer(BID_INPUT_RSRAW_1, request->getImageBuffer(BID_INPUT_RSRAW_1));
        setRawDumpExtNames(ImgBufInfo, BID_INPUT_RSRAW_1);

        ImgBufInfo->addIImageBuffer(BID_INPUT_RSRAW_2, request->getImageBuffer(BID_INPUT_RSRAW_2));
        setRawDumpExtNames(ImgBufInfo, BID_INPUT_RSRAW_2);

        ImgBufInfo->addIImageBuffer(BID_INPUT_FSRAW_1, request->getImageBuffer(BID_INPUT_FSRAW_1));
        setRawDumpExtNames(ImgBufInfo, BID_INPUT_FSRAW_1);

        ImgBufInfo->addIImageBuffer(BID_INPUT_FSRAW_2, request->getImageBuffer(BID_INPUT_FSRAW_2));
        setRawDumpExtNames(ImgBufInfo, BID_INPUT_FSRAW_2);

        ImgBufInfo->addMetadata(BID_META_IN_APP, request->getMetadata(BID_META_IN_APP));
        ImgBufInfo->addMetadata(BID_META_IN_HAL, request->getMetadata(BID_META_IN_HAL));
        ImgBufInfo->addMetadata(BID_META_IN_HAL_MAIN2, request->getMetadata(BID_META_IN_HAL_MAIN2));

        handleData(ROOT_HR_TO_BSS,  ImgBufInfo);
    }
    return MTRUE;
}