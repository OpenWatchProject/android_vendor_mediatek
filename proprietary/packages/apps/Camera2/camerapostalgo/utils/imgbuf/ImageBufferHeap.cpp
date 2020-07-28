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
#define LOG_TAG "Postalgo/Cam1Heap"
//
#include "MyUtils.h"
#include "BaseImageBufferHeap.h"
#include <utils/imgbuf/ImageBufferHeap.h>
//
using namespace android;
using namespace NSCam;
using namespace NSCam::Utils;
//


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
 *
 ******************************************************************************/
#define GET_BUF_VA(plane, va, index)                   (plane >= (index+1)) ? va : 0
#define GET_BUF_ID(plane, memID, index)                (plane >= (index+1)) ? memID : 0


/******************************************************************************
 *  Image Buffer Heap (Camera1).
 ******************************************************************************/
class ImageBufferHeapImpl : public ImageBufferHeap
                          , public NSImageBufferHeap::BaseImageBufferHeap
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Creation.
    static  ImageBufferHeapImpl*    create(
                                        char const* szCallerName,
                                        ImgParam_t const& rImgParam,
                                        PortBufInfo_v1 const& rPortBufInfo,
                                        MBOOL const enableLog = MTRUE
                                    );

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  BaseImageBufferHeap Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////
    virtual char const*             impGetMagicName()                   const   { return magicName(); }
    virtual void*                   impGetMagicInstance()               const   { return (void *)this; }
    virtual HeapInfoVect_t const&   impGetHeapInfo()                    const   { return mvHeapInfo; }

    virtual MBOOL                   impInit(BufInfoVect_t const& rvBufInfo);
    virtual MBOOL                   impUninit(BufInfoVect_t const& rvBufInfo);
    virtual MBOOL                   impReconfig(BufInfoVect_t const& rvBufInfo) { return MFALSE; }

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

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////
    virtual MBOOL                   doMapPhyAddr(char const* szCallerName, HeapInfo const& rHeapInfo, BufInfo& rBufInfo);
    virtual MBOOL                   doUnmapPhyAddr(char const* szCallerName, HeapInfo const& rHeapInfo, BufInfo& rBufInfo);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Instantiation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////                    Destructor/Constructors.

    /**
     * Disallowed to directly delete a raw pointer.
     */
    virtual                         ~ImageBufferHeapImpl() {}
                                    ImageBufferHeapImpl(
                                        char const* szCallerName,
                                        ImgParam_t const& rImgParam,
                                        PortBufInfo_v1 const& rPortBufInfo
                                    );

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Data Members.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////                    Info to Allocate.
    size_t                          mBufStridesInBytesToAlloc[3];  // buffer strides in bytes.

protected:  ////                    Info of Allocated Result.
    PortBufInfo_v1                  mPortBufInfo;   //
    HeapInfoVect_t                  mvHeapInfo;     //
    BufInfoVect_t                   mvBufInfo;      //

};


/******************************************************************************
 *
 ******************************************************************************/
ImageBufferHeap*
ImageBufferHeap::
create(
    char const* szCallerName,
    ImgParam_t const& rImgParam,
    PortBufInfo_v1 const& rPortBufInfo,
    MBOOL const enableLog
)
{
    return ImageBufferHeapImpl::create(szCallerName, rImgParam, rPortBufInfo, enableLog);
}


/******************************************************************************
 *
 ******************************************************************************/
ImageBufferHeapImpl*
ImageBufferHeapImpl::
create(
    char const* szCallerName,
    ImgParam_t const& rImgParam,
    PortBufInfo_v1 const& rPortBufInfo,
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
    ImageBufferHeapImpl* pHeap = NULL;
    pHeap = new ImageBufferHeapImpl(szCallerName, rImgParam, rPortBufInfo);
    if  ( ! pHeap )
    {
        CAM_LOGE("Fail to new");
        return NULL;
    }
    //
    if  ( ! pHeap->onCreate(rImgParam.imgSize, rImgParam.imgFormat,
                rImgParam.bufSize, rPortBufInfo.secType, enableLog) )
    {
        CAM_LOGE("onCreate");
        delete pHeap;
        return NULL;
    }
    //
    return pHeap;
}


/******************************************************************************
 *
 ******************************************************************************/
