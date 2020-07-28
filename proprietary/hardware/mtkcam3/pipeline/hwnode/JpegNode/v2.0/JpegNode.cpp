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

#include "JpegNodeImp.h"
#include <pthread.h>
#include <sys/prctl.h>


#ifdef MTKCAM_BGS_JPEG_OUT_BUFFER_IMPORT
static const bool kIsImportJpegOutGraphicBuffer = true;
#else
static const bool kIsImportJpegOutGraphicBuffer = false;
#endif

#ifdef MTKCAM_BGS_JPEG_OUT_BUFFER_FREE
static const bool kIsFreeJpegOutGraphicBuffer = true;
#else
static const bool kIsFreeJpegOutGraphicBuffer = false;
#endif

#ifdef MTKCAM_JPEG_ASYNC_WAIT_ENCODE_REQ
  static const bool kIsAsyncWaitEncodeRequest = true;
#else
  static const bool kIsAsyncWaitEncodeRequest = false;
#endif

// Jpeg encode block unit
static const int g_encodeUnit = 16;

/******************************************************************************
 *
 ******************************************************************************/
static inline
MBOOL
isStream(sp<IStreamInfo> pStreamInfo, StreamId_T streamId )
{
    return pStreamInfo.get() && pStreamInfo->getStreamId() == streamId;
}

template <typename T>
static inline MBOOL
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

template <typename T>
static inline MVOID
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

static
MBOOL
isHwEncodeSupported(int const format)
{
    if( format == eImgFmt_YUY2 ||
        format == eImgFmt_NV12 ||
        format == eImgFmt_NV21 )
        return MTRUE;
    return MFALSE;
}

static
MRect
calCropAspect(MSize const& srcSize, MSize const& dstSize)
{
    MRect crop;
#define align2(x) (((x) + 1) & (~0x1))
    MUINT32 val0 = srcSize.w * dstSize.h;
    MUINT32 val1 = srcSize.h * dstSize.w;
    if( val0 > val1 ) {
        crop.s.w = (dstSize.h > 0) ? align2(val1 / dstSize.h) : 0;
        crop.s.h = srcSize.h;
        crop.p.x = (srcSize.w - crop.s.w) / 2;
        crop.p.y = 0;
    }
    else if ( val0 < val1 ) {
        crop.s.w = srcSize.w;
        crop.s.h = (dstSize.w > 0) ? align2(val0 / dstSize.w) : 0;
        crop.p.x = 0;
        crop.p.y = (srcSize.h - crop.s.h) / 2;
    }
    else {
        crop = MRect(MPoint(0,0), srcSize);
    }
#undef align2
    return crop;
}

static
MUINT32
calcZoomRatio(
    MBOOL const jpeg_configured,
    MRect const& cropRegion,
    MRect const& activeArray,
    MSize const& rSize,
    MINT32 const enableLog
)
{
    MUINT32 zoomRatio = 100;
    if( !jpeg_configured ) {
        MY_LOGW("jpeg stream is not configured");
        return zoomRatio;
    }

    MRect const cropAspect = calCropAspect(cropRegion.s, rSize);
    if( ! cropAspect.s ) {
        MY_LOGW("cropRegion(%d, %d, %dx%d), jpeg size %dx%d",
            cropRegion.p.x, cropRegion.p.y, cropRegion.s.w, cropRegion.s.h,
            rSize.w, rSize.h
            );
        return zoomRatio;
    }

    {
        MUINT32 val0 = cropAspect.s.w * activeArray.s.h;
        MUINT32 val1 = cropAspect.s.h * activeArray.s.w;
        if( val0 > val1 )
            zoomRatio = activeArray.s.w * 100 / cropAspect.s.w;
        else
            zoomRatio = activeArray.s.h * 100 / cropAspect.s.h;
    }

    MY_LOGD_IF(enableLog, "active(%d, %d, %dx%d), cropRegion(%d, %d, %dx%d), zoomRatio %d",
            activeArray.p.x, activeArray.p.y, activeArray.s.w, activeArray.s.h,
            cropRegion.p.x, cropRegion.p.y, cropRegion.s.w, cropRegion.s.h,
            zoomRatio
            );
    return zoomRatio;
}

static
MVOID
dumpYUVBuffer(
    MUINT32 const frameNo,
    sp<IImageBuffer>& rpImageBuffer,
    MUINT32 const idx
)
{
    char filename[256] = {0};
    sprintf(filename, "/sdcard/DCIM/Camera/Buffer_frame%d_%dx%d_%d_%d.yuv", frameNo,
        rpImageBuffer->getImgSize().w,
        rpImageBuffer->getImgSize().h,
        idx,
        rpImageBuffer->getFD(0)
        );

    MBOOL rets = rpImageBuffer->saveToFile(filename);
    MY_LOGI("[DUMP_JPG] SaveFile[%s]:(%d)", filename, rets);

}

android::sp<JpegNode>
JpegNode::
createInstance()
{
    JpgLog();
    return new JpegNodeImp();
}

JpegNodeImp::
JpegNodeImp()
    : BaseNode()
    , JpegNode()
    //
    , mConfigRWLock()
    , mpInAppMeta(NULL)
    , mpInHalMeta_capture(NULL)
    , mpInHalMeta_streaming(NULL)
    , mpOutMetaStreamInfo_Result(NULL)
    , mpInYuv_main(NULL)
    , mpInYuv_thumbnail(NULL)
    , mpInYuv_main2(NULL)
    , mpInYuv_Y16(NULL)
    , mpOutJpeg(NULL)
    , mpOutJpegAppSegments(NULL)
    , mBufPool(NULL)
    , mpInYuv_Y16_r(NULL)
    , mOutputJpegStreamBuf(NULL)
    , mOutputJpegBuffer(NULL)
    , mOutputJpegBufferHeap(NULL)
    , mOutputY16StreamBuf(NULL)
    , mOutputY16Buffer(NULL)
    //
    //
    , mbRequestDrained(MFALSE)
    , mbRequestExit(MFALSE)
    , mbRequestDequeSuspend(MFALSE)
    , mInQueuePreReleaseRequestNum(0)
    , mIsPreReleaseRequest(MFALSE)
    , mPropPreReleaseMode(0)
    , mIsPreReleaseEnable(MFALSE)
    , mbAppnOnly(MFALSE)
    , mHeicYuvSize()
    , mpJpegFrame(NULL)
    , mIsAsyncWaitEncodeReqEnable(MFALSE)
    , mPropJpegAsyncMode(0)
    , mJpegAsyncWaitEncodeReqLevel(0)
    , mPropJpegSwMode(0)
    , mpJpgPack(NULL)
{
    JpgLog();
    mNodeName = "JpegNode";//default name

    MINT32 enable = ::property_get_int32("vendor.jpeg.rotation.enable", 1);
    info.mJpegRotationEnable = (enable & 0x1)? MTRUE : MFALSE;
    MY_LOGD_IF(info.mJpegRotationEnable, "Jpeg Rotation enable");

    info.mLogLevel = ::property_get_int32("vendor.debug.camera.log", 0);
    if ( info.mLogLevel == 0 ) {
        info.mLogLevel = ::property_get_int32("vendor.debug.camera.log.JpegNode", 0);
    }
    MINT32 forceDbg;
#if     (MTKCAM_HW_NODE_LOG_LEVEL_DEFAULT > 3)
        forceDbg = 1;  // for ENG build
#elif   (MTKCAM_HW_NODE_LOG_LEVEL_DEFAULT > 2)
        forceDbg = 1;  // for USERDEBUG build
#else
        forceDbg = 0;  // for USER build
#endif
    info.mDbgInfoEnable= ::property_get_int32("vendor.debug.camera.dbginfo", forceDbg);
    info.muDumpBuffer = ::property_get_int32("vendor.debug.camera.dump.JpegNode", 0);
    info.mFlip = ::property_get_int32("vendor.debug.camera.Jpeg.flip", 0);
    mPropPreReleaseMode = ::property_get_int32("vendor.debug.camera.bgservice.mode", 0);

    MINT32 propAsyncMode = 0;
    MINT32 propJpegSwMode = 0;

    if (kIsAsyncWaitEncodeRequest)
        propAsyncMode = 4;

    mJpegAsyncWaitEncodeReqLevel = ::property_get_int32("vendor.debug.camera.jpeg.async", propAsyncMode);

    mPropJpegSwMode = ::property_get_int32("vendor.debug.camera.jpeg.swmode", propJpegSwMode);


    sem_init(&mRequestQueueCond, 0, 0);
    sem_init(&mbRequestDrainedCond, 0, 1);          //fix to 1
    sem_init(&mbRequestDequeSuspendCond, 0, 0);

    sem_init(&mReqNewTriggerEncodeCond, 0, 0);
    sem_init(&mReqWaitDoneQueueStillFreeCond, 0, 1);        //depends on parallel degree (ex. 1,2,4)
    sem_init(&mReqWaitDoneQueueIdleCond, 0, 1);

}

JpegNodeImp::
~JpegNodeImp()
{
    JpgLog();
    sem_destroy(&mRequestQueueCond);
    sem_destroy(&mbRequestDrainedCond);
    sem_destroy(&mbRequestDequeSuspendCond);

    sem_destroy(&mReqNewTriggerEncodeCond);
    sem_destroy(&mReqWaitDoneQueueStillFreeCond);
    sem_destroy(&mReqWaitDoneQueueIdleCond);

}


static void onProcessRequest(
    JpegNodeImp& jpg,
    Request& req
)
{
    {
        // set thread naming
        pthread_setname_np(pthread_self(), JPEGTHREAD_NAME);
    }
    CAM_TRACE_NAME(__FUNCTION__);
    sp<encode_frame> enc_frame = req.getEncodeFrame();
    // lock buffer
    if (req.lockMeta() != OK) {
        MY_LOGE("lock request meta failed!");
        req.mbIsEncodeJpegError = MTRUE;
        return;
    }
    if (req.prepareMeta() != OK) {
        MY_LOGE("prepareMeta failed!");
        req.mbIsEncodeJpegError = MTRUE;
        return;
    }

    if (req.lockInImg() != OK) {
        MY_LOGE("lock request input image failed");
        req.mbIsEncodeJpegError = MTRUE;
        return;
    }

    if (req.prepareExif() != OK) {
        MY_LOGE("prepareExif failed!");
        return;
    }

    if (req.lockOutImg() != OK) {
        MY_LOGE("lock request output image failed");
        req.mbIsEncodeJpegError = MTRUE;
        return;
    }

    enc_frame->muJpegSwMode = jpg.mPropJpegSwMode;

    // encode flow
    if (!jpg.mbAppnOnly) {
        std::thread encodeThumbThread;
        if (enc_frame->mbHasThumbnail) {
            encodeThumbThread = std::thread(&JpegNodeImp::encodeThumbnailJpeg, &jpg, std::ref(enc_frame));
        }
        jpg.encodeMainJpeg(enc_frame);

        if(enc_frame->mbHasThumbnail &&
                encodeThumbThread.joinable()) {
            encodeThumbThread.join();
        }
    } else {
        if (enc_frame->mbHasThumbnail)
            jpg.encodeThumbnailJpeg(enc_frame);
    }

    req.updateMetadata(enc_frame->mParams);

    if (req.unlock() != OK) {
        MY_LOGE("request unlock failed!");
    }

    // exif
    jpg.encodeExif(enc_frame);

    return;
}

static void extractRequestLoop(JpegNodeImp& jpg)
{
    JpgLog();

    bool stop = false;

    while(!stop) {
        sp<IPipelineFrame> pFrame = nullptr;
        {
            // wait for request queue & get the requestQueue lock
            jpg.mbRequestDrained = MTRUE;
            //sem_post(&jpg.mbRequestDrainedCond);

            // wait request enque
            MY_LOGD_IF(jpg.info.mLogLevel > 2, "wait mRequestQueueCond +");
            sem_wait(&jpg.mRequestQueueCond);
            MY_LOGD_IF(jpg.info.mLogLevel > 2, "wait mRequestQueueCond done -");

            if (jpg.mIsPreReleaseEnable) {
                // for bgs case, wait flush handle pre-release request

                MBOOL bSuspend = MFALSE;
                {
                    std::lock_guard<std::mutex> _l(jpg.mRequestQueueLock);
                    bSuspend = jpg.mbRequestDequeSuspend;
                }
                if (bSuspend == MTRUE) {
                    // set dained flag
                    {
                        std::lock_guard<std::mutex> _l(jpg.mRequestQueueLock);
                        jpg.mbRequestDrained = MTRUE;
                    }
                    //MY_LOGD("mbRequestDrainedCond broadcast!");
                    //sem_post(&jpg.mbRequestDrainedCond);

                    MY_LOGI("mRequestQueue.size:%zu mbRequestDequeSuspendCond wait+", jpg.mRequestQueue.size());
                    sem_wait(&jpg.mbRequestDequeSuspendCond);
                    MY_LOGI("mRequestQueue.size:%zu mbRequestDequeSuspendCond wait-", jpg.mRequestQueue.size());
                    {
                        std::lock_guard<std::mutex> _l(jpg.mRequestQueueLock);
                        jpg.mbRequestDequeSuspend = MFALSE;
                    }
                }
            }

            std::lock_guard<std::mutex> _l(jpg.mRequestQueueLock);
            MY_LOGD_IF(jpg.info.mLogLevel > 2, "extract Request ing...");

            // suspend flush, do this before mRequestQueueCond will block when empty flush
            MY_LOGI("mbRequestDrainedCond wait + ");
            sem_wait(&jpg.mbRequestDrainedCond);
            MY_LOGI("mbRequestDrainedCond wait - ");

            if (jpg.mbRequestExit) {
                stop = true;
            } else if (!jpg.mRequestQueue.empty()) {

                // prepare Request
                if (jpg.mIsPreReleaseEnable) {

                    if ( jpg.mInQueuePreReleaseRequestNum > 0 ) {
                        jpg.mIsPreReleaseRequest = MTRUE;
                        jpg.mInQueuePreReleaseRequestNum--;

                        jpg.mpJpegFrame = *jpg.mJpegFrameQueue.begin();
                        jpg.mJpegFrameQueue.erase(jpg.mJpegFrameQueue.begin());
                    } else {
                        jpg.mIsPreReleaseRequest = MFALSE;
                    }
                } else {
                    jpg.mIsPreReleaseRequest = MFALSE;
                }

                //  Here the queue is not empty, take the first request from the queue.
                if (jpg.mIsPreReleaseRequest == MFALSE) {
                    jpg.mbRequestDrained = MFALSE;
                } else {
                    // pre-release flow
                    // only first flush need to wait current frame done,
                    // the rest flush don't need to wait.
                    jpg.mbRequestDrained = MTRUE;
                }

                //jpg.mbRequestDrained = MFALSE;
                pFrame = *jpg.mRequestQueue.begin();
                jpg.mRequestQueue.erase(jpg.mRequestQueue.begin());
            }
        }
        if (pFrame != nullptr) {
            if (!jpg.mbAppnOnly) {
                jpg.processJpeg(pFrame, jpg);
            } else {
                jpg.processJpegAppSegments(pFrame, jpg);
            }
        }

        sem_post(&jpg.mbRequestDrainedCond);
    }
}

