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
#if defined(MTK_ION_SUPPORT)
#define LOG_TAG "MtkCam/SecHeap"
//
#include <mtkcam/utils/imgbuf/ISecureImageBufferHeap.h>
//
#include <sys/prctl.h>
#include <sys/mman.h>
#include <linux/mman-proprietary.h>
#include <linux/mtk_ion.h>
#include <linux/ion_drv.h>
//
#include <atomic>
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
#include "BaseImageBufferHeap.h"
#include "MyUtils.h"
#include <mtkcam/utils/debug/debug.h>
#include <mtkcam/utils/std/ULog.h>

CAM_ULOG_DECLARE_MODULE_ID(MOD_IMAGE_BUFFER);
//
using namespace android;
using namespace NSCam;
using namespace NSCam::Utils;

#define ALIGN(x, alignment) (((x) + ((alignment)-1)) & ~((alignment)-1))
#define ARRAY_SIZE(_Array)  (sizeof(_Array) / sizeof(_Array[0]))

/******************************************************************************
 *
 ******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_ULOGMV("[%s::%s] " fmt, getMagicName(), __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_ULOGMD("[%s::%s] " fmt, getMagicName(), __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_ULOGMI("[%s::%s] " fmt, getMagicName(), __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_ULOGMW("[%s::%s] " fmt, getMagicName(), __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_ULOGME("[%s::%s] " fmt, getMagicName(), __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_ULOGM_ASSERT(0, "[%s::%s] " fmt, getMagicName(), __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_ULOGM_FATAL("[%s::%s] " fmt, getMagicName(), __FUNCTION__, ##arg)
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
class SecureBlobDerivedIonHeap;
class SecureImageBufferHeap : public ISecureImageBufferHeap
                         , public NSImageBufferHeap::BaseImageBufferHeap
{
    friend  class SecureBlobDerivedIonHeap;
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Params for Allocations.
    typedef ISecureImageBufferHeap::AllocImgParam_t    AllocImgParam_t;
    typedef ISecureImageBufferHeap::AllocExtraParam    AllocExtraParam;

public:     ////                    Creation.
    static  SecureImageBufferHeap*     create(
                                        char const* szCallerName,
                                        AllocImgParam_t const& rImgParam,
                                        AllocExtraParam const& rExtraParam = AllocExtraParam(),
                                        MBOOL const enableLog = MTRUE
                                    );

    virtual ISecureImageBufferHeap*    createImageBufferHeap_FromBlobHeap(
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
    virtual MBOOL                   doAllocIon(MyIonInfo& rIonInfo, size_t boundaryInBytesToAlloc, SecType secType);
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
    virtual                         ~SecureImageBufferHeap() {}
                                    SecureImageBufferHeap(
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
    MyIonInfoVect_t                 mvIonInfo;
    MyHeapInfoVect_t                mvHeapInfo;     //
    MyBufInfoVect_t                 mvBufInfo;      //
    std::vector<size_t>             mvBufOffsetInBytes;
};


/******************************************************************************
 * Blob-heap derived ion image buffer heap
 ******************************************************************************/
class SecureBlobDerivedIonHeap : public SecureImageBufferHeap
{
public:
    sp<SecureImageBufferHeap>          mBlobHeap;

