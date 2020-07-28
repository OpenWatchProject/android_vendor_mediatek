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

#ifndef _MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_UTILS_IMGBUF_BASEIMAGEBUFFERHEAP_H_
#define _MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_UTILS_IMGBUF_BASEIMAGEBUFFERHEAP_H_
//
#include <utils/imgbuf/IImageBuffer.h>
//
#include <list>
#include <memory>
//
#include <utils/KeyedVector.h>
#include <utils/Mutex.h>
#include <utils/Printer.h>
#include <utils/RefBase.h>
#include <utils/String8.h>
#include <utils/Timers.h>
#include <utils/Vector.h>
//
#include "IIonDevice.h"
//


/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {
namespace NSImageBufferHeap {
using namespace android;


/**
 *
 */
struct  IRegistrationCookie
{
    virtual     ~IRegistrationCookie() {}
};


/**
 *
 */
class BaseImageBufferHeap;

/******************************************************************************
 *  Image Buffer Heap (Base).
 ******************************************************************************/
class BaseImageBufferHeap : public virtual IImageBufferHeap, public virtual RefBase
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IImageBufferHeap Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Reference Counting.
    virtual MVOID                   incStrong(MVOID const* id)          const   { RefBase::incStrong(id); }
    virtual MVOID                   decStrong(MVOID const* id)          const   { RefBase::decStrong(id); }
    virtual MINT32                  getStrongCount()                    const   { return RefBase::getStrongCount(); }

public:     ////                    Image Attributes.
    virtual MINT                    getImgFormat()                      const   { return mImgFormat; }
    virtual MSize const&            getImgSize()                        const   { return mImgSize; }
    virtual size_t                  getImgBitsPerPixel()                const;
    virtual size_t                  getPlaneBitsPerPixel(size_t index)  const;
    virtual size_t                  getPlaneCount()                     const   { return mPlaneCount; }
    virtual size_t                  getBitstreamSize()                  const   { return mBitstreamSize; }
    virtual MBOOL                   setBitstreamSize(size_t const bitstreamsize);
    virtual void                    setColorArrangement(MINT32 const colorArrangement);
    virtual MINT32                  getColorArrangement()               const   { return mColorArrangement; };
    virtual MBOOL                   updateImgInfo(
                                        MSize const& imgSize,
                                        MINT const imgFormat,
                                        size_t const sizeInBytes[3],
                                        size_t const rowStrideInBytes[3],
                                        size_t const bufPlaneSize
                                    );

public:     ////                    Buffer Attributes.
    virtual MBOOL                   getLogCond()                        const   { return mEnableLog; }
    virtual char const*             getMagicName()                      const   { return impGetMagicName(); }
    virtual void*                   getMagicInstance()                  const   { return impGetMagicInstance(); }
    virtual MINT32                  getHeapID(size_t index)             const;
    virtual size_t                  getHeapIDCount()                    const;
    virtual MINTPTR                 getBufPA(size_t index)              const;
    virtual MINTPTR                 getBufVA(size_t index)              const;
    virtual size_t                  getBufSizeInBytes(size_t index)     const;
    virtual size_t                  getBufStridesInBytes(size_t index)  const;

    /**
     * Buffer customized size, which means caller specified the buffer size he wants
     * of the given plane. Caller usually gives this value because he wants vertical
     * padding of image. This method returns 0 if caller didn't specify the customized
     * buffer size of the given plane.
     */
    virtual size_t                  getBufCustomSizeInBytes(size_t /*index*/) const  { return 0; }
    virtual void*                   getHWBuffer() const   { return NULL; }

public:     ////                    Buffer Operations.
    virtual MBOOL                   lockBuf(
                                        char const* szCallerName,
                                        MINT usage
                                    );
    virtual MBOOL                   unlockBuf(
                                        char const* szCallerName
                                    );
    virtual MBOOL                   syncCache(eCacheCtrl const ctrl);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IImageBuffer Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////

    /**
     * Create an IImageBuffer instance with its ROI equal to the image full
     * resolution of this heap.
     */
    virtual IImageBuffer*           createImageBuffer(ImgBufCreator* pCreator = NULL);

    /**
     * This call is legal only if the heap format is blob.
     *
     * From the given blob heap, create an IImageBuffer instance with a specified
     * offset and size, and its format equal to blob.
     */
    virtual IImageBuffer*           createImageBuffer_FromBlobHeap(
                                        size_t      offsetInBytes,
                                        size_t      sizeInBytes
                                    );

    /**
     * This call is legal only if the heap format is blob.
     *
     * From the given blob heap, create an IImageBuffer instance with a specified
     * offset, image format, image size in pixels, and buffer strides in pixels.
     */
    virtual IImageBuffer*           createImageBuffer_FromBlobHeap(
                                        size_t      offsetInBytes,
                                        MINT32      imgFormat,
                                        MSize const&imgSize,
                                        size_t const bufStridesInBytes[3]
                                    );

    /**
     * Create an IImageBuffer instance indicating the left-side or right-side
     * buffer within a side-by-side image.
     *
     * Left side if isRightSide = 0; otherwise right side.
     */
    virtual IImageBuffer*           createImageBuffer_SideBySide(
                                        MBOOL       isRightSide
                                    );

    /**
     * This call is legal only if the heap format is blob.
     *
     * From the given blob heap, create multiple IImageBuffer instances with a specified
     * ImageBufferInfo.
     *
     * Attention: Caller gets raw pointer and MUST free the pointer after using it.
     *
     */
    virtual std::vector<IImageBuffer*>
                                    createImageBuffers_FromBlobHeap(
                                        const ImageBufferInfo& info,
                                        const char* callerName,
                                        MBOOL dirty_content
                                    );


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Definitions.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////                    Heap Info.
                                    struct HeapInfo : public LightRefBase<HeapInfo>
                                    {
                                    MINT32      heapID;             // heap ID.
                                                //
                                                HeapInfo()
                                                    : heapID(-1)
                                                {
                                                }
                                    };
    typedef Vector<sp<HeapInfo> >   HeapInfoVect_t;

public:     ////                    Buffer Info.
                                    struct BufInfo : public LightRefBase<BufInfo>
                                    {
                                    MINTPTR     pa;                 // (plane) physical address
                                    MINTPTR     va;                 // (plane) virtual address
                                    size_t      stridesInBytes;     // (plane) strides in bytes
                                    size_t      sizeInBytes;        // (plane) size in bytes
                                                //
                                                BufInfo(
                                                    MINTPTR         _pa = 0,
                                                    MINTPTR         _va = 0,
                                                    size_t          _stridesInBytes = 0,
                                                    size_t          _sizeInBytes = 0
                                                )
                                                    : pa(_pa)
                                                    , va(_va)
                                                    , stridesInBytes(_stridesInBytes)
                                                    , sizeInBytes(_sizeInBytes)
                                                {
                                                }
                                    };
    typedef Vector<sp<BufInfo> >    BufInfoVect_t;

public:     ////                    Buffer Lock Info.
                                    struct BufLockInfo
                                    {
                                    String8         mUser;
                                    pid_t           mTid;
                                    struct timespec mTimestamp;
                                    };

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Template-Method Pattern. Subclass must implement them.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////
    /**
     * Return a pointer to a null-terminated string to indicate a magic name of
     * buffer type.
     */
    virtual char const*             impGetMagicName()                   const   = 0;

    /**
     * Return instance of subclass
     */
    virtual void*                   impGetMagicInstance()               const   = 0;

    /**
     * This call is valid after calling impLockBuf();
     * invalid after impUnlockBuf().
     */
    virtual HeapInfoVect_t const&   impGetHeapInfo()                    const   = 0;

    /**
     * onCreate() must be invoked by a subclass when its instance is created to
     * inform this base class of a creating event.
     * The call impInit(), implemented by a subclass, will be invoked by this
     * base class during onCreate() for initialization.
     * As to buffer information (i.e. BufInfoVect_t), buffer strides in pixels
     * and buffer size in bytes of each plane as well as the vector size MUST be
     * legal, at least, after impInit() return success.
     *
     * onLastStrongRef() will be invoked to indicate the last one reference to
     * this instance before it is freed.
     * The call impUninit(), implemented by a subclass, will be invoked by this
     * base class during onLastStrongRef() for uninitialization.
     */
    virtual MBOOL                   impInit(BufInfoVect_t const& rvBufInfo)     = 0;
    virtual MBOOL                   impUninit(BufInfoVect_t const& rvBufInfo)   = 0;
    virtual MBOOL                   impReconfig(BufInfoVect_t const& rvBufInfo) = 0;

public:     ////
    /**
     * As to buffer information (i.e. BufInfoVect_t), buffer strides in bytes
     * and buffer size in bytes of each plane as well as the vector size MUST be
     * always legal.
     *
     * After calling impLockBuf() successfully, the heap information from
     * impGetHeapInfo() must be legal; virtual address and physical address of
     * each plane must be legal if any SW usage and any HW usage are specified,
     * respectively.
     */
    virtual MBOOL                   impLockBuf(
                                        char const* szCallerName,
                                        MINT usage,
                                        BufInfoVect_t const& rvBufInfo
                                    )                                           = 0;
    virtual MBOOL                   impUnlockBuf(
                                        char const* szCallerName,
                                        MINT usage,
                                        BufInfoVect_t const& rvBufInfo
                                    )                                           = 0;

protected:  ////
    /**
     * The call impPrintLocked(), implemented by a subclass, will be invoked by
     * this base class during printLocked().
     */
    virtual android::String8        impPrintLocked() const { return android::String8(""); }

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Helper Functions.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////                    Helper Params.
                                    struct HelperParamMapPA
                                    {
                                    MINTPTR     phyAddr;        // physicall address; in/out
                                    MINTPTR     virAddr;        // virtual address
                                    MINT32      ionFd;          // ION file descriptor
                                    size_t      size;
                                    MINT32      security;
                                    MINT32      coherence;
                                    };

                                    struct HelperParamFlushCache
                                    {
                                    MINTPTR     phyAddr;        // physical address
                                    MINTPTR     virAddr;        // virtual address
                                    MINT32      ionFd;          // ION file descriptor
                                    size_t      size;
                                    };

protected:  ////                    Helper Functions.
    virtual MBOOL                   helpMapPhyAddr(
                                        char const* szCallerName,
                                        HelperParamMapPA& rParam
                                    );

    virtual MBOOL                   helpUnmapPhyAddr(
                                        char const* szCallerName,
                                        HelperParamMapPA const& rParam
                                    );

    virtual MBOOL                   helpFlushCache(
                                        eCacheCtrl const ctrl,
                                        HelperParamFlushCache const* paParam,
                                        size_t const num
                                    );

    virtual MBOOL                   helpCheckBufStrides(
                                        size_t const planeIndex,
                                        size_t const planeBufStridesInBytes
                                    ) const;

    virtual size_t                  helpQueryBufSizeInBytes(
                                        size_t const planeIndex,
                                        size_t const planeStridesInBytes
                                    ) const;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:    ////                    Called inside lock.
    virtual MBOOL                   initLocked();
    virtual MBOOL                   uninitLocked();
    virtual MBOOL                   lockBufLocked(char const* szCallerName, MINT usage);
    virtual MBOOL                   unlockBufLocked(char const* szCallerName);

public:     ////
            MVOID                   dumpInfoLocked();
            MVOID                   printLocked(android::Printer& printer, uint32_t indent);
            MVOID                   print(android::Printer& printer, uint32_t indent);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Instantiation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////                    Destructor/Constructors.
    /**
     * Disallowed to directly delete a raw pointer.
     */
    virtual                         ~BaseImageBufferHeap();
                                    BaseImageBufferHeap(char const* szCallerName);

protected:  ////                    Callback (LastStrongRef@RefBase)
    virtual void                    onFirstRef();
    virtual void                    onLastStrongRef(const void* id);

protected:  ////                    Callback (Create)
                                    struct OnCreate
                                    {
                                        MSize   imgSize;
                                        MINT    imgFormat = eImgFmt_BLOB;
                                        size_t  bitstreamSize = 0;
                                        SecType secType = SecType::mem_normal;
                                        MBOOL   enableLog = MTRUE;
                                        int     useSharedDeviceFd = -1;
                                    };
    virtual MBOOL                   onCreate(OnCreate const& arg);
    virtual MBOOL                   onCreate(
                                        MSize const& imgSize,
                                        MINT const imgFormat = eImgFmt_BLOB,
                                        size_t const bitstreamSize = 0,
                                        SecType const secType = SecType::mem_normal,
                                        MBOOL const enableLog = MTRUE
                                    );

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Data Members.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:    ////                    Heap Info.
    KeyedVector<MINT32, MINTPTR>    mBufMap;        //  keyedVector<memID, pa>
    mutable Mutex                   mInitMtx;
    mutable Mutex                   mLockMtx;
    MINT32 volatile                 mLockCount;
    MINT32                          mLockUsage;
    std::list<BufLockInfo>          mLockInfoList;
    BufInfoVect_t                   mvBufInfo;

private:    ////                    Image Attributes.
    MSize                           mImgSize;
    MINT                            mImgFormat;
    size_t                          mPlaneCount;
    size_t                          mBitstreamSize; // in bytes
    MINT32                          mColorArrangement;
    MBOOL                           mEnableLog;

protected:
    struct timespec                 mCreationTimestamp;
    nsecs_t                         mCreationTimeCost = 0;
    String8                         mCallerName;

    /**
     * -1: auto
     *  1: yes (use shared fd)
     *  0: no  (standalone fd)
     */
    MINT                            mUseSharedDeviceFd = -1;
    std::shared_ptr<IIonDevice>     mIonDevice = nullptr;
private:
    ImgBufCreator*                  mCreator;

};


/******************************************************************************
 *
 ******************************************************************************/
};  // namespace NSImageBufferHeap
};  // namespace NSCam
#endif  //_MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_UTILS_IMGBUF_BASEIMAGEBUFFERHEAP_H_

