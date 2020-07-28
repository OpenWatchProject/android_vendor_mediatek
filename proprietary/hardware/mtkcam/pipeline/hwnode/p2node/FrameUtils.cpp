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
#include "mtkcam/def/BuiltinTypes.h"

#include <mtkcam/utils/imgbuf/IIonImageBufferHeap.h>

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
    if (muState > 0) {
        const MUINT32 status = (muState != STATE_WRITE_FAIL) ?
                               StreamControl::eStreamStatus_FILLED : StreamControl::eStreamStatus_ERROR;
        //
        mpStreamCtrl->releaseMetadata(mpStreamBuffer, mpMetadata);
        mpStreamCtrl->releaseMetaStream(mpFrame, mpStreamBuffer, status);
        MY_LOGD_IF(P2_DEBUG_LOG, "release meta[%u] streamId[0x%x] status: 0x%x meta:%p",mpFrame->getFrameNo() , (uint32_t)mStreamId, status, mpMetadata);
    }
    else {
        mpStreamCtrl->releaseMetadata(mpStreamBuffer, mpMetadata);
        mpStreamCtrl->releaseMetaStream(mpFrame, mpStreamBuffer, StreamControl::eStreamStatus_NOT_USED);
        MY_LOGW_IF(P2_DEBUG_LOG, "release meta[%u] streamId[0x%x] NOT_USED meta:%p",mpFrame->getFrameNo() , (uint32_t)mStreamId, mpMetadata);
    }

}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
MetaHandle::
updateState(BufferState_t const state) {
    Mutex::Autolock _l(mLock);
    if (muState == STATE_NOT_USED || muState == STATE_WRITE_FAIL ) {
        MY_LOGW("streamId %#" PRIx64 " muState %d state %d",
                mStreamId, muState, state);
    }
    else {
        MY_LOGW_IF(state == STATE_WRITE_FAIL, "streamId %#" PRIx64 " set fail, state %d -> %d",
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
        MY_LOGD_IF(0, "create buffer handler, stream:%#" PRIx64 ", transform:%d, usage:%d",
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
    MY_LOGD_IF(P2_DEBUG_LOG, "~FrameNo:%d ()StreamBufferHandle(%#" PRIx64 ") state: 0x%x heap: %p", mpFrame->getFrameNo(),mStreamId, muState, mpImageBufferHeap.get());
    if (!mpStreamCtrl)
        MY_LOGE("StreamCtrl not exit.");
    else {
        if (muState != STATE_NOT_USED) {
            const MUINT32 status = (muState != STATE_WRITE_FAIL) ?
                                   StreamControl::eStreamStatus_FILLED : StreamControl::eStreamStatus_ERROR;
            if(mpImageBuffer!=NULL){
                mpStreamCtrl->releaseImageBuffer(mpStreamBuffer, mpImageBuffer);
            }
            else {
                mpStreamCtrl->releaseImageBufferHeap(mpStreamBuffer, mpImageBufferHeap);
            }
            //mpStreamCtrl->releaseImageBuffer(mpStreamBuffer, mpImageBuffer);
            mpStreamCtrl->releaseImageStream(mpFrame, mpStreamBuffer, status);
        }
        else {
            mpStreamCtrl->releaseImageStream(mpFrame, mpStreamBuffer, StreamControl::eStreamStatus_NOT_USED);
        }

        // for pr-apply release
        mpStreamCtrl->onPartialFrameDone(mpFrame);

    }
}

/******************************************************************************
 *
 ******************************************************************************/

IImageBufferHeap *
StreamBufferHandle::
getImageBufferHeap()
{
    Mutex::Autolock _l(mLock);
    if (mpImageBufferHeap == NULL) {
        // get buffer from streambuffer
        const MERROR ret = mpStreamCtrl->acquireImageBufferHeap(
                mStreamId, mpStreamBuffer, mpImageBufferHeap);
        // update initial state
        if (ret == OK)
            muState = mpStreamCtrl->isInImageStream(mStreamId) ? STATE_READABLE : STATE_WRITABLE;
        //return ret;
    }
    return mpImageBufferHeap.get();

}

/******************************************************************************
 *
 ******************************************************************************/

MERROR
StreamBufferHandle::
waitState(
        BufferState_t const state,
        __attribute__((unused))nsecs_t const nsTimeout)
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

    //if (muState != state) {
    //    mCond.waitRelative(mLock, nsTimeout);
    //}
    //return (muState == state) ? OK : TIMED_OUT;
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
        MY_LOGW("streamId %#" PRIx64 " state %d -> %d",
                mStreamId, muState, state);
    }
    else {
        MY_LOGW_IF(state == STATE_WRITE_FAIL, "streamId %#" PRIx64 " set fail: state %d -> %d",
                   mStreamId, muState, state);
        muState = state;
    }
    //mCond.broadcast();
}
#if SUPPORT_3RD_PARTY
/******************************************************************************
 *
 ******************************************************************************/
sp<BufferHandle>
WorkingBufferHandle::
create(
        char const *szName,
        NSCam::plugin::BufferItem bufItem,
        sp<NSCam::plugin::IVendorManager> pVendorMgr,
        NSCam::plugin::MetaItem meta,
        MUINT32 uUerId
        )
{
   MINT32 format = bufItem.heap->getImgFormat();
   MINT32 formatStream = bufItem.streamInfo->getImgFormat();
   MSize size = MSize(bufItem.heap->getImgSize().w, bufItem.heap->getImgSize().h);
   MSize sizeStream = MSize(bufItem.heap->getImgSize().w, bufItem.heap->getImgSize().h);
   String8 bufferName = String8::format("%s:s%dx%d:f%d_%dx%d:f%d",
      szName, size.w, size.h, format, sizeStream.w, sizeStream.h, formatStream);
   sp<IImageBuffer> pImageBuffer = NULL;

   if (format == eImgFmt_BLOB && format != formatStream)
   {
      size_t strideInBytes[3] = {0};
      sizeStream = bufItem.streamInfo->getImgSize();
      MUINT32 plane = NSCam::Utils::Format::queryPlaneCount(formatStream);
      for (MUINT32 i = 0; i < plane; i++)
      {
          strideInBytes[i] = (Utils::Format::queryPlaneWidthInPixels(formatStream, i, sizeStream.w) *
              Utils::Format::queryPlaneBitsPerPixel(formatStream, i)) / 8;
#ifdef __DEBUG
          MY_LOGD_IF(P2_DEBUG_LOG, "strideInBytes[%d]:%d", i, strideInBytes[i]);
#endif
      }

      pImageBuffer = bufItem.heap.get()->createImageBuffer_FromBlobHeap((size_t)0,
          (MINT32)formatStream, sizeStream, strideInBytes);
   }
   else
   {
      pImageBuffer = bufItem.heap.get()->createImageBuffer();
   }

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

    MY_LOGD_IF(P2_DEBUG_LOG, "working buffer[%s]: create successfully, heap(%p), imgbuffer(%p)",
               bufferName.string(), bufItem.heap.get(), pImageBuffer.get());

    return new WorkingBufferHandle(bufferName.string(), pImageBuffer, bufItem.heap.get(), pVendorMgr, meta, uUerId,bufItem);
}
#endif
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
    MUINT32 plane = NSCam::Utils::Format::queryPlaneCount(format);
    size_t bufBoundaryInBytes[3] = {0, 0, 0};
    size_t bufStridesInBytes[3] = {0};

    for (MUINT32 i = 0; i < plane; i++) {
        bufStridesInBytes[i] = NSCam::Utils::Format::queryPlaneWidthInPixels(format, i, size.w) *
                               NSCam::Utils::Format::queryPlaneBitsPerPixel(format, i) / 8;
    }
    // create buffer
    IImageBufferAllocator::ImgParam imgParam =
            IImageBufferAllocator::ImgParam(
                    (EImageFormat) format,
                    size, bufStridesInBytes,
                    bufBoundaryInBytes, plane);

    sp<IImageBufferHeap> pHeap =
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

    MY_LOGD_IF(P2_DEBUG_LOG, "working buffer[%s]: create successfully heap(%p)", bufferName.string(), pHeap.get());

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
    String8 strLog;
    strLog += String8::format("imgbuffer(%p) ", mpImageBuffer.get());

    mpImageBuffer->unlockBuf(mBufferName.string());
    mpImageBuffer.clear();
#if SUPPORT_3RD_PARTY
    strLog += String8::format("heap(%p) ", mpImageBufferHeap.get());
    if(mpVendorMgr.get() &&
        mBufItem.streamInfo->getStreamId()==NSCam::plugin::IVENDOR_STREAMID_IMAGE_PIPE_WORKING
        && mBufItem.bufferStatus& NSCam::plugin::BUFFER_OUT)
    {
        strLog += String8::format("bufferStatus: 0x%x ", mBufItem.bufferStatus);
        mpVendorMgr->releaseWorkingBuffer(muUerId, mMeta, mBufItem);
    }
    mpImageBufferHeap.clear();
#endif
    strLog += String8::format("WBuffer[%s] release successfully", mBufferName.string());
    MY_LOGD("%s", strLog.string());
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
    // MRect s_crop = transform(cropInfo.tranActive2Sensor, cropInfo.crop_a);
    MRect s_crop;
    cropInfo.matActive2Sensor.transform(cropInfo.crop_a, s_crop);
#define abs(x,y) ((x)>(y)?(x)-(y):(y)-(x))
#define FOV_DIFF_TOLERANCE (3)
    MRect s_viewcrop;
    // pillarbox
    if (s_crop.s.w * dstSize.h > s_crop.s.h * dstSize.w) {
        s_viewcrop.s.w = div_round(s_crop.s.h * dstSize.w, dstSize.h);
        s_viewcrop.s.h = s_crop.s.h;
        s_viewcrop.p.x = s_crop.p.x + ((s_crop.s.w - s_viewcrop.s.w) >> 1);
        if( s_viewcrop.p.x < 0 && abs(s_viewcrop.p.x, 0) < FOV_DIFF_TOLERANCE )
           s_viewcrop.p.x = 0;
        s_viewcrop.p.y = s_crop.p.y;
    }
    // letterbox
    else {
        s_viewcrop.s.w = s_crop.s.w;
        s_viewcrop.s.h = div_round(s_crop.s.w * dstSize.h, dstSize.w);
        s_viewcrop.p.x = s_crop.p.x;
        s_viewcrop.p.y = s_crop.p.y + ((s_crop.s.h - s_viewcrop.s.h) >> 1);
        if( s_viewcrop.p.y < 0 && abs(s_viewcrop.p.y, 0) < FOV_DIFF_TOLERANCE ) {
           s_viewcrop.p.y = 0;
        }
    }
    MY_LOGD_IF(bEnableLog, "s_cropRegion(%d, %d, %dx%d), dst %dx%d, view crop(%d, %d, %dx%d)",
               s_crop.p.x, s_crop.p.y,
               s_crop.s.w, s_crop.s.h,
               dstSize.w, dstSize.h,
               s_viewcrop.p.x, s_viewcrop.p.y,
               s_viewcrop.s.w, s_viewcrop.s.h
    );
#undef FOV_DIFF_TOLERANCE

    // according to sensor mode adjust crop region
    float fov_diff_x = 0.0f;
    float fov_diff_y = 0.0f;
    NSCamHW::HwTransHelper hwTransHelper(cropInfo.uOpenId);
    hwTransHelper.calculateFovDifference(cropInfo.sensorMode, &fov_diff_x, &fov_diff_y);
    //
    float ratio_s = (float)cropInfo.crop_p1_sensor.s.w / (float)cropInfo.crop_p1_sensor.s.h;
    float ratio_d = (float)s_viewcrop.s.w / (float)s_viewcrop.s.h;
    MY_LOGD_IF(bEnableLog,"ratio_s:%f ratio_d:%f", ratio_s, ratio_d);
    /*
     * handle HAL3 sensor mode 16:9 FOV
     */
    //
    if( (s_viewcrop.p.x < 0 || s_viewcrop.p.y < 0) && abs(ratio_s, ratio_d) < 0.1f) {
        MRect refined = s_viewcrop;
        float ratio = (float)cropInfo.crop_p1_sensor.s.h / (float)s_viewcrop.s.h;
        refined.s.w = s_viewcrop.s.w * ratio;
        refined.s.h = s_viewcrop.s.h * ratio;
        refined.p.x = s_viewcrop.p.x + (s_viewcrop.s.w-refined.s.w)/2.0f;
        refined.p.y = s_viewcrop.p.y + (s_viewcrop.s.h-refined.s.h)/2.0f;
        //
        s_viewcrop = refined;
        MY_LOGD_IF(bEnableLog, "refine negative crop ratio %f r.s(%dx%d) r.p(%d, %d)",ratio, refined.s.w,refined.s.h, refined.p.x, refined.p.y ); 
    }
    else if(fov_diff_y < 1 && fov_diff_y > fov_diff_x && (s_viewcrop.s.w*cropInfo.crop_p1_sensor.s.h < s_viewcrop.s.h*cropInfo.crop_p1_sensor.s.w) 
            && (s_viewcrop.p.x < cropInfo.crop_p1_sensor.p.x) && (s_viewcrop.p.y < cropInfo.crop_p1_sensor.p.y) ) {
        MRect refined = s_viewcrop;
        float dY = 1.0f - fov_diff_y;
        refined.s.w = s_viewcrop.s.w * dY;
        refined.s.h = s_viewcrop.s.h * dY;
        refined.p.x = s_viewcrop.p.x + (s_viewcrop.s.w-refined.s.w)/2.0f;
        refined.p.y = s_viewcrop.p.y + (s_viewcrop.s.h-refined.s.h)/2.0f;
        //
        s_viewcrop = refined;
        float dX = 1.0f - fov_diff_x;
        MY_LOGD_IF(bEnableLog, "dX %f dY %f r.s(%dx%d) r.p(%d, %d)", dX, dY, refined.s.w,refined.s.h, refined.p.x, refined.p.y );
    }
    //
    MY_LOGD_IF(bEnableLog, "p1 sensor crop(%d, %d,%dx%d), %d, %d",
                    cropInfo.crop_p1_sensor.p.x, cropInfo.crop_p1_sensor.p.y,
                    cropInfo.crop_p1_sensor.s.w, cropInfo.crop_p1_sensor.s.h,
                    s_viewcrop.s.w*cropInfo.crop_p1_sensor.s.h,  s_viewcrop.s.h*cropInfo.crop_p1_sensor.s.w);
#undef abs
    if (isResized) {
        MRect r_viewcrop = transform(cropInfo.tranSensor2Resized, s_viewcrop);
        result.s = r_viewcrop.s;
        result.p_integral = r_viewcrop.p + cropInfo.eis_mv_r.p;
        result.p_fractional = cropInfo.eis_mv_r.pf;

        // make sure hw limitation
        result.s.w &= ~(0x1);
        result.s.h &= ~(0x1);

        // check boundary
        if (refineBoundary(cropInfo.dstsize_resizer, result, bEnableLog)) {
            MY_LOGW_IF(bEnableLog, "[FIXME] need to check crop!");
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
        if (refineBoundary(cropInfo.sensor_size, result, bEnableLog)) {
            MY_LOGW_IF(bEnableLog, "[FIXME] need to check crop!");
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
        MCropRect &crop,
        MBOOL bEnableLog)
{
    // tolerance
    if (crop.p_integral.x == -1)
        crop.p_integral.x = 0;

    if (crop.p_integral.y == -1)
        crop.p_integral.y = 0;

    MBOOL isRefined = MFALSE;
    MCropRect refined = crop;
    if (crop.p_integral.x < 0) {
        isRefined = MTRUE;
        refined.p_integral.x = 0;
    }

    if (crop.p_integral.y < 0) {
        isRefined = MTRUE;
        refined.p_integral.y = 0;
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

        MY_LOGW_IF(bEnableLog, "buffer size:%dx%d, crop(%d/%d, %d/%d, %dx%d) -> refined crop(%d/%d, %d/%d, %dx%d)",
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
/*
    MY_LOGD("tran active to sensor o %d, %d, s %dx%d -> %dx%d",
        crop.tranActive2Sensor.tarOrigin.x,
        crop.tranActive2Sensor.tarOrigin.y,
        crop.tranActive2Sensor.oldScale.w,
        crop.tranActive2Sensor.oldScale.h,
        crop.tranActive2Sensor.newScale.w,
        crop.tranActive2Sensor.newScale.h
    );
*/
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
