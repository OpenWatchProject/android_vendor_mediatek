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

#include "P2Common.h"
#include "FrameUtils.h"

#include <mtkcam/utils/imagebuf/IIonImageBufferHeap.h>
#include <mtkcam/v3/utils/streaminfo/ImageStreamInfo.h>

using namespace NSCam;
using namespace NSCam::Utils;
using namespace NSCam::v3::Utils;

/******************************************************************************
 *
 ******************************************************************************/
FrameLifeHolder::
FrameLifeHolder(
        MINT32 const openId,
        StreamControl *const pCtrl,
        sp<IPipelineFrame> const &pFrame,
        MBOOL const enableLog
)
    : mbEnableLog(enableLog),
      mOpenId(openId),
      mpStreamControl(pCtrl),
      mpFrame(pFrame)
{
    if  ( ATRACE_ENABLED() ) {
        mTraceName = String8::format("Cam:%d:IspP2|%d|request:%d frame:%d", mOpenId, mpFrame->getRequestNo(), mpFrame->getRequestNo(), mpFrame->getFrameNo());
        CAM_TRACE_ASYNC_BEGIN(mTraceName.string(), 0);
    }
    CAM_TRACE_ASYNC_BEGIN("P2:FrameLife", mpFrame->getFrameNo());
    MY_LOGD_IF(mbEnableLog, "frame[%d]/request[%d] +", mpFrame->getFrameNo(), mpFrame->getRequestNo());
}


/******************************************************************************
 *
 ******************************************************************************/
FrameLifeHolder::
~FrameLifeHolder() {
    if  ( ! mTraceName.isEmpty() ) {
        CAM_TRACE_ASYNC_END(mTraceName.string(), 0);
    }
    if (mpStreamControl)
        mpStreamControl->onFrameDone(mpFrame);
    MY_LOGD_IF(mbEnableLog, "frame[%d]/request[%d] -", mpFrame->getFrameNo(), mpFrame->getRequestNo());
    CAM_TRACE_ASYNC_END("P2:FrameLife", mpFrame->getFrameNo());
}


/******************************************************************************
 *
 ******************************************************************************/
sp<MetaHandle>
MetaHandle::
create(
        StreamControl *const pCtrl,
        sp<IPipelineFrame> const &pFrame,
        StreamId_T const streamId)
{
    // check StreamBuffer here
    sp<IMetaStreamBuffer> pStreamBuffer = NULL;
    if (pCtrl && OK == pCtrl->acquireMetaStream(pFrame, streamId, pStreamBuffer))
    {
        IMetadata *pMeta = NULL;
        if (OK == pCtrl->acquireMetadata(streamId, pStreamBuffer, pMeta)) {
            BufferState_t const init_state =
                    pCtrl->isInMetaStream(streamId) ? STATE_READABLE : STATE_WRITABLE;

            return new MetaHandle(
                    pCtrl, pFrame,
                    streamId, pStreamBuffer,
                    init_state, pMeta
            );
        }
        else {
            pCtrl->releaseMetaStream(pFrame, pStreamBuffer, StreamControl::eStreamStatus_NOT_USED);
        }
    }
    //
    return NULL;
}


/******************************************************************************
 *
 ******************************************************************************/
