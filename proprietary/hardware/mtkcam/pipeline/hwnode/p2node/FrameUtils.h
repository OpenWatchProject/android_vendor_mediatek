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

#ifndef _MTK_PLATFORM_HARDWARE_INCLUDE_MTKCAM_V3_HWNODE_P2NODE_FRAMEUTILS_H_
#define _MTK_PLATFORM_HARDWARE_INCLUDE_MTKCAM_V3_HWNODE_P2NODE_FRAMEUTILS_H_

#include "P2Common.h"
//
#include <mtkcam/drv/iopipe/PostProc/INormalStream.h>
#include <mtkcam/utils/imgbuf/IImageBuffer.h>
#include <mtkcam/utils/hw/HwTransform.h>
#include <mtkcam/pipeline/extension/IVendorManager.h>
#include <utils/Thread.h>
#define WAITBUFFER_TIMEOUT  (1000000000L)

typedef NSCam::NSIoPipe::MCropRect MCropRect;
typedef NSCam::NSIoPipe::QParams QParams;
typedef NSCam::NSIoPipe::NSPostProc::INormalStream INormalStream;
using namespace NSCam;

class StreamControl {
public:
    typedef enum {
        eStreamStatus_NOT_USED = (0x00000000UL),
        eStreamStatus_FILLED = (0x00000001UL),
        eStreamStatus_ERROR = (0x00000001UL << 1),
    } eStreamStatus_t;

public:

    virtual                         ~StreamControl() { };

public:

    virtual MERROR getInfoIOMapSet(
            sp<IPipelineFrame> const &pFrame,
            IPipelineFrame::InfoIOMapSet &rIOMapSet
    ) const = 0;

    // query in/out stream function
    virtual MBOOL isInImageStream(
            StreamId_T const streamId
    ) const = 0;

    virtual MBOOL isInMetaStream(
            StreamId_T const streamId
    ) const = 0;

    // image stream related
    virtual MERROR acquireImageStream(
            sp<IPipelineFrame> const &pFrame,
            StreamId_T const streamId,
            sp<IImageStreamBuffer> &rpStreamBuffer
    ) = 0;

    virtual MVOID releaseImageStream(
            sp<IPipelineFrame> const &pFrame,
            sp<IImageStreamBuffer> const pStreamBuffer,
            MUINT32 const status
    ) const = 0;

    virtual MERROR acquireImageBufferHeap(
            StreamId_T const streamId,
            sp<IImageStreamBuffer> const pStreamBuffer,
            sp<IImageBufferHeap> &rpImageBufferHeap
    ) const = 0;

    virtual MVOID releaseImageBufferHeap(
            sp<IImageStreamBuffer> const pStreamBuffer,
            sp<IImageBufferHeap> const pImageBufferHeap
    ) const = 0;

    virtual MERROR acquireImageBuffer(
            StreamId_T const streamId,
            sp<IImageStreamBuffer> const pStreamBuffer,
            sp<IImageBuffer> &rpImageBuffer,
            MBOOL const bForceWriteLock
    ) const = 0;

    virtual MVOID releaseImageBuffer(
            sp<IImageStreamBuffer> const pStreamBuffer,
            sp<IImageBuffer> const pImageBuffer
    ) const = 0;

    // meta stream related
    virtual MERROR acquireMetaStream(
            sp<IPipelineFrame> const &pFrame,
            StreamId_T const streamId,
            sp<IMetaStreamBuffer> &rpStreamBuffer
    ) = 0;

    virtual MVOID releaseMetaStream(
            sp<IPipelineFrame> const &pFrame,
            sp<IMetaStreamBuffer> const pStreamBuffer,
            MUINT32 const status
    ) const = 0;

    virtual MERROR acquireMetadata(
            StreamId_T const streamId,
            sp<IMetaStreamBuffer> const pStreamBuffer,
            IMetadata *&rpMetadata
    ) const = 0;

    virtual MVOID releaseMetadata(
            sp<IMetaStreamBuffer> const pStreamBuffer,
            IMetadata *const pMetadata
    ) const = 0;

    // frame control related
    virtual MVOID onPartialFrameDone(
            sp<IPipelineFrame> const &pFrame
    ) = 0;

    virtual MVOID onFrameDone(
            sp<IPipelineFrame> const &pFrame
    ) = 0;

};


