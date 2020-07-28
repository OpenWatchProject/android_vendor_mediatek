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

#define LOG_TAG "ContextBuilder/ImageStreamManager"
//
#include <mtkcam/middleware/v1/LegacyPipeline/stereo/ContextBuilder/ImageStreamManager.h>
//
#include <mtkcam/utils/std/Log.h>
//
#include <mtkcam/pipeline/utils/streaminfo/ImageStreamInfo.h>
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
/******************************************************************************
 *
 ******************************************************************************/
sp<ImageStreamManager>
ImageStreamManager::
create(const image_stream_info_pool_setting* data)
{
    return new ImageStreamManager(data);
}
/******************************************************************************
 *
 ******************************************************************************/
void
ImageStreamManager::
destroy()
{
    MY_LOGD("+");
    mvImageStreamContainer.clear();
    mvImageStreamPoolTypeContainer.clear();
    sp<BufProviderMetaInfoSet> provider = nullptr;
    for(MUINT i=0;i<mvImageStreamPoolContainer.size();++i)
    {
        provider = mvImageStreamPoolContainer.valueAt(i);
        if(provider != nullptr)
        {
            if(provider->bufProvider!=nullptr)
            {
                MY_LOGD("CKH: flush %s", provider->bufProvider->queryImageStreamInfo()->getStreamName());
                provider->bufProvider->flush();
            }
        }
    }
    provider=nullptr;
    mvImageStreamPoolContainer.clear();
    mbIsDestroy = true;
}
//
ImageStreamManager::
ImageStreamManager(const image_stream_info_pool_setting* data)
{
    MY_LOGD("constr(%x)", this);

    init(data);
}
/******************************************************************************
 *
 ******************************************************************************/
ImageStreamManager::
~ImageStreamManager()
{
    MY_LOGD("%d", mbIsDestroy);
    if(!mbIsDestroy)
    {
        MY_LOGD("Auto destroy");
        destroy();
    }
    MY_LOGD("dcot(%x)", this);
}
/******************************************************************************
 *
 ******************************************************************************/
MUINT32
ImageStreamManager::
init(const image_stream_info_pool_setting* data)
{
    struct image_stream_info_pool_setting const* pImageStream_Setting = data;

    sp<IImageStreamInfo> tempMetaStreamInfo = NULL;
    ssize_t index = -1;
    while(pImageStream_Setting->streamId)
    {
        if(isRawFormat(pImageStream_Setting->imgFormat))
        {
            tempMetaStreamInfo = createRawImageStreamInfo(
                                        pImageStream_Setting->streamName,
                                        pImageStream_Setting->streamId,
                                        pImageStream_Setting->streamType,
                                        pImageStream_Setting->maxBufNum,
                                        pImageStream_Setting->minInitBufNum,
                                        pImageStream_Setting->usageForAllocator,
                                        pImageStream_Setting->imgFormat,
                                        pImageStream_Setting->imgSize,
                                        pImageStream_Setting->stride);
        }
        else
        {
            tempMetaStreamInfo = createImageStreamInfo(
                                        pImageStream_Setting->streamName,
                                        pImageStream_Setting->streamId,
                                        pImageStream_Setting->streamType,
                                        pImageStream_Setting->maxBufNum,
                                        pImageStream_Setting->minInitBufNum,
                                        pImageStream_Setting->usageForAllocator,
                                        pImageStream_Setting->imgFormat,
                                        pImageStream_Setting->imgSize,
                                        pImageStream_Setting->transform);
        }
        //
        index = mvImageStreamContainer.indexOfKey(
                                    pImageStream_Setting->streamId);
        //
        if(index<0)
        {
            mvImageStreamContainer.add(
                                    pImageStream_Setting->streamId,
                                    tempMetaStreamInfo);
            mvImageStreamPoolTypeContainer.add(
                                    pImageStream_Setting->streamId,
                                    pImageStream_Setting->bufStreamType);
            sp<BufProviderMetaInfoSet> pSet = new BufProviderMetaInfoSet;
            mvImageStreamPoolContainer.add(
                                    pImageStream_Setting->streamId,
                                    pSet);
        }
        pImageStream_Setting++;
    }
    MY_LOGD("mvImageStreamContainer.size() is %d", mvImageStreamContainer.size());
    mbIsDestroy = MFALSE;
    return mvImageStreamContainer.size();
}

/******************************************************************************
 *
 ******************************************************************************/