ImageBufferHeapImpl::
ImageBufferHeapImpl(
    char const* szCallerName,
    ImgParam_t const& rImgParam,
    PortBufInfo_v1 const& rPortBufInfo
)
    : BaseImageBufferHeap(szCallerName)
    //
    , mPortBufInfo(rPortBufInfo)
    , mvHeapInfo()
    , mvBufInfo()
    //
{
    ::memcpy(mBufStridesInBytesToAlloc, rImgParam.bufStridesInBytes, sizeof(mBufStridesInBytesToAlloc));
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
ImageBufferHeapImpl::
impInit(BufInfoVect_t const& rvBufInfo)
{
    MBOOL ret = MFALSE;
    MUINT32 planesSizeInBytes = 0;    // for calculating n-plane va
    //
    MY_LOGD_IF(getLogCond(), "continuos(%d) plane(%zu), memID(0x%x/0x%x/0x%x), va(0x%" PRIxPTR "/0x%" PRIxPTR "/0x%" PRIxPTR ")",
            mPortBufInfo.continuos, getPlaneCount(), GET_BUF_ID(getPlaneCount(), mPortBufInfo.memID[0], 0),
            (mPortBufInfo.continuos) ? 0 : GET_BUF_ID(getPlaneCount(), mPortBufInfo.memID[1], 1),
            (mPortBufInfo.continuos) ? 0 : GET_BUF_ID(getPlaneCount(), mPortBufInfo.memID[2], 2),
            GET_BUF_VA(getPlaneCount(), mPortBufInfo.virtAddr[0], 0),
            (mPortBufInfo.continuos) ? 0 : GET_BUF_VA(getPlaneCount(), mPortBufInfo.virtAddr[1], 1),
            (mPortBufInfo.continuos) ? 0 : GET_BUF_VA(getPlaneCount(), mPortBufInfo.virtAddr[2], 2)
            );
    mvHeapInfo.setCapacity(getPlaneCount());
    mvBufInfo.setCapacity(getPlaneCount());
    for (MUINT32 i = 0; i < getPlaneCount(); i++)
    {
        if  ( ! helpCheckBufStrides(i, mBufStridesInBytesToAlloc[i]) )
        {
            goto lbExit;
        }
        //
        {
            sp<HeapInfo> pHeapInfo = new HeapInfo;
            mvHeapInfo.push_back(pHeapInfo);
            pHeapInfo->heapID = ( MTRUE == mPortBufInfo.continuos ) ? mPortBufInfo.memID[0] : mPortBufInfo.memID[i];
            //
            sp<BufInfo> pBufInfo = new BufInfo;
            mvBufInfo.push_back(pBufInfo);
            pBufInfo->stridesInBytes = mBufStridesInBytesToAlloc[i];
            pBufInfo->sizeInBytes = helpQueryBufSizeInBytes(i, mBufStridesInBytesToAlloc[i]);
            pBufInfo->va = ( MTRUE == mPortBufInfo.continuos )
                            ? mPortBufInfo.virtAddr[0] + planesSizeInBytes
                            : mPortBufInfo.virtAddr[i];
            //
            planesSizeInBytes += pBufInfo->sizeInBytes;
            //
            rvBufInfo[i]->stridesInBytes = pBufInfo->stridesInBytes;
            rvBufInfo[i]->sizeInBytes = pBufInfo->sizeInBytes;
        }
    }
    //
    ret = MTRUE;
lbExit:
    //MY_LOGD_IF(getLogCond(), "- ret:%d", ret);
    return  ret;
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
ImageBufferHeapImpl::
impUninit(BufInfoVect_t const& /*rvBufInfo*/)
{
    //
    //MY_LOGD_IF(getLogCond(), "-");
    return  MTRUE;
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
ImageBufferHeapImpl::
impLockBuf(
    char const* szCallerName,
    MINT usage,
    BufInfoVect_t const& rvBufInfo
)
{
    MBOOL ret = MFALSE;
    MUINT32 planesSizeInBytes = 0;    // for calculating n-plane pa
    //
    for (MUINT32 i = 0; i < rvBufInfo.size(); i++)
    {
        sp<HeapInfo> pHeapInfo = mvHeapInfo[i];
        sp<BufInfo> pBufInfo = rvBufInfo[i];
        //
        //  SW Access.
        pBufInfo->va = ( 0 != (usage & eBUFFER_USAGE_SW_MASK) ) ? mvBufInfo[i]->va : 0;
        //
        //  HW Access.
        if  ( 0 != (usage & eBUFFER_USAGE_HW_MASK) )
        {
            if  ( ! doMapPhyAddr(szCallerName, *pHeapInfo, *pBufInfo) )
            {
                MY_LOGE("%s@ doMapPhyAddr at %d-th plane", szCallerName, i);
                goto lbExit;
            }
            if ( ( i > 0 )
                && ( mvHeapInfo[i]->heapID != -1 )  // ion fd
                && ( mvHeapInfo[i]->heapID == mvHeapInfo[i-1]->heapID ) // continuous memory
                )
            {
                planesSizeInBytes += mvBufInfo[i-1]->sizeInBytes;
                pBufInfo->pa += planesSizeInBytes;
            }
        }
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
ImageBufferHeapImpl::
impUnlockBuf(
    char const* szCallerName,
    MINT usage,
    BufInfoVect_t const& rvBufInfo
)
{
    //
    for (MUINT32 i = 0; i < rvBufInfo.size(); i++)
    {
        sp<HeapInfo> pHeapInfo = mvHeapInfo[i];
        sp<BufInfo> pBufInfo = rvBufInfo[i];
        //
        //  HW Access.
        if  ( 0 != (usage & eBUFFER_USAGE_HW_MASK) )
        {
            if  ( 0 != pBufInfo->pa ) {
                doUnmapPhyAddr(szCallerName, *pHeapInfo, *pBufInfo);
                pBufInfo->pa = 0;
            }
            else {
                MY_LOGW("%s@ skip PA=0 at %d-th plane", szCallerName, i);
            }
        }
        //
        //  SW Access.
        if  ( 0 != (usage & eBUFFER_USAGE_SW_MASK) )
        {
            if  ( 0 != pBufInfo->va ) {
                pBufInfo->va = 0;
            }
            else {
                MY_LOGW("%s@ skip VA=0 at %d-th plane", szCallerName, i);
            }
        }
    }
    //
    return  MTRUE;
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
ImageBufferHeapImpl::
doMapPhyAddr(char const* szCallerName, HeapInfo const& rHeapInfo, BufInfo& rBufInfo)
{
    HelperParamMapPA param;
    param.phyAddr   = 0;
    param.virAddr   = rBufInfo.va;
    param.ionFd     = rHeapInfo.heapID;
    param.size      = rBufInfo.sizeInBytes;
    param.security  = mPortBufInfo.security;
    param.coherence = mPortBufInfo.coherence;

    // when secure path rBufInfo.va would be secure_handle
    if (CC_UNLIKELY(mPortBufInfo.security)) {
        MY_LOGW("DEPRECATED, please use ISecureImageBufferHeap to " \
                "allocate/manage secure buffer");
        rBufInfo.pa = rBufInfo.va;
        return MTRUE;
    }

    if  ( ! helpMapPhyAddr(szCallerName, param) )
    {
        MY_LOGE("helpMapPhyAddr");
        return  MFALSE;
    }
    //
    rBufInfo.pa = param.phyAddr;
    //
    return  MTRUE;
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
ImageBufferHeapImpl::
doUnmapPhyAddr(char const* szCallerName, HeapInfo const& rHeapInfo, BufInfo& rBufInfo)
{
    HelperParamMapPA param;
    param.phyAddr   = rBufInfo.pa;
    param.virAddr   = rBufInfo.va;
    param.ionFd     = rHeapInfo.heapID;
    param.size      = rBufInfo.sizeInBytes;
    param.security  = mPortBufInfo.security;
    param.coherence = mPortBufInfo.coherence;

    // when secure path no need to unmapPhyAddress
    if (CC_UNLIKELY(mPortBufInfo.security)) {
        MY_LOGW("DEPRECATED, please use ISecureImageBufferHeap to " \
                "allocate/manage secure buffer");
        rBufInfo.pa = 0;
        return MTRUE;
    }

    if  ( ! helpUnmapPhyAddr(szCallerName, param) )
    {
        MY_LOGE("helpUnmapPhyAddr");
        return  MFALSE;
    }
    //
    rBufInfo.pa = 0;
    //
    return  MTRUE;
}

