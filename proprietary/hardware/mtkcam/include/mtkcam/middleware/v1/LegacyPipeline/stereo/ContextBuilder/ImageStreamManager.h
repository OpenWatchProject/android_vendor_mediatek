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
#ifndef _MTK_PLATFORM_HARDWARE_INCLUDE_MTKCAM_V3_IMAGE_STREAM_MANAGER_H_
#define _MTK_PLATFORM_HARDWARE_INCLUDE_MTKCAM_V3_IMAGE_STREAM_MANAGER_H_
//
#include <utils/RefBase.h>
#include <utils/StrongPointer.h>
#include <utils/String8.h>
//
#include <mtkcam/pipeline/stream/IStreamInfo.h>
//
#include <utils/KeyedVector.h>
//
#include "ContextBuilder.h"
#include <mtkcam/middleware/v1/LegacyPipeline/buffer/StreamBufferProvider.h>
//
using namespace android;
namespace NSCam
{
namespace v3
{
class IImageStreamInfo;
//
class ImageStreamManager
    : public virtual RefBase
{
    public:
        //
        struct image_stream_info_pool_setting
        {
            char const*         streamName;
            StreamId_T          streamId;
            MUINT32             streamType;
            size_t              maxBufNum;
            size_t              minInitBufNum;
            MUINT               usageForAllocator;
            MINT                imgFormat;
            //MSize const&        imgSize;
            MSize               imgSize;
            size_t              stride;
            MUINT32             transform;
            NSPipelineContext::eStreamType      bufStreamType;
            sp<NSCam::v1::StreamBufferProvider> bufHandler;
        };
        //
        class BufProviderMetaInfoSet
            : public android::RefBase
        {
            public:
                sp<NSCam::v1::StreamBufferProvider> bufProvider;
                Vector<StreamId_T >                 vMetaStream;
                                                    //
                                                    BufProviderMetaInfoSet()
                                                        : android::RefBase()
                                                        , bufProvider(NULL)
                                                    {
                                                    }
                virtual                             ~BufProviderMetaInfoSet()
                                                    {
                                                        bufProvider = NULL;
                                                        vMetaStream.clear();
                                                    }
        };
        //
        ImageStreamManager(const image_stream_info_pool_setting* data);

        virtual ~ImageStreamManager();

        static sp<ImageStreamManager>                   create(const image_stream_info_pool_setting* data);
        void                                            destroy();

        MUINT32                                         getSize();

        void                                            dump();

        // Get
        sp<IImageStreamInfo>                            getStreamInfoByStreamId(StreamId_T id);
        NSPipelineContext::eStreamType                  getPoolTypeByStreamId(StreamId_T id);
        sp<NSCam::v1::StreamBufferProvider>             getBufProviderByStreamId(StreamId_T id);
        //
        NSPipelineContext::eStreamType                  getPoolTypeByIndex(MUINT32 index);
        sp<IImageStreamInfo>                            getStreamInfoByIndex(MUINT32 index);
        sp<NSCam::v1::StreamBufferProvider>             getBufProviderByIndex(MUINT32 index);
        MERROR                                          updateBufProvider(
                                                                 StreamId_T id,
                                                                 sp<NSCam::v1::StreamBufferProvider> bufProvider,
                                                                 Vector<StreamId_T > const& vMetaStream
                                                                 );
        MBOOL                                           updateStreamInfo(
                                                                StreamId_T id,
                                                                sp<IImageStreamInfo> info
                                                                );
        sp<BufProviderMetaInfoSet>                      getBufProviderMetaInfoSet(MUINT32 idx);

    private:
        MUINT32                                         init(const image_stream_info_pool_setting* data);
        MBOOL                                           isRawFormat(MINT iFmt);
        sp<IImageStreamInfo>                             createImageStreamInfo(
                                                                char const*         streamName,
                                                                StreamId_T          streamId,
                                                                MUINT32             streamType,
                                                                size_t              maxBufNum,
                                                                size_t              minInitBufNum,
                                                                MUINT               usageForAllocator,
                                                                MINT                imgFormat,
                                                                MSize const&        imgSize,
                                                                MUINT32             transform
                                                        );
        sp<IImageStreamInfo>
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
                                                        );

    //
    private:
        DefaultKeyedVector<StreamId_T, sp<IImageStreamInfo> >            mvImageStreamContainer;
        DefaultKeyedVector<StreamId_T, NSPipelineContext::eStreamType >  mvImageStreamPoolTypeContainer;
        DefaultKeyedVector<StreamId_T, sp<BufProviderMetaInfoSet> >      mvImageStreamPoolContainer;
        MBOOL                                                            mbIsDestroy = MFALSE;
};
};
};
#endif //_MTK_PLATFORM_HARDWARE_INCLUDE_MTKCAM_V3_IMAGE_STREAM_MANAGER_H_