class MetaHandle
        : public VirtualLightRefBase {
public:
    typedef enum {
        STATE_NOT_USED,
        STATE_READABLE,
        STATE_WRITABLE,
        STATE_WRITE_OK = STATE_READABLE,
        STATE_WRITE_FAIL = 0xFF,
    } BufferState_t;

public:
    static sp<MetaHandle> create(
            StreamControl *const pCtrl,
            sp<IPipelineFrame> const &pFrame,
            StreamId_T const streamId
    );

    ~MetaHandle();

protected:
    MetaHandle(
            StreamControl *pCtrl,
            sp<IPipelineFrame> const &pFrame,
            StreamId_T const streamId,
            sp<IMetaStreamBuffer> const pStreamBuffer,
            BufferState_t const init_state,
            IMetadata *const pMeta
    )
            : mpStreamCtrl(pCtrl), mpFrame(pFrame), mStreamId(streamId), mpStreamBuffer(pStreamBuffer),
              mpMetadata(pMeta), muState(init_state) { }

public:
    IMetadata *getMetadata() { return mpMetadata; }

#if 0
    virtual MERROR waitState(
        BufferState_t const state,
        nsecs_t const nsTimeout = WAITBUFFER_TIMEOUT
    ) = 0;
#endif

    MVOID updateState(
            BufferState_t const state
    );

    StreamId_T getStreamId() { return mStreamId; };

private:
    Mutex mLock;
    //Condition                       mCond;
    StreamControl *const mpStreamCtrl;
    sp<IPipelineFrame> const mpFrame;
    StreamId_T const mStreamId;
    sp<IMetaStreamBuffer> const mpStreamBuffer;
    IMetadata *const mpMetadata;
    MUINT32 muState;
};


class BufferHandle
        : public VirtualLightRefBase {
public:
    typedef enum {
        STATE_NOT_USED,
        STATE_READABLE,
        STATE_WRITABLE,
        STATE_WRITE_OK = STATE_READABLE,
        STATE_WRITE_FAIL,
    } BufferState_t;

public:
    virtual                         ~BufferHandle() { }

public:
    virtual IImageBuffer *getBuffer() = 0;

    virtual IImageStreamBuffer *getStreamBuffer() = 0;

    virtual IImageBufferHeap *getImageBufferHeap() = 0;

    virtual MERROR waitState(
            BufferState_t const state,
            nsecs_t const nsTimeout = WAITBUFFER_TIMEOUT
    ) = 0;

    virtual MVOID updateState(
            BufferState_t const state
    ) = 0;

    virtual MUINT32 getState() = 0;

    virtual MUINT32 getUsage() { return 0; };

    virtual MUINT32 getTransform() { return 0; };

    virtual StreamId_T getStreamId() { return -1; };

};


class StreamBufferHandle
        : public BufferHandle {
public:
    static sp<BufferHandle> create(
            StreamControl *const pCtrl,
            sp<IPipelineFrame> const &pFrame,
            StreamId_T const streamId
    );

    ~StreamBufferHandle();

protected:
    StreamBufferHandle(
            StreamControl *pCtrl,
            sp<IPipelineFrame> const &pFrame,
            StreamId_T const streamId,
            sp<IImageStreamBuffer> const pStreamBuffer,
            MUINT32 const uTransform,
            MUINT32 const uUsage
    )
            : mpStreamCtrl(pCtrl), mpFrame(pFrame), mStreamId(streamId), mpStreamBuffer(pStreamBuffer),
              muState(STATE_NOT_USED), muTransform(uTransform), muUsage(uUsage), mpImageBuffer(NULL), mpImageBufferHeap(NULL){ }

public:
    IImageBuffer *getBuffer() { return mpImageBuffer.get(); }

    IImageBufferHeap *getImageBufferHeap() ;

    IImageStreamBuffer *getStreamBuffer() { return mpStreamBuffer.get(); }

    MERROR waitState(
            BufferState_t const state,
            nsecs_t const nsTimeout
    );

    MVOID updateState(
            BufferState_t const state
    );

    virtual MUINT32 getState() { return muState; };

    virtual MUINT32 getTransform() { return muTransform; };

    virtual MUINT32 getUsage() { return muUsage; };

    virtual StreamId_T getStreamId() { return mStreamId; };


private:
    Mutex mLock;
    android::Condition mCond;
    StreamControl *const mpStreamCtrl;
    sp<IPipelineFrame> const mpFrame;
    StreamId_T const mStreamId;
    sp<IImageStreamBuffer> mpStreamBuffer;
    MUINT32 muState;
    MUINT32 muTransform;
    MUINT32 muUsage;
    sp<IImageBuffer> mpImageBuffer;
    sp<IImageBufferHeap> mpImageBufferHeap;
};