static void waitJpegEncodeReqDone(JpegNodeImp& jpg)
{
    JpgLog();

    sp<jpeg_frame> pJpegWaitDoneFrame = NULL;

    while(1)
    {

        // wait encode new trigger
        MY_LOGI("mReqNewTriggerEncodeCond wait + ");
        sem_wait(&jpg.mReqNewTriggerEncodeCond);
        MY_LOGI("mReqNewTriggerEncodeCond wait - ");

        if (jpg.mbRequestExit)
        {
            MY_LOGD("got exit signal...");
            return;
        }

        {
            std::lock_guard<std::mutex> _l(jpg.mWaitDoneQueueLock);
            pJpegWaitDoneFrame = *jpg.mJpegWaitDoneQueue.begin();
            MY_LOGD("wait start: mJpegWaitDoneQueue size %zu! ", jpg.mJpegWaitDoneQueue.size());
        }

        auto& reqList(pJpegWaitDoneFrame->m_vReqList);
        auto& jpeg_threads(pJpegWaitDoneFrame->m_vReqEncodeThreads);
        sp<IPipelineFrame> pFrame = pJpegWaitDoneFrame->m_pFrame;

        MBOOL bIsEncodeError = MFALSE;
        for (size_t i = 0; i < jpeg_threads.size(); i++) {
            if (jpeg_threads[i].joinable())
                jpeg_threads[i].join();

            // check errors and do error handling
            if (reqList[i].mbIsEncodeJpegError == MTRUE) {
                MY_LOGE("encode error happens in req %zu!!", i);
                bIsEncodeError = MTRUE;
            }
        }
        //FIXME do Jpeg Pack flow
        vector<sp<IImageBufferHeap>> vHeap;
        if ((bIsEncodeError == MFALSE) && jpg.info.mbPackBitStream)
        {
            MY_LOGD("doPack +");
            vHeap.resize(2, NULL);
            vHeap[0] = jpg.mBufPool->getBufHeap(jpg.mpInYuv_main->getStreamId());
            vHeap[1] = jpg.mBufPool->getBufHeap(jpg.mpInYuv_main2->getStreamId());
            MUINT const usage = GRALLOC_USAGE_SW_READ_OFTEN;
            sp<IImageBuffer> buf1 = vHeap[0]->createImageBuffer();
            sp<IImageBufferHeap> InYuv_Y16Heap = NULL;
            if (buf1 != NULL) {
                MBOOL ret = buf1->lockBuf(LOG_TAG, usage);
                MY_LOGA_IF(!ret, "%s", "lockBuffer failed, please check VA usage!");
            }
            sp<IImageBuffer> buf2 = vHeap[1]->createImageBuffer();
            if (buf2 != NULL) {
                MBOOL ret = buf2->lockBuf(LOG_TAG, usage);
                MY_LOGA_IF(!ret, "%s", "lockBuffer failed, please check VA usage!");
            }
            if (jpg.mpOutJpeg != NULL) {
                jpg.getBufferAndLock(pFrame,
                        jpg.mpOutJpeg->getStreamId(),
                        jpg.mOutputJpegStreamBuf,
                        jpg.mOutputJpegBufferHeap,
                        jpg.mOutputJpegBuffer,
                        MFALSE);
            }
            if (jpg.mpInYuv_Y16 != NULL) {
                jpg.getBufferAndLock(pFrame,
                        jpg.mpInYuv_Y16_r->getStreamId(),
                        jpg.mOutputY16StreamBuf,
                        InYuv_Y16Heap,
                        jpg.mOutputY16Buffer,
                        MTRUE);
            }
            if (buf1 != NULL && buf2 != NULL &&
                    jpg.mOutputJpegBuffer != NULL &&
                    jpg.mpInYuv_Y16_r != NULL && jpg.mpJpgPack !=NULL) {
                // pack
                MY_LOGD_IF(jpg.info.mLogLevel, "buf1 for working jpeg: size(%d) va(%p) bitStream(%d)",
                        (int)buf1->getBufSizeInBytes(0),
                        (void*)buf1->getBufVA(0),
                        (int)buf1->getBitstreamSize());
                MY_LOGD_IF(jpg.info.mLogLevel, "buf2 for working jpeg: size(%d) va(%p) bitStream(%d)",
                        (int)buf2->getBufSizeInBytes(0),
                        (void*)buf2->getBufVA(0),
                        (int)buf2->getBitstreamSize());
                MY_LOGD_IF(jpg.info.mLogLevel, "mOutputY16Buffer : size(%d) va(%p)",
                        (int)jpg.mOutputY16Buffer->getBufSizeInBytes(0),
                        (void*)jpg.mOutputY16Buffer->getBufVA(0));
                MY_LOGD_IF(jpg.info.mLogLevel, "mOutputJpegBuffer : size(%d) va(%p)",
                        (int)jpg.mOutputJpegBuffer->getBufSizeInBytes(0),
                        (void*)jpg.mOutputJpegBuffer->getBufVA(0));
                JpgPack::ConfigJpgParams config(buf1, buf2,
                        jpg.mOutputY16Buffer, jpg.mOutputJpegBuffer);

                jpg.mpJpgPack->config(config);
                size_t packSize = jpg.mpJpgPack->process();
                jpg.mOutputJpegBufferHeap->setBitstreamSize(packSize);
                MY_LOGD_IF(jpg.info.mLogLevel, "after pack size : %d", (int)packSize);
                jpg.mpJpgPack->uninit();

                if (jpg.mIsPreReleaseRequest == MTRUE) {
                    auto& pOutImgBufferHeap(jpg.mOutputJpegBufferHeap);
                    sp<IGraphicImageBufferHeap> pGraphicImageBufferHeap = NULL;
                    GrallocStaticInfo staticInfo = {};
                    pGraphicImageBufferHeap = IGraphicImageBufferHeap::castFrom(pOutImgBufferHeap.get());
                    if (pGraphicImageBufferHeap != NULL) {
                        int rc = IGrallocHelper::singleton()->query(*pGraphicImageBufferHeap->getBufferHandlePtr(), &staticInfo, NULL);
                        if (rc != OK) {
                            MY_LOGE("Fail to query blob buffer - rc:%d", rc);
                        } else {
                            MINTPTR jpegBuf = reinterpret_cast<MINTPTR>(pOutImgBufferHeap->getBufVA(0));
                            size_t jpegDataSize = pOutImgBufferHeap->getBitstreamSize();
                            size_t jpegBufSize = staticInfo.widthInPixels;
                            CameraBlob* pTransport = reinterpret_cast<CameraBlob*>(jpegBuf + jpegBufSize - sizeof(CameraBlob));
                            pTransport->blobId = CameraBlobId::JPEG;
                            pTransport->blobSize = jpegDataSize;
                            MY_LOGD("CameraBlob added (packed): frameNo:%d, jpegBuf:%#" PRIxPTR " bufsize:%zu datasize:%zu",
                                    pFrame->getFrameNo(), jpegBuf, jpegBufSize, jpegDataSize);
                        }
                    } else {
                        MY_LOGE("Fail to cast IGraphicImageBufferHeap !!");
                    }
                }
            }
            // release Jpeg

            if (buf1 != NULL) {
                jpg.mBufPool->putBufHeap(jpg.mpInYuv_main->getStreamId(), buf1->getFD(0));
                buf1->unlockBuf(LOG_TAG);
            }
            if (buf2 != NULL) {
                jpg.mBufPool->putBufHeap(jpg.mpInYuv_main2->getStreamId(), buf2->getFD(0));
                buf2->unlockBuf(LOG_TAG);
            }

            if (jpg.info.muDumpBuffer) {
                if(!NSCam::Utils::makePath(JPEG_DUMP_PATH, 0660))
                    MY_LOGI("makePath[%s] fails", JPEG_DUMP_PATH);

                NSCam::Utils::saveBufToFile(filename_pack,
                        (unsigned char*)jpg.mOutputJpegBuffer->getBufVA(0),
                        jpg.mOutputJpegBuffer->getBufSizeInBytes(0));

                MY_LOGI("[DUMP_JPG_PACKED] SaveFile[%s]!", filename_pack);
            }

            jpg.returnBuffer(pFrame, jpg.mOutputJpegStreamBuf,
                    jpg.mOutputJpegBuffer);
            jpg.returnBuffer(pFrame, jpg.mOutputY16StreamBuf,
                    jpg.mOutputY16Buffer);
            MY_LOGD("doPack -");
        }
        else
        {

            if (jpg.mIsPreReleaseRequest == MFALSE) {
                if (jpg.mvInYuv_main.size() > 1 && jpg.mvInYuv_main[1] != NULL) {
                    IStreamBufferSet& streamBufferSet = pFrame->getStreamBufferSet();
                    MY_LOGD_IF( 1, "mark unused stream release status: InYuvMain2!!");
                    streamBufferSet.markUserStatus(
                            jpg.mvInYuv_main[1]->getStreamId(),
                            jpg.getNodeId(),
                            IUsersManager::UserStatus::RELEASE
                            );
                }
                if (jpg.mpInYuv_Y16_r != NULL) {
                    IStreamBufferSet& streamBufferSet = pFrame->getStreamBufferSet();
                    MY_LOGD_IF( 1, "mark unused stream release status: InY16!!");
                    streamBufferSet.markUserStatus(
                            jpg.mpInYuv_Y16_r->getStreamId(),
                            jpg.getNodeId(),
                            IUsersManager::UserStatus::RELEASE
                            );
                }
            } else {
                if (jpg.mpJpegFrame->m_pIn2Yuv_ImgStreamBuffer) {
                    auto& rpStreamBuffer(jpg.mpJpegFrame->m_pIn2Yuv_ImgStreamBuffer);
                    MY_LOGD_IF( 1, "mark unused stream release status: InYuvMain2(pre-release)!!");
                    rpStreamBuffer->markUserStatus(
                            jpg.mpJpegFrame->m_NodeId,
                            IUsersManager::UserStatus::RELEASE
                            );
                }
                if (jpg.mpJpegFrame->m_pInY16_ImgStreamBuffer) {
                    auto& rpStreamBuffer(jpg.mpJpegFrame->m_pInY16_ImgStreamBuffer);
                    MY_LOGD_IF( 1, "mark unused stream release status: InY16(pre-release)!!");
                    rpStreamBuffer->markUserStatus(
                            jpg.mpJpegFrame->m_NodeId,
                            IUsersManager::UserStatus::RELEASE
                            );
                }
            }
        }

        //free graphic buffer
        if (jpg.mIsPreReleaseRequest == MTRUE) {
            auto& graphicBufferHandle(jpg.mpJpegFrame->m_GraphicBufferImportHandle);

            if (graphicBufferHandle != NULL) {

                if  ( auto helper = IGrallocHelper::singleton()) {

                    //buffer_handle_t* bufferHandle = const_cast<buffer_handle_t*>(pGraphicImageBufferHeap->getBufferHandlePtr());
                    if (kIsFreeJpegOutGraphicBuffer) {
                        helper->freeBuffer(graphicBufferHandle);
                        MY_LOGI_IF( 1, "GraphicImageBuffer freebuffer %p, fd %d, FrameNo %d!!", graphicBufferHandle, graphicBufferHandle->data[0], pFrame->getFrameNo());
                    } else {
                        MY_LOGI_IF( 1, "GraphicImageBuffer skip freebuffer %p, fd %d, FrameNo %d!!", graphicBufferHandle, graphicBufferHandle->data[0], pFrame->getFrameNo());
                    }
                } else {
                    MY_LOGE_IF( 1, "GraphicImageBuffer get helper fail!!");
                }
            } else {
                MY_LOGW_IF( 1, "Out Stream is not GraphicImageBuffer, skip unmap graphic buffer!!");
            }
        }
        if (bIsEncodeError == MFALSE) {
            jpg.finalizeEncodeFrame(pFrame);
        } else {
            jpg.errorHandle(pFrame);
        }
        jpg.mOutputJpegStreamBuf = NULL;
        jpg.mOutputY16StreamBuf = NULL;
        jpg.mpJpegFrame = NULL;

        {
            std::lock_guard<std::mutex> _l(jpg.mWaitDoneQueueLock);

            int size = jpg.mJpegWaitDoneQueue.size();
            jpg.mJpegWaitDoneQueue.erase(jpg.mJpegWaitDoneQueue.begin());
            MY_LOGD("wait done: mJpegWaitDoneQueue size %d -> %zu! ", size, jpg.mJpegWaitDoneQueue.size());

            if (jpg.mJpegWaitDoneQueue.empty())
            {
                sem_post(&jpg.mReqWaitDoneQueueIdleCond);
                MY_LOGD("post mReqWaitDoneQueueIdleCond ");
            }
        }

        // Notify encode done
        sem_post(&jpg.mReqWaitDoneQueueStillFreeCond);

        reqList.clear();
        jpeg_threads.clear();
        pJpegWaitDoneFrame = NULL;

    }

}



MERROR
JpegNodeImp::
init(InitParams const& rParams)
{

    JpgLog();
    mOpenId = rParams.openId;
    mNodeId = rParams.nodeId;
    mNodeName = rParams.nodeName;
    MY_LOGD("OpenId %d, nodeId %#" PRIxPTR ", name %s",
            getOpenId(), getNodeId(), getNodeName() );
    mWorkerThread = std::async( std::launch::async, [&]() -> void {
        ::prctl(
            PR_SET_NAME,
            (unsigned long)((std::string("JpegNode-")+std::to_string(rParams.openId)).c_str()),
            0, 0, 0);
        return extractRequestLoop(std::ref(*this));
    });

    // request encode thread
    mReqEncodeThread = std::async( std::launch::async, [&]() -> void {
        ::prctl(
            PR_SET_NAME,
            (unsigned long)((std::string("JpegNode-")+std::to_string(rParams.openId)).c_str()),
            0, 0, 0);
        return waitJpegEncodeReqDone(std::ref(*this));
    });

    {
        sp<IMetadataProvider> pMetadataProvider =
            NSMetadataProviderManager::valueFor(getOpenId());
        if( ! pMetadataProvider.get() ) {
            MY_LOGE(" ! pMetadataProvider.get() ");
            return DEAD_OBJECT;
        }

        IMetadata static_meta = pMetadataProvider->getMtkStaticCharacteristics();
        if( !tryGetMetadata<MRect>(
                    &static_meta, MTK_SENSOR_INFO_ACTIVE_ARRAY_REGION,
                    info.mActiveArray)
          ) {
            MY_LOGE("no static info: MTK_SENSOR_INFO_ACTIVE_ARRAY_REGION");
            return UNKNOWN_ERROR;
        }

        if( !tryGetMetadata<MUINT8>(
                    &static_meta, MTK_SENSOR_INFO_FACING,
                    info.muFacing)
          ) {
            MY_LOGE("no static info: MTK_SENSOR_INFO_FACING");
            return UNKNOWN_ERROR;
        }

        MY_LOGD("active array(%d, %d, %dx%d), facing %d",
                info.mActiveArray.p.x, info.mActiveArray.p.y,
                info.mActiveArray.s.w, info.mActiveArray.s.h,
                info.muFacing);
    }

    return OK;
}

MERROR
JpegNodeImp::
config(ConfigParams const& rParams)
{
    JpgLog();
    CHECK_ERROR( verifyConfigParams(rParams) );
    flush();
    {
        RWLock::AutoWLock _l(mConfigRWLock);
        // meta
        mpInAppMeta = rParams.pInAppMeta;
        mpInHalMeta_capture = rParams.pInHalMeta_capture;
        mpInHalMeta_streaming     = rParams.pInHalMeta_streaming;
        mpOutMetaStreamInfo_Result = rParams.pOutAppMeta;

        if (mpInHalMeta_capture != NULL) {
            mHalMetaMap.emplace(
                std::make_pair(
                    mpInHalMeta_capture->getStreamId(),
                    mpInHalMeta_capture));
            MY_LOGD("debug capture InHalMeta streamId : %#" PRIx64 "",
                mpInHalMeta_capture->getStreamId());
        }
        if (mpInHalMeta_streaming != NULL) {
            mHalMetaMap.emplace(
                std::make_pair(
                    mpInHalMeta_streaming->getStreamId(),
                    mpInHalMeta_streaming));
            MY_LOGD("debug streaming InHalMeta streamId : %#" PRIx64 "",
                mpInHalMeta_streaming->getStreamId());
        }
        if (mpOutMetaStreamInfo_Result != NULL) {
            MY_LOGD("debug InOutMeta streamId : %#" PRIx64 "",
                mpOutMetaStreamInfo_Result->getStreamId());
        }

        mpInYuv_main = rParams.pInYuv_Main;
        mpInYuv_thumbnail = rParams.pInYuv_Thumbnail;
        mpOutJpeg = rParams.pOutJpeg;
        mpOutJpegAppSegments = rParams.pOutJpegAppSegment;
        mpInYuv_main2 = rParams.pInYuv_Main2;
        mpInYuv_Y16 = rParams.pInYuv_Y16;

        // Heic flow control flag: Android Q
        if (mpOutJpegAppSegments != NULL) {
            mbAppnOnly = MTRUE;
            mHeicYuvSize = rParams.heicYuvSize;
        }

        if (mpInYuv_main != NULL && mpInYuv_main2 != NULL && mpInYuv_thumbnail != NULL) {
            mvInYuv_main.resize(2, NULL);
            mvInYuv_thumbnail.resize(2, NULL);
        } else if (mbAppnOnly){
            mvInYuv_thumbnail.resize(1, NULL);
        } else {
            mvInYuv_main.resize(1, NULL);
            mvInYuv_thumbnail.resize(1, NULL);
        }

        if (mPropPreReleaseMode == 0) {  //depends VendorTag
            mIsPreReleaseEnable = rParams.bIsPreReleaseEnable;
        } else if (mPropPreReleaseMode == 1) { //force enable
            mIsPreReleaseEnable = MTRUE;
        } else if (mPropPreReleaseMode == 2) { //force disable
            mIsPreReleaseEnable = MFALSE;
        }
        MY_LOGD("mPropPreReleaseMode %d, bIsPreReleaseEnable %d, mIsPreReleaseEnable:%d !",
                    mPropPreReleaseMode, rParams.bIsPreReleaseEnable, mIsPreReleaseEnable);
        if (mpInYuv_main != NULL){
            MY_LOGD("mpInYuv_main:%dx%d",
                mpInYuv_main->getImgSize().w,
                mpInYuv_main->getImgSize().h);
        }
        if (mpInYuv_thumbnail != NULL){
            MY_LOGD("mpInYuv_thumbnail:%dx%d",
                mpInYuv_thumbnail->getImgSize().w,
                mpInYuv_thumbnail->getImgSize().h);
        }
        if (mpInYuv_main2 != NULL){
            MY_LOGD("mpInYuv_main2:%dx%d",
                mpInYuv_main2->getImgSize().w,
                mpInYuv_main2->getImgSize().h);
            // FIXME
            vector<sp<IImageStreamInfo>> vStreamInfo;
            vStreamInfo.push_back(mpInYuv_main);
            vStreamInfo.push_back(mpInYuv_main2);
            mBufPool = new JpegBufPool(vStreamInfo, 1/*buffer count*/);
            if(mBufPool != NULL) {
                mBufPool->commitPool();
            }
            // initialize plugin
            mpJpgPack = new JpgPack();
            if (!mpJpgPack)
                mpJpgPack->init();
        }
        if (mpInYuv_Y16 != NULL) {
            MY_LOGD("mpInYuv_Y16:%dx%d",
                mpInYuv_Y16->getImgSize().w,
                mpInYuv_Y16->getImgSize().h);
        }

        if (mpOutJpeg != NULL) {
            MY_LOGD("mpOutJpeg:%dx%d",
                mpOutJpeg->getImgSize().w, mpOutJpeg->getImgSize().h);
        }

        if (mpOutJpegAppSegments != NULL) {
            MY_LOGD("mpOutJpegAppSegments:%dx%d",
                mpOutJpegAppSegments->getImgSize().w, mpOutJpegAppSegments->getImgSize().h);
        }

        if ((mJpegAsyncWaitEncodeReqLevel > 0) &&
             (mpInYuv_main2 == NULL) && (mpInYuv_Y16 == NULL) &&
            (mbAppnOnly == MFALSE) && (mIsPreReleaseEnable == MFALSE))
        {
            mIsAsyncWaitEncodeReqEnable = MTRUE;
            sem_init(&mReqWaitDoneQueueStillFreeCond, 0, mJpegAsyncWaitEncodeReqLevel);
        }
        MY_LOGD("defaultAsync %d, mbAppnOnly %d, propSwMode %d, propAsync %d, Level %d, AsyncEnable:%d !",
        kIsAsyncWaitEncodeRequest, mbAppnOnly, mPropJpegSwMode, mPropJpegAsyncMode, mJpegAsyncWaitEncodeReqLevel, mIsAsyncWaitEncodeReqEnable);

    }

    return OK;
}

MERROR
JpegNodeImp::
uninit()
{
    JpgLog();
    if ( OK != flush() )
        MY_LOGE("flush failed");
    {
        std::lock_guard<std::mutex> _l(mRequestQueueLock);
        mbRequestExit = true;
        MY_LOGD("release request queue +");
        sem_post(&mRequestQueueCond);
        sem_post(&mReqNewTriggerEncodeCond);
        MY_LOGD("release request queue -");
    }

    if (mWorkerThread.valid())
    {
        MY_LOGD("join workerThread +");
        mWorkerThread.wait();
        MY_LOGD("join workerThread -");
    }

    if (mReqEncodeThread.valid())
    {
        MY_LOGD("join mReqEncodeThread +");
        mReqEncodeThread.wait();
        MY_LOGD("join mReqEncodeThread -");
    }


    return OK;
}
MERROR
JpegNodeImp::
flush(
    android::sp<IPipelineFrame> const& pFrame
)
{
    if (mIsPreReleaseEnable) {

        if( ! pFrame.get() ) {
            MY_LOGE("Null frame");
            return BAD_VALUE;
        }

        MY_LOGI("FrameNo : %d, RequestNo : %d", pFrame->getFrameNo(), pFrame->getRequestNo());

        //queue ppFrame if not error
        IStreamBufferSet&      rStreamBufferSet = pFrame->getStreamBufferSet();
        sp<IImageStreamBuffer> rpStreamBuffer = rStreamBufferSet.getImageBuffer(mpInYuv_main->getStreamId(), getNodeId());
        if  ( rpStreamBuffer == 0 ) {
            MY_LOGE("[frame:%d node:%#" PRIxPTR "], streamID(%#" PRIx64 ")", pFrame->getFrameNo(), getNodeId(), mpInYuv_main->getStreamId());
            return NAME_NOT_FOUND;
        }
        //  Check buffer status.
        if  ( rpStreamBuffer->hasStatus(STREAM_BUFFER_STATUS::ERROR) ) {
            //  The producer ahead of this user may fail to render this buffer's content.
            MY_LOGE("[frame:%u node:%#" PRIxPTR "][stream buffer:%s] bad status:%d", pFrame->getFrameNo(), getNodeId(), rpStreamBuffer->getName(), rpStreamBuffer->getStatus());
            return BaseNode::flush(pFrame);
        }
        {
            std::lock_guard<std::mutex> _l(mRequestQueueLock);
            if (mbRequestDequeSuspend == MFALSE) {
                MY_LOGD("flush frame set mbRequestDequeSuspend!");
                mbRequestDequeSuspend = MTRUE;
            }
        }
        return this->queue(pFrame);
    }
    return BaseNode::flush(pFrame);
}


