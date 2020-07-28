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

#define LOG_TAG "MtkCam/PipeManager"

#include <mtkcam/def/common.h>
using namespace android;
using namespace NSCam;

#include <vector>
using namespace std;

#include <mtkcam/pipeline/pipeline/PipelineContext.h>
#include <v3/pipeline/PipelineContextImpl.h>
using namespace NSCam::v3::NSPipelineContext;

#include <mtkcam/pipeline/utils/streaminfo/MetaStreamInfo.h>
#include <mtkcam/pipeline/utils/streaminfo/ImageStreamInfo.h>
#include <v1/BufferProvider/StreamBufferProvider.h>
#include <mtkcam/pipeline/stream/IStreamInfo.h>
using namespace NSCam::v3;

#include <v1/BufferProvider/StreamBufferProvider.h>
using namespace NSCam::v1;

#include <mtkcam/middleware/v1/LegacyPipeline/ILegacyPipeline.h>
#include <mtkcam/middleware/v1/LegacyPipeline/LegacyPipelineManager.h>
using namespace NSCam::v1::NSLegacyPipeline;

/******************************************************************************
*
*******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_LOGV("[%d:%s] " fmt, getOpenId(), __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("[%d:%s] " fmt, getOpenId(), __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("[%d:%s] " fmt, getOpenId(), __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("[%d:%s] " fmt, getOpenId(), __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("[%d:%s] " fmt, getOpenId(), __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("[%d:%s] " fmt, getOpenId(), __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("[%d:%s] " fmt, getOpenId(), __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( (cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( (cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)
//
#define MY_LOGD1(...)               MY_LOGD_IF((mLogLevel>=1),__VA_ARGS__)
#define MY_LOGD2(...)               MY_LOGD_IF((mLogLevel>=2),__VA_ARGS__)
#define MY_LOGD3(...)               MY_LOGD_IF((mLogLevel>=3),__VA_ARGS__)
//
#define FUNC_START                  MY_LOGD1("+")
#define FUNC_END                    MY_LOGD1("-")
//
vector<wp<LegacyPipelineManager> > LegacyPipelineManager::mgrs = vector<wp<LegacyPipelineManager> >();

/******************************************************************************
*
*******************************************************************************/
sp<LegacyPipelineManager>
LegacyPipelineManager::
getInstance(MINT32 const sensorId)
{
    //check if exist
    vector<wp<LegacyPipelineManager> >::iterator mgrIter = mgrs.begin();
    while (mgrIter != mgrs.end())
    {
        sp<LegacyPipelineManager> spMgr = (*mgrIter).promote();
        if( spMgr == NULL )
        {
            mgrIter = mgrs.erase(mgrIter);
        }
        else if( spMgr->getSensorId() == sensorId )
        {
            return spMgr;
        }
        else
        {
            ++mgrIter;
        }
    }

    //create one and return
    wp<LegacyPipelineManager> mgr = new LegacyPipelineManager(sensorId);
    sp<LegacyPipelineManager> spMgr = mgr.promote();
    if( spMgr == NULL )
    {
        MY_LOGE("LegacyPipelineManager create fail.");
        return NULL;
    }
    mgrs.push_back(mgr);
    return spMgr;
}


/******************************************************************************
*
*******************************************************************************/
LegacyPipelineManager::
LegacyPipelineManager(MINT32 const sensorId)
    : mSensorId(sensorId)
    , mPipelineIdCount(0)
{
    mspvLegacyPipeline.clear();
    mspvConsumerBufferProvider.clear();
}


/******************************************************************************
*
*******************************************************************************/
LegacyPipelineManager::
~LegacyPipelineManager()
{
    vector<sp<ILegacyPipeline> >::iterator pipeIter;
    for( pipeIter = mspvLegacyPipeline.begin() ; pipeIter != mspvLegacyPipeline.end() ; pipeIter++ )
    {
        (*pipeIter)->destroyInstance();
    }
    mspvLegacyPipeline.clear();

    mspvConsumerBufferProvider.clear();
}


/******************************************************************************
*
*******************************************************************************/
MINT32
LegacyPipelineManager::
registerLegacyPipeline(sp<ILegacyPipeline> const legacyPipeline)
{
    if( legacyPipeline == NULL )
    {
        MY_LOGW("legacyPipeline is NULL, register fail.");
        return -1;
    }
    legacyPipeline->setId(mPipelineIdCount);
    mPipelineIdCount++;
    mspvLegacyPipeline.push_back(legacyPipeline);
    return legacyPipeline->getId();
}


/******************************************************************************
*
*******************************************************************************/
MERROR
LegacyPipelineManager::
registerConsumerBufferProvider(sp<StreamBufferProvider> const consumerBufferProvider)
{
    if( consumerBufferProvider == NULL )
    {
        MY_LOGW("consumerBufferProvider is NULL, register fail.");
        return BAD_VALUE;
    }
    mspvConsumerBufferProvider.push_back(consumerBufferProvider);
    return OK;
}


/******************************************************************************
*
*******************************************************************************/
MVOID
LegacyPipelineManager::
destroyLegacyPipeline(MINT32 const legacyPipelineId)
{
    vector<sp<ILegacyPipeline> >::iterator pipeIter;
    for( pipeIter = mspvLegacyPipeline.begin() ; pipeIter != mspvLegacyPipeline.end() ; pipeIter++ )
    {
        if( legacyPipelineId == (*pipeIter)->getId() )
        {
            break;
        }
    }

    if( pipeIter == mspvLegacyPipeline.end() )
    {
        MY_LOGW("legacyPipelineId:%d not found.", legacyPipelineId);
        return;
    }

    (*pipeIter)->destroyInstance();
    mspvLegacyPipeline.erase(pipeIter);
    return;
}


/******************************************************************************
*
*******************************************************************************/
wp<ILegacyPipeline>
LegacyPipelineManager::
getLegacyPipeline(MINT32 const legacyPipelineId)
{
    vector<sp<ILegacyPipeline> >::iterator pipeIter;
    for( pipeIter = mspvLegacyPipeline.begin() ; pipeIter != mspvLegacyPipeline.end() ; pipeIter++ )
    {
        if( legacyPipelineId == (*pipeIter)->getId() )
        {
            return (*pipeIter).get();
        }
    }

    MY_LOGW("legacyPipelineId:%d not found", legacyPipelineId);
    return NULL;
}


/******************************************************************************
*
*******************************************************************************/
sp<StreamBufferProvider>
LegacyPipelineManager::
getConsumerBufferProvider(StreamId_T const streamId)
{
    vector<sp<StreamBufferProvider> >::iterator providerIter;
    sp<IImageStreamInfo> imageStreamInfo;
    for( providerIter = mspvConsumerBufferProvider.begin() ; providerIter != mspvConsumerBufferProvider.end() ; providerIter++ )
    {
        (*providerIter)->queryImageStreamInfo(imageStreamInfo);
        if( streamId == imageStreamInfo->getStreamId() )
        {
            sp<StreamBufferProvider> provider = (*providerIter);
            mspvConsumerBufferProvider.erase(providerIter);
            return provider;
        }
    }

    MY_LOGW("streamId:%d not found", streamId);
    return NULL;
}


/******************************************************************************
*
*******************************************************************************/
MVOID
LegacyPipelineManager::
dump()
{
    //TBD

    return;
}