sp<IImageStreamInfo>
ImageStreamManager::
createImageStreamInfo(
    char const*         streamName,
    StreamId_T          streamId,
    MUINT32             streamType,
    size_t              maxBufNum,
    size_t              minInitBufNum,
    MUINT               usageForAllocator,
    MINT                imgFormat,
    MSize const&        imgSize,
    MUINT32             transform
)
{
    IImageStreamInfo::BufPlanes_t bufPlanes;
#define addBufPlane(planes, height, stride)                                      \
        do{                                                                      \
            size_t _height = (size_t)(height);                                   \
            size_t _stride = (size_t)(stride);                                   \
            IImageStreamInfo::BufPlane bufPlane= { _height * _stride, _stride }; \
            planes.push_back(bufPlane);                                          \
        }while(0)
    switch( imgFormat ) {
        case eImgFmt_YV12:
            addBufPlane(bufPlanes , imgSize.h      , imgSize.w);
            addBufPlane(bufPlanes , imgSize.h >> 1 , imgSize.w >> 1);
            addBufPlane(bufPlanes , imgSize.h >> 1 , imgSize.w >> 1);
            break;
        case eImgFmt_NV21:
            addBufPlane(bufPlanes , imgSize.h      , imgSize.w);
            addBufPlane(bufPlanes , imgSize.h >> 1 , imgSize.w);
            break;
        case eImgFmt_RAW16:
        case eImgFmt_YUY2:
        case eImgFmt_Y16:
        case eImgFmt_BAYER12_UNPAK:
            addBufPlane(bufPlanes , imgSize.h      , imgSize.w << 1);
            break;
        case eImgFmt_Y8:
            addBufPlane(bufPlanes , imgSize.h      , imgSize.w);
            break;
        case eImgFmt_STA_BYTE:
            addBufPlane(bufPlanes , imgSize.h      , imgSize.w);
            break;
        case eImgFmt_RGBA8888:
            addBufPlane(bufPlanes , imgSize.h      , imgSize.w<<2);
            break;
        case eImgFmt_BLOB:
                    /*
                    add 328448 for image size
                    standard exif: 1280 bytes
                    4 APPn for debug exif: 0xFF80*4 = 65408*4 bytes
                    max thumbnail size: 64K bytes
                    */
            addBufPlane(bufPlanes , 1              , (imgSize.w * imgSize.h * 12 / 10) + 328448); //328448 = 64K+1280+65408*4
            break;
        default:
            MY_LOGE("streamId= %#" PRIx64 " format not support yet %d", streamId, imgFormat);
            break;
    }
#undef  addBufPlane

    sp<ImageStreamInfo>
        pStreamInfo = new ImageStreamInfo(
                streamName,
                streamId,
                streamType,
                maxBufNum, minInitBufNum,
                usageForAllocator, imgFormat, imgSize, bufPlanes, transform
                );

    if( pStreamInfo == NULL ) {
        MY_LOGE("create ImageStream failed, %s, %#" PRIx64 ,
                streamName, streamId);
    }

    return pStreamInfo;
}
/******************************************************************************
 *
 ******************************************************************************/
MUINT32
ImageStreamManager::
getSize()
{
    return mvImageStreamContainer.size();
}
/******************************************************************************
 *
 ******************************************************************************/
sp<IImageStreamInfo>
ImageStreamManager::
getStreamInfoByIndex(MUINT32 index)
{
    return mvImageStreamContainer.valueAt(index);
}
/******************************************************************************
 *
 ******************************************************************************/
NSPipelineContext::eStreamType
ImageStreamManager::
getPoolTypeByIndex(MUINT32 index)
{
    return mvImageStreamPoolTypeContainer.valueAt(index);
}
/******************************************************************************
 *
 ******************************************************************************/
sp<NSCam::v1::StreamBufferProvider>
ImageStreamManager::
getBufProviderByIndex(MUINT32 index)
{
    sp<BufProviderMetaInfoSet> spSet = mvImageStreamPoolContainer.valueAt(index);
    return (spSet != NULL)?(spSet->bufProvider):(NULL);
}
/******************************************************************************
 *
 ******************************************************************************/
NSPipelineContext::eStreamType
ImageStreamManager::
getPoolTypeByStreamId(StreamId_T id)
{
    if(mvImageStreamPoolTypeContainer.size() > 0)
    {
        ssize_t keyIndex = mvImageStreamPoolTypeContainer.indexOfKey(id);
        if(keyIndex >= 0)
        {
            return mvImageStreamPoolTypeContainer.valueAt(keyIndex);
        }
        else
        {
            MY_LOGW("StreamId_T: (%llx) is not contained in mvImageStreamPoolTypeContainer.", id);
            return NSPipelineContext::eStreamType();
        }
    }
    MY_LOGW("mvImageStreamPoolTypeContainer.size() is 0.");
    return NSPipelineContext::eStreamType();
}
/******************************************************************************
 *
 ******************************************************************************/
