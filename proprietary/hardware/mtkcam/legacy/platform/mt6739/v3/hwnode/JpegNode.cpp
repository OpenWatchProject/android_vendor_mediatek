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

#define LOG_TAG "MtkCam/JpegNode"
//
#include <mtkcam/Log.h>
#include <mtkcam/v3/hwnode/JpegNode.h>
#include "BaseNode.h"
//
#include <utils/RWLock.h>
#include <utils/Thread.h>
//
#include <sys/prctl.h>
#include <sys/resource.h>
//
#include <cutils/properties.h>
//
#include <mtkcam/iopipe/SImager/ISImager.h>
//
#include <vector>
//
#include <mtkcam/metadata/IMetadataProvider.h>
#include <mtkcam/metadata/client/mtk_metadata_tag.h>
#include <mtkcam/v3/hal/mtk_platform_metadata_tag.h>
//
#include "mtkcam/exif/IBaseCamExif.h"
#include <mtkcam/exif/StdExif.h>
//
using namespace android;
using namespace NSCam;
using namespace NSCam::v3;
using namespace NSCam::Utils::Sync;

using namespace NSIoPipe;
using namespace std;

/******************************************************************************
 *
 ******************************************************************************/
#define JPEGTHREAD_NAME ("Cam@Jpeg")
#define JPEGTHREAD_POLICY     (SCHED_OTHER)
#define JPEGTHREAD_PRIORITY   (0)
//
#define DBG_BOUND_WIDTH       (320)
#define DBG_BOUND_HEIGH       (240)
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

#define FUNC_START     MY_LOGD("+")
#define FUNC_END       MY_LOGD("-")


/******************************************************************************
 *
 ******************************************************************************/
static inline
MBOOL
isStream(sp<IStreamInfo> pStreamInfo, StreamId_T streamId )
{
    return pStreamInfo.get() && pStreamInfo->getStreamId() == streamId;
}


/******************************************************************************
 *
 ******************************************************************************/
template <typename T>
inline MBOOL
tryGetMetadata(
    IMetadata const* const pMetadata,
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


/******************************************************************************
 *
 ******************************************************************************/
template <typename T>
inline MVOID
updateEntry(
    IMetadata* pMetadata,
    MUINT32 const tag,
    T const& val
)
{
    if( pMetadata == NULL ) {
        MY_LOGE("pMetadata == NULL");
        return;
    }

    IMetadata::IEntry entry(tag);
    entry.push_back(val, Type2Type<T>());
    pMetadata->update(tag, entry);
}


/******************************************************************************
 *
 ******************************************************************************/
static
MRect
calCropAspect(MSize const& srcSize, MSize const& dstSize)
{
    MRect crop;
#define align2(x) (((x) + 1) & (~0x1))
    MUINT32 val0 = srcSize.w * dstSize.h;
    MUINT32 val1 = srcSize.h * dstSize.w;
    if( val0 > val1 ) {
        crop.s.w = align2(val1 / dstSize.h);
        crop.s.h = srcSize.h;
        crop.p.x = (srcSize.w - crop.s.w) / 2;
        crop.p.y = 0;
    }
    else if ( val0 < val1 ) {
        crop.s.w = srcSize.w;
        crop.s.h = align2(val0 / dstSize.w);
        crop.p.x = 0;
        crop.p.y = (srcSize.h - crop.s.h) / 2;
    }
    else {
        crop = MRect(MPoint(0,0), srcSize);
    }
#undef align2
    return crop;
}


/******************************************************************************
 *
 ******************************************************************************/
class JpegNodeImp
    : public BaseNode
    , public JpegNode
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                                            Definitions.
    typedef android::sp<IPipelineFrame>                     QueNode_T;
    typedef android::List<QueNode_T>                        Que_T;

protected:

    class EncodeThread
        : public Thread
    {

    public:

                                    EncodeThread(JpegNodeImp* pNodeImp)
                                        : mpNodeImp(pNodeImp)
                                    {}

                                    ~EncodeThread()
                                    {}

    public:

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    //  Thread Interface.
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    public:
                    // Ask this object's thread to exit. This function is asynchronous, when the
                    // function returns the thread might still be running. Of course, this
                    // function can be called from a different thread.
                    virtual void        requestExit();

                    // Good place to do one-time initializations
                    virtual status_t    readyToRun();

    private:
                    // Derived class must implement threadLoop(). The thread starts its life
                    // here. There are two ways of using the Thread object:
                    // 1) loop: if threadLoop() returns true, it will be called again if
                    //          requestExit() wasn't called.
                    // 2) once: if threadLoop() returns false, the thread will exit upon return.
                    virtual bool        threadLoop();

    private:

        JpegNodeImp*                  mpNodeImp;

    };

    //
public:     ////                    Operations.

                                    JpegNodeImp();

                                    ~JpegNodeImp();

    virtual MERROR                  config(ConfigParams const& rParams);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IPipelineNode Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Operations.

    virtual MERROR                  init(InitParams const& rParams);

    virtual MERROR                  uninit();

    virtual MERROR                  flush();

    virtual MERROR                  flush(
                                        android::sp<IPipelineFrame> const &pFrame
                                    );

    virtual MERROR                  queue(
                                        android::sp<IPipelineFrame> pFrame
                                    );

protected:  ////                    Operations.
    MERROR                          onDequeRequest( //TODO: check frameNo
                                        android::sp<IPipelineFrame>& rpFrame
                                    );
    MVOID                           onProcessFrame(
                                        android::sp<IPipelineFrame> const& pFrame
                                    );
    MERROR                          verifyConfigParams(
                                        ConfigParams const & rParams
                                    ) const;

    MVOID                           waitForRequestDrained();


    MERROR                          getImageBufferAndLock(
                                        android::sp<IPipelineFrame> const& pFrame,
                                        StreamId_T const streamId,
                                        sp<IImageStreamBuffer>& rpStreamBuffer,
                                        sp<IImageBuffer>& rpImageBuffer
                                    );

    MERROR                          getMetadataAndLock(
                                        android::sp<IPipelineFrame> const& pFrame,
                                        StreamId_T const streamId,
                                        sp<IMetaStreamBuffer>& rpStreamBuffer,
                                        IMetadata*& rpOutMetadataResult
                                    );

    MVOID                           returnMetadataAndUnlock(
                                        android::sp<IPipelineFrame> const& pFrame,
                                        StreamId_T const streamId,
                                        sp<IMetaStreamBuffer>  rpStreamBuffer,
                                        IMetadata* rpOutMetadataResult,
                                        MBOOL success = MTRUE
                                    );

    MBOOL                           isInMetaStream(
                                        StreamId_T const streamId
                                    ) const;

    MBOOL                           isInImageStream(
                                        StreamId_T const streamId
                                    ) const;

    IImageBuffer*                   getJpegBuffer(MBOOL const isMain, MSize const size);

    MVOID                           returnJpegBuffer(MBOOL const isMain, IImageBuffer*& pBuf);

    IImageBuffer*                   allocJpegBuffer(MSize const size) const;

    MVOID                           freeJpegBuffer(IImageBuffer*& pBuf) const;

    MBOOL                           isHwEncodeSupported(int const format) const;

private:    ////                    to sync main yuv & thumbnail yuv

    struct jpeg_params
    {
        // gps related
        IMetadata::IEntry           gpsCoordinates;
        IMetadata::IEntry           gpsProcessingMethod;
        IMetadata::IEntry           gpsTimestamp;
        //
        MINT32                      orientation;
        MUINT8                      quality;
        MUINT8                      quality_thumbnail;
        MSize                       size_thumbnail;
        //
        MRect                       cropRegion;
        //
                                    jpeg_params()
                                        : gpsCoordinates()
                                        , gpsProcessingMethod()
                                        , gpsTimestamp()
                                        //
                                        , orientation(0)
                                        , quality(90)
                                        , quality_thumbnail(90)
                                        , size_thumbnail(0,0)
                                        //
                                        , cropRegion()
                                    {}
    };

    class encode_frame
    {
    public:
        sp<IPipelineFrame> const    mpFrame;
        MBOOL                       mbHasThumbnail;
        MBOOL                       mbSuccess;
        //
        jpeg_params                 mParams;
        //
        sp<IImageBuffer>            mpJpeg_Main;
        sp<IImageBuffer>            mpJpeg_Thumbnail;
        //
        StdExif                     exif;//[++]name
        sp<IImageStreamBuffer>      mpOutImgStreamBuffer;//
        sp<IImageBufferHeap>        mpOutImgBufferHeap;//
                                    //
        IMetadata*                    pInMeta_Hal;
        sp<IMetaStreamBuffer>        pInMetaStream_Hal;
                                    encode_frame(
                                        sp<IPipelineFrame> const pFrame,
                                        MBOOL const hasThumbnail
                                        )
                                        : mpFrame(pFrame)
                                        , mbHasThumbnail(hasThumbnail)
                                        , mbSuccess(MTRUE)
                                        , mpJpeg_Main(NULL)
                                        , mpJpeg_Thumbnail(NULL)
                                        , mpOutImgStreamBuffer(NULL)
                                        , mpOutImgBufferHeap(NULL)
                                        , pInMeta_Hal(NULL)
                                        , pInMetaStream_Hal(NULL)
                                    {}
    };

    MVOID                           updatePartialFrame(
                                        encode_frame* rpEncodeFrame
                                    );

    MERROR                          getPartialFrame(
                                        MUINT32 frameNo,
                                        encode_frame*& rpEncodeFrame
                                    );

    MVOID                           finalizeEncodeFrame(
                                        encode_frame*& rpEncodeFrame
                                    );

    MERROR                          packJpeg(
                                        encode_frame* const rpEncodeFrame,
                                        IImageBuffer* pOutImageBuffer
                                    );

    MVOID                           getJpegParams(
                                        IMetadata* pMetadata_request,
                                        jpeg_params& rParams
                                    ) const;

    MERROR                          getImageBufferAndLock(
                                        android::sp<IPipelineFrame> const& pFrame,
                                        StreamId_T const streamId,
                                        sp<IImageStreamBuffer>& rpStreamBuffer,
                                        sp<IImageBuffer>& rpImageBuffer,
                                        encode_frame*& rpEncodeFrame,
                                        MBOOL const isMain
                                    );

    MVOID                           updateMetadata(
                                        //IMetadata* pMetadata_request,
                                        jpeg_params& rParams,
                                        IMetadata* pMetadata_result
                                    ) const;

    MERROR                          makeExifHeader(
                                        encode_frame* rpEncodeFrame,
                                        MINT8 * const pOutExif,
                                        // [IN/OUT] in: exif buf size, out: exif header size
                                        size_t& rOutExifSize
                                    );

    MVOID                           updateStdExifParam(
                                        MSize const&                rSize,
                                        IMetadata* const            rpHalMeta,
                                        jpeg_params const&          rParams,
                                        ExifParams &                rStdParams
                                    ) const;

    MVOID                           updateStdExifParam_3A(
                                        IMetadata const&            rMeta,
                                        ExifParams &                rStdParams
                                    ) const;

    MVOID                           updateStdExifParam_gps(
                                        IMetadata::IEntry const& rGpsCoordinates,
                                        IMetadata::IEntry const& rGpsProcessingMethod,
                                        IMetadata::IEntry const& rGpsTimestamp,
                                        ExifParams &        rStdParams
                                    ) const;

    MVOID                           updateDebugInfoToExif(
                                        IMetadata* const            rpHalMeta,
                                        StdExif &                   exif
                                    ) const;

    MUINT32                         calcZoomRatio(
                                        MRect      const&   cropRegion,
                                        MSize      const&   rSize
                                    ) const;


    MVOID                           errorMetaHandle(
                                        encode_frame*& rpEncodeFrame
                                    );

    MVOID                           errorHandle(
                                        encode_frame*& rpEncodeFrame
                                    );

    MVOID                           unlockImage(
                                        sp<IImageStreamBuffer>& rpStreamBuffer,
                                        sp<IImageBuffer>& rpImageBuffer
                                    );

protected:  ////                    hw related

    class my_encode_params
    {
        public:
            // buffer
            IImageBuffer*           pSrc;
            IImageBuffer*           pDst;

            // settings
            MUINT32                 transform;
            MRect                   crop;
            MUINT32                 isSOI;
            MUINT32                 quality;
            MUINT32                 codecType;
    };

    MERROR                          hardwareOps_encode(
                                        my_encode_params& rParams
                                    );

    IImageBuffer*                   allocBuffer(
                                        MSize const size,
                                        int const format
                                    ) const;


protected:

    MERROR                          threadSetting();

protected:  ////                    Data Members. (Config)
    mutable RWLock                  mConfigRWLock;
    // meta
    sp<IMetaStreamInfo>             mpInAppMeta;
    sp<IMetaStreamInfo>             mpInHalMeta;
    sp<IMetaStreamInfo>             mpOutMetaStreamInfo_Result;

    // image
    sp<IImageStreamInfo>            mpInYuv_main;
    sp<IImageStreamInfo>            mpInYuv_thumbnail;
    sp<IImageStreamInfo>            mpOutJpeg;

protected:  ////                    Data Members. (Request Queue)
    mutable Mutex                   mRequestQueueLock;
    Condition                       mRequestQueueCond;
    Que_T                           mRequestQueue;
    MBOOL                           mbRequestDrained;
    Condition                       mbRequestDrainedCond;
    MBOOL                           mbRequestExit;

private:   ////                     Threads
    sp<EncodeThread>                mpEncodeThread;

private:
    mutable Mutex                   mEncodeLock;
    MBOOL                           mbIsEncoding;
    MUINT32                         muCurFrameNo;
    // should be only accessed by encode thread
    vector<IImageBuffer*>           mvWorkingJpeg_Main;
    vector<IImageBuffer*>           mvWorkingJpeg_Thumbnail;
    //
    encode_frame*                   mpCurEncFrame;

private: // static infos

    MUINT8                          muFacing; // ref: MTK_LENS_FACING_
    MRect                           mActiveArray;
    MBOOL                           mJpegRotationEnable;
    MBOOL                           mDbgInfoEnable;
};


