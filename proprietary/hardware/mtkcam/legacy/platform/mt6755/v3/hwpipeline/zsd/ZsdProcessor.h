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

#ifndef _MTK_HARDWARE_MTKCAM_MIDDLEWARE_V3_PIPELINE_ZSD_ZSDPROCESSOR_H_
#define _MTK_HARDWARE_MTKCAM_MIDDLEWARE_V3_PIPELINE_ZSD_ZSDPROCESSOR_H_
//
#include <utils/RefBase.h>
#include <utils/StrongPointer.h>
//
#include <vector>
//
#include "../MyUtils.h"
//
#include <mtkcam/v3/utils/streambuf/StreamBuffers.h>
#include "../utils/ExtStreamBuffers.h"

#include <libladder.h>
#define DUMP_CALLSTACK(_x_)                                 \
        do {                                                \
            std::string callstack;                          \
            UnwindCurThreadBT(&callstack);                  \
            __android_log_write(ANDROID_LOG_INFO, LOG_TAG, callstack.c_str()); \
        } while(0)

/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {
namespace v3 {
namespace Zsd {

/****************************************************************************
 *
 ******************************************************************************/
class ZsdProcessor
    : public virtual android::RefBase
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Definitions.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                                Definitions.
    typedef Utils::ExtImageStreamBuffer::       Allocator
                                                ExtImageStreamBufferAllocatorT;
    typedef ExtImageStreamBufferAllocatorT::    StreamBufferPoolT
                                                ExtImageStreamBufferPoolT;

    struct ImageStreamConfig {
        android::sp<IImageStreamInfo>   pInfo;
        bool            bExtPool;
    };

    struct MetaStreamConfig {
        android::sp<IMetaStreamInfo>    pInfo;
        bool            bExtPool;
    };

    struct ImageItem : android::RefBase {
        android::sp<Utils::ExtImageStreamBuffer> pStreamBuffer;
        bool            valid = false;
                        ImageItem(android::sp<Utils::ExtImageStreamBuffer> pSB=nullptr, bool bValid=false)
                            : pStreamBuffer(pSB), valid(bValid)
                        {}
    };

    struct MetaItem : android::RefBase {
        android::sp<IMetaStreamBuffer> pStreamBuffer;
        bool            valid = false;
                        MetaItem(android::sp<IMetaStreamBuffer> pSB=nullptr, bool bValid=false)
                            : pStreamBuffer(pSB), valid(bValid)
                        {}
    };


    typedef android::DefaultKeyedVector<StreamId_T, ImageStreamConfig >
                                                ImageConfigSet;
    typedef android::DefaultKeyedVector<StreamId_T, MetaStreamConfig >
                                                MetaConfigSet;
    typedef android::DefaultKeyedVector<StreamId_T, android::sp<ImageItem> >
                                                ImageSet;
    typedef android::DefaultKeyedVector<StreamId_T, android::sp<MetaItem> >
                                                MetaSet;


    struct InfoSetT {
        ImageConfigSet  vImageConfig;
        MetaConfigSet   vMetaConfig;
        auto            clear() -> void
                        { vImageConfig.clear(); vMetaConfig.clear(); }
    };

    struct BufferSetT {
        ImageSet        vImageSet;
        MetaSet         vMetaSet;
    };

    enum FrameStatus {
        eStatus_UNINITED    = 0,
        eStatus_INITED      = 1,
        eStatus_REGISTERED  = 2,
        eStatus_DEQUED      = 3,
        eStatus_ENQUED      = 4,        // need to collected all buffer to set status to ENQUED
        eStatus_ZSD         = 5,
        eStatus_RELEASED    = 6,
    };

    struct FrameSetT {
        BufferSetT      bufferSet;
        int64_t         requestNo = -1;
        uint64_t        timestampShutter = 0;
        FrameStatus     status = eStatus_UNINITED;
        size_t          imageDone = 0;
        size_t          metaDone = 0;

        void            reset()
                        {
                            requestNo = -1;
                            timestampShutter = 0;
                            status = eStatus_UNINITED;
                            imageDone = 0;
                            metaDone = 0;
                        }

        void            updateStatus(FrameStatus fStatus)
                        { status=fStatus; }

        void            updateShutter(uint64_t shutter)
                        { timestampShutter = shutter; }

    };


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    static auto         createInstance() -> android::sp<ZsdProcessor>;

    // 1.   CONFIGURATION STAGE : prepare buffer pool
    // config needed streams for zsd usage.
    // this module should allocate bufferpool if needed.
    virtual auto        configStreamIds(
                            InfoSetT& rStreamConfigSet
                        ) -> int                                                = 0;

    // 2.0   REQUEST STAGE : user recieves zsd request hint.
    // 2.0.1 REQUEST STAGE: check buffer of specific request is ready or not
    // user can only use getZsdBuffers() only when specific request is ready.
    virtual auto        isZsdBuffersReady(
                            int64_t serialNo
                        ) -> bool                                               = 0;

    // 2.0.2 REQUEST STAGE : get zsd buffers (@ StreamBuffer, need to reset UsersManager)
    // need to reset UsersManager for each StreamBuffer
    // get external buffers for zsd usage in specific request, most called after isExternalBuffersReady()
    virtual auto        getZsdBuffers(
                            BufferSetT& rBufferSet,
                            int64_t serialNo,
                            int64_t newSerialNo
                        ) -> int                                                = 0;

    // 2.1   REQUEST STAGE : normal request
    // 2.1.1 REQUEST STAGE : register allocated buffer
    // register StreamBuffer already allocated (from AppStreamMgr).
    // keep its lifecycle in ZsdProcessor by holding strong poniter.
    virtual auto        registerAllocatedBuffers(
                            BufferSetT& rBufferSet,
                            int64_t serialNo
                        ) -> int                                                = 0;

    // 2.1.2 REQUEST STAGE : deque buffer allocated from this module / registerd by user
    // deque external buffers from ZsdProcessor.
    // need to implement external pool for image; meta buffer is run-time allocated.
    virtual auto        dequeBuffers(
                            BufferSetT& rBufferSet,
                            int64_t serialNo
                        ) -> int                                                = 0;

    // 3.0   CALLBACK STAGE : zsd request callback.
    // 3.0.1 CALLBACK STAGE : notify zsd request done. release buffers ownership for following requests.
    virtual auto        notifyZsdDone(
                            int64_t serialNo
                        ) -> int                                                = 0;

    // 3.1   CALLBACK STAGE : normal request callback.
    // 3.1.1 CALLBACK STAGE : notify completed buffers
    // enque USED buffers to ZsdProcessor for future use.
    virtual auto        enqueBuffers(
                            BufferSetT& rBufferSet,
                            int64_t serialNo
                        ) -> int                                                = 0;

    // 3.1.2 CALLBACK STAGE : notify completed image buffers
    // enque USED images to ZsdProcessor for future use.
    virtual auto        enqueExternalImages(
                            ImageSet& rImageSet,
                            int64_t serialNo
                        ) -> int                                                = 0;

    // 3.1.3 CALLBACK STAGE : notify completed meta buffers
    // enque USED metas to ZsdProcessor for future use.
    virtual auto        enqueExternalMetas(
                            MetaSet& rMetaSet,
                            int64_t serialNo
                        ) -> int                                                = 0;

    // clear inflight external buffers.
    virtual auto        clear() -> int                                          = 0;


    // virtual auto        dump() -> void = 0;
    // virtual auto        dumpCircularBuffer() -> void = 0;
    // virtual auto        dumpImageSet(ImageSet& imageSet) -> void = 0;
    // virtual auto        dumpMetaSet(MetaSet& metaSet) -> void = 0;

};


/******************************************************************************
 *
 ******************************************************************************/
};  //namespace Zsd
};  //namespace v3
};  //namespace NSCam
#endif  //_MTK_HARDWARE_MTKCAM_MIDDLEWARE_V3_PIPELINE_ZSD_ZSDPROCESSOR_H_

