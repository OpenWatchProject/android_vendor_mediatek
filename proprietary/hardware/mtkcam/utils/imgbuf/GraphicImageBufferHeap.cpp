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

#define LOG_TAG "MtkCam/GraphicImageBufferHeap"
//
#include "MyUtils.h"
#include "BaseImageBufferHeap.h"
#include <mtkcam/utils/imgbuf/IGraphicImageBufferHeap.h>
#include <mtkcam/utils/std/ULog.h>

#include <vndk/hardware_buffer.h>
#include <ui/gralloc_extra.h>

#include <linux/ion_drv.h>

CAM_ULOG_DECLARE_MODULE_ID(MOD_IMAGE_BUFFER);
//
using namespace android;
using namespace NSCam;
//


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
#define MY_LOGV_IF(cond, ...)       do { if (            (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if (            (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if (            (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( CC_UNLIKELY(cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( CC_UNLIKELY(cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( CC_UNLIKELY(cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( CC_UNLIKELY(cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)


/******************************************************************************
 *  Image Buffer Heap.
 ******************************************************************************/
namespace {
class GraphicImageBufferHeapBase : public IGraphicImageBufferHeap
                             , protected NSImageBufferHeap::BaseImageBufferHeap
{
    friend  class IGraphicImageBufferHeap;
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IGraphicImageBufferHeap Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Accessors.
    virtual buffer_handle_t         getBufferHandle()   const   { return mBufferParams.handle; }
    virtual buffer_handle_t*        getBufferHandlePtr()const   { return const_cast<buffer_handle_t*>(&mBufferParams.handle); }
    virtual MINT                    getAcquireFence()   const   { return mAcquireFence; }
    virtual MVOID                   setAcquireFence(MINT fence) { mAcquireFence = fence; }
    virtual MINT                    getReleaseFence()   const   { return mReleaseFence; }
    virtual MVOID                   setReleaseFence(MINT fence) { mReleaseFence = fence; }

    virtual int                     getGrallocUsage()   const   { return mBufferParams.usage; }

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  BaseImageBufferHeap Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////
    virtual char const*             impGetMagicName()                   const   { return magicName(); }
    virtual void*                   impGetMagicInstance()               const   { return (void *)this; }
    virtual HeapInfoVect_t const&   impGetHeapInfo()                    const   { return *(HeapInfoVect_t*)(&mvHeapInfo); }
    virtual MBOOL                   impInit(BufInfoVect_t const& rvBufInfo);
    virtual MBOOL                   impUninit(BufInfoVect_t const& rvBufInfo);
    virtual MBOOL                   impReconfig(BufInfoVect_t const& rvBufInfo __unused) { return MFALSE; }

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
    virtual void*                   getHWBuffer()  const    { return (void*)ANativeWindowBuffer_getHardwareBuffer(mpANWBuffer); };

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Definitions.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////                    Heap Info.
    struct MyHeapInfo : public HeapInfo
    {
    };
    typedef Vector<sp<MyHeapInfo> > MyHeapInfoVect_t;

protected:  ////                    Buffer Info.
    typedef Vector<sp<BufInfo> >    MyBufInfoVect_t;

    struct BufferParams {
        int width = 0;
        int height = 0;
        int stride = 0;
        int format = 0;
        int usage = 0;
        uintptr_t layerCount = 0;
        buffer_handle_t handle = nullptr;
        bool owner = false;
        SecType secType = SecType::mem_normal;
    };

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Instantiation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////                    Destructor/Constructors.

    /**
     * Disallowed to directly delete a raw pointer.
     */
    virtual                         ~GraphicImageBufferHeapBase() = default;
                                    GraphicImageBufferHeapBase(
                                        char const* szCallerName,
                                        BufferParams& rBufParams,
                                        ANativeWindowBuffer* pANWBuffer,
                                        MINT const acquire_fence,
                                        MINT const release_fence
                                    );

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Data Members.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////
    MyHeapInfoVect_t                mvHeapInfo;
    MyBufInfoVect_t                 mvBufInfo;
    GrallocDynamicInfo              mGrallocInfo;

protected:  ////
    BufferParams                    mBufferParams;
    ANativeWindowBuffer*            mpANWBuffer;
    MINT                            mAcquireFence;
    MINT                            mReleaseFence;
};

class NormalGraphicImageBufferHeap final : protected GraphicImageBufferHeapBase
{
protected:
    friend class IGraphicImageBufferHeap;
    /**
     * Disallowed to directly delete a raw pointer.
     */
    NormalGraphicImageBufferHeap(
            char const* szCallerName,
            BufferParams& rBufParams,
            ANativeWindowBuffer* pANWBuffer,
            MINT const acquire_fence,
            MINT const release_fence)
    : GraphicImageBufferHeapBase(
            szCallerName, rBufParams, pANWBuffer, acquire_fence, release_fence)
    {};

