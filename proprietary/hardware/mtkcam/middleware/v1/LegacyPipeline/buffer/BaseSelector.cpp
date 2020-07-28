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

#define LOG_TAG "MtkCam/BaseSelector"
//
#include "MyUtils.h"
//
#include <utils/RWLock.h>
//
#include <mtkcam/utils/imgbuf/ImageBufferHeap.h>
#include <mtkcam/pipeline/utils/streaminfo/ImageStreamInfo.h>
#include <mtkcam/pipeline/stream/IStreamInfo.h>
//
#include <mtkcam/middleware/v1/LegacyPipeline/buffer/StreamBufferProvider.h>
//
#include <mtkcam/middleware/v1/LegacyPipeline/StreamId.h>


using namespace android;
using namespace NSCam;
using namespace NSCam::v1;
using namespace NSCam::Utils;

using namespace NSCam::v3::Utils;
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
#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( (cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( (cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)

#if 1
#define FUNC_START     MY_LOGD("+")
#define FUNC_END       MY_LOGD("-")
#else
#define FUNC_START
#define FUNC_END
#endif

/******************************************************************************
 *
 ******************************************************************************/
BaseSelector::
BaseSelector()
{
    mLogLevel = property_get_int32("debug.camera.log", 0);
    if ( mLogLevel == 0 ) {
        mLogLevel = property_get_int32("debug.camera.log.selector", 0);
    }
}

/******************************************************************************
 *
 ******************************************************************************/
android::status_t
BaseSelector::
setPool(sp<IImageStreamInfo> streamInfo, android::wp<IConsumerPool> rpPool)
{
    const StreamId_T rStreamId = streamInfo->getStreamId();
    if(mPoolItemMap.indexOfKey(rStreamId) >= 0)
        mPoolItemMap.removeItem(rStreamId);
    mPoolItemMap.add(rStreamId, PoolItemSet{streamInfo, rpPool});
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
android::status_t
BaseSelector::
copyPoolsToSelector(android::sp<ISelector> targetSelector)
{
    for( size_t i = 0; i < mPoolItemMap.size() ; i++)
    {
        targetSelector->setPool(mPoolItemMap.valueAt(i).streamInfo, mPoolItemMap.valueAt(i).rpPool);
    }
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
android::status_t
BaseSelector::
getResultCount(MUINT32& rCount)
{
    return INVALID_OPERATION;
}

/******************************************************************************
 *
 ******************************************************************************/
android::status_t
BaseSelector::
transferResults(android::sp<ISelector> target)
{
    MUINT32 maxCount;

    if (getResultCount(maxCount) != OK) {
        MY_LOGE("Get result count failed.");
        return INVALID_OPERATION;
    }

    for (; maxCount > 0; maxCount--){

        MUINT32 curCount;

        if(getResultCount(curCount) == OK && curCount > 0){
            MINT32 reqno;
            Vector<MetaItemSet> metas;
            Vector<BufferItemSet> buffers;

            if (getResult(reqno, metas, buffers) != OK) {
                MY_LOGE("Get a result failed");
                continue;
            } else if (target->selectResult(reqno, metas, buffers, MFALSE) != OK) {
                MY_LOGE("Fail to transfer a result to target.");
            }
        }else{
            break;
        }
    }

    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
android::status_t
BaseSelector::
queryCollectImageStreamInfo(Vector<android::sp<IImageStreamInfo>>& resultStreamInfo, MBOOL isFull)
{
    for( size_t i = 0; i < mPoolItemMap.size() ; i++)
    {
        // rrzo ONLY be for stereo
        if (isFull == MFALSE && mPoolItemMap.valueAt(i).streamInfo->getStreamId() == eSTREAMID_IMAGE_PIPE_RAW_RESIZER)
            continue;

        resultStreamInfo.push_back(mPoolItemMap.valueAt(i).streamInfo);
    }
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
ISelector::SelectorType
BaseSelector::
getSelectorType() const
{
    return SelectorType_Unknown;
}

/******************************************************************************
 *
 ******************************************************************************/
status_t
BaseSelector::
returnBufferToPool(
    BufferItemSet&   rBuffer
)
{
    ssize_t index = mPoolItemMap.indexOfKey(rBuffer.id);
    if (index < 0)
    {
        MY_LOGE("Cannot get consumer pool from poolMap to return buffer.");
        return UNKNOWN_ERROR;
    }

    sp<IConsumerPool> pPool = mPoolItemMap[index].rpPool.promote();
    if ( pPool == 0 )
    {
        MY_LOGE("Cannot promote consumer pool to return buffer.");
        return UNKNOWN_ERROR;
    }

    MY_LOGD_IF( 2, "return buf:%p, id(0x%" PRIx64 ")", rBuffer.heap.get(), rBuffer.id);
    if ( pPool->returnBuffer( rBuffer.heap ) != OK)
    {
        MY_LOGE("Cannot return buffer to cunsumer pool.");
        return UNKNOWN_ERROR;
    }
    return NO_ERROR;
}

/******************************************************************************
*
******************************************************************************/
android::status_t
BaseSelector::
returnBuffersToPool(Vector<BufferItemSet>& rvBuffers)
{
    //
    MBOOL success = MTRUE;
    for (size_t i = 0 ; i < rvBuffers.size(); i++)
    {
        if ( returnBufferToPool(rvBuffers.editItemAt(i)) != NO_ERROR)
            success = MFALSE;
    }

    rvBuffers.clear();

    if( success )
        return NO_ERROR;
    else
        return UNKNOWN_ERROR;
}

