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
 * MediaTek Inc. (C) 2017. All rights reserved.
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

/**
 * @file MulitFramePostProcRequestBuilder.h
 * @brief multi frame post proc request builder
*/

#ifndef _MTK_LEGACYPIPELINE_DUAL_3RD_POST_PROC_REQUEST_BUILDER_H_
#define _MTK_LEGACYPIPELINE_DUAL_3RD_POST_PROC_REQUEST_BUILDER_H_

// Standard C header file

// Android system/core header file
#include <utils/RefBase.h>
// mtkcam custom header file
#include <mtkcam/def/Errors.h>
// mtkcam global header file

// Module header file

// Local header file
#include "../inc/IPostProcRequestBuilder.h"

/*******************************************************************************
* Namespace start.
********************************************************************************/
using namespace std;
using namespace android;
//
namespace android {
namespace NSPostProc {
/*******************************************************************************
* Class Define
********************************************************************************/
/**
 * @class MulitFramePostProcRequestBuilder
 * @brief multi frame post proc request builder
 */
class Dual3rdPostProcRequestBuilder
    : public PostProcRequestBuilderBase,
      public NSCam::v1::IImageCallback,
      public NSCam::v1::Notifier
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Instantiation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
// Constructor
    Dual3rdPostProcRequestBuilder() = delete;
    Dual3rdPostProcRequestBuilder(
                        std::string const& name,
                        android::sp<NSCam::v1::NSLegacyPipeline::ILegacyPipeline> pipeline,
                        android::sp<ImageStreamManager> imageStreamManager,
                        android::sp<PostProcRequestSetting> setting);
    virtual ~Dual3rdPostProcRequestBuilder();
public:
    status_t allocOutputBuffer() override final;
    status_t setInputBuffer(
                        MUINT32 inputDataSeq,
                        MUINT32 index,
                        const std::map<MINT32, IPostProcRequestBuilder::CallbackBufferPoolItem>& bufferPoolTable) override final;
    status_t onGetPrecedingData(
                        MUINT32 index,
                        IMetadata& appMetadata,
                        IMetadata& halMetadata,
                        ThirdPartyType& type) override final;
    status_t onGetWorkingData(
                        MUINT32 index,
                        IMetadata& appMetadata,
                        IMetadata& halMetadata,
                        ThirdPartyType& type) override final;
public:
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IImageCallback interface
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    MERROR                  onResultReceived(
                                        MUINT32    const requestNo,
                                        StreamId_T const streamId,
                                        MBOOL      const errorBuffer,
                                        android::sp<IImageBuffer>& pBuffer) override;
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Notifier interface
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    virtual MVOID                       onBufferAcquired(
                                            MINT32           rRequestNo,
                                            StreamId_T       rStreamId
                                        ) override;

    virtual bool                        onBufferReturned(
                                            MINT32                         rRequestNo,
                                            StreamId_T                     rStreamId,
                                            bool                           bErrorBuffer,
                                            android::sp<IImageBufferHeap>& rpBuffer
                                        )override;
private:
    MUINT32                             mTotalImgCount = 0;
    MBOOL                               mbSupportJpeg00 = MFALSE;
    MBOOL                               mbSupportDepthOut = MFALSE;

    // output info
    MSize                               mFinalPictureSize;
    MSize                               mFinalThumbnailSize;
    MSize                               mDepthMapSize;
};

/*******************************************************************************
* Namespace end.
********************************************************************************/

};
};


#endif  //_MTK_LEGACYPIPELINE_DUAL_3RD_POST_PROC_REQUEST_BUILDER_H_
