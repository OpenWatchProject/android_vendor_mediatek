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

#ifndef _MTK_HARDWARE_MTKCAM3_INCLUDE_MTKCAM3_PIPELINE_STREAM_ISTREAMBUFFERPROVIDER_H_
#define _MTK_HARDWARE_MTKCAM3_INCLUDE_MTKCAM3_PIPELINE_STREAM_ISTREAMBUFFERPROVIDER_H_
//
#include <stdint.h>
//
#include "IStreamBuffer.h"


/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam::v3 {


/**
 * @class IImageStreamBufferProvider
 *
 */
class IImageStreamBufferProvider
{
public:

    virtual         ~IImageStreamBufferProvider() = default;

    /**
     * Synchronous call for the pipeline (as the producer) to ask for an (empty)
     * output stream buffer from the provider.
     *
     * This call doesn't return until an empty buffer is available or it timed out.
     *
     * @param rpStreamBuffer: It's assigned to an empty stream buffer by the provider on the call.
     *
     * @param in: The input argument.
     *
     * @return An error code.
     *      0: success
     *      -ETIMEDOUT: This call timed out.
     */
    struct RequestStreamBuffer
    {
        /**
         * The stream info to request.
         */
        IImageStreamInfo const*
                    pStreamInfo = nullptr;

        /**
         * The specified timeout, in nanoseconds. No timeout by default.
         */
        uint64_t    timeout = UINT64_MAX;

        /**
         * The request number.
         */
        uint32_t    requestNo = 0;

    };
    virtual auto    requestStreamBuffer(
                        android::sp<IImageStreamBuffer>& rpStreamBuffer,
                        RequestStreamBuffer const& in
                    ) -> int                                                = 0;

};


/******************************************************************************
 *
 ******************************************************************************/
};  //namespace NSCam::v3
#endif  //_MTK_HARDWARE_MTKCAM3_INCLUDE_MTKCAM3_PIPELINE_STREAM_ISTREAMBUFFERPROVIDER_H_