MetaHandle::
~MetaHandle() {
    if (muState != STATE_NOT_USED) {
        const MUINT32 status = (muState != STATE_WRITE_FAIL) ?
                               StreamControl::eStreamStatus_FILLED : StreamControl::eStreamStatus_ERROR;
        //
        mpStreamCtrl->releaseMetadata(mpStreamBuffer, mpMetadata);
        mpStreamCtrl->releaseMetaStream(mpFrame, mpStreamBuffer, status);
    }
    else {
        mpStreamCtrl->releaseMetaStream(mpFrame, mpStreamBuffer, StreamControl::eStreamStatus_NOT_USED);
    }
    //MY_LOGD("release meta[%d] streamId[0x%x]",mpFrame->getFrameNo() ,mStreamId);
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
MetaHandle::
updateState(BufferState_t const state) {
    Mutex::Autolock _l(mLock);
    if (muState == STATE_NOT_USED) {
        MY_LOGW("streamId %#" PRIxPTR " state %d -> %d",
                mStreamId, muState, state);
    }
    else {
        MY_LOGW_IF(state == STATE_WRITE_FAIL, "streamId %#" PRIxPTR " set fail, state %d -> %d",
                   mStreamId, muState, state);
        muState = state;
    }
    //mCond.broadcast();
}


/******************************************************************************
 *
 ******************************************************************************/
sp<BufferHandle>
StreamBufferHandle::
create(
        StreamControl *const pCtrl,
        sp<IPipelineFrame> const &pFrame,
        StreamId_T const streamId)
{
    // check StreamBuffer here
    sp<IImageStreamBuffer> pStreamBuffer = NULL;
    if (OK == pCtrl->acquireImageStream(
            pFrame,
            streamId,
            pStreamBuffer))
    {
        MUINT32 uTransform = pStreamBuffer->getStreamInfo()->getTransform();
        MUINT32 uUsage = pStreamBuffer->getStreamInfo()->getUsageForAllocator();
        MY_LOGD_IF(0, "create buffer handler, stream:0x%x, transform:%d, usage:%d",
                   streamId, uTransform, uUsage);

        return new StreamBufferHandle(
                pCtrl, pFrame,
                streamId, pStreamBuffer,
                uTransform, uUsage);
    }

    return NULL;
}


/******************************************************************************
 *
 ******************************************************************************/
StreamBufferHandle::
~StreamBufferHandle() {
    if (muState != STATE_NOT_USED) {
        const MUINT32 status = (muState != STATE_WRITE_FAIL) ?
                               StreamControl::eStreamStatus_FILLED : StreamControl::eStreamStatus_ERROR;

        mpStreamCtrl->releaseImageBuffer(mpStreamBuffer, mpImageBuffer);
        mpStreamCtrl->releaseImageStream(mpFrame, mpStreamBuffer, status);
    }
    else {
        mpStreamCtrl->releaseImageStream(mpFrame, mpStreamBuffer, StreamControl::eStreamStatus_NOT_USED);
    }
}


/******************************************************************************
 *
 ******************************************************************************/

MERROR
StreamBufferHandle::
waitState(
        BufferState_t const state,
        nsecs_t const nsTimeout)
{
    Mutex::Autolock _l(mLock);
    if (mpImageBuffer == NULL) {
        // get buffer from streambuffer
        const MERROR ret = mpStreamCtrl->acquireImageBuffer(
                mStreamId, mpStreamBuffer, mpImageBuffer, (state == STATE_WRITABLE));
        // update initial state
        if (ret == OK)
            muState = mpStreamCtrl->isInImageStream(mStreamId) ? STATE_READABLE : STATE_WRITABLE;
        //return ret;
    }

    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
StreamBufferHandle::
updateState(BufferState_t const state) {
    Mutex::Autolock _l(mLock);
    if (muState == STATE_NOT_USED) {
        MY_LOGW("streamId %#" PRIxPTR " state %d -> %d",
                mStreamId, muState, state);
    }
    else {
        MY_LOGW_IF(state == STATE_WRITE_FAIL, "streamId %#" PRIxPTR " set fail: state %d -> %d",
                   mStreamId, muState, state);
        muState = state;
    }
}


/******************************************************************************
 *
 ******************************************************************************/
sp<BufferHandle>
WorkingBufferHandle::
create(
        char const *szName,
        MINT32 format,
        MSize size) {
    String8 bufferName = String8::format("%s:s%dx%d:f%d", szName, size.w, size.h, format);

    // query format
    MUINT32 plane = Format::queryPlaneCount(format);
    size_t bufBoundaryInBytes[3] = {0, 0, 0};
    size_t bufStridesInBytes[3] = {0};

    for (MUINT32 i = 0; i < plane; i++) {
        bufStridesInBytes[i] = Format::queryPlaneWidthInPixels(format, i, size.w) *
                               Format::queryPlaneBitsPerPixel(format, i) / 8;
    }
    // create buffer
    IImageBufferAllocator::ImgParam imgParam =
            IImageBufferAllocator::ImgParam(
                    (EImageFormat) format,
                    size, bufStridesInBytes,
                    bufBoundaryInBytes, plane);

    sp<IIonImageBufferHeap> pHeap =
            IIonImageBufferHeap::create(bufferName.string(), imgParam);
    if (pHeap == NULL) {
        MY_LOGE("working buffer[%s]: create heap failed", bufferName.string());
        return NULL;
    }
    sp<IImageBuffer> pImageBuffer = pHeap->createImageBuffer();
    if (pImageBuffer == NULL) {
        MY_LOGE("working buffer[%s]: create image buffer failed", bufferName.string());
        return NULL;
    }

    // lock buffer
    MUINT const usage = (GRALLOC_USAGE_SW_READ_OFTEN |
                         GRALLOC_USAGE_SW_WRITE_OFTEN |
                         GRALLOC_USAGE_HW_CAMERA_READ |
                         GRALLOC_USAGE_HW_CAMERA_WRITE);
    if (!(pImageBuffer->lockBuf(bufferName.string(), usage))) {
        MY_LOGE("working buffer[%s]: lock image buffer failed", bufferName.string());
        return NULL;
    }

    MY_LOGD_IF(P2_DEBUG_LOG, "working buffer[%s]: create successfully", bufferName.string());

    return new WorkingBufferHandle(bufferName.string(), pImageBuffer);
}


/******************************************************************************
 *
 ******************************************************************************/
WorkingBufferHandle::
~WorkingBufferHandle() {
    if (mpImageBuffer == NULL) {
        MY_LOGW("working buffer[%s]: not existed", mBufferName.string());
        return;
    }

    mpImageBuffer->unlockBuf(mBufferName.string());
    mpImageBuffer.clear();
    MY_LOGD_IF(P2_DEBUG_LOG, "working buffer[%s]: release successfully", mBufferName.string());
}

/******************************************************************************
 *
 ******************************************************************************/

sp<PoolBufferAllocator>
PoolBufferAllocator::
create(MINT32 format, MSize size) {

    String8 poolName = String8::format("pool:s%dx%d:f%d", size.w, size.h, format);

    // query format
    MUINT32 plane = Format::queryPlaneCount(format);
    size_t bufBoundaryInBytes[3] = {0, 0, 0};
    size_t bufStridesInBytes[3] = {0};

    IImageStreamInfo::BufPlanes_t bufPlanes;
    IImageStreamInfo::BufPlane bufPlane;

    for (MUINT32 i = 0; i < plane; i++) {
        bufStridesInBytes[i] = Format::queryPlaneWidthInPixels(format, i, size.w) *
                               Format::queryPlaneBitsPerPixel(format, i) / 8;

        bufPlane.rowStrideInBytes = bufStridesInBytes[i];
        bufPlane.sizeInBytes = bufStridesInBytes[i] *
            Format::queryPlaneHeightInPixels(format, i, size.h);
        bufPlanes.push_back(bufPlane);


    }
    // create allocator's parameter
    IImageBufferAllocator::ImgParam imgParam =
            IImageBufferAllocator::ImgParam(
                    (EImageFormat) format,
                    size, bufStridesInBytes,
                    bufBoundaryInBytes, plane);


    // create stream info
    MUINT const usage = eBUFFER_USAGE_SW_MASK |
                        eBUFFER_USAGE_HW_CAMERA_READWRITE;

    sp<IImageStreamInfo> pStreamInfo = new ImageStreamInfo(
        poolName.string(),
        0, eSTREAMTYPE_IMAGE_OUT, 10, 1,
        usage, format, size, bufPlanes
    );

    if (pStreamInfo == NULL) {
        MY_LOGE("create ImageStream failed:%s", poolName.string());
        return NULL;
    }

    sp<HalImageStreamBufferPoolT> pPool = new HalImageStreamBufferPoolT(
        poolName.string(),
        HalImageStreamBufferAllocatorT(pStreamInfo.get(), imgParam)
    );

    if (pPool == NULL) {
        MY_LOGE("Fail to new a image pool:%s", poolName.string());
        return NULL;
    }

    MERROR err = pPool->initPool("P2POOL", pStreamInfo->getMaxBufNum(), pStreamInfo->getMinInitBufNum());
    if (OK!=err) {
        MY_LOGE("%s: initPool err:%d(%s)", poolName.string(), err, ::strerror(-err));
        return NULL;
    }

    return new PoolBufferAllocator(pPool);

}


/******************************************************************************
 *
 ******************************************************************************/
sp<IImageStreamBuffer>
PoolBufferAllocator::
acquire() {

    sp<IImageStreamBuffer> pStreamBuffer;;
    mpPool->acquireFromPool("P2Node:Pool", pStreamBuffer, ::s2ns(10));

    return pStreamBuffer;
}


/******************************************************************************
 *
 ******************************************************************************/
void
PoolBufferAllocator::
release(sp<IImageStreamBuffer> pStreamBuffer) {
    mpPool->releaseToPool("P2Node:Pool", pStreamBuffer);
}


/******************************************************************************
 *
 ******************************************************************************/
PoolBufferAllocator::
~PoolBufferAllocator() {
    mpPool->uninitPool("P2Node:Pool");
}


/******************************************************************************
 *
 ******************************************************************************/
sp<BufferHandle>
PoolBufferHandle::
create(char const *szName, sp<PoolBufferAllocator> pPool) {
    sp<IImageStreamBuffer> pStreamBuffer = pPool->acquire();
    if (pStreamBuffer == NULL) {
        MY_LOGE("pool buffer[%s]: create image steam buffer failed", szName);
        return NULL;
    }

    sp<IImageBufferHeap>  pImageBufferHeap = pStreamBuffer->tryWriteLock("P2POOL");
    if (pImageBufferHeap == NULL) {
        MY_LOGE("pImageBufferHeap == NULL");
        return NULL;
    }

    sp<IImageBuffer> pImageBuffer = pImageBufferHeap->createImageBuffer();

    // lock buffer
    MUINT const usage = (GRALLOC_USAGE_SW_READ_OFTEN |
                         GRALLOC_USAGE_SW_WRITE_OFTEN |
                         GRALLOC_USAGE_HW_CAMERA_READ |
                         GRALLOC_USAGE_HW_CAMERA_WRITE);
    if (!(pImageBuffer->lockBuf("P2POOL", usage))) {
        MY_LOGE("pool buffer: lock image buffer failed");
        return NULL;
    }

    MY_LOGD_IF(P2_DEBUG_LOG, "pool buffer[%s]: create successfully", szName);

    return new PoolBufferHandle(szName, pPool, pStreamBuffer, pImageBuffer);
}


/******************************************************************************
 *
 ******************************************************************************/
PoolBufferHandle::
~PoolBufferHandle() {
    if (mpStreamBuffer == NULL || mpImageBuffer == NULL) {
        MY_LOGW("pool buffer[%s]: not existed", mBufferName.string());
        return;
    }

    mpImageBuffer->unlockBuf("P2POOL");
    mpStreamBuffer->unlock("P2POOL", mpImageBuffer->getImageBufferHeap());
    mpPool->release(mpStreamBuffer);
    MY_LOGD_IF(P2_DEBUG_LOG, "pool buffer[%s]: release successfully", mBufferName.string());
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
Cropper::
calcBufferCrop(
        MSize const &srcSize,
        MSize const &dstSize,
        MRect &viewCrop)
{
    // pillarbox
    if (srcSize.w * dstSize.h > srcSize.h * dstSize.w) {
        viewCrop.s.w = div_round(srcSize.h * dstSize.w, dstSize.h);
        viewCrop.s.h = srcSize.h;
        viewCrop.p.x = ((srcSize.w - viewCrop.s.w) >> 1);
        viewCrop.p.y = 0;
    }
    // letterbox
    else {
        viewCrop.s.w = srcSize.w;
        viewCrop.s.h = div_round(srcSize.w * dstSize.h, dstSize.w);
        viewCrop.p.x = 0;
        viewCrop.p.y = ((srcSize.h - viewCrop.s.h) >> 1);
    }
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
Cropper::
calcViewAngle(
        MBOOL bEnableLog,
        CropInfo const &cropInfo,
        MSize const &dstSize,
        MCropRect &result)
{
    MBOOL const isResized = cropInfo.isResized;
    //coordinates: s_: sensor
    MRect s_crop = transform(cropInfo.tranActive2Sensor, cropInfo.crop_a);

    MRect s_viewcrop;
    // pillarbox
    if (s_crop.s.w * dstSize.h > s_crop.s.h * dstSize.w) {
        s_viewcrop.s.w = div_round(s_crop.s.h * dstSize.w, dstSize.h);
        s_viewcrop.s.h = s_crop.s.h;
        s_viewcrop.p.x = s_crop.p.x + ((s_crop.s.w - s_viewcrop.s.w) >> 1);
        s_viewcrop.p.y = s_crop.p.y;
    }
    // letterbox
    else {
        s_viewcrop.s.w = s_crop.s.w;
        s_viewcrop.s.h = div_round(s_crop.s.w * dstSize.h, dstSize.w);
        s_viewcrop.p.x = s_crop.p.x;
        s_viewcrop.p.y = s_crop.p.y + ((s_crop.s.h - s_viewcrop.s.h) >> 1);
    }
    MY_LOGD_IF(bEnableLog, "s_cropRegion(%d, %d, %dx%d), dst %dx%d, view crop(%d, %d, %dx%d)",
               s_crop.p.x, s_crop.p.y,
               s_crop.s.w, s_crop.s.h,
               dstSize.w, dstSize.h,
               s_viewcrop.p.x, s_viewcrop.p.y,
               s_viewcrop.s.w, s_viewcrop.s.h
    );

    if (isResized) {
        MRect r_viewcrop = transform(cropInfo.tranSensor2Resized, s_viewcrop);
        result.s = r_viewcrop.s;
        result.p_integral = r_viewcrop.p + cropInfo.eis_mv_r.p;
        result.p_fractional = cropInfo.eis_mv_r.pf;

        // make sure hw limitation
        result.s.w &= ~(0x1);
        result.s.h &= ~(0x1);

        // check boundary
        if (refineBoundary(cropInfo.dstsize_resizer, result)) {
            MY_LOGE("[FIXME] need to check crop!");
            Cropper::dump(cropInfo);
        }
    }
    else {
        result.s = s_viewcrop.s;
        result.p_integral = s_viewcrop.p + cropInfo.eis_mv_s.p;
        result.p_fractional = cropInfo.eis_mv_s.pf;

        // make sure hw limitation
        result.s.w &= ~(0x1);
        result.s.h &= ~(0x1);

        // check boundary
        if (refineBoundary(cropInfo.sensor_size, result)) {
            MY_LOGE("[FIXME] need to check crop!");
            Cropper::dump(cropInfo);
        }
    }

    MY_LOGD_IF(bEnableLog, "resized %d, crop %d/%d, %d/%d, %dx%d",
               isResized,
               result.p_integral.x,
               result.p_integral.y,
               result.p_fractional.x,
               result.p_fractional.y,
               result.s.w,
               result.s.h
    );
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
Cropper::
refineBoundary(
        MSize const &bufSize,
        MCropRect &crop)
{
    MBOOL isRefined = MFALSE;
    MCropRect refined = crop;
    if (crop.p_integral.x < 0) {
        refined.p_integral.x = 0;
        isRefined = MTRUE;
    }
    if (crop.p_integral.y < 0) {
        refined.p_integral.y = 0;
        isRefined = MTRUE;
    }

    int const carry_x = (crop.p_fractional.x != 0) ? 1 : 0;
    if ((refined.p_integral.x + crop.s.w + carry_x) > bufSize.w) {
        refined.s.w = bufSize.w - refined.p_integral.x - carry_x;
        isRefined = MTRUE;
    }
    int const carry_y = (crop.p_fractional.y != 0) ? 1 : 0;
    if ((refined.p_integral.y + crop.s.h + carry_y) > bufSize.h) {
        refined.s.h = bufSize.h - refined.p_integral.y - carry_y;
        isRefined = MTRUE;
    }

    if (isRefined) {
        // make sure hw limitation
        refined.s.w &= ~(0x1);
        refined.s.h &= ~(0x1);

        MY_LOGE("buffer size:%dx%d, crop(%d/%d, %d/%d, %dx%d) -> refined crop(%d/%d, %d/%d, %dx%d)",
                bufSize.w, bufSize.h,
                crop.p_integral.x,
                crop.p_integral.y,
                crop.p_fractional.x,
                crop.p_fractional.y,
                crop.s.w,
                crop.s.h,
                refined.p_integral.x,
                refined.p_integral.y,
                refined.p_fractional.x,
                refined.p_fractional.y,
                refined.s.w,
                refined.s.h
        );
        crop = refined;
    }
    return isRefined;
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
Cropper::
dump(CropInfo const &crop) {
    MY_LOGD("isResized %d", crop.isResized);
    MY_LOGD("p1 sensro crop(%d,%d,%dx%d), resizer size(%dx%d), crop dma(%d,%d,%dx%d)",
            crop.crop_p1_sensor.p.x,
            crop.crop_p1_sensor.p.y,
            crop.crop_p1_sensor.s.w,
            crop.crop_p1_sensor.s.h,
            crop.dstsize_resizer.w,
            crop.dstsize_resizer.h,
            crop.crop_dma.p.x,
            crop.crop_dma.p.y,
            crop.crop_dma.s.w,
            crop.crop_dma.s.h
    );

    MY_LOGD("tran active to sensor o %d, %d, s %dx%d -> %dx%d",
        crop.tranActive2Sensor.tarOrigin.x,
        crop.tranActive2Sensor.tarOrigin.y,
        crop.tranActive2Sensor.oldScale.w,
        crop.tranActive2Sensor.oldScale.h,
        crop.tranActive2Sensor.newScale.w,
        crop.tranActive2Sensor.newScale.h
    );

    MY_LOGD("tran sensor to resized o %d, %d, s %dx%d -> %dx%d",
            crop.tranSensor2Resized.tarOrigin.x,
            crop.tranSensor2Resized.tarOrigin.y,
            crop.tranSensor2Resized.oldScale.w,
            crop.tranSensor2Resized.oldScale.h,
            crop.tranSensor2Resized.newScale.w,
            crop.tranSensor2Resized.newScale.h
    );
    MY_LOGD("modified active crop %d, %d, %dx%d",
            crop.crop_a.p.x,
            crop.crop_a.p.y,
            crop.crop_a.s.w,
            crop.crop_a.s.h
    );
    MY_LOGD("isEisOn %d", crop.isEisEabled);
    MY_LOGD("mv in active %d/%d, %d/%d",
            crop.eis_mv_a.p.x, crop.eis_mv_a.pf.x,
            crop.eis_mv_a.p.y, crop.eis_mv_a.pf.y
    );
    MY_LOGD("mv in sensor %d/%d, %d/%d",
            crop.eis_mv_s.p.x, crop.eis_mv_s.pf.x,
            crop.eis_mv_s.p.y, crop.eis_mv_s.pf.y
    );
    MY_LOGD("mv in resized %d/%d, %d/%d",
            crop.eis_mv_r.p.x, crop.eis_mv_r.pf.x,
            crop.eis_mv_r.p.y, crop.eis_mv_r.pf.y
    );
}
