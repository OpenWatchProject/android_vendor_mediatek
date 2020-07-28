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

#ifndef _MTK_HARDWARE_INCLUDE_MTKCAM_UTILS_RINGBUFFER_H_
#define _MTK_HARDWARE_INCLUDE_MTKCAM_UTILS_RINGBUFFER_H_
//
#include <deque>
#include <iterator>
#include <list>
#include <memory>
//
#include <cutils/compiler.h>


/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {
namespace Utils {


/******************************************************************************
 *
 ******************************************************************************/
template <class T, class ContainerT = std::list<T>>
class RingBuffer
{
public:
    // The type of elements stored in the RingBuffer.
    typedef typename ContainerT::value_type value_type;

    // The size type.
    typedef typename ContainerT::size_type size_type;

    // A type representing rvalue from param type.
    typedef value_type&& rvalue_type;

    // A iterator used to iterate through the RingBuffer.
    typedef typename ContainerT::iterator iterator;

    // A const iterator used to iterate through the RingBuffer.
    typedef typename ContainerT::const_iterator const_iterator;

public:

    /**
     *  Create an empty RingBuffer with the specified capacity.
     *
     *  @post capacity() == buffer_capacity && size() == 0
     *  @param capacity The maximum number of elements which can be stored in the RingBuffer.
     *  @note Complexity
     *        Constant.
     */
    explicit RingBuffer(size_type capacity)
      : mCapacity{capacity}, mSize{0}
    {
    }

    /**
     *  The destructor.
     *
     *  @note Iterator Invalidation
     *        Invalidates all iterators pointing to the RingBuffer
     *        (including iterators equal to end()).
     *  @note Complexity
     *        Constant (in the size of the RingBuffer) for scalar types;
     *        linear for other types.
     */
    ~RingBuffer() {
        mBuffer.clear();
        mCapacity = 0;
        mSize = 0;
    }

    /**
     *  Insert a new element at the end of the RingBuffer.
     *  The new element is initialized as a copy of item.
     *
     *  @post if capacity() > 0 then back() == item
     *        If the RingBuffer is full, the first element will be removed.
     *        If the capacity is 0, nothing will be inserted.
     *  @param item The element to be inserted.
     *  @note Iterator Invalidation
     *        Does not invalidate any iterators with the exception of iterators
     *        pointing to the overwritten element.
     *  @note Complexity
     *        Constant (in the size of the RingBuffer).
     */
    void push_back(const value_type& item) {

        if  ( capacity() == 0 ) {
            return;
        }

        if  ( full() ) {
            mBuffer.pop_front();
            mBuffer.push_back(item);
        }
        else {
            mBuffer.push_back(item);
            ++mSize;
        }

    }

    /**
     *  Insert a new element at the end of the RingBuffer.
     *  The new element is initialized as a copy of item.
     *
     *  @post if capacity() > 0 then back() == item
     *        If the RingBuffer is full, the first element will be removed.
     *        If the capacity is 0, nothing will be inserted.
     *  @param item The element to be inserted.
     *  @note Iterator Invalidation
     *        Does not invalidate any iterators with the exception of iterators
     *        pointing to the overwritten element.
     *  @note Complexity
     *        Constant (in the size of the RingBuffer).
     */
    void push_back(rvalue_type item) {

        if  ( capacity() == 0 ) {
            return;
        }

        if  ( full() ) {
            mBuffer.pop_front();
            mBuffer.push_back(std::forward<T>(item));
        }
        else {
            mBuffer.push_back(std::forward<T>(item));
            ++mSize;
        }

    }

    /**
     *  Get the iterator pointing to the beginning of RingBuffer.
     *
     *  @return An iterator pointing to the first element of the RingBuffer.
     *          If the RingBuffer is empty, it returns an iterator equal to
     *          the one returned by end().
     *  @note Iterator Invalidation
     *        Does not invalidate any iterators.
     *  @note Complexity
     *        Constant (in the size of the RingBuffer).
     */
    iterator begin() { return mBuffer.begin(); }

    /**
     *  Get the iterator pointing to the end of RingBuffer.
     *
     *  @return An iterator pointing to the element one behind the last element
     *          of the RingBuffer. If the RingBuffer is empty, it returns an
     *          iterator equal to the one returned by begin().
     *  @note Iterator Invalidation
     *        Does not invalidate any iterators.
     *  @note Complexity
     *        Constant (in the size of the RingBuffer).
     */
    iterator end() { return mBuffer.end(); }

    /**
     *  Get the const iterator pointing to the beginning of RingBuffer.
     *
     *  @return A const iterator pointing to the first element of the RingBuffer.
     *          If the RingBuffer is empty, it returns an iterator equal to
     *          the one returned by end() const.
     *  @note Iterator Invalidation
     *        Does not invalidate any iterators.
     *  @note Complexity
     *        Constant (in the size of the RingBuffer).
     */
    const_iterator begin() const { return mBuffer.begin(); }

    /**
     *  Get the const iterator pointing to the end of RingBuffer.
     *
     *  @return A const iterator pointing to the element one behind the last element
     *          of the RingBuffer. If the RingBuffer is empty, it returns an
     *          iterator equal to the one returned by begin() const.
     *  @note Iterator Invalidation
     *        Does not invalidate any iterators.
     *  @note Complexity
     *        Constant (in the size of the RingBuffer).
     */
    const_iterator end() const { return mBuffer.end(); }

    /**
     *  Get the number of elements currently stored in the RingBuffer.
     *
     *  @return The number of elements stored in the RingBuffer.
     *  @note Iterator Invalidation
     *        Does not invalidate any iterators.
     *  @note Complexity
     *        Constant (in the size of the RingBuffer).
     */
    size_type size() const { return mSize; }

    /**
     *  Get the capacity of the RingBuffer.
     *
     *  @return The maximum number of elements which can be stored in the RingBuffer.
     *  @note Iterator Invalidation
     *        Does not invalidate any iterators.
     *  @note Complexity
     *        Constant (in the size of the RingBuffer).
     */
    size_type capacity() const { return mCapacity; }

    /**
     *  Is the RingBuffer empty?
     *
     *  @return true if there are no elements stored in the RingBuffer;
     *          false otherwise.
     *  @note Iterator Invalidation
     *        Does not invalidate any iterators.
     *  @note Complexity
     *        Constant (in the size of the RingBuffer).
     */
    bool empty() const { return size() == 0; }

    /**
     *  Is the RingBuffer full?
     *
     *  @return true if the number of elements stored in the RingBuffer
     *          equals the capacity of the RingBuffer; false otherwise.
     *  @note Iterator Invalidation
     *        Does not invalidate any iterators.
     *  @note Complexity
     *        Constant (in the size of the RingBuffer).
     */
    bool full() const { return capacity() == size(); }

    /**
     *  Remove all stored elements from the RingBuffer.
     *
     *  @post size() == 0
     *  @note Iterator Invalidation
     *        Invalidates all iterators pointing to the RingBuffer
     *        (except iterators equal to end()).
     *  @note Complexity
     *        Constant (in the size of the RingBuffer) for scalar types;
     *        linear for other types.
     */
    void clear() {
        mSize = 0;
        mBuffer.clear();
    }

private:

    // The capacity of the RingBuffer.
    size_type       mCapacity;

    // The number of items currently stored in the RingBuffer.
    size_type       mSize;

    // The internal buffer used for storing elements in the RingBuffer.
    ContainerT      mBuffer;

};


/******************************************************************************
 *
 ******************************************************************************/
};  // namespace Utils
};  // namespace NSCam
#endif  //_MTK_HARDWARE_INCLUDE_MTKCAM_UTILS_RINGBUFFER_H_