class WorkingBufferHandle
        : public BufferHandle {
public:
    static sp<BufferHandle> create(
            char const *szName,
            MINT32 format,
            MSize size
    );
#if SUPPORT_3RD_PARTY
    static sp<BufferHandle> create(
            char const *szName,
            NSCam::plugin::BufferItem bufItem,
            sp<NSCam::plugin::IVendorManager> pVendorMgr,
            NSCam::plugin::MetaItem meta,
            MUINT32 uUerId
    );

    NSCam::plugin::BufferItem getBufferItem() { return mBufItem; }
#endif
    ~WorkingBufferHandle();

protected:
    WorkingBufferHandle(
            char const *szName,
            sp<IImageBuffer> pImageBuffer
    )
            : muState(STATE_NOT_USED), muTransform(0), mpImageBuffer(pImageBuffer), mBufferName(szName)
#if SUPPORT_3RD_PARTY
            , mpImageBufferHeap(NULL)
            , mpVendorMgr(NULL)
            , muUerId(0)
#endif
             { };
#if SUPPORT_3RD_PARTY
    WorkingBufferHandle(
            char const *szName,
            sp<IImageBuffer> pImageBuffer,
            sp<IImageBufferHeap> pImageBufferHeap,
            sp<NSCam::plugin::IVendorManager> pVendorMgr,
            NSCam::plugin::MetaItem meta,
            MUINT32 uUerId,
            NSCam::plugin::BufferItem bufItem
    )
            : muState(STATE_NOT_USED), muTransform(0), mpImageBuffer(pImageBuffer),mBufferName(szName),mpImageBufferHeap(pImageBufferHeap)
             ,mpVendorMgr(pVendorMgr), mMeta(meta), muUerId(uUerId), mBufItem(bufItem) { };
#endif
public:
    IImageBuffer *getBuffer() { return mpImageBuffer.get(); }

    IImageStreamBuffer *getStreamBuffer() {return NULL;};

    IImageBufferHeap *getImageBufferHeap() { return NULL; }

    MERROR waitState(
            __attribute__((unused))BufferState_t const state, __attribute__((unused))nsecs_t const nsTimeout
    ) { return OK; };

    MVOID updateState(
            BufferState_t const state
    ) {
        muState = state;
#if SUPPORT_3RD_PARTY
        mBufItem.bufferStatus |= muState==BufferHandle::STATE_WRITE_OK
                                   ?  NSCam::plugin::BUFFER_WRITE_OK
                                   :  NSCam::plugin::BUFFER_WRITE_ERROR;
#endif
       };

    virtual MUINT32 getState() { return muState; };

    virtual MUINT32 getTransform() { return muTransform; };


private:
    MUINT32 muState;
    MUINT32 muTransform;
    sp<IImageBuffer> mpImageBuffer;
    String8 mBufferName;
#if SUPPORT_3RD_PARTY
    sp<IImageBufferHeap> mpImageBufferHeap;
    sp<NSCam::plugin::IVendorManager> mpVendorMgr;
    NSCam::plugin::MetaItem mMeta;
    MUINT32 muUerId;
    NSCam::plugin::BufferItem mBufItem;
#endif
};


class FrameLifeHolder : public VirtualLightRefBase {
public:
    FrameLifeHolder(
            MINT32 const openId,
            StreamControl *const pCtrl,
            sp<IPipelineFrame> const &pFrame,
            MBOOL const enableLog
    );

    ~FrameLifeHolder();

public:
    MVOID onPartialFrameDone() {
        if (mpStreamControl)
            mpStreamControl->onPartialFrameDone(mpFrame);
    }

private:
    MBOOL const mbEnableLog;
    MINT32 const mOpenId;
    String8 mTraceName;
    //stream control & dispatch
    StreamControl *const mpStreamControl;
    sp<IPipelineFrame> const mpFrame;
};


class Cropper {
public:
    struct CropInfo : public RefBase {
        // port
        MBOOL isResized;
        //
        MSize sensor_size;
        // p1 crop infos
        MRect crop_p1_sensor;
        MSize dstsize_resizer;
        MRect crop_dma;
        //
        //simpleTransform      tranActive2Sensor;
        NSCamHW::HwMatrix matActive2Sensor;
        NSCamHW::HwMatrix matSensor2Active;
        simpleTransform tranSensor2Resized;
        MINT32  sensorMode;
        MUINT32   uOpenId;
        //
        // target crop: cropRegion
        // not applied eis's mv yet, but the crop area is already reduced by
        // EIS ratio.
        // _a: active array coordinates
        // _s: sensor coordinates
        // active array coordinates
        MRect crop_a;
        //MPoint               crop_a_p;
        //MSize                crop_a_size;
        // sensor coordinates
        //MPoint               crop_s_p;
        //MSize                crop_s_size;
        // resized coordinates
        //
        MBOOL isEisEabled;
        vector_f eis_mv_a; //active array coor.
        vector_f eis_mv_s; //sensor coor.
        vector_f eis_mv_r; //resized coor.

    };

    static MVOID calcBufferCrop(
            MSize const &srcSize,
            MSize const &dstSize,
            MRect &viewCrop
    );

    static MVOID calcViewAngle(
            MBOOL bEnableLog,
            CropInfo const &cropInfo,
            MSize const &dstSize,
            MCropRect &result
    );

    static MBOOL refineBoundary(
            MSize const &bufSize,
            MCropRect &crop,
            MBOOL bEnableLog
    );

    static MVOID dump(
            CropInfo const &cropInfo
    );

};

#endif
