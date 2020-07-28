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

#ifndef _MTK_CAMERA_FEATURE_PIPE_CORE_SMART_BUFFER_H_
#define _MTK_CAMERA_FEATURE_PIPE_CORE_SMART_BUFFER_H_

#include <utils/RefBase.h>
#include <utils/Mutex.h>

#define COMPARE(op)                                   \
inline bool operator op (const sb<T> &o) const {      \
  return mPtr op o.mPtr;                              \
}                                                     \
inline bool operator op (const T *o) const {          \
  return mPtr op o;                                   \
}                                                     \
template <typename U>                                 \
inline bool operator op (const sb<U> &o) const {      \
  return mPtr op o.mPtr;                              \
}                                                     \
template <typename U>                                 \
inline bool operator op (const U *o) const {          \
  return mPtr op o;                                   \
}

namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {

template <typename T>
class sb
{
public:
  sb() : mPtr(0) {}

  sb(T* other);
  sb(const sb<T>& other);
  sb(const android::sp<T>& other);
  ~sb();

  sb& operator = (T* other);
  sb& operator = (const sb<T>& other);
  sb& operator = (const android::sp<T>& other);

  inline  T&      operator* () const  { return *mPtr; }
  inline  T*      operator-> () const { return mPtr.get();  }
  inline  T*      get() const         { return mPtr.get(); }

  COMPARE(==);
  COMPARE(!=);
  COMPARE(>);
  COMPARE(<);
  COMPARE(<=);
  COMPARE(>=);

private:
  void inc(const android::sp<T> &ptr);
  void dec(const android::sp<T> &ptr);
  android::sp<T> mPtr;
};

template<typename T>
void sb<T>::inc(const android::sp<T> &ptr)
{
  if( ptr != NULL )
  {
    ptr->incSbCount();
  }
}

template<typename T>
void sb<T>::dec(const android::sp<T> &ptr)
{
  if( ptr != NULL )
  {
    ptr->decSbCount();
  }
}

template<typename T>
sb<T>::sb(T* other)
: mPtr(other)
{
  inc(other);
}

template<typename T>
sb<T>::sb(const sb<T>& other)
: mPtr(other.mPtr)
{
  inc(mPtr);
}

template<typename T>
sb<T>::sb(const android::sp<T>& other)
: mPtr(other)
{
  inc(mPtr);
}

template<typename T>
sb<T>::~sb()
{
  dec(mPtr);
}

template<typename T>
sb<T>& sb<T>::operator = (const sb<T>& other)
{
  android::sp<T> otherPtr(other.mPtr);
  inc(otherPtr);
  dec(mPtr);
  mPtr = otherPtr;
  return *this;
}

template<typename T>
sb<T>& sb<T>::operator = (const android::sp<T>& other)
{
  inc(other);
  dec(mPtr);
  mPtr = other;
  return *this;
}

template<typename T>
sb<T>& sb<T>::operator = (T* other)
{
  inc(other);
  dec(mPtr);
  mPtr = other;
  return *this;
}

#undef COMPARE

} // namespace NSFeaturePipe
} // namespace NSCamFeature
} // namespace NSCam

#endif  // _MTK_CAMERA_FEATURE_PIPE_CORE_SMART_BUFFER_H_