    ~NormalGraphicImageBufferHeap() = default;
};

class SecureGraphicImageBufferHeap final : protected GraphicImageBufferHeapBase
{
public:
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  BaseImageBufferHeap Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    MBOOL impLockBuf(
            char const* szCallerName,
            MINT usage,
            BufInfoVect_t const& rvBufInfo) override;
    MBOOL impUnlockBuf(
            char const* szCallerName,
            MINT usage,
            BufInfoVect_t const& rvBufInfo) override;

protected:
    friend class IGraphicImageBufferHeap;
    /**
     * Disallowed to directly delete a raw pointer.
     */
    SecureGraphicImageBufferHeap(
            char const* szCallerName,
            BufferParams& rBufParams,
            ANativeWindowBuffer* pANWBuffer,
            MINT const acquire_fence,
            MINT const release_fence);

    ~SecureGraphicImageBufferHeap() = default;

private:
    SECHAND mSecureHandle;

    void getSecureHandle(SECHAND& secureHandle);
};

};  //namespace


IGraphicImageBufferHeap*
IGraphicImageBufferHeap::
castFrom(
    IImageBufferHeap* pImageBufferHeap
)
{
    if (CC_UNLIKELY(strcmp(pImageBufferHeap->getMagicName(),IGraphicImageBufferHeap::magicName())))
    {
//        CAM_ULOGMW("MagicName (%s,%s) is different!", pImageBufferHeap->getMagicName(),IGraphicImageBufferHeap::magicName());
        return nullptr;
    }
    else
    {
        return static_cast<GraphicImageBufferHeapBase*>(pImageBufferHeap->getMagicInstance());
    }
}


/******************************************************************************
 *
 ******************************************************************************/