sp<NSCam::v1::StreamBufferProvider>
ImageStreamManager::
getBufProviderByStreamId(StreamId_T id)
{
    if(mvImageStreamPoolContainer.size() > 0)
    {
        ssize_t keyIndex = mvImageStreamPoolContainer.indexOfKey(id);
        if(keyIndex >= 0)
        {
            return mvImageStreamPoolContainer.valueAt(keyIndex)->bufProvider;
        }
        else
        {
            MY_LOGW("StreamId_T: (%llx) is not contained in mvImageStreamPoolContainer.", id);
            return NULL;
        }
    }
    MY_LOGW("mvImageStreamPoolContainer.size() is 0.");
    return NULL;
}
/******************************************************************************
 *
 ******************************************************************************/
sp<IImageStreamInfo>
ImageStreamManager::
getStreamInfoByStreamId(StreamId_T id)
{
    if(mvImageStreamContainer.size() > 0)
    {
        ssize_t keyIndex = mvImageStreamContainer.indexOfKey(id);
        if(keyIndex >= 0)
        {
            return mvImageStreamContainer.valueAt(keyIndex);
        }
        else
        {
            MY_LOGW("StreamId_T: (%llx) is not contained in mvImageStreamContainer.", id);
            return NULL;
        }
    }
    MY_LOGW("mvImageStreamContainer.size() is 0.");
    return NULL;
}
/******************************************************************************
 *
 ******************************************************************************/
void
ImageStreamManager::
dump()
{
    MY_LOGD("+");
    MUINT32 size = mvImageStreamContainer.size();
    if(size > 0)
    {
        ImageStreamInfo* temp = NULL;
        for(MUINT32 i=0;i<size;++i)
        {
            MY_LOGD("StreamId (%llx) ImageStreamInfo (0x%x) StreamType (%d) BufProvide(0x%x)",
                        mvImageStreamContainer.keyAt(i), mvImageStreamContainer.valueAt(i).get(),
                        mvImageStreamPoolTypeContainer.valueAt(i),
                        mvImageStreamPoolContainer.valueAt(i)->bufProvider.get());

            MY_LOGD("Dump ImageStreamInfo Start===============");
            temp = reinterpret_cast<ImageStreamInfo*>(mvImageStreamContainer.valueAt(i).get());
            if(temp != NULL)
            {
                MY_LOGD("Name(%s) StreamId(%xllx) StreamType(%d) MaxBuf(%d) MinInitBuf(%d)",
                        temp->getStreamName(),
                        temp->getStreamId(),
                        temp->getStreamType(),
                        temp->getMaxBufNum(),
                        temp->getMinInitBufNum());
                MY_LOGD("Usage(%x) ImgFormat(%x) ImgSize(%dx%d), transform(%d)",
                        temp->getUsageForAllocator(),
                        temp->getImgFormat(),
                        temp->getImgSize().w,
                        temp->getImgSize().h,
                        temp->getTransform());
            }
            MY_LOGD("Dump MetaStreamInfo End===============");
        }
    }

    size = mvImageStreamPoolContainer.size();
    for(MUINT32 i=0; i<size; i++)
    {
        sp<BufProviderMetaInfoSet> pSet = mvImageStreamPoolContainer.valueAt(i);
        MY_LOGD("idx(%d), bufProvider(%x)==================",
                i,
                pSet->bufProvider.get());

        for(MUINT32 j=0; j<pSet->vMetaStream.size(); j++)
        {
            MY_LOGD(" - MetaStreamId(%llx)", pSet->vMetaStream[j]);
        }
    }
    MY_LOGD("-");
}
/******************************************************************************
 *
 ******************************************************************************/
MBOOL
ImageStreamManager::
isRawFormat(MINT iFmt)
{
    MBOOL ret = MFALSE;
    switch(iFmt)
    {
    case eImgFmt_BAYER8:
    case eImgFmt_BAYER10:
    case eImgFmt_BAYER12:
    case eImgFmt_BAYER14:
    //
    case eImgFmt_FG_BAYER8:
    case eImgFmt_FG_BAYER10:
    case eImgFmt_FG_BAYER12:
    case eImgFmt_FG_BAYER14:
    case eImgFmt_UFO_BAYER10:
    case eImgFmt_STA_2BYTE: // LCSO with LCE3.0
        ret = MTRUE;
    }
    //
    return ret;
}
/******************************************************************************
 *
 ******************************************************************************/
MERROR
ImageStreamManager::
updateBufProvider(
    StreamId_T id,
    sp<NSCam::v1::StreamBufferProvider> bufProvider,
    Vector<StreamId_T > const& vMetaStream
)
{
    MY_LOGD("+");

    ssize_t index = -1;
    //
    if(bufProvider == NULL) {
        MY_LOGE("Input a NULL provider");
        return BAD_VALUE;
    }
    if((index = mvImageStreamContainer.indexOfKey(id)) < 0) {
        MY_LOGW("StreamID(%llx) doest not exist in container!!", id);
        return BAD_VALUE;
    }
    if(getPoolTypeByIndex(index) != NSPipelineContext::eStreamType_IMG_HAL_PROVIDER)
    {
        MY_LOGW("StreamID(%llx) buffer-type(%d) is not HAL_PROVIDER!!", id, getPoolTypeByIndex(index));
        return BAD_VALUE;
    }
    mvImageStreamPoolContainer.valueAt(index)->bufProvider = bufProvider;
    mvImageStreamPoolContainer.valueAt(index)->vMetaStream = vMetaStream;
    MY_LOGD("-");
    return OK;
}
/******************************************************************************
 *
 ******************************************************************************/
