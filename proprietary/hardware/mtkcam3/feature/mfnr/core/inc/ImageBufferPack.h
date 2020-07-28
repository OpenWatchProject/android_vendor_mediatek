/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2016. All rights reserved.
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
#ifndef __MFLL_IMAGEBUFFER_PACK_H__
#define __MFLL_IMAGEBUFFER_PACK_H__

// MFNR Core
#include <mtkcam3/feature/mfnr/MfllTypes.h>
#include <mtkcam3/feature/mfnr/IMfllImageBuffer.h>

// STL
#include <memory> // std::shared_ptr
#include <mutex> // std::mutex, std::lock_guard
#include <cstdint> // intptr_t
#include <functional>


namespace mfll
{
// A thread-safe buffer pack. Caller should invoke methods of ImageBufferPack to
// access buffer normally. But for initailization, caller can operate basic
// operation by invoking attributes.
//
// Note: In-using since MFNR Core Library 2.0
class ImageBufferPack
{
// attributes
public:
    sp<IMfllImageBuffer>    imgbuf;
    mutable std::mutex      locker;


// methods
public:
    void setImgBufLocked(const sp<IMfllImageBuffer>& buffer)
    {
        std::lock_guard<std::mutex> __l(this->locker);
        this->imgbuf = buffer;
    }

    sp<IMfllImageBuffer> getImgBufLocked() const
    {
        std::lock_guard<std::mutex> __l(this->locker);
        return this->imgbuf;
    }

    void releaseBufLocked() noexcept
    {
        std::lock_guard<std::mutex> __l(this->locker);
        this->imgbuf = nullptr;
    }


// static methods
public:
    static void swap(ImageBufferPack* a, ImageBufferPack* b)
    {
        ImageBufferPack null_pack;

        if (a == nullptr)
            a = &null_pack;
        if (b == nullptr)
            b = &null_pack;

        // android::sp::operator = is thread-safe.
        auto temp_buf = a->imgbuf;
        a->imgbuf = b->imgbuf;
        b->imgbuf = temp_buf;
    }


// operator assignment
public:
    ImageBufferPack& operator=(const ImageBufferPack& other)
    {
        if (&other == this)
            return *this;

        this->imgbuf = other.imgbuf;

        return *this;
    }

    ImageBufferPack& operator=(ImageBufferPack&& other)
    {
        if (&other == this)
            return *this;

        this->imgbuf = other.imgbuf;
        other.imgbuf = nullptr;

        return *this;
    }

    ImageBufferPack& operator=(const sp<IMfllImageBuffer>& buffer)
    {
        this->imgbuf = buffer;
        return *this;
    }

    ImageBufferPack& operator=(sp<IMfllImageBuffer>&& buffer)
    {
        this->imgbuf = buffer;
        buffer = nullptr;
        return *this;
    }

    ImageBufferPack& operator=(IMfllImageBuffer* pBuffer)
    {
        this->imgbuf = pBuffer;
        return *this;
    }


// operator compare
public:
    bool operator==(IMfllImageBuffer* pBuffer)
    {
        return (this->imgbuf.get() == pBuffer);
    }

    bool operator!=(IMfllImageBuffer* pBuffer)
    {
        return (this->imgbuf.get() != pBuffer);
    }


// constructor & destructor
public:
    // default constructor
    ImageBufferPack() noexcept
    {
    }

    // copy constructor
    ImageBufferPack(const ImageBufferPack& other) noexcept
    {
        *this = other;
    }

    // move constructor
    ImageBufferPack(ImageBufferPack&& other) noexcept
    {
        *this = std::move(other);
    }

    ~ImageBufferPack() = default;

}; // class ImageBufferPack
}; // namespace mfll
#endif