IGraphicImageBufferHeap*
IGraphicImageBufferHeap::
create(
    char const* szCallerName,
    uint32_t width,
    uint32_t height,
    uint32_t usage,
    buffer_handle_t* buffer,
    MINT const acquire_fence,
    MINT const release_fence,
    bool keepOwnership,
    SecType secType,
    std::shared_ptr<IIonDevice> ionDevice
)
{
    if  (CC_UNLIKELY( ! buffer )) {
        CAM_ULOGME("bad buffer_handle_t");
        return nullptr;
    }
    //
    if  (CC_UNLIKELY( ! *buffer )) {
        CAM_ULOGME("bad *buffer_handle_t");
        return nullptr;
    }
    //
    if ( keepOwnership==true )
    {
        CAM_ULOGME("not implement keepOwnership=ture");
        return nullptr;
    }
    //
    GrallocStaticInfo staticInfo;
    status_t status = IGrallocHelper::singleton()->query(*buffer, &staticInfo, nullptr);
    if  (CC_UNLIKELY( OK != status )) {
        CAM_ULOGME("cannot query the real format from buffer_handle_t - status:%d(%s)", status, ::strerror(-status));
        return  nullptr;
    }
    //
    MSize alloc_size = MSize(width, height);
    if ( staticInfo.format==eImgFmt_BLOB )  // resize from [w x h] to [w*h x 1]
        alloc_size = MSize(width*height, 1);
    //
    GraphicImageBufferHeapBase::BufferParams rBufParams =
    {
        .width          = alloc_size.w,
        .height         = alloc_size.h,
        .stride         = 0,
        .format         = staticInfo.format,
        .usage          = static_cast<int>(usage),
        .layerCount     = 1,
        .handle         = const_cast<native_handle_t*>(*buffer),
        .owner          = keepOwnership,
        .secType        = secType,
    };
    auto *pHeap = [&]() -> GraphicImageBufferHeapBase*
    {
        if (secType == SecType::mem_normal)
            return new NormalGraphicImageBufferHeap(
                    szCallerName, rBufParams, nullptr, acquire_fence, release_fence);

        return new SecureGraphicImageBufferHeap(
                szCallerName, rBufParams, nullptr, acquire_fence, release_fence);
    }();
    //
    if  (CC_UNLIKELY( ! pHeap )) {
        CAM_ULOGME("Fail to new a heap");
        return nullptr;
    }
    MSize const imgSize(rBufParams.width, rBufParams.height);
    //MINT format = pGraphicBuffer->getPixelFormat();
    MINT format = staticInfo.format;
    switch  (format)
    {
    case eImgFmt_BLOB:
        CAM_ULOGMW("create() based-on buffer_handle_t...");
#if 0
        CAM_ULOGMW("force to convert BLOB format to JPEG format");
        format = eImgFmt_JPEG;
#else
        CAM_ULOGMW("should we convert BLOB format to JPEG format ???");
#endif
        break;
    }
    auto ret = pHeap->onCreate(GraphicImageBufferHeapBase::OnCreate{
                        .imgSize = imgSize,
                        .imgFormat = format,
                        .bitstreamSize = 0,
                        .secType = secType,
                        .enableLog = MFALSE,
                        .useSharedDeviceFd = -1,
                        .ionDevice = ionDevice,
                    });
    if  (CC_UNLIKELY( ! ret )) {
        CAM_ULOGME("onCreate width(%d) height(%d)", imgSize.w, imgSize.h);
        delete pHeap;
        return nullptr;
    }
    //
    return pHeap;
}


/******************************************************************************
 *
 ******************************************************************************/
