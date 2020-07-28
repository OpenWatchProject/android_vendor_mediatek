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
 * MediaTek Inc. (C) 2018. All rights reserved.
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

#ifndef _MTKCAM_FEATURE_UTILS_LOG_STRING_H_
#define _MTKCAM_FEATURE_UTILS_LOG_STRING_H_

#include <stdarg.h>
#include <stdio.h>
#include <string>

namespace NSCam {
namespace NSCamFeature {

template <unsigned N = 2048>
class FixLenLogString
{
public:
  FixLenLogString();
  FixLenLogString(size_t size);
  FixLenLogString(const FixLenLogString &src);
  FixLenLogString& operator=(const FixLenLogString &src);
  ~FixLenLogString();

  int append(const char *fmt, ...);
  void clear();

  const char* c_str() const;
  size_t length() const;
  size_t size() const;

private:
  void copy(const FixLenLogString &src);
  void allocate(size_t size);
  void deallocate();

private:
  char mStr[N] = { 0 };
  char *mPtr = mStr;
  size_t mSize = N;
  size_t mLen = 0;
  bool mFull = false;
};

typedef FixLenLogString<> LogString;

template <unsigned N>
FixLenLogString<N>::FixLenLogString()
{
}

template <unsigned N>
FixLenLogString<N>::FixLenLogString(size_t size)
{
  allocate(size);
}

template <unsigned N>
FixLenLogString<N>::FixLenLogString(const FixLenLogString &src)
{
  copy(src);
}

template <unsigned N>
FixLenLogString<N>& FixLenLogString<N>::operator=(const FixLenLogString<N> &src)
{
  copy(src);
  return *this;
}

template <unsigned N>
FixLenLogString<N>::~FixLenLogString()
{
  deallocate();
}

template <unsigned N>
int FixLenLogString<N>::append(const char *fmt, ...)
{
  int len = 0;
  if( mPtr && !mFull && fmt )
  {
    va_list args;
    va_start(args, fmt);
    len = vsnprintf(mPtr+mLen, mSize-mLen, fmt, args);
    va_end(args);
    if( len < 0 )
    {
      mFull = true;
      mLen = strlen(mPtr);
    }
    else
    {
      mLen += len;
      if( mLen >= mSize )
      {
        mFull = true;
        mLen = mSize-1;
      }
    }
  }
  return len;
}

template <unsigned N>
void FixLenLogString<N>::clear()
{
  mPtr[0] = '\0';
  mLen = 0;
  mFull = false;
}

template <unsigned N>
const char* FixLenLogString<N>::c_str() const
{
  return mPtr;
}

template <unsigned N>
size_t FixLenLogString<N>::length() const
{
  return mLen;
}

template <unsigned N>
size_t FixLenLogString<N>::size() const
{
  return mSize;
}

template <unsigned N>
void FixLenLogString<N>::copy(const FixLenLogString<N> &src)
{
  allocate(src.mSize);
  memcpy(mPtr, src.mPtr, std::max(src.mLen, 1));
  mLen = src.mLen;
  mFull = src.mFull;
}

template <unsigned N>
void FixLenLogString<N>::allocate(size_t size)
{
  if( size > mSize )
  {
    deallocate();
    mPtr = new char[size];
    mPtr[0] = '\0';
    mSize = size;
  }
}

template <unsigned N>
void FixLenLogString<N>::deallocate()
{
  if( mPtr != mStr )
  {
    delete [] mPtr;
    mPtr = mStr;
    mPtr[0] = '\0';
    mSize = N;
  }
}

} // Feature
} // NSCam

#endif // _MTKCAM_FEATURE_UTILS_LOG_STRING_H_