MBOOL
ImageStreamManager::
updateStreamInfo(
    StreamId_T id,
    sp<IImageStreamInfo> pInfo
)
{
    if(pInfo == nullptr)
    {
        MY_LOGE("pInfo is NULL.");
        return MFALSE;
    }
    ssize_t index = mvImageStreamContainer.indexOfKey(id);
    if(index>=0)
    {
        sp<IImageStreamInfo> pStreamInfo = mvImageStreamContainer.valueAt(index);
        if(pStreamInfo->getStreamType()!=pInfo->getStreamType())
        {
            MY_LOGW("StreamType is unsynchronized. ori(%d) obj(%d)",
                        pStreamInfo->getStreamType(),
                        pInfo->getStreamType());
        }
        if(pStreamInfo->getUsageForAllocator()!=pInfo->getUsageForAllocator())
        {
            MY_LOGW("UsageForAllocator is unsynchronized. ori(%d) obj(%d)",
                        pStreamInfo->getUsageForAllocator(),
                        pInfo->getUsageForAllocator());
        }
        if(pStreamInfo->getImgFormat()!=pInfo->getImgFormat())
        {
            MY_LOGW("ImgFormat is unsynchronized. ori(%d) obj(%d)",
                        pStreamInfo->getImgFormat(),
                        pInfo->getImgFormat());
        }
        if(pStreamInfo->getImgSize().w!=pInfo->getImgSize().w)
        {
            MY_LOGW("ImgSize.w is unsynchronized. ori(%d) obj(%d)",
                        pStreamInfo->getImgSize().w,
                        pInfo->getImgSize().w);
        }
        if(pStreamInfo->getImgSize().h!=pInfo->getImgSize().h)
        {
            MY_LOGW("ImgSize.h is unsynchronized. ori(%d) obj(%d)",
                        pStreamInfo->getImgSize().h,
                        pInfo->getImgSize().h);
        }
        MY_LOGD("Update stream info stremaId(%llx)", id);
        mvImageStreamContainer.replaceValueFor(id, pInfo);
    }
    else
    {
        MY_LOGE("StreamId(%llx) is not exist in table.", id);
        return MFALSE;
    }
    return MTRUE;
}
/******************************************************************************
 *
 ******************************************************************************/
sp<ImageStreamManager::BufProviderMetaInfoSet>
ImageStreamManager::
getBufProviderMetaInfoSet(MUINT32 idx)
{
    return mvImageStreamPoolContainer.valueAt(idx);
}
/******************************************************************************
 *
 ******************************************************************************/
sp<IImageStreamInfo>
ImageStreamManager::
createRawImageStreamInfo(
    char const*         streamName,
    StreamId_T          streamId,
    MUINT32             streamType,
    size_t              maxBufNum,
    size_t              minInitBufNum,
    MUINT               usageForAllocator,
    MINT                imgFormat,
    MSize const&        imgSize,
    size_t const        stride
)
{
    IImageStreamInfo::BufPlanes_t bufPlanes;
    //
#define addBufPlane(planes, height, stride)                                      \
        do{                                                                      \
            size_t _height = (size_t)(height);                                   \
            size_t _stride = (size_t)(stride);                                   \
            IImageStreamInfo::BufPlane bufPlane= { _height * _stride, _stride }; \
            planes.push_back(bufPlane);                                          \
        }while(0)
    //
    switch( imgFormat ) {
        case eImgFmt_BAYER10:
        case eImgFmt_FG_BAYER10:
        case eImgFmt_BAYER8: // LCSO
        case eImgFmt_STA_2BYTE: // LCSO with LCE3.0
            addBufPlane(bufPlanes , imgSize.h, stride);
            break;
        default:
            MY_LOGE("format not support yet %p", imgFormat);
            break;
    }
#undef  addBufPlane

    sp<ImageStreamInfo>
        pStreamInfo = new ImageStreamInfo(
                streamName,
                streamId,
                streamType,
                maxBufNum, minInitBufNum,
                usageForAllocator, imgFormat, imgSize, bufPlanes
                );

    if( pStreamInfo == NULL ) {
        MY_LOGE("create ImageStream failed, %s, %#" PRIx64 ,
                streamName, streamId);
    }

    return pStreamInfo;
}

