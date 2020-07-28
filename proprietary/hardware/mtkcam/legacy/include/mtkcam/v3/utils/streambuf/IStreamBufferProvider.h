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

#ifndef _MTK_HARDWARE_INCLUDE_MTKCAM_V3_UTILS_STREAMBUF_ISTREAMBUFFERPROVIDER_H_
#define _MTK_HARDWARE_INCLUDE_MTKCAM_V3_UTILS_STREAMBUF_ISTREAMBUFFERPROVIDER_H_
//
#include <utils/Timers.h>
#include <utils/RefBase.h>
#include <utils/StrongPointer.h>
#include <common.h>
//
#include <v3/stream/IStreamInfo.h>
#include "StreamBuffers.h"


/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {
namespace v3 {
namespace Utils {

/**
 * @class IStreamBufferProvider
 *
 */
class IStreamBufferProvider
    : public virtual android::RefBase
{

public:

    /**
     * deque StreamBuffer.
     *
     * @param[in] iRequestNo: the request number for using this StreamBuffer.
     *                   The callee might implement sync mechanism.
     *
     * @param[in] pStreamInfo: a non-null strong pointer for describing the
                          properties of image stream.
     *
     * @param[out] rpStreamBuffer: a reference to a newly created StreamBuffer.
     *                     The callee must promise its value by deque/allocate
     *                     image buffer heap for generating StreamBuffer.
     *
     * @return 0 indicates success; non-zero indicates an error code.
     */
    virtual MERROR                      dequeStreamBuffer(
                                            MUINT32 const iRequestNo,
                                            android::sp<IImageStreamInfo> const pStreamInfo,
                                            android::sp<HalImageStreamBuffer> &rpStreamBuffer
                                        )                                   =0;

    /**
     * enque StreamBuffer.
     *
     * @param[in] pStreamInfo: a non-null strong pointer for describing the
                          properties of image stream.
     *
     * @param[in] rpStreamBuffer: a StreamBuffer to be destroyed.
     *                     The callee must enque/destroy the appended image buffer heap.
     *
     * @param[in] bBufStatus: the status of StreamBuffer.
     *
     * @return 0 indicates success; non-zero indicates an error code.
     */
    virtual MERROR                      enqueStreamBuffer(
                                            android::sp<IImageStreamInfo> const pStreamInfo,
                                            android::sp<HalImageStreamBuffer> rpStreamBuffer,
                                            MUINT32  bBufStatus
                                        )                                   =0;
};

/******************************************************************************
 *
 ******************************************************************************/
};  //namespace Utils
};  //namespace v3
};  //namespace NSCam
#endif  //_MTK_HARDWARE_INCLUDE_MTKCAM_V3_UTILS_STREAMBUF_ISTREAMBUFFERPROVIDER_H_