MERROR
JpegNodeImp::
flush()
{
    JpgLog();
#define MARK_USER_STATUS(sBufSet, streamBuffer, nId, status)   \
        do {                                                \
            sBufSet.markUserStatus(                         \
                 streamBuffer->getStreamId(),               \
                 nId,                                       \
                 status                                     \
                 );                                         \
        }while(0)



    if (!mIsPreReleaseEnable) {

        // 1. clear requests
        {
            std::lock_guard<std::mutex> _l(mRequestQueueLock);
            //
            Que_T::iterator it = mRequestQueue.begin();
            while ( it != mRequestQueue.end() ) {
                BaseNode::flush(*it);
                it = mRequestQueue.erase(it);
            }
        }
        // 2. wait enque thread
        waitForRequestDrained();


        MY_LOGD("wait mReqWaitDoneQueueIdleCond + ");
        sem_wait(&mReqWaitDoneQueueIdleCond);
        sem_post(&mReqWaitDoneQueueIdleCond);
        MY_LOGD("wait mReqWaitDoneQueueIdleCond - ");


    } else {
        //
        // pre-release step
        // 1. suspend deque request thread
        MBOOL bIsReq = MFALSE;
        {
            std::lock_guard<std::mutex> _l(mRequestQueueLock);
            bIsReq = !mRequestQueue.empty();
            if (bIsReq == MTRUE) {
                if (mbRequestDequeSuspend == MFALSE) {
                    MY_LOGI("flush set mbRequestDequeSuspend!");
                    mbRequestDequeSuspend = MTRUE;
                }
            } else {
                if (mbRequestDequeSuspend == MTRUE) {
                    MY_LOGI("flush disable mbRequestDequeSuspend!");
                    mbRequestDequeSuspend = MFALSE;
                }
            }
        }
        //
        // 2. wait enque thread finish
        waitForRequestDrained();

        if (bIsReq == MFALSE) {
            MY_LOGI("flush zero req return!");
            return OK;
        }
        //
        // 3. process enque buffer
        // a) get stream buffer
        // b) mark in/out buffer status as used and pre-release
        // c) apply & dispatch pre-release frame
        {
            std::lock_guard<std::mutex> _l(mRequestQueueLock);

            MUINT32 index = 0;
            Que_T::iterator it = mRequestQueue.begin();
            for (; it != mRequestQueue.end(); ) {

                if (index < mInQueuePreReleaseRequestNum) {
                    index++;
                    it++;
                    continue;  //skip already dispatch preRelease frame
                }

                {
                    //queue ppFrame if not error
                    IStreamBufferSet&      rStreamBufferSet = (*it)->getStreamBufferSet();
                    MBOOL bIsBadRequest = MFALSE;

                    sp<IImageStreamBuffer> rpStreamBuffer = rStreamBufferSet.getImageBuffer(mpInYuv_main->getStreamId(), getNodeId());
                    if ( rpStreamBuffer == 0 ) {
                        MY_LOGE("[frame:%d node:%#" PRIxPTR "], streamID(%#" PRIx64 ")", (*it)->getFrameNo(), getNodeId(), mpInYuv_main->getStreamId());
                        bIsBadRequest = MTRUE;
                    } else if ( rpStreamBuffer->hasStatus(STREAM_BUFFER_STATUS::ERROR) ) {
                        //  Check buffer status.
                        //  The producer ahead of this user may fail to render this buffer's content.
                        MY_LOGE("[frame:%u node:%#" PRIxPTR "][stream buffer:%s] bad status:%d", (*it)->getFrameNo(), getNodeId(), rpStreamBuffer->getName(), rpStreamBuffer->getStatus());
                        bIsBadRequest = MTRUE;
                    }

                    if (bIsBadRequest == MTRUE) {
                        BaseNode::flush(*it);
                        it = mRequestQueue.erase(it);
                        continue;
                    }
                }

                auto& pFrame(*it);
                mInQueuePreReleaseRequestNum++;
                MY_LOGI("mark PreRelease FrameNo : %d, RequestNo : %d, PreReleaseNum : %d",
                          pFrame->getFrameNo(), pFrame->getRequestNo(), mInQueuePreReleaseRequestNum);

                sp<jpeg_frame> pEncodeFrameNode = new jpeg_frame(pFrame);

                //enque encode_frame
                mJpegFrameQueue.push_back(pEncodeFrameNode);
                pEncodeFrameNode->m_NodeId = getNodeId();

                // call PreReleaseRequest
                IPreReleaseRequestMgr::getInstance()->registerStreamBuffer(pFrame);

                //1. get stream buffer

                //2. mark in/out buffer status as pre-release
                IStreamBufferSet&      streamBufferSet      = pFrame->getStreamBufferSet();
                auto status = IUsersManager::UserStatus::PRE_RELEASE;
                              /*IUsersManager::UserStatus::USED | */

                sp<IImageStreamInfo> yuv_main2 = NULL;
                sp<IImageStreamInfo> yuv_y16 = NULL;
                {
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
                        //return OK;
                    }

                    {
                        IPipelineFrame::MetaInfoIOMap const& metaIOMap = IOMapSet.mMetaInfoIOMapSet[0];
                        for (size_t i = 0; i < metaIOMap.vIn.size(); i++) {
                            StreamId_T const streamId = metaIOMap.vIn.keyAt(i);
                            auto search = mHalMetaMap.find((int)streamId);
                            if (search != mHalMetaMap.end()) {
                                MY_LOGD("StreamId : %#" PRIx64 "", streamId);
                                pEncodeFrameNode->m_pInHalMeta = search->second;
                                break;
                            }
                        }
                        if (pEncodeFrameNode->m_pInHalMeta == nullptr)
                        {
                            MY_LOGE("PipelineContext doesn't setup input hal meta");
                            //return OK;
                        }
                    }

                    // query if use thumbnail
                    IPipelineFrame::ImageInfoIOMap const& imageIOMap = IOMapSet.mImageInfoIOMapSet[0];
                    for( size_t i = 0; i < imageIOMap.vIn.size(); i++ )
                    {
                        StreamId_T const streamId = imageIOMap.vIn.keyAt(i);
                        if( isStream(mpInYuv_thumbnail, streamId) ) {
                            pEncodeFrameNode->m_bHasThumbnail = MTRUE;
                            MY_LOGD("need Thumbnail!");
                            break;
                        }
                    }

                    // check if need pack bistream
                    {
                        IPipelineFrame::ImageInfoIOMap const& imageIOMap = IOMapSet.mImageInfoIOMapSet[0];
                        MBOOL bIsMain2 = MFALSE;
                        MBOOL bIsY16   = MFALSE;
                        for( size_t i = 0; i < imageIOMap.vIn.size(); i++ )
                        {
                            StreamId_T const streamId = imageIOMap.vIn.keyAt(i);
                            if (isStream(mpInYuv_main2, streamId)) {
                                yuv_main2 = imageIOMap.vIn.valueAt(i);
                                //mvInYuv_main[1] = yuv_main2;
                                if (checkBufferAvailable(pFrame, yuv_main2->getStreamId()) == OK) {
                                    bIsMain2 = MTRUE;
                                }
                            }
                            if (isStream(mpInYuv_Y16, streamId)) {
                                yuv_y16 = imageIOMap.vIn.valueAt(i);
                                //mpInYuv_Y16_r = yuv_y16;
                                if (checkBufferAvailable(pFrame, yuv_y16->getStreamId()) == OK) {
                                    bIsY16 = MTRUE;
                                }
                            }
                        }
                        if (bIsMain2 == MTRUE && bIsY16 == MTRUE) {
                            pEncodeFrameNode->m_bPackBitStream = MTRUE;
                        }
                        MY_LOGW_IF( 1, "check m_bPackBitStream %d, FrameNo:%d!!",
                            pEncodeFrameNode->m_bPackBitStream, pFrame->getFrameNo());

                    }

                }

                // src main YUV
                {
                    sp<IImageStreamBuffer> rpStreamBuffer;
                    MERROR const err = ensureImageBufferAvailable_(
                            pFrame->getFrameNo(),
                            mpInYuv_main->getStreamId(),
                            streamBufferSet,
                            pEncodeFrameNode->m_pInYuv_ImgStreamBuffer /*rpStreamBuffer*/
                            );

                    if ( err != OK ) {
                        MY_LOGE("ensureImageBufferAvailable_ (InYUV) fail, FrameNo:%d, StreamId %#" PRIx64 "!!",
                                       pFrame->getFrameNo(), mpInYuv_main->getStreamId());
                        //return err;
                    }

                    MARK_USER_STATUS(streamBufferSet, mpInYuv_main, getNodeId(), status);
                }

                // src thumbnail YUV
                if(pEncodeFrameNode->m_bHasThumbnail == MTRUE) {

                    MERROR const err = ensureImageBufferAvailable_(
                            pFrame->getFrameNo(),
                            mpInYuv_thumbnail->getStreamId(),
                            streamBufferSet,
                            pEncodeFrameNode->m_pInThumbYuv_ImgStreamBuffer /*rpStreamBuffer*/
                            );

                    if ( err != OK ) {
                        MY_LOGE("ensureImageBufferAvailable_ (InThumbYUV) fail, FrameNo:%d, StreamId %#" PRIx64 "!!",
                                       pFrame->getFrameNo(), mpInYuv_thumbnail->getStreamId());
                        //return err;
                    }

                    MARK_USER_STATUS(streamBufferSet, mpInYuv_thumbnail, getNodeId(), status);
                }


                {
                    // src main2_YUV
                    if (yuv_main2.get() != NULL) {
                        MERROR const err = ensureImageBufferAvailable_(
                                pFrame->getFrameNo(),
                                yuv_main2->getStreamId(),
                                streamBufferSet,
                                pEncodeFrameNode->m_pIn2Yuv_ImgStreamBuffer /*rpStreamBuffer*/
                                );

                        if ( err != OK ) {
                            MY_LOGW_IF( 1, "ensureImageBufferAvailable_ (In2YUV) fail, FrameNo:%d, StreamId %#" PRIx64 "!!",
                                           pFrame->getFrameNo(), yuv_main2->getStreamId());
                            //return err;
                        }
                        if (pEncodeFrameNode->m_pIn2Yuv_ImgStreamBuffer) {
                            MARK_USER_STATUS(streamBufferSet, yuv_main2, getNodeId(), status);
                        }
                    }

                    // src Y16 buffer
                    if (yuv_y16.get() != NULL) {
                        MERROR const err = ensureImageBufferAvailable_(
                                pFrame->getFrameNo(),
                                yuv_y16->getStreamId(),
                                streamBufferSet,
                                pEncodeFrameNode->m_pInY16_ImgStreamBuffer /*rpStreamBuffer*/
                                );

                        if ( err != OK ) {
                            MY_LOGW_IF( 1, "ensureImageBufferAvailable_ (InY16) fail, FrameNo:%d, StreamId %#" PRIx64 "!!",
                                           pFrame->getFrameNo(), yuv_y16->getStreamId());
                            //return err;
                        }
                        if (pEncodeFrameNode->m_pInY16_ImgStreamBuffer) {
                            MARK_USER_STATUS(streamBufferSet, yuv_y16, getNodeId(), status);
                        }
                    }

                }

                // dst bistream buffer
                {
                    auto& pImageBufferHeap(pEncodeFrameNode->m_pOutJpeg_ImgBufferHeap);
                    sp<IGraphicImageBufferHeap> pGraphicImageBufferHeap = NULL;
                    auto& graphicBufferHandle(pEncodeFrameNode->m_GraphicBufferImportHandle);
                    sp<IImageBufferHeap> pHeap = NULL;

                    MERROR const err = ensureImageBufferAvailable_(
                            pFrame->getFrameNo(),
                            mpOutJpeg->getStreamId(),
                            streamBufferSet,
                            pEncodeFrameNode->m_pOutJpeg_ImgStreamBuffer /*rpStreamBuffer*/
                            );
                    if ( err != OK ) {
                        MY_LOGE("ensureImageBufferAvailable_ (Out) fail, FrameNo:%d, StreamId %#" PRIx64 "!!",
                                       pFrame->getFrameNo(), mpOutJpeg->getStreamId());
                        //return err;
                    }
                    //import buffer handle if graphic buffer
                    pImageBufferHeap = pEncodeFrameNode->m_pOutJpeg_ImgStreamBuffer->tryWriteLock(getNodeName());

                    if (pImageBufferHeap != NULL) {
                        pGraphicImageBufferHeap = IGraphicImageBufferHeap::castFrom(pImageBufferHeap.get());
                    }

                    if (pGraphicImageBufferHeap != NULL) {

                        auto helper = IGrallocHelper::singleton();
                        graphicBufferHandle = *const_cast<buffer_handle_t*>(pGraphicImageBufferHeap->getBufferHandlePtr());

                        if (kIsImportJpegOutGraphicBuffer) {
                            if  ( !helper
                               || !helper->importBuffer(graphicBufferHandle)
                               )
                            {
                                MY_LOGE("importBuffer (Out) fail, FrameNo:%d, StreamId %#" PRIx64 "!!",
                                               pFrame->getFrameNo(), mpOutJpeg->getStreamId());
                            } else {
                                MY_LOGI_IF(info.mLogLevel, "GraphicImageBuffer importBuffer success, FrameNo:%d, StreamId %#" PRIx64 "!!",
                                               pFrame->getFrameNo(), mpOutJpeg->getStreamId());
                                {
                                    int acquire_fence = -1;
                                    int release_fence = -1;
                                    std::string bufferName = "";

                                    sp<IImageStreamInfo> pStreamInfo = const_cast<IImageStreamInfo*>(pEncodeFrameNode->m_pOutJpeg_ImgStreamBuffer->getStreamInfo());
                                    pHeap = IGraphicImageBufferHeap::create(
                                            (bufferName + ":" + pStreamInfo->getStreamName()).c_str(),
                                            pStreamInfo->getUsageForAllocator(),
                                            pStreamInfo->getImgFormat() == eImgFmt_BLOB ? MSize(pStreamInfo->getAllocBufPlanes().planes[0].rowStrideInBytes, 1): pStreamInfo->getImgSize(),
                                            pStreamInfo->getImgFormat() == eImgFmt_JPEG ? eImgFmt_JPEG : pStreamInfo->getAllocImgFormat(),
                                            &graphicBufferHandle,
                                            acquire_fence,
                                            release_fence
                                        );

                                   MY_LOGI_IF( 1, "GraphicImageBuffer repack done, FrameNo:%d, StreamId %#" PRIx64 "!!",
                                                  pFrame->getFrameNo(), mpOutJpeg->getStreamId());
                                }
                            }
                        } else {
                            MY_LOGW_IF( 1, "Out Stream GraphicImageBuffer repack is disable, FrameNo:%d, StreamId %#" PRIx64 "!!",
                                           pFrame->getFrameNo(), mpOutJpeg->getStreamId());
                        }

                    } else {
                        MY_LOGW_IF( 1, "Out Stream is not GraphicImageBuffer, FrameNo:%d, StreamId %#" PRIx64 "!!",
                                       pFrame->getFrameNo(), mpOutJpeg->getStreamId());
                    }


                    pEncodeFrameNode->m_pOutJpeg_ImgStreamBuffer->unlock(getNodeName(), pImageBufferHeap.get());
                    if (pHeap != NULL) {
                        pImageBufferHeap = pHeap;
                    }

                    MARK_USER_STATUS(streamBufferSet, mpOutJpeg, getNodeId(), status);
                }

                // In App metadata
                {
                    MERROR const err = ensureMetaBufferAvailable_(
                            pFrame->getFrameNo(),
                            mpInAppMeta->getStreamId(),
                            streamBufferSet,
                            pEncodeFrameNode->m_pInMetaStream_Request /*rpStreamBuffer*/
                            );

                    if ( err != OK ) {
                        MY_LOGE("ensureMetaBufferAvailable_ (InApp) fail, FrameNo:%d, StreamId %#" PRIx64 "!!",
                                       pFrame->getFrameNo(), mpInAppMeta->getStreamId());
                        //return err;
                    }

                    MARK_USER_STATUS(streamBufferSet, mpInAppMeta, getNodeId(), status);
                }

                // In Hal Metadata
                {
                    MERROR const err = ensureMetaBufferAvailable_(
                            pFrame->getFrameNo(),
                            pEncodeFrameNode->m_pInHalMeta->getStreamId(),
                            streamBufferSet,
                            pEncodeFrameNode->m_pInMetaStream_Hal /*rpStreamBuffer*/
                            );

                    if ( err != OK ) {
                        MY_LOGE("ensureMetaBufferAvailable_ (InHal) fail, FrameNo:%d, StreamId %#" PRIx64 "!!",
                                       pFrame->getFrameNo(), pEncodeFrameNode->m_pInHalMeta->getStreamId());
                        //return err;
                    }

                    MARK_USER_STATUS(streamBufferSet, pEncodeFrameNode->m_pInHalMeta, getNodeId(), status);
                }

                // Out Metadata
                {
                    MERROR const err = ensureMetaBufferAvailable_(
                            pFrame->getFrameNo(),
                            mpOutMetaStreamInfo_Result->getStreamId(),
                            streamBufferSet,
                            pEncodeFrameNode->m_pOutMetaStream_Result /*rpStreamBuffer*/
                            );

                    if ( err != OK ) {
                        MY_LOGE("ensureMetaBufferAvailable_ (out) fail, FrameNo:%d, StreamId %#" PRIx64 "!!",
                                       pFrame->getFrameNo(), mpOutMetaStreamInfo_Result->getStreamId());
                        //return err;
                    }

                    {
                        if  ( pEncodeFrameNode->m_pOutMetaStream_Result == NULL ) {
                            MY_LOGE("[frame:%u node:%#" PRIxPTR "], streamID(%#" PRIx64 ")", pFrame->getFrameNo(), getNodeId(),
                                                                                    mpOutMetaStreamInfo_Result->getStreamId());
                            //return NAME_NOT_FOUND;
                        } else {
                            pEncodeFrameNode->m_pOutMetaStream_Result->markStatus(STREAM_BUFFER_STATUS::ERROR);
                        }
                    }

                    MARK_USER_STATUS(streamBufferSet, mpOutMetaStreamInfo_Result, getNodeId(), status);
                }

                //3. apply pre-release
                streamBufferSet.applyPreRelease(getNodeId());
                MY_LOGD("applyPreRelease FrameNo : %d, RequestNo : %d, PreReleaseNum : %d",
                          pFrame->getFrameNo(), pFrame->getRequestNo(), mInQueuePreReleaseRequestNum);
                //4. onDispatchFrame
                onDispatchFrame(pFrame);
                index++;
                it++;
            }
        }
        //
        // 4. resume deque request thread
        {
            std::lock_guard<std::mutex> _l(mRequestQueueLock);
            //mbRequestDequeSuspend = MFALSE;
            MY_LOGI("flush post mbRequestDequeSuspendCond!");
            sem_post(&mbRequestDequeSuspendCond);
        }
    }

    return OK;
}

MERROR
JpegNodeImp::
queue(android::sp<IPipelineFrame> pFrame)
{
    JpgLog();

    if( ! pFrame.get() ) {
        MY_LOGE("Null frame");
        return BAD_VALUE;
    }

    MY_LOGI("FrameNo : %d, RequestNo : %d", pFrame->getFrameNo(), pFrame->getRequestNo());

    std::lock_guard<std::mutex> _l(mRequestQueueLock);
    Que_T::iterator it = mRequestQueue.end();
    for (; it != mRequestQueue.begin(); ) {
        --it;
        if  ( 0 <= (MINT32)(pFrame->getFrameNo() - (*it)->getFrameNo()) ) {
            ++it;   //insert(): insert before the current node
            break;
        }
    }
    mRequestQueue.insert(it, pFrame);
    sem_post(&mRequestQueueCond);
    //
    return OK;
}

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
    // check if halImage enable
    if  ( ! rParams.pOutAppMeta.get() && ! property_get_int32("vendor.debug.camera.haljpeg",0) ) {
        MY_LOGE("no out app meta");
    }
    if  (  rParams.pOutJpeg.get() && NULL == rParams.pInYuv_Main.get() ) {
        MY_LOGE("no in hal main yuv image");
        return BAD_VALUE;
    }
    if  ( ! rParams.pOutJpeg.get() && ! rParams.pOutJpegAppSegment.get()) {
        MY_LOGE("no out hal jpeg image");
        return BAD_VALUE;
    }
    if (rParams.pOutJpeg.get() && rParams.pOutJpegAppSegment.get()) {
        MY_LOGE("cannot config Jpeg & JpegAppSegments in the same time");
        return BAD_VALUE;
    }
    //
    MY_LOGD_IF( rParams.pInAppMeta.get() && rParams.pOutAppMeta.get(),
            "stream: [meta] in app %#" PRIx64 ", out app %#" PRIx64,
            rParams.pInAppMeta->getStreamId(),
            rParams.pOutAppMeta->getStreamId()
            );
    MY_LOGD_IF( rParams.pInHalMeta_capture.get(),
            "stream: [meta] in hal capture %#" PRIx64,
            rParams.pInHalMeta_capture->getStreamId()
            );
    MY_LOGD_IF( rParams.pInHalMeta_streaming.get(),
            "stream: [meta] in hal streaming %#" PRIx64,
            rParams.pInHalMeta_streaming->getStreamId());
    MY_LOGD_IF(rParams.pInYuv_Main.get(),
            "stream: [img] in main %#" PRIx64,
            rParams.pInYuv_Main->getStreamId()
            );
    MY_LOGD_IF(rParams.pInYuv_Thumbnail.get(),
            "stream: [img] in thumbnail %#" PRIx64,
            rParams.pInYuv_Thumbnail->getStreamId()
            );
    MY_LOGD_IF(rParams.pInYuv_Main2.get(),
            "stream: [img] in main2 %#" PRIx64,
            rParams.pInYuv_Main2->getStreamId()
            );
    MY_LOGD_IF(rParams.pInYuv_Y16.get(),
            "stream: [img] in main2 %#" PRIx64,
            rParams.pInYuv_Y16->getStreamId()
            );
    MY_LOGD_IF(rParams.pOutJpeg.get(),
            "stream: [img] out Jpeg %#" PRIx64,
            rParams.pOutJpeg->getStreamId()
            );
    MY_LOGD_IF(rParams.pOutJpegAppSegment.get(),
            "stream: [img] out JpegAppSegments %#" PRIx64 "thumbnail size(%dx%d)",
            rParams.pOutJpegAppSegment->getStreamId(), rParams.heicYuvSize.w, rParams.heicYuvSize.h
            );
    //
    return OK;
}

