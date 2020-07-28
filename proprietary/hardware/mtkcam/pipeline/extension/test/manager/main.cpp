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
#define LOG_TAG "vmgr_test"
//
#include <cutils/properties.h>
//
#include <stdlib.h>
#include <utils/Errors.h>
#include <utils/List.h>
#include <utils/RefBase.h>
//
#include <mtkcam/utils/std/Log.h>
#include <mtkcam/utils/std/Trace.h>
#include <mtkcam/utils/std/common.h>
#include <mtkcam/utils/metadata/client/mtk_metadata_tag.h>
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>
//
#include <mtkcam/pipeline/pipeline/IPipelineDAG.h>
#include <mtkcam/pipeline/pipeline/IPipelineNode.h>

#include <mtkcam/pipeline/pipeline/PipelineContext.h>
//
#include <mtkcam/pipeline/utils/streambuf/StreamBufferPool.h>
#include <mtkcam/pipeline/utils/streambuf/StreamBuffers.h>
#include <mtkcam/pipeline/utils/streaminfo/MetaStreamInfo.h>
#include <mtkcam/pipeline/utils/streaminfo/ImageStreamInfo.h>
//
#include <mtkcam/utils/imgbuf/IIonImageBufferHeap.h>
//
#include <mtkcam/drv/IHalSensor.h>
#include <mtkcam/drv/iopipe/CamIO/INormalPipe.h>
//
#include <mtkcam/pipeline/hwnode/NodeId.h>
#include <mtkcam/pipeline/hwnode/P1Node.h>
#include <mtkcam/pipeline/hwnode/P2Node.h>
#include <mtkcam/pipeline/hwnode/JpegNode.h>
//
#include <mtkcam/utils/metastore/ITemplateRequest.h>
#include <mtkcam/utils/metastore/IMetadataProvider.h>
#include <hardware/camera3.h> // for template
#include <mtkcam/utils/metadata/IMetadataTagSet.h>
#include <mtkcam/utils/metadata/IMetadataConverter.h>
//
#include <mtkcam/pipeline/extension/IVendorManager.h>

#include <mtkcam/middleware/v1/LegacyPipeline/buffer/BufferPoolImp.h>
#include <v1/adapter/utils/ExifJpeg/ExifJpegUtils.h>

using namespace NSCam;
using namespace v3;
using namespace NSCam::v3::Utils;
using namespace android;
using namespace NSCam::plugin;
using namespace NSCam::v3::NSPipelineContext;
using namespace NSCam::NSIoPipe;
using namespace NSCam::NSIoPipe::NSCamIOPipe;

/******************************************************************************
 *
 ******************************************************************************/