IGraphicImageBufferHeap*
IGraphicImageBufferHeap::
create(
    char const* szCallerName,
    uint32_t usage,
    MSize imgSize,
    int format,
    buffer_handle_t* buffer,
    MINT const acquire_fence,
    MINT const release_fence,
    int stride,
    SecType secType,
    std::shared_ptr<IIonDevice> ionDevice
)
{
    if  (CC_UNLIKELY( ! buffer )) {
        CAM_ULOGME("bad buffer_handle_t");
        return nullptr;
    }
    //
    if  (CC_UNLIKELY( ! *buffer )) {
        CAM_ULOGME("bad *buffer_handle_t");
        return nullptr;
    }
    //
    GrallocStaticInfo staticInfo;
    status_t status = IGrallocHelper::singleton()->query(*buffer, &staticInfo, nullptr);
    if  (CC_UNLIKELY( OK != status )) {
        CAM_ULOGME("cannot query the real format from buffer_handle_t - status:%d(%s)", status, ::strerror(-status));
        return nullptr;
    }
    //
    MSize alloc_size = imgSize;
    if ( staticInfo.format == eImgFmt_BLOB )  // resize from [w x h] to [w*h x 1]
        alloc_size = MSize(imgSize.w*imgSize.h, 1);
    //
    switch (staticInfo.format)
    {
    case eImgFmt_BLOB:
        if ( format == eImgFmt_JPEG ) {
            CAM_ULOGMD("BLOB -> JPEG");
            break;
        }
        else if ( format == eImgFmt_ISP_TUING ) {
           CAM_LOGD_IF(1, "BLOB -> ISP_TUNING");
           break;
        }
        CAM_ULOGME("the buffer format is BLOB, but the given format:%d != JPEG", format);
        return nullptr;
        break;
    case HAL_PIXEL_FORMAT_RAW16:
        CAM_ULOGMD("RAW16 -> %#x(%s)",
            format, NSCam::Utils::Format::queryImageFormatName(format).c_str());
        break;
    case HAL_PIXEL_FORMAT_CAMERA_OPAQUE:
        if ( (format >= eImgFmt_BAYER8 && format <= eImgFmt_BAYER14) || (format >= eImgFmt_UFO_BAYER8 && format <= eImgFmt_UFO_BAYER14) )
        {
            CAM_ULOGMI("HAL_PIXEL_FORMAT_CAMERA_OPAQUE -> PACK_RAW(0x%X)", format);
            break;
        }
        if (CC_UNLIKELY( format != staticInfo.format && format != eImgFmt_BLOB )) {
            CAM_ULOGME("the buffer_handle_t format:%d is neither BLOB, nor the given format:%d", staticInfo.format, format);
            return nullptr;
        }
        break;
    default:
        if (CC_UNLIKELY( format != staticInfo.format && format != eImgFmt_BLOB )) {
            CAM_ULOGME("the buffer_handle_t format:%d is neither BLOB, nor the given format:%d", staticInfo.format, format);
            return nullptr;
        }
        break;
    }
    //
    GraphicImageBufferHeapBase::BufferParams rBufParams =
    {
        .width          = alloc_size.w,
        .height         = alloc_size.h,
        .stride         = stride,
        .format         = format,
        .usage          = static_cast<int>(usage),
        .layerCount     = 1,
        .handle         = const_cast<native_handle_t*>(*buffer),
        .owner          = false,
        .secType        = secType,
    };
    auto *pHeap = [&]() -> GraphicImageBufferHeapBase*
    {
        if (secType == SecType::mem_normal)
            return new NormalGraphicImageBufferHeap(
                    szCallerName, rBufParams, nullptr, acquire_fence, release_fence);

        return new SecureGraphicImageBufferHeap(
                szCallerName, rBufParams, nullptr, acquire_fence, release_fence);
    }();
    //
    if  (CC_UNLIKELY( ! pHeap )) {
        CAM_ULOGME("Fail to new a heap");
        return nullptr;
    }
    //
    //BLOB: [w*h x 1]
    //JPEG: [w*h x 1] or [w x h]?
    auto ret = pHeap->onCreate(GraphicImageBufferHeapBase::OnCreate{
                        .imgSize = alloc_size,
                        .imgFormat = format,
                        .bitstreamSize = 0,
                        .secType = secType,
                        .enableLog = MFALSE,
                        .useSharedDeviceFd = -1,
                        .ionDevice = ionDevice,
                    });
    if  (CC_UNLIKELY( ! ret )) {
        CAM_ULOGME("onCreate imgSize:%dx%d format:%d buffer format:%d",
            imgSize.w, imgSize.h, format, staticInfo.format);
        delete pHeap;
        return nullptr;
    }
    //
    return pHeap;
}


/******************************************************************************
 *
 ******************************************************************************/
