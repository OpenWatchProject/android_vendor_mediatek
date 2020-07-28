/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2015. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

#ifndef _UTILS_IMAGEBUFFERUTILS_H_
#define _UTILS_IMAGEBUFFERUTILS_H_

#include <utils/Singleton.h>
#include <utils/KeyedVector.h>
#include <utils/StrongPointer.h>

#include <utils/def/BuiltinTypes.h>
#include <utils/imgbuf/IImageBuffer.h>

using namespace android;

// ---------------------------------------------------------------------------

namespace NSCam {

// ---------------------------------------------------------------------------

class ImageBufferUtils : public Singleton<ImageBufferUtils>
{
public:
    // allocBuffer() is the utility function that
    // 1. allocate ION buffer and map it to ImageBuffer
    // 2. locks the ImageBuffer
    //
    // NOTE: all buffers allocated from this API should be returned by
    // calling deallocBuffer()
    MBOOL allocBuffer(
            sp<IImageBuffer>& imageBuf,
            MUINT32 w, MUINT32 h, MUINT32 format, MBOOL isContinuous = MTRUE);

    // deallocBuffer() is the utility function that
    // 1. unlock the ImageBuffer
    void deallocBuffer(IImageBuffer* pBuf);
    void deallocBuffer(sp<IImageBuffer>& pBuf);

    // createBufferAlias() is the utility function that
    // 1. creates an ImageBuffer from the original ImageBuffer.
    // ('alias' means that their content are the same,
    //  excepts the alignment and the format of each ImageBuffer)
    // 2. unlocks the original ImageBuffer
    // 3. locks and returns the new ImageBuffer
    IImageBuffer* createBufferAlias(IImageBuffer* pOriginalBuf,
            MUINT32 w, MUINT32 h, EImageFormat format);

    // removeBufferAlias() is the utility function that
    // 1. unlocks and destroys the new ImageBuffer
    // 2. locks the original ImageBuffer
    MBOOL removeBufferAlias(IImageBuffer* pOriginalBuf, IImageBuffer* pAliasBuf);

    MBOOL createBuffer(sp<IImageBuffer> & output_buf, sp<IImageBuffer> & input_buf);

private:
    // mInternalBufferList is use for internal memory management
    mutable Mutex mInternalBufferListLock;
    DefaultKeyedVector< MINTPTR, sp<IImageBuffer> > mInternalBufferList;
};

// ---------------------------------------------------------------------------

}; // namespace NSCam

#endif // _UTILS_IMAGEBUFFERUTILS_H_
