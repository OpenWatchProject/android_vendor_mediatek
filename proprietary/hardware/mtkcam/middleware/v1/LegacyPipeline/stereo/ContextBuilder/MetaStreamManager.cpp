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

#define LOG_TAG "ContextBuilder/MetaStreamManager"
#define CONTEXT_NAME "Stereo_P2"
//
#include <mtkcam/middleware/v1/LegacyPipeline/stereo/ContextBuilder/MetaStreamManager.h>
//
#include <mtkcam/utils/std/Log.h>
//
#include <mtkcam/pipeline/utils/streaminfo/MetaStreamInfo.h>
//
#include <mtkcam/pipeline/hwnode/StreamId.h>
//
#include <utils/StrongPointer.h>
//
#include <map>
//
#include <utils/RefBase.h>
#include <utils/StrongPointer.h>
//
#include <mtkcam/middleware/v1/LegacyPipeline/stereo/ContextBuilder/ContextBuilder.h>
//
using namespace NSCam;
using namespace v3;
using namespace NSCam::v3::Utils;
using namespace android;
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
sp<MetaStreamManager>
MetaStreamManager::
create(const metadata_info_setting* data)
{
    return new MetaStreamManager(data);
}
//
void
MetaStreamManager::
destroy()
{
    MY_LOGD("+");
    mvMetadataContainer.clear();
    mbIsDestroy = true;
}
//
MetaStreamManager::
MetaStreamManager(const metadata_info_setting* data)
{
    init(data);
}
//
MetaStreamManager::
~MetaStreamManager()
{
    MY_LOGD("%d", mbIsDestroy);
    if(!mbIsDestroy)
    {
        MY_LOGD("Auto destroy");
        destroy();
    }
    MY_LOGD("dcot(%x)", this);
}
//
MUINT32
MetaStreamManager::
init(const metadata_info_setting* data)
{
    struct metadata_info_setting const* pMetadata_Setting = data;
    
    sp<IMetaStreamInfo> tempMetaStreamInfo = NULL;
    ssize_t index = -1;
    while(pMetadata_Setting->streamId)
    {
        tempMetaStreamInfo = new MetaStreamInfo(
                                    pMetadata_Setting->name,
                                    pMetadata_Setting->streamId,
                                    pMetadata_Setting->streamType,
                                    pMetadata_Setting->maxBufNum,
                                    pMetadata_Setting->minInitButNum);
        //
        index = mvMetadataContainer.indexOfKey(
                                    pMetadata_Setting->streamId);
        //
        if(index<0)
        {
            mvMetadataContainer.add(
                                    pMetadata_Setting->streamId,
                                    tempMetaStreamInfo);
            mvMetadataPoolContainer.add(pMetadata_Setting->streamId,
                                        pMetadata_Setting->streamPoolType);
        }
        pMetadata_Setting++;
    }
    MY_LOGD("mvMetadataContainer.size() is %d", mvMetadataContainer.size());
    mbIsDestroy = MFALSE;
    return mvMetadataContainer.size();
}
//
MUINT32
MetaStreamManager::
getSize()
{
    return mvMetadataContainer.size();
}
//
sp<IMetaStreamInfo>
MetaStreamManager::
getStreamInfoByIndex(MUINT32 index)
{
    return mvMetadataContainer.valueAt(index);
}
//
NSPipelineContext::eStreamType
MetaStreamManager::
getPoolTypeByIndex(MUINT32 index)
{
    return mvMetadataPoolContainer.valueAt(index);
}
//
sp<IMetaStreamInfo>
MetaStreamManager::
getStreamInfoByStreamId(StreamId_T id)
{
    if(mvMetadataContainer.size() > 0)
    {
        ssize_t keyIndex = mvMetadataContainer.indexOfKey(id);
        if(keyIndex >= 0)
        {
            return mvMetadataContainer.valueAt(keyIndex);
        }
        else
        {
            MY_LOGW("StreamId_T: (%llx) is not contained in mvMetadataContainer.", id);
            return NULL;
        }
    }
    MY_LOGW("mvMetadataContainer.size() is 0.");
    return NULL;
}
//
NSPipelineContext::eStreamType
MetaStreamManager::
getPoolTypeByStreamId(StreamId_T id)
{
    if(mvMetadataPoolContainer.size() > 0)
    {
        ssize_t keyIndex = mvMetadataPoolContainer.indexOfKey(id);
        if(keyIndex >= 0)
        {
            return mvMetadataPoolContainer.valueAt(keyIndex);
        }
        else
        {
            MY_LOGW("StreamId_T: (%llx) is not contained in mvMetadataPoolContainer.", id);
            return NSPipelineContext::eStreamType();
        }
    }
    MY_LOGW("mvMetadataPoolContainer.size() is 0.");
    return NSPipelineContext::eStreamType();
}
//
void
MetaStreamManager::
dump()
{
    MUINT32 size = mvMetadataContainer.size();
    if(size > 0)
    {
        MetaStreamInfo* temp = NULL;
        for(MUINT32 i=0;i<size;++i)
        {
            MY_LOGD("StreamId (%llx) MetaStreamInfo (0x%x) StreamType (%d)",
            mvMetadataContainer.keyAt(i), mvMetadataContainer.valueAt(i).get(),
            mvMetadataPoolContainer.valueAt(i));
            
            MY_LOGD("Dump MetaStreamInfo Start===============");
            temp = reinterpret_cast<MetaStreamInfo*>(mvMetadataContainer.valueAt(i).get());
            if(temp != NULL)
            {
                MY_LOGD("Name(%s) StreamId(%llx) StreamType(%d) MaxBuf(%d) MinInitBuf(%d)",
                        temp->getStreamName(),
                        temp->getStreamId(),
                        temp->getStreamType(),
                        temp->getMaxBufNum(),
                        temp->getMinInitBufNum());
            }
            MY_LOGD("Dump MetaStreamInfo End===============");
        }
    }
}