IGraphicImageBufferHeap*
IGraphicImageBufferHeap::
create(
    char const* szCallerName,
    ANativeWindowBuffer* pANWBuffer,
    MINT const acquire_fence,
    MINT const release_fence,
    bool keepOwnership,
    SecType secType,
    std::shared_ptr<IIonDevice> ionDevice
)
{
    if  (CC_UNLIKELY( pANWBuffer == 0 )) {
        CAM_ULOGME("NULL ANativeWindowBuffer");
        return nullptr;
    }
    //
    if ( keepOwnership==true )
    {
        CAM_ULOGME("not implement keepOwnership=ture");
        return nullptr;
    }
    //
    GraphicImageBufferHeapBase::BufferParams rBufParams =
    {
        .width          = pANWBuffer->width,
        .height         = pANWBuffer->height,
        .stride         = 0,
        .format         = pANWBuffer->format,
        .usage          = static_cast<int>(pANWBuffer->usage),
        .layerCount     = pANWBuffer->layerCount,
        .handle         = const_cast<native_handle_t*>(pANWBuffer->handle),
        .owner          = keepOwnership,
        .secType        = secType,
    };
    //
    GrallocStaticInfo staticInfo;
    status_t status = IGrallocHelper::singleton()->query(rBufParams.handle, &staticInfo, nullptr);
    if  (CC_UNLIKELY( OK != status )) {
        CAM_ULOGME("cannot query the real format from buffer_handle_t - status:%d(%s)", status, ::strerror(-status));
        return  nullptr;
    }
    //
    MINT const format = staticInfo.format;
    //MINT const format1 = pGraphicBuffer->getPixelFormat();
    MSize const imgSize(rBufParams.width, rBufParams.height);
    //
    auto *pHeap = [&]() -> GraphicImageBufferHeapBase*
    {
        if (secType == SecType::mem_normal)
            return new NormalGraphicImageBufferHeap(
                    szCallerName, rBufParams, pANWBuffer, acquire_fence, release_fence);

        return new SecureGraphicImageBufferHeap(
                szCallerName, rBufParams, pANWBuffer, acquire_fence, release_fence);
    }();
    if  (CC_UNLIKELY( ! pHeap )) {
        CAM_ULOGME("Fail to new a heap");
        return nullptr;
    }
    auto ret = pHeap->onCreate(GraphicImageBufferHeapBase::OnCreate{
                        .imgSize = imgSize,
                        .imgFormat = format,
                        .bitstreamSize = 0,
                        .secType = secType,
                        .enableLog = MFALSE,
                        .useSharedDeviceFd = -1,
                        .ionDevice = ionDevice,
                    });
    if  (CC_UNLIKELY( ! ret )) {
        CAM_ULOGME("onCreate width(%d) height(%d)", imgSize.w, imgSize.h);
        delete pHeap;
        return nullptr;
    }
    //
    return pHeap;
}


/******************************************************************************
 *
 ******************************************************************************/
