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

#include <ByteBuffer.h>
#include <string.h>

using namespace android;

ByteBuffer::ByteBuffer() : mBuffer(NULL), mLen(0)
{
}

ByteBuffer::ByteBuffer(char* b, size_t len)
{
    allocate((const char*)b, len);
}

ByteBuffer::ByteBuffer(const char* b, size_t len)
{
    allocate(b, len);
}

ByteBuffer::ByteBuffer(char* str)
{
    allocate((const char*)str, strlen(str));
}

ByteBuffer::ByteBuffer(const char* str)
{
    allocate(str, strlen(str));
}

ByteBuffer::ByteBuffer(const ByteBuffer& copy)
{
    allocate(copy.mBuffer, copy.mLen);
}

ByteBuffer& ByteBuffer::operator=(const ByteBuffer& other)
{
    cleanUp();
    allocate(other.mBuffer, other.mLen);
    return *this;
}

ByteBuffer::~ByteBuffer()
{
    cleanUp();
}

const char* ByteBuffer::buffer() const
{
    return mBuffer;
}

size_t ByteBuffer::length() const
{
    return mLen;
}

void ByteBuffer::setTo(char* b, size_t len)
{
    setTo((const char*)b, len);
}

void ByteBuffer::setTo(const char* b, size_t len)
{
    cleanUp();
    allocate(b, len);
}

void ByteBuffer::setTo(char* str)
{
    setTo((const char*)str);
}

void ByteBuffer::setTo(const char* str)
{
    setTo(str, strlen(str));
}

void ByteBuffer::allocate(const char* b, size_t len)
{
    if (NULL == b || 0 == len)
    {
        mBuffer = NULL;
        mLen = 0;
    }
    else
    {
        mBuffer = new char[len];
        memset(mBuffer, 0, len);
        memcpy(mBuffer, b, len);
        mLen = len;
    }
}

void ByteBuffer::cleanUp()
{
    if (NULL != mBuffer)
    {
        delete[] mBuffer;
        mBuffer = NULL;
    }
    mLen = 0;
}