MVOID
JpegNodeImp::
waitForRequestDrained()
{
    JpgLog();
    //
    std::lock_guard<std::mutex> _l(mRequestQueueLock);

    if( !mbRequestDrained )
    {
        MY_LOGD("wait for request drained + ");
        sem_wait(&mbRequestDrainedCond);
        MY_LOGD("wait for request drained - ");
        sem_post(&mbRequestDrainedCond);
    }
    //
}

MVOID
JpegNodeImp::
finalizeEncodeFrame(
    android::sp<IPipelineFrame>& rpFrame
)
{
    IStreamBufferSet& streamBufferSet = rpFrame->getStreamBufferSet();

    if (mIsPreReleaseRequest == MFALSE) {
        streamBufferSet.applyRelease(getNodeId());
        onDispatchFrame(rpFrame);
    }

}

MVOID
JpegNodeImp::
processJpeg(android::sp<IPipelineFrame>& pFrame, JpegNodeImp& jpg) {

    sp<jpeg_frame> pTriggerFrame = NULL;

    if (jpg.mIsAsyncWaitEncodeReqEnable == MTRUE)
    {
        MY_LOGI("mReqWaitDoneQueueStillFreeCond wait + ");
        sem_wait(&jpg.mReqWaitDoneQueueStillFreeCond);
        MY_LOGI("mReqWaitDoneQueueStillFreeCond wait - ");
    }

    //jpeg_frame config
    if (jpg.mIsPreReleaseRequest) {
        jpg.mOutputJpegStreamBuf = jpg.mpJpegFrame->m_pOutJpeg_ImgStreamBuffer;
        jpg.mOutputY16StreamBuf  = jpg.mpJpegFrame->m_pInY16_ImgStreamBuffer;
    }


    pTriggerFrame = new jpeg_frame(pFrame);

    pTriggerFrame->m_NodeId = jpg.getNodeId();

    // prepare encode data
    auto& reqList(pTriggerFrame->m_vReqList);
    reqList.clear();

    jpg.mapRequest(pFrame, reqList, jpg.info);

    // do encode
    auto& jpeg_threads(pTriggerFrame->m_vReqEncodeThreads);
    jpeg_threads.clear();

    for(size_t i = 0; i < reqList.size(); i++) {
        jpeg_threads.push_back(
                std::thread(onProcessRequest,
                    std::ref(jpg),
                    std::ref(reqList[i])));
    }

    // return if async wait encode request
    if (jpg.mIsAsyncWaitEncodeReqEnable == MTRUE)
    {
        {
            //enque jpeg_frame
            std::lock_guard<std::mutex> _l(jpg.mWaitDoneQueueLock);

            int size = jpg.mJpegWaitDoneQueue.size();

            if (jpg.mJpegWaitDoneQueue.empty())
            {
                MY_LOGD("wait mReqWaitDoneQueueIdleCond + ");
                sem_wait(&jpg.mReqWaitDoneQueueIdleCond);
                MY_LOGD("wait mReqWaitDoneQueueIdleCond - ");
            }
            jpg.mJpegWaitDoneQueue.push_back(pTriggerFrame);
            MY_LOGD("push request to mJpegWaitDoneQueue size %d -> %zu! ", size, jpg.mJpegWaitDoneQueue.size());

        }
        sem_post(&jpg.mReqNewTriggerEncodeCond);
        return;
    }

    MBOOL bIsEncodeError = MFALSE;
    for (size_t i = 0; i < jpeg_threads.size(); i++) {
        if (jpeg_threads[i].joinable())
            jpeg_threads[i].join();

        // check errors and do error handling
        if (reqList[i].mbIsEncodeJpegError == MTRUE) {
            MY_LOGE("encode error happens in req %zu!!", i);
            bIsEncodeError = MTRUE;
        }
    }
    //FIXME do Jpeg Pack flow
    vector<sp<IImageBufferHeap>> vHeap;

    if ((bIsEncodeError == MFALSE) && jpg.info.mbPackBitStream) {
        MY_LOGD("doPack +");
        vHeap.resize(2, NULL);
        vHeap[0] = jpg.mBufPool->getBufHeap(jpg.mpInYuv_main->getStreamId());
        vHeap[1] = jpg.mBufPool->getBufHeap(jpg.mpInYuv_main2->getStreamId());
        MUINT const usage = GRALLOC_USAGE_SW_READ_OFTEN;
        sp<IImageBuffer> buf1 = vHeap[0]->createImageBuffer();
        sp<IImageBufferHeap> InYuv_Y16Heap = NULL;
        if (buf1 != NULL) {
            MBOOL ret = buf1->lockBuf(LOG_TAG, usage);
            MY_LOGA_IF(!ret, "%s", "lockBuffer failed, please check VA usage!");
        }
        sp<IImageBuffer> buf2 = vHeap[1]->createImageBuffer();
        if (buf2 != NULL) {
            MBOOL ret = buf2->lockBuf(LOG_TAG, usage);
            MY_LOGA_IF(!ret, "%s", "lockBuffer failed, please check VA usage!");
        }
        if (jpg.mpOutJpeg != NULL) {
            jpg.getBufferAndLock(pFrame,
                    jpg.mpOutJpeg->getStreamId(),
                    jpg.mOutputJpegStreamBuf,
                    jpg.mOutputJpegBufferHeap,
                    jpg.mOutputJpegBuffer,
                    MFALSE);
        }
        if (jpg.mpInYuv_Y16 != NULL) {
            jpg.getBufferAndLock(pFrame,
                    jpg.mpInYuv_Y16_r->getStreamId(),
                    jpg.mOutputY16StreamBuf,
                    InYuv_Y16Heap,
                    jpg.mOutputY16Buffer,
                    MTRUE);
        }
        if (buf1 != NULL && buf2 != NULL &&
                jpg.mOutputJpegBuffer != NULL &&
                jpg.mpInYuv_Y16_r != NULL && jpg.mpJpgPack !=NULL) {
            // pack
            MY_LOGD_IF(jpg.info.mLogLevel, "buf1 for working jpeg: size(%d) va(%p) bitStream(%d)",
                    (int)buf1->getBufSizeInBytes(0),
                    (void*)buf1->getBufVA(0),
                    (int)buf1->getBitstreamSize());
            MY_LOGD_IF(jpg.info.mLogLevel, "buf2 for working jpeg: size(%d) va(%p) bitStream(%d)",
                    (int)buf2->getBufSizeInBytes(0),
                    (void*)buf2->getBufVA(0),
                    (int)buf2->getBitstreamSize());
            MY_LOGD_IF(jpg.info.mLogLevel, "mOutputY16Buffer : size(%d) va(%p)",
                    (int)jpg.mOutputY16Buffer->getBufSizeInBytes(0),
                    (void*)jpg.mOutputY16Buffer->getBufVA(0));
            MY_LOGD_IF(jpg.info.mLogLevel, "mOutputJpegBuffer : size(%d) va(%p)",
                    (int)jpg.mOutputJpegBuffer->getBufSizeInBytes(0),
                    (void*)jpg.mOutputJpegBuffer->getBufVA(0));
            JpgPack::ConfigJpgParams config(buf1, buf2,
                    jpg.mOutputY16Buffer, jpg.mOutputJpegBuffer);

            jpg.mpJpgPack->config(config);
            size_t packSize = jpg.mpJpgPack->process();
            jpg.mOutputJpegBufferHeap->setBitstreamSize(packSize);
            MY_LOGD_IF(jpg.info.mLogLevel, "after pack size : %d", (int)packSize);
            jpg.mpJpgPack->uninit();

            if (jpg.mIsPreReleaseRequest == MTRUE) {
                auto& pOutImgBufferHeap(jpg.mOutputJpegBufferHeap);
                sp<IGraphicImageBufferHeap> pGraphicImageBufferHeap = NULL;
                GrallocStaticInfo staticInfo = {};
                pGraphicImageBufferHeap = IGraphicImageBufferHeap::castFrom(pOutImgBufferHeap.get());
                if (pGraphicImageBufferHeap != NULL) {
                    int rc = IGrallocHelper::singleton()->query(*pGraphicImageBufferHeap->getBufferHandlePtr(), &staticInfo, NULL);
                    if (rc != OK) {
                        MY_LOGE("Fail to query blob buffer - rc:%d", rc);
                    } else {
                        MINTPTR jpegBuf = reinterpret_cast<MINTPTR>(pOutImgBufferHeap->getBufVA(0));
                        size_t jpegDataSize = pOutImgBufferHeap->getBitstreamSize();
                        size_t jpegBufSize = staticInfo.widthInPixels;
                        CameraBlob* pTransport = reinterpret_cast<CameraBlob*>(jpegBuf + jpegBufSize - sizeof(CameraBlob));
                        pTransport->blobId = CameraBlobId::JPEG;
                        pTransport->blobSize = jpegDataSize;
                        MY_LOGD("CameraBlob added (packed): frameNo:%d, jpegBuf:%#" PRIxPTR " bufsize:%zu datasize:%zu",
                                pFrame->getFrameNo(), jpegBuf, jpegBufSize, jpegDataSize);
                    }
                } else {
                    MY_LOGE("Fail to cast IGraphicImageBufferHeap !!");
                }
            }
        }
        // release Jpeg

        if (buf1 != NULL) {
            jpg.mBufPool->putBufHeap(jpg.mpInYuv_main->getStreamId(), buf1->getFD(0));
            buf1->unlockBuf(LOG_TAG);
        }
        if (buf2 != NULL) {
            jpg.mBufPool->putBufHeap(jpg.mpInYuv_main2->getStreamId(), buf2->getFD(0));
            buf2->unlockBuf(LOG_TAG);
        }

        if (jpg.info.muDumpBuffer) {
            if(!NSCam::Utils::makePath(JPEG_DUMP_PATH, 0660))
                MY_LOGI("makePath[%s] fails", JPEG_DUMP_PATH);

            NSCam::Utils::saveBufToFile(filename_pack,
                    (unsigned char*)jpg.mOutputJpegBuffer->getBufVA(0),
                    jpg.mOutputJpegBuffer->getBufSizeInBytes(0));

            MY_LOGI("[DUMP_JPG_PACKED] SaveFile[%s]!", filename_pack);
        }

        jpg.returnBuffer(pFrame, jpg.mOutputJpegStreamBuf,
                jpg.mOutputJpegBuffer);
        jpg.returnBuffer(pFrame, jpg.mOutputY16StreamBuf,
                jpg.mOutputY16Buffer);
        MY_LOGD("doPack -");
    } else {

        if (jpg.mIsPreReleaseRequest == MFALSE) {
            if (jpg.mvInYuv_main.size() > 1 && jpg.mvInYuv_main[1] != NULL) {
                IStreamBufferSet& streamBufferSet = pFrame->getStreamBufferSet();
                MY_LOGD_IF( 1, "mark unused stream release status: InYuvMain2!!");
                streamBufferSet.markUserStatus(
                        jpg.mvInYuv_main[1]->getStreamId(),
                        jpg.getNodeId(),
                        IUsersManager::UserStatus::RELEASE
                        );
            }
            if (jpg.mpInYuv_Y16_r != NULL) {
                IStreamBufferSet& streamBufferSet = pFrame->getStreamBufferSet();
                MY_LOGD_IF( 1, "mark unused stream release status: InY16!!");
                streamBufferSet.markUserStatus(
                        jpg.mpInYuv_Y16_r->getStreamId(),
                        jpg.getNodeId(),
                        IUsersManager::UserStatus::RELEASE
                        );
            }
        } else {
            if (jpg.mpJpegFrame->m_pIn2Yuv_ImgStreamBuffer) {
                auto& rpStreamBuffer(jpg.mpJpegFrame->m_pIn2Yuv_ImgStreamBuffer);
                MY_LOGD_IF( 1, "mark unused stream release status: InYuvMain2(pre-release)!!");
                rpStreamBuffer->markUserStatus(
                        jpg.mpJpegFrame->m_NodeId,
                        IUsersManager::UserStatus::RELEASE
                        );
            }
            if (jpg.mpJpegFrame->m_pInY16_ImgStreamBuffer) {
                auto& rpStreamBuffer(jpg.mpJpegFrame->m_pInY16_ImgStreamBuffer);
                MY_LOGD_IF( 1, "mark unused stream release status: InY16(pre-release)!!");
                rpStreamBuffer->markUserStatus(
                        jpg.mpJpegFrame->m_NodeId,
                        IUsersManager::UserStatus::RELEASE
                        );
            }
        }
    }

    //free graphic buffer
    if (jpg.mIsPreReleaseRequest == MTRUE) {
        auto& graphicBufferHandle(jpg.mpJpegFrame->m_GraphicBufferImportHandle);

        if (graphicBufferHandle != NULL) {

            if  ( auto helper = IGrallocHelper::singleton()) {

                //buffer_handle_t* bufferHandle = const_cast<buffer_handle_t*>(pGraphicImageBufferHeap->getBufferHandlePtr());
                if (kIsFreeJpegOutGraphicBuffer) {
                    helper->freeBuffer(graphicBufferHandle);
                    MY_LOGI_IF( 1, "GraphicImageBuffer freebuffer %p, fd %d, FrameNo %d!!", graphicBufferHandle, graphicBufferHandle->data[0], pFrame->getFrameNo());
                } else {
                    MY_LOGI_IF( 1, "GraphicImageBuffer skip freebuffer %p, fd %d, FrameNo %d!!", graphicBufferHandle, graphicBufferHandle->data[0], pFrame->getFrameNo());
                }
            } else {
                MY_LOGE_IF( 1, "GraphicImageBuffer get helper fail!!");
            }
        } else {
            MY_LOGW_IF( 1, "Out Stream is not GraphicImageBuffer, skip unmap graphic buffer!!");
        }
    }
    if (bIsEncodeError == MFALSE) {
        jpg.finalizeEncodeFrame(pFrame);
    } else {
        jpg.errorHandle(pFrame);
    }
    jpg.mOutputJpegStreamBuf = NULL;
    jpg.mOutputY16StreamBuf = NULL;
    jpg.mpJpegFrame = NULL;
}

MVOID
JpegNodeImp::
processJpegAppSegments(android::sp<IPipelineFrame>& pFrame, JpegNodeImp& jpg) {

    // prepare encode data
    vector<Request> reqList;
    jpg.mapRequest(pFrame, reqList, jpg.info);
    // do encode
    vector<std::thread> jpeg_threads;
    for(size_t i = 0; i < reqList.size(); i++) {
        jpeg_threads.push_back(
                std::thread(onProcessRequest,
                    std::ref(jpg),
                    std::ref(reqList[i])));
    }
    MBOOL bIsEncodeError = MFALSE;
    for (size_t i = 0; i < jpeg_threads.size(); i++) {
        if (jpeg_threads[i].joinable())
            jpeg_threads[i].join();

        // check errors and do error handling
        if (reqList[i].mbIsEncodeJpegError == MTRUE) {
            MY_LOGE("encode error happens in req %zu!!", i);
            bIsEncodeError = MTRUE;
        }
    }

    if (bIsEncodeError == MFALSE) {
        jpg.finalizeEncodeFrame(pFrame);
    } else {
        jpg.errorHandle(pFrame);
    }
    jpg.mpJpegFrame = NULL;
}