GraphicImageBufferHeapBase::
GraphicImageBufferHeapBase(
    char const* szCallerName,
    BufferParams& rBufParams,
    ANativeWindowBuffer* pANWBuffer,
    MINT const acquire_fence,
    MINT const release_fence
)
    : IGraphicImageBufferHeap()
    , BaseImageBufferHeap(szCallerName)
    //
    , mvHeapInfo()
    , mvBufInfo()
    , mGrallocInfo()
    //
    , mBufferParams(rBufParams)
    , mpANWBuffer(nullptr)
    , mAcquireFence(acquire_fence)
    , mReleaseFence(release_fence)
{
    mpANWBuffer = pANWBuffer;
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
GraphicImageBufferHeapBase::
impInit(BufInfoVect_t const& rvBufInfo)
{
    GrallocStaticInfo staticInfo;
    IGrallocHelper*const pGrallocHelper = IGrallocHelper::singleton();
    //
    status_t status = OK;
    status = pGrallocHelper->setDirtyCamera(getBufferHandle());
    if  (CC_UNLIKELY( OK != status )) {
        MY_LOGE("cannot set dirty - status:%d(%s)", status, ::strerror(-status));
        return MFALSE;
    }
    //
    status = pGrallocHelper->setColorspace_JFIF(getBufferHandle());
    if  (CC_UNLIKELY( OK != status )) {
        MY_LOGE("cannot set colorspace JFIF - status:%d(%s)", status, ::strerror(-status));
        return MFALSE;
    }
    //
    Vector<int> const& ionFds = mGrallocInfo.ionFds;
    status = pGrallocHelper->query(getBufferHandle(), &staticInfo, &mGrallocInfo);
    if  (CC_UNLIKELY( OK != status )) {
        MY_LOGE("cannot query the real format from buffer_handle_t - status:%d[%s]", status, ::strerror(status));
        return MFALSE;
    }
    //
    mvHeapInfo.setCapacity(getPlaneCount());
    mvBufInfo.setCapacity(getPlaneCount());
    //
    if (eImgFmt_BLOB != getImgFormat()) // non-Blob
    {
        if ( ionFds.size() == 1 )
        {
            size_t sizeInBytes = 0;
            for (size_t i = 0; i < staticInfo.planes.size(); i++) {
                sizeInBytes += staticInfo.planes[i].sizeInBytes;
            }
            //
            HelperParamMapPA param;
            ::memset(&param, 0, sizeof(param));
            param.phyAddr   = 0;
            param.virAddr   = 0;
            param.ionFd     = ionFds[0];
            param.size      = sizeInBytes;
            if  (CC_UNLIKELY( ! helpMapPhyAddr(__FUNCTION__, param) )) {
                MY_LOGE("helpMapPhyAddr");
                return  MFALSE;
            }
            //
            MINTPTR pa = param.phyAddr;
            for (size_t i = 0; i < getPlaneCount(); i++)
            {
                sp<MyHeapInfo> pHeapInfo = new MyHeapInfo;
                mvHeapInfo.push_back(pHeapInfo);
                pHeapInfo->heapID = ionFds[0];
                //
                sp<BufInfo> pBufInfo = new BufInfo;
                mvBufInfo.push_back(pBufInfo);
                pBufInfo->stridesInBytes= mBufferParams.stride > 0 ? mBufferParams.stride : staticInfo.planes[i].rowStrideInBytes;
                pBufInfo->sizeInBytes   = staticInfo.planes[i].sizeInBytes;
                pBufInfo->pa            = pa;
                pa += pBufInfo->sizeInBytes;
                //
                rvBufInfo[i]->stridesInBytes= pBufInfo->stridesInBytes;
                rvBufInfo[i]->sizeInBytes   = pBufInfo->sizeInBytes;
            }
        }
        else
        if  ( ionFds.size() == getPlaneCount() )
        {
            MY_LOGD_IF(0, "getPlaneCount():%zu", getPlaneCount());
            for (size_t i = 0; i < getPlaneCount(); i++)
            {
                HelperParamMapPA param;
                ::memset(&param, 0, sizeof(param));
                param.phyAddr   = 0;
                param.virAddr   = 0;
                param.ionFd     = ionFds[i];
                param.size      = staticInfo.planes[i].sizeInBytes;
                if  (CC_UNLIKELY( ! helpMapPhyAddr(__FUNCTION__, param) )) {
                    MY_LOGE("helpMapPhyAddr");
                    return  MFALSE;
                }
                MINTPTR pa = param.phyAddr;
                //
                sp<MyHeapInfo> pHeapInfo = new MyHeapInfo;
                mvHeapInfo.push_back(pHeapInfo);
                pHeapInfo->heapID = ionFds[i];
                //
                sp<BufInfo> pBufInfo = new BufInfo;
                mvBufInfo.push_back(pBufInfo);
                pBufInfo->stridesInBytes= staticInfo.planes[i].rowStrideInBytes;
                pBufInfo->sizeInBytes   = staticInfo.planes[i].sizeInBytes;
                pBufInfo->pa            = pa;
                //
                rvBufInfo[i]->stridesInBytes= pBufInfo->stridesInBytes;
                rvBufInfo[i]->sizeInBytes   = pBufInfo->sizeInBytes;
            }
        }
        else
        {
            MY_LOGE("Unsupported ionFds:#%zu plane:#%zu", ionFds.size(), getPlaneCount());
            return MFALSE;
        }
    }
    else // Blob-format
    {
       MSize imgSize = getImgSize();

        if (ionFds.size() != 1)
        {
            MY_LOGE("!!err: Blob should have ionFds.size=1 but actual-size=%zu", ionFds.size());
            return MFALSE;
        }

        size_t sizeInBytes = imgSize.w * imgSize.h;

        rvBufInfo[0]->stridesInBytes = sizeInBytes;
        rvBufInfo[0]->sizeInBytes    = sizeInBytes;

        //
        HelperParamMapPA param;
        ::memset(&param, 0, sizeof(param));
        param.phyAddr   = 0;
        param.virAddr   = 0;
        param.ionFd     = ionFds[0];
        param.size      = sizeInBytes;
        if  (CC_UNLIKELY( ! helpMapPhyAddr(__FUNCTION__, param) )) {
            MY_LOGE("helpMapPhyAddr");
            return  MFALSE;
        }
        //
        MINTPTR pa = param.phyAddr;
        sp<MyHeapInfo> pHeapInfo = new MyHeapInfo;
        mvHeapInfo.push_back(pHeapInfo);
        pHeapInfo->heapID = ionFds[0];
        //
        sp<BufInfo> pBufInfo = new BufInfo;
        mvBufInfo.push_back(pBufInfo);
        pBufInfo->stridesInBytes= sizeInBytes;
        pBufInfo->sizeInBytes   = sizeInBytes;
        pBufInfo->pa            = pa;
        pa += pBufInfo->sizeInBytes;
        //
        rvBufInfo[0]->stridesInBytes= pBufInfo->stridesInBytes;
        rvBufInfo[0]->sizeInBytes   = pBufInfo->sizeInBytes;
    }

    //
    return  MTRUE;
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
GraphicImageBufferHeapBase::
impUninit(BufInfoVect_t const& /*rvBufInfo*/)
{
    Vector<int> const& ionFds = mGrallocInfo.ionFds;
    //
    if ( ionFds.size() == 1 )
    {
        size_t sizeInBytes = 0;
        for (size_t i = 0; i < mvBufInfo.size(); i++) {
            sizeInBytes += mvBufInfo[i]->sizeInBytes;
        }
        //
        HelperParamMapPA param;
        ::memset(&param, 0, sizeof(param));
        param.phyAddr   = mvBufInfo[0]->pa;
        param.virAddr   = 0;
        param.ionFd     = ionFds[0];
        param.size      = sizeInBytes;
        if  (CC_UNLIKELY( ! helpUnmapPhyAddr(__FUNCTION__, param) )) {
            MY_LOGE("helpUnmapPhyAddr");
        }
    }
    else
    if  ( ionFds.size() == getPlaneCount() )
    {
        for (size_t i = 0; i < getPlaneCount(); i++)
        {
            HelperParamMapPA param;
            ::memset(&param, 0, sizeof(param));
            param.phyAddr   = mvBufInfo[i]->pa;
            param.virAddr   = 0;
            param.ionFd     = ionFds[i];
            param.size      = mvBufInfo[i]->sizeInBytes;
            if  (CC_UNLIKELY( ! helpUnmapPhyAddr(__FUNCTION__, param) )) {
                MY_LOGE("helpUnmapPhyAddr");
            }
        }
    }
    else
    {
        MY_LOGE("Unsupported ionFds:#%zu plane:#%zu", ionFds.size(), getPlaneCount());
    }
    //
    mvBufInfo.clear();
    mvHeapInfo.clear();
    //
    return  MTRUE;
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
GraphicImageBufferHeapBase::
impLockBuf(
    char const* szCallerName,
    MINT usage,
    BufInfoVect_t const& rvBufInfo
)
{
    void* vaddr = nullptr;
    status_t status = IGrallocHelper::singleton()->lock(
                            mBufferParams.handle, mBufferParams.usage,
                            mBufferParams.width, mBufferParams.height, &vaddr
                            );
    if  (CC_UNLIKELY( OK != status )) {
        MY_LOGE(
            "%s IGrallocHelper::lock - status:%d(%s) usage:(Req:%#x|Config:%#x) format:%#x",
            szCallerName, status, ::strerror(-status),
            usage, mBufferParams.usage, mBufferParams.format
        );
        return MFALSE;
    }
    //
    //  SW Access.
    if  ( 0 != (usage & (eBUFFER_USAGE_SW_MASK)) ) {
        MY_LOGF_IF( 0==vaddr, "SW Access but va=0 - usage:(Req:%#x|Config:%#x) format:%#x",
                    usage, mBufferParams.usage, mBufferParams.format);
        MY_LOGF_IF( 1<mGrallocInfo.ionFds.size(), "[Not Implement] ionFds:#%zu>1",
                    mGrallocInfo.ionFds.size());
    }
    MINTPTR va = reinterpret_cast<MINTPTR>(vaddr);
    for (size_t i = 0; i < getPlaneCount(); i++)
    {
        //  SW Access.
        if  ( 0 != (usage & (eBUFFER_USAGE_SW_MASK)) ) {
            rvBufInfo[i]->va = va;
            va += mvBufInfo[i]->sizeInBytes;
        }
        //
        //  HW Access
        if  ( 0 != (usage & (~eBUFFER_USAGE_SW_MASK)) ) {
            rvBufInfo[i]->pa = mvBufInfo[i]->pa;
        }
    }
    //
    return MTRUE;
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
GraphicImageBufferHeapBase::
impUnlockBuf(
    char const* szCallerName,
    MINT usage,
    BufInfoVect_t const& rvBufInfo
)
{
    for (size_t i = 0; i < getPlaneCount(); i++)
    {
        rvBufInfo[i]->va = 0;
        rvBufInfo[i]->pa = 0;
    }
    //
    status_t status = IGrallocHelper::singleton()->unlock(mBufferParams.handle);
    MY_LOGW_IF(
        OK!=status,
        "%s GraphicBuffer::unlock - status:%d(%s) usage:%#x",
        szCallerName, status, ::strerror(-status), usage
    );
    //
    return  MTRUE;
}


/******************************************************************************
 *
 ******************************************************************************/
SecureGraphicImageBufferHeap::SecureGraphicImageBufferHeap(
        char const* szCallerName,
        BufferParams& rBufParams,
        ANativeWindowBuffer* pANWBuffer,
        MINT const acquire_fence,
        MINT const release_fence)
    : GraphicImageBufferHeapBase(
            szCallerName, rBufParams, pANWBuffer, acquire_fence, release_fence)
{
    getSecureHandle(mSecureHandle);
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL SecureGraphicImageBufferHeap::impLockBuf(
        char const* szCallerName,
        MINT usage,
        BufInfoVect_t const& rvBufInfo)
{
    for (size_t i = 0; i < getPlaneCount(); i++)
    {
        if ((usage & eBUFFER_USAGE_SW_MASK) > 0)
            rvBufInfo[i]->va = mSecureHandle;

        if ((usage & eBUFFER_USAGE_HW_MASK) > 0)
            rvBufInfo[i]->pa = mSecureHandle;
    }

    return MTRUE;
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL SecureGraphicImageBufferHeap::impUnlockBuf(
        char const* szCallerName,
        MINT usage,
        BufInfoVect_t const& rvBufInfo)
{
    for (size_t i = 0; i < getPlaneCount(); i++)
    {
        rvBufInfo[i]->va = 0;
        rvBufInfo[i]->pa = 0;
    }

    return MTRUE;
}


/******************************************************************************
 *
 ******************************************************************************/
void SecureGraphicImageBufferHeap::getSecureHandle(SECHAND& secureHandle)
{
    buffer_handle_t bufferHandle(mBufferParams.handle);
    MY_LOGA_IF(!bufferHandle, "invalid buffer handle");
    int retval = gralloc_extra_query(bufferHandle,
            GRALLOC_EXTRA_ATTRIBUTE_QUERY::GRALLOC_EXTRA_GET_SECURE_HANDLE,
            reinterpret_cast<void *>(&secureHandle));
    MY_LOGA_IF(retval != GRALLOC_EXTRA_OK,
            "get secure handle failed: FD(%u)", bufferHandle->data[0]);
}
