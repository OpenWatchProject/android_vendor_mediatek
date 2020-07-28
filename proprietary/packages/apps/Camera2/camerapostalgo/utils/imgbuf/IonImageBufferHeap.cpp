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
#if 0//defined(MTK_ION_SUPPORT)
#define LOG_TAG "Postalgo/IonHeap"
//
#include <utils/imgbuf/IIonImageBufferHeap.h>
//
#include <sys/prctl.h>
#include <sys/mman.h>
#include <linux/mman-proprietary.h>
#include <linux/ion_drv.h>
//
#include <atomic>
#include <functional>
#include <future>
#include <list>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>
//
#include <ion/ion.h>
#include <libion_mtk/include/ion.h>
//
#include <utils/Thread.h>
//
#include "BaseImageBufferHeap.h"
#include "IIonDevice.h"
#include "MyUtils.h"
//
using namespace android;
using namespace NSCam;
using namespace NSCam::Utils;

#define ALIGN(x, alignment) (((x) + ((alignment)-1)) & ~((alignment)-1))
#define ARRAY_SIZE(_Array)  (sizeof(_Array) / sizeof(_Array[0]))

/******************************************************************************
 *
 ******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_LOGV("[%s::%s] " fmt, getMagicName(), __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("[%s::%s] " fmt, getMagicName(), __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("[%s::%s] " fmt, getMagicName(), __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("[%s::%s] " fmt, getMagicName(), __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("[%s::%s] " fmt, getMagicName(), __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("[%s::%s] " fmt, getMagicName(), __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("[%s::%s] " fmt, getMagicName(), __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( (cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( (cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)


/******************************************************************************
 *  Image Buffer Heap (ION).
 ******************************************************************************/
namespace {
class BlobDerivedIonHeap;
class IonImageBufferHeap : public IIonImageBufferHeap
                         , public NSImageBufferHeap::BaseImageBufferHeap
{
    friend  class BlobDerivedIonHeap;
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Params for Allocations.
    typedef IIonImageBufferHeap::AllocImgParam_t    AllocImgParam_t;
    typedef IIonImageBufferHeap::AllocExtraParam    AllocExtraParam;

public:     ////                    Creation.
    static  IonImageBufferHeap*     create(
                                        char const* szCallerName,
                                        AllocImgParam_t const& rImgParam,
                                        AllocExtraParam const& rExtraParam = AllocExtraParam(),
                                        MBOOL const enableLog = MTRUE
                                    );

    virtual IIonImageBufferHeap*    createImageBufferHeap_FromBlobHeap(
                                        char const* szCallerName,
                                        AllocImgParam_t const& rImgParam,
                                        MBOOL const enableLog
                                    );

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  BaseImageBufferHeap Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////
    virtual size_t                  getBufCustomSizeInBytes(
                                        size_t index
                                    )  const override;

    virtual char const*             impGetMagicName()                   const   { return magicName(); }
    virtual void*                   impGetMagicInstance()               const   { return (void *)this; }
    virtual HeapInfoVect_t const&   impGetHeapInfo()                    const   { return *(HeapInfoVect_t*)(&mvHeapInfo); }

    virtual MBOOL                   impInit(BufInfoVect_t const& rvBufInfo);
    virtual MBOOL                   impUninit(BufInfoVect_t const& rvBufInfo);
    virtual MBOOL                   impReconfig(BufInfoVect_t const& rvBufInfo);

public:     ////
    virtual MBOOL                   impLockBuf(
                                        char const* szCallerName,
                                        MINT usage,
                                        BufInfoVect_t const& rvBufInfo
                                    );
    virtual MBOOL                   impUnlockBuf(
                                        char const* szCallerName,
                                        MINT usage,
                                        BufInfoVect_t const& rvBufInfo
                                    );

protected:  ////
    virtual android::String8        impPrintLocked() const;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Definitions.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////                    ION Info.
                                    struct MyIonInfo
                                    {
                                    ion_user_handle_t   ionHandle = 0;
                                    int                 ionFd = -1;
                                    size_t              sizeInBytes = 0;    //the real allocated size
                                    MINTPTR             pa = 0;
                                    MINTPTR             va = 0;
                                    int                 prot = 0;           //prot flag on mmap()
                                    };
    typedef std::vector<MyIonInfo>                      MyIonInfoVect_t;

protected:  ////                    Heap Info.
                                    struct MyHeapInfo : public HeapInfo
                                    {
                                    };
    typedef android::Vector<android::sp<MyHeapInfo> >   MyHeapInfoVect_t;

protected:  ////                    Buffer Info.
                                    struct MyBufInfo : public BufInfo
                                    {
                                    };
    typedef android::Vector<android::sp<MyBufInfo> >    MyBufInfoVect_t;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////
    virtual MBOOL                   isContiguousPlanes() const { return mContiguousPlanes; }
    virtual MBOOL                   doAllocIon(MyIonInfo& rIonInfo, size_t boundaryInBytesToAlloc);
    virtual MVOID                   doDeallocIon(MyIonInfo& rIonInfo);

    virtual MBOOL                   doMapPhyAddr(char const* szCallerName, MyIonInfo& rIonInfo);
    virtual MBOOL                   doUnmapPhyAddr(char const* szCallerName, MyIonInfo& rIonInfo);
    virtual MBOOL                   doFlushCache();
    virtual std::vector<size_t>     doCalculateOffsets(std::vector<size_t>const& vBoundaryInBytes, std::vector<size_t>const& vSizeInBytes) const;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Instantiation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////                    Destructor/Constructors.

    /**
     * Disallowed to directly delete a raw pointer.
     */
    virtual                         ~IonImageBufferHeap() {}
                                    IonImageBufferHeap(
                                        char const* szCallerName,
                                        AllocImgParam_t const& rImgParam,
                                        AllocExtraParam const& rExtraParam
                                    );

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Data Members.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////                    Info to Allocate.
    AllocExtraParam                 mExtraParam;
    MBOOL const                     mContiguousPlanes;  // contiguous memory for multiple planes if MTRUE
    size_t                          mBufStridesInBytesToAlloc[3];   // buffer strides in bytes.
    std::vector<size_t>             mBufBoundaryInBytesToAlloc;     // the address will be a multiple of boundary in bytes, which must be a power of two.
    size_t                          mBufCustomSizeInBytesToAlloc[3];// customized buffer size in bytes, it may be 0 as default
    size_t                          mBufReusableSizeInBytesToAlloc[3];// reusable buffer size in bytes, it may be 0 as default

protected:  ////                    Info of Allocated Result.
    nsecs_t                         mIonAllocTimeCost = 0;
    MyIonInfoVect_t                 mvIonInfo;
    MyHeapInfoVect_t                mvHeapInfo;     //
    MyBufInfoVect_t                 mvBufInfo;      //
    std::vector<size_t>             mvBufOffsetInBytes;
};


/******************************************************************************
 * Blob-heap derived ion image buffer heap
 ******************************************************************************/
class BlobDerivedIonHeap : public IonImageBufferHeap
{
public:
    sp<IonImageBufferHeap>          mBlobHeap;

                                    BlobDerivedIonHeap(
                                        char const* szCallerName,
                                        AllocImgParam_t const& rImgParam,
                                        AllocExtraParam const& rExtraParam,
                                        sp<IonImageBufferHeap>const& pBlobHeap
                                    )
                                        : IonImageBufferHeap(szCallerName, rImgParam, rExtraParam)
                                        , mBlobHeap(pBlobHeap)
                                    {}

protected:  ////
    virtual android::String8        impPrintLocked() const
                                    {
                                        return IonImageBufferHeap::impPrintLocked() + " (BLOB-derived)";
                                    }

protected:
    virtual MBOOL                   doAllocIon(MyIonInfo& rIonInfo, size_t /*boundaryInBytesToAlloc*/) override
                                    {
                                        auto const& rBlobIonInfo = mBlobHeap->mvIonInfo[0];

                                        if  (CC_UNLIKELY(rIonInfo.sizeInBytes > rBlobIonInfo.sizeInBytes)) {
                                            CAM_LOGE("[%s] BLOB heap size:%zu < requested heap size:%zu", __FUNCTION__, rBlobIonInfo.sizeInBytes, rIonInfo.sizeInBytes);
                                            return MFALSE;
                                        }

                                        rIonInfo.ionHandle = rBlobIonInfo.ionHandle;
                                        rIonInfo.ionFd     = rBlobIonInfo.ionFd;
                                        return MTRUE;
                                    }

    virtual MVOID                   doDeallocIon(MyIonInfo& rIonInfo) override
                                    {
                                        rIonInfo.ionFd = -1;
                                        rIonInfo.ionHandle = 0;
                                    }

};
};  // namespace


/******************************************************************************
 *
 ******************************************************************************/
IIonImageBufferHeap*
IIonImageBufferHeap::
create(
    char const* szCallerName,
    AllocImgParam_t const& rImgParam,
    AllocExtraParam const& rExtraParam,
    MBOOL const enableLog
)
{
    return IonImageBufferHeap::create(szCallerName, rImgParam, rExtraParam, enableLog);
}


/******************************************************************************
 *
 ******************************************************************************/
IonImageBufferHeap*
IonImageBufferHeap::
create(
    char const* szCallerName,
    AllocImgParam_t const& rImgParam,
    AllocExtraParam const& rExtraParam,
    MBOOL const enableLog
)
{
#if 0
    MUINT const planeCount = Format::queryPlaneCount(rImgParam.imgFormat);
    for (MUINT i = 0; i < planeCount; i++)
    {
        CAM_LOGW_IF(
            0!=(rImgParam.bufBoundaryInBytes[i]%L1_CACHE_BYTES),
            "BoundaryInBytes[%d]=%d is not a multiple of %d",
            i, rImgParam.bufBoundaryInBytes[i], L1_CACHE_BYTES
        );
    }
#endif
    //
    IonImageBufferHeap* pHeap = NULL;
    pHeap = new IonImageBufferHeap(szCallerName, rImgParam, rExtraParam);
    if  (CC_UNLIKELY( ! pHeap ))
    {
        CAM_LOGE("%s@ Fail to new", (szCallerName?szCallerName:"unknown"));
        return NULL;
    }
    //
    if  (CC_UNLIKELY( ! pHeap->onCreate(rImgParam.imgSize, rImgParam.imgFormat, rImgParam.bufSize, rExtraParam.secType, enableLog) ))
    {
        CAM_LOGE("%s@ onCreate", (szCallerName?szCallerName:"unknown"));
        delete pHeap;
        return NULL;
    }
    //
    return pHeap;
}