MERROR
JpegNodeImp::
mapRequest(
    android::sp<IPipelineFrame>& rpFrame,
    vector<Request>& reqList,
    static_info& info)
{
    std::vector<sp<IImageStreamBuffer>> vpInMainYUV_StreamBuffer;
    std::vector<sp<IImageStreamBuffer>> vpInThumbYUV_StreamBuffer;

    mpInYuv_Y16_r = NULL;

    for (size_t i = 0; i < mvInYuv_main.size(); i++)
        mvInYuv_main[i] = NULL;

    for (size_t i = 0; i < mvInYuv_thumbnail.size(); i++)
        mvInYuv_thumbnail[i] = NULL;

    if(mIsPreReleaseRequest == MTRUE) {
        vpInMainYUV_StreamBuffer.resize(2);
        vpInThumbYUV_StreamBuffer.resize(1);
    }

    IPipelineFrame::InfoIOMapSet IOMapSet;
    if(
            OK != rpFrame->queryInfoIOMapSet( getNodeId(), IOMapSet )
            || IOMapSet.mImageInfoIOMapSet.size() != 1
            || IOMapSet.mMetaInfoIOMapSet.size() != 1
      ) {
        MY_LOGA("queryInfoIOMap failed, IOMap img/meta: %zu/%zu",
                IOMapSet.mImageInfoIOMapSet.size(),
                IOMapSet.mMetaInfoIOMapSet.size()
               );
    }

    // meta
    {
        // hal meta stream
        IPipelineFrame::MetaInfoIOMap const& metaIOMap = IOMapSet.mMetaInfoIOMapSet[0];
        for (size_t i = 0; i < metaIOMap.vIn.size(); i++) {
            StreamId_T const streamId = metaIOMap.vIn.keyAt(i);
            auto search = mHalMetaMap.find((int)streamId);
            if (search != mHalMetaMap.end()) {
                MY_LOGD_IF(info.mLogLevel, "StreamId : %#" PRIx64 "", streamId);
                mvInHalMeta.push_back(search->second);
            }
        }
        MY_LOGA_IF(mvInHalMeta.size() == 0, "PipelineContext doesn't setup input hal meta");
        for (size_t i = 0; i < metaIOMap.vIn.size(); i++) {
            StreamId_T const streamId = metaIOMap.vIn.keyAt(i);
            if (isStream(mpInAppMeta, streamId)) {
                MY_LOGD_IF(info.mLogLevel, "StreamId : %#" PRIx64 "", streamId);
                mvInAppMeta.push_back(mpInAppMeta);
            }
        }
        MY_LOGA_IF(mvInAppMeta.size() == 0, "PipelineContext doesn't setup input app meta");
    }

    // image
    {
        MBOOL needMain2 = MFALSE;
        MBOOL needY16 = MFALSE;
        IPipelineFrame::ImageInfoIOMap const& imageIOMap = IOMapSet.mImageInfoIOMapSet[0];
        for( size_t i = 0; i < imageIOMap.vIn.size(); i++ )
        {
            StreamId_T const streamId = imageIOMap.vIn.keyAt(i);
            if(isStream(mpInYuv_thumbnail, streamId)) {
                MY_LOGD_IF(info.mLogLevel, "accept Thumbnail(%#" PRIx64 ")!", streamId);
                sp<IImageStreamInfo> yuv_thumb = imageIOMap.vIn.valueAt(i);
                mvInYuv_thumbnail[0] = yuv_thumb;
                if (mIsPreReleaseRequest == MTRUE) {
                    vpInThumbYUV_StreamBuffer[0] = mpJpegFrame->m_pInThumbYuv_ImgStreamBuffer;
                }
            }
            if (!mbAppnOnly && isStream(mpInYuv_main, streamId)) {
                MY_LOGD_IF(info.mLogLevel, "accept YUV Main1(%#" PRIx64 ")!", streamId);
                sp<IImageStreamInfo> yuv_main = imageIOMap.vIn.valueAt(i);
                mvInYuv_main[0] = yuv_main;
                if (mIsPreReleaseRequest == MTRUE) {
                    vpInMainYUV_StreamBuffer[0] = mpJpegFrame->m_pInYuv_ImgStreamBuffer;
                }
            }
            if (!mbAppnOnly && isStream(mpInYuv_main2, streamId)) {
                MY_LOGD_IF(info.mLogLevel, "accept YUV Main2(%#" PRIx64 ")!", streamId);
                sp<IImageStreamInfo> yuv_main2 = imageIOMap.vIn.valueAt(i);
                mvInYuv_main[1] = yuv_main2;
                if (mIsPreReleaseRequest == MFALSE) {
                    if (checkBufferAvailable(rpFrame, yuv_main2->getStreamId()) == OK) {
                        needMain2 = MTRUE;
                    } else {
                        MY_LOGW("Fail to get main2 stream, set Pack hint as false");
                    }
                } else {
                    // pre-release flow
                    auto& rpStreamBuffer(mpJpegFrame->m_pIn2Yuv_ImgStreamBuffer);
                    vpInMainYUV_StreamBuffer[1] = rpStreamBuffer;

                    if (mpJpegFrame->m_bPackBitStream == MTRUE) {
                        sp<IImageBufferHeap> pImageBufferHeap = NULL;
                        MY_LOGD("check streamId: %#" PRIx64 ", try R/W Lock + ", streamId);
                        pImageBufferHeap = rpStreamBuffer->tryReadLock(mNodeName, true);
                        MY_LOGD("check streamId: %#" PRIx64 ", try R/W Lock - ", streamId);

                        //  Check buffer status.
                        if  ( rpStreamBuffer->hasStatus(STREAM_BUFFER_STATUS::ERROR) ) {
                            //  The producer ahead of this user may fail to render this buffer's content.
                            MY_LOGE("[frame:%u node:%#" PRIxPTR "][stream buffer:%s] bad status:%d",
                                    rpFrame->getFrameNo(), getNodeId(),
                                    rpStreamBuffer->getName(), rpStreamBuffer->getStatus());
                            MY_LOGW("mbPackBitStream %d, set Pack hint to false due to bad status!", mpJpegFrame->m_bPackBitStream);
                            mpJpegFrame->m_bPackBitStream = MFALSE;
                        }
                        rpStreamBuffer->unlock(getNodeName(), pImageBufferHeap.get());
                    }
                }
            }
            if (!mbAppnOnly && isStream(mpInYuv_Y16, streamId)) {
                MY_LOGD_IF(info.mLogLevel, "accept YUV Y16(%#" PRIx64 ")!", streamId);
                sp<IImageStreamInfo> yuv_y16 = imageIOMap.vIn.valueAt(i);
                mpInYuv_Y16_r = yuv_y16;
                if (mIsPreReleaseRequest == MFALSE) {
                    if (checkBufferAvailable(rpFrame, mpInYuv_Y16_r->getStreamId()) == OK) {
                        needY16 = MTRUE;
                    } else {
                        MY_LOGW("fail to get y16 buffer, set Pack hint as false");
                    }
                } else {
                    // pre-release flow
                    auto& rpStreamBuffer(mpJpegFrame->m_pInY16_ImgStreamBuffer);

                    if (mpJpegFrame->m_bPackBitStream == MTRUE) {
                        sp<IImageBufferHeap> pImageBufferHeap = NULL;
                        MY_LOGI("check streamId: %#" PRIx64 ", try R/W Lock + ", streamId);
                        pImageBufferHeap = rpStreamBuffer->tryReadLock(mNodeName, true);
                        MY_LOGI("check streamId: %#" PRIx64 ", try R/W Lock - ", streamId);

                        //  Check buffer status.
                        if  ( rpStreamBuffer->hasStatus(STREAM_BUFFER_STATUS::ERROR) ) {
                            //  The producer ahead of this user may fail to render this buffer's content.
                            MY_LOGE("[frame:%u node:%#" PRIxPTR "][stream buffer:%s] bad status:%d",
                                    rpFrame->getFrameNo(), getNodeId(),
                                    rpStreamBuffer->getName(), rpStreamBuffer->getStatus());
                            MY_LOGW("mbPackBitStream %d, set Pack hint to false due to bad status!", mpJpegFrame->m_bPackBitStream);
                            mpJpegFrame->m_bPackBitStream = MFALSE;
                        }
                        rpStreamBuffer->unlock(getNodeName(), pImageBufferHeap.get());
                    }
                }
            }
        }
        // pre-release flow
        if (mIsPreReleaseRequest == MTRUE) {
            info.mbPackBitStream = mpJpegFrame->m_bPackBitStream;
            MY_LOGW("mbPackBitStream %d, set Pack hint in Pre-Release frame!", info.mbPackBitStream);
        } else {
            if(needMain2 && needY16)
                info.mbPackBitStream = MTRUE;
            else
                info.mbPackBitStream = MFALSE;
        }
    }

    // create Request
    if (!mbAppnOnly) {
        for (size_t i = 0; i < mvInYuv_main.size(); i++) {
            // only Main encode thread(i == 0) needs to parse meta
            MBOOL skipParseMeta = (i != 0);

            if (mvInYuv_main[i] == NULL) {
                continue;
            }

            if ((i == 1) && (info.mbPackBitStream == MFALSE)) {
                MY_LOGW("mbPackBitStream %d, skip encode request %zd!!", (int)info.mbPackBitStream, i);
                continue;
            }

            Request r(mvInYuv_main[i], mvInYuv_thumbnail[i], mpOutJpeg,
                    mvInAppMeta[0], mvInHalMeta[0], mpOutMetaStreamInfo_Result,
                    rpFrame, getNodeId(), getNodeName(), info, mBufPool, skipParseMeta, mbAppnOnly, MSize(0, 0));

            // pre-release flow
            if (mIsPreReleaseRequest == MTRUE) {
                if(i < vpInMainYUV_StreamBuffer.size())
                    r.mpInMainImageStreamBuffer  = vpInMainYUV_StreamBuffer[i];
                if(i < vpInThumbYUV_StreamBuffer.size())
                    r.mpInThumbImageStreamBuffer = vpInThumbYUV_StreamBuffer[i];
                r.mpOutJpegImageStreamBuffer = mpJpegFrame->m_pOutJpeg_ImgStreamBuffer;

                r.mpInHalMetaStreamBuffer    = mpJpegFrame->m_pInMetaStream_Hal;
                r.mpInAppMetaStreamBuffer    = mpJpegFrame->m_pInMetaStream_Request;
                r.mpOutAppMetaStreamBuffer   = mpJpegFrame->m_pOutMetaStream_Result;
                r.mbIsEncodePreRelease       = mIsPreReleaseRequest;
            }
            reqList.push_back(r);
        }
    } else {
        if (mvInYuv_thumbnail.size() > 0) {
            sp<IImageStreamInfo> mainInfo = NULL;
            if (mvInYuv_thumbnail[0] == NULL) {
                MY_LOGD("app not set thumbnail size, so streamInfo is NULL");
            }
            Request r(mainInfo, mvInYuv_thumbnail[0], mpOutJpegAppSegments,
                mvInAppMeta[0], mvInHalMeta[0], mpOutMetaStreamInfo_Result,
                rpFrame, getNodeId(), getNodeName(), info, mBufPool, MFALSE, mbAppnOnly, mHeicYuvSize);
            reqList.push_back(r);
        } else {
            MY_LOGE("Error iomap setting no thumbnail input");
            return BAD_VALUE;
        }
    }

    return OK;
}

MERROR
JpegNodeImp::
checkBufferAvailable(
    android::sp<IPipelineFrame> const& pFrame,
    StreamId_T const streamId
)
{
    IStreamBufferSet& rStreamBufferSet = pFrame->getStreamBufferSet();
    sp<IImageStreamBuffer> rpStreamBuffer = NULL;
    MERROR const err = ensureImageBufferAvailable_(
                pFrame->getFrameNo(),
                streamId,
                rStreamBufferSet,
                rpStreamBuffer
                );
    if (err != OK) {
        return err;
    }

    return OK;
}

MERROR
JpegNodeImp::
errorHandle(
    android::sp<IPipelineFrame> const& pFrame
)
{
    MY_LOGE("Discard frameNo=%d", pFrame->getRequestNo());

    MERROR err = BaseNode::flush(pFrame);

    return err;
}

MERROR
JpegNodeImp::
getBufferAndLock(
        android::sp<IPipelineFrame> const& pFrame,
        StreamId_T const streamId,
        sp<IImageStreamBuffer>& rpStreamBuffer,
        sp<IImageBufferHeap>& rpImageBufferHeap,
        sp<IImageBuffer>& rpImageBuffer,
        MBOOL isIn
)
{
    IStreamBufferSet& rStreamBufferSet = pFrame->getStreamBufferSet();
    sp<IImageBufferHeap> pImageBufferHeap = NULL;
    if(NULL == pImageBufferHeap.get()) {
        if(NULL == rpStreamBuffer.get()) {
            MERROR const err = ensureImageBufferAvailable_(
                    pFrame->getFrameNo(),
                    streamId,
                    rStreamBufferSet,
                    rpStreamBuffer
                    );
            if( err != OK )
                return err;
        }
        MY_LOGD_IF(info.mLogLevel, "getBufferAndLock streamId: %#" PRIx64 ", isIn: %d, try R/W Lock + ", streamId, isIn);
        pImageBufferHeap = isIn ?
            rpStreamBuffer->tryReadLock(getNodeName(), true) :
            rpStreamBuffer->tryWriteLock(getNodeName());
        MY_LOGD_IF(info.mLogLevel, "getBufferAndLock streamId: %#" PRIx64 ", isIn: %d, try R/W Lock - ", streamId, isIn);
        if (NULL == pImageBufferHeap.get()) {
            MY_LOGE("rpImageBufferHeap is NULL");
            return BAD_VALUE;
        }
        MUINT groupUsage = rpStreamBuffer->queryGroupUsage(getNodeId());
        rpImageBuffer = pImageBufferHeap->createImageBuffer();
        if (rpImageBuffer == NULL) {
            rpStreamBuffer->unlock(getNodeName(), pImageBufferHeap.get());
            MY_LOGE("rpImageBuffer == NULL");
            return BAD_VALUE;
        }
        groupUsage |= eBUFFER_USAGE_SW_WRITE_OFTEN;
        MBOOL ret = rpImageBuffer->lockBuf(getNodeName(), groupUsage);
        MY_LOGA_IF(!ret, "%s", "lockBuffer failed, please check VA usage!");

        rpImageBufferHeap = pImageBufferHeap;
        MY_LOGD_IF(info.mLogLevel, "stream buffer: (%#" PRIx64 ") %p, heap: %p, buffer: %p, usage: %u",
                streamId, rpStreamBuffer.get(), pImageBufferHeap.get(), rpImageBuffer.get(), groupUsage);
    }
    return OK;
}

MERROR
JpegNodeImp::
returnBuffer(
        android::sp<IPipelineFrame> const& pFrame,
        sp<IImageStreamBuffer>& rpStreamBuffer,
        sp<IImageBuffer>& rpImageBuffer
)
{
    if (rpStreamBuffer == NULL || rpImageBuffer == NULL)
        return BAD_VALUE;
    IStreamBufferSet& streamBufferSet = pFrame->getStreamBufferSet();
    rpImageBuffer->unlockBuf(mNodeName);
    rpStreamBuffer->unlock(mNodeName, rpImageBuffer->getImageBufferHeap());

    if (mIsPreReleaseRequest == MFALSE) {
        streamBufferSet.markUserStatus(
                rpStreamBuffer->getStreamInfo()->getStreamId(),
                mNodeId,
                IUsersManager::UserStatus::USED |
                IUsersManager::UserStatus::RELEASE
                );
    } else {
        rpStreamBuffer->markUserStatus(
                getNodeId(),
                IUsersManager::UserStatus::USED |
                IUsersManager::UserStatus::RELEASE
                );
    }
    return OK;

}