/******************************************************************************
 *
 ******************************************************************************/
android::sp<JpegNode>
JpegNode::
createInstance()
{
    return new JpegNodeImp();
}


/******************************************************************************
 *
 ******************************************************************************/
JpegNodeImp::
JpegNodeImp()
    : BaseNode()
    , JpegNode()
    //
    , mConfigRWLock()
    //
    , mbRequestDrained(MFALSE)
    , mbRequestExit(MFALSE)
    //
    , mpEncodeThread(NULL)
    //
    , mbIsEncoding(MFALSE)
    , muCurFrameNo(0)
    //
    , mpCurEncFrame(NULL)
    , muFacing(0)
    //
    , mJpegRotationEnable(MFALSE)
{
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.jpeg.rotation.enable", value, "1");
    int32_t enable = atoi(value);
    mJpegRotationEnable = (enable & 0x1)? MTRUE : MFALSE;
    MY_LOGD("Jpeg Rotation enable: %d", mJpegRotationEnable);
    mDbgInfoEnable= ::property_get_int32("debug.camera.dbginfo", 1);
}


/******************************************************************************
 *
 ******************************************************************************/
JpegNodeImp::
~JpegNodeImp()
{
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
JpegNodeImp::
init(InitParams const& rParams)
{
    FUNC_START;
    //
    mOpenId = rParams.openId;
    mNodeId = rParams.nodeId;
    mNodeName = rParams.nodeName;
    //
    MY_LOGD("OpenId %d, nodeId %#" PRIxPTR ", name %s",
            getOpenId(), getNodeId(), getNodeName() );
    //
    mpEncodeThread = new EncodeThread(this);
    if( mpEncodeThread->run(JPEGTHREAD_NAME) != OK ) {
        return UNKNOWN_ERROR;
    }
    //
    {
        sp<IMetadataProvider> pMetadataProvider = NSMetadataProviderManager::valueFor(getOpenId());
        if( ! pMetadataProvider.get() ) {
            MY_LOGE(" ! pMetadataProvider.get() ");
            return DEAD_OBJECT;
        }

        IMetadata static_meta = pMetadataProvider->getMtkStaticCharacteristics();
        if( !tryGetMetadata<MRect>(
                    &static_meta, MTK_SENSOR_INFO_ACTIVE_ARRAY_REGION,
                    mActiveArray)
          ) {
            MY_LOGE("no static info: MTK_SENSOR_INFO_ACTIVE_ARRAY_REGION");
            return UNKNOWN_ERROR;
        }

        if( !tryGetMetadata<MUINT8>(
                    &static_meta, MTK_SENSOR_INFO_FACING,
                    muFacing)
          ) {
            MY_LOGE("no static info: MTK_SENSOR_INFO_FACING");
            return UNKNOWN_ERROR;
        }

        MY_LOGD_IF(1,"active array(%d, %d, %dx%d), facing %d",
                mActiveArray.p.x, mActiveArray.p.y, mActiveArray.s.w, mActiveArray.s.h,
                muFacing);
    }
    //
    FUNC_END;
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
JpegNodeImp::
config(ConfigParams const& rParams)
{
    FUNC_START;
    {
        MERROR const err = verifyConfigParams(rParams);
        if( err != OK ) {
            MY_LOGE("verifyConfigParams failed, err = %d", err);
            return err;
        }
    }
    //
    flush();
    //
    {
        RWLock::AutoWLock _l(mConfigRWLock);
        // meta
        mpInAppMeta       = rParams.pInAppMeta;
        mpInHalMeta       = rParams.pInHalMeta;
        mpOutMetaStreamInfo_Result = rParams.pOutAppMeta;
        // image
        mpInYuv_main      = rParams.pInYuv_Main;
        mpInYuv_thumbnail = rParams.pInYuv_Thumbnail;
        mpOutJpeg         = rParams.pOutJpeg;
        //
    }
    MY_LOGD("mpInYuv_main:%dx%d mpInYuv_thumbnail:%dx%d",
        mpInYuv_main->getImgSize().w, mpInYuv_main->getImgSize().h,
        mpInYuv_thumbnail->getImgSize().w, mpInYuv_thumbnail->getImgSize().h);
    //
    FUNC_END;
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
JpegNodeImp::
uninit()
{
    FUNC_START;
    //
    if ( OK != flush() )
        MY_LOGE("flush failed");
    //
    // exit threads
    mpEncodeThread->requestExit();
    // join
    mpEncodeThread->join();
    //
    mpEncodeThread = NULL;
    //
    FUNC_END;
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
JpegNodeImp::
flush(android::sp<IPipelineFrame> const &pFrame)
{
    return BaseNode::flush(pFrame);
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
JpegNodeImp::
flush()
{
    FUNC_START;
    //
    // 1. clear requests
    {
        Mutex::Autolock _l(mRequestQueueLock);
        //
        Que_T::iterator it = mRequestQueue.begin();
        while ( it != mRequestQueue.end() ) {
            BaseNode::flush(*it);
            it = mRequestQueue.erase(it);
        }
    }
    //
    // 2. wait enque thread
    waitForRequestDrained();
    //
    // 3. clear working buffer
#if 0
    {
        for(size_t i = 0; i < mvWorkingJpeg_Main.size(); i++) {
            freeJpegBuffer(mvWorkingJpeg_Main[i]);
        }
        mvWorkingJpeg_Main.clear();
        //
        for(size_t i = 0; i < mvWorkingJpeg_Thumbnail.size(); i++) {
            freeJpegBuffer(mvWorkingJpeg_Thumbnail[i]);
        }
        mvWorkingJpeg_Thumbnail.clear();
    }
#endif
    //
    FUNC_END;
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
JpegNodeImp::
queue(android::sp<IPipelineFrame> pFrame)
{
    FUNC_START;
    //
    if( ! pFrame.get() ) {
        MY_LOGE("Null frame");
        return BAD_VALUE;
    }

    Mutex::Autolock _l(mRequestQueueLock);
    // TODO: handle main & thumnail yuvs are not queued in the same time
    //if( !mbIsEncoding || (mbIsEncoding && pFrame->getFrameNo() != muCurFrameNo) {
        //  Make sure the request with a smaller frame number has a higher priority.
        Que_T::iterator it = mRequestQueue.end();
        for (; it != mRequestQueue.begin(); ) {
            --it;
            if  ( 0 <= (MINT32)(pFrame->getFrameNo() - (*it)->getFrameNo()) ) {
                ++it;   //insert(): insert before the current node
                break;
            }
        }
        mRequestQueue.insert(it, pFrame);
    //}
    //else {

    //}
    mRequestQueueCond.signal();
    //
    FUNC_END;
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
JpegNodeImp::
onDequeRequest(
    android::sp<IPipelineFrame>& rpFrame
)
{
    FUNC_START;
    MY_LOGD("mRequestQueue.size:%zu", mRequestQueue.size());
    Mutex::Autolock _l(mRequestQueueLock);

    //  Wait until the queue is not empty or not going exit
    while ( mRequestQueue.empty() && ! mbRequestExit )
    {
        // set dained flag
        mbRequestDrained = MTRUE;
        mbRequestDrainedCond.signal();
        //
        status_t status = mRequestQueueCond.wait(mRequestQueueLock);
        if  ( OK != status ) {
            MY_LOGW(
                "wait status:%d:%s, mRequestQueue.size:%zu",
                status, ::strerror(-status), mRequestQueue.size()
            );
        }
    }

    if  ( mbRequestExit ) {
        MY_LOGW_IF(!mRequestQueue.empty(), "[flush] mRequestQueue.size:%zu", mRequestQueue.size());
        return DEAD_OBJECT;
    }

    //  Here the queue is not empty, take the first request from the queue.
    mbRequestDrained = MFALSE;
    rpFrame = *mRequestQueue.begin();
    mRequestQueue.erase(mRequestQueue.begin());
    //
    FUNC_END;
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
JpegNodeImp::
onProcessFrame(
    android::sp<IPipelineFrame> const& pFrame
)
{
    //TODO: error handling in this function
    FUNC_START;
    //
    IPipelineFrame::InfoIOMapSet IOMapSet;
    if(
            OK != pFrame->queryInfoIOMapSet( getNodeId(), IOMapSet )
            || IOMapSet.mImageInfoIOMapSet.size() != 1
            || IOMapSet.mMetaInfoIOMapSet.size() != 1
      ) {
        MY_LOGE("queryInfoIOMap failed, IOMap img/meta: %zu/%zu",
                IOMapSet.mImageInfoIOMapSet.size(),
                IOMapSet.mMetaInfoIOMapSet.size()
                );
        return;
    }

    encode_frame* pEncodeFrame = NULL;
    {
        MERROR ret = getPartialFrame(pFrame->getFrameNo(), pEncodeFrame);
        if( ret == NAME_NOT_FOUND ) {
            //
            MBOOL useThumbnail = MFALSE;

            // query if use thumbnail
            IPipelineFrame::ImageInfoIOMap const& imageIOMap = IOMapSet.mImageInfoIOMapSet[0];
            for( size_t i = 0; i < imageIOMap.vIn.size(); i++ )
            {
                StreamId_T const streamId = imageIOMap.vIn.keyAt(i);
                if( isStream(mpInYuv_thumbnail, streamId) ) {
                    useThumbnail = MTRUE;
                    break;
                }
            }

            // new frame
            pEncodeFrame = new encode_frame(pFrame, useThumbnail);

            // get jpeg params
            if( pEncodeFrame ) {
                IMetadata* pInMeta_Request = NULL;
                sp<IMetaStreamBuffer> pInMetaStream_Request = NULL;

                MERROR const err = getMetadataAndLock(
                        pFrame,
                        mpInAppMeta->getStreamId(),
                        pInMetaStream_Request,
                        pInMeta_Request
                        );

                if( err != OK ) {
                    MY_LOGE("getMetadataAndLock err = %d", err);
                    errorMetaHandle(pEncodeFrame);
                    delete pEncodeFrame;
                    pEncodeFrame = NULL;
                    return;
                }

                getJpegParams(pInMeta_Request, pEncodeFrame->mParams);

                returnMetadataAndUnlock(
                        pFrame,
                        mpInAppMeta->getStreamId(),
                        pInMetaStream_Request,
                        pInMeta_Request
                        );
            }
            //pEncodeFrame->mParams.size_thumbnail.w = 176;
            //pEncodeFrame->mParams.size_thumbnail.h = 99;
        }

        if( pEncodeFrame == NULL ) {
            MY_LOGE("pEncodeFrame == NULL");
            return;
        }
    }

    // 2. get src buffers & internal dst buffer for bitstream
    while( pEncodeFrame->mpJpeg_Main.get() == NULL ) {
        //[++]get out buffer and lock
        {
            IStreamBufferSet&      streamBufferSet      = pFrame->getStreamBufferSet();
            StreamId_T const       stream_in            = mpOutJpeg->getStreamId();
            sp<IImageStreamBuffer>& pOutImgStreamBuffer = pEncodeFrame->mpOutImgStreamBuffer;
            sp<IImageBuffer>       pOutImageBuffer      = NULL;
            //
            MERROR const err = getImageBufferAndLock(
                    pFrame,
                    stream_in,
                    pOutImgStreamBuffer,
                    pOutImageBuffer,
                    pEncodeFrame,
                    MTRUE//is main
                    );
            if( err == NAME_NOT_FOUND ) {
                MY_LOGD("cannot find stream %#" PRIx64 ", skip it",
                        stream_in
                        );
                break;
            } else if( err != OK ) {
                MY_LOGE("getImageBufferAndLock err = %d", err);
                errorHandle(pEncodeFrame);
                delete pEncodeFrame;
                pEncodeFrame = NULL;
                return;
            }
            //
            //remember main&thumb buffer
            pEncodeFrame->mpJpeg_Main = pOutImageBuffer;
        }

        // main jpeg is not encoded, try get yuv for main jpeg
        IStreamBufferSet&      streamBufferSet      = pFrame->getStreamBufferSet();
        StreamId_T const       stream_in            = mpInYuv_main->getStreamId();
        sp<IImageStreamBuffer> pInImageStreamBuffer = NULL;
        sp<IImageBuffer>       pInImageBuffer       = NULL;
        //
        MERROR const err = getImageBufferAndLock(
                pFrame,
                stream_in,
                pInImageStreamBuffer,
                pInImageBuffer
                );
        if( err == NAME_NOT_FOUND ) {
            MY_LOGD("cannot find stream %#" PRIx64 ", skip it",
                    stream_in
                    );
            break;
        } else if( err != OK ) {
            MY_LOGE("getImageBufferAndLock err = %d", err);
            unlockImage(pEncodeFrame->mpOutImgStreamBuffer, pEncodeFrame->mpJpeg_Main);
            errorHandle(pEncodeFrame);
            delete pEncodeFrame;
            pEncodeFrame = NULL;
            return;
        }
        if(pEncodeFrame->mpOutImgBufferHeap == NULL)
            MY_LOGE("@@pEncodeFrame->mpOutImgBufferHeap == NULL");

        // do encode
        {
            my_encode_params params;
            params.pSrc = pInImageBuffer.get();
            params.pDst = pEncodeFrame->mpJpeg_Main.get();
            params.transform = 0; //TODO
            params.crop = MRect(MPoint(0,0), pInImageBuffer->getImgSize());
            params.isSOI = 0;
            params.quality = pEncodeFrame->mParams.quality;
            params.codecType = isHwEncodeSupported(pInImageBuffer->getImgFormat()) ?
                    NSSImager::JPEGENC_HW_FIRST : NSSImager::JPEGENC_SW;
            //
            MERROR const err = hardwareOps_encode(params);
            if( err != OK ) {
                pEncodeFrame->mbSuccess = MFALSE;
            }
            MY_LOGD("main bistream size %zu", pEncodeFrame->mpJpeg_Main->getBitstreamSize());
        }
        {
            bool shouldPrint = false;
            if (shouldPrint) {
                char filename[256];
                sprintf(filename, "/sdcard/DCIM/Camera/mainYUV_%d_%d_%d_%d.yuv", pFrame->getFrameNo(),
                    pInImageBuffer->getImgSize().w,
                    pInImageBuffer->getImgSize().h,
                    pEncodeFrame->mParams.orientation);
                NSCam::Utils::saveBufToFile(filename, (unsigned char*)pInImageBuffer->getBufVA(0), pInImageBuffer->getBufSizeInBytes(0));
                shouldPrint = false;
            }
        }

        //
        pInImageBuffer->unlockBuf(getNodeName());
        pInImageStreamBuffer->unlock(getNodeName(), pInImageBuffer->getImageBufferHeap());
        //
        streamBufferSet.markUserStatus(
                pInImageStreamBuffer->getStreamInfo()->getStreamId(),
                getNodeId(),
                IUsersManager::UserStatus::USED |
                IUsersManager::UserStatus::RELEASE
                );

        // 3. end
        {
            size_t const totalJpegSize =
                pEncodeFrame->mpJpeg_Main->getBitstreamSize() + pEncodeFrame->exif.getHeaderSize();
            pEncodeFrame->mpJpeg_Main->getImageBufferHeap()->setBitstreamSize(totalJpegSize);
        }
        pEncodeFrame->mpJpeg_Main->unlockBuf(getNodeName());
        MY_LOGD("@@Main encode end");
        break;
    }

    while( pEncodeFrame->mbHasThumbnail && pEncodeFrame->mpJpeg_Thumbnail.get() == NULL ) {
        //[++]get out buffer and lock
        {
            IStreamBufferSet&      streamBufferSet      = pFrame->getStreamBufferSet();
            StreamId_T const       stream_in            = mpOutJpeg->getStreamId();
            sp<IImageStreamBuffer>& pOutImgStreamBuffer = pEncodeFrame->mpOutImgStreamBuffer;
            sp<IImageBuffer>       pOutImageBuffer      = NULL;
            //
            MERROR const err = getImageBufferAndLock(
                    pFrame,
                    stream_in,
                    pOutImgStreamBuffer,
                    pOutImageBuffer,
                    pEncodeFrame,
                    MFALSE//is Thumbnail
                    );
            if( err == NAME_NOT_FOUND ) {
                MY_LOGD("cannot find stream %#" PRIx64 ", skip it",
                        stream_in
                        );
                break;
            } else if( err != OK ) {
                MY_LOGE("getImageBufferAndLock err = %d", err);
                errorHandle(pEncodeFrame);
                delete pEncodeFrame;
                pEncodeFrame = NULL;
                return;
            }
            //
            //remember main&thumb buffer(sp<IImageBuffer>)
            pEncodeFrame->mpJpeg_Thumbnail = pOutImageBuffer;
        }

        // thumbnail jpeg is not encoded, try get yuv for main jpeg
        IStreamBufferSet&      streamBufferSet      = pFrame->getStreamBufferSet();
        StreamId_T const       stream_in            = mpInYuv_thumbnail->getStreamId();
        sp<IImageStreamBuffer> pInImageStreamBuffer = NULL;
        sp<IImageBuffer>       pInImageBuffer       = NULL;
        //
        MERROR const err = getImageBufferAndLock(
                pFrame,
                stream_in,
                pInImageStreamBuffer,
                pInImageBuffer
                );
        if( err == NAME_NOT_FOUND ) {
            MY_LOGD("cannot find stream %#" PRIx64 ", skip it",
                    stream_in
                   );
            break;
        } else if( err != OK ) {
            MY_LOGE("getImageBufferAndLock(pInImageStreamBuffer) err = %d", err);
            unlockImage(pEncodeFrame->mpOutImgStreamBuffer, pEncodeFrame->mpJpeg_Thumbnail);
            errorHandle(pEncodeFrame);
            delete pEncodeFrame;
            pEncodeFrame = NULL;
            return;
        }
        //
        MSize const thumbsize = pEncodeFrame->mParams.size_thumbnail;

        // do encode
        {
            //
            my_encode_params params;
            params.pSrc = pInImageBuffer.get();
            params.pDst = pEncodeFrame->mpJpeg_Thumbnail.get();
            params.transform = 0; //TODO
            params.crop = calCropAspect(pInImageBuffer->getImgSize(), thumbsize);
            params.isSOI = 1;
            params.quality = pEncodeFrame->mParams.quality_thumbnail;
            params.codecType = NSSImager::JPEGENC_SW;
            //
            MERROR const err = hardwareOps_encode(params);
            if( err != OK ) {
                pEncodeFrame->mbSuccess = MFALSE;
            }
        }
        //
        {
            bool shouldPrint = false;
            if (shouldPrint) {
                char filename[256];
                sprintf(filename, "/sdcard/DCIM/Camera/thumbnailYUV_%d_%d_%d_%d.yuv", pFrame->getFrameNo(),
                    pInImageBuffer->getImgSize().w,
                    pInImageBuffer->getImgSize().h,
                    pEncodeFrame->mParams.orientation);
                NSCam::Utils::saveBufToFile(filename, (unsigned char*)pInImageBuffer->getBufVA(0), pInImageBuffer->getBufSizeInBytes(0));
                shouldPrint = false;
            }
        }

        pInImageBuffer->unlockBuf(getNodeName());
        pInImageStreamBuffer->unlock(getNodeName(), pInImageBuffer->getImageBufferHeap());
        //
        streamBufferSet.markUserStatus(
                pInImageStreamBuffer->getStreamInfo()->getStreamId(),
                getNodeId(),
                IUsersManager::UserStatus::USED |
                IUsersManager::UserStatus::RELEASE
                );

        pEncodeFrame->mpJpeg_Thumbnail->unlockBuf(getNodeName());
        MY_LOGD("@@Thumbnail encode end");
    }

    // 4. if no thumbnail, copy to dst buffer & release buffers/metadata
    //    else add to pending list to wait for the other src buffer
    if(
            // condition 1: without thumbnail
            (!pEncodeFrame->mbHasThumbnail && pEncodeFrame->mpJpeg_Main.get()) ||
            // condition 2: with thumbnail
            (pEncodeFrame->mbHasThumbnail && pEncodeFrame->mpJpeg_Main.get() && pEncodeFrame->mpJpeg_Thumbnail.get() )
      ) {
        finalizeEncodeFrame(pEncodeFrame);
        //
        delete pEncodeFrame;
        pEncodeFrame = NULL;
    }
    else {
        updatePartialFrame(pEncodeFrame);
    }


    FUNC_END;
    return;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
JpegNodeImp::
verifyConfigParams(
    ConfigParams const & rParams
) const
{
    if  ( ! rParams.pInAppMeta.get() ) {
        MY_LOGE("no in app meta");
        return BAD_VALUE;
    }
    //if  ( ! rParams.pInHalMeta.get() ) {
    //    MY_LOGE("no in hal meta");
    //    return BAD_VALUE;
    //}
    if  ( ! rParams.pOutAppMeta.get() ) {
        MY_LOGE("no out app meta");
        return BAD_VALUE;
    }
    if  (  NULL == rParams.pInYuv_Main.get() ) {
        MY_LOGE("no in hal main yuv image");
        return BAD_VALUE;
    }
    //if  (  NULL == rParams.pInYuv_Thumbnail.get() ) {
    //    MY_LOGE("no in hal thumbnail yuv image");
    //    return BAD_VALUE;
    //}

    if  ( ! rParams.pOutJpeg.get() ) {
        MY_LOGE("no out hal jpeg image");
        return BAD_VALUE;
    }
    //
    MY_LOGD_IF(1,
            "stream: [meta] in app %#" PRIx64 ", out app %#" PRIx64,
            rParams.pInAppMeta->getStreamId(),
            rParams.pOutAppMeta->getStreamId()
            );
    MY_LOGD_IF( rParams.pInHalMeta.get(),
            "stream: [meta] in hal %#" PRIx64,
            rParams.pInHalMeta->getStreamId()
            );
    MY_LOGD_IF(rParams.pInYuv_Main.get(),
            "stream: [img] in main %#" PRIx64,
            rParams.pInYuv_Main->getStreamId()
            );
    MY_LOGD_IF(rParams.pInYuv_Thumbnail.get(),
            "stream: [img] in thumbnail %#" PRIx64,
            rParams.pInYuv_Thumbnail->getStreamId()
            );
    MY_LOGD_IF(rParams.pOutJpeg.get(),
            "stream: [img] out jpeg %#" PRIx64,
            rParams.pOutJpeg->getStreamId()
            );
    //
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
JpegNodeImp::
waitForRequestDrained()
{
    FUNC_START;
    //
    Mutex::Autolock _l(mRequestQueueLock);
    if( !mbRequestDrained ) {
        MY_LOGD("wait for request drained");
        mbRequestDrainedCond.wait(mRequestQueueLock);
    }
    //
    FUNC_END;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
JpegNodeImp::
getImageBufferAndLock(
    android::sp<IPipelineFrame> const& pFrame,
    StreamId_T const streamId,
    sp<IImageStreamBuffer>& rpStreamBuffer,
    sp<IImageBuffer>& rpImageBuffer
)
{
    IStreamBufferSet&      rStreamBufferSet = pFrame->getStreamBufferSet();
    sp<IImageBufferHeap>   pImageBufferHeap = NULL;
    MERROR const err = ensureImageBufferAvailable_(
            pFrame->getFrameNo(),
            streamId,
            rStreamBufferSet,
            rpStreamBuffer
            );

    if( err != OK )
        return err;

    //  Query the group usage.
    MUINT const groupUsage = rpStreamBuffer->queryGroupUsage(getNodeId());

    pImageBufferHeap = isInImageStream(streamId) ?
        rpStreamBuffer->tryReadLock(getNodeName()) :
        rpStreamBuffer->tryWriteLock(getNodeName());

    if (pImageBufferHeap == NULL) {
        MY_LOGE("pImageBufferHeap == NULL");
        return BAD_VALUE;
    }

    rpImageBuffer = pImageBufferHeap->createImageBuffer();
    if (rpImageBuffer == NULL) {
        //rpStreamBuffer->unlock(getNodeName(), pImageBufferHeap);
        MY_LOGE("rpImageBuffer == NULL");
        return BAD_VALUE;
    }
    rpImageBuffer->lockBuf(getNodeName(), groupUsage);

    MY_LOGD("stream buffer: (%#" PRIx64 ") %p, heap: %p, buffer: %p, usage: 0x%x",
        streamId, rpStreamBuffer.get(), pImageBufferHeap.get(), rpImageBuffer.get(), groupUsage);

    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
JpegNodeImp::
getMetadataAndLock(
    android::sp<IPipelineFrame> const& pFrame,
    StreamId_T const streamId,
    sp<IMetaStreamBuffer>& rpStreamBuffer,
    IMetadata*& rpMetadata
)
{
    IStreamBufferSet&      rStreamBufferSet = pFrame->getStreamBufferSet();
    MY_LOGD("nodeID %#" PRIxPTR " streamID %#" PRIx64 ,getNodeId(), streamId);
    MERROR const err = ensureMetaBufferAvailable_(
            pFrame->getFrameNo(),
            streamId,
            rStreamBufferSet,
            rpStreamBuffer
            );
    MY_LOGD_IF(rpStreamBuffer==NULL," rpStreamBuffer==NULL");

    if( err != OK )
        return err;

    rpMetadata = isInMetaStream(streamId) ?
        rpStreamBuffer->tryReadLock(getNodeName()) :
        rpStreamBuffer->tryWriteLock(getNodeName());
    if( rpMetadata == NULL ) {
        MY_LOGE("[frame:%d node:%#" PRIxPTR "][stream buffer:%s] cannot get metadata",
                pFrame->getFrameNo(), getNodeId(), rpStreamBuffer->getName());
        return BAD_VALUE;
    }

    MY_LOGD_IF(1,"stream %#" PRIx64 ": stream buffer %p, metadata: %p",
        streamId, rpStreamBuffer.get(), rpMetadata);

    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
JpegNodeImp::
returnMetadataAndUnlock(
    android::sp<IPipelineFrame> const& pFrame,
    StreamId_T const streamId,
    sp<IMetaStreamBuffer>  rpStreamBuffer,
    IMetadata* rpMetadata,
    MBOOL success
)
{
    IStreamBufferSet&     rStreamBufferSet = pFrame->getStreamBufferSet();
    //
    if( rpStreamBuffer.get() == NULL ) {
        MY_LOGE("StreamId %#" PRIx64 ": rpStreamBuffer == NULL",
                streamId);
        return;
    }
    //
    //Buffer Producer must set this status.
    if( !isInMetaStream(streamId) ) {
        if  ( success ) {
            rpStreamBuffer->markStatus(STREAM_BUFFER_STATUS::WRITE_OK);
        }
        else {
            rpStreamBuffer->markStatus(STREAM_BUFFER_STATUS::WRITE_ERROR);
        }
    }
    //
    if( rpMetadata )
        rpStreamBuffer->unlock(getNodeName(), rpMetadata);
    //
    //  Mark this buffer as USED by this user.
    //  Mark this buffer as RELEASE by this user.
    rStreamBufferSet.markUserStatus(
            streamId, getNodeId(),
            IUsersManager::UserStatus::USED |
            IUsersManager::UserStatus::RELEASE
            );
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
JpegNodeImp::
isInMetaStream(
    StreamId_T const streamId
) const
{
    RWLock::AutoRLock _l(mConfigRWLock);
    return isStream(mpInAppMeta, streamId) || isStream(mpInHalMeta, streamId);
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
JpegNodeImp::
isInImageStream(
    StreamId_T const streamId
) const
{
    RWLock::AutoRLock _l(mConfigRWLock);
    //
    if( isStream(mpInYuv_main, streamId) )
        return MTRUE;
    //
    if( isStream(mpInYuv_thumbnail, streamId) )
        return MTRUE;
    //
    MY_LOGD_IF(0, "stream id %#" PRIx64 " is not in-stream", streamId);
    return MFALSE;
}


/******************************************************************************
 *
 ******************************************************************************/
IImageBuffer*
JpegNodeImp::
getJpegBuffer(MBOOL const isMain, MSize const size)
{
#define tryGetOrAllocBuf( vWorkingJpeg, rpBuffer, rSize )             \
    do{                                                               \
        /* find buffer with same size */                              \
        vector<IImageBuffer*>::iterator ppBuf = vWorkingJpeg.begin(); \
        while( ppBuf != vWorkingJpeg.end() ) {                        \
            if( (*ppBuf)->getImgSize() != rSize ) {                   \
                MY_LOGD("buf size %dx%d != %dx%d",                    \
                        (*ppBuf)->getImgSize().w,                     \
                        (*ppBuf)->getImgSize().h,                     \
                        rSize.w, rSize.h);                            \
                freeJpegBuffer(*ppBuf);                               \
                ppBuf = vWorkingJpeg.erase(ppBuf);                    \
            } else {                                                  \
                rpBuffer = *ppBuf;                                    \
                ppBuf = vWorkingJpeg.erase(ppBuf);                    \
                break;                                                \
            }                                                         \
        }                                                             \
        if( rpBuffer == NULL ) {                                      \
            rpBuffer = allocJpegBuffer(rSize);                        \
        }                                                             \
    }                                                                 \
    while(0)

    IImageBuffer* pBuf = NULL;

    if( isMain ) {
        tryGetOrAllocBuf( mvWorkingJpeg_Main, pBuf, size );
    } else {
        tryGetOrAllocBuf( mvWorkingJpeg_Thumbnail, pBuf, size );
    }

    if( pBuf ) {
        MUINT const usage = eBUFFER_USAGE_SW_MASK |
            eBUFFER_USAGE_HW_CAMERA_READWRITE
            ;
        MY_LOGW("[FIXME] lock usage 0x%x", usage);
        pBuf->lockBuf(getNodeName(), usage);
    }
    return pBuf;
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
JpegNodeImp::
returnJpegBuffer(MBOOL const isMain, IImageBuffer*& pBuf)
{
#define MAX_NUM_WORKING_BUFFER    (1)
#define returnToQueueAndFreeSomeBuffers( vWorkingJpeg, rpBuffer )              \
    do{                                                                        \
        MINT32 toFreeCount = vWorkingJpeg.size() + 1 - MAX_NUM_WORKING_BUFFER; \
        if( toFreeCount > 0 ) {                                                \
            for( MINT32 i = 0; i < toFreeCount; i++ ) {                        \
                freeJpegBuffer(vWorkingJpeg[i]);                               \
            }                                                                  \
            vWorkingJpeg.erase(                                                \
                    vWorkingJpeg.begin(),                                      \
                    vWorkingJpeg.begin() + toFreeCount                         \
                    );                                                         \
        }                                                                      \
        vWorkingJpeg.push_back(pBuf);                                          \
    }while(0)
    //
    if( pBuf == NULL ) {
        MY_LOGE("pBuf == NULL");
        return;
    }
    //
    pBuf->unlockBuf(getNodeName());

    if( isMain ) {
        returnToQueueAndFreeSomeBuffers( mvWorkingJpeg_Main, pBuf );
    } else {
        returnToQueueAndFreeSomeBuffers( mvWorkingJpeg_Thumbnail, pBuf );
    }
    pBuf = NULL;
}


/******************************************************************************
 *
 ******************************************************************************/
IImageBuffer*
JpegNodeImp::
allocJpegBuffer(MSize const size) const
{
#define calcJpegSize(w,h)    ((w)*(h)*6u/5u)

    IImageBuffer* pBuf = NULL;

    IImageBufferAllocator::ImgParam imgParam(
            size,
            calcJpegSize(size.w,size.h),
            0
            );

    IImageBufferAllocator* allocator = IImageBufferAllocator::getInstance();
    pBuf = allocator->alloc(LOG_TAG, imgParam);
    //if( pBuf == NULL ) {
    //    MY_LOGE("alloc buf failed, %dx%d",
    //            size.w, size.h);
    //}

    MY_LOGD("alloc %dx%d: %p", size.w, size.h, pBuf);
    return pBuf;
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
JpegNodeImp::
freeJpegBuffer(IImageBuffer*& pBuf) const
{
    if( pBuf == NULL )
        return;

    MY_LOGD("free %dx%d: %p", pBuf->getImgSize().w, pBuf->getImgSize().h, pBuf);
    IImageBufferAllocator* allocator = IImageBufferAllocator::getInstance();
    allocator->free(pBuf);
    pBuf = NULL;
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
JpegNodeImp::
updatePartialFrame(
        encode_frame* rpEncodeFrame
)
{
    MY_LOGD("encode_frame no %u: %p",
            rpEncodeFrame->mpFrame->getFrameNo(), rpEncodeFrame);

    if( mpCurEncFrame ) {
        MY_LOGE("wrong encode_frame cur %p ,new %p",
                mpCurEncFrame, rpEncodeFrame);
        return;
    }
    mpCurEncFrame = rpEncodeFrame;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
JpegNodeImp::
getPartialFrame(
    MUINT32 frameNo,
    encode_frame*& rpEncodeFrame
)
{
    if( mpCurEncFrame ) {
        if( mpCurEncFrame->mpFrame->getFrameNo() != frameNo ) {
            MY_LOGE("wrong frameNo cur %u != new %u",
                    mpCurEncFrame->mpFrame->getFrameNo(), frameNo);
            return BAD_VALUE;
        }
        rpEncodeFrame = mpCurEncFrame;
        mpCurEncFrame = NULL;
        return OK;
    }
    return NAME_NOT_FOUND;
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
JpegNodeImp::
finalizeEncodeFrame(
    encode_frame*& rpEncodeFrame
)
{
    sp<IPipelineFrame> const& pFrame = rpEncodeFrame->mpFrame;
    IStreamBufferSet& streamBufferSet = pFrame->getStreamBufferSet();

    // update metadata
    {
        IMetadata* pOutMeta_Result = NULL;
        sp<IMetaStreamBuffer> pOutMetaStream_Result = NULL;

        MERROR const err = getMetadataAndLock(
                pFrame,
                mpOutMetaStreamInfo_Result->getStreamId(),
                pOutMetaStream_Result,
                pOutMeta_Result
                );

        updateMetadata(rpEncodeFrame->mParams, pOutMeta_Result);

        returnMetadataAndUnlock(
                pFrame,
                mpOutMetaStreamInfo_Result->getStreamId(),
                pOutMetaStream_Result,
                pOutMeta_Result,
                rpEncodeFrame->mbSuccess
                );
    }

    // get out buffer
    {
        //
        sp<IImageStreamBuffer> pOutImgStreamBuffer = rpEncodeFrame->mpOutImgStreamBuffer;//
        sp<IImageBuffer>       pOutImageBuffer     = NULL;

        //use heap to create ImageBuffer
        pOutImageBuffer = rpEncodeFrame->mpOutImgBufferHeap->createImageBuffer_FromBlobHeap(0, rpEncodeFrame->exif.getHeaderSize());

        if (pOutImageBuffer.get() == NULL) {
            MY_LOGE("rpImageBuffer == NULL");
        }
        MUINT const groupUsage = pOutImgStreamBuffer->queryGroupUsage(getNodeId());
        pOutImageBuffer->lockBuf(getNodeName(), groupUsage);

        //[++]make exif header
        size_t exifSize  = rpEncodeFrame->exif.getHeaderSize();
        MINT8 * pExifBuf = reinterpret_cast<MINT8*>(pOutImageBuffer->getBufVA(0));
        if( pExifBuf == NULL
        || OK != makeExifHeader(rpEncodeFrame, pExifBuf, exifSize)
           ) {
            rpEncodeFrame->mbSuccess = MFALSE;
            MY_LOGE("frame %u make exif header failed: buf %p, size %zu",
                    rpEncodeFrame->mpFrame->getFrameNo(),
                    pExifBuf, exifSize);
        }
        if( pExifBuf )
            pExifBuf = NULL;

        //
        pOutImageBuffer->unlockBuf(getNodeName());
        pOutImgStreamBuffer->unlock(getNodeName(), pOutImageBuffer->getImageBufferHeap());
        pOutImgStreamBuffer->markStatus(
                rpEncodeFrame->mbSuccess ?
                STREAM_BUFFER_STATUS::WRITE_OK : STREAM_BUFFER_STATUS::WRITE_ERROR
                );

        //  Mark this buffer as USED by this user.
        //  Mark this buffer as RELEASE by this user.
        streamBufferSet.markUserStatus(
                pOutImgStreamBuffer->getStreamInfo()->getStreamId(),
                getNodeId(),
                IUsersManager::UserStatus::USED |
                IUsersManager::UserStatus::RELEASE
                );
    }
    //
    // release
    streamBufferSet.applyRelease(getNodeId());
    onDispatchFrame(pFrame);

}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
JpegNodeImp::
getJpegParams(
    IMetadata* pMetadata_request,
    jpeg_params& rParams
) const
{
    if( NULL == pMetadata_request)
    {
        MY_LOGE("pMetadata_request=NULL");
    }
    rParams.gpsCoordinates =
        pMetadata_request->entryFor(MTK_JPEG_GPS_COORDINATES);

    rParams.gpsProcessingMethod =
        pMetadata_request->entryFor(MTK_JPEG_GPS_PROCESSING_METHOD);

    rParams.gpsTimestamp =
        pMetadata_request->entryFor(MTK_JPEG_GPS_TIMESTAMP);

#define getParam(meta, tag, type, param)                \
    do {                                                \
        if( !tryGetMetadata<type>(meta, tag, param) ) { \
            MY_LOGW("no tag: %s", #tag);                \
        }                                               \
    } while(0)
#define getAppParam(tag, type, param) getParam(pMetadata_request, tag, type, param)

    // request from app
    getAppParam(MTK_JPEG_ORIENTATION      , MINT32, rParams.orientation);
    getAppParam(MTK_JPEG_QUALITY          , MUINT8, rParams.quality);
    getAppParam(MTK_JPEG_THUMBNAIL_QUALITY, MUINT8, rParams.quality_thumbnail);
    getAppParam(MTK_JPEG_THUMBNAIL_SIZE   , MSize , rParams.size_thumbnail);
    getAppParam(MTK_SCALER_CROP_REGION    , MRect , rParams.cropRegion);

#undef getAppParam
#undef getParam
    if ( mJpegRotationEnable ) {
        if( rParams.orientation == 90 || rParams.orientation == 270)
        {
            MINT32 tmp = rParams.size_thumbnail.w;
            rParams.size_thumbnail.w = rParams.size_thumbnail.h;
            rParams.size_thumbnail.h = tmp;
        }
    }
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
JpegNodeImp::
updateMetadata(
    //IMetadata* pMetadata_request,
    jpeg_params& rParams,
    IMetadata* pMetadata_result
) const
{
#define updateNonEmptyEntry(pMetadata, tag, entry) \
    do{                                            \
        if( !entry.isEmpty() ) {                   \
            pMetadata->update(tag, entry);         \
        }                                          \
    }while(0)

    // gps related
    updateNonEmptyEntry(pMetadata_result , MTK_JPEG_GPS_COORDINATES       , rParams.gpsCoordinates);
    updateNonEmptyEntry(pMetadata_result , MTK_JPEG_GPS_PROCESSING_METHOD , rParams.gpsProcessingMethod);
    updateNonEmptyEntry(pMetadata_result , MTK_JPEG_GPS_TIMESTAMP         , rParams.gpsTimestamp);
    //
    updateEntry<MINT32>(pMetadata_result , MTK_JPEG_ORIENTATION       , rParams.orientation);
    updateEntry<MUINT8>(pMetadata_result , MTK_JPEG_QUALITY           , rParams.quality);
    updateEntry<MUINT8>(pMetadata_result , MTK_JPEG_THUMBNAIL_QUALITY , rParams.quality_thumbnail);
    updateEntry<MSize>(pMetadata_result  , MTK_JPEG_THUMBNAIL_SIZE    , rParams.size_thumbnail);
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
JpegNodeImp::
makeExifHeader(
    encode_frame* rpEncodeFrame,
    MINT8 * const pOutExif,
    size_t& rOutExifSize // [IN/OUT] in: exif buf size, out: exif header size
)
{
    MERROR ret;

    //IMetadata*            pInMeta_Hal       = NULL;
    //sp<IMetaStreamBuffer> pInMetaStream_Hal = NULL;
    //

    //
//    rpEncodeFrame->exif.setOutputBuffer(pOutExif, rOutExifSize);
    //
    // update debug info
    MINT32 bound = DBG_BOUND_WIDTH * DBG_BOUND_HEIGH;
    if(rpEncodeFrame->mpJpeg_Main->getImgSize().w * rpEncodeFrame->mpJpeg_Main->getImgSize().h > bound)
        if(mDbgInfoEnable)
            updateDebugInfoToExif(rpEncodeFrame->pInMeta_Hal, rpEncodeFrame->exif);
    //
    ret = rpEncodeFrame->exif.make((MUINTPTR)pOutExif, rOutExifSize);
    //
lbExit:
    returnMetadataAndUnlock(
            rpEncodeFrame->mpFrame,
            mpInHalMeta->getStreamId(),
            rpEncodeFrame->pInMetaStream_Hal,
            rpEncodeFrame->pInMeta_Hal
            );
    //
    rpEncodeFrame->exif.uninit();
    //
    return ret;
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
JpegNodeImp::
updateStdExifParam(
    MSize const&                rSize,
    IMetadata* const            rpHalMeta,
    jpeg_params const&          rParams,
    ExifParams &                rStdParams
) const
{
    rStdParams.u4ImageWidth  = rSize.w;
    rStdParams.u4ImageHeight = rSize.h;
    //
    // 3A
    if( rpHalMeta ) {
        IMetadata exifMeta;
        if( tryGetMetadata<IMetadata>(rpHalMeta, MTK_3A_EXIF_METADATA, exifMeta) ) {
            updateStdExifParam_3A(exifMeta, rStdParams);
        }
        else {
            MY_LOGW("no tag: MTK_3A_EXIF_METADATA");
        }
    }
    else {
        MY_LOGW("no in hal meta");
    }
    // gps
    updateStdExifParam_gps(
            rParams.gpsCoordinates, rParams.gpsProcessingMethod, rParams.gpsTimestamp,
            rStdParams
            );
    // others
    if ( mJpegRotationEnable)
        rStdParams.u4Orientation = 22;
    else
        rStdParams.u4Orientation = rParams.orientation;
    rStdParams.u4ZoomRatio   = calcZoomRatio(rParams.cropRegion, rSize);
    rStdParams.u4Facing      = (muFacing == MTK_LENS_FACING_BACK) ? 0 : 1;
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
JpegNodeImp::
updateStdExifParam_3A(
    IMetadata const&            rMeta,
    ExifParams &                rStdParams
) const
{
#define getParam(meta, tag, type, param)                      \
            do {                                              \
                type val = 0;                                 \
                if( !tryGetMetadata<type>(meta, tag, val) ) { \
                    MY_LOGW("no tag: %s", #tag);              \
                }                                             \
                param = val;                                  \
            } while(0)
#if 1
            // from result meta of 3A
            getParam(&rMeta, MTK_3A_EXIF_FNUMBER,             MINT32, rStdParams.u4FNumber);
            getParam(&rMeta, MTK_3A_EXIF_FOCAL_LENGTH,        MINT32, rStdParams.u4FocalLength);
            getParam(&rMeta, MTK_3A_EXIF_AWB_MODE,            MINT32, rStdParams.u4AWBMode);
            getParam(&rMeta, MTK_3A_EXIF_LIGHT_SOURCE,        MINT32, rStdParams.u4LightSource);
            getParam(&rMeta, MTK_3A_EXIF_EXP_PROGRAM,         MINT32, rStdParams.u4ExpProgram);
            getParam(&rMeta, MTK_3A_EXIF_SCENE_CAP_TYPE,      MINT32, rStdParams.u4SceneCapType);
            getParam(&rMeta, MTK_3A_EXIF_FLASH_LIGHT_TIME_US, MINT32, rStdParams.u4FlashLightTimeus);
            getParam(&rMeta, MTK_3A_EXIF_AE_METER_MODE,       MINT32, rStdParams.u4AEMeterMode);
            getParam(&rMeta, MTK_3A_EXIF_AE_EXP_BIAS,         MINT32, rStdParams.i4AEExpBias);
            getParam(&rMeta, MTK_3A_EXIF_CAP_EXPOSURE_TIME,   MINT32, rStdParams.u4CapExposureTime);
            getParam(&rMeta, MTK_3A_EXIF_AE_ISO_SPEED,        MINT32, rStdParams.u4AEISOSpeed);
#endif

#undef getParam
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
JpegNodeImp::
updateStdExifParam_gps(
    IMetadata::IEntry const& rGpsCoordinates,
    IMetadata::IEntry const& rGpsProcessingMethod,
    IMetadata::IEntry const& rGpsTimestamp,
    ExifParams &        rStdParams
) const
{
    if( rGpsCoordinates.count() == 3 ) {
        rStdParams.u4GpsIsOn = 1;
        // latitude
        ::snprintf(
                reinterpret_cast<char*>(rStdParams.uGPSLatitude), 32,
                "%f", rGpsCoordinates.itemAt(0, Type2Type<MDOUBLE>())
                );
        // longitude
        ::snprintf(
                reinterpret_cast<char*>(rStdParams.uGPSLongitude), 32,
                "%f", rGpsCoordinates.itemAt(1, Type2Type<MDOUBLE>())
                );
        // altitude
        rStdParams.u4GPSAltitude = (MUINT32)rGpsCoordinates.itemAt(2, Type2Type<MDOUBLE>());

        // timestamp
        if( !rGpsTimestamp.isEmpty() )
            ::snprintf(
                    reinterpret_cast<char*>(rStdParams.uGPSTimeStamp), 32,
                    "%" PRId64 , rGpsTimestamp.itemAt(0, Type2Type<MINT64>())
                    );
        else
            MY_LOGW("no MTK_JPEG_GPS_TIMESTAMP");

        if( !rGpsProcessingMethod.isEmpty() ) {
            size_t size = rGpsProcessingMethod.count();
            if( size > 64 ) {
                MY_LOGW("gps processing method too long, size %zu", size);
                size = 64;
            }

            for( size_t i = 0; i < size; i++ ) {
                rStdParams.uGPSProcessingMethod[i] =
                    rGpsProcessingMethod.itemAt(i, Type2Type<MUINT8>());
            }
            rStdParams.uGPSProcessingMethod[63] = '\0'; //null-terminating
        }
        else
            MY_LOGW("no MTK_JPEG_GPS_PROCESSING_METHOD");
    }
    else {
        MY_LOGD_IF( 1,
                "no gps data, coordinates count %d",
                rGpsCoordinates.count()
                );
        // no gps data
        rStdParams.u4GpsIsOn = 0;
    }
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
JpegNodeImp::
updateDebugInfoToExif(
    IMetadata* const            rpHalMeta,
    StdExif &                   exif
) const
{
    if( rpHalMeta ) {
        IMetadata exifMeta;
        if( tryGetMetadata<IMetadata>(rpHalMeta, MTK_3A_EXIF_METADATA, exifMeta) ) {
            MUINT32 dbgKey = MTK_3A_EXIF_DEBUGINFO_BEGIN;
            MUINT32 dbgVal = MTK_3A_EXIF_DEBUGINFO_BEGIN + 1;
            while( dbgVal < MTK_3A_EXIF_DEBUGINFO_END ) {
                MINT32 key;
                IMetadata::Memory dbgmem;
                if( tryGetMetadata<MINT32>(&exifMeta, dbgKey, key) &&
                        tryGetMetadata<IMetadata::Memory>(&exifMeta, dbgVal, dbgmem)
                  )
                {
                    MINT32 ID;
                    void* data = dbgmem.begin();
                    size_t size = dbgmem.size();
                    if( size > 0 ) {
                        MY_LOGD_IF(1, "key 0x%x, data %p, size %zu", key, data, size);
                        exif.sendCommand(CMD_REGISTER, key, reinterpret_cast<MUINTPTR>(&ID));
                        exif.sendCommand(CMD_SET_DBG_EXIF, ID, reinterpret_cast<MUINTPTR>(data), size);
                    }
                    else {
                        MY_LOGW("key 0x%x with size %zu", key, size);
                    }
                }
                //
                dbgKey +=2;
                dbgVal +=2;
            }
        }
        else {
            MY_LOGW("no tag: MTK_3A_EXIF_METADATA");
        }
    }
    else {
        MY_LOGW("no in hal meta");
    }
}


/******************************************************************************
 *
 ******************************************************************************/
MUINT32
JpegNodeImp::
calcZoomRatio(
    MRect      const&   cropRegion,
    MSize      const&   rSize
) const
{
    RWLock::AutoRLock _l(mConfigRWLock);
    MUINT32 zoomRatio = 100;
    if( ! mpOutJpeg.get() ) {
        MY_LOGW("jpeg stream is not configured");
        return 100;
    }

    MRect const cropAspect = calCropAspect(cropRegion.s, rSize);//mpOutJpeg->getImgSize()
    if( ! cropAspect.s ) {
        MY_LOGW("cropRegion(%d, %d, %dx%d), jpeg size %dx%d",
            cropRegion.p.x, cropRegion.p.y, cropRegion.s.w, cropRegion.s.h,
            rSize.w, rSize.h
            );
        return 100;
    }

    {
        MUINT32 val0 = cropAspect.s.w * mActiveArray.s.h;
        MUINT32 val1 = cropAspect.s.h * mActiveArray.s.w;
        if( val0 > val1 )
            zoomRatio = mActiveArray.s.w * 100 / cropAspect.s.w;
        else
            zoomRatio = mActiveArray.s.h * 100 / cropAspect.s.h;
    }

    MY_LOGD_IF(0, "active(%d, %d, %dx%d), cropRegion(%d, %d, %dx%d), zoomRatio %d",
            mActiveArray.p.x, mActiveArray.p.y, mActiveArray.s.w, mActiveArray.s.h,
            cropRegion.p.x, cropRegion.p.y, cropRegion.s.w, cropRegion.s.h,
            zoomRatio
            );
    return zoomRatio;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
JpegNodeImp::
hardwareOps_encode(
    my_encode_params& rParams
)
{
    FUNC_START;
    //
    MBOOL ret = MTRUE;
    //
    NSSImager::ISImager* pSImager = NSSImager::ISImager::createInstance(rParams.pSrc);
    if( pSImager == NULL ) {
        MY_LOGE("create SImage failed");
        return UNKNOWN_ERROR;
    }

    ret = pSImager->setTargetImgBuffer(rParams.pDst)

        && pSImager->setTransform(rParams.transform)

        && pSImager->setCropROI(rParams.crop)

        && pSImager->setEncodeParam(
                rParams.isSOI,
                rParams.quality,
                rParams.codecType
                )
        && pSImager->execute();

    pSImager->destroyInstance();
    pSImager = NULL;
    //
    if( !ret ) {
        MY_LOGE("encode failed");
        return UNKNOWN_ERROR;
    }
    //
    FUNC_END;
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
JpegNodeImp::
threadSetting()
{
    //
    //  thread policy & priority
    //  Notes:
    //      Even if pthread_create() with SCHED_OTHER policy, a newly-created thread
    //      may inherit the non-SCHED_OTHER policy & priority of the thread creator.
    //      And thus, we must set the expected policy & priority after a thread creation.
    MINT tid;
    struct sched_param sched_p;
    ::sched_getparam(0, &sched_p);
    if (JPEGTHREAD_POLICY == SCHED_OTHER) {
        sched_p.sched_priority = 0;
        ::sched_setscheduler(0, JPEGTHREAD_POLICY, &sched_p);
        ::setpriority(PRIO_PROCESS, 0, JPEGTHREAD_PRIORITY);   //  Note: "priority" is nice value.
    } else {
        sched_p.sched_priority = JPEGTHREAD_PRIORITY;          //  Note: "priority" is real-time priority.
        ::sched_setscheduler(0, JPEGTHREAD_POLICY, &sched_p);
    }

    MY_LOGD("tid(%d) policy(%d) priority(%d)", ::gettid(), JPEGTHREAD_POLICY, JPEGTHREAD_PRIORITY);

    return OK;

}


/******************************************************************************
 *
 ******************************************************************************/
void
JpegNodeImp::EncodeThread::
requestExit()
{
    //TODO: refine this
    Mutex::Autolock _l(mpNodeImp->mRequestQueueLock);
    mpNodeImp->mbRequestExit = MTRUE;
    mpNodeImp->mRequestQueueCond.signal();
}


/******************************************************************************
 *
 ******************************************************************************/
status_t
JpegNodeImp::EncodeThread::
readyToRun()
{
    return mpNodeImp->threadSetting();
}


/******************************************************************************
 *
 ******************************************************************************/
bool
JpegNodeImp::EncodeThread::
threadLoop()
{
    sp<IPipelineFrame> pFrame;
    if  (
            !exitPending()
        &&  OK == mpNodeImp->onDequeRequest(pFrame)
        &&  pFrame != 0
        )
    {
        mpNodeImp->onProcessFrame(pFrame);
        return true;
    }

    MY_LOGD("exit encode thread");
    return  false;

}
/******************************************************************************
 *
 ******************************************************************************/
MERROR
JpegNodeImp::
getImageBufferAndLock(
    android::sp<IPipelineFrame> const& pFrame,
    StreamId_T const streamId,
    sp<IImageStreamBuffer>& rpStreamBuffer,
    sp<IImageBuffer>& rpImageBuffer,
    encode_frame*& rpEncodeFrame,
    MBOOL const isMain
)
{
    sp<IImageStreamInfo> pStreamInfo = pFrame->getStreamInfoSet().getImageInfoFor(streamId);
    IStreamBufferSet&      rStreamBufferSet = pFrame->getStreamBufferSet();
    sp<IImageBufferHeap>   pImageBufferHeap = rpEncodeFrame->mpOutImgBufferHeap;
    MSize imageSize = MSize(mpInYuv_main->getImgSize().w, mpInYuv_main->getImgSize().h);

    if ( mJpegRotationEnable ) {
        if( rpEncodeFrame->mParams.orientation == 90 || rpEncodeFrame->mParams.orientation == 270)
        {
            imageSize.w = mpInYuv_main->getImgSize().h;
            imageSize.h = mpInYuv_main->getImgSize().w;
        }
    }
    MY_LOGD("Main Yuv imgSize:%dx%d transform:%d orientation(metadata):%d",
            pStreamInfo->getImgSize().w, pStreamInfo->getImgSize().h,
            pStreamInfo->getTransform(), rpEncodeFrame->mParams.orientation);

    if(NULL == pImageBufferHeap.get()){

        MERROR const err = ensureImageBufferAvailable_(
                pFrame->getFrameNo(),
                streamId,
                rStreamBufferSet,
                rpStreamBuffer
                );
        if( err != OK )
            return err;

        pImageBufferHeap = isInImageStream(streamId) ?
            rpStreamBuffer->tryReadLock(getNodeName()) :
            rpStreamBuffer->tryWriteLock(getNodeName());

        if (pImageBufferHeap.get() == NULL) {
            MY_LOGE("pImageBufferHeap == NULL");
            return BAD_VALUE;
        }

        rpEncodeFrame->mpOutImgBufferHeap = pImageBufferHeap;
        //MY_LOGD("@@pImageBufferHeap->getBufSizeInBytes(0) = %zu, heap format = %#x",
        //        pImageBufferHeap->getBufSizeInBytes(0),rpEncodeFrame->mpOutImgBufferHeap->getImgFormat());

        rpEncodeFrame->pInMetaStream_Hal = NULL;
        rpEncodeFrame->pInMeta_Hal = NULL;
        MERROR const err1 = getMetadataAndLock(
                rpEncodeFrame->mpFrame,
                mpInHalMeta->getStreamId(),
                rpEncodeFrame->pInMetaStream_Hal,
                rpEncodeFrame->pInMeta_Hal
                );
        //
        if( err1 != OK ) {
            MY_LOGE("getMetadataAndLock(pInMetaStream_Hal) err = %d", err1);
            return BAD_VALUE;
        }

        ExifParams stdParams;

        // update standard exif params
        updateStdExifParam(
                imageSize,
                rpEncodeFrame->pInMeta_Hal,
                rpEncodeFrame->mParams,
                stdParams
                );
        //
        MINT32 bound = DBG_BOUND_WIDTH * DBG_BOUND_HEIGH;
        if(imageSize.w * imageSize.h > bound) {
            rpEncodeFrame->exif.init(stdParams, mDbgInfoEnable);
            MY_LOGD("init (%dx%d)",
                imageSize.w, imageSize.h);
        } else {
            rpEncodeFrame->exif.init(stdParams, 0);
            MY_LOGD("skip init (%dx%d)",
                imageSize.w, imageSize.h);
        }

    }
    //  Query the group usage.
    MUINT const groupUsage = rpStreamBuffer->queryGroupUsage(getNodeId());

    size_t thumbnailMaxSize = 0;

    if( rpEncodeFrame->mbHasThumbnail )
    {
        thumbnailMaxSize = (rpEncodeFrame->mParams.size_thumbnail.w) * (rpEncodeFrame->mParams.size_thumbnail.h) * 18 / 10;
        size_t res = thumbnailMaxSize % 128;
        if( res != 0 )
            thumbnailMaxSize = thumbnailMaxSize + 128 - res;
    }

    size_t headerSize = 0;
    headerSize = rpEncodeFrame->exif.getStdExifSize() + rpEncodeFrame->exif.getDbgExifSize() + thumbnailMaxSize;

    if(headerSize % 128 != 0)
        MY_LOGW("not aligned header size %zu", headerSize);
    //
    if (!isMain)
    {

        size_t thumbnailOffset = rpEncodeFrame->exif.getStdExifSize();

        MY_LOGD("(w,h)=(%dx%d) thumbOffset = %zu, thumbMaxSize = %zu",
                rpEncodeFrame->mParams.size_thumbnail.w, rpEncodeFrame->mParams.size_thumbnail.h, thumbnailOffset, thumbnailMaxSize);

        size_t const bufStridesInBytes[3] = {thumbnailMaxSize, 0 ,0};
        rpImageBuffer = pImageBufferHeap->createImageBuffer_FromBlobHeap(
                thumbnailOffset, eImgFmt_JPEG, rpEncodeFrame->mParams.size_thumbnail, bufStridesInBytes);

        if (rpImageBuffer == NULL) {
            MY_LOGE("rpImageThumbnailBuffer == NULL");
            return BAD_VALUE;
        }
    }
    else
    {
        rpEncodeFrame->exif.setMaxThumbnail(thumbnailMaxSize);

        size_t mainOffset = rpEncodeFrame->exif.getHeaderSize();

        size_t mainMaxSize = pImageBufferHeap->getBufSizeInBytes(0) - mainOffset;

        MY_LOGD("mainOffset = %zu, mainMaxSize = %zu", mainOffset, mainMaxSize);

        size_t const bufStridesInBytes[3] = {mainMaxSize, 0 ,0};
        rpImageBuffer = pImageBufferHeap->createImageBuffer_FromBlobHeap(
                mainOffset, eImgFmt_JPEG, imageSize, bufStridesInBytes
                );
        if (rpImageBuffer == NULL) {
            MY_LOGE("rpImageMainBuffer == NULL");
            return BAD_VALUE;
        }

    }
    rpImageBuffer->lockBuf(getNodeName(), groupUsage);
    MY_LOGD("stream buffer: (%#" PRIx64 ") %p, heap: %p, buffer: %p, usage: 0x%x",
            streamId, rpStreamBuffer.get(), pImageBufferHeap.get(), rpImageBuffer.get(), groupUsage);
    return OK;
}
/******************************************************************************
 *
 ******************************************************************************/
IImageBuffer*
JpegNodeImp::
allocBuffer(MSize const size, int const format) const
{

    IImageBuffer* pBuf = NULL;

    MSize bufStridesInPixels[3] = {0};

    MUINT32 bufStridesInBytes[3] = {0};

using namespace NSCam::Utils::Format;

    MUINT32 plane = queryPlaneCount(format);

    for (int i = 0; i < (int)queryPlaneCount(format); i++)
    {
        bufStridesInBytes[i] =
            (queryPlaneWidthInPixels(format,i, size.w) * queryPlaneBitsPerPixel(format,i)) >> 3;
    }

    MINT32 bufBoundaryInBytes[3] = {0, 0, 0};

    IImageBufferAllocator::ImgParam imgParam(
            format, size, bufStridesInBytes, bufBoundaryInBytes, plane
            );

    IImageBufferAllocator* allocator = IImageBufferAllocator::getInstance();

    pBuf = allocator->alloc(LOG_TAG, imgParam);

    MY_LOGD("alloc %dx%d: %p", size.w, size.h, pBuf);

    return pBuf;
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
JpegNodeImp::
isHwEncodeSupported(int const format) const
{
    if( format == eImgFmt_YUY2 ||
        format == eImgFmt_NV12 ||
        format == eImgFmt_NV21 )
        return MTRUE;
    return MFALSE;
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
JpegNodeImp::
errorMetaHandle(
    encode_frame*& rpEncodeFrame
)
{
    FUNC_START;
    IStreamBufferSet&      streamBufferSet      = rpEncodeFrame->mpFrame->getStreamBufferSet();
    //in meta
    {
    //  Mark this buffer as RELEASE by this user.
    streamBufferSet.markUserStatus(
            mpInAppMeta->getStreamId(), getNodeId(),
            IUsersManager::UserStatus::RELEASE
            );
    }
    {// in hal meta
        streamBufferSet.markUserStatus(
                        mpInHalMeta->getStreamId(),
                        getNodeId(),
                        IUsersManager::UserStatus::RELEASE
                        );
    }
     // out metadata
    {
        //  Mark this buffer as RELEASE by this user. not USED will be marked error
        streamBufferSet.markUserStatus(
                        mpOutMetaStreamInfo_Result->getStreamId(),
                        getNodeId(),
                        IUsersManager::UserStatus::RELEASE
                        );
    }
    {//in image main
         streamBufferSet.markUserStatus(
                         mpInYuv_main->getStreamId(),
                         getNodeId(),
                         IUsersManager::UserStatus::RELEASE
                         );


    }
    {//in image thumbnail

         streamBufferSet.markUserStatus(
                         mpInYuv_thumbnail->getStreamId(),
                         getNodeId(),
                         IUsersManager::UserStatus::RELEASE
                         );

    }
    {//out image
        //  Mark this buffer as RELEASE by this user. not USED will be marked error
        streamBufferSet.markUserStatus(
                    mpOutJpeg->getStreamId(),
                    getNodeId(),
                    IUsersManager::UserStatus::RELEASE
                    );

    }

    streamBufferSet.applyRelease(getNodeId());
    onDispatchFrame(rpEncodeFrame->mpFrame);

    FUNC_END;
}



/******************************************************************************
 *
 ******************************************************************************/
MVOID
JpegNodeImp::
errorHandle(
    encode_frame*& rpEncodeFrame
)
{   FUNC_START;
    IStreamBufferSet&      streamBufferSet      = rpEncodeFrame->mpFrame->getStreamBufferSet();
    // in metadata
    {
    }
    // in hal metadata
    {
        streamBufferSet.markUserStatus(
                        mpInHalMeta->getStreamId(),
                        getNodeId(),
                        IUsersManager::UserStatus::RELEASE
                    );

    }
    // out metadata
        {

        //  Mark this buffer as RELEASE by this user. not USED will be marked error
            streamBufferSet.markUserStatus(
                        mpOutMetaStreamInfo_Result->getStreamId(),
                        getNodeId(),
                        IUsersManager::UserStatus::RELEASE
                        );
        }

    {//in image main
         streamBufferSet.markUserStatus(
                         mpInYuv_main->getStreamId(),
                         getNodeId(),
                         IUsersManager::UserStatus::RELEASE
                    );
    }

    {//in image thumbnail

            streamBufferSet.markUserStatus(
                         mpInYuv_thumbnail->getStreamId(),
                        getNodeId(),
                        IUsersManager::UserStatus::RELEASE
                        );
        }

    //Image out
    {
        streamBufferSet.markUserStatus(
                         mpOutJpeg->getStreamId(),
                    getNodeId(),
                    IUsersManager::UserStatus::RELEASE
                    );
    }
    streamBufferSet.applyRelease(getNodeId());
    onDispatchFrame(rpEncodeFrame->mpFrame);

    FUNC_END;

}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
JpegNodeImp::
unlockImage(
    sp<IImageStreamBuffer>& rpStreamBuffer,
    sp<IImageBuffer>& rpImageBuffer
)
{
        rpImageBuffer->unlockBuf(getNodeName());
    rpStreamBuffer->unlock(getNodeName(), rpImageBuffer->getImageBufferHeap());
}