/******************************************************************************
 *
 ******************************************************************************/
IIonImageBufferHeap*
IonImageBufferHeap::
createImageBufferHeap_FromBlobHeap(
    char const* szCallerName,
    AllocImgParam_t const& rImgParam,
    MBOOL const enableLog
)
{
    if  (CC_UNLIKELY(eImgFmt_BLOB != getImgFormat())) {
        MY_LOGE("Not a blob heap");
        dumpInfoLocked();
        return nullptr;
    }

    if  (CC_UNLIKELY(
            (0 != rImgParam.bufBoundaryInBytes[0]) &&
            (0 == mBufBoundaryInBytesToAlloc[0] || mBufBoundaryInBytesToAlloc[0] > rImgParam.bufBoundaryInBytes[0])
        ))
    {
        MY_LOGE("%s@ The newly requested boundary:%zu couldn't be supported from the source BLOB heap boundary:%zu",
            (szCallerName?szCallerName:"unknown"), rImgParam.bufBoundaryInBytes[0], mBufBoundaryInBytesToAlloc[0]);
        dumpInfoLocked();
        return nullptr;
    }

    sp<IonImageBufferHeap> pBlobHeap = this;
    auto extraParam = mExtraParam;
    extraParam.contiguousPlanes = MTRUE; // must be contiguous planes since it's originally from a BLOB heap
    BlobDerivedIonHeap* pHeap = new BlobDerivedIonHeap(szCallerName, rImgParam, extraParam, pBlobHeap);
    if  (CC_UNLIKELY( ! pHeap )) {
        MY_LOGE("%s@ Fail to new", (szCallerName?szCallerName:"unknown"));
        return nullptr;
    }
    //
    if  (CC_UNLIKELY( ! pHeap->onCreate(rImgParam.imgSize, rImgParam.imgFormat, rImgParam.bufSize, mExtraParam.secType, enableLog) )) {
        MY_LOGE("%s@ onCreate", (szCallerName?szCallerName:"unknown"));
        delete pHeap;
        return nullptr;
    }
    //
    pBlobHeap = nullptr;
    return pHeap;
}


/******************************************************************************
 *
 ******************************************************************************/
IonImageBufferHeap::
IonImageBufferHeap(
    char const* szCallerName,
    AllocImgParam_t const& rImgParam,
    AllocExtraParam const& rExtraParam
)
    : BaseImageBufferHeap(szCallerName)
    //
    , mExtraParam(rExtraParam)
    , mContiguousPlanes(rExtraParam.contiguousPlanes)
    , mBufBoundaryInBytesToAlloc(
        rImgParam.bufBoundaryInBytes,
        rImgParam.bufBoundaryInBytes + ARRAY_SIZE(rImgParam.bufBoundaryInBytes))
    //
    , mvHeapInfo()
    , mvBufInfo()
    //
{
    ::memcpy(mBufStridesInBytesToAlloc, rImgParam.bufStridesInBytes, sizeof(mBufStridesInBytesToAlloc));
    ::memcpy(mBufCustomSizeInBytesToAlloc, rImgParam.bufCustomSizeInBytes, sizeof(mBufCustomSizeInBytesToAlloc));
    ::memcpy(mBufReusableSizeInBytesToAlloc, rImgParam.bufReusableSizeInBytes, sizeof(mBufReusableSizeInBytesToAlloc));
}


/******************************************************************************
 * Buffer minimum size in bytes; always legal.
 ******************************************************************************/