MVOID
Request::
getJpegParams(
    IMetadata* pMetadata_request,
    jpeg_params& rParams
    ) const
{
    if( NULL == pMetadata_request)
    {
        MY_LOGE("pMetadata_request=NULL");
        return;
    }
    rParams.gpsCoordinates =
        pMetadata_request->entryFor(MTK_JPEG_GPS_COORDINATES);

    rParams.gpsProcessingMethod =
        pMetadata_request->entryFor(MTK_JPEG_GPS_PROCESSING_METHOD);

    rParams.gpsTimestamp =
        pMetadata_request->entryFor(MTK_JPEG_GPS_TIMESTAMP);

#define getParam(meta, tag, type, param, needWarn)      \
    do {                                                \
        if( !tryGetMetadata<type>(meta, tag, param) ) { \
            if (needWarn) MY_LOGW("no tag: %s", #tag);  \
            else MY_LOGW_IF( mInfo.mLogLevel,"no tag: %s", #tag); \
        }                                               \
    } while(0)
#define getAppParam(tag, type, param ,needWarn) getParam(pMetadata_request, tag, type, param, needWarn)

    // request from app
    getAppParam(MTK_JPEG_ORIENTATION      , MINT32, rParams.orientation, MTRUE);
    getAppParam(MTK_JPEG_QUALITY          , MUINT8, rParams.quality, MTRUE);
    getAppParam(MTK_JPEG_THUMBNAIL_QUALITY, MUINT8, rParams.quality_thumbnail, MTRUE);
    getAppParam(MTK_JPEG_THUMBNAIL_SIZE   , MSize , rParams.size_thumbnail, MTRUE);
    getAppParam(MTK_SCALER_CROP_REGION    , MRect , rParams.cropRegion, MTRUE);

    getAppParam(MTK_CONTROL_CAPTURE_JPEG_FLIP_MODE  , MINT32, rParams.flipMode, MFALSE);

#undef getAppParam
#undef getParam

}

MVOID
JpegNodeImp::
encodeThumbnailJpeg(
    sp<encode_frame>& pEncodeFrame)
{
    {
        // set thread naming
        pthread_setname_np(pthread_self(), THUMBTHREAD_NAME);
    }
    MSize thumbsize = pEncodeFrame->mParams.size_thumbnail;
    MUINT32 transform = (pEncodeFrame->mpYUV_MainStreamInfo.get()) ? pEncodeFrame->mpYUV_MainStreamInfo->getTransform() : 0;
    // do encode
    {
        my_encode_params params;
        params.pSrc = pEncodeFrame->mpYUV_Thumbnail.get();
        params.pDst = pEncodeFrame->mpJpeg_Thumbnail.get();
        if (pEncodeFrame->mParams.flipMode || info.mFlip) {
            if( pEncodeFrame->mParams.orientation == 90 && transform & eTransform_ROT_90)
            {
                params.transform = eTransform_ROT_90 | eTransform_FLIP_V;
                std::swap(thumbsize.w, thumbsize.h);
            } else if (pEncodeFrame->mParams.orientation == 180 && transform & eTransform_ROT_180)
            {
                params.transform = eTransform_FLIP_V;
            } else if (pEncodeFrame->mParams.orientation == 270 && transform & eTransform_ROT_270)
            {
                params.transform = eTransform_ROT_90 | eTransform_FLIP_H;
                std::swap(thumbsize.w, thumbsize.h);
            } else
            {
                params.transform = eTransform_FLIP_H;
            }

#if MTKCAM_EARLY_FLIP_SUPPORT == 1
            // has flipped in P2A, need to un-flip
            auto doUnflip= [&](MUINT32& trans, MINT32 dvOri) -> MVOID {
                if (dvOri == 270) {
                    MY_LOGD("dvOri %d, Capture vertical", dvOri);
                    trans |= eTransform_FLIP_V;
                } else if (dvOri == 90) {
                    MY_LOGD("dvOri %d, Capture vertical", dvOri);
                    trans &= ~eTransform_FLIP_V;
                } else if (dvOri == 180) {
                    MY_LOGD("dvOri %d, Capture horizontal",dvOri);
                    trans |= eTransform_FLIP_H;
                } else if (dvOri == 0) {
                    MY_LOGD("dvOri %d, Capture horizontal",dvOri);
                    trans &= ~eTransform_FLIP_H;
                }
            };

            if (pEncodeFrame->mParams.isFlipped) {
                MY_LOGD("before trans 0x%" PRIx64, params.transform);
                doUnflip(params.transform, pEncodeFrame->mParams.orientation);
                MY_LOGD("after trans 0x%" PRIx64, params.transform);
            }
#endif
        } else {
            if( pEncodeFrame->mParams.orientation == 90 && transform & eTransform_ROT_90)
            {
                params.transform = eTransform_ROT_90;
                std::swap(thumbsize.w, thumbsize.h);
            } else if (pEncodeFrame->mParams.orientation == 180 && transform & eTransform_ROT_180)
            {
                params.transform = eTransform_ROT_180;
            } else if (pEncodeFrame->mParams.orientation == 270 && transform & eTransform_ROT_270)
            {
                params.transform = eTransform_ROT_270;
                std::swap(thumbsize.w, thumbsize.h);
            } else
            {
                params.transform = 0;
            }
        }

        params.crop =
            calCropAspect(pEncodeFrame->mpYUV_Thumbnail->getImgSize(), thumbsize);
        params.isSOI = 1;
        params.quality = pEncodeFrame->mParams.quality_thumbnail;
        params.codecType = NSSImager::JPEGENC_SW;
        //
        MINT32 quality = params.quality;
        while(quality > 0) {
            MERROR const err = hardwareOps_encode(params);
            if( err != OK ) {
                MY_LOGE("thumb encode fail src %p, fmt 0x%x, dst %zx, fmt 0x%x",
                    params.pSrc, params.pSrc->getImgFormat(),
                    params.pDst->getBufVA(0), params.pDst->getImgFormat());
                pEncodeFrame->mbSuccess = MFALSE;
            } else {
                if (pEncodeFrame->thumbnailMaxSize < params.pDst->getBitstreamSize()) {
                    if (params.pDst->getBitstreamSize() >
                        (pEncodeFrame->thumbnailMaxSize + pEncodeFrame->exif.getDbgExifSize())) {
                        MY_LOGE("Thumbnail over encode! encode bitstreamSize");
                    } else {
                        MY_LOGW("Thumbnail bitStream size is too big, scale down quality and re-encode again!");
                        quality -= 10;
                        if (quality > 0) {
                            params.quality = quality;
                        }
                        continue;
                    }
                }
            }
            break;
        }
        if (quality <= 0 || !pEncodeFrame->mbSuccess) {
            MY_LOGE("Thumbnail encode fail!");
        }
    }
}

MVOID
JpegNodeImp::
encodeMainJpeg(
    sp<encode_frame>& pEncodeFrame)
{
    MINT8 iEncTypeCheck = isHwEncodeSupported(pEncodeFrame->mpYUV_Main->getImgFormat()) ?
                          NSSImager::JPEGENC_HW_ONLY : NSSImager::JPEGENC_SW;
    my_encode_params params;
    params.pSrc = pEncodeFrame->mpYUV_Main.get();
    params.pDst = pEncodeFrame->mpJpeg_Main.get();
    params.transform = 0;
    params.crop = MRect(MPoint(0,0), pEncodeFrame->mpYUV_Main->getImgSize());
    params.isSOI = (!pEncodeFrame->mbSkipParseMeta) ? 0 : 1;
    params.quality = pEncodeFrame->mParams.quality;
    if(pEncodeFrame->miJpegEncType != -1){
        iEncTypeCheck = pEncodeFrame->miJpegEncType;
    }
    if (pEncodeFrame->muJpegSwMode > 0)
    {
        iEncTypeCheck = NSSImager::JPEGENC_SW;
    }
    params.codecType = iEncTypeCheck;
    //
    MERROR const err = hardwareOps_encode(params);
    if( err != OK ) {
        pEncodeFrame->mbSuccess = MFALSE;
    }

    pEncodeFrame->mpOutImgBufferHeap->setBitstreamSize(
        params.pDst->getBitstreamSize());

    if (info.muDumpBuffer) {
        char filename[256] = {0};
        sprintf(filename, "/sdcard/DCIM/Camera/Buffer_frame%d_%dx%d_%d.jpg",
                pEncodeFrame->mpFrame->getFrameNo(),
                pEncodeFrame->mpJpeg_Main->getImgSize().w,
                pEncodeFrame->mpJpeg_Main->getImgSize().h,
                pEncodeFrame->mpJpeg_Main->getFD(0)
               );
        NSCam::Utils::saveBufToFile(filename,
                (unsigned char*)pEncodeFrame->mpJpeg_Main->getBufVA(0),
                pEncodeFrame->mpJpeg_Main->getBufSizeInBytes(0));
    }

}

MVOID
JpegNodeImp::
encodeExif(
    sp<encode_frame>& pEncodeFrame)
{
    if (pEncodeFrame->mbSkipParseMeta) {
        MY_LOGD_IF(info.mLogLevel, "SKIP encodeExif for main2 request");
        return;
    }
    sp<IImageStreamBuffer> pOutImgStreamBuffer =
        pEncodeFrame->mpOutImgStreamBuffer;
    sp<IImageBuffer> pOutImageBuffer = NULL;

    if (mbAppnOnly) {
        pOutImageBuffer =
            pEncodeFrame->mpOutImgBufferHeap->createImageBuffer_FromBlobHeap(0,
                    pEncodeFrame->exif.getHeaderSize());
    } else {
        pOutImageBuffer =
            pEncodeFrame->mpOutImgBufferHeap->createImageBuffer();
    }

    if (pOutImageBuffer.get() == NULL) {
        MY_LOGE("rpImageBuffer == NULL");
        return;
    }

    MUINT const usage = GRALLOC_USAGE_SW_READ_OFTEN | GRALLOC_USAGE_SW_WRITE_OFTEN;
    MBOOL ret = pOutImageBuffer->lockBuf(getNodeName(), usage);
    MY_LOGA_IF(!ret, "%s", "lockBuffer failed, please check VA usage!");

    size_t exifSize  = 0;
    MINT8 * pExifBuf = reinterpret_cast<MINT8*>(pOutImageBuffer->getBufVA(0));
    if( pExifBuf == NULL
            || OK != ExifUtils::makeExifHeader(pEncodeFrame, pExifBuf, exifSize, mbAppnOnly)
      ) {
        pEncodeFrame->mbSuccess = MFALSE;
        MY_LOGE("frame %u make exif header failed: buf %p, size %zu",
                pEncodeFrame->mpFrame->getFrameNo(),
                pExifBuf, exifSize);
    }
    if( pExifBuf )
        pExifBuf = NULL;

    size_t JpegBitStreamSize = 0;
    if (pEncodeFrame->mpJpeg_Main != NULL) {
        JpegBitStreamSize = pEncodeFrame->exif.getHeaderSize() +
            pEncodeFrame->mpJpeg_Main->getBitstreamSize();
        pEncodeFrame->mpJpeg_Main->setBitstreamSize(
                JpegBitStreamSize);
    } else {
        JpegBitStreamSize = pEncodeFrame->exif.getHeaderSize();
    }

    pEncodeFrame->mpOutImgBufferHeap->setBitstreamSize(
            JpegBitStreamSize);

    //if (mIsPreReleaseRequest == MTRUE)
    {
        auto& pOutImgBufferHeap(pEncodeFrame->mpOutImgBufferHeap);
        sp<IGraphicImageBufferHeap> pGraphicImageBufferHeap = NULL;
        GrallocStaticInfo staticInfo = {};
        pGraphicImageBufferHeap = IGraphicImageBufferHeap::castFrom(pOutImgBufferHeap.get());
        if (pGraphicImageBufferHeap != NULL) {
            int rc = IGrallocHelper::singleton()->query(*pGraphicImageBufferHeap->getBufferHandlePtr(), &staticInfo, NULL);
            if (rc != OK) {
                MY_LOGE("Fail to query blob buffer - rc:%d", rc);
            } else {
                MINTPTR jpegBuf = reinterpret_cast<MINTPTR>(pOutImgBufferHeap->getBufVA(0));
                size_t jpegDataSize = pOutImgBufferHeap->getBitstreamSize();
                size_t jpegBufSize = staticInfo.widthInPixels;
                CameraBlob* pTransport = reinterpret_cast<CameraBlob*>(jpegBuf + jpegBufSize - sizeof(CameraBlob));
                pTransport->blobId = CameraBlobId::JPEG;
                pTransport->blobSize = jpegDataSize;
                MY_LOGD("CameraBlob added: frameNo:%d, jpegBuf:%#" PRIxPTR " bufsize:%zu datasize:%zu",
                        pEncodeFrame->mpFrame->getFrameNo(), jpegBuf, jpegBufSize, jpegDataSize);
            }
        } else {
            MY_LOGE("Fail to cast IGraphicImageBufferHeap !!");
        }
    }

    pOutImageBuffer->unlockBuf(getNodeName());

    if (info.muDumpBuffer && pEncodeFrame->mpJpeg_Main != NULL) {
        sp<IImageStreamBuffer> pStreamBuffer =
            pEncodeFrame->mpOutImgStreamBuffer;
        int jpeg_size = pEncodeFrame->exif.getHeaderSize() +
            pEncodeFrame->mpJpeg_Main->getBitstreamSize();
        sp<IImageBuffer> dumpImgBuffer =
            pEncodeFrame->mpOutImgBufferHeap->createImageBuffer_FromBlobHeap(0, jpeg_size);
        if(dumpImgBuffer.get() == NULL) {
            MY_LOGE("dumpBuffer == NULL");
            return;
        }

        MUINT groupUsage = 0;
        if (pStreamBuffer != NULL) {
            groupUsage = pStreamBuffer->queryGroupUsage(getNodeId());
            groupUsage |= eBUFFER_USAGE_SW_READ_OFTEN;
        } else {
            groupUsage |= eBUFFER_USAGE_SW_READ_OFTEN;
        }
        MBOOL ret = dumpImgBuffer->lockBuf(getNodeName(), groupUsage);
        MY_LOGA_IF(!ret, "%s", "lockBuffer failed, please check VA usage!");

        if(!NSCam::Utils::makePath(JPEG_DUMP_PATH, 0660))
            MY_LOGI("makePath[%s] fails", JPEG_DUMP_PATH);

        MBOOL rets = dumpImgBuffer->saveToFile(filename);
        MY_LOGI("[DUMP_JPG] SaveFile[%s]:(%d)", filename, rets);

        dumpImgBuffer->unlockBuf(getNodeName());


    }

    if (pOutImgStreamBuffer != NULL) {
        pOutImgStreamBuffer->unlock(getNodeName(), pOutImageBuffer->getImageBufferHeap());
        pOutImgStreamBuffer->markUserStatus(
                getNodeId(),
                IUsersManager::UserStatus::USED |
                IUsersManager::UserStatus::RELEASE
                );

    }
}

MERROR
JpegNodeImp::
hardwareOps_encode(
    my_encode_params& rParams
)
{
    CAM_TRACE_NAME(__FUNCTION__);
    MY_LOGD("+ src: %dx%d, dst: %dx%d, format: 0x%x, crop(%d,%d;%d,%d)",rParams.pSrc-> getImgSize().w, rParams.pSrc-> getImgSize().h,
            rParams.pDst-> getImgSize().w, rParams.pDst-> getImgSize().h,
            rParams.pDst->getImgFormat(),
            rParams.crop.p.x, rParams.crop.p.y,
            rParams.crop.s.w, rParams.crop.s.h
           );

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
                rParams.codecType,
                false
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
    MY_LOGD("- bistream size %zu. Check bit: 0x%x 0x%x", rParams.pDst->getBitstreamSize(),
            (*(reinterpret_cast<MUINT8*>(rParams.pDst->getBufVA(0)))),
            *(reinterpret_cast<MUINT8*>(rParams.pDst->getBufVA(0))+1));
    return OK;
}

MVOID
Request::
unlockImage(
    sp<IImageStreamBuffer>& rpStreamBuffer,
    sp<IImageBuffer>& rpImageBuffer
)
{
    if( rpStreamBuffer == NULL || rpImageBuffer == NULL ) {
        MY_LOGE("rpStreamBuffer %p, rpImageBuffer %p should not be NULL",
                rpStreamBuffer.get(), rpImageBuffer.get());
        return;
    }
    rpImageBuffer->unlockBuf(mNodeName);
    rpStreamBuffer->unlock(mNodeName, rpImageBuffer->getImageBufferHeap());
}

MERROR
Request::
getInImageBuffer(
        android::sp<IPipelineFrame> const& pFrame,
        StreamId_T const streamId,
        sp<IImageStreamBuffer>& rpStreamBuffer,
        sp<IImageBuffer>& rpImageBuffer,
        sp<IImageStreamInfo>& rpImageStreamInfo
        )
{
    IStreamBufferSet& rStreamBufferSet = pFrame->getStreamBufferSet();
    sp<IImageBufferHeap> pImageBufferHeap = NULL;

    if (rpStreamBuffer.get() == NULL) {
        MERROR const err = acquireImageBuffer(
                pFrame->getFrameNo(),
                streamId,
                rStreamBufferSet,
                rpStreamBuffer
                );

        if( err != OK )
            return err;
        if (rpStreamBuffer == NULL) {
            MY_LOGE("acquireImageBuffer failed! rpStreamBuffer is NULL");
            return BAD_VALUE;
        }
    }

    //  Query the group usage.
    MUINT groupUsage = rpStreamBuffer->queryGroupUsage(mNodeId);

    MY_LOGD_IF(mInfo.mLogLevel, "getInImageBuffer streamId: %#" PRIx64 ", try R/W Lock + ", streamId);
    pImageBufferHeap = isInImageStream(streamId) ?
        rpStreamBuffer->tryReadLock(mNodeName, true) :
        rpStreamBuffer->tryWriteLock(mNodeName);
    MY_LOGD_IF(mInfo.mLogLevel, "getInImageBuffer streamId: %#" PRIx64 ", try R/W Lock - ", streamId);

    if (pImageBufferHeap == NULL) {
        MY_LOGE("pImageBufferHeap == NULL");
        return BAD_VALUE;
    }

    rpImageBuffer = pImageBufferHeap->createImageBuffer();
    rpImageStreamInfo
        = const_cast<IImageStreamInfo*>(rpStreamBuffer->getStreamInfo());
    if (rpImageBuffer == NULL) {
        rpStreamBuffer->unlock(mNodeName, pImageBufferHeap.get());
        MY_LOGE("rpImageBuffer == NULL");
        return BAD_VALUE;
    }
    if (rpImageBuffer->getImgSize().w % g_encodeUnit ||
            rpImageBuffer->getImgSize().h % g_encodeUnit) {
        // for sw padding when yuv size is not aligned with encode unit(16),
        // in case of encoding with garbage data
        groupUsage |= GRALLOC_USAGE_SW_WRITE_OFTEN;
    }
    MBOOL ret = rpImageBuffer->lockBuf(mNodeName, groupUsage);
    MY_LOGA_IF(!ret, "%s", "lockBuffer failed, please check VA usage!");

    MY_LOGD_IF(mInfo.mLogLevel, "stream buffer: (%#" PRIx64 ") %p, heap: %p, buffer: %p, usage: %u",
        streamId, rpStreamBuffer.get(), pImageBufferHeap.get(),
        rpImageBuffer.get(), groupUsage);

    return OK;
}

MERROR
Request::
getOutJpegImageBuffer(
    android::sp<IPipelineFrame> const& pFrame,
    StreamId_T const streamId,
    sp<IImageStreamBuffer>& rpStreamBuffer,
    sp<IImageBuffer>& rpImageBuffer,
    sp<encode_frame>& rpEncodeFrame,
    sp<IImageBufferHeap>& rpImageBufferHeap
    )
{
    IStreamBufferSet& rStreamBufferSet = pFrame->getStreamBufferSet();

    if(NULL == rpImageBufferHeap.get()) {
        if (rpStreamBuffer.get() == NULL) {
            MERROR const err = acquireImageBuffer(
                    pFrame->getFrameNo(),
                    streamId,
                    rStreamBufferSet,
                    rpStreamBuffer
                    );
            if( err != OK )
                return err;
        }

        // pre-release
#if ENABLE_PRERELEASE
/*
        if (streamId != 0) {
            MY_LOGD("test prerelease flow start w/ timeline counter: %d", mTimelineCounter);
            //prepare timeline & release fence
            sp<IFence> release_fence;
            int sync_fence_fd = mpTimeline->createFence("RF_Jpeg", ++mTimelineCounter);
            rStreamBufferSet.setUserReleaseFence(streamId, mNodeId, sync_fence_fd);
            //
            if ( rpStreamBuffer.get() ) {
                rStreamBufferSet.markUserStatus(
                        streamId,
                        getNodeId(),
                        IUsersManager::UserStatus::USED | IUsersManager::UserStatus::PRE_RELEASE
                        );
                rStreamBufferSet.applyPreRelease(getNodeId());
            }
        }
*/
#endif
        MY_LOGD_IF(mInfo.mLogLevel, "getOutJpegImageBuffer streamId: %#" PRIx64 ", try R/W Lock + ", streamId);
        rpImageBufferHeap = isInImageStream(streamId) ?
            rpStreamBuffer->tryReadLock(mNodeName, true) :
            rpStreamBuffer->tryWriteLock(mNodeName);
        MY_LOGD_IF(mInfo.mLogLevel, "getOutJpegImageBuffer streamId: %#" PRIx64 ", try R/W Lock - ", streamId);

        if (NULL == rpImageBufferHeap.get()) {
            MY_LOGE("rpImageBufferHeap is NULL");
            return BAD_VALUE;
        }
    }
    if (rpEncodeFrame->mpYUV_MainStreamInfo.get()) {
        size_t mainOffset = rpEncodeFrame->exif.getHeaderSize();
        size_t mainMaxSize = rpImageBufferHeap->getBufSizeInBytes(0) - mainOffset;

        // precheck jpeg bit stream size
        size_t mainYUV = 0;
        sp<IImageStreamInfo> Main_YuvStreamInfo =
            rpEncodeFrame->mpYUV_MainStreamInfo;
        for(size_t i = 0; i < Main_YuvStreamInfo->getBufPlanes().count; i++) {
            mainYUV += Main_YuvStreamInfo->getBufPlanes().planes[i].sizeInBytes;
        }
        size_t warnRatioSize = mainYUV*JPEG_ENC_WARN_RATIO;
        if ((size_t)(mainMaxSize) < warnRatioSize) {
            MY_LOGW("dst bs buffer size %zu is smaller than warnRatioSize %zu,"
            " input main YUV size %zu!", mainMaxSize, warnRatioSize, mainYUV);
        }

        MUINT32 transform = Main_YuvStreamInfo->getTransform();
        MSize imageSize = MSize(Main_YuvStreamInfo->getImgSize().w,
            Main_YuvStreamInfo->getImgSize().h);

        size_t const bufStridesInBytes[3] = {mainMaxSize, 0 ,0};
        rpImageBuffer = rpImageBufferHeap->createImageBuffer_FromBlobHeap(
                mainOffset, eImgFmt_JPEG, imageSize, bufStridesInBytes
                );
        if (NULL == rpImageBuffer.get()) {
            if (rpStreamBuffer != NULL) {
                rpStreamBuffer->unlock(mNodeName, rpImageBufferHeap.get());
            }
            MY_LOGE("rpImageMainBuffer is NULL");
            return BAD_VALUE;
        }
        //  Query the group usage.
        MUINT const usage = (GRALLOC_USAGE_SW_READ_OFTEN|
                GRALLOC_USAGE_SW_WRITE_OFTEN|
                GRALLOC_USAGE_HW_CAMERA_READ|
                GRALLOC_USAGE_HW_CAMERA_WRITE);
        MUINT const groupUsage = (rpStreamBuffer != NULL) ?
            rpStreamBuffer->queryGroupUsage(mNodeId) : usage;
        MBOOL ret = rpImageBuffer->lockBuf(mNodeName, groupUsage);
        MY_LOGA_IF(!ret, "%s", "lockBuffer failed, please check VA usage!");

        MY_LOGD_IF(mInfo.mLogLevel, "stream buffer(%#" PRIx64 ") %p, heap(0x%x): %p, buffer: %p, "
            "usage: %x, trans:%d, ori:%d, heapVA: %zx, bufferVA: %zx",
                streamId, rpStreamBuffer.get(),
                rpImageBufferHeap->getImgFormat(),
                rpImageBufferHeap.get(), rpImageBuffer.get(),
                groupUsage,
                transform, rpEncodeFrame->mParams.orientation,
                rpImageBufferHeap->getBufVA(0), rpImageBuffer->getBufVA(0));
    }
    return OK;

}

MERROR
Request::
getOutThumbImageBuffer(
    StreamId_T const streamId,
    sp<encode_frame> const& rpEncodeFrame,
    sp<IImageBufferHeap> const& rpImageBufferHeap,
    sp<IImageBuffer>& rpImageBuffer /*out*/
    )
{
    if(rpImageBufferHeap == NULL) {
        MY_LOGE("heap not exist");
        return BAD_VALUE;
    }

    //get thumb IImageBuffer
    size_t thumbnailMaxSize = rpEncodeFrame->thumbnailMaxSize;
    size_t thumbnailOffset = rpEncodeFrame->exif.getStdExifSize();

    size_t const bufStridesInBytes[3] = {thumbnailMaxSize, 0 ,0};
    size_t bufBoundaryInBytes[] = {0, 0, 0};
    // ref v1 prepare heap & imagebuffer
    IImageBufferAllocator::ImgParam imgParam =
        IImageBufferAllocator::ImgParam(
                rpImageBufferHeap->getImgFormat(),//blob
                MSize(
                    rpEncodeFrame->mParams.size_thumbnail.w,
                    rpEncodeFrame->mParams.size_thumbnail.h
                    ),
                bufStridesInBytes,
                bufBoundaryInBytes,
                Format::queryPlaneCount(rpImageBufferHeap->getImgFormat())
                );

    PortBufInfo_v1 portBufInfo = PortBufInfo_v1(
            rpImageBufferHeap->getHeapID(),
            (MUINTPTR)(rpImageBufferHeap->getBufVA(0) + thumbnailOffset)
            );

    MBOOL mbEnableIImageBufferLog = MTRUE;
    sp<IImageBufferHeap> pHeap = ImageBufferHeap::create(
            LOG_TAG,
            imgParam,
            portBufInfo,
            mbEnableIImageBufferLog
            );
    if(pHeap == 0) {
        MY_LOGE("pHeap is NULL");
        return BAD_VALUE;
    }
    rpImageBuffer = pHeap->createImageBuffer_FromBlobHeap(
            0,
            eImgFmt_JPEG,
            rpEncodeFrame->mParams.size_thumbnail,
            bufStridesInBytes
            );
    MUINT const groupUsage = eBUFFER_USAGE_SW_WRITE_OFTEN | eBUFFER_USAGE_HW_CAMERA_READ;
    MBOOL ret = rpImageBuffer->lockBuf(
            mNodeName, groupUsage
            );
    MY_LOGA_IF(!ret, "%s", "lockBuffer failed, please check VA usage!");

    if (NULL == rpImageBuffer.get()) {
        MY_LOGE("rpImageThumbnailBuffer == NULL");
        return BAD_VALUE;
    }

    MY_LOGD_IF(mInfo.mLogLevel, "thumb stream buffer(%#" PRIx64 "), "
        "heap(0x%x): %p, buffer: %p, usage: %x, heapVA: %zx, bufferVA: %zx",
            streamId, rpImageBufferHeap->getImgFormat(),
            rpImageBufferHeap.get(), rpImageBuffer.get(), groupUsage,
            rpImageBufferHeap->getBufVA(0), rpImageBuffer->getBufVA(0));
    return OK;
}

MERROR
Request::
getMetadataBuffer(
        android::sp<IPipelineFrame> const& pFrame,
        StreamId_T const streamId,
        sp<IMetaStreamBuffer>& rpStreamBuffer,
        IMetadata*& rpMetadata
        )
{
    IStreamBufferSet& rStreamBufferSet = pFrame->getStreamBufferSet();
    MY_LOGD_IF(mInfo.mLogLevel, "nodeID %d streamID %#" PRIx64 " ", mNodeId, streamId);

    if (rpStreamBuffer.get() == NULL) {
        MERROR const err = acquireMetaBuffer(
                pFrame->getFrameNo(),
                streamId,
                rStreamBufferSet,
                rpStreamBuffer
                );

        if( err != OK )
        {
            MY_LOGD_IF(rpStreamBuffer == NULL,
                "streamId(%#" PRIx64 ") meta streamBuf not exit", streamId);
            return err;
        }
    }

    MY_LOGD_IF(mInfo.mLogLevel, "getMetadataBuffer streamId: %#" PRIx64 ", try R/W Lock + ", streamId);
    rpMetadata = isInMetaStream(streamId) ?
        rpStreamBuffer->tryReadLock(mNodeName, true) :
        rpStreamBuffer->tryWriteLock(mNodeName);
    MY_LOGD_IF(mInfo.mLogLevel, "getMetadataBuffer streamId: %#" PRIx64 ", try R/W Lock - ", streamId);

    if( rpMetadata == NULL ) {
        MY_LOGE("[frame:%u node:%d][stream buffer:%s] cannot get metadata",
                pFrame->getFrameNo(), mNodeId, rpStreamBuffer->getName());
        return BAD_VALUE;
    }

    MY_LOGD_IF(mInfo.mLogLevel, "stream %#" PRIx64 ": stream buffer %p, metadata: %p",
        streamId, rpStreamBuffer.get(), rpMetadata);

    return OK;
}

MERROR
Request::
returnMetadataBuffer(
        android::sp<IPipelineFrame> const& pFrame,
        StreamId_T const streamId,
        sp<IMetaStreamBuffer>  rpStreamBuffer,
        IMetadata* rpMetadata,
        MBOOL success
        )
{
    IStreamBufferSet& rStreamBufferSet = pFrame->getStreamBufferSet();
    //
    if( rpStreamBuffer.get() == NULL ) {
        MY_LOGE("StreamId %#" PRIx64 ": rpStreamBuffer == NULL",
                streamId);
        return BAD_VALUE;
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
        rpStreamBuffer->unlock(mNodeName, rpMetadata);
    //
    //  Mark this buffer as USED by this user.
    //  Mark this buffer as RELEASE by this user.
    if (mbIsEncodePreRelease == MFALSE) {
        rStreamBufferSet.markUserStatus(
                streamId, mNodeId,
                IUsersManager::UserStatus::USED |
                IUsersManager::UserStatus::RELEASE
                );
    } else {
        rpStreamBuffer->markUserStatus(
                mNodeId,
                IUsersManager::UserStatus::USED |
                IUsersManager::UserStatus::RELEASE
                );
    }
    return OK;

}

MERROR
Request::
returnImageBuffer(
        android::sp<IPipelineFrame> const& pFrame,
        StreamId_T const streamId,
        sp<IImageStreamBuffer>& rpStreamBuffer,
        sp<IImageBuffer> rpImageBuffer,
        sp<IImageBuffer> rpImageBuffer2
        )
{
    IStreamBufferSet& streamBufferSet = pFrame->getStreamBufferSet();

    if (mInfo.mbPackBitStream) {
        if(!isInImageStream(streamId)) {
            mJpegPool->putBufHeap(streamId, rpImageBuffer->getFD(0));
       }
    }
    if (rpImageBuffer != NULL)
        rpImageBuffer->unlockBuf(mNodeName);
    if (rpImageBuffer2 != NULL)
        rpImageBuffer2->unlockBuf(mNodeName);
    if(rpStreamBuffer != NULL && isInImageStream(streamId)) {
        rpStreamBuffer->unlock(mNodeName, rpImageBuffer->getImageBufferHeap());
        if (mbIsEncodePreRelease == MFALSE) {
            streamBufferSet.markUserStatus(
                    rpStreamBuffer->getStreamInfo()->getStreamId(),
                    mNodeId,
                    IUsersManager::UserStatus::USED |
                    IUsersManager::UserStatus::RELEASE
                    );
        } else {
            rpStreamBuffer->markUserStatus(
                    mNodeId,
                    IUsersManager::UserStatus::USED |
                    IUsersManager::UserStatus::RELEASE
                    );
        }
    }


    return OK;
}

MERROR
Request::
prepareMeta()
{
    if (mbSkipParseMeta) {
        MY_LOGD_IF(mInfo.mLogLevel, "SKIP parse meta for main2 request");
        return OK;
    }
    if (mInAppMeta != NULL) {
        mpEncodeFrame->mParams.rotationEnable = mInfo.mJpegRotationEnable;
        getJpegParams(mInAppMeta, mpEncodeFrame->mParams);
    }



    MUINT8 encodeType;
    if (tryGetMetadata<MUINT8>(mInHalMeta, MTK_JPG_ENCODE_TYPE, encodeType)) {
        mpEncodeFrame->miJpegEncType = encodeType;
        MY_LOGD("Assign encode type manually.(%d)", mpEncodeFrame->miJpegEncType);
    }

#if MTKCAM_EARLY_FLIP_SUPPORT == 1
    MINT32 isFlipped;
    if (tryGetMetadata<MINT32>(mInHalMeta, MTK_FEATURE_FLIP_IN_P2A, isFlipped)) {
        mpEncodeFrame->mParams.isFlipped = isFlipped;
        MY_LOGW("Assign isFlipped %d if has flipped in capture P2ANode", mpEncodeFrame->mParams.isFlipped);
    }
#endif

    // Active array
    mpEncodeFrame->mParams.activeArray = mInfo.mActiveArray;
    mpEncodeFrame->mParams.configured = (mpJpegStreamInfo != NULL) ? MTRUE : MFALSE;
    mpEncodeFrame->mParams.facing = mInfo.muFacing;
    if (!tryGetMetadata<MINT32>(mInHalMeta, MTK_PIPELINE_UNIQUE_KEY, mUniqueKey)) {
        MY_LOGW("no tag MTK_PIPELINE_UNIQUE_KEY");
        mUniqueKey = -1;
    }
    if (!tryGetMetadata<MINT32>(mInHalMeta, MTK_PIPELINE_FRAME_NUMBER, mFrameNumber)) {
        MY_LOGW("no tag MTK_PIPELINE_FRAME_NUMBER");
        mFrameNumber = -1;
    }
    if (!tryGetMetadata<MINT32>(mInHalMeta, MTK_PIPELINE_REQUEST_NUMBER, mRequestNumber)) {
        MY_LOGW("no tag MTK_PIPELINE_REQUEST_NUMBER");
        mRequestNumber = -1;
    }
    if (!tryGetMetadata<MINT32>(mInHalMeta, MTK_FEATURE_FREE_MEMORY_MBYTE, mFreeMemoryMBytes)) {
        MY_LOGW("no tag MTK_FEATURE_FREE_MEMORY_MBYTE");
        mFreeMemoryMBytes = -1;
    }

    // gen Jpeg filename
    if (mInfo.muDumpBuffer) {
        FILE_DUMP_NAMING_HINT hint;
        hint.UniqueKey = mUniqueKey;
        hint.FrameNo = mFrameNumber;
        hint.RequestNo = mRequestNumber;
        MBOOL res = MTRUE;
        res = extract(&hint, mInHalMeta);
        if (!res) {
            MY_LOGW("[DUMP_JPG] extract with metadata fail (%d)", res);
        }
        genFileName_JPG(filename, sizeof(filename), &hint, nullptr);
        MY_LOGD("enable muDumpBuffer FileName[%s]", filename);
        genFileName_JPG(filename_pack, sizeof(filename_pack), &hint, "packed");
        MY_LOGD("enable muDumpBuffer FileName_pack[%s]", filename_pack);
    }

    return OK;
}

MVOID
Request::
updateMetadata(
    jpeg_params& rParams
)
{
    if (mbSkipParseMeta) {
        MY_LOGD_IF(mInfo.mLogLevel, "SKIP update meta for main2 request");
        return;
    }
#define updateNonEmptyEntry(pMetadata, tag, entry) \
    do{                                            \
        if( !entry.isEmpty() ) {                   \
            pMetadata->update(tag, entry);         \
        }                                          \
    }while(0)

    // gps related
    updateNonEmptyEntry(mOutAppMeta, MTK_JPEG_GPS_COORDINATES, rParams.gpsCoordinates);
    updateNonEmptyEntry(mOutAppMeta, MTK_JPEG_GPS_PROCESSING_METHOD, rParams.gpsProcessingMethod);
    updateNonEmptyEntry(mOutAppMeta, MTK_JPEG_GPS_TIMESTAMP, rParams.gpsTimestamp);
    //
    updateEntry<MINT32>(mOutAppMeta, MTK_JPEG_ORIENTATION, rParams.orientation);
    updateEntry<MUINT8>(mOutAppMeta, MTK_JPEG_QUALITY, rParams.quality);
    updateEntry<MUINT8>(mOutAppMeta, MTK_JPEG_THUMBNAIL_QUALITY, rParams.quality_thumbnail);
    updateEntry<MSize>(mOutAppMeta, MTK_JPEG_THUMBNAIL_SIZE, rParams.size_thumbnail);

}

MERROR
Request::
prepareExif()
{
    if (mbSkipParseMeta) {
        MY_LOGD_IF(mInfo.mLogLevel, "SKIP prepare exif for main2 request");
        return OK;
    }

    MBOOL needExifRotate = MTRUE;
    ExifParams stdParams;
    MSize imageSize(0,0);
    MUINT32 transform = eTransform_None;
    if (!mbAppnOnly) {
        imageSize = mpEncodeFrame->mpYUV_MainStreamInfo->getImgSize();
        transform = mpEncodeFrame->mpYUV_MainStreamInfo->getTransform();

        if((mpEncodeFrame->mParams.orientation == 90  &&  transform & eTransform_ROT_90)  ||
                (mpEncodeFrame->mParams.orientation == 270  &&  transform & eTransform_ROT_270) ||
                (mpEncodeFrame->mParams.orientation == 180  &&  transform & eTransform_ROT_180)
          )
            needExifRotate = MFALSE;
    } else {
        imageSize = mpJpegStreamInfo->getImgSize();
    }
    //
    // update standard exif params
    ExifUtils::updateStdExifParam(
            needExifRotate,
            imageSize,
            mInAppMeta,
            mInHalMeta,
            mpEncodeFrame->mParams,
            stdParams,
            mInfo.mLogLevel
            );
    std::map<MUINT32, MUINT32> debugInfoList;
    {
        using namespace dbg_cam_common_param_1;
        debugInfoList[CMN_TAG_VERSION] = CMN_DEBUG_TAG_VERSION_DP;//tag version : sub version(high 2 byte) | major version(low 2 byte)
        debugInfoList[CMN_TAG_PIPELINE_UNIQUE_KEY] = mUniqueKey;
        debugInfoList[CMN_TAG_PIPELINE_FRAME_NUMBER] = mFrameNumber;
        debugInfoList[CMN_TAG_PIPELINE_REQUEST_NUMBER] = mRequestNumber;
        debugInfoList[CMN_TAG_SYS_FREE_MEMORY_MBYTES] = mFreeMemoryMBytes;
    }
    IMetadata exifMetadata;
    tryGetMetadata<IMetadata>(mInHalMeta, MTK_3A_EXIF_METADATA, exifMetadata);
    if (DebugExifUtils::setDebugExif(
                DebugExifUtils::DebugExifType::DEBUG_EXIF_CAM,
                static_cast<MUINT32>(MTK_CMN_EXIF_DBGINFO_KEY),
                static_cast<MUINT32>(MTK_CMN_EXIF_DBGINFO_DATA),
                debugInfoList,
                &exifMetadata) == nullptr)
    {
        MY_LOGW("set debug exif to metadata fail");
    }

    MINT32 bound = DBG_BOUND_WIDTH * DBG_BOUND_HEIGH;
    if(imageSize.w * imageSize.h > bound) {
        stdParams.bNeedSOI = (mbAppnOnly) ? MFALSE : MTRUE;
        mpEncodeFrame->exif.init(stdParams, mInfo.mDbgInfoEnable);
        if(mInfo.mDbgInfoEnable)
            ExifUtils::updateDebugInfoToExif(&exifMetadata, mpEncodeFrame->exif, mInfo.mLogLevel);

        MY_LOGD_IF(mInfo.mLogLevel, "init (%dx%d)",
                imageSize.w, imageSize.h);
    } else {
        stdParams.bNeedSOI = (mbAppnOnly) ? MFALSE : MTRUE;
        mpEncodeFrame->exif.init(stdParams, 0);
        MY_LOGD("skip init (%dx%d)",
                imageSize.w, imageSize.h);
    }

    //set thumbnail max size & thumbnail size need to be 128 alignment
    size_t &thumbMaxSize = mpEncodeFrame->thumbnailMaxSize;

    if( mpEncodeFrame->mbHasThumbnail )
    {
        thumbMaxSize = (mpEncodeFrame->mParams.size_thumbnail.w) * (mpEncodeFrame->mParams.size_thumbnail.h) * 18 / 10;

        size_t thumbnailSize = 0;
        if ((EXIFAPP1_MAX_SIZE - mpEncodeFrame->exif.getStdExifSize()) < thumbMaxSize) {
            thumbnailSize = EXIFAPP1_MAX_SIZE - mpEncodeFrame->exif.getStdExifSize();
            size_t res = thumbnailSize % EXIFHEADER_ALIGN;
            if( res != 0 )
                thumbnailSize = thumbnailSize - res;
        } else {
            thumbnailSize = thumbMaxSize;
            size_t res = thumbnailSize % EXIFHEADER_ALIGN;
            if (res != 0) {
                // prevent it would exceed EXIFAPP1_MAX_SIZE after doing thumbnail size 128 alignemt
                if(thumbnailSize + EXIFHEADER_ALIGN > EXIFAPP1_MAX_SIZE) {
                    thumbnailSize -= res;
                } else {
                    thumbnailSize = thumbnailSize + EXIFHEADER_ALIGN -res;
                }
            }
        }
        thumbMaxSize = thumbnailSize;
    }

    size_t headerSize = mpEncodeFrame->exif.getStdExifSize() +
        mpEncodeFrame->exif.getDbgExifSize() + thumbMaxSize;
    if(headerSize % EXIFHEADER_ALIGN != 0)
        MY_LOGW("not aligned header size %zu", headerSize);

    mpEncodeFrame->exif.setMaxThumbnail(thumbMaxSize);

    return OK;
}

MERROR
ExifUtils::
updateStdExifParam(
    MBOOL const& rNeedExifRotate,
    MSize const& rSize,
    IMetadata* const rpAppMeta,
    IMetadata* const rpHalMeta,
    jpeg_params const& rParams,
    ExifParams & rStdParams,
    MINT32 const enableLog
)
{
    rStdParams.u4ImageWidth  = rSize.w;
    rStdParams.u4ImageHeight = rSize.h;
    //
    // 3A
    if( rpHalMeta && rpAppMeta) {
        IMetadata exifMeta;
        if( tryGetMetadata<IMetadata>(rpHalMeta, MTK_3A_EXIF_METADATA, exifMeta) ) {
            ExifUtils::updateStdExifParam_3A(exifMeta, *rpAppMeta, rStdParams, enableLog);
        }
        else {
            MY_LOGW("no tag: MTK_3A_EXIF_METADATA");
        }
    }
    else {
        MY_LOGW("no in hal meta or app meta");
    }
    // gps
    ExifUtils::updateStdExifParam_gps(
            rParams.gpsCoordinates, rParams.gpsProcessingMethod, rParams.gpsTimestamp,
            rStdParams
            );
    // icc profile
    if( rpHalMeta ) {
        MINT32 iccIdx = -1;
        if( !tryGetMetadata<MINT32>(rpHalMeta, MTK_ISP_COLOR_SPACE, iccIdx) )
        {
            MY_LOGW_IF(enableLog, "no tag: MTK_ISP_COLOR_SPACE");
        }
        else {
          if(iccIdx==MTK_ISP_COLOR_SPACE_SRGB)
              rStdParams.u4ICCIdx = EXIF_ICC_PROFILE_SRGB;
          else if (iccIdx==MTK_ISP_COLOR_SPACE_DISPLAY_P3)
              rStdParams.u4ICCIdx = EXIF_ICC_PROFILE_DCI_P3;
          else
              MY_LOGW("not support isp profile in MTK_ISP_COLOR_SPACE %d ", iccIdx);
        }
    }
    // others
    if ( !rNeedExifRotate )
        rStdParams.u4Orientation = 22;
    else
        rStdParams.u4Orientation = rParams.orientation;

    rStdParams.u4ZoomRatio =
        calcZoomRatio(rParams.configured, rParams.cropRegion,
                rParams.activeArray, rSize, enableLog);
    rStdParams.u4Facing = (rParams.facing == MTK_LENS_FACING_BACK) ? 0 : 1;
    return OK;
}

MERROR
ExifUtils::
updateStdExifParam_3A(
        IMetadata const& rMeta,
        IMetadata const& rAppMeta,
        ExifParams & rStdParams,
        MINT32 const enableLog
        )
{
#define getParam(meta, tag, type, param, needWarn)    \
    do {                                              \
        type val = 0;                                 \
        if( !tryGetMetadata<type>(meta, tag, val) ) { \
            if (needWarn) MY_LOGW("no tag: %s", #tag);\
            else MY_LOGW_IF(enableLog, "no tag: %s", #tag);\
        }                                             \
        param = val;                                  \
    } while(0)
#if 1
    // from result meta of 3A
    // for Hal3 yuv reprocessing [must need!]
    getParam(&rMeta, MTK_3A_EXIF_FNUMBER,             MINT32, rStdParams.u4FNumber, MTRUE); /**/
    if (rStdParams.u4FNumber == 0) {
        MFLOAT fNumber = 0.0f;
        getParam(&rAppMeta, MTK_LENS_APERTURE, MFLOAT, fNumber, MTRUE);
        rStdParams.u4FNumber = fNumber*FNUMBER_PRECISION;
        MY_LOGD("miss in Hal find APP MTK_LENS_APERTURE : %d", rStdParams.u4FNumber);
    }
    getParam(&rMeta, MTK_3A_EXIF_FOCAL_LENGTH,        MINT32, rStdParams.u4FocalLength, MTRUE); /**/
    if (rStdParams.u4FocalLength == 0) {
        MFLOAT focalLength = 0.0f;
        getParam(&rAppMeta, MTK_LENS_FOCAL_LENGTH, MFLOAT, focalLength, MTRUE);
        rStdParams.u4FocalLength = focalLength*1000;
        MY_LOGD("miss in Hal find APP MTK_LENS_FOCAL_LENGTH : %d", rStdParams.u4FocalLength);
    }
    getParam(&rMeta, MTK_3A_EXIF_CAP_EXPOSURE_TIME,   MINT32, rStdParams.u4CapExposureTime, MTRUE); /**/
    if (rStdParams.u4CapExposureTime == 0) {
        MINT64 capExposure = 0;
        getParam(&rAppMeta, MTK_SENSOR_EXPOSURE_TIME, MINT64, capExposure, MTRUE);
        rStdParams.u4CapExposureTime = (MINT32)(capExposure/1000);
        MY_LOGD("miss in Hal find APP MTK_3A_EXIF_CAP_EXPOSURE_TIME : %" PRId32, rStdParams.u4CapExposureTime);
    }
    getParam(&rMeta, MTK_3A_EXIF_AE_ISO_SPEED,        MINT32, rStdParams.u4AEISOSpeed, MTRUE); /**/
    if (rStdParams.u4AEISOSpeed == 0) {
        getParam(&rAppMeta, MTK_SENSOR_SENSITIVITY, MINT32, rStdParams.u4AEISOSpeed, MTRUE);
        MY_LOGD("miss in Hal find APP MTK_SENSOR_SENSITIVITY : %d", rStdParams.u4AEISOSpeed);
    }

    //
    getParam(&rMeta, MTK_3A_EXIF_FOCAL_LENGTH_35MM,   MINT32, rStdParams.u4FocalLength35mm, MFALSE);
    getParam(&rMeta, MTK_3A_EXIF_AWB_MODE,            MINT32, rStdParams.u4AWBMode, MTRUE);
    getParam(&rMeta, MTK_3A_EXIF_LIGHT_SOURCE,        MINT32, rStdParams.u4LightSource, MTRUE);
    getParam(&rMeta, MTK_3A_EXIF_EXP_PROGRAM,         MINT32, rStdParams.u4ExpProgram, MTRUE);
    getParam(&rMeta, MTK_3A_EXIF_SCENE_CAP_TYPE,      MINT32, rStdParams.u4SceneCapType, MTRUE);
    getParam(&rMeta, MTK_3A_EXIF_FLASH_LIGHT_TIME_US, MINT32, rStdParams.u4FlashLightTimeus, MTRUE);
    getParam(&rMeta, MTK_3A_EXIF_AE_METER_MODE,       MINT32, rStdParams.u4AEMeterMode, MTRUE);
    getParam(&rMeta, MTK_3A_EXIF_AE_EXP_BIAS,         MINT32, rStdParams.i4AEExpBias, MTRUE);

#endif

#undef getParam
    return OK;
}

MERROR
ExifUtils::
updateStdExifParam_gps(
        IMetadata::IEntry const& rGpsCoordinates,
        IMetadata::IEntry const& rGpsProcessingMethod,
        IMetadata::IEntry const& rGpsTimestamp,
        ExifParams & rStdParams
        )
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
                    "%" PRId64 "", rGpsTimestamp.itemAt(0, Type2Type<MINT64>())
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
    return OK;
}

MERROR
ExifUtils::
updateDebugInfoToExif(
    IMetadata* const pExifMeta,
    StdExif & exif,
    MINT32 const enableLog
)
{
    if( pExifMeta == NULL )
    {
        MY_LOGW("pExifMeta is NULL, update debug info to exif fail");
        return BAD_VALUE;
    }
    MUINT32 dbgKey = MTK_3A_EXIF_DEBUGINFO_BEGIN;
    MUINT32 dbgVal = MTK_3A_EXIF_DEBUGINFO_BEGIN + 1;
    while( dbgVal < MTK_3A_EXIF_DEBUGINFO_END ) {
        MINT32 key;
        IMetadata::Memory dbgmem;
        if( tryGetMetadata<MINT32>(pExifMeta, dbgKey, key) &&
                tryGetMetadata<IMetadata::Memory>(pExifMeta, dbgVal, dbgmem)
          )
        {
            MINT32 ID;
            void* data = static_cast<void*>(dbgmem.editArray());
            size_t size = dbgmem.size();
            if( size > 0 ) {
                MY_LOGD_IF(enableLog, "key 0x%x, data %p, size %zu", key, data, size);
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
    return OK;
}

MERROR
ExifUtils::
makeExifHeader(
    sp<encode_frame> rpEncodeFrame,
    MINT8 * const pOutExif,
    size_t& rOutExifSize, // [IN/OUT] in: exif buf size, out: exif header size
    MBOOL const isAppnOnly
)
{
    MERROR ret;
    //
    MBOOL needSOI = MTRUE;
    if (isAppnOnly) {
        needSOI = MFALSE;
    }
    ret = rpEncodeFrame->exif.make((MUINTPTR)pOutExif, rOutExifSize, needSOI);
    //
    rpEncodeFrame->exif.uninit();
    //
    return ret;
}

MBOOL
Request::
isInImageStream(
    StreamId_T const streamId
) const
{
    //
    if( isStream(mpMainStreamInfo, streamId) )
        return MTRUE;
    //
    if( isStream(mpThumbStreamInfo, streamId) )
        return MTRUE;
    //
    MY_LOGD_IF(mInfo.mLogLevel, "stream id %#" PRIx64 " is not in-stream", streamId);
    return MFALSE;
}

MBOOL
Request::
isInMetaStream(
    StreamId_T const streamId
) const
{
    return isStream(mpInAppMeta, streamId) || isStream(mpInHalMeta, streamId);
}

MERROR
Request::
lockInImg()
{
    if (!mbAppnOnly) {
        sp<IImageStreamInfo> pYuvStreamInfo_req = NULL;
        // getImageBuffer
        if (getInImageBuffer(mpFrame, mpMainStreamInfo->getStreamId(),
                    mpInMainImageStreamBuffer, mpInMainImageBuffer, pYuvStreamInfo_req) != OK) {
            MY_LOGE("get input main imagebuffer failed!");
            return BAD_VALUE;
        } else {
            mpEncodeFrame->mpYUV_Main = mpInMainImageBuffer;
            mpEncodeFrame->mpYUV_MainStreamInfo = pYuvStreamInfo_req;
            MUINT32 transform = mpEncodeFrame->mpYUV_MainStreamInfo->getTransform();
            if (mpEncodeFrame->mParams.rotationEnable) {
                if( (mpEncodeFrame->mParams.orientation == 90 && transform & eTransform_ROT_90)
                        || (mpEncodeFrame->mParams.orientation == 270 && transform & eTransform_ROT_270))
                {
                    std::swap(mpEncodeFrame->mParams.size_thumbnail.w,
                            mpEncodeFrame->mParams.size_thumbnail.h);
                }
                MY_LOGD_IF(mInfo.mLogLevel, "@@getJpegParams thumb size(w,h)=(%dx%d)",
                        mpEncodeFrame->mParams.size_thumbnail.w, mpEncodeFrame->mParams.size_thumbnail.h);
            }
            // check thumbnail size
            if (!mpEncodeFrame->mParams.size_thumbnail.w ||
                    !mpEncodeFrame->mParams.size_thumbnail.h) {
                MY_LOGD("App meta is not set thumbnail size, check request streamInfo size");
                if (mpThumbStreamInfo.get() != NULL) {
                    if (mpThumbStreamInfo->getImgSize().w &&
                            mpThumbStreamInfo->getImgSize().h) {
                        mpEncodeFrame->mParams.size_thumbnail =
                            mpThumbStreamInfo->getImgSize();
                        if (mInfo.mJpegRotationEnable) {
                            if( mpEncodeFrame->mParams.orientation == 90 ||
                                    mpEncodeFrame->mParams.orientation == 270)
                            {
                                std::swap(mpEncodeFrame->mParams.size_thumbnail.w,
                                        mpEncodeFrame->mParams.size_thumbnail.h);
                            }
                            MY_LOGD_IF(0,"@@getJpegParams thumb size(w,h)=(%dx%d)",
                                    mpEncodeFrame->mParams.size_thumbnail.w,
                                    mpEncodeFrame->mParams.size_thumbnail.h);
                        }
                    } else {
                        MY_LOGW("Thumbnail size is not set!");
                        mpEncodeFrame->mbHasThumbnail = MFALSE;
                    }
                } else {
                    MY_LOGW("Thumbnail size is not set!");
                    mpEncodeFrame->mbHasThumbnail = MFALSE;
                }
            }
            if (mInfo.mLogLevel >= 2) {
                dumpYUVBuffer(mpFrame->getFrameNo(), mpInMainImageBuffer, 0);
            }
        }
    }
    if (mpEncodeFrame->mbHasThumbnail) {
        sp<IImageStreamInfo> pYuvThumbStreamInfo_req = NULL;
        if (getInImageBuffer(mpFrame, mpThumbStreamInfo->getStreamId(),
                    mpInThumbImageStreamBuffer, mpInThumbImageBuffer, pYuvThumbStreamInfo_req) != OK) {
            MY_LOGE("get input thumbnail imagebuffer failed!");
            return BAD_VALUE;
        } else {
            mpEncodeFrame->mpYUV_Thumbnail = mpInThumbImageBuffer;
            mpEncodeFrame->mpYUV_ThumbStreamInfo = pYuvThumbStreamInfo_req;
            if (mInfo.mLogLevel >= 2) {
                dumpYUVBuffer(mpFrame->getFrameNo(), mpInThumbImageBuffer, 1);
            }
        }
    }

    return OK;
}

MERROR
Request::
lockOutImg()
{
    if (mInfo.mbPackBitStream) {
        sp<IImageBufferHeap> Main_working = NULL;
        StreamId_T streamId = mpMainStreamInfo->getStreamId();
        if ((Main_working = mJpegPool->getBufHeap(streamId)) == NULL) {
            MY_LOGE("get output working Jpeg ImageBuffer failed!");
            return BAD_VALUE;
        } else {
            if (getOutJpegImageBuffer(mpFrame, mpJpegStreamInfo->getStreamId(),
                        mpOutJpegImageStreamBuffer , mpOutJpegImageBuffer,
                        mpEncodeFrame, Main_working) != OK)
            {
                MY_LOGE("get output Jpeg ImageBuffer failed!");
                mpEncodeFrame->mbBufValid = MFALSE;
                unlockImage(mpOutJpegImageStreamBuffer, mpOutJpegImageBuffer);
                return BAD_VALUE;
            } else {
                mpEncodeFrame->mpJpeg_Main = mpOutJpegImageBuffer;
                mpEncodeFrame->mpOutImgBufferHeap = Main_working;
            }
            if (mpEncodeFrame->mbHasThumbnail &&
                    getOutThumbImageBuffer(mpJpegStreamInfo->getStreamId(),
                        mpEncodeFrame, Main_working,
                        mpOutThumbImageBuffer) != OK) {
                MY_LOGE("get out thumb imagebuffer failed!");
                mpEncodeFrame->mbBufValid = MFALSE;
                unlockImage(mpOutJpegImageStreamBuffer, mpOutThumbImageBuffer);
                return BAD_VALUE;
            } else {
                mpEncodeFrame->mpJpeg_Thumbnail = mpOutThumbImageBuffer;
            }
        }
    } else {
        if (getOutJpegImageBuffer(mpFrame, mpJpegStreamInfo->getStreamId(),
                    mpOutJpegImageStreamBuffer, mpOutJpegImageBuffer,
                    mpEncodeFrame, mpEncodeFrame->mpOutImgBufferHeap) != OK) {
            MY_LOGE("get output Jpeg imagebuffer failed!");
            unlockImage(mpOutJpegImageStreamBuffer, mpOutJpegImageBuffer);
            mpEncodeFrame->mbBufValid = MFALSE;
            return BAD_VALUE;
        } else {
            mpEncodeFrame->mpJpeg_Main = mpOutJpegImageBuffer;
            mpEncodeFrame->mpOutImgStreamBuffer = mpOutJpegImageStreamBuffer;
        }

        if (mpEncodeFrame->mbHasThumbnail &&
                getOutThumbImageBuffer(mpJpegStreamInfo->getStreamId(),
                    mpEncodeFrame, mpEncodeFrame->mpOutImgBufferHeap,
                    mpOutThumbImageBuffer) != OK) {
            MY_LOGE("get output thumb imagebuffer failed!");
            unlockImage(mpOutJpegImageStreamBuffer, mpOutThumbImageBuffer);
            mpEncodeFrame->mbBufValid = MFALSE;
            return BAD_VALUE;
        } else {
            mpEncodeFrame->mpJpeg_Thumbnail = mpOutThumbImageBuffer;
        }
    }

    return OK;
}

MERROR
Request::
lockMeta()
{
    if (mbSkipParseMeta) {
        MY_LOGD_IF(mInfo.mLogLevel, "SKIP lockmeta for main2 request");
        return OK;
    }
    // getMetadataBuffer
    if (mpInAppMeta != NULL && getMetadataBuffer(mpFrame, mpInAppMeta->getStreamId(),
                mpInAppMetaStreamBuffer, mInAppMeta) != OK) {
        MY_LOGE("get input app meta failed!");
        return BAD_VALUE;
    }

    if (mpInHalMeta != NULL && getMetadataBuffer(mpFrame, mpInHalMeta->getStreamId(),
                mpInHalMetaStreamBuffer, mInHalMeta) != OK) {
        MY_LOGE("get input hal meta failed!");
        return BAD_VALUE;
    }

    if (mpOutAppMeta != NULL && getMetadataBuffer(mpFrame, mpOutAppMeta->getStreamId(),
                mpOutAppMetaStreamBuffer, mOutAppMeta) != OK) {
        MY_LOGE("get output app meta failed!");
        return BAD_VALUE;
    }
    return OK;
}

MERROR
Request::
unlock()
{
    // returnImageBuffer
    if (mpMainStreamInfo != NULL &&
            returnImageBuffer(mpFrame, mpMainStreamInfo->getStreamId(),
        mpInMainImageStreamBuffer, mpInMainImageBuffer) != OK) {
        MY_LOGE("return Main image buffer failed!");
        return BAD_VALUE;
    }
    if (mpThumbStreamInfo != NULL &&
            returnImageBuffer(mpFrame, mpThumbStreamInfo->getStreamId(),
        mpInThumbImageStreamBuffer, mpInThumbImageBuffer) != OK) {
        MY_LOGE("return Thumbnail image buffer failed!");
        return BAD_VALUE;
    }

    if (returnImageBuffer(mpFrame, mpJpegStreamInfo->getStreamId(),
        mpOutJpegImageStreamBuffer, mpOutJpegImageBuffer,
        mpOutThumbImageBuffer) != OK) {
        MY_LOGE("return Thumbnail image buffer failed!");
        return BAD_VALUE;
    }
    if (mbSkipParseMeta) {
        MY_LOGD_IF(mInfo.mLogLevel, "SKIP return Meta for main2 request");
        return OK;
    }
    // returnMetadataBuffer
    if (mpInAppMeta != NULL &&
            returnMetadataBuffer(mpFrame, mpInAppMeta->getStreamId(),
        mpInAppMetaStreamBuffer, mInAppMeta) != OK) {
        MY_LOGE("return App meta buffer failed!");
        return BAD_VALUE;
    }
    if (mpInHalMeta != NULL &&
            returnMetadataBuffer(mpFrame, mpInHalMeta->getStreamId(),
        mpInHalMetaStreamBuffer, mInHalMeta) != OK) {
        MY_LOGE("return Hal meta buffer failed!");
        return BAD_VALUE;
    }

    if (mpOutAppMeta != NULL &&
            returnMetadataBuffer(mpFrame, mpOutAppMeta->getStreamId(),
        mpOutAppMetaStreamBuffer, mOutAppMeta) != OK) {
        MY_LOGE("return App meta buffer failed!");
        return BAD_VALUE;
    }


    return OK;
}

MERROR
Request::
acquireImageBuffer(
    MUINT32 const frameNo,
    StreamId_T const streamId,
    IStreamBufferSet& rStreamBufferSet,
    sp<IImageStreamBuffer>& rpStreamBuffer,
    MBOOL acquire
    )
{
    //  Ensure this buffer really comes with the request.
    //  A buffer may not exist due to partial requests.
    rpStreamBuffer = rStreamBufferSet.getImageBuffer(streamId, mNodeId);
    if  ( rpStreamBuffer == 0 ) {
        MY_LOGD("[frame:%d node:%d], streamID(%#" PRIx64 ")",
            frameNo, mNodeId, streamId);
        return NAME_NOT_FOUND;
    }
    if (acquire) {
        //  Mark this buffer as ACQUIRED by this user.
        rStreamBufferSet.markUserStatus(
            streamId, mNodeId, IUsersManager::UserStatus::ACQUIRE);
    }
    //  Check buffer status.
    if  ( rpStreamBuffer->hasStatus(STREAM_BUFFER_STATUS::ERROR) ) {
        //  The producer ahead of this user may fail to render this buffer's content.
        MY_LOGE("[frame:%u node:%d][stream buffer:%s] bad status:%d",
            frameNo, mNodeId, rpStreamBuffer->getName(), rpStreamBuffer->getStatus());
        rpStreamBuffer = 0;
        return BAD_VALUE;
    }
    return  OK;

}

MERROR
Request::
acquireMetaBuffer(
    MUINT32 const frameNo,
    StreamId_T const streamId,
    IStreamBufferSet& rStreamBufferSet,
    sp<IMetaStreamBuffer>& rpStreamBuffer,
    MBOOL acquire
    )
{
    rpStreamBuffer = rStreamBufferSet.getMetaBuffer(streamId, mNodeId);
    if  ( rpStreamBuffer == 0 ) {
        MY_LOGD("[frame:%u node:%d], streamID(%#" PRIx64 ")",
            frameNo, mNodeId, streamId);
        return NAME_NOT_FOUND;
    }
    if (acquire) {
        //  Mark this buffer as ACQUIRED by this user.
        rStreamBufferSet.markUserStatus(streamId, mNodeId, IUsersManager::UserStatus::ACQUIRE);
    }
    //  Check buffer status.
    if  ( rpStreamBuffer->hasStatus(STREAM_BUFFER_STATUS::ERROR) ) {
        MY_LOGE("[frame:%u node:%d][stream buffer:%s] bad status:%d",
            frameNo, mNodeId, rpStreamBuffer->getName(), rpStreamBuffer->getStatus());
        rpStreamBuffer = 0;
        return BAD_VALUE;
    }

    return  OK;

}

JpegBufPool::
~JpegBufPool()
{
    JpgLog();
    if (!mPools.empty())
        mPools.clear();
    if (!mvImageStreamInfo.empty())
        mvImageStreamInfo.clear();
}

MERROR
JpegBufPool::
commitPool()
{
    std::lock_guard<std::mutex> m(mPoolLock);
    for(size_t i = 0; i < mvImageStreamInfo.size(); i++) {
        vector<BufLot> vbufLot;
        MSize size = mvImageStreamInfo[i]->getImgSize();
        MINT32 sizeInBytes =
            size.w * size.h * 2;
        IImageBufferAllocator::ImgParam imgParam_jpeg(sizeInBytes, 0);
        for (int j = 0; j < mBufCount; j++) {
            BufLot lot;
            sp<IImageBufferHeap> pHeap =
                IIonImageBufferHeap::create(mvImageStreamInfo[i]->getStreamName(),
                        imgParam_jpeg);
            lot.pHeap = pHeap;
            MBOOL ret = pHeap->lockBuf(LOG_TAG, GRALLOC_USAGE_SW_READ_OFTEN);
            MY_LOGA_IF(!ret, "%s", "lockBuffer failed, please check VA usage!");

            lot.fd = pHeap->getHeapID(0);
            pHeap->unlockBuf(LOG_TAG);
            lot.status = BufStatus::AVAILABLE;
            vbufLot.push_back(lot);
        }
        mPools.emplace(std::make_pair(mvImageStreamInfo[i]->getStreamId(), vbufLot));
    }

    return OK;
}

sp<IImageBufferHeap>
JpegBufPool::
getBufHeap(StreamId_T const streamId)
{
    std::lock_guard<std::mutex> m(mPoolLock);
    sp<IImageBufferHeap> pHeap = NULL;
    if (mPools.empty()) {
        MY_LOGE("Buffer pool is not committed. It's empty");
        return pHeap;
    }
    auto search = mPools.find(streamId);
    if (search != mPools.end()) {
        vector<BufLot> lot = search->second;
        for(size_t i = 0; i< lot.size(); i++) {
            if (lot[i].status == AVAILABLE) {
                pHeap = lot[i].pHeap;
                lot[i].status = ACQUIRED;
                break;
            }
        }
        if (pHeap == NULL) {
            MY_LOGE("cannot find available buffer id(%#" PRIx64 ")", streamId);
        }
    }

    return pHeap;
}

MERROR
JpegBufPool::
putBufHeap(StreamId_T const streamId, MINT32 ion_fd)
{

    std::lock_guard<std::mutex> m(mPoolLock);
    if (mPools.empty()) {
        MY_LOGE("pool released before buffer released");
        return DEAD_OBJECT;
    }
    MBOOL found = MFALSE;
    auto search = mPools.find(streamId);
    if (search != mPools.end()) {
        vector<BufLot> lot = search->second;
        for(size_t i = 0 ; i < lot.size(); i++) {
            if (lot[i].status == ACQUIRED && ion_fd == lot[i].fd) {
                lot[i].status = AVAILABLE;
            }
        }
        if (found) {
            MY_LOGE("cannot find the BufferLot by id(%#" PRIx64 ")", streamId);
            return BAD_VALUE;
        }
    }

    return OK;
}