#if 0
#define MY_LOGV(fmt, arg...)        CAM_LOGV("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#else
#define MY_LOGV(fmt, arg...)        CAM_LOGV("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg);\
                                    printf("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg);
#define MY_LOGD(fmt, arg...)        CAM_LOGD("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg);\
                                    printf("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg);
#define MY_LOGI(fmt, arg...)        CAM_LOGI("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg);\
                                    printf("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg);
#define MY_LOGW(fmt, arg...)        CAM_LOGW("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg);\
                                    printf("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg);
#define MY_LOGE(fmt, arg...)        CAM_LOGE("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg);\
                                    printf("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg);
#endif
//
#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
//
#define TEST(cond, result)          do { if ( (cond) == (result) ) { printf("Pass\n"); } else { printf("Failed\n"); } }while(0)
#define FUNCTION_IN     MY_LOGD_IF(1, "+");


typedef enum
{
  eTEST_CASE_P1,
  eTEST_CASE_P1_P2,
  eTEST_CASE_P1_P2_WORK,
  eTEST_CASE_MAX,
} eTestCase ;

static
sp<ImageStreamInfo> createRawImageStreamInfo(
        char const*         streamName,
        StreamId_T          streamId,
        MUINT32             streamType,
        size_t              maxBufNum,
        size_t              minInitBufNum,
        MUINT               usageForAllocator,
        MINT                imgFormat,
        MSize const&        imgSize,
        size_t const        stride
        );

static
sp<ImageStreamInfo> createImageStreamInfo(
        char const*         streamName,
        StreamId_T          streamId,
        MUINT32             streamType,
        size_t              maxBufNum,
        size_t              minInitBufNum,
        MUINT               usageForAllocator,
        MINT                imgFormat,
        MSize const&        imgSize,
        MUINT32             transform = 0
        );


unsigned int testCase = 0;    // set for number of test case
int testRequestCnt    = 4;    // set for request count
int testDumpImage     = 0;    // dump raw,yuv,jpeg to files (raw or yuv: /sdcard/camera_dump/, jpeg: /data/)
//
int testRequestCounter = 0;
int deviceId           = 0;

/******************************************************************************
 *
 ******************************************************************************/

namespace {
    enum BUFFER_CASE
    {
        CASE_WORK_IN_OUT,
        CASE_DST_OUT,
        CASE_SRC_IN,
        CASE_SRC_IN_DST_OUT,
    };

    enum BUFFER_TYPE
    {
        BUFFER_TYPE_IN,
        BUFFER_TYPE_OUT,
        BUFFER_TYPE_NONE
    };

    enum STREAM_ID{
        STREAM_ID_RAW1 = 1,     // resized raw
        STREAM_ID_RAW2,         // full raw
        STREAM_ID_YUV1,         // P2 out, display
        STREAM_ID_YUV_MAIN,     // P2 out, YUV main
        STREAM_ID_YUV_THUMBNAIL,// P2 out, YUV thumbnail
        STREAM_ID_JPEG,         // Jpeg Out
        //
        STREAM_ID_METADATA_CONTROL_APP,
        STREAM_ID_METADATA_CONTROL_HAL,
        STREAM_ID_METADATA_RESULT_P1_APP,
        STREAM_ID_METADATA_RESULT_P1_HAL,
        STREAM_ID_METADATA_RESULT_P2_APP,
        STREAM_ID_METADATA_RESULT_P2_HAL,
        STREAM_ID_METADATA_RESULT_JPEG_APP,
        //STREAM_ID_APPMETADATA2,
        //STREAM_ID_HALMETADATA1
    };

    class PluginProcessor;
    class Pool;
    IHalSensor* mpSensorHalObj = NULL;
    //
    static MUINT32 requestTemplate = CAMERA3_TEMPLATE_PREVIEW;
    //
    P1Node::SensorParams        gSensorParam;
    P1Node::ConfigParams        gP1ConfigParam;
    P2Node::ConfigParams        gP2ConfigParam;
    //
    MSize                       gRrzoSize;
    const MINT                  gRrzoFormat = eImgFmt_FG_BAYER10;
    size_t                      gRrzoStride;
    //
    MSize                       gImgoSize;
    const MINT                  gImgoFormat = eImgFmt_BAYER10;
    size_t                      gImgoStride;
    //
    android::sp<PipelineContext> gContext;
    //
    sp<IMetaStreamInfo>         gControlMeta_App;
    sp<IMetaStreamInfo>         gControlMeta_Hal;
    sp<IMetaStreamInfo>         gResultMeta_P1_App;
    sp<IMetaStreamInfo>         gResultMeta_P1_Hal;
    sp<IMetaStreamInfo>         gResultMeta_P2_App;
    sp<IMetaStreamInfo>         gResultMeta_P2_Hal;
    sp<IMetaStreamInfo>         gResultMeta_Jpeg_App;
    //
    sp<IImageStreamInfo>        gImage_RrzoRaw;
    sp<IImageStreamInfo>        gImage_ImgoRaw;
    sp<IImageStreamInfo>        gImage_Yuv;
    sp<IImageStreamInfo>        gImage_YuvJpeg;
    sp<IImageStreamInfo>        gImage_YuvWork;
    sp<IImageStreamInfo>        gImage_YuvThumbnail;
    sp<IImageStreamInfo>        gImage_Jpeg;
    //
    sp<IImageBufferHeap>        gFakeHeap;
    sp<PluginProcessor>         gProcessor;
    //
    // requestBuilder
    sp<RequestBuilder>          gRequestBuilderCap;
    sp<RequestBuilder>          gRequestBuilderP1;
    sp<RequestBuilder>          gRequestBuilderPrv;
    sp<RequestBuilder>          gRequestBuilderPrvWork;
    //
    sp<Pool>                    gJpegPool;

        //
    class DataCallback : public IVendorManager::IDataCallback
    {
    public:
                            DataCallback() {};
                            ~DataCallback() {};
        status_t            onDataReceived(
                                MBOOL   const        isLastCb,
                                MUINT32 const        requestNo,
                                MetaSet              /*result*/,
                                Vector< BufferItem > buffers
                            )
                            {
                                String8 str = String8::format("receive last:%d req:%d ", isLastCb, requestNo);
                                for (size_t i = 0; i < buffers.size(); ++i)
                                {
                                    str += String8::format("[%zu]stream:%ld ", i, buffers[i].streamInfo->getStreamId());
                                }
                                MY_LOGD("%s", str.string());
                                return OK;
                            }
    };

    class Pool : public RefBase
    {
    public:
        Pool( sp<IImageStreamInfo> info)
            : mInfo(info)
        {
            pPool = new NSCam::v1::BufferPoolImp(info);
            pPool->allocateBuffer(LOG_TAG, info->getMaxBufNum(), info->getMinInitBufNum());
        }
        Pool( sp<IVendorManager> mgr )
            : mManager(mgr)
        {}
        ~Pool() {};
    public:
        sp<IImageBufferHeap> acquire(MetaItem item)
        {
            if ( mManager.get() ) {
                mSetting = item;
                mManager->acquireWorkingBuffer(0, mSetting , mWorkBuf);
                return mWorkBuf.heap;
            } else {
                sp<IImageBufferHeap> heap;
                MUINT32 transform;
                pPool->acquireFromPool(LOG_TAG, 0, heap, transform);
                return heap;
            }
        }
        sp<IImageBufferHeap> acquire()
        {
            if ( mManager.get() ) {
                MY_LOGE("must provide meta itme.");
                return nullptr;
            }
            sp<IImageBufferHeap> heap;
            MUINT32 transform;
            pPool->acquireFromPool(LOG_TAG, 0, heap, transform);
            return heap;
        }
        void release(sp<IImageBufferHeap> heap ){
            if ( mManager.get() ) {
                mManager->releaseWorkingBuffer(0, mSetting , mWorkBuf);
            } else {
                pPool->releaseToPool(LOG_TAG, 0, heap, 0, 0);
            }
        }

    private:
        sp<NSCam::v1::BufferPoolImp> pPool;
        sp<IImageStreamInfo>         mInfo;
        //
        sp<IVendorManager>           mManager;
        BufferItem                   mWorkBuf;
        MetaItem                     mSetting;
    };

    class RHandler
        : public IVendorManager::IDataCallback
    {
    public:
        RHandler( MINT32 reqNo, sp<IVendorManager> mgr )
            : mRequestNo(reqNo)
            , mDone(false)
            , mManager(mgr)
        {}
        ~RHandler() {};
    public:
        void addPool( sp<IImageStreamInfo> info, sp<Pool> pPool, MINT32 type)
        {
            MINT32 streamId = info->getStreamId();
            switch(type) {
                case BUFFER_TYPE::BUFFER_TYPE_IN: {
                    MY_LOGD("add in:%d\n", streamId);
                    mvIn.add(streamId, info);
                } break;
                case BUFFER_TYPE::BUFFER_TYPE_OUT: {
                    MY_LOGD("add out:%d\n", streamId);
                    mvOut.add(streamId, info);
                    MetaItem meta;
                    BufferItem item;
                    item.bufferStatus = BUFFER_OUT;
                    item.streamInfo   = info;
                    item.heap         = pPool->acquire(meta);
                    mSrcBuffer.push_back(item);
                } break;
                default:{
                    MY_LOGD("add other:%d\n", streamId);
                    // mvIn.add(streamId, info);
                }break;
            };
            mPoolContainer.add(streamId, pPool);
        }
        void addMeta(MINT32 streamId, IMetadata* meta)
        {
            if ( streamId == STREAM_ID_METADATA_CONTROL_APP)
                mSetting.setting.appMeta = *meta;
            else if ( streamId == STREAM_ID_METADATA_RESULT_P1_HAL)
                mSetting.setting.halMeta = *meta;
            else if ( streamId == STREAM_ID_METADATA_RESULT_P1_APP)
                mSetting.others.push_back(*meta);
        }
        void execute() {
                if ( mManager->queue(mRequestNo, this, mSrcBuffer, mSetting) != OK ) {
                    MY_LOGE("%d queue fail.\n", mRequestNo);
                    for (size_t i = 0; i < mSrcBuffer.size(); ++i) {
                        sp<Pool> pool = mPoolContainer.valueFor(mSrcBuffer[i].streamInfo->getStreamId());
                        pool->release(mSrcBuffer[i].heap);
                    }
                    mDone = true;
                    mCond.signal();
                }
                mSrcBuffer.clear();
            }
        void wait(){
            Mutex::Autolock _l(mLock);
            if ( mDone ) return;
            MERROR err = mCond.wait(mLock);
        }

    public:     //// IDataCallback interface
        status_t    onDataReceived(
                        MBOOL   const        isLastCb,
                        MUINT32 const        requestNo,
                        MetaSet              /*result*/,
                        Vector< BufferItem > buffers
                    )
                    {
                        Mutex::Autolock _l(mLock);
                        String8 str = String8::format("receive last:%d req:%d ", isLastCb, requestNo);
                        for (size_t i = 0; i < buffers.size(); ++i)
                        {
                            str += String8::format("[%zu]stream:%ld ", i, buffers[i].streamInfo->getStreamId());
                        }
                        MY_LOGD("RHandler %s \n", str.string());
                        //
                        for (size_t i = 0; i < buffers.size(); ++i)
                        {
                            if ( mPoolContainer.indexOfKey(buffers[i].streamInfo->getStreamId()) >= 0 ) {
                                // if ( buffers[i].streamInfo->getStreamId() == STREAM_ID_YUV_MAIN)
                                //     saveJpeg(buffers[i].heap);
                                sp<Pool> pool = mPoolContainer.valueFor(buffers[i].streamInfo->getStreamId());
                                pool->release(buffers[i].heap);
                            } else {
                                // saveJpeg(buffers[i].heap);
                                mManager->releaseWorkingBuffer(0, mSetting , buffers.editItemAt(i));
                            }
                        }
                        buffers.clear();

                        if( isLastCb ) {
                            mDone = true;
                            mCond.signal();
                        }
                        return OK;
                    }
        sp<IImageBufferHeap>        acquireHeap(
                                            sp<IImageStreamInfo> const pStreamInfo
                                        ){
                                            sp<Pool> pool = mPoolContainer.valueFor(pStreamInfo->getStreamId());
                                            sp<IImageBufferHeap> heap = pool->acquire(mSetting);
                                            mHeapContainer.add(pStreamInfo->getStreamId(), heap);
                                            //
                                            return heap;
                                        }

        MERROR                      releaseHeap(
                                            sp<IImageStreamInfo> const pStreamInfo,
                                            sp<IImageBufferHeap> heap
                                        ){
                                            BufferItem item;
                                            if ( mvIn.indexOfKey(pStreamInfo->getStreamId()) >= 0) {
                                                item.bufferStatus = BUFFER_IN;
                                                item.streamInfo   = mvIn.valueFor(pStreamInfo->getStreamId());
                                            }
                                            else if ( mvOut.indexOfKey(pStreamInfo->getStreamId()) >= 0) {
                                                item.bufferStatus = BUFFER_OUT;
                                                item.streamInfo   = mvOut.valueFor(pStreamInfo->getStreamId());
                                            } else {
                                                sp<Pool> pool = mPoolContainer.valueFor(pStreamInfo->getStreamId());
                                                pool->release(heap);
                                                MY_LOGD("no need for plugin stream:0x%lx", pStreamInfo->getStreamId());
                                                return OK;
                                            }
                                            item.heap         = mHeapContainer.valueFor(pStreamInfo->getStreamId());
                                            mSrcBuffer.push_back(item);
                                            return OK;
                                        }

        void                        saveJpeg( sp<IImageBufferHeap> mainHeap) {
                                        Mutex::Autolock _l(mJpegLock);
                                        // save jpeg
                                        IMetadata temp;
                                        sp<IImageBufferHeap>     jpegHeap = gJpegPool->acquire();
                                        sp<IImageBuffer>         jpegMain = mainHeap->createImageBuffer();
                                        sp<NSCam::Utils::ExifJpegUtils> pExJpg = NSCam::Utils::ExifJpegUtils::createInstance(
                                                    deviceId, temp, temp,
                                                    /*dst*/jpegHeap,
                                                    /*src*/jpegMain, nullptr
                                                );
                                        sp<IImageBuffer> pImageBuffer = jpegHeap->createImageBuffer();

                                        if (pImageBuffer == NULL) {
                                            MY_LOGE("pImageBuffer == NULL\n");
                                            return;
                                        }
                                        pImageBuffer->lockBuf(LOG_TAG, eBUFFER_USAGE_SW_MASK);
                                        pImageBuffer->saveToFile("/data/jpegtest.jpg");
                                        pImageBuffer->unlockBuf(LOG_TAG);
                                        gJpegPool->release(jpegHeap);
                                        MY_LOGD("Jpeg saved.");
                                    }
    private:
        MINT32               mRequestNo;
        MBOOL                mDone;
        sp<IVendorManager>   mManager;
        MetaItem             mSetting;
        Vector< BufferItem > mSrcBuffer;
        KeyedVector<MINT32, sp<Pool> >             mPoolContainer;
        KeyedVector<MINT32, sp<IImageStreamInfo> > mvIn;
        KeyedVector<MINT32, sp<IImageStreamInfo> > mvOut;
        KeyedVector<MINT32, sp<IImageBufferHeap> > mHeapContainer;
        //
        Mutex                           mLock;
        Condition                       mCond;
        //
        Mutex                           mJpegLock;
    };

    class PluginProcessor
        : public NSCam::v3::IPipelineBufferSetFrameControl::IAppCallback
        , public IStreamBufferProvider
    {
    public:
        PluginProcessor() {};
        ~PluginProcessor() {};

    public:
        void registerReq(MINT32 requestNo, sp<RHandler> handler)
                { mvRequest.add(requestNo, handler); }
        void clear() {
            mvRequest.clear();
            mMap.clear();
        }
        void waitUntilDrained()
        {
            MY_LOGD("+");
            for (size_t i = 0; i < mvRequest.size(); ++i)
            {
                mvRequest.valueAt(i)->wait();
            }
            MY_LOGD("-");
        }

    public:     //// IAppCallback interface
        virtual MVOID           updateFrame(
                                    MUINT32 const requestNo,
                                    MINTPTR const /*userId*/,
                                    IPipelineBufferSetFrameControl::IAppCallback::Result const& result
                                ) {
                                    for(size_t i = 0; i < result.vAppOutMeta.size(); ++i ) {
                                        IMetadata* res = result.vAppOutMeta[i]->tryReadLock(LOG_TAG);
                                        MINT32 streamId = result.vAppOutMeta[i]->getStreamInfo()->getStreamId();
                                        mvRequest.valueFor(requestNo)->addMeta(streamId, res);
                                        result.vAppOutMeta[i]->unlock(LOG_TAG, res);
                                    }
                                    for(size_t i = 0; i < result.vHalOutMeta.size(); ++i ) {
                                        IMetadata* res = result.vHalOutMeta[i]->tryReadLock(LOG_TAG);
                                        MINT32 streamId = result.vHalOutMeta[i]->getStreamInfo()->getStreamId();
                                        mvRequest.valueFor(requestNo)->addMeta(streamId, res);
                                        result.vHalOutMeta[i]->unlock(LOG_TAG, res);
                                    }
                                    if ( result.bFrameEnd ) mvRequest.valueFor(requestNo)->execute();
                                }
    public:
        virtual MERROR                  dequeStreamBuffer(
                                            MUINT32 const iRequestNo,
                                            android::sp<IImageStreamInfo> const pStreamInfo,
                                            android::sp<HalImageStreamBuffer> &rpStreamBuffer
                                        ){
                                            MY_LOGD("cathy deque [%u]streamId:0x%lx \n", iRequestNo, pStreamInfo->getStreamId());
                                            if ( mvRequest.indexOfKey(iRequestNo) < 0 ) {
                                                MY_LOGE("request %u not found.\n", iRequestNo);
                                                return OK;
                                            }
                                            sp<IImageBufferHeap> heap =
                                                mvRequest.valueFor(iRequestNo)->acquireHeap(pStreamInfo);
                                            rpStreamBuffer = new HalImageStreamBufferProvider(
                                                    pStreamInfo,
                                                    heap,
                                                    static_cast< wp<IStreamBufferProvider> >(this)
                                            );
                                            //
                                            Buffer buf;
                                            buf.heap    = heap;
                                            buf.handler = mvRequest.valueFor(iRequestNo);
                                            mMap.add(rpStreamBuffer, buf);
                                            //
                                            return OK;
                                        }

        virtual MERROR                  enqueStreamBuffer(
                                            android::sp<IImageStreamInfo> const pStreamInfo,
                                            android::sp<HalImageStreamBuffer> rpStreamBuffer,
                                            MUINT32  /*bBufStatus*/
                                        ){
                                            MY_LOGD("cathy return streamId:0x%lx", pStreamInfo->getStreamId());
                                            if ( mMap.indexOfKey(rpStreamBuffer) < 0 ) {
                                                MY_LOGE("map %p not found.\n", rpStreamBuffer.get());
                                                return OK;
                                            }
                                            Buffer *buf = &(mMap.editValueFor(rpStreamBuffer));
                                            (*buf).handler->releaseHeap(pStreamInfo, (*buf).heap);
                                            mMap.removeItem(rpStreamBuffer);
                                            return OK;
                                        }

    private:
        struct Buffer
        {
            sp<RHandler>         handler;
            sp<IImageBufferHeap> heap;
        };
        KeyedVector<MINT32, sp<RHandler>> mvRequest;
        KeyedVector<sp<HalImageStreamBuffer>, Buffer> mMap;
    };

}; // namespace

/******************************************************************************
 *
 ******************************************************************************/
template <typename T>
inline MBOOL
tryGetMetadata(
    IMetadata* pMetadata,
    MUINT32 const tag,
    T & rVal
)
{
    if( pMetadata == NULL ) {
        MY_LOGE("pMetadata == NULL");
        return MFALSE;
    }

    IMetadata::IEntry entry = pMetadata->entryFor(tag);
    if( !entry.isEmpty() ) {
        rVal = entry.itemAt(0, Type2Type<T>());
        return MTRUE;
    }
    return MFALSE;
}

template <typename T>
inline MBOOL
trySetMetadata(
    IMetadata* pMetadata,
    MUINT32 const tag,
    T const& val
)
{
    if( pMetadata == NULL ) {
        MY_LOGE("pMetadata == NULL");
        return MFALSE;
    }

    IMetadata::IEntry entry(tag);
    entry.push_back(val, Type2Type<T>());
    pMetadata->update(tag, entry);
    return MTRUE;
}

/******************************************************************************
 *
 ******************************************************************************/
sp<ImageStreamInfo>
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
        case eImgFmt_YUY2:
            addBufPlane(bufPlanes , imgSize.h      , imgSize.w << 1);
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
            MY_LOGE("format not support yet 0x%x \n", imgFormat);
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
        MY_LOGE("create ImageStream failed, %s, %ld \n",
                streamName, streamId);
    }

    return pStreamInfo;
}


/******************************************************************************
 *
 ******************************************************************************/
sp<ImageStreamInfo>
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
    switch( imgFormat ) {
        case eImgFmt_BAYER10:
        case eImgFmt_FG_BAYER10:
            addBufPlane(bufPlanes , imgSize.h, stride);
            break;
        default:
            MY_LOGE("format not support yet 0x%x \n", imgFormat);
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
        MY_LOGE("create ImageStream failed, %s, %ld \n",
                streamName, streamId);
    }

    return pStreamInfo;
}

/******************************************************************************
 *
 ******************************************************************************/
/******************************************************************************
 *
 ******************************************************************************/
void setupMetaStreamInfo()
{
    gControlMeta_App =
        new MetaStreamInfo(
                "App:Meta:Control",
                STREAM_ID_METADATA_CONTROL_APP,
                eSTREAMTYPE_META_IN,
                0
                );
    gControlMeta_Hal =
        new MetaStreamInfo(
                "Hal:Meta:Control",
                STREAM_ID_METADATA_CONTROL_HAL,
                eSTREAMTYPE_META_IN,
                0
                );
    gResultMeta_P1_App =
        new MetaStreamInfo(
                "App:Meta:ResultP1",
                STREAM_ID_METADATA_RESULT_P1_APP,
                eSTREAMTYPE_META_OUT,
                0
                );
    gResultMeta_P1_Hal =
        new MetaStreamInfo(
                "Hal:Meta:ResultP1",
                STREAM_ID_METADATA_RESULT_P1_HAL,
                eSTREAMTYPE_META_INOUT,
                0
                );
    gResultMeta_P2_App =
        new MetaStreamInfo(
                "App:Meta:ResultP2",
                STREAM_ID_METADATA_RESULT_P2_APP,
                eSTREAMTYPE_META_OUT,
                0
                );
    gResultMeta_P2_Hal =
        new MetaStreamInfo(
                "Hal:Meta:ResultP2",
                STREAM_ID_METADATA_RESULT_P2_HAL,
                eSTREAMTYPE_META_INOUT,
                0
                );
    gResultMeta_Jpeg_App =
        new MetaStreamInfo(
                "App:Meta:Jpeg",
                STREAM_ID_METADATA_RESULT_JPEG_APP,
                eSTREAMTYPE_META_OUT,
                0
                );
}

void setupImageStreamInfo()
{
    MSize const& jpegSize = gImgoSize;
    MSize const& thumbnailSize = MSize(120,80);
    {// Resized Raw
        MSize const& size = gRrzoSize;
        MINT const format = gRrzoFormat;
        size_t const stride = gRrzoStride;
        MUINT const usage = 0;//eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_HW_CAMERA_READWRITE ;
        gImage_RrzoRaw = createRawImageStreamInfo(
                "Hal:Image:Resiedraw",
                STREAM_ID_RAW1,
                eSTREAMTYPE_IMAGE_INOUT,
                6, 4,
                usage, format, size, stride
                );
    }
    {// Full Raw
        MSize const& size = gImgoSize;
        MINT const format = gImgoFormat;
        size_t const stride = gImgoStride;
        MUINT const usage = 0;//eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_HW_CAMERA_READWRITE ;
        gImage_ImgoRaw = createRawImageStreamInfo(
                "Hal:Image:Fullraw",
                STREAM_ID_RAW2,
                eSTREAMTYPE_IMAGE_INOUT,
                6, 4,
                usage, format, size, stride
                );
    }
    {// Display
        MSize const& size = MSize(640,480);
        MINT const format = eImgFmt_YUY2;
        MUINT const usage = 0;//eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_HW_CAMERA_READWRITE ;//0;
        gImage_Yuv = createImageStreamInfo(
                "Hal:Image:yuv",
                STREAM_ID_YUV1,
                eSTREAMTYPE_IMAGE_INOUT,
                5, 1,
                usage, format, size
                );
    }
    {// JpegYuv
      /* Important!!, a problem will happen if maxnum is 1. (Buffer pool shouldn't be lack by design)
         A thread may be pending on mItemMapLock in pipelineframe's markUserStatus()
         while another thread is waiting this buffer in getImageBuffer() */
        MSize const& size = jpegSize;
        MINT const format = eImgFmt_YUY2;
        MUINT const usage = 0;//eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_HW_CAMERA_READWRITE ;//0;
        gImage_YuvJpeg = createImageStreamInfo(
                "Hal:Image:YuvJpeg",
                STREAM_ID_YUV_MAIN,
                eSTREAMTYPE_IMAGE_INOUT,
                5, 1,
                usage, format, size
                );
    }
    {// working buffer
        MSize const& size = jpegSize;
        MINT const format = eImgFmt_YUY2;
        MUINT const usage = 0;//eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_HW_CAMERA_READWRITE ;//0;
        gImage_YuvWork = createImageStreamInfo(
                "Hal:Image:YuvWork",
                IVENDOR_STREAMID_IMAGE_PIPE_WORKING,
                eSTREAMTYPE_IMAGE_INOUT,
                5, 1,
                usage, format, size
                );
    }
    {// JpegYuvThumbnail
        MSize const& size = thumbnailSize;
        MINT const format = eImgFmt_YUY2;
        MUINT const usage = 0;//eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_HW_CAMERA_READWRITE ;//0;
        gImage_YuvThumbnail = createImageStreamInfo(
                "Hal:Image:YuvThumbnail",
                STREAM_ID_YUV_THUMBNAIL,
                eSTREAMTYPE_IMAGE_INOUT,
                5, 1,
                usage, format, size
                );
    }
    {// Jpeg
        MSize const& size = jpegSize;
        MINT const format = eImgFmt_BLOB;
        MUINT const usage = 0;//eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_HW_CAMERA_READWRITE ;//0;
        gImage_Jpeg = createImageStreamInfo(
                "Hal:Image:jpeg",
                STREAM_ID_JPEG,
                eSTREAMTYPE_IMAGE_INOUT,
                1, 1,
                usage, format, size
                );
        gJpegPool = new Pool(gImage_Jpeg);
    }
    {
        IImageStreamInfo::BufPlanes_t const& bufPlanes = gImage_YuvThumbnail->getBufPlanes();
        size_t bufStridesInBytes[3] = {0};
        size_t bufBoundaryInBytes[3]= {0};
        for (size_t i = 0; i < bufPlanes.size(); i++) {
            bufStridesInBytes[i] = bufPlanes[i].rowStrideInBytes;
        }

        IImageBufferAllocator::ImgParam imgParam(
            gImage_YuvThumbnail->getImgFormat(),
            gImage_YuvThumbnail->getImgSize(),
            bufStridesInBytes, bufBoundaryInBytes,
            bufPlanes.size()
            );
        MY_LOGD("format:%x, size:(%d,%d), stride:%zu, boundary:%zu, planes:%zu",
            gImage_YuvThumbnail->getImgFormat(),
            gImage_YuvThumbnail->getImgSize().w, gImage_YuvThumbnail->getImgSize().h,
            bufStridesInBytes[0], bufBoundaryInBytes[0], bufPlanes.size()
            );
        gFakeHeap = IIonImageBufferHeap::create(
                                gImage_YuvThumbnail->getStreamName(),
                                imgParam,
                                IIonImageBufferHeap::AllocExtraParam(),
                                MFALSE
                            );
    }
}

/******************************************************************************
 *
 ******************************************************************************/
void prepareSensor()
{
    IHalSensorList* const pHalSensorList = MAKE_HalSensorList();
    MUINT num = pHalSensorList ? pHalSensorList->searchSensors() : 0;
    MY_LOGD("searchSensors (%d)\n", num);

    mpSensorHalObj = pHalSensorList->createSensor("tester", deviceId);
    if( ! mpSensorHalObj ) {
        MY_LOGE("create sensor failed");
        exit(1);
        return;
    }
    MUINT32    sensorArray[1] = {static_cast<MUINT32>(deviceId)};
    mpSensorHalObj->powerOn("tester", 1, &sensorArray[0]);

    MUINT32 sensorDev = pHalSensorList->querySensorDevIdx(deviceId);
    SensorStaticInfo sensorStaticInfo;
    memset(&sensorStaticInfo, 0, sizeof(SensorStaticInfo));
    pHalSensorList->querySensorStaticInfo(sensorDev, &sensorStaticInfo);
    //
    gSensorParam.mode = SENSOR_SCENARIO_ID_NORMAL_CAPTURE;//SENSOR_SCENARIO_ID_NORMAL_PREVIEW;
    gSensorParam.size = MSize(sensorStaticInfo.captureWidth, sensorStaticInfo.captureHeight);//(sensorStaticInfo.previewWidth, sensorStaticInfo.previewHeight);
    #ifdef USING_MTK_LDVT /*[EP_TEMP]*/ //[FIXME] TempTestOnly - USING_FAKE_SENSOR
    gSensorParam.fps = 1;
    #else
    gSensorParam.fps = sensorStaticInfo.captureFrameRate/10;//previewFrameRate/10;
    #endif
    gSensorParam.pixelMode = 0;
    //
    mpSensorHalObj->sendCommand(
            pHalSensorList->querySensorDevIdx(deviceId),
            SENSOR_CMD_GET_SENSOR_PIXELMODE,
            (MUINTPTR)(&gSensorParam.mode),
            (MUINTPTR)(&gSensorParam.fps),
            (MUINTPTR)(&gSensorParam.pixelMode));
    //
    MY_LOGD("sensor params mode %d, size %dx%d, fps %d, pixelmode %d\n",
            gSensorParam.mode,
            gSensorParam.size.w, gSensorParam.size.h,
            gSensorParam.fps,
            gSensorParam.pixelMode);
    //exit(1);
}

/******************************************************************************
 *
 ******************************************************************************/
void closeSensor()
{
    MUINT32    sensorArray[1] = {static_cast<MUINT32>(deviceId)};
    mpSensorHalObj->powerOff("tester", 1, &sensorArray[0]);
}


/******************************************************************************
 *
 ******************************************************************************/
void prepareConfiguration()
{
#define ALIGN_2(x)     (((x) + 1) & (~1))
    //
    {
        if ( NSMetadataProviderManager::indexOfKey(deviceId) < 0) {
            MY_LOGD("pMetadataProvider ...\n");
            sp<IMetadataProvider> pMetadataProvider = IMetadataProvider::create(deviceId);
            if (pMetadataProvider.get() != NULL) {
                MY_LOGD("pMetadataProvider (%p) +++\n", pMetadataProvider.get());
            }
            NSMetadataProviderManager::add(deviceId, pMetadataProvider.get());
            if (pMetadataProvider.get() != NULL) {
                MY_LOGD("pMetadataProvider (%p) ---\n", pMetadataProvider.get());
            }
        }
    }
    {
        ITemplateRequest* obj = NSTemplateRequestManager::valueFor(deviceId);
        if(obj == NULL) {
            obj = ITemplateRequest::getInstance(deviceId);
            NSTemplateRequestManager::add(deviceId, obj);
        }
    }
    //
    MSize rrzoSize = MSize( ALIGN_2(gSensorParam.size.w / 2), ALIGN_2(gSensorParam.size.h / 2) );
    //
    #if 0
    NSImageio::NSIspio::ISP_QUERY_RST queryRst;
    //
    NSImageio::NSIspio::ISP_QuerySize(
            NSImageio::NSIspio::EPortIndex_RRZO,
            NSImageio::NSIspio::ISP_QUERY_X_PIX|
            NSImageio::NSIspio::ISP_QUERY_STRIDE_PIX|
            NSImageio::NSIspio::ISP_QUERY_STRIDE_BYTE,
            (EImageFormat)gRrzoFormat,
            rrzoSize.w,
            queryRst,
            gSensorParam.pixelMode  == 0 ?
            NSImageio::NSIspio::ISP_QUERY_1_PIX_MODE :
            NSImageio::NSIspio::ISP_QUERY_2_PIX_MODE
            );
    rrzoSize.w = queryRst.x_pix;
    gRrzoSize = MSize(rrzoSize.w, rrzoSize.h);
    gRrzoStride = queryRst.stride_byte;
    #else
    NSCam::NSIoPipe::NSCamIOPipe::NormalPipe_QueryInfo qry_rrzo;
    NSCam::NSIoPipe::NSCamIOPipe::INormalPipeModule::get()->query(
        NSCam::NSIoPipe::PORT_RRZO.index,
        NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd_X_PIX|NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd_STRIDE_BYTE,
        (EImageFormat)gRrzoFormat, rrzoSize.w, qry_rrzo);
    rrzoSize.w = qry_rrzo.x_pix;
    gRrzoSize = MSize(rrzoSize.w, rrzoSize.h);
    gRrzoStride = qry_rrzo.stride_byte;
    #endif
    MY_LOGD("rrzo size %dx%d, stride %d\n", gRrzoSize.w, gRrzoSize.h, (int)gRrzoStride);
    //
    //
    MSize imgoSize = MSize( gSensorParam.size.w, gSensorParam.size.h );
    //
    #if 0
    NSImageio::NSIspio::ISP_QUERY_RST queryRstF;
    //
    NSImageio::NSIspio::ISP_QuerySize(
            NSImageio::NSIspio::EPortIndex_IMGO,
            NSImageio::NSIspio::ISP_QUERY_X_PIX|
            NSImageio::NSIspio::ISP_QUERY_STRIDE_PIX|
            NSImageio::NSIspio::ISP_QUERY_STRIDE_BYTE,
            (EImageFormat)gImgoFormat,
            imgoSize.w,
            queryRstF,
            gSensorParam.pixelMode  == 0 ?
            NSImageio::NSIspio::ISP_QUERY_1_PIX_MODE :
            NSImageio::NSIspio::ISP_QUERY_2_PIX_MODE
            );
    imgoSize.w = queryRstF.x_pix;
    gImgoSize = MSize(imgoSize.w, imgoSize.h);
    gImgoStride = queryRstF.stride_byte;
    #else
    NSCam::NSIoPipe::NSCamIOPipe::NormalPipe_QueryInfo qry_imgo;
    NSCam::NSIoPipe::NSCamIOPipe::INormalPipeModule::get()->query(
        NSCam::NSIoPipe::PORT_IMGO.index,
        NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd_X_PIX|NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd_STRIDE_BYTE,
        (EImageFormat)gImgoFormat, imgoSize.w, qry_imgo);
    imgoSize.w = qry_imgo.x_pix;
    gImgoSize = MSize(imgoSize.w, imgoSize.h);
    gImgoStride = qry_imgo.stride_byte;
    #endif
    MY_LOGD("imgo size %dx%d, stride %d\n", gImgoSize.w, gImgoSize.h, (int)gImgoStride);
    //
    //exit(1);
}
/******************************************************************************
*
*******************************************************************************/
static
MVOID   add_stream_to_set( StreamSet& set, sp<IStreamInfo> pInfo ) {
    if( pInfo.get() ) set.add(pInfo->getStreamId());
}
/******************************************************************************
 *
 ******************************************************************************/
void setupPipelineContext()
{
    gContext = PipelineContext::create("test");
    if( !gContext.get() ) {
        MY_LOGE("cannot create context\n");
        return;
    }
    //
    gContext->beginConfigure();
    //
    // 1. Streams ***************
    //
    // 1.a. check if stream exist
    // 1.b. setup streams
    {
        // Meta
        StreamBuilder(eStreamType_META_APP, gControlMeta_App)
            .build(gContext);
        StreamBuilder(eStreamType_META_HAL, gControlMeta_Hal)
            .build(gContext);
        StreamBuilder(eStreamType_META_APP, gResultMeta_P1_App)
            .build(gContext);
        StreamBuilder(eStreamType_META_HAL, gResultMeta_P1_Hal)
            .build(gContext);
        StreamBuilder(eStreamType_META_APP, gResultMeta_P2_App)
            .build(gContext);
        StreamBuilder(eStreamType_META_HAL, gResultMeta_P2_Hal)
            .build(gContext);
        StreamBuilder(eStreamType_META_APP, gResultMeta_Jpeg_App)
            .build(gContext);
        // Image
        StreamBuilder(eStreamType_IMG_HAL_POOL, gImage_RrzoRaw)
            .build(gContext);
        StreamBuilder(eStreamType_IMG_HAL_PROVIDER, gImage_ImgoRaw)
            .setProvider(gProcessor)
            .build(gContext);
        StreamBuilder(eStreamType_IMG_HAL_POOL, gImage_Yuv)
            .build(gContext);
        StreamBuilder(eStreamType_IMG_HAL_PROVIDER, gImage_YuvWork)
            .setProvider(gProcessor)
            .build(gContext);
        StreamBuilder(eStreamType_IMG_HAL_PROVIDER, gImage_YuvJpeg)
            .setProvider(gProcessor)
            .build(gContext);
        StreamBuilder(eStreamType_IMG_HAL_PROVIDER, gImage_YuvThumbnail)
            .setProvider(gProcessor)
            .build(gContext);
        StreamBuilder(eStreamType_IMG_HAL_POOL, gImage_Jpeg)  //FIXME: Need to provide provider or AppImage
            .build(gContext);
    }
    //
    // 2. Nodes   ***************
    //
    // 2.a. check if node exist
    // 2.b. setup nodes
    //
    {
        typedef P1Node                  NodeT;
        typedef NodeActor< NodeT >      MyNodeActorT;
        //
        MY_LOGD("Nodebuilder p1 +\n");
        NodeT::InitParams initParam;
        {
            initParam.openId   = deviceId;
            initParam.nodeId   = eNODEID_P1Node;
            initParam.nodeName = "P1Node";
        }
        NodeT::ConfigParams cfgParam;
        {
            cfgParam.pInAppMeta        = gControlMeta_App;
            cfgParam.pInHalMeta        = gControlMeta_Hal;
            cfgParam.pOutAppMeta       = gResultMeta_P1_App;
            cfgParam.pOutHalMeta       = gResultMeta_P1_Hal;
            cfgParam.pOutImage_resizer = gImage_RrzoRaw;
            cfgParam.pvOutImage_full.push_back(gImage_ImgoRaw); //N/A
            cfgParam.sensorParams = gSensorParam;
            #if 1 // test the NULL pool parameter
            cfgParam.pStreamPool_resizer = NULL;
            cfgParam.pStreamPool_full    = NULL;
            #else
            cfgParam.pStreamPool_resizer = gContext->queryImageStreamPool(STREAM_ID_RAW1);
            cfgParam.pStreamPool_full    = gContext->queryImageStreamPool(STREAM_ID_RAW2);
            #endif
        }
        //
        sp<MyNodeActorT> pNode = new MyNodeActorT( NodeT::createInstance() );
        pNode->setInitParam(initParam);
        pNode->setConfigParam(cfgParam);
        //
        StreamSet vIn;
        add_stream_to_set(vIn, gControlMeta_App);
        add_stream_to_set(vIn, gControlMeta_Hal);
        //
        StreamSet vOut;
        add_stream_to_set(vOut, gImage_RrzoRaw);
        add_stream_to_set(vOut, gImage_ImgoRaw);
        add_stream_to_set(vOut, gResultMeta_P1_App);
        add_stream_to_set(vOut, gResultMeta_P1_Hal);
        //
        NodeBuilder aNodeBuilder(eNODEID_P1Node, pNode);

        aNodeBuilder.addStream(NodeBuilder::eDirection_IN , vIn);
        aNodeBuilder.addStream(NodeBuilder::eDirection_OUT, vOut);

        if (gImage_RrzoRaw != 0)
            aNodeBuilder.setImageStreamUsage(gImage_RrzoRaw->getStreamId(), eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_HW_CAMERA_WRITE);
        if (gImage_ImgoRaw != 0)
            aNodeBuilder.setImageStreamUsage(gImage_ImgoRaw->getStreamId(), eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_HW_CAMERA_WRITE);

        MERROR ret = aNodeBuilder.build(gContext);

        MY_LOGD("Nodebuilder p1 -\n");

        if( ret != OK ) {
            MY_LOGE("build p1 node error\n");
            return;
        }
    }

    {
        typedef P2Node                  NodeT;
        typedef NodeActor< NodeT >      MyNodeActorT;
        //
        MY_LOGD("Nodebuilder p2 +\n");
        NodeT::InitParams initParam;
        {
            initParam.openId   = deviceId;
            initParam.nodeId   = eNODEID_P2Node;
            initParam.nodeName = "P2Node";
        }
        NodeT::ConfigParams cfgParam;
        {
            cfgParam.pInAppMeta    = gControlMeta_App;
            cfgParam.pInAppRetMeta = gResultMeta_P1_App;
            cfgParam.pInHalMeta    = gResultMeta_P1_Hal;
            cfgParam.pOutAppMeta   = gResultMeta_P2_App;
            cfgParam.pOutHalMeta   = gResultMeta_P2_Hal;
            if (gImage_ImgoRaw != 0)
                cfgParam.pvInFullRaw.push_back(gImage_ImgoRaw);
            cfgParam.pInResizedRaw = gImage_RrzoRaw;
            if (gImage_Yuv != 0)
                cfgParam.vOutImage.push_back(gImage_Yuv);
            if (gImage_YuvWork != 0)
                cfgParam.vOutImage.push_back(gImage_YuvWork);
            if (gImage_YuvJpeg != 0)
                cfgParam.vOutImage.push_back(gImage_YuvJpeg);
            if (gImage_YuvThumbnail != 0)
                cfgParam.vOutImage.push_back(gImage_YuvThumbnail);
//            cfgParam.pOutFDImage = gImage_YuvThumbnail;  // FIXME: why ? IMG2O can output not only FD but also Thumbnail to save a run.
        }
        //
        sp<MyNodeActorT> pNode = new MyNodeActorT( NodeT::createInstance() );
        pNode->setInitParam(initParam);
        pNode->setConfigParam(cfgParam);
        //
        NodeBuilder aNodeBuilder(eNODEID_P2Node, pNode);
        //
        StreamSet vIn;
        add_stream_to_set(vIn, gImage_ImgoRaw);
        add_stream_to_set(vIn, gImage_RrzoRaw);
        add_stream_to_set(vIn, gControlMeta_App);
        add_stream_to_set(vIn, gResultMeta_P1_App);
        add_stream_to_set(vIn, gResultMeta_P1_Hal);
        //
        StreamSet vOut;
        add_stream_to_set(vOut, gImage_Yuv);
        add_stream_to_set(vOut, gImage_YuvWork);
        add_stream_to_set(vOut, gImage_YuvJpeg);
        add_stream_to_set(vOut, gImage_YuvThumbnail);
        add_stream_to_set(vOut, gResultMeta_P2_App);
        add_stream_to_set(vOut, gResultMeta_P2_Hal);

        aNodeBuilder.addStream(NodeBuilder::eDirection_IN , vIn);
        aNodeBuilder.addStream(NodeBuilder::eDirection_OUT, vOut);

        if (gImage_RrzoRaw != 0)
            aNodeBuilder.setImageStreamUsage(gImage_RrzoRaw->getStreamId(), eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_HW_CAMERA_READ);
        if (gImage_ImgoRaw != 0)
            aNodeBuilder.setImageStreamUsage(gImage_ImgoRaw->getStreamId(), eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_HW_CAMERA_READ);
        if (gImage_Yuv != 0)
            aNodeBuilder.setImageStreamUsage(gImage_Yuv    ->getStreamId(), eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_HW_CAMERA_WRITE);
        if (gImage_YuvWork != 0)
            aNodeBuilder.setImageStreamUsage(gImage_YuvWork->getStreamId(), eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_HW_CAMERA_WRITE);
        if (gImage_YuvJpeg != 0)
            aNodeBuilder.setImageStreamUsage(gImage_YuvJpeg->getStreamId(), eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_HW_CAMERA_WRITE);
        if (gImage_YuvThumbnail != 0)
            aNodeBuilder.setImageStreamUsage(gImage_YuvThumbnail->getStreamId(), eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_HW_CAMERA_WRITE);

        MERROR ret = aNodeBuilder.build(gContext);

        MY_LOGD("Nodebuilder p2 -\n");

        if( ret != OK ) {
            MY_LOGE("build p2 node error\n");
            return;
        }
    }

    {
        typedef JpegNode                NodeT;
        typedef NodeActor< NodeT >      MyNodeActorT;
        //
        MY_LOGD("Nodebuilder Jpeg +\n");
        NodeT::InitParams initParam;
        {
            initParam.openId   = deviceId;
            initParam.nodeId   = eNODEID_JpegNode;
            initParam.nodeName = "JpegNode";
        }
        NodeT::ConfigParams cfgParam;
        {
            cfgParam.pInAppMeta       = gControlMeta_App;
            cfgParam.pInHalMeta       = gResultMeta_P2_Hal;
            cfgParam.pOutAppMeta      = gResultMeta_Jpeg_App;
            cfgParam.pInYuv_Main      = gImage_YuvJpeg;
            cfgParam.pInYuv_Thumbnail = gImage_YuvThumbnail;
            cfgParam.pOutJpeg         = gImage_Jpeg;
        }
        //
        sp<MyNodeActorT> pNode = new MyNodeActorT( NodeT::createInstance() );
        pNode->setInitParam(initParam);
        pNode->setConfigParam(cfgParam);
        //
        NodeBuilder aNodeBuilder(eNODEID_JpegNode, pNode);
        //
        StreamSet vIn;
        add_stream_to_set(vIn, gImage_YuvJpeg);
        add_stream_to_set(vIn, gImage_YuvThumbnail);
        add_stream_to_set(vIn, gControlMeta_App);
        add_stream_to_set(vIn, gResultMeta_P2_Hal);
        //
        StreamSet vOut;
        add_stream_to_set(vOut, gImage_Jpeg);
        add_stream_to_set(vOut, gResultMeta_Jpeg_App);

        aNodeBuilder.addStream(NodeBuilder::eDirection_IN , vIn);
        aNodeBuilder.addStream(NodeBuilder::eDirection_OUT, vOut);

        if (gImage_YuvJpeg != 0)
            aNodeBuilder.setImageStreamUsage(gImage_YuvJpeg->getStreamId(), eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_HW_CAMERA_READ);
        if (gImage_YuvThumbnail != 0)
            aNodeBuilder.setImageStreamUsage(gImage_YuvThumbnail->getStreamId(), eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_HW_CAMERA_READ);
        if (gImage_Jpeg != 0)
            aNodeBuilder.setImageStreamUsage(gImage_Jpeg->getStreamId(), eBUFFER_USAGE_SW_MASK | eBUFFER_USAGE_HW_CAMERA_WRITE);

        MERROR ret = aNodeBuilder.build(gContext);

        MY_LOGD("Nodebuilder Jpeg -\n");

        if( ret != OK ) {
            MY_LOGE("build Jpeg node error\n");
            return;
        }
    }
    //
    // 3. Pipeline **************
    //
    {
        NodeEdgeSet edges;
        edges.addEdge(eNODEID_P1Node, eNODEID_P2Node);
        edges.addEdge(eNODEID_P2Node, eNODEID_JpegNode);

        MERROR ret = PipelineBuilder()
            .setRootNode(
                    NodeSet().add(eNODEID_P1Node)
                    )
            .setNodeEdges(
                    edges
                    )
            .build(gContext);
        if( ret != OK ) {
            MY_LOGE("build pipeline error\n");
            return;
        }
    }
    //
    gContext->endConfigure(true);
}
/******************************************************************************
 *
 ******************************************************************************/
void setupRequestBuilder()
{
    {// ONLY P1
        gRequestBuilderP1 = new RequestBuilder();

        gRequestBuilderP1->setIOMap(
                                eNODEID_P1Node,
                                IOMapSet().add(
                                  IOMap()
                                  .addOut(gImage_RrzoRaw->getStreamId())
                                  .addOut(gImage_ImgoRaw->getStreamId())
                                  ),
                                IOMapSet().add(
                                    IOMap()
                                    .addIn(gControlMeta_App->getStreamId())
                                    .addIn(gControlMeta_Hal->getStreamId())
                                    .addOut(gResultMeta_P1_App->getStreamId())
                                    .addOut(gResultMeta_P1_Hal->getStreamId())
                                  )
                              );
#if 0 // produce Yuv Images
        gRequestBuilderP1->setIOMap(
                                eNODEID_P2Node,
                                IOMapSet().add(
                                  IOMap()
                                  .addIn(gImage_RrzoRaw->getStreamId())
                                  .addOut(gImage_Yuv->getStreamId())
                                  ),
                                IOMapSet().add(
                                    IOMap()
                                    .addIn(gControlMeta_App->getStreamId())
                                    .addIn(gResultMeta_P1_Hal->getStreamId())
                                    .addOut(gResultMeta_P2_App->getStreamId())
                                    .addOut(gResultMeta_P2_Hal->getStreamId())
                                  )
                              );
        gRequestBuilderP1->setNodeEdges(
                                NodeEdgeSet().addEdge(eNODEID_P1Node, eNODEID_P2Node)
                              );
#endif

        gRequestBuilderP1->setRootNode(
                                NodeSet().add(eNODEID_P1Node)
                              );
        gRequestBuilderP1->updateFrameCallback(gProcessor);
    }

    {// Preview Request
        gRequestBuilderPrv = new RequestBuilder();

        gRequestBuilderPrv->setIOMap(
                                eNODEID_P1Node,
                                IOMapSet().add(
                                  IOMap()
                                  .addOut(gImage_RrzoRaw->getStreamId())
                                  .addOut(gImage_ImgoRaw->getStreamId())
                                  ),
                                IOMapSet().add(
                                    IOMap()
                                    .addIn(gControlMeta_App->getStreamId())
                                    .addIn(gControlMeta_Hal->getStreamId())
                                    .addOut(gResultMeta_P1_App->getStreamId())
                                    .addOut(gResultMeta_P1_Hal->getStreamId())
                                  )
                              );
#if 0 // test to use 2 IOMaps to produce Yuv Images
        IOMapSet imgIOMapSet, metaIOMapSet;
        imgIOMapSet.add(
                      IOMap()
                      .addIn(gImage_RrzoRaw->getStreamId())
                      .addOut(gImage_Yuv->getStreamId())
                    );
        imgIOMapSet.add(
                      IOMap()
                      .addIn(gImage_ImgoRaw->getStreamId())
                      .addOut(gImage_YuvJpeg->getStreamId())
                      .addOut(gImage_YuvThumbnail->getStreamId())
                    );
        metaIOMapSet.add(
                      IOMap()
                      .addIn(gControlMeta_App->getStreamId())
                      .addIn(gResultMeta_P1_Hal->getStreamId())
                      .addOut(gResultMeta_P2_App->getStreamId())
                      .addOut(gResultMeta_P2_Hal->getStreamId())
                    );
        gRequestBuilderPrv->setIOMap(
                                eNODEID_P2Node,
                                imgIOMapSet,
                                metaIOMapSet
                              );
#else
        gRequestBuilderPrv->setIOMap(
                                eNODEID_P2Node,
                                IOMapSet().add(
                                  IOMap()
                                  .addIn(gImage_ImgoRaw->getStreamId())
                                  .addOut(gImage_YuvJpeg->getStreamId())
                                  ),
                                IOMapSet().add(
                                    IOMap()
                                    .addIn(gControlMeta_App->getStreamId())
                                    .addIn(gResultMeta_P1_Hal->getStreamId())
                                    .addOut(gResultMeta_P2_App->getStreamId())
                                    .addOut(gResultMeta_P2_Hal->getStreamId())
                                  )
                              );
#endif
        gRequestBuilderPrv->setRootNode(
                                NodeSet().add(eNODEID_P1Node)
                              );
        gRequestBuilderPrv->setNodeEdges(
                                NodeEdgeSet().addEdge(eNODEID_P1Node, eNODEID_P2Node)
                              );
        gRequestBuilderPrv->updateFrameCallback(gProcessor);
    }

    {// p2 out to working buffer
        gRequestBuilderPrvWork = new RequestBuilder();
        gRequestBuilderPrvWork->setIOMap(
                                eNODEID_P1Node,
                                IOMapSet().add(
                                  IOMap()
                                  .addOut(gImage_RrzoRaw->getStreamId())
                                  .addOut(gImage_ImgoRaw->getStreamId())
                                  ),
                                IOMapSet().add(
                                    IOMap()
                                    .addIn(gControlMeta_App->getStreamId())
                                    .addIn(gControlMeta_Hal->getStreamId())
                                    .addOut(gResultMeta_P1_App->getStreamId())
                                    .addOut(gResultMeta_P1_Hal->getStreamId())
                                  )
                              );

        gRequestBuilderPrvWork->setIOMap(
                                eNODEID_P2Node,
                                IOMapSet().add(
                                  IOMap()
                                  .addIn(gImage_ImgoRaw->getStreamId())
                                  .addOut(gImage_YuvWork->getStreamId())
                                  ),
                                IOMapSet().add(
                                    IOMap()
                                    .addIn(gControlMeta_App->getStreamId())
                                    .addIn(gResultMeta_P1_Hal->getStreamId())
                                    .addOut(gResultMeta_P2_App->getStreamId())
                                    .addOut(gResultMeta_P2_Hal->getStreamId())
                                  )
                              );
        gRequestBuilderPrvWork->setRootNode(
                                NodeSet().add(eNODEID_P1Node)
                              );
        gRequestBuilderPrvWork->setNodeEdges(
                                NodeEdgeSet().addEdge(eNODEID_P1Node, eNODEID_P2Node)
                              );
        gRequestBuilderPrvWork->updateFrameCallback(gProcessor);
    }

    {// Capture Request
        gRequestBuilderCap = new RequestBuilder();

        gRequestBuilderCap->setIOMap(
                                eNODEID_P1Node,
                                IOMapSet().add(
                                  IOMap()
                                  .addOut(gImage_RrzoRaw->getStreamId())
                                  .addOut(gImage_ImgoRaw->getStreamId())
                                  ),
                                IOMapSet().add(
                                    IOMap()
                                    .addIn(gControlMeta_App->getStreamId())
                                    .addIn(gControlMeta_Hal->getStreamId())
                                    .addOut(gResultMeta_P1_App->getStreamId())
                                    .addOut(gResultMeta_P1_Hal->getStreamId())
                                  )
                              );
        gRequestBuilderCap->setIOMap(
                                eNODEID_P2Node,
                                IOMapSet().add(
                                  IOMap()
                                  .addIn(gImage_ImgoRaw->getStreamId())
                                  .addOut(gImage_Yuv->getStreamId())
                                  .addOut(gImage_YuvJpeg->getStreamId())
                                  .addOut(gImage_YuvThumbnail->getStreamId())
                                  ),
                                IOMapSet().add(
                                    IOMap()
                                    .addIn(gControlMeta_App->getStreamId())
                                    .addIn(gResultMeta_P1_App->getStreamId())
                                    .addIn(gResultMeta_P1_Hal->getStreamId())
                                    .addOut(gResultMeta_P2_App->getStreamId())
                                    .addOut(gResultMeta_P2_Hal->getStreamId())
                                  )
                              );
        gRequestBuilderCap->setIOMap(
                                eNODEID_JpegNode,
                                IOMapSet().add(
                                  IOMap()
                                  .addIn(gImage_YuvJpeg->getStreamId())
                                  .addIn(gImage_YuvThumbnail->getStreamId())
                                  .addOut(gImage_Jpeg->getStreamId())
                                  ),
                                IOMapSet().add(
                                    IOMap()
                                    .addIn(gControlMeta_App->getStreamId())
                                    .addIn(gResultMeta_P2_Hal->getStreamId())
                                    .addOut(gResultMeta_Jpeg_App->getStreamId())
                                  )
                              );
        gRequestBuilderCap->setRootNode(
                                NodeSet().add(eNODEID_P1Node)
                              );

        NodeEdgeSet edges;
        edges.addEdge(eNODEID_P1Node, eNODEID_P2Node);
        edges.addEdge(eNODEID_P2Node, eNODEID_JpegNode);

        gRequestBuilderCap->setNodeEdges(edges);
        gRequestBuilderCap->updateFrameCallback(gProcessor);
    }
}
/******************************************************************************
 *
 ******************************************************************************/
IMetaStreamBuffer*
createMetaStreamBuffer(
    android::sp<IMetaStreamInfo> pStreamInfo,
    IMetadata const& rSettings,
    MBOOL const repeating
)
{
    HalMetaStreamBuffer*
    pStreamBuffer =
    HalMetaStreamBuffer::Allocator(pStreamInfo.get())(rSettings);
    //
    pStreamBuffer->setRepeating(repeating);
    //
    return pStreamBuffer;
}


/******************************************************************************
 *
 ******************************************************************************/
sp<IMetaStreamBuffer>
get_default_request()
{
    sp<IMetaStreamBuffer> pSBuffer;

    ITemplateRequest* obj = NSTemplateRequestManager::valueFor(deviceId);
    if(obj == NULL) {
        obj = ITemplateRequest::getInstance(deviceId);
        NSTemplateRequestManager::add(deviceId, obj);
    }
    IMetadata meta = obj->getMtkData(requestTemplate);
    //
    pSBuffer = createMetaStreamBuffer(gControlMeta_App, meta, false);
    //
    return pSBuffer;
}

/******************************************************************************
 *
 ******************************************************************************/
void finishPipelineContext()
{
    MY_LOGD("waitUntilDrained ...\n");
    gContext->waitUntilDrained();
    MY_LOGD("waitUntilDrained END\n");
}

/******************************************************************************
 *
 ******************************************************************************/
void flush(MINT64 caller)
{
    sp<IVendorManager> pManager = NSVendorManager::get(deviceId);
    if ( pManager != 0 ){
        if ( caller == CALLER_SHOT_MAX_NUMBER) {
            MINT64 mask = 1;
            while( mask < CALLER_SHOT_MAX_NUMBER ) {
                pManager->beginFlush(mask);
                pManager->endFlush(mask);
                mask = mask<<1;
            }
        } else {
            pManager->beginFlush(caller);
            pManager->endFlush(caller);
        }
    }
}

/******************************************************************************
 *
 ******************************************************************************/
void clear_global_var()
{
    sp<IVendorManager> pManager = NSVendorManager::get(deviceId);
    if ( pManager != 0 ){
        MY_LOGD("flush ...\n");
        MINT64 mask = 1;
        while( mask < CALLER_SHOT_MAX_NUMBER ) {
            flush(mask);
            mask = mask<<1;
        }
        MY_LOGD("set Manager to Null ...\n");
        pManager = NULL;
    }

    gFakeHeap = nullptr;

    if ( gContext != 0 ){
        MY_LOGD("flush ...\n");
        gContext->flush();
        MY_LOGD("waitUntilDrained ...\n");
        gContext->waitUntilDrained();
        MY_LOGD("set Context to Null ...\n");
        gContext = NULL;
    }

    gProcessor = NULL;
    gJpegPool  = NULL;
}

void createBufferInfo(
    int testCase,
    Vector<MINT64>& combination,
    FrameInfo& info,
    Vector< BufferItem >& srcBuffer
)
{
    if ( combination.size() == 0) {
        switch(testCase)
        {
            case BUFFER_CASE::CASE_WORK_IN_OUT:
            {
                combination.push_back(MTK_PLUGIN_MODE_NR);
                combination.push_back(MTK_PLUGIN_MODE_MFNR);
            } break;
            case BUFFER_CASE::CASE_DST_OUT:
            {
                combination.push_back(MTK_PLUGIN_MODE_NR);
                combination.push_back(MTK_PLUGIN_MODE_HDR);
            } break;
            case BUFFER_CASE::CASE_SRC_IN:
            {
                combination.push_back(MTK_PLUGIN_MODE_MFNR);
                combination.push_back(MTK_PLUGIN_MODE_TEST_PRV);
            } break;
            case BUFFER_CASE::CASE_SRC_IN_DST_OUT:
            {
                combination.push_back(MTK_PLUGIN_MODE_MFNR);
                combination.push_back(MTK_PLUGIN_MODE_HDR);
            } break;
        };
    }

    for (size_t i = 0; i < combination.size(); ++i) {
        VendorInfo vinfo;
        vinfo.vendorMode = combination[i];
        //
        info.vVendorInfo.add(combination[i], vinfo);
    }
    //
    switch(testCase)
    {
        case BUFFER_CASE::CASE_WORK_IN_OUT:
        {
        } break;
        case BUFFER_CASE::CASE_DST_OUT:
        {
            info.vVendorInfo.editValueFor(combination[combination.size() - 1]).appOut.push_back(STREAM_ID_YUV1);
            info.vVendorInfo.editValueFor(combination[combination.size() - 1]).appOut.push_back(STREAM_ID_YUV_MAIN);

            {
                BufferItem item;
                item.bufferStatus = BUFFER_OUT;
                item.heap         = gFakeHeap;
                item.streamInfo   = gImage_Yuv;
                srcBuffer.push_back(item);
            }
            {
                BufferItem item;
                item.bufferStatus = BUFFER_OUT;
                item.heap         = gFakeHeap;
                item.streamInfo   = gImage_YuvJpeg;
                srcBuffer.push_back(item);
            }
        } break;
        case BUFFER_CASE::CASE_SRC_IN:
        {
            info.vVendorInfo.editValueFor(combination[0]).appIn.push_back(STREAM_ID_RAW1);
            {
                BufferItem item;
                item.bufferStatus = BUFFER_IN;
                item.heap         = gFakeHeap;
                item.streamInfo   = gImage_RrzoRaw;
                srcBuffer.push_back(item);
            }
        } break;
        case BUFFER_CASE::CASE_SRC_IN_DST_OUT:
        {
            info.vVendorInfo.editValueFor(combination[0]).appIn.push_back(STREAM_ID_RAW1);
            info.vVendorInfo.editValueFor(combination[combination.size() - 1]).appOut.push_back(STREAM_ID_YUV1);
            info.vVendorInfo.editValueFor(combination[combination.size() - 1]).appOut.push_back(STREAM_ID_YUV_MAIN);
            {
                BufferItem item;
                item.bufferStatus = BUFFER_IN;
                item.heap         = gFakeHeap;
                item.streamInfo   = gImage_RrzoRaw;
                srcBuffer.push_back(item);
            }
            {
                BufferItem item;
                item.bufferStatus = BUFFER_OUT;
                item.heap         = gFakeHeap;
                item.streamInfo   = gImage_Yuv;
                srcBuffer.push_back(item);
            }
            {
                BufferItem item;
                item.bufferStatus = BUFFER_OUT;
                item.heap         = gFakeHeap;
                item.streamInfo   = gImage_YuvJpeg;
                srcBuffer.push_back(item);
            }
        } break;
    };
}

/******************************************************************************
 *
 ******************************************************************************/
void testSingleShot(
    sp<DataCallback> _local,
    Vector<MINT64>& combination,
    MINT64 caller,
    FrameInfo& info,
    Vector< BufferItem >& srcBuffer)
{
    sp<IVendorManager> pManager = NSVendorManager::get(deviceId);
    if ( !pManager.get() ) {
        MY_LOGD("null pManager\n");
        return;
    }
    InputInfo in;
    OutputInfo out;
    in.combination = combination;
    pManager->get( caller, in, out );
    //
    info.frameNo = testRequestCounter;
    info.curAppControl += out.settings[0].setting.appMeta;
    info.curHalControl += out.settings[0].setting.halMeta;

    InputSetting inSetting;
    inSetting.vFrame.push_back(info);

    pManager->set( caller, inSetting );
    //
    MetaItem             setting;
    setting.setting.appMeta += out.settings[0].setting.appMeta;
    setting.setting.halMeta += out.settings[0].setting.halMeta;
    //
    pManager->queue(testRequestCounter, _local, srcBuffer, setting);

    testRequestCounter++;
}

/******************************************************************************
 *
 ******************************************************************************/
void testShot(sp<DataCallback> _local, Vector<MINT64>& combination, MINT64 caller )
{
    sp<IVendorManager> pManager = NSVendorManager::get(deviceId);
    for( int i = 0; i < testRequestCnt; ++i ) {
        FrameInfo info;
        Vector< BufferItem > srcBuffer;
        //createBufferInfo((BUFFER_CASE::CASE_WORK_IN_OUT + i), combination, info, srcBuffer);
        createBufferInfo(BUFFER_CASE::CASE_WORK_IN_OUT, combination, info, srcBuffer);
        testSingleShot(_local, combination, caller, info, srcBuffer);
    }

}

/******************************************************************************
 *
 ******************************************************************************/
void testCombination(sp<DataCallback> _local, MINT64 caller )
{
    Vector<MINT64> combination;
    {
        combination.push_back(MTK_PLUGIN_MODE_NR);
        combination.push_back(MTK_PLUGIN_MODE_HDR);

        FrameInfo info;
        Vector< BufferItem > srcBuffer;
        createBufferInfo(BUFFER_CASE::CASE_WORK_IN_OUT, combination, info, srcBuffer);
        testSingleShot(_local, combination, caller, info, srcBuffer);
    }
    //
    {
        combination.push_back(MTK_PLUGIN_MODE_MFNR);

        FrameInfo info;
        Vector< BufferItem > srcBuffer;
        createBufferInfo(BUFFER_CASE::CASE_WORK_IN_OUT, combination, info, srcBuffer);
        testSingleShot(_local, combination, caller, info, srcBuffer);
    }
    //
    {
        combination.push_back(MTK_PLUGIN_MODE_TEST_PRV);

        FrameInfo info;
        Vector< BufferItem > srcBuffer;
        createBufferInfo(BUFFER_CASE::CASE_WORK_IN_OUT, combination, info, srcBuffer);
        testSingleShot(_local, combination, caller, info, srcBuffer);
    }
}

/******************************************************************************
 *
 ******************************************************************************/
void testCaller(sp<DataCallback> _local )
{
    FrameInfo info;
    Vector< BufferItem > srcBuffer;
    Vector<MINT64> combination;
    combination.push_back(MTK_PLUGIN_MODE_NR);
    combination.push_back(MTK_PLUGIN_MODE_HDR);
    combination.push_back(MTK_PLUGIN_MODE_MFNR);
    testSingleShot(_local, combination, CALLER_PIPELINEMODEL_DEFAULT, info, srcBuffer);
    //
    combination.clear();
    combination.push_back(MTK_PLUGIN_MODE_MFNR);
    combination.push_back(MTK_PLUGIN_MODE_TEST_PRV);
    testSingleShot(_local, combination, CALLER_SHOT_SMART, info, srcBuffer);
    //
    flush(CALLER_PIPELINEMODEL_DEFAULT);
}

/******************************************************************************
 *
 ******************************************************************************/
void buildPipeline()
{
    prepareSensor();
    prepareConfiguration();
    setupMetaStreamInfo();
    setupImageStreamInfo();
    setupPipelineContext(); // Configure Stage
    setupRequestBuilder(); // SetUp RequestBuilder
}

sp<IPipelineFrame>
getPipelineFrame(MINT32 frameNo, MINT32 testCase, IMetadata app, IMetadata hal)
{
    sp<IPipelineFrame> pFrame;

    sp<IMetaStreamBuffer> pAppMetaControlSB = get_default_request();
    sp<HalMetaStreamBuffer> pHalMetaControlSB =
        HalMetaStreamBuffer::Allocator(gControlMeta_Hal.get())();
    {
        // modify app control metadata
        IMetadata* pMetadata = pAppMetaControlSB->tryWriteLock(LOG_TAG);
        (*pMetadata) += app;
        pAppMetaControlSB->unlock(LOG_TAG, pMetadata);
    }
    {
        // modify hal control metadata
        IMetadata* pMetadata = pHalMetaControlSB->tryWriteLock(LOG_TAG);
        trySetMetadata<MSize>(pMetadata, MTK_HAL_REQUEST_SENSOR_SIZE, gSensorParam.size);
        (*pMetadata) += hal;

        // turn off plugin
        //trySetMetadata<MUINT8>(pMetadata, MTK_P2NODE_UT_PLUGIN_FLAG, 0);

        pHalMetaControlSB->unlock(LOG_TAG, pMetadata);
    }
    {
        sp<RequestBuilder> pRequestBuilder;
        switch(testCase)
        {
            case eTEST_CASE_P1:
                pRequestBuilder = gRequestBuilderP1;
                break;
            case eTEST_CASE_P1_P2:
                pRequestBuilder = gRequestBuilderPrv;
                break;
            case eTEST_CASE_P1_P2_WORK:
                pRequestBuilder = gRequestBuilderPrvWork;
                break;
            default:
                break ;
        }

        if (pRequestBuilder.get())
        {
                pRequestBuilder->setMetaStreamBuffer(
                                      gControlMeta_App->getStreamId(),
                                      pAppMetaControlSB
                                  );
                pRequestBuilder->setMetaStreamBuffer(
                                      gControlMeta_Hal->getStreamId(),
                                      pHalMetaControlSB
                                  );
                pFrame = pRequestBuilder->build(frameNo, gContext);
        }
        if( ! pFrame.get() )
        {
            MY_LOGE("build request failed\n");
        }
    }
    if (0)  // Dump All MetaData
    {
        IMetadataTagSet const &mtagInfo = IDefaultMetadataTagSet::singleton()->getTagSet();
        sp<IMetadataConverter> mMetaDataConverter = IMetadataConverter::createInstance(mtagInfo);

        MY_LOGD("\n\nDump AppMeta:\n");
        MY_LOGD("==========================================================\n");
        IMetadata* pMetadata = pAppMetaControlSB->tryReadLock(LOG_TAG);
        pMetadata->dump();
        MY_LOGD("==========================================================\n");
        mMetaDataConverter->dumpAll(*pMetadata);
        pAppMetaControlSB->unlock(LOG_TAG, pMetadata);
        MY_LOGD("==========================================================\n");
        MY_LOGD("\n\nDump HalMeta:\n");
        MY_LOGD("==========================================================\n");
        pMetadata = pHalMetaControlSB->tryReadLock(LOG_TAG);
        pMetadata->dump();
//        MY_LOGD("==========================================================\n");
//        mMetaDataConverter->dumpAll(*pMetadata);  // Not Support
        pHalMetaControlSB->unlock(LOG_TAG, pMetadata);
        MY_LOGD("==========================================================\n");
    }

    return pFrame;
}

void testRAW_YUV(Vector<MINT64>& combination)
{
    int frameNo  = 0;
    int reqCount = 6;
    MINT64 caller = CALLER_PIPELINEMODEL_DEFAULT;
    sp<Pool> full_raw_pool      = new Pool(gImage_ImgoRaw);
    sp<Pool> Jpeg_yuv_pool      = new Pool(gImage_YuvJpeg);
    sp<Pool> Thumb_yuv_pool     = new Pool(gImage_YuvThumbnail);
    sp<IVendorManager> pManager = NSVendorManager::get(deviceId);

    OutputInfo out;
    // plugin setting
    {
        InputInfo in;
        in.combination = combination;
        in.fullRaw = gImage_ImgoRaw;
        in.jpegYuv = gImage_YuvJpeg;
        in.thumbnailYuv = gImage_YuvThumbnail;
        in.vYuv.push_back(gImage_Yuv);
        pManager->get( caller, in, out );
        //
        reqCount = out.frameCount;
        if ( out.inCategory != FORMAT_RAW || out.outCategory != FORMAT_YUV ) {
            MY_LOGE("category check fail %d, %d.\n", out.inCategory, out.outCategory);
            return;
        }
        //
        InputSetting inSetting;
        int temp = frameNo;
        for ( int i = 0; i < reqCount; ++i ) {
            FrameInfo info;
            info.frameNo = temp++;
            info.curAppControl += out.settings[i].setting.appMeta;
            info.curHalControl += out.settings[i].setting.halMeta;

            inSetting.vFrame.push_back(info);
        }

        pManager->set( caller, inSetting );
    }

    // pipeline + plugin
    for (int i = 0; i < reqCount; ++i)
    {
        MY_LOGD("request:%d +\n", frameNo);
        sp<IPipelineFrame> pFrame = getPipelineFrame(
                                        frameNo, eTEST_CASE_P1,
                                        out.settings[i].setting.appMeta,
                                        out.settings[i].setting.halMeta
                                    );
        //
        sp<RHandler> handler = new RHandler(frameNo, pManager);
        handler->addPool(gImage_ImgoRaw, full_raw_pool, BUFFER_TYPE::BUFFER_TYPE_IN);
        if( i == 0 ) {
            handler->addPool(gImage_YuvJpeg, Jpeg_yuv_pool, BUFFER_TYPE::BUFFER_TYPE_OUT);
            handler->addPool(gImage_YuvThumbnail, Thumb_yuv_pool, BUFFER_TYPE::BUFFER_TYPE_OUT);
        }
        //
        gProcessor->registerReq(frameNo, handler);
        //
        if( pFrame.get() ) {
            if( OK != gContext->queue(pFrame) ) {
                MY_LOGE("queue pFrame failed\n");
            }
        }
        MY_LOGD("request:%d -\n", frameNo);
        frameNo++;
    }

    gContext->waitUntilDrained();
    gProcessor->waitUntilDrained();
    gProcessor->clear();

}

void testYUV_YUV(Vector<MINT64>& combination, MINT32 testCase)
{
    int frameNo  = 0;
    int reqCount = 6;
    MINT64 caller = CALLER_PIPELINEMODEL_DEFAULT;
    sp<IVendorManager> pManager = NSVendorManager::get(deviceId);
    sp<Pool> full_raw_pool      = new Pool(gImage_ImgoRaw);
    sp<Pool> Jpeg_yuv_pool;
    if ( testCase == CASE_WORK_IN_OUT )
        Jpeg_yuv_pool = new Pool(pManager);
    else if ( testCase == CASE_DST_OUT )
        Jpeg_yuv_pool = new Pool(gImage_YuvJpeg);

    OutputInfo out;
    // plugin setting
    {
        InputInfo in;
        in.combination = combination;
        in.fullRaw = gImage_ImgoRaw;
        in.jpegYuv = gImage_YuvJpeg;
        in.thumbnailYuv = gImage_YuvThumbnail;
        in.vYuv.push_back(gImage_Yuv);
        pManager->get( caller, in, out );
        //
        reqCount = out.frameCount;
        if ( out.inCategory != FORMAT_YUV || out.outCategory != FORMAT_YUV ) {
            MY_LOGE("category check fail.\n");
            return;
        }
        //
        InputSetting inSetting;
        int temp = frameNo;
        for ( int i = 0; i < reqCount; ++i ) {
            FrameInfo info;
            info.frameNo = temp++;
            info.curAppControl += out.settings[i].setting.appMeta;
            info.curHalControl += out.settings[i].setting.halMeta;

            inSetting.vFrame.push_back(info);
        }

        pManager->set( caller, inSetting );
    }

    // pipeline + plugin
    for (int i = 0; i < reqCount; ++i)
    {
        MY_LOGD("request:%d +\n", frameNo);
        sp<IPipelineFrame> pFrame = getPipelineFrame(
                                        frameNo,
                                        (CASE_WORK_IN_OUT == testCase) ? eTEST_CASE_P1_P2_WORK : eTEST_CASE_P1_P2,
                                        out.settings[i].setting.appMeta,
                                        out.settings[i].setting.halMeta
                                    );
        //
        sp<RHandler> handler = new RHandler(frameNo, pManager);
        handler->addPool(gImage_ImgoRaw, full_raw_pool, BUFFER_TYPE::BUFFER_TYPE_NONE);
        if ( CASE_WORK_IN_OUT == testCase )
            handler->addPool(gImage_YuvWork, Jpeg_yuv_pool, BUFFER_TYPE::BUFFER_TYPE_IN);
        else if ( CASE_DST_OUT == testCase )
            handler->addPool(gImage_YuvJpeg, Jpeg_yuv_pool, BUFFER_TYPE::BUFFER_TYPE_IN);
        //
        gProcessor->registerReq(frameNo, handler);
        //
        if( pFrame.get() ) {
            if( OK != gContext->queue(pFrame) ) {
                MY_LOGE("queue pFrame failed\n");
            }
        }
        MY_LOGD("request:%d -\n", frameNo);
        frameNo++;
    }

    gContext->waitUntilDrained();
    gProcessor->waitUntilDrained();
    gProcessor->clear();

}

/******************************************************************************
 *
 ******************************************************************************/
int main(int /*argc*/, char** /*argv*/)
{
    android::sp<IVendorManager> pManager = IVendorManager::createInstance("test code", deviceId);
    NSVendorManager::add(deviceId, pManager.get());
    gProcessor = new PluginProcessor();

    setupImageStreamInfo();

#if 0
    {
        sp<DataCallback> _local = new DataCallback();
        MY_LOGD("[UT]start pure manager test ...\n");
        // pure manager test
        Vector<MINT64> combination;
        MINT64 caller = CALLER_PIPELINEMODEL_DEFAULT;
        MINT64 caller2 = CALLER_FLOWCONTROL_DEFAULT;
        //
        FrameInfo info;
        Vector< BufferItem > srcBuffer;
        createBufferInfo(BUFFER_CASE::CASE_SRC_IN_DST_OUT, combination, info, srcBuffer);
        //
        MY_LOGD("\t[UT]test + ...\n");
        testSingleShot(_local, combination, caller, info, srcBuffer);
        testShot(_local, combination, caller);
        MY_LOGD("\t[UT]test- ...\n");
        //pManager->dump(caller);
        //
        MY_LOGD("\t[UT]test combination + ...\n");
        testCombination(_local, caller);
        MY_LOGD("\t[UT]test combination - ...\n");
        //pManager->dump(caller2);
        //
        flush(caller);
        //
        MY_LOGD("\t[UT]test caller + ...\n");
        testCaller(_local);
        MY_LOGD("\t[UT]test caller - ...\n");
        //pManager->dump(caller2);
        /*testErrorHandle(_local);*/
        MY_LOGD("[UT]finish pure manager test ...\n");
    }
#endif

#if 1
    {
        MY_LOGD("[UT]start fake processor test ...\n");
        buildPipeline();

        {
            Vector<MINT64> combination;
            combination.push_back(MTK_PLUGIN_MODE_MFNR);
            testRAW_YUV(combination);
        }
        {
            Vector<MINT64> combination;
            combination.push_back(MTK_PLUGIN_MODE_NR);
            testYUV_YUV(combination, CASE_WORK_IN_OUT);
        }
        {
            /*Vector<MINT64> combination;
            combination.push_back(MTK_PLUGIN_MODE_NR);
            testYUV_YUV(combination, CASE_DST_IN);*/
        }
        /*testRAW_RAW();
        testBoth();*/

        finishPipelineContext();
        closeSensor();
        MY_LOGD("[UT]finish fake processor test ...\n");
    }
#endif

    clear_global_var();
    NSVendorManager::remove(deviceId);

    MY_LOGD("[VendorManager] end of test\n");
    return 0;
}