size_t
IonImageBufferHeap::
getBufCustomSizeInBytes(size_t index) const
{
    if  ( index >= getPlaneCount() )
    {
        MY_LOGI("Bad index(%zu) >= PlaneCount(%zu)", index, getPlaneCount());
        //dumpCallStack(__FUNCTION__);
        return  0;
    }
    //
    return  mBufCustomSizeInBytesToAlloc[index];
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
IonImageBufferHeap::
impInit(BufInfoVect_t const& rvBufInfo)
{
    //  rIonInfo.pa
    auto mapPA = [this](char const* szCallerName, MyIonInfo& rIonInfo) {
        if  (CC_UNLIKELY( ! doMapPhyAddr(szCallerName, rIonInfo) )) {
            CAM_LOGE("[mapPA] %s@ doMapPhyAddr", szCallerName);
            return false;
        }
        return true;
    };

    MBOOL ret = MFALSE;
    MBOOL bMapPhyAddr = ((getImgFormat() & 0xFF00) == eImgFmt_RAW_START
                      || (getImgFormat() & 0xFF00) == eImgFmt_STA_START) ;
    //
    //  Allocate memory and setup mBufHeapInfo & rBufHeapInfo.
    //  Allocated memory of each plane is not contiguous.
    std::vector<size_t> vSizeInBytesToAlloc(getPlaneCount(), 0);
    mvHeapInfo.setCapacity(getPlaneCount());
    mvBufInfo.setCapacity(getPlaneCount());
    for (size_t i = 0; i < getPlaneCount(); i++)
    {
        if  (CC_UNLIKELY( ! helpCheckBufStrides(i, mBufStridesInBytesToAlloc[i]) ))
        {
            goto lbExit;
        }
        //
        {
            sp<MyHeapInfo> pHeapInfo = new MyHeapInfo;
            mvHeapInfo.push_back(pHeapInfo);
            //
            sp<MyBufInfo> pBufInfo = new MyBufInfo;
            mvBufInfo.push_back(pBufInfo);
            pBufInfo->stridesInBytes = mBufStridesInBytesToAlloc[i];
            // if the customized buffer size is greater than the expected buffer size,
            // uses the given buffer size for vertical padding usage.
            size_t bufSizeInBytesToAlloc = (mBufCustomSizeInBytesToAlloc[i] > mBufReusableSizeInBytesToAlloc[i]) ?
                                            mBufCustomSizeInBytesToAlloc[i] : mBufReusableSizeInBytesToAlloc[i];
            pBufInfo->sizeInBytes = helpQueryBufSizeInBytes(i, mBufStridesInBytesToAlloc[i]) > bufSizeInBytesToAlloc ?
                                    helpQueryBufSizeInBytes(i, mBufStridesInBytesToAlloc[i]) : bufSizeInBytesToAlloc;
            if( bufSizeInBytesToAlloc != 0
                && helpQueryBufSizeInBytes(i, mBufStridesInBytesToAlloc[i]) != bufSizeInBytesToAlloc )
            {
                MY_LOGI("special case, calc size(%zu), min size(%zu), reusable size(%zu), final size(%zu)"
                        , helpQueryBufSizeInBytes(i, mBufStridesInBytesToAlloc[i])
                        , mBufCustomSizeInBytesToAlloc[i]
                        , mBufReusableSizeInBytesToAlloc[i]
                        , pBufInfo->sizeInBytes);
            }
            //
            //  setup return buffer information.
            rvBufInfo[i]->stridesInBytes = pBufInfo->stridesInBytes;
            rvBufInfo[i]->sizeInBytes = pBufInfo->sizeInBytes;

            //  the real size(s) to allocate memory.
            vSizeInBytesToAlloc[i] = pBufInfo->sizeInBytes;

            //allocate reusable buffer, resume the current data format's buffer size info
            if (mBufCustomSizeInBytesToAlloc[i] != mBufReusableSizeInBytesToAlloc[i]) {
                pBufInfo->sizeInBytes = helpQueryBufSizeInBytes(i, mBufStridesInBytesToAlloc[i]) > mBufCustomSizeInBytesToAlloc[i] ?
                                        helpQueryBufSizeInBytes(i, mBufStridesInBytesToAlloc[i]) : mBufCustomSizeInBytesToAlloc[i];
                MY_LOGD("resume pBufInfo: sizeInBytes(%zu --> %zu)",
                        rvBufInfo[i]->sizeInBytes, pBufInfo->sizeInBytes);
                rvBufInfo[i]->sizeInBytes = pBufInfo->sizeInBytes;
            }
        }
    }
    //
    mvBufOffsetInBytes = doCalculateOffsets(mBufBoundaryInBytesToAlloc, vSizeInBytesToAlloc);
    //
    // Allocate memory and setup mvIonInfo & mvHeapInfo.
    if  ( ! isContiguousPlanes() )
    {
        //  Usecases:
        //      default (for most cases)

        mvIonInfo.resize(getPlaneCount());
        for (size_t i = 0; i < getPlaneCount(); i++) {
            auto& rIonInfo = mvIonInfo[i];
            rIonInfo.sizeInBytes = vSizeInBytesToAlloc[i];
            if  (CC_UNLIKELY( ! doAllocIon(rIonInfo, mBufBoundaryInBytesToAlloc[i]) )) {
                MY_LOGE("doAllocIon");
                goto lbExit;
            }
            mvHeapInfo[i]->heapID = rIonInfo.ionFd;

            // map physical Address for raw buffer in creating period
            if  (bMapPhyAddr) {
                if  (CC_UNLIKELY( ! mapPA(mCallerName.c_str(), rIonInfo) )) {
                    goto lbExit;
                }
                mvBufInfo[i]->pa = rIonInfo.pa;
                rvBufInfo[i]->pa = rIonInfo.pa;
            }
        }
    }
    else
    {
        //  Usecases:
        //      Blob heap -> multi-plane (e.g. YV12) heap

        mvIonInfo.resize(1);
        auto& rIonInfo = mvIonInfo[0];
        rIonInfo.sizeInBytes = mvBufOffsetInBytes.back() + vSizeInBytesToAlloc.back(); // total size in bytes
        if  (CC_UNLIKELY( ! doAllocIon(rIonInfo, mBufBoundaryInBytesToAlloc[0]) )) {
            MY_LOGE("doAllocIon");
            goto lbExit;
        }
        for (size_t i = 0; i < getPlaneCount(); i++) {
            mvHeapInfo[i]->heapID = rIonInfo.ionFd;
        }

        // map physical Address for raw buffer in creating period
        if  (bMapPhyAddr) {
            if  (CC_UNLIKELY( ! mapPA(mCallerName.c_str(), rIonInfo) )) {
                goto lbExit;
            }
            MINTPTR const pa = rIonInfo.pa;
            for (size_t i = 0; i < rvBufInfo.size(); i++) {
                rvBufInfo[i]->pa = mvBufInfo[i]->pa = pa + mvBufOffsetInBytes[i];
            }
        }
    }
    //
    ret = MTRUE;
lbExit:
    if  (CC_UNLIKELY( ! ret ))
    {
        impUninit(rvBufInfo);
        mIonDevice.reset();
    }
    MY_LOGD_IF(1, "- ret:%d", ret);
    return  ret;
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
IonImageBufferHeap::
impReconfig(BufInfoVect_t const& rvBufInfo)
{
    String8 log;
    std::vector<size_t> vSizeInBytes(rvBufInfo.size(), 0);
    MY_LOGD("rvBufInfo.size:%zu,  mvBufInfo.size:%zu", rvBufInfo.size(), mvBufInfo.size());

    for (size_t i = 0; i < rvBufInfo.size(); ++i)
    {
        sp<BufInfo> pSrc = rvBufInfo[i];
        sp<MyBufInfo> pDst = mvBufInfo[i];

        pDst->stridesInBytes = pSrc->stridesInBytes;
        pDst->sizeInBytes = pSrc->sizeInBytes;
        mBufCustomSizeInBytesToAlloc[i] = pSrc->sizeInBytes;
        vSizeInBytes[i] = pSrc->sizeInBytes;
        //
        log += String8::format("[%zu] special case, min size(%zu), reusable size(%zu), final size(%zu), final stride(%zu)\n"
                , i
                , mBufCustomSizeInBytesToAlloc[i]
                , mBufReusableSizeInBytesToAlloc[i]
                , pDst->sizeInBytes
                , pDst->stridesInBytes);
    }
    mvBufOffsetInBytes = doCalculateOffsets(mBufBoundaryInBytesToAlloc, vSizeInBytes);
    log += String8::format("offset: ");
    for (auto const& offset : mvBufOffsetInBytes) {
        log += String8::format("%zu ", offset);
    }
    MY_LOGI("%s", log.c_str());
    return  MTRUE;
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
IonImageBufferHeap::
impUninit(BufInfoVect_t const& rvBufInfo)
{
    MY_LOGD("rvBufInfo.size:%zu,  mvBufInfo.size:%zu", rvBufInfo.size(), mvBufInfo.size());

    for (size_t i = 0; i < mvBufInfo.size(); i++) {
        auto pHeapInfo = mvHeapInfo[i];
        auto pBufInfo = mvBufInfo[i];
        //
        pHeapInfo->heapID = -1;
        pBufInfo->va = 0;
        pBufInfo->pa = 0;
    }

    // unlock buffer for current used planes.
    for (size_t i = 0; i < rvBufInfo.size(); i++) {
        auto pbufinfo = rvBufInfo[i];
        pbufinfo->va = 0;
        pbufinfo->pa = 0;
    }

    for (size_t i = 0; i < mvIonInfo.size(); i++) {
        auto& rIonInfo = mvIonInfo[i];
        if  ( rIonInfo.va ) {
            if ( mIonDevice ) {
                ::ion_munmap(mIonDevice->getDeviceFd(), (void *)rIonInfo.va, rIonInfo.sizeInBytes);
            }
            rIonInfo.va = 0;
            rIonInfo.prot = 0;
        }
        if( rIonInfo.pa ) {
            doUnmapPhyAddr(mCallerName.c_str(), rIonInfo);
            rIonInfo.pa = 0;
        }
        //
        doDeallocIon(rIonInfo);
    }
    //
    mIonDevice.reset();
    //
    MY_LOGD_IF(1, "-");
    return  MTRUE;
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
IonImageBufferHeap::
doAllocIon(MyIonInfo& rIonInfo, size_t boundaryInBytesToAlloc)
{
    nsecs_t const startTime = ::systemTime();

    int err = 0;
    int ion_prot_flags = mExtraParam.nocache ? 0 : (ION_FLAG_CACHED | ION_FLAG_CACHED_NEEDS_SYNC);
    //
    //  ion_alloc_camera: buf handle
    err = ::ion_alloc_camera(mIonDevice->getDeviceFd(), rIonInfo.sizeInBytes, boundaryInBytesToAlloc, ion_prot_flags, &rIonInfo.ionHandle);
    if  (CC_UNLIKELY( 0 != err ))
    {
        MY_LOGE("ion_alloc_camera returns %d - DevFD:%d sizeInBytes:%zu boundaryInBytesToAlloc:%zu prot:%x", err, mIonDevice->getDeviceFd(), rIonInfo.sizeInBytes, boundaryInBytesToAlloc, ion_prot_flags);
        goto lbExit;
    }
    //
    //  ion_share: buf handle -> buf fd
    err = ::ion_share(mIonDevice->getDeviceFd(), rIonInfo.ionHandle, &rIonInfo.ionFd);
    if  (CC_UNLIKELY( 0 != err || -1 == rIonInfo.ionFd ))
    {
        MY_LOGE("ion_share returns %d, BufFD:%d", err, rIonInfo.ionFd);
        goto lbExit;
    }
    //
    {
        // ion_mm_buf_debug_info_t::dbg_name[48] is made up of as below:
        //      [  size ]-[   format   ]-[      caller name    ]
        //      [ max 9 ]-[   max 14   ]-[        max 23       ]
        //  For example:
        //      5344x3008-YUY2-ZsdShot:Yuv
        //      1920x1080-FG_BAYER10-Hal:Image:Resiedraw
        std::string resolution = std::to_string(getImgSize().w) + "x" + std::to_string(getImgSize().h);
        std::string format = NSCam::Utils::Format::queryImageFormatName(getImgFormat());
        std::string dbgName = resolution.substr(0, 9) + "-" + format.substr(0, 14) + "-" + mCallerName.c_str();

        struct ion_mm_data data;
        data.mm_cmd = ION_MM_SET_DEBUG_INFO;
        data.buf_debug_info_param.handle = rIonInfo.ionHandle;
        ::strncpy(data.buf_debug_info_param.dbg_name, dbgName.c_str(), sizeof(ion_mm_buf_debug_info_t::dbg_name));
        data.buf_debug_info_param.dbg_name[sizeof(ion_mm_buf_debug_info_t::dbg_name)-1] = '\0';
        data.buf_debug_info_param.value1 = 0;
        data.buf_debug_info_param.value2 = 0;
        data.buf_debug_info_param.value3 = 0;
        data.buf_debug_info_param.value4 = 0;
        err = ::ion_custom_ioctl(mIonDevice->getDeviceFd(), ION_CMD_MULTIMEDIA, &data);
        if  (CC_UNLIKELY(0 != err)) {
            MY_LOGE("ion_custom_ioctl(ION_MM_SET_DEBUG_INFO) returns %d, BufFD:%d", err, rIonInfo.ionFd);
            goto lbExit;
        }
    }
    //
    //  ion_mmap: buf fd -> virtual address (NON-Cachable)
#if 0
    rIonInfo.va = (MINTPTR)::ion_mmap(
        mIonDevice->getDeviceFd(), NULL, rIonInfo.sizeInBytes,
        PROT_READ|PROT_WRITE|PROT_NOCACHE,
        MAP_SHARED, rIonInfo.ionFd, 0
    );
    if  ( 0 == rIonInfo.va || -1 == rIonInfo.va )
    {
        MY_LOGE(
            "ion_mmap returns %d - DevFD:%d BufFD:%d BufSize:%zu",
            rIonInfo.va, mIonDevice->getDeviceFd(), rIonInfo.ionFd, rIonInfo.sizeInBytes
        );
        goto lbExit;
    }
#endif
    //
    //
    mIonAllocTimeCost += (::systemTime() - startTime);
    return  MTRUE;
lbExit:
    return  MFALSE;
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
IonImageBufferHeap::
doDeallocIon(MyIonInfo& rIonInfo)
{
    //  ion_munmap: virtual address
#if 0
    if  ( 0 != rIonInfo.va )
    {
        ::ion_munmap(mIonDevice->getDeviceFd(), (void *)rIonInfo.va, rIonInfo.sizeInBytes);
        rIonInfo.va = 0;
    }
#endif
    //
    //  ion_share_close: buf fd
    if  (CC_LIKELY( 0 <= rIonInfo.ionFd ))
    {
        ::ion_share_close(mIonDevice->getDeviceFd(), rIonInfo.ionFd);
        rIonInfo.ionFd = -1;
    }
    //
    //  ion_free: buf handle
    if  (CC_LIKELY( 0 != rIonInfo.ionHandle ))
    {
        ::ion_free(mIonDevice->getDeviceFd(), rIonInfo.ionHandle);
        rIonInfo.ionHandle = 0;
    }
}


/******************************************************************************
 *
 ******************************************************************************/
/* Performance consideration for Lock/unLockBuf
 *  Keep va/pa in mvBufInfo to avoid map va/pa each time and
 *  the va/pa will only be unmapped when destroying the heap.
 */
/******************************************************************************
 *
 ******************************************************************************/
MBOOL
IonImageBufferHeap::
impLockBuf(
    char const* szCallerName,
    MINT usage,
    BufInfoVect_t const& rvBufInfo
)
{
    int prot_flag = 0;
    if  ( 0 != (usage & eBUFFER_USAGE_SW_READ_MASK) ) { prot_flag |= PROT_READ; }
    if  ( 0 != (usage & eBUFFER_USAGE_SW_WRITE_MASK) ){ prot_flag |= PROT_WRITE; }

    //  rIonInfo.va
    auto mapVA = [prot_flag](char const* szCallerName, MyIonInfo& rIonInfo, int const ionDeviceFd) {
        if  ( (rIonInfo.prot ^ prot_flag) & prot_flag ) {
            if  ( rIonInfo.va ) {
                ::ion_munmap(ionDeviceFd, (void *)rIonInfo.va, rIonInfo.sizeInBytes);
            }
            rIonInfo.va = (MUINTPTR)::ion_mmap(ionDeviceFd, NULL, rIonInfo.sizeInBytes, prot_flag, MAP_SHARED, rIonInfo.ionFd, 0);
            rIonInfo.prot = prot_flag;
        }
        if  (CC_UNLIKELY( 0 == rIonInfo.va || -1 == rIonInfo.va )) {
            CAM_LOGE(
                "[mapVA] %s@ ion_mmap returns %#" PRIxPTR " - DevFD:%d IonFD:%d sizeInBytes:%zu prot:(%x %x)",
                szCallerName, rIonInfo.va, ionDeviceFd, rIonInfo.ionFd, rIonInfo.sizeInBytes, rIonInfo.prot, prot_flag
            );
            return false;
        }
        return true;
    };

    //  rIonInfo.pa
    auto mapPA = [this](char const* szCallerName, MyIonInfo& rIonInfo) {
        if  (CC_UNLIKELY( 0 == rIonInfo.pa )) {
            if  (CC_UNLIKELY( ! doMapPhyAddr(szCallerName, rIonInfo) )) {
                CAM_LOGE("[mapPA] %s@ doMapPhyAddr", szCallerName);
                return false;
            }
        }
        return true;
    };

    MBOOL ret = MFALSE;
    //
    if  ( mvIonInfo.size() == rvBufInfo.size() )
    {
        //  Usecases:
        //      default (for most cases)

        for (size_t i = 0; i < rvBufInfo.size(); i++) {
            auto& rIonInfo = mvIonInfo[i];
            //
            //  SW Access.
            if  ( 0 != (usage & eBUFFER_USAGE_SW_MASK) )
            {
                if  (CC_UNLIKELY( ! mapVA(szCallerName, rIonInfo, mIonDevice->getDeviceFd()) )) {
                    goto lbExit;
                }
                mvBufInfo[i]->va = rIonInfo.va;
                rvBufInfo[i]->va = rIonInfo.va;
            }
            //
            //  HW Access.
            if  ( 0 != (usage & eBUFFER_USAGE_HW_MASK) )
            {
                if  (CC_UNLIKELY( ! mapPA(szCallerName, rIonInfo) )) {
                    goto lbExit;
                }
                mvBufInfo[i]->pa = rIonInfo.pa;
                rvBufInfo[i]->pa = rIonInfo.pa;
            }
        }
    }
    else
    if  ( mvIonInfo.size() == 1 )
    {
        //  Usecases:
        //      Blob heap -> multi-plane (e.g. YV12) heap

        auto& rIonInfo = mvIonInfo[0];
        //
        //  SW Access.
        if  ( 0 != (usage & eBUFFER_USAGE_SW_MASK) )
        {
            if  (CC_UNLIKELY( ! mapVA(szCallerName, rIonInfo, mIonDevice->getDeviceFd()) )) {
                goto lbExit;
            }
            //
            MINTPTR const va = rIonInfo.va;
            for (size_t i = 0; i < rvBufInfo.size(); i++) {
                rvBufInfo[i]->va = mvBufInfo[i]->va = va + mvBufOffsetInBytes[i];
            }
        }
        //
        //  HW Access.
        if  ( 0 != (usage & eBUFFER_USAGE_HW_MASK) )
        {
            if  (CC_UNLIKELY( ! mapPA(szCallerName, rIonInfo) )) {
                goto lbExit;
            }
            //
            MINTPTR const pa = rIonInfo.pa;
            for (size_t i = 0; i < rvBufInfo.size(); i++) {
                rvBufInfo[i]->pa = mvBufInfo[i]->pa = pa + mvBufOffsetInBytes[i];
            }
        }
    }
    else
    if  ( mvIonInfo.size() > rvBufInfo.size() )
    {
        //  Usecases: for reuse buffers cases

        for (size_t i = 0; i < rvBufInfo.size(); i++) {
            auto& rIonInfo = mvIonInfo[i];
            //
            //  SW Access.
            if  ( 0 != (usage & eBUFFER_USAGE_SW_MASK) )
            {
                if  (CC_UNLIKELY( ! mapVA(szCallerName, rIonInfo, mIonDevice->getDeviceFd()) )) {
                    goto lbExit;
                }
                mvBufInfo[i]->va = rIonInfo.va;
                rvBufInfo[i]->va = rIonInfo.va;
            }
            //
            //  HW Access.
            if  ( 0 != (usage & eBUFFER_USAGE_HW_MASK) )
            {
                if  (CC_UNLIKELY( ! mapPA(szCallerName, rIonInfo) )) {
                    goto lbExit;
                }
                mvBufInfo[i]->pa = rIonInfo.pa;
                rvBufInfo[i]->pa = rIonInfo.pa;
            }
        }
    }
    else
    {
        MY_LOGE("%s@ Unsupported #plane:%zu #rvBufInfo:%zu #mvIonInfo:%zu",
            szCallerName, getPlaneCount(), rvBufInfo.size(), mvIonInfo.size());
        return MFALSE;
    }
    //
    ret = MTRUE;
lbExit:
    if  ( ! ret )
    {
        impUnlockBuf(szCallerName, usage, rvBufInfo);
    }
    return ret;
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
IonImageBufferHeap::
impUnlockBuf(
    char const* szCallerName,
    MINT usage,
    BufInfoVect_t const& rvBufInfo
)
{
    for (size_t i = 0; i < rvBufInfo.size(); i++)
    {
        sp<MyHeapInfo> pHeapInfo = mvHeapInfo[i];
        sp<BufInfo> pBufInfo = rvBufInfo[i];
        //
        //  HW Access.
        if  ( 0 != (usage & eBUFFER_USAGE_HW_MASK) )
        {
            if  ( 0 != pBufInfo->pa ) {
                //doUnmapPhyAddr(szCallerName, *pHeapInfo, *pBufInfo);
                pBufInfo->pa = 0;
            }
            else {
                MY_LOGW("%s@ skip PA=0 at %zu-th plane", szCallerName, i);
            }
        }
        //
        //  SW Access.
        if  ( 0 != (usage & eBUFFER_USAGE_SW_MASK) )
        {
            if  ( 0 != pBufInfo->va ) {
                //::ion_munmap(mIonDevice->getDeviceFd(), (void *)pBufInfo->va, pBufInfo->sizeInBytes);
                pBufInfo->va = 0;
            }
            else {
                MY_LOGW("%s@ skip VA=0 at %zu-th plane", szCallerName, i);
            }
        }
    }
    //
#if 0
    //  SW Write + Cacheable Memory => Flush Cache.
    if  ( 0!=(usage & eBUFFER_USAGE_SW_WRITE_MASK) && 0==mExtraParam.nocache )
    {
        doFlushCache();
    }
#endif
    //
    return  MTRUE;
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
IonImageBufferHeap::
doMapPhyAddr(char const* szCallerName, MyIonInfo& rIonInfo)
{
    HelperParamMapPA param;
    param.phyAddr   = 0;
    param.virAddr   = rIonInfo.va;
    param.ionFd     = rIonInfo.ionFd;
    param.size      = rIonInfo.sizeInBytes;
    param.security  = mExtraParam.security;
    param.coherence = mExtraParam.coherence;
    if  ( ! helpMapPhyAddr(szCallerName, param) )
    {
        MY_LOGE("helpMapPhyAddr");
        return  MFALSE;
    }
    //
    rIonInfo.pa = param.phyAddr;
    //
    return  MTRUE;
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
IonImageBufferHeap::
doUnmapPhyAddr(char const* szCallerName, MyIonInfo& rIonInfo)
{
    HelperParamMapPA param;
    param.phyAddr   = rIonInfo.pa;
    param.virAddr   = rIonInfo.va;
    param.ionFd     = rIonInfo.ionFd;
    param.size      = rIonInfo.sizeInBytes;
    param.security  = mExtraParam.security;
    param.coherence = mExtraParam.coherence;
    if  ( ! helpUnmapPhyAddr(szCallerName, param) )
    {
        MY_LOGE("helpUnmapPhyAddr");
        return  MFALSE;
    }
    //
    rIonInfo.pa = 0;
    //
    return  MTRUE;
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
IonImageBufferHeap::
doFlushCache()
{
#if 0
    Vector<HelperParamFlushCache> vParam;
    vParam.insertAt(0, mvHeapInfo.size());
    HelperParamFlushCache*const aParam = vParam.editArray();
    for (MUINT i = 0; i < vParam.size(); i++)
    {
        aParam[i].phyAddr = mvBufInfo[i]->pa;
        aParam[i].virAddr = mvBufInfo[i]->va;
        aParam[i].ionFd   = mvHeapInfo[i]->heapID;
        aParam[i].size    = mvBufInfo[i]->sizeInBytes;
    }
    if  ( ! helpFlushCache(aParam, vParam.size()) )
    {
        MY_LOGE("helpFlushCache");
        return  MFALSE;
    }
#endif
    return  MTRUE;
}


/******************************************************************************
 *
 ******************************************************************************/
std::vector<size_t>
IonImageBufferHeap::
doCalculateOffsets(std::vector<size_t>const& vBoundaryInBytes, std::vector<size_t>const& vSizeInBytes) const
{
    std::vector<size_t> vOffset(getPlaneCount(), 0);// offset=0 for multiple ion heap
    if  ( isContiguousPlanes() ) {
        MINTPTR start = 0;
        for (size_t i = 0; i < vOffset.size(); i++) {
            start = (0==vBoundaryInBytes[i] ? start : ALIGN(start, vBoundaryInBytes[i]));
            vOffset[i] = start;
            start += vSizeInBytes[i];
            //MY_LOGW_IF(0 < i && 0 == vOffset[i], "%zu start:%zu offset:%zu size:%zu", i, start, vOffset[i], vSizeInBytes[i]);
        }
    }
    return vOffset;
}


/******************************************************************************
 *
 ******************************************************************************/
android::String8
IonImageBufferHeap::
impPrintLocked() const
{
    android::String8 s;
    //
    s += android::String8::format("ion{fd/handle/size/pa/va}:[");
    for (auto const& v : mvIonInfo) {
        s += android::String8::format(
            " (%d/%#x %zu %08" PRIxPTR " %08" PRIxPTR ")",
            v.ionFd, v.ionHandle, v.sizeInBytes, v.pa, v.va);
    }
    s += " ]";
    //
    s += android::String8::format(" offset:[");
    for (auto const& v : mvBufOffsetInBytes) {
        s += android::String8::format(" %zu", v);
    }
    s += " ]";
    //
    s += android::String8::format(" boundary:[");
    for (size_t i = 0; i < mvBufOffsetInBytes.size() && i < mBufBoundaryInBytesToAlloc.size(); i++) {
        s += android::String8::format(" %zu", mBufBoundaryInBytesToAlloc[i]);
    }
    s += " ]";
    //
    s += android::String8::format(" contiguousPlanes:%c", (isContiguousPlanes()?'Y':'N'));
    //
    s += android::String8::format(" ion-cost(us):%" PRId64 "", mIonAllocTimeCost/1000);
    //
    return s;
}


/******************************************************************************
 *  ION Image Buffer Heap Allocator Interface.
 ******************************************************************************/
namespace {
class FixedSizeAllocatorImpl : public IIonImageBufferHeapAllocator
                                        /*, public IDebuggee*/
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Definitions.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////

    enum class EState
    {
        UNKNOWN = 0,
        UNRESERVE,
        RESERVE,
    };

public:     ////
    class MyHeap;
    using MyHeapListT = std::list<android::sp<MyHeap>>;
    class MyHeap : public IonImageBufferHeap
    {
    public:
        mutable std::atomic_int32_t mStrong{0};

        static  MyHeap* create(
                            char const* name,
                            AllocImgParam_t const& rImgParam,
                            AllocExtraParam const& rExtraParam,
                            MBOOL const enableLog
                        )
                        {
                            CAM_TRACE_CALL();

                            MyHeap* pHeap = new MyHeap(name, rImgParam, rExtraParam);
                            if  ( ! pHeap ) {
                                CAM_LOGE("Fail to new MyHeap");
                                return nullptr;
                            }
                            //
                            if  ( ! pHeap->onCreate(rImgParam.imgSize, rImgParam.imgFormat, rImgParam.bufSize, rExtraParam.secType, enableLog) ) {
                                CAM_LOGE("onCreate");
                                delete pHeap;
                                return nullptr;
                            }
                            //
                            return pHeap;
                        }

                        template<typename... Args>
                        MyHeap(const Args&... args)
                            : IonImageBufferHeap(args...)
                        {}

        virtual MVOID   incStrong(MVOID const* id) const
                        {
                            mStrong.fetch_add(1, std::memory_order_relaxed);
                            IonImageBufferHeap::incStrong(id);
                        }

        virtual MVOID   decStrong(MVOID const* id) const
                        {
                            IonImageBufferHeap::decStrong(id);
                            const int32_t c = mStrong.fetch_sub(1, std::memory_order_release);
                            if (c == 2) {
                                std::atomic_thread_fence(std::memory_order_acquire);
                                FixedSizeAllocatorImpl::getInstance()->release(const_cast<MyHeap*>(this));
                            }
                        }
    };

    class HeapManager
    {
    protected:
        struct timespec             mTimestamp;
        std::string const           mName;
        ReserveParam const          mReserveParam;
        AllocImgParam_t const       mAllocImgParam;
        AllocExtraParam_t const     mAllocExtraParam;

        std::weak_ptr<HeapManager>  mWeak;

        std::future<void>           mReserveFuture;
        std::atomic_bool            isStartUnreserve{false};

        mutable std::mutex          mHeapListLock;
        MyHeapListT                 mFreedHeapList; //available
        MyHeapListT                 mAllocHeapList; //in-flight

    public:
                        HeapManager(char const* /*szUserName*/, ReserveParam const& param)
                            : mName{"FixedSizeAllocator"}
                            , mReserveParam(param)
                            , mAllocImgParam(param.bufferSizeInBytes, 0)
                            , mAllocExtraParam()
                        {
                            NSCam::Utils::LogTool::get()->getCurrentLogTime(&mTimestamp);
                        }

                        ~HeapManager()
                        {
                            CAM_LOGD("[%s] + mFreedHeapList.size:%zu, mAllocHeapList.size:%zu", __FUNCTION__, mFreedHeapList.size(), mAllocHeapList.size());
                            mFreedHeapList.clear();
                            mAllocHeapList.clear();
                            CAM_LOGD("[%s] -", __FUNCTION__);
                        }

        auto            setWeak(std::weak_ptr<HeapManager>const& w) { mWeak = w; }

        auto            getReserveParam() const -> ReserveParam const& { return mReserveParam; }

        auto            print(::android::Printer& printer) const -> void
                        {
                            size_t freedSize{0}, allocSize{0};
                            {
                                std::lock_guard<std::mutex> _l(mHeapListLock);
                                freedSize = mFreedHeapList.size();
                                allocSize = mAllocHeapList.size();
                            }
                            printer.printFormatLine(
                                "%s reserved:%zux%zu AllocExtraParam:(nocache,security,coherence)=(%d,%d,%d) "
                                "isStartUnreserve:%d mFreedHeapList:#%zu mAllocHeapList:#%zu",
                                NSCam::Utils::LogTool::get()->convertToFormattedLogTime(&mTimestamp).c_str(),
                                mReserveParam.numberOfBuffers, mReserveParam.bufferSizeInBytes,
                                mAllocExtraParam.nocache, mAllocExtraParam.security, mAllocExtraParam.coherence,
                                isStartUnreserve.load(), freedSize, allocSize
                            );
                        }

        auto            create(
                            char const* szUserName,
                            AllocImgParam_t const& rImgParam,
                            AllocExtraParam_t const& rExtraParam,
                            MBOOL const enableLog
                        ) -> IIonImageBufferHeap*
                        {
                            CAM_TRACE_CALL();
                            if  (CC_UNLIKELY(
                                    rExtraParam.nocache != mAllocExtraParam.nocache
                                ||  rExtraParam.security != mAllocExtraParam.security
                                ||  rExtraParam.coherence != mAllocExtraParam.coherence
                                //||  rExtraParam.contiguousPlanes != MTRUE
                                ))
                            {
                                CAM_LOGE(
                                    "[%s] %s: not allowed to allocate from the reserved memory with different extra params"
                                    " - nocache/security/coherence:reserved(%d/%d/%d)!=request(%d/%d/%d), request contiguousPlanes:%d!=1",
                                    __FUNCTION__, szUserName,
                                    mAllocExtraParam.nocache, mAllocExtraParam.security, mAllocExtraParam.coherence,
                                    rExtraParam.nocache, rExtraParam.security, rExtraParam.coherence, rExtraParam.contiguousPlanes
                                );
                                return nullptr;
                            }

                            std::lock_guard<std::mutex> _l(mHeapListLock);

                            if  ( mFreedHeapList.empty() ) {
                                CAM_LOGW("[%s] %s: has no free heap available - reserved %zux%zu",
                                    __FUNCTION__, szUserName, mReserveParam.numberOfBuffers, mReserveParam.bufferSizeInBytes);
                                return nullptr;
                            }

                            auto it = mFreedHeapList.begin();
                            mAllocHeapList.splice(mAllocHeapList.end(), mFreedHeapList, it);

                            IIonImageBufferHeap* pHeap = nullptr;
                            MyHeap* pMyHeap = (*it).get();
                            if  (CC_LIKELY(pMyHeap!=nullptr)) {
                                pHeap = pMyHeap->createImageBufferHeap_FromBlobHeap(
                                    szUserName, rImgParam, enableLog
                                );
                                #if 1
                                CAM_LOGI_IF(pMyHeap!=nullptr, "[%s] MyHeap:%p getStrongCount:%d mStrong:%d", __FUNCTION__, pMyHeap, pMyHeap->getStrongCount(), pMyHeap->mStrong.load());
                                CAM_LOGI_IF(pHeap!=nullptr, "[%s] pHeap:%p getStrongCount:%x", __FUNCTION__, pHeap, pHeap->getStrongCount());
                                #endif
                            }
                            return pHeap;
                        }

        auto            release(MyHeap* pMyHeap) -> MVOID
                        {
                            CAM_TRACE_CALL();
                            std::lock_guard<std::mutex> _l(mHeapListLock);
                            for (MyHeapListT::iterator it = mAllocHeapList.begin(); it != mAllocHeapList.end(); it++) {
                                if  ( (*it).get() == pMyHeap ) {
                                    mFreedHeapList.splice(mFreedHeapList.end(), mAllocHeapList, it);
                                    #if 1
                                    CAM_LOGI("[%s] hit! MyHeap:%p getStrongCount:%d mStrong:%d", __FUNCTION__, pMyHeap, pMyHeap->getStrongCount(), pMyHeap->mStrong.load());
                                    #endif
                                    return;
                                }
                            }
                            CAM_LOGW_IF(1, "[%s] not found! MyHeap:%p #mFreedHeapList:%zu, #mAllocHeapList:%zu", __FUNCTION__, pMyHeap, mFreedHeapList.size(), mAllocHeapList.size());
                        }

        auto            startReserve() -> MBOOL
                        {
                            mReserveFuture = std::async(std::launch::async, [this](std::weak_ptr<HeapManager> weak){
                                std::string const threadName{std::string{"RSV:"}+mName};
                                ::prctl(PR_SET_NAME, (unsigned long)threadName.c_str(), 0, 0, 0);
                                CAM_TRACE_CALL();
                                //With this shared ptr, we're sure all members are alive inside this thread.
                                std::shared_ptr<HeapManager> shared = weak.lock();
                                if  ( shared == nullptr ) {
                                    CAM_LOGE("[%s] Dead heap manager on reserving thread", threadName.c_str());
                                    return;
                                }

                                auto const reserveParam = mReserveParam;
                                CAM_LOGI_IF(1, "FixedSizeAllocator: reserving thread start... - reserved:%zux%zu",
                                    reserveParam.numberOfBuffers, reserveParam.bufferSizeInBytes);

                                bool toRelease = false;
                                for (size_t i = 0; i < reserveParam.numberOfBuffers; i++) {
                                    if  ( isStartUnreserve.load() ) {
                                        toRelease = true;
                                        break;
                                    }

                                    MyHeap* pHeap = MyHeap::create(mName.c_str(), mAllocImgParam, mAllocExtraParam, MFALSE/*enableLog*/);
                                    if  ( pHeap != nullptr ) {
                                        std::lock_guard<std::mutex> _l(mHeapListLock);
                                        mFreedHeapList.emplace_back(pHeap);
                                    }
                                    else {
                                        CAM_LOGE("[%s] %zu: Fail to reserve heaps", threadName.c_str(), i);
                                        toRelease = true;
                                        break;
                                    }
                                }

                                if  (toRelease) {
                                    CAM_LOGW("[%s] cancel reserving and start releasing heaps - isStartUnreserve:%d", threadName.c_str(), isStartUnreserve.load());
                                    doReleaseHeaps();
                                }

                                shared.reset();
                                CAM_LOGI_IF(1, "FixedSizeAllocator: reserving thread done - reserved:%zux%zu isStartUnreserve:%d",
                                    reserveParam.numberOfBuffers, reserveParam.bufferSizeInBytes, isStartUnreserve.load());
                            }, mWeak);
                            return mReserveFuture.valid() ? MTRUE : MFALSE;
                        }

        auto            startUnreserve(char const* /*szUserName*/) -> void
                        {
                            CAM_TRACE_CALL();

                            //wait for the exit of the reserve thread.
                            isStartUnreserve.store(true);
                            if  ( mReserveFuture.valid() ) {
                                mReserveFuture.wait();
                            }
                            //release all heaps on a thread.
                            std::thread t([this](std::weak_ptr<HeapManager> weak){
                                std::string const threadName{std::string{"URSV:"}+mName};
                                ::prctl(PR_SET_NAME, (unsigned long)threadName.c_str(), 0, 0, 0);
                                //With this shared ptr, we're sure all members are alive inside this thread.
                                std::shared_ptr<HeapManager> shared = weak.lock();
                                if  ( shared == nullptr ) {
                                    CAM_LOGW("[%s] Dead heap manager on unreserving thread", threadName.c_str());
                                    return;
                                }
                                CAM_LOGI_IF(1, "FixedSizeAllocator: unreserving thread start...");
                                doReleaseHeaps();
                                shared.reset();
                                CAM_LOGI_IF(1, "FixedSizeAllocator: unreserving thread done");
                            }, mWeak);
                            t.detach();
                        }

    protected:
        auto            doReleaseHeaps() -> void
                        {
                            CAM_TRACE_CALL();

                            //Don't clear mFreedHeapList & mAllocHeapList inside locking.
                            //For that purpose, move all elements from mFreedHeapList & mAllocHeapList
                            //to temp. lists, and then clear temp lists outside locking.
                            MyHeapListT temp1, temp2;
                            {
                                //Transfer all elements from mXXXHeapList to temp list.
                                //After that, mXXXHeapList is empty.

                                std::lock_guard<std::mutex> _l(mHeapListLock);

                                temp1.splice(temp1.end(), mFreedHeapList);
                                mFreedHeapList.clear();

                                temp2.splice(temp2.end(), mAllocHeapList);
                                mAllocHeapList.clear();
                            }
                            temp1.clear();
                            temp2.clear();
                        }
    };

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////                    State Machine
    std::mutex                      mStateLock;
    EState                          mState = EState::UNRESERVE;

protected:  ////                    Reserved Heaps.
    std::mutex                      mHeapManagerLock;
    std::shared_ptr<HeapManager>    mHeapManager;
    std::atomic_int32_t             mReserveRefCount{0};

public:
                    ~FixedSizeAllocatorImpl()
                    {
                    }

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////
    virtual auto    debuggeeName() const -> std::string { return "NSCam::IIonImageBufferHeapAllocator"; }
    virtual auto    debug(
                        android::Printer& printer __unused,
                        const std::vector<std::string>& options __unused
                    ) -> void
                    {
                        int32_t refcount{0};
                        std::shared_ptr<HeapManager> pHeapManager = nullptr;
                        EState state = EState::UNKNOWN;
                        {
                            std::lock_guard<std::mutex> _l(mStateLock);
                            state = mState;
                            std::lock_guard<std::mutex> _lhm(mHeapManagerLock);
                            pHeapManager = mHeapManager;
                            refcount = mReserveRefCount.load();
                        }

                        printer.printFormatLine("ReserveRefCount:%d State:%u HeapManager:%p", refcount, state, pHeapManager.get());
                        if ( pHeapManager != nullptr ) {
                            pHeapManager->print(printer);
                            pHeapManager.reset();
                        }
                    }

public:     ////    IIonImageBufferHeapAllocator Interface.

    static  auto    getInstance() -> FixedSizeAllocatorImpl*
                    {
                        static auto inst = std::make_shared<FixedSizeAllocatorImpl>();
                        return inst.get();
                    }

    virtual auto    unreserve(char const* szUserName) -> MBOOL
                    {
                        CAM_TRACE_CALL();

                        if  ( CC_UNLIKELY(!szUserName) ) {
                            CAM_LOGE("[%s] unknown name", __FUNCTION__);
                            return MFALSE;
                        }

                        std::shared_ptr<HeapManager> pHeapManager = nullptr;
                        EState state = EState::UNKNOWN;
                        {
                            std::lock_guard<std::mutex> _l(mStateLock);
                            state = mState;
                            CAM_LOGI("[%s] %s: try to unreserve - refcount:%d state:(%u)",
                                __FUNCTION__, szUserName, mReserveRefCount.load(), state);
                            switch (state)
                            {
                            case EState::RESERVE:{
                                std::lock_guard<std::mutex> _l(mHeapManagerLock);
                                auto refCount = mReserveRefCount.fetch_sub(1);
                                if (1 == refCount) {
                                    if (CC_LIKELY(mHeapManager != nullptr)) {
                                        //There's a deadlock if mHeapManager's dtor invokes heap's dtor inside locking.
                                        //Therefore, reset mHeapManager (and hence invoke heap's dtors) outside locking.
                                        pHeapManager = mHeapManager;
                                        mHeapManager->startUnreserve(szUserName);
                                        mHeapManager.reset();
                                    }
                                    mState = EState::UNRESERVE;
                                }
                                }break;

                            case EState::UNRESERVE:
                            default:
                                CAM_LOGD("[%s] %s@ state:(%u) mReserveRefCount:%d - do nothing",
                                    __FUNCTION__, szUserName, state, mReserveRefCount.load());
                                return MFALSE;
                                break;
                            }
                        }

                        if (pHeapManager != nullptr) {
                            CAM_LOGD_IF(1, "[%s] %s@ state:(%u) - reset heap manager", __FUNCTION__, szUserName, state);
                            pHeapManager.reset();
                        }
                        return MTRUE;
                    }

    virtual auto    reserve(
                        char const* szUserName,
                        ReserveParam const& param
                    ) -> MBOOL
                    {
                        CAM_TRACE_CALL();

                        if  ( CC_UNLIKELY(!szUserName) ) {
                            CAM_LOGE("[%s] unknown name", __FUNCTION__);
                            return MFALSE;
                        }

                        if  ( CC_UNLIKELY(0==param.numberOfBuffers || 0==param.bufferSizeInBytes) ) {
                            CAM_LOGE("[%s] %s: try to reserve bad size:%zux%zu", __FUNCTION__, szUserName, param.numberOfBuffers, param.bufferSizeInBytes);
                            return MFALSE;
                        }

                        EState state = EState::UNKNOWN;
                        {
                            std::lock_guard<std::mutex> _l(mStateLock);
                            state = mState;
                            std::lock_guard<std::mutex> _lhm(mHeapManagerLock);
                            CAM_LOGI("[%s] %s: try to reserve %zux%zu - refcount:%d state:(%u) mHeapManager:%p",
                                __FUNCTION__, szUserName,
                                param.numberOfBuffers, param.bufferSizeInBytes,
                                mReserveRefCount.load(), state, mHeapManager.get());
                            switch (state)
                            {
                            case EState::RESERVE:{
                                if (CC_UNLIKELY(mHeapManager == nullptr)) {
                                    CAM_LOGE("[%s] %s@ state:(%u) - Bad mHeapManager",
                                        __FUNCTION__, szUserName, state);
                                    return MFALSE;
                                }

                                if ( 0!=memcmp(&mHeapManager->getReserveParam(), &param, sizeof(ReserveParam)) ) {
                                    CAM_LOGE("[%s] %s@ state:(%u) - not allowed to reserve the memory with different settings again",
                                        __FUNCTION__, szUserName, state);
                                    return MFALSE;
                                }

                                mReserveRefCount.fetch_add(1);
                                }break;

                            case EState::UNRESERVE:{
                                mHeapManager = std::make_shared<HeapManager>(szUserName, param);
                                if  (CC_UNLIKELY(mHeapManager == nullptr)) {
                                    CAM_LOGE("[%s] %s@ state:(%u) - Bad mHeapManager",
                                        __FUNCTION__, szUserName, state);
                                    return MFALSE;
                                }
                                //
                                mHeapManager->setWeak(mHeapManager);
                                if ( ! mHeapManager->startReserve() ) {
                                    CAM_LOGE("[%s] %s@ state:(%u) - Fail on mHeapManager->startReserve",
                                        __FUNCTION__, szUserName, state);
                                    mHeapManager.reset();
                                    return MFALSE;
                                }

                                mReserveRefCount.fetch_add(1);
                                mState = EState::RESERVE;
                                }break;

                            default:
                                break;
                            }
                        }
                        return MTRUE;
                    }

    virtual auto    reserveWithoutUndo(
                        char const* szUserName,
                        ReserveParam const& param
                    ) -> MBOOL
                    {
                        CAM_LOGE("[%s] Not Supported!!! %s: try to reserve size:%zux%zu",
                            __FUNCTION__, (szUserName?szUserName:"unknown"),
                            param.numberOfBuffers, param.bufferSizeInBytes);
                        return MFALSE;
                    }

    virtual auto    create(
                        char const* szUserName,
                        AllocImgParam_t const& rImgParam,
                        AllocExtraParam_t const& rExtraParam,
                        MBOOL const enableLog
                    ) -> IIonImageBufferHeap*
                    {
                        CAM_TRACE_CALL();

                        if  (CC_UNLIKELY(!szUserName)) {
                            CAM_LOGE("[%s] unknown name", __FUNCTION__);
                            return nullptr;
                        }

                        if  (CC_UNLIKELY(0!=rImgParam.bufBoundaryInBytes[0])) {
                            CAM_LOGE("[%s] %s: bufBoundaryInBytes[0]:%zu!=0", __FUNCTION__, szUserName, rImgParam.bufBoundaryInBytes[0]);
                            return nullptr;
                        }

                        EState state = EState::UNKNOWN;
                        do {
                            std::lock_guard<std::mutex> _l(mStateLock);
                            state = mState;

                            if  (state == EState::RESERVE) {
                                //alloc from the reserved pool
                                std::lock_guard<std::mutex> _l(mHeapManagerLock);
                                if  ( CC_UNLIKELY(mHeapManager == nullptr) ) {
                                    CAM_LOGW("[%s] %s@ state:RESERVE - Bad mHeapManager", __FUNCTION__, szUserName);
                                    break;
                                }
                                auto pHeap = mHeapManager->create(szUserName, rImgParam, rExtraParam, enableLog);
                                if  ( CC_UNLIKELY(pHeap == nullptr) ) {
                                    CAM_LOGW("[%s] %s@ state:RESERVE - fail on mHeapManager->create", __FUNCTION__, szUserName);
                                    break;
                                }
                                return pHeap;
                            }
                        } while(0);

                        //Here we allocate the heap way as usual.
                        CAM_LOGD_IF(0, "[%s] %s@ state:(%u) - alloc normally", __FUNCTION__, szUserName, state);
                        return IIonImageBufferHeap::create(szUserName, rImgParam, rExtraParam, enableLog);
                    }

    auto            release(MyHeap* pHeap) -> MVOID
                    {
                        CAM_TRACE_CALL();

                        std::shared_ptr<HeapManager> pHeapManager = nullptr;
                        {
                            std::lock_guard<std::mutex> _l(mStateLock);
                            if  (mState == EState::RESERVE) {
                                std::lock_guard<std::mutex> _l(mHeapManagerLock);
                                pHeapManager = mHeapManager;
                            }
                        }

                        if  ( CC_UNLIKELY(pHeapManager == nullptr) ) {
                            CAM_LOGW("[%s] state:RESERVE - Bad mHeapManager for pHeap:%p", __FUNCTION__, pHeap);
                            return;
                        }
                        //release to the reserved pool
                        pHeapManager->release(pHeap);
                        pHeapManager.reset();
                    }

};
};  // namespace


/******************************************************************************
 *  ION Image Buffer Heap Allocator Interface.
 ******************************************************************************/
namespace {
class IonDriverAllocatorImpl : public IIonImageBufferHeapAllocator
                             /*, public IDebuggee*/
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Definitions.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////

    class ReserveThread : public android::Thread
    {
    public:
        using CommandT = std::function<void()>;

    protected:
        //Pending Command
        mutable ::android::Mutex    mLock1;
        ::android::Condition        mCond1;
        CommandT                    mCmd1 = nullptr;
        int                         mCmd1No = -1;       //command number
        int                         mCmd1Counter = 0;   //incremented when add().

        //In-flight Command
        mutable ::android::Mutex    mLock2;
        ::android::Condition        mCond2;
        CommandT                    mCmd2 = nullptr;
        int                         mCmd2No = -1;       //command number
        int                         mLastCmd2No = -1;   //the last command number
                                                        //with its associated command has been done.
    private:
        virtual auto    threadLoop() -> bool override
                        {
                            CAM_TRACE_CALL();

                            auto waitForPendingCommand = [this]()->bool{
                                Mutex::Autolock _l(mLock1);
                                while ( ! exitPending() && ! mCmd1 ) {
                                    int err = mCond1.wait(mLock1);
                                    CAM_LOGW_IF(OK != err,
                                        "[%s] exitPending:%d err:%d(%s)",
                                        __FUNCTION__, exitPending(), err, ::strerror(-err)
                                    );
                                }
                                return nullptr!=mCmd1;
                            };

                            if ( waitForPendingCommand() )
                            {
                                {
                                    Mutex::Autolock _l1(mLock1);
                                    Mutex::Autolock _l2(mLock2);

                                    //Transfers the pending command to the in-flight command.
                                    mCmd2 = mCmd1;
                                    mCmd1 = nullptr;
                                    mCmd2No = mCmd1No;
                                    mCmd1No = -1;
                                    mCond1.broadcast();
                                }
                                {
                                    Mutex::Autolock _l(mLock2);
                                    if  ( mCmd2 != nullptr ) {
                                        mCmd2();
                                        mCmd2 = nullptr;
                                        mLastCmd2No = mCmd2No;
                                        mCmd2No = -1;
                                        mCond2.broadcast();
                                    }
                                }
                            }
                            return true;
                        }

    public:
        auto            add(CommandT cmd) -> int
                        {
                            if  (CC_UNLIKELY(nullptr==cmd)) {
                                CAM_LOGE("[%s] Bad command", __FUNCTION__);
                                return -1;
                            }

                            Mutex::Autolock _l(mLock1);
                            mCmd1 = cmd;  //force to overwrite the old command.
                            mCmd1No = mCmd1Counter++;
                            mCond1.broadcast();
                            return mCmd1No;
                        }

        auto            waitDone(int cmdNo, nsecs_t const timeout) -> int
                        {
                            CAM_TRACE_CALL();

                            if  (CC_UNLIKELY(cmdNo<0)) {
                                CAM_LOGE("Bad commandNo:%d<0", cmdNo);
                                return NO_INIT;
                            }

                            nsecs_t const startTime = ::systemTime();
                            auto timeoutToWait = [=](){
                                nsecs_t const elapsedInterval = (::systemTime() - startTime);
                                nsecs_t const timeoutToWait = (timeout > elapsedInterval)
                                                            ? (timeout - elapsedInterval)
                                                            :   0
                                                            ;
                                return timeoutToWait;
                            };
                            //
                            int err = OK;
                            Mutex::Autolock _l(mLock2);
                            while ( ! exitPending() && cmdNo > mLastCmd2No ) {
                                err = mCond2.waitRelative(mLock2, timeoutToWait());
                                if  ( OK != err ) {
                                    break;
                                }
                            }
                            //
                            if  ( cmdNo > mLastCmd2No ) {
                                CAM_LOGW(
                                    "[%s] cmdNo:%d > mLastCmd2No:%d exitPending:%d "
                                    "timeout(ns):%" PRId64 " elapsed(ns):%" PRId64 " err:%d(%s)",
                                    __FUNCTION__, cmdNo, mLastCmd2No, exitPending(),
                                    timeout, (::systemTime() - startTime), err, ::strerror(-err)
                                );
                                if  ( exitPending() ) { return DEAD_OBJECT; }
                                return err;
                            }
                            return OK;
                        }
    };

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////
    android::sp<ReserveThread>  mReserveThread = nullptr;

    mutable ::android::Mutex    mLock;
    ReserveParam                mReserveParam;
    size_t                      mBoundaryInBytes = 0;
    unsigned int                mFlags = (ION_FLAG_CACHED | ION_FLAG_CACHED_NEEDS_SYNC);

public:
                    ~IonDriverAllocatorImpl()
                    {
                    }

                    IonDriverAllocatorImpl()
                    {
                        mReserveThread = new ReserveThread();
                        if  ( mReserveThread == nullptr ) {
                            CAM_LOGE("[%s] Bad mReserveThread", __FUNCTION__);
                        }
                        else {
                            int status = mReserveThread->run("Ion-Reserving");
                            if  ( OK != status ) {
                                CAM_LOGE("[%s] Fail to run the reserve thread - status:%d(%s)", __FUNCTION__, status, ::strerror(-status));
                                mReserveThread = nullptr;
                            }
                        }
                    }

protected:
    static auto     doQuerySizeByDriver() -> unsigned int
                    {
                        CAM_TRACE_CALL();

                        std::shared_ptr<IIonDevice> pIonDevice = IIonDeviceProvider::get()->makeIonDevice("IonDriverAllocator");
                        if  (CC_UNLIKELY(pIonDevice == nullptr)) {
                            CAM_LOGE("[%s] fail to makeIonDevice", __FUNCTION__);
                            return false;
                        }

                        unsigned int sizeInBytes = 0;
                        int err = ion_alloc_camera_pool(
                            pIonDevice->getDeviceFd(),
                            0/*len*/, 0/*align*/, 0/*flags*/, //don't care
                            &sizeInBytes,
                            ION_MM_QRY_CACHE_POOL
                        );
                        pIonDevice = nullptr;

                        if  (CC_UNLIKELY(err!=0)) {
                            CAM_LOGE("[%s] ION_MM_QRY_CACHE_POOL - err:%d(%s)", __FUNCTION__, err, ::strerror(-err));
                            return 0;
                        }
                        return sizeInBytes;
                    }

    static auto     doReserveByDriver(size_t len, size_t align, unsigned int flags) -> bool
                    {
                        CAM_TRACE_CALL();

                        std::shared_ptr<IIonDevice> pIonDevice = IIonDeviceProvider::get()->makeIonDevice("IonDriverAllocator");
                        if  (CC_UNLIKELY(pIonDevice == nullptr)) {
                            CAM_LOGE("[%s] fail to makeIonDevice", __FUNCTION__);
                            return false;
                        }

                        nsecs_t const startTime = ::systemTime();
                        int err = ion_alloc_camera_pool(
                            pIonDevice->getDeviceFd(), len, align, flags,
                            /*unsigned int *ret*/nullptr,
                            ION_MM_ACQ_CACHE_POOL
                        );
                        nsecs_t const duration = (::systemTime() - startTime);
                        pIonDevice = nullptr;

                        if  (CC_UNLIKELY(err!=0)) {
                            CAM_LOGE("[%s] ION_MM_ACQ_CACHE_POOL - err:%d(%s)", __FUNCTION__, err, ::strerror(-err));
                            return false;
                        }
                        else {
                            CAM_LOGI_IF(duration>=1000000, "[%s] ion_alloc_camera_pool - duration(ns):%" PRId64 " len:%zu align:%zu flags:%x", __FUNCTION__, duration, len, align, flags);
                        }
                        return true;
                    }

    auto            doReserve(bool waitDone, size_t len, size_t align, unsigned int flags) -> MBOOL
                    {
                        CAM_TRACE_CALL();

                        if  (CC_UNLIKELY(mReserveThread == nullptr)) {
                            CAM_LOGE("[%s] Bad mReserveThread", __FUNCTION__);
                            return MFALSE;
                        }

                        nsecs_t const startTime = ::systemTime();
                        auto cmdNo = mReserveThread->add(std::bind(doReserveByDriver, len, align, flags));
                        MBOOL ret = MTRUE;
                        if  ( ! waitDone ) {
                            ret = (cmdNo >= 0);
                        }
                        else {
                            int err = mReserveThread->waitDone(cmdNo, 1000000000/*1sec*/);
                            if  (CC_UNLIKELY(OK!=err)) {
                                CAM_LOGE("[%s] waitDone fail - err:%d(%s)", __FUNCTION__, err, ::strerror(-err));
                                ret = MFALSE;
                            }
                        }
                        nsecs_t const duration = (::systemTime() - startTime);

                        CAM_LOGI("[%s] duration(ns):%" PRId64 " cmdNo:%d waitDone:%d ret:%d len:%zu align:%zu flags:%x",
                            __FUNCTION__, duration, cmdNo, waitDone, ret, len, align, flags);
                        return ret;
                    }

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////
    virtual auto    debuggeeName() const -> std::string { return "NSCam::IIonImageBufferHeapAllocator"; }
    virtual auto    debug(
                        android::Printer& printer __unused,
                        const std::vector<std::string>& options __unused
                    ) -> void
                    {
                        ReserveParam reserveparam;
                        {
                            Mutex::Autolock _l(mLock);
                            reserveparam = mReserveParam;
                        }
                        printer.printFormatLine(
                            "flags:%x reserved-sizes(bytes):[ request:%zux%zu=%zu driver:%d ]",
                            mFlags,
                            reserveparam.numberOfBuffers, reserveparam.bufferSizeInBytes,
                            reserveparam.numberOfBuffers*reserveparam.bufferSizeInBytes,
                            doQuerySizeByDriver()
                        );
                    }

public:     ////    IIonImageBufferHeapAllocator Interface.

    static  auto    getInstance() -> IonDriverAllocatorImpl*
                    {
                        static auto pIonDeviceProvider __unused = IIonDeviceProvider::get();
                        static auto inst = std::make_shared<IonDriverAllocatorImpl>();
                        return inst.get();
                    }

    virtual auto    unreserve(char const* szUserName) -> MBOOL
                    {
                        CAM_TRACE_CALL();

                        if  ( CC_UNLIKELY(!szUserName) ) {
                            CAM_LOGE("[%s] unknown name", __FUNCTION__);
                            return MFALSE;
                        }

                        CAM_LOGI("[%s] %s: try to unreserve %zux%zu",
                            __FUNCTION__, szUserName,
                            mReserveParam.numberOfBuffers, mReserveParam.bufferSizeInBytes);
                        {
                            Mutex::Autolock _l(mLock);
                            ::memset(&mReserveParam, 0, sizeof(mReserveParam));
                        }
                        return doReserve(false, 0, mBoundaryInBytes, mFlags);
                    }

    virtual auto    reserve(
                        char const* szUserName,
                        ReserveParam const& param
                    ) -> MBOOL
                    {
                        CAM_TRACE_CALL();

                        if  ( CC_UNLIKELY(!szUserName) ) {
                            CAM_LOGE("[%s] unknown name", __FUNCTION__);
                            return MFALSE;
                        }

                        if  ( CC_UNLIKELY(0==param.numberOfBuffers || 0==param.bufferSizeInBytes) ) {
                            CAM_LOGE("[%s] %s: try to reserve bad size:%zux%zu", __FUNCTION__, szUserName, param.numberOfBuffers, param.bufferSizeInBytes);
                            return MFALSE;
                        }

                        CAM_LOGI("[%s] %s: try to reserve %zux%zu -> %zux%zu",
                            __FUNCTION__, szUserName,
                            mReserveParam.numberOfBuffers, mReserveParam.bufferSizeInBytes,
                            param.numberOfBuffers, param.bufferSizeInBytes);
                        {
                            Mutex::Autolock _l(mLock);
                            mReserveParam = param;
                        }
                        return doReserve(true, (param.numberOfBuffers*param.bufferSizeInBytes), mBoundaryInBytes, mFlags);
                    }

    virtual auto    reserveWithoutUndo(
                        char const* szUserName,
                        ReserveParam const& param
                    ) -> MBOOL
                    {
                        CAM_TRACE_CALL();

                        if  ( CC_UNLIKELY(!szUserName) ) {
                            CAM_LOGE("[%s] unknown name", __FUNCTION__);
                            return MFALSE;
                        }

                        if  ( CC_UNLIKELY(0==param.numberOfBuffers || 0==param.bufferSizeInBytes) ) {
                            CAM_LOGE("[%s] %s: try to reserve bad size:%zux%zu", __FUNCTION__, szUserName, param.numberOfBuffers, param.bufferSizeInBytes);
                            return MFALSE;
                        }

                        nsecs_t const startTime = ::systemTime();
                        auto const align = mBoundaryInBytes;
                        auto const flags = mFlags;
                        auto ret = doReserveByDriver((param.numberOfBuffers*param.bufferSizeInBytes), align, flags);
                        nsecs_t const duration = (::systemTime() - startTime);
                        CAM_LOGI("[%s] %s: try to reserve %zux%zu align:%zu flags:%x - duration(ns):%" PRId64 " ret:%d",
                            __FUNCTION__, szUserName,
                            param.numberOfBuffers, param.bufferSizeInBytes,
                            align, flags, duration, ret);
                        return ret ? MTRUE : MFALSE;
                    }

    virtual auto    create(
                        char const* szUserName,
                        AllocImgParam_t const& rImgParam,
                        AllocExtraParam_t const& rExtraParam,
                        MBOOL const enableLog
                    ) -> IIonImageBufferHeap*
                    {
                        CAM_TRACE_CALL();

                        if  (CC_UNLIKELY(!szUserName)) {
                            CAM_LOGE("[%s] unknown name", __FUNCTION__);
                            return nullptr;
                        }

                        if  (CC_UNLIKELY(0!=rImgParam.bufBoundaryInBytes[0])) {
                            CAM_LOGE("[%s] %s: bufBoundaryInBytes[0]:%zu!=0", __FUNCTION__, szUserName, rImgParam.bufBoundaryInBytes[0]);
                            return nullptr;
                        }

                        nsecs_t const startTime = ::systemTime();
                        IIonImageBufferHeap* p = IIonImageBufferHeap::create(szUserName, rImgParam, rExtraParam, enableLog);
                        nsecs_t const duration = (::systemTime() - startTime);
                        CAM_LOGD_IF(0, "[%s] %s: IIonImageBufferHeap::create - duration(ns):%" PRId64 "", __FUNCTION__, szUserName, duration);
                        return p;
                    }

};
};  // namespace


/******************************************************************************
 *
 ******************************************************************************/
IIonImageBufferHeapAllocator*
IIonImageBufferHeapAllocator::getInstance()
{
#if 0
    return FixedSizeAllocatorImpl::getInstance();
#else
    return IonDriverAllocatorImpl::getInstance();
#endif
}


/******************************************************************************
 *
 ******************************************************************************/
#endif  //MTK_ION_SUPPORT