                                    SecureBlobDerivedIonHeap(
                                        char const* szCallerName,
                                        AllocImgParam_t const& rImgParam,
                                        AllocExtraParam const& rExtraParam,
                                        sp<SecureImageBufferHeap>const& pBlobHeap
                                    )
                                        : SecureImageBufferHeap(szCallerName, rImgParam, rExtraParam)
                                        , mBlobHeap(pBlobHeap)
                                    {}

protected:  ////
    virtual android::String8        impPrintLocked() const
                                    {
                                        return SecureImageBufferHeap::impPrintLocked() + " (BLOB-derived)";
                                    }

protected:
    virtual MBOOL                   doAllocIon(MyIonInfo& rIonInfo, size_t /*boundaryInBytesToAlloc*/, SecType /*secType*/) override
                                    {
                                        auto const& rBlobIonInfo = mBlobHeap->mvIonInfo[0];

                                        if  (CC_UNLIKELY(rIonInfo.sizeInBytes > rBlobIonInfo.sizeInBytes)) {
                                            CAM_ULOGME("[%s] BLOB heap size:%zu < requested heap size:%zu", __FUNCTION__, rBlobIonInfo.sizeInBytes, rIonInfo.sizeInBytes);
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
ISecureImageBufferHeap*
ISecureImageBufferHeap::
create(
    char const* szCallerName,
    AllocImgParam_t const& rImgParam,
    AllocExtraParam const& rExtraParam,
    MBOOL const enableLog
)
{
    return SecureImageBufferHeap::create(szCallerName, rImgParam, rExtraParam, enableLog);
}


/******************************************************************************
 *
 ******************************************************************************/
SecureImageBufferHeap*
SecureImageBufferHeap::
create(
    char const* szCallerName,
    AllocImgParam_t const& rImgParam,
    AllocExtraParam const& rExtraParam,
    MBOOL const enableLog
)
{
    MUINT const planeCount = Format::queryPlaneCount(rImgParam.imgFormat);
#if 0
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
    SecureImageBufferHeap* pHeap = NULL;
    pHeap = new SecureImageBufferHeap(szCallerName, rImgParam, rExtraParam);
    if  ( ! pHeap )
    {
        CAM_ULOGME("%s@ Fail to new", (szCallerName?szCallerName:"unknown"));
        return NULL;
    }
    //
    if  ( ! pHeap->onCreate(rImgParam.imgSize, rImgParam.imgFormat,
                rImgParam.bufSize, rExtraParam.secType, enableLog) )
    {
        CAM_ULOGME("%s@ onCreate", (szCallerName?szCallerName:"unknown"));
        delete pHeap;
        return NULL;
    }
    //
    return pHeap;
}


/******************************************************************************
 *
 ******************************************************************************/
ISecureImageBufferHeap*
SecureImageBufferHeap::
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

    sp<SecureImageBufferHeap> pBlobHeap = this;
    auto extraParam = mExtraParam;
    extraParam.contiguousPlanes = MTRUE; // must be contiguous planes since it's originally from a BLOB heap
    SecureBlobDerivedIonHeap* pHeap = new SecureBlobDerivedIonHeap(szCallerName, rImgParam, extraParam, pBlobHeap);
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
SecureImageBufferHeap::
SecureImageBufferHeap(
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
SecureImageBufferHeap::
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
SecureImageBufferHeap::
impInit(BufInfoVect_t const& rvBufInfo)
{
    //  rIonInfo.pa
    auto mapPA = [this](char const* szCallerName, MyIonInfo& rIonInfo) {
        if  (CC_UNLIKELY( ! doMapPhyAddr(szCallerName, rIonInfo) )) {
            CAM_ULOGME("[mapPA] %s@ doMapPhyAddr", szCallerName);
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
        if  ( ! helpCheckBufStrides(i, mBufStridesInBytesToAlloc[i]) )
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
            if (! doAllocIon(rIonInfo, mBufBoundaryInBytesToAlloc[i], mExtraParam.secType)) {
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
        if (! doAllocIon(rIonInfo, mBufBoundaryInBytesToAlloc[0], mExtraParam.secType)) {
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
    if  ( ! ret )
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
SecureImageBufferHeap::
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
SecureImageBufferHeap::
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
MVOID
SecureImageBufferHeap::
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
    if  ( 0 <= rIonInfo.ionFd )
    {
        ::ion_share_close(mIonDevice->getDeviceFd(), rIonInfo.ionFd);
        rIonInfo.ionFd = -1;
    }
    //
    //  ion_free: buf handle
    if  ( 0 != rIonInfo.ionHandle )
    {
        if(ion_free(mIonDevice->getDeviceFd(), rIonInfo.ionHandle)) {
            MY_LOGE("ion_free is failed : ionDevFD(%d), ionHandle(%d)", mIonDevice->getDeviceFd(), rIonInfo.ionHandle);
        } else {
            rIonInfo.ionHandle = 0;
        }
    }
}

MBOOL
SecureImageBufferHeap::
doAllocIon(MyIonInfo& rIonInfo, size_t boundaryInBytesToAlloc, SecType secType)
{
    int err = 0;
    int ion_prot_flags = mExtraParam.nocache ? 0 : (ION_FLAG_CACHED | ION_FLAG_CACHED_NEEDS_SYNC);
    int sec_type_flags = (secType == SecType::mem_secure) ? (int)(ION_HEAP_MULTIMEDIA_TYPE_2D_FR_MASK) : (int)(ION_HEAP_MULTIMEDIA_TYPE_PROT_MASK);
    MY_LOGD("Allocate secure memory type : %d", sec_type_flags);

    if (ion_alloc(mIonDevice->getDeviceFd(), rIonInfo.sizeInBytes, boundaryInBytesToAlloc,
                sec_type_flags, ion_prot_flags, &rIonInfo.ionHandle))
    {
        MY_LOGE("ion_alloc failed: ionDevFD(%d), BufSize(%zu)",
                mIonDevice->getDeviceFd(), rIonInfo.sizeInBytes);
        goto lbExit;
    }

    // obtain a file descriptor that can pass to other clients
    if (ion_share(mIonDevice->getDeviceFd(), rIonInfo.ionHandle, &rIonInfo.ionFd))
    {
        MY_LOGE("ion_share failed: ionDevFD(%d), BufSize(%zu)," \
                " ionHandle(%d)",
                mIonDevice->getDeviceFd(), rIonInfo.sizeInBytes, rIonInfo.ionHandle);
        goto lbExit;
    }

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


    return MTRUE;
lbExit:
    return  MFALSE;
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
SecureImageBufferHeap::
impLockBuf(
    char const* szCallerName,
    MINT usage,
    BufInfoVect_t const& rvBufInfo
)
{
    auto fillSecureHandle = [](const struct ion_sys_data& sysData,
            MINT usage, auto& myBufInfo, auto& bufInfo)
    {
        if (usage & eBUFFER_USAGE_SW_MASK)
        {
            myBufInfo->va = sysData.get_phys_param.phy_addr;
            bufInfo->va   = sysData.get_phys_param.phy_addr;
        }

        if (usage & eBUFFER_USAGE_HW_MASK)
        {
            myBufInfo->pa = sysData.get_phys_param.phy_addr;
            bufInfo->pa   = sysData.get_phys_param.phy_addr;
        }
    };

    MBOOL ret = MFALSE;
    if (mvIonInfo.size() == rvBufInfo.size())
    {
        for (size_t i = 0; i < rvBufInfo.size(); i++) {
            auto& rIonInfo = mvIonInfo[i];
            struct ion_sys_data sys_data;

            //  SW and/or HW Access
            if  ((usage & eBUFFER_USAGE_SW_MASK) || (usage & eBUFFER_USAGE_HW_MASK))
            {
                sys_data.sys_cmd = ION_SYS_GET_PHYS;
                sys_data.get_phys_param.handle = rIonInfo.ionHandle;
                if (ion_custom_ioctl(mIonDevice->getDeviceFd(), ION_CMD_SYSTEM, &sys_data))
                {
                    MY_LOGE("ion_custom_ioctl failed to get secure handle");
                    goto lbExit;
                }

                MY_LOGI("Secure memory locked: handle(0x%x) size(%lu)",
                        sys_data.get_phys_param.phy_addr, sys_data.get_phys_param.len);

                fillSecureHandle(sys_data, usage, mvBufInfo[i], rvBufInfo[i]);
            }
        }

    }
    else if(mvIonInfo.size() == 1)
    {
        // Usecases: Blob heap -> multi-plane (e.g. YV12) heap
        auto& rIonInfo = mvIonInfo[0];
        struct ion_sys_data sys_data;

        //  SW and/or HW Access
        if  ((usage & eBUFFER_USAGE_SW_MASK) || (usage & eBUFFER_USAGE_HW_MASK))
        {
            sys_data.sys_cmd = ION_SYS_GET_PHYS;
            sys_data.get_phys_param.handle = rIonInfo.ionHandle;
            if (ion_custom_ioctl(mIonDevice->getDeviceFd(), ION_CMD_SYSTEM, &sys_data))
            {
                MY_LOGE("ion_custom_ioctl failed to get secure handle");
                goto lbExit;
            }

            MY_LOGI("Secure memory allocated: handle(0x%x) size(%lu)",
                    sys_data.get_phys_param.phy_addr, sys_data.get_phys_param.len);
            for (size_t i = 0; i < rvBufInfo.size(); i++)
                fillSecureHandle(sys_data, usage, mvBufInfo[i], rvBufInfo[i]);
        }
    }

    ret = MTRUE;

lbExit:
    if  (!ret)
        impUnlockBuf(szCallerName, usage, rvBufInfo);

    return ret;
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
SecureImageBufferHeap::
impUnlockBuf(
    char const* szCallerName,
    MINT usage,
    BufInfoVect_t const& rvBufInfo
)
{
    for (size_t i = 0; i < rvBufInfo.size(); i++)
    {
        auto& pBufInfo(rvBufInfo[i]);

        //  HW Access
        if  (usage & eBUFFER_USAGE_HW_MASK)
        {
            if  (0 != pBufInfo->pa)
                pBufInfo->pa = 0;
            else
                MY_LOGW("%s@ skip PA=0 at %zu-th plane", szCallerName, i);
        }

        //  SW Access
        if  (usage & eBUFFER_USAGE_SW_MASK)
        {
            if  (0 != pBufInfo->va)
                pBufInfo->va = 0;
            else
                MY_LOGW("%s@ skip VA=0 at %zu-th plane", szCallerName, i);
        }
    }

    return  MTRUE;
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
SecureImageBufferHeap::
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
SecureImageBufferHeap::
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
SecureImageBufferHeap::
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
SecureImageBufferHeap::
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
SecureImageBufferHeap::
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
    return s;
}


/******************************************************************************
 *  ION Image Buffer Heap Allocator Interface.
 ******************************************************************************/
namespace {
class FixedSizeAllocatorImpl : public ISecureImageBufferHeapAllocator
                             , public IDebuggee
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
    class MyHeap : public SecureImageBufferHeap
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
                            MyHeap* pHeap = new MyHeap(name, rImgParam, rExtraParam);
                            if  ( ! pHeap ) {
                                CAM_ULOGME("Fail to new MyHeap");
                                return nullptr;
                            }
                            //
                            if  ( ! pHeap->onCreate(rImgParam.imgSize,
                                        rImgParam.imgFormat, rImgParam.bufSize,
                                        rExtraParam.secType, enableLog) ) {
                                CAM_ULOGME("onCreate");
                                delete pHeap;
                                return nullptr;
                            }
                            //
                            return pHeap;
                        }

                        template<typename... Args>
                        MyHeap(const Args&... args)
                            : SecureImageBufferHeap(args...)
                        {}

        virtual MVOID   incStrong(MVOID const* id) const
                        {
                            mStrong.fetch_add(1, std::memory_order_relaxed);
                            SecureImageBufferHeap::incStrong(id);
                        }

        virtual MVOID   decStrong(MVOID const* id) const
                        {
                            SecureImageBufferHeap::decStrong(id);
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
                            CAM_ULOGMD("[%s] + mFreedHeapList.size:%zu, mAllocHeapList.size:%zu", __FUNCTION__, mFreedHeapList.size(), mAllocHeapList.size());
                            mFreedHeapList.clear();
                            mAllocHeapList.clear();
                            CAM_ULOGMD("[%s] -", __FUNCTION__);
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
                        ) -> ISecureImageBufferHeap*
                        {
                            if  (CC_UNLIKELY(
                                    rExtraParam.nocache != mAllocExtraParam.nocache
                                ||  rExtraParam.security != mAllocExtraParam.security
                                ||  rExtraParam.coherence != mAllocExtraParam.coherence
                                //||  rExtraParam.contiguousPlanes != MTRUE
                                ))
                            {
                                CAM_ULOGME(
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
                                CAM_ULOGMW("[%s] %s: has no free heap available - reserved %zux%zu",
                                    __FUNCTION__, szUserName, mReserveParam.numberOfBuffers, mReserveParam.bufferSizeInBytes);
                                return nullptr;
                            }

                            auto it = mFreedHeapList.begin();
                            mAllocHeapList.splice(mAllocHeapList.end(), mFreedHeapList, it);

                            ISecureImageBufferHeap* pHeap = nullptr;
                            MyHeap* pMyHeap = (*it).get();
                            if  (CC_LIKELY(pMyHeap!=nullptr)) {
                                pHeap = pMyHeap->createImageBufferHeap_FromBlobHeap(
                                    szUserName, rImgParam, enableLog
                                );
                                #if 1
                                CAM_ULOGMI("[%s] MyHeap:%p getStrongCount:%d mStrong:%d", __FUNCTION__, pMyHeap, pMyHeap->getStrongCount(), pMyHeap->mStrong.load());
                                CAM_ULOGMI("[%s] pHeap:%p getStrongCount:%x", __FUNCTION__, pHeap, pHeap->getStrongCount());
                                #endif
                            }
                            return pHeap;
                        }

        auto            release(MyHeap* pMyHeap) -> MVOID
                        {
                            std::lock_guard<std::mutex> _l(mHeapListLock);
                            for (MyHeapListT::iterator it = mAllocHeapList.begin(); it != mAllocHeapList.end(); it++) {
                                if  ( (*it).get() == pMyHeap ) {
                                    mFreedHeapList.splice(mFreedHeapList.end(), mAllocHeapList, it);
                                    #if 1
                                    CAM_ULOGMI("[%s] hit! MyHeap:%p getStrongCount:%d mStrong:%d", __FUNCTION__, pMyHeap, pMyHeap->getStrongCount(), pMyHeap->mStrong.load());
                                    #endif
                                    return;
                                }
                            }
                            CAM_ULOGMW("[%s] not found! MyHeap:%p #mFreedHeapList:%zu, #mAllocHeapList:%zu", __FUNCTION__, pMyHeap, mFreedHeapList.size(), mAllocHeapList.size());
                        }

        auto            startReserve() -> MBOOL
                        {
                            mReserveFuture = std::async(std::launch::async, [this](std::weak_ptr<HeapManager> weak){
                                std::string const threadName{std::string{"RSV:"}+mName};
                                ::prctl(PR_SET_NAME, (unsigned long)threadName.c_str(), 0, 0, 0);
                                //With this shared ptr, we're sure all members are alive inside this thread.
                                std::shared_ptr<HeapManager> shared = weak.lock();
                                if  ( shared == nullptr ) {
                                    CAM_ULOGME("[%s] Dead heap manager on reserving thread", threadName.c_str());
                                    return;
                                }

                                auto const reserveParam = mReserveParam;
                                CAM_ULOGMI("FixedSizeAllocator: reserving thread start... - reserved:%zux%zu",
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
                                        CAM_ULOGME("[%s] %zu: Fail to reserve heaps", threadName.c_str(), i);
                                        toRelease = true;
                                        break;
                                    }
                                }

                                if  (toRelease) {
                                    CAM_ULOGMW("[%s] cancel reserving and start releasing heaps - isStartUnreserve:%d", threadName.c_str(), isStartUnreserve.load());
                                    doReleaseHeaps();
                                }

                                shared.reset();
                                CAM_ULOGMI("FixedSizeAllocator: reserving thread done - reserved:%zux%zu isStartUnreserve:%d",
                                    reserveParam.numberOfBuffers, reserveParam.bufferSizeInBytes, isStartUnreserve.load());
                            }, mWeak);
                            return mReserveFuture.valid() ? MTRUE : MFALSE;
                        }

        auto            startUnreserve(char const* /*szUserName*/) -> void
                        {
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
                                    CAM_ULOGMW("[%s] Dead heap manager on unreserving thread", threadName.c_str());
                                    return;
                                }
                                CAM_ULOGMI("FixedSizeAllocator: unreserving thread start...");
                                doReleaseHeaps();
                                shared.reset();
                                CAM_ULOGMI("FixedSizeAllocator: unreserving thread done");
                            }, mWeak);
                            t.detach();
                        }

    protected:
        auto            doReleaseHeaps() -> void
                        {
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
protected:  ////
    std::shared_ptr<IDebuggeeCookie>mDebuggeeCookie = nullptr;

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
                        CAM_ULOGMD("+ mDebuggeeCookie:%p", mDebuggeeCookie.get());
                        mDebuggeeCookie = nullptr;
                        CAM_ULOGMD("-");
                    }

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////
    virtual auto    debuggeeName() const -> std::string { return "NSCam::ISecureImageBufferHeapAllocator"; }
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

public:     ////    ISecureImageBufferHeapAllocator Interface.

    static  auto    getInstance() -> FixedSizeAllocatorImpl*
                    {
                        //Make sure IDebuggeeManager singleton is fully constructed before this singleton.
                        //So that it's safe to access IDebuggeeManager instance from this singleton's destructor.
                        static auto pDbgMgr = IDebuggeeManager::get();
                        static auto inst = std::make_shared<FixedSizeAllocatorImpl>();
                        static auto init = [](){
                            if ( pDbgMgr ) {
                                inst->mDebuggeeCookie = pDbgMgr->attach(inst);
                            }
                            return true;
                        }();
                        return inst.get();
                    }

    virtual auto    unreserve(char const* szUserName) -> MBOOL
                    {
                        if  ( CC_UNLIKELY(!szUserName) ) {
                            CAM_ULOGME("[%s] unknown name", __FUNCTION__);
                            return MFALSE;
                        }

                        std::shared_ptr<HeapManager> pHeapManager = nullptr;
                        EState state = EState::UNKNOWN;
                        {
                            std::lock_guard<std::mutex> _l(mStateLock);
                            state = mState;
                            CAM_ULOGMI("[%s] %s: try to unreserve - refcount:%d state:(%u)",
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
                                CAM_ULOGMD("[%s] %s@ state:(%u) mReserveRefCount:%d - do nothing",
                                    __FUNCTION__, szUserName, state, mReserveRefCount.load());
                                return MFALSE;
                                break;
                            }
                        }

                        if (pHeapManager != nullptr) {
                            CAM_ULOGMD("[%s] %s@ state:(%u) - reset heap manager", __FUNCTION__, szUserName, state);
                            pHeapManager.reset();
                        }
                        return MTRUE;
                    }

    virtual auto    reserve(
                        char const* szUserName,
                        ReserveParam const& param
                    ) -> MBOOL
                    {
                        if  ( CC_UNLIKELY(!szUserName) ) {
                            CAM_ULOGME("[%s] unknown name", __FUNCTION__);
                            return MFALSE;
                        }

                        if  ( CC_UNLIKELY(0==param.numberOfBuffers || 0==param.bufferSizeInBytes) ) {
                            CAM_ULOGME("[%s] %s: try to reserve bad size:%zux%zu", __FUNCTION__, szUserName, param.numberOfBuffers, param.bufferSizeInBytes);
                            return MFALSE;
                        }

                        EState state = EState::UNKNOWN;
                        {
                            std::lock_guard<std::mutex> _l(mStateLock);
                            state = mState;
                            std::lock_guard<std::mutex> _lhm(mHeapManagerLock);
                            CAM_ULOGMI("[%s] %s: try to reserve %zux%zu - refcount:%d state:(%u) mHeapManager:%p",
                                __FUNCTION__, szUserName,
                                param.numberOfBuffers, param.bufferSizeInBytes,
                                mReserveRefCount.load(), state, mHeapManager.get());
                            switch (state)
                            {
                            case EState::RESERVE:{
                                if (CC_UNLIKELY(mHeapManager == nullptr)) {
                                    CAM_ULOGME("[%s] %s@ state:(%u) - Bad mHeapManager",
                                        __FUNCTION__, szUserName, state);
                                    return MFALSE;
                                }

                                if ( 0!=memcmp(&mHeapManager->getReserveParam(), &param, sizeof(ReserveParam)) ) {
                                    CAM_ULOGME("[%s] %s@ state:(%u) - not allowed to reserve the memory with different settings again",
                                        __FUNCTION__, szUserName, state);
                                    return MFALSE;
                                }

                                mReserveRefCount++;
                                }break;

                            case EState::UNRESERVE:{
                                mHeapManager = std::make_shared<HeapManager>(szUserName, param);
                                if  (CC_UNLIKELY(mHeapManager == nullptr)) {
                                    CAM_ULOGME("[%s] %s@ state:(%u) - Bad mHeapManager",
                                        __FUNCTION__, szUserName, state);
                                    return MFALSE;
                                }
                                //
                                mHeapManager->setWeak(mHeapManager);
                                if ( ! mHeapManager->startReserve() ) {
                                    CAM_ULOGME("[%s] %s@ state:(%u) - Fail on mHeapManager->startReserve",
                                        __FUNCTION__, szUserName, state);
                                    mHeapManager.reset();
                                    return MFALSE;
                                }

                                mReserveRefCount++;
                                mState = EState::RESERVE;
                                }break;

                            default:
                                break;
                            }
                        }
                        return MTRUE;
                    }

    virtual auto    create(
                        char const* szUserName,
                        AllocImgParam_t const& rImgParam,
                        AllocExtraParam_t const& rExtraParam,
                        MBOOL const enableLog
                    ) -> ISecureImageBufferHeap*
                    {
                        if  (CC_UNLIKELY(!szUserName)) {
                            CAM_ULOGME("[%s] unknown name", __FUNCTION__);
                            return nullptr;
                        }

                        if  (CC_UNLIKELY(0!=rImgParam.bufBoundaryInBytes[0])) {
                            CAM_ULOGME("[%s] %s: bufBoundaryInBytes[0]:%zu!=0", __FUNCTION__, szUserName, rImgParam.bufBoundaryInBytes[0]);
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
                                    CAM_ULOGMW("[%s] %s@ state:RESERVE - Bad mHeapManager", __FUNCTION__, szUserName);
                                    break;
                                }
                                auto pHeap = mHeapManager->create(szUserName, rImgParam, rExtraParam, enableLog);
                                if  ( CC_UNLIKELY(pHeap == nullptr) ) {
                                    CAM_ULOGMW("[%s] %s@ state:RESERVE - fail on mHeapManager->create", __FUNCTION__, szUserName);
                                    break;
                                }
                                return pHeap;
                            }
                        } while(0);

                        //Here we allocate the heap way as usual.
                        //CAM_LOGD_IF(0, "[%s] %s@ state:(%u) - alloc normally", __FUNCTION__, szUserName, state);
                        return ISecureImageBufferHeap::create(szUserName, rImgParam, rExtraParam, enableLog);
                    }

    auto            release(MyHeap* pHeap) -> MVOID
                    {
                        std::shared_ptr<HeapManager> pHeapManager = nullptr;
                        {
                            std::lock_guard<std::mutex> _l(mStateLock);
                            if  (mState == EState::RESERVE) {
                                std::lock_guard<std::mutex> _l(mHeapManagerLock);
                                pHeapManager = mHeapManager;
                            }
                        }

                        if  ( CC_UNLIKELY(pHeapManager == nullptr) ) {
                            CAM_ULOGMW("[%s] state:RESERVE - Bad mHeapManager for pHeap:%p", __FUNCTION__, pHeap);
                            return;
                        }
                        //release to the reserved pool
                        pHeapManager->release(pHeap);
                        pHeapManager.reset();
                    }

};
};  // namespace


/******************************************************************************
 *
 ******************************************************************************/
ISecureImageBufferHeapAllocator*
ISecureImageBufferHeapAllocator::getInstance()
{
    return FixedSizeAllocatorImpl::getInstance();
}


/******************************************************************************
 *
 ******************************************************************************/
#endif  //MTK_ION_SUPPORT


