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

#ifndef _MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_UTILS_IMGBUF_IIONIMAGEBUFFERHEAP_H_
#define _MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_UTILS_IMGBUF_IIONIMAGEBUFFERHEAP_H_
//
#include "IImageBuffer.h"


/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {


/******************************************************************************
 *  Image Buffer Heap (ION).
 ******************************************************************************/
class IIonImageBufferHeap : public virtual IImageBufferHeap
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Params for Allocations.
    typedef IImageBufferAllocator::ImgParam AllocImgParam_t;

                                    struct AllocExtraParam
                                    {
                                    MINT32              nocache = 0;
                                    MINT32              security = 0;
                                    MINT32              coherence = 0;
                                    MBOOL               contiguousPlanes = MFALSE; // contiguous memory for multiple planes if MTRUE
                                    SecType             secType = SecType::mem_normal;
                                                        //
                                                        AllocExtraParam()
                                                        {
                                                        }
                                    };

public:     ////                    Creation.
    static  IIonImageBufferHeap*    create(
                                        char const* szCallerName,
                                        AllocImgParam_t const& rImgParam,
                                        AllocExtraParam const& rExtraParam = AllocExtraParam(),
                                        MBOOL const enableLog = MTRUE
                                    );

    /**
     * This call is legal only if the heap format is blob.
     *
     * From the given blob heap, create an IIonImageBufferHeap instance with a
     * specified offset, image format, image size in pixels, and buffer strides
     * in pixels.
     */
    virtual IIonImageBufferHeap*    createImageBufferHeap_FromBlobHeap(
                                        char const* szCallerName,
                                        AllocImgParam_t const& rImgParam,
                                        MBOOL const enableLog = MTRUE
                                    )                                      = 0;

protected:  ////                    Destructor/Constructors.
    /**
     * Disallowed to directly delete a raw pointer.
     */
    virtual                         ~IIonImageBufferHeap() {}

public:     ////                    Attributes.
    static  char const*             magicName() { return "IonHeap"; }

};


/******************************************************************************
 *  ION Image Buffer Heap Allocator Interface.
 ******************************************************************************/
class IIonImageBufferHeapAllocator
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////
    typedef IIonImageBufferHeap::AllocImgParam_t AllocImgParam_t;
    typedef IIonImageBufferHeap::AllocExtraParam AllocExtraParam_t;
                                    struct ReserveParam
                                    {
                                        size_t          numberOfBuffers = 0;
                                        size_t          bufferSizeInBytes = 0;
                                    };

public:     ////
    static IIonImageBufferHeapAllocator*
                                    getInstance();

    virtual                         ~IIonImageBufferHeapAllocator() = default;

    /**
     * The calls reserve() and unreserve() should be called in a pair.
     */
    virtual MBOOL                   unreserve(char const* szUserName)       = 0;
    virtual MBOOL                   reserve(
                                        char const* szUserName,
                                        ReserveParam const& param
                                    )                                       = 0;

    /**
     * This call is similar to reserve(), and however, unreserve() is unexpected
     * to be called, so that the reserved memory is recycled in some manners
     * depending on the underlying implementation.
     *
     * This call may fail since not of all implementations support the recycle
     * of the reserved memory.
     */
    virtual MBOOL                   reserveWithoutUndo(
                                        char const* szUserName,
                                        ReserveParam const& param
                                    )                                       = 0;

    /**
     * Notes:
     *  AllocImgParam_t::bufBoundaryInBytes[0] must be 0.
     *  AllocExtraParam_t::contiguousPlanes must be MTURE.
     */
    virtual IIonImageBufferHeap*    create(
                                        char const* szUserName,
                                        AllocImgParam_t const& rImgParam,
                                        AllocExtraParam_t const& rExtraParam = AllocExtraParam_t(),
                                        MBOOL const enableLog = MTRUE
                                    )                                       = 0;

};


/******************************************************************************
 *
 ******************************************************************************/
};  // namespace NSCam
#endif  //_MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_UTILS_IMGBUF_IIONIMAGEBUFFERHEAP_H_

