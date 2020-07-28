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

#define LOG_TAG "MtkCam/RecordClient"
//
#include <cutils/atomic.h>
#include <linux/ion_drv.h>  // for ION_CMDS, ION_CACHE_SYNC_TYPE
#include <libion_mtk/include/ion.h>
#include <linux/ioctl.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <ion/ion.h>
//
#include <mtkcam/utils/fwk/MtkCameraParameters.h>
#include <mtkcam/utils/fwk/MtkCameraParameters.h>
//
#include "../MyUtils.h"
using namespace android;
using namespace MtkCamUtils;
//
#include "RecBufManager.h"
//
using namespace NSCamClient;
using namespace NSRecordClient;
//
/******************************************************************************
*
*******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_LOGV("(%d)(%s)[RecBufManager::%s] " fmt, ::gettid(), getName(), __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("(%d)(%s)[RecBufManager::%s] " fmt, ::gettid(), getName(), __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("(%d)(%s)[RecBufManager::%s] " fmt, ::gettid(), getName(), __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("(%d)(%s)[RecBufManager::%s] " fmt, ::gettid(), getName(), __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("(%d)(%s)[RecBufManager::%s] " fmt, ::gettid(), getName(), __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("(%d)(%s)[RecBufManager::%s] " fmt, ::gettid(), getName(), __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("(%d)(%s)[RecBufManager::%s] " fmt, ::gettid(), getName(), __FUNCTION__, ##arg)
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
RecImgBuf*
RecImgBuf::
alloc(
    camera_request_memory   requestMemory,
    sp<RecImgInfo>&         rpImgInfo,
    void*                   user
)
{
    bool ret = true;
    RecImgBuf* pRecImgBuf = NULL;
    camera_memory_t* camera_memory = NULL;
    int32_t IonDevFd = -1,IonBufFd = -1, IonBufFlag = 0,ImgBufSize = 0;
    ion_user_handle_t IonHandle = (int)NULL;
    sp<GraphicBuffer> spGraphicBuffer = NULL;
    //
    if(rpImgInfo->mbMetaMode)
    {
        //(1) create gralloc buffer
        CAM_LOGD("[RecImgBuf::alloc][GraphicBuffer] Size(%d x %d), F(0x%X), BU(0x%X)",
                rpImgInfo->mu4ImgWidth,
                rpImgInfo->mu4ImgHeight,
                rpImgInfo->mu4ImgFormat,
                rpImgInfo->mu4BufUsage);
        spGraphicBuffer = new GraphicBuffer(rpImgInfo->mu4ImgWidth,
                                            rpImgInfo->mu4ImgHeight,
                                            rpImgInfo->mu4ImgFormat,
                                            rpImgInfo->mu4BufUsage);


        // (2) create camera_memory buffer
        //* --------------------------------------------------------------
        //* |  kMetadataBufferTypeGrallocSource | sizeof(buffer_handle_t) |
        //* --------------------------------------------------------------
        ImgBufSize = sizeof(int)+sizeof(buffer_handle_t);
    }
    else
    {
        ImgBufSize = rpImgInfo->mImgBufSize;
        IonBufFlag = ION_FLAG_CACHED | ION_FLAG_CACHED_NEEDS_SYNC;
    }
    //
    //
    CAM_LOGD("[RecImgBuf::alloc][mt_ion_open]");
    IonDevFd = mt_ion_open(LOG_TAG);
    if(IonDevFd < 0)
    {
        CAM_LOGE("[RecImgBuf::alloc]ion_open fail");
        goto lbExit;
    }
    //
    CAM_LOGD("[RecImgBuf::alloc][ion_alloc_mm]IonDevFd(%d), ImgBufSize(%d)",
            IonDevFd,
            ImgBufSize);
    if(ion_alloc_mm(
        IonDevFd,
        ImgBufSize,
        32,
        IonBufFlag,
        &IonHandle))
    {
        CAM_LOGE("[RecImgBuf::alloc]ion_alloc_mm fail");
        goto lbExit;
    }
    //
    CAM_LOGD("[RecImgBuf::alloc][ion_share]");
    if(ion_share(
        IonDevFd,
        IonHandle,
        &IonBufFd))
    {
        CAM_LOGE("[RecImgBuf::alloc]ion_share fail");
        goto lbExit;
    }
    //
    CAM_LOGD("[RecImgBuf::alloc][requestMemory] fd(%d), S(%d)", IonBufFd, ImgBufSize);
    camera_memory = requestMemory(IonBufFd, ImgBufSize, 1, user);
    if  ( ! camera_memory )
    {
        ret = false;
        CAM_LOGE("[RecImgBuf::alloc][requestMemory] id:%d, size:%d", IonBufFd, ImgBufSize);
        goto lbExit;
    }
    //
    //
    pRecImgBuf = new RecImgBuf(
                            *camera_memory,
                            rpImgInfo,
                            IonDevFd,
                            IonBufFd,
                            IonHandle,
                            spGraphicBuffer);
lbExit:
    return  ret == true ? pRecImgBuf : NULL;
}


/******************************************************************************
 *
 ******************************************************************************/
void
RecImgBuf::
configPhyAddr()
{
    if(mpBufPA == NULL)
    {
        ion_user_handle_t IonHandle = (int)NULL;
        struct ion_sys_data sys_data;
        //
        if(ion_import(mIonDevFd, mi4BufIonFd, &IonHandle))
        {
            CAM_LOGE("[RecImgBuf::configPhyAddr]ion_import fail, bufIonFd(0x%x)", mi4BufIonFd);
        }
        //
        sys_data.sys_cmd = ION_SYS_GET_PHYS;
        sys_data.get_phys_param.handle = IonHandle;
        if (ion_custom_ioctl(mIonDevFd, ION_CMD_SYSTEM, &sys_data))
        {
            CAM_LOGE("[RecImgBuf::configPhyAddr]IOCTL[ION_IOC_CUSTOM] Get Phys failed!\n");
        }
        else
        {
            mpBufPA = (void*)((MINTPTR)(sys_data.get_phys_param.phy_addr));
        }
        //
        CAM_LOGD("[RecImgBuf::configPhyAddr]Addr(%p)",mpBufPA);
        //
        if(ion_free(mIonDevFd, IonHandle))
        {
            CAM_LOGE("[RecImgBuf::configPhyAddr]ion_free fail");
        }
    }
}


/******************************************************************************
 *
 ******************************************************************************/
void
RecImgBuf::
setGrallocBuffer(sp<GraphicBuffer>& pGraphicBuffer)
{
    mpGraphicBuffer = pGraphicBuffer;
    //
    #if 0 //For imporve performance, we use setIonFd() and setPhyAddr() without query by new handle.
    if(mpImgInfo->mbMetaMode)
    {
        gralloc_extra_query(mpGraphicBuffer->handle, GRALLOC_EXTRA_GET_ION_FD, &mi4BufIonFd);
        if ( mi4BufIonFd < 0 )
        {
            CAM_LOGE("[RecImgBuf::RecImgBuf] fd:%d <= 0", mi4BufIonFd);
        }
    }
    //
    ion_user_handle_t IonHandle = NULL;
    struct ion_sys_data sys_data;
    //
    if(ion_import(mIonDevFd, mi4BufIonFd, &IonHandle))
    {
        CAM_LOGE("[RecImgBuf::configPhyAddr]ion_import fail, bufIonFd(0x%x)", mi4BufIonFd);
    }
    //
    sys_data.sys_cmd = ION_SYS_GET_PHYS;
    sys_data.get_phys_param.handle = IonHandle;
    if (ion_custom_ioctl(mIonDevFd, ION_CMD_SYSTEM, &sys_data))
    {
        CAM_LOGE("[RecImgBuf::configPhyAddr]IOCTL[ION_IOC_CUSTOM] Get Phys failed!\n");
    }
    else
    {
        mpBufPA = (void*)((MINTPTR)(sys_data.get_phys_param.phy_addr));
    }
    //
    if(ion_free(mIonDevFd, IonHandle))
    {
        CAM_LOGE("[RecImgBuf::configPhyAddr]ion_free fail");
    }
    #endif
}


/******************************************************************************
 *
 ******************************************************************************/
RecImgBuf::
RecImgBuf(
    camera_memory_t const&      rCamMem,
    sp<RecImgInfo>&             rpImgInfo,
    int32_t                     IonDevFd,
    int32_t                     IonBufFd,
    ion_user_handle_t           IonHandle,
    sp<GraphicBuffer>           spGraphicBuffer
)
    : ICameraImgBuf()
    , mpImgInfo(rpImgInfo)
    , mi8Timestamp(0)
    , mCamMem(rCamMem)
    , mIonDevFd(IonDevFd)
    , mIonBufFd(IonBufFd)
    , mIonHandle(IonHandle)
    , mpGraphicBuffer(spGraphicBuffer)
    , mu4BufUsage(0)
    , mi4BufIonFd(-1)
    , mpBufVA(NULL)
    , mpBufPA(NULL)
{
    if(mpImgInfo->mbMetaMode)
    {
        //CAM_LOGD("[RecImgBuf::RecImgBuf]gralloc on");
        //
        //(.1) query image real format
        gralloc_extra_query(mpGraphicBuffer->handle, GRALLOC_EXTRA_GET_FORMAT, &mpImgInfo->mu4ImgFormat);

        //(.2) query buffer stride
        if( mpImgInfo->mu4ImgFormat == HAL_PIXEL_FORMAT_YV12 ||
            mpImgInfo->mu4ImgFormat == HAL_PIXEL_FORMAT_I420)
        {
            int UStrideMask = 15;
            int VStrideMask = 15;  //because of no way to get U and V stride
            mpImgInfo->mu4ImgStrides[0] = mpGraphicBuffer->getStride();
            mpImgInfo->mu4ImgStrides[1] = ((~UStrideMask) & (UStrideMask + (rpImgInfo->mu4ImgWidth >> 1)));
            mpImgInfo->mu4ImgStrides[2] = ((~VStrideMask) & (VStrideMask + (rpImgInfo->mu4ImgWidth >> 1)));

            //(.3) calculate buffer size
            mpImgInfo->mImgBufSize = mpImgInfo->mu4ImgStrides[0] * (rpImgInfo->mu4ImgHeight);
            mpImgInfo->mImgBufSize += mpImgInfo->mu4ImgStrides[1] * (rpImgInfo->mu4ImgHeight >> 1);
            mpImgInfo->mImgBufSize += mpImgInfo->mu4ImgStrides[2] * (rpImgInfo->mu4ImgHeight >> 1);

            //(.4)
            mpImgInfo->mu4BitsPerPixel = 12;
        }
        else
        {
            //TODO
        }
        //(.5) query buffer ion fd
        gralloc_extra_query(mpGraphicBuffer->handle, GRALLOC_EXTRA_GET_ION_FD, &mi4BufIonFd);
        if ( mi4BufIonFd < 0 )
        {
            CAM_LOGE("[RecImgBuf::RecImgBuf] fd:%d <= 0", mi4BufIonFd);
        }
        //
        if(mpImgInfo->mu4ImgFormat == HAL_PIXEL_FORMAT_YV12)
        {
            mpImgInfo->ms8ImgFormat = CameraParameters::PIXEL_FORMAT_YUV420P;
        }
        else
        if(mpImgInfo->mu4ImgFormat == HAL_PIXEL_FORMAT_I420)
        {
            mpImgInfo->ms8ImgFormat = MtkCameraParameters::PIXEL_FORMAT_YUV420I;
        }
        else
        {
            //TODO
        }
        //
        CAM_LOGD("[RecImgBuf::RecImgBuf][MemInfo] hdl(%p), fd(%d), mem(%p), VA(%p), S(%d), W(%d), H(%d), F(%s), fd(%d), stride{%d, %d, %d}, S(%d)",
                mpGraphicBuffer->handle,
                mIonBufFd,
                (uint32_t*)&mCamMem,
                (uint32_t*)mCamMem.data,
                (int)mCamMem.size,
                getImgWidth(),
                getImgHeight(),
                getImgFormat().string(),
                getIonFd(),
                getImgWidthStride(0),
                getImgWidthStride(1),
                getImgWidthStride(2),
                (int)getBufSize());
    }
    else
    {
        CAM_LOGD("[RecImgBuf::RecImgBuf]gralloc off");
    }


}


/******************************************************************************
 *
 ******************************************************************************/
RecImgBuf::
~RecImgBuf()
{
    struct ion_handle_data IonHandleData;
    //
    CAM_LOGD(
        "[RecImgBuf::~RecImgBuf]"
        "Name(%s),ION(%d),Addr(%p/%p),Size(%d),Fmt(%s),Str(%d),W(%d),H(%d),BPP(%d),TS(%lld)",
        getBufName(),
        getIonFd(),
        getVirAddr(),
        getPhyAddr(),
        (int)getBufSize(),
        getImgFormat().string(),
        getImgWidthStride(),
        getImgWidth(),
        getImgHeight(),
        getBitsPerPixel(),
        (long long)getTimestamp()
    );

    //
    //
    if(mIonDevFd >= 0)
    {
        if(mIonBufFd >= 0)
        {
            ion_share_close(
                mIonDevFd,
                mIonBufFd);
        }
        //
        ion_free(
            mIonDevFd,
            mIonHandle);
        //
        ion_close(mIonDevFd);
    }
    if  ( mCamMem.release )
    {
        mCamMem.release(&mCamMem);
        mCamMem.release = NULL;
    }
    //
    if(mpImgInfo->mbMetaMode)
    {
        if (mpGraphicBuffer != 0)
        {
            mpGraphicBuffer = 0;
        }
    }
}


/******************************************************************************
 *
 ******************************************************************************/
void
RecImgBuf::
dump() const
{
    CAM_LOGD(
        "[RecImgBuf::dump]"
        "Name(%s),ION(%d),Addr(%p/%p),Size(%d),Fmt(%s),Str(%d),W(%d),H(%d),BPP(%d),TS(%lld),S/C(%d/%d)",
        getBufName(),
        getIonFd(),
        getVirAddr(),
        getPhyAddr(),
        (int)getBufSize(),
        getImgFormat().string(),
        getImgWidthStride(),
        getImgWidth(),
        getImgHeight(),
        getBitsPerPixel(),
        (long long)getTimestamp(),
        getBufSecu(),
        getBufCohe()
    );
}


/******************************************************************************
 *
 ******************************************************************************/
RecBufManager*
RecBufManager::
alloc(
    bool const              bMetaMode,
    char const*const        szImgFormat,
    uint32_t const          u4BufUsage,
    uint32_t const          u4ImgFormat,
    uint32_t const          u4ImgWidth,
    uint32_t const          u4ImgHeight,
    uint32_t const          u4ImgStrides[3],
    uint32_t const          u4BufCount,
    char const*const        szName,
    camera_request_memory   requestMemory,
    int32_t const           i4BufSecu,
    int32_t const           i4BufCohe,
    void*                   user
)
{
    RecBufManager* pMgr = new RecBufManager(
                                bMetaMode,
                                szImgFormat,
                                u4BufUsage,
                                u4ImgFormat,
                                u4ImgWidth,
                                u4ImgHeight,
                                u4ImgStrides,
                                u4BufCount,
                                szName,
                                requestMemory,
                                i4BufSecu,
                                i4BufCohe);
    //
    if  ( pMgr && ! pMgr->init(user) )
    {
        // return NULL due to init failure.
        pMgr = NULL;
    }
    //
    return pMgr;
}


/******************************************************************************
 *
 ******************************************************************************/
int32_t RecBufManager::
allocateExtraBuffer(void* user)
{
    int32_t index;
    index = this->doAllocate(user);
    if( index >= 0 )
    {
        ++mu4BufCount;
    }
    return index;
}


/******************************************************************************
 *
 ******************************************************************************/
RecBufManager::
RecBufManager(
    bool const              bMetaMode,
    char const*const        szImgFormat,
    uint32_t const          u4BufUsage,
    uint32_t const          u4ImgFormat,
    uint32_t const          u4ImgWidth,
    uint32_t const          u4ImgHeight,
    uint32_t const          u4ImgStrides[3],
    uint32_t const          u4BufCount,
    char const*const        szName,
    camera_request_memory   requestMemory,
    int32_t const           i4BufSecu,
    int32_t const           i4BufCohe
)
    : RefBase()
    //
    , mbMetaMode(bMetaMode)
    , ms8Name(szName)
    , ms8ImgFormat(szImgFormat)
    , mu4BufUsage(u4BufUsage)
    , mu4ImgFormat(u4ImgFormat)
    , mu4ImgWidth(u4ImgWidth)
    , mu4ImgHeight(u4ImgHeight)
    , mu4ImgStrides
    { u4ImgStrides[0]
     ,u4ImgStrides[1]
     ,u4ImgStrides[2]}
    , mu4BufCount(u4BufCount)
    //
    , mvImgBuf()
    , mRequestMemory(requestMemory)
    //
    , mi4BufSecu(i4BufSecu)
    , mi4BufCohe(i4BufCohe)
    //
{
    MY_LOGD("");
}


/******************************************************************************
 *
 ******************************************************************************/
RecBufManager::
~RecBufManager()
{
    uninit();
    //
    MY_LOGD("");
}


/******************************************************************************
 *
 ******************************************************************************/
bool
RecBufManager::
init(void* user)
{
    MY_LOGD("+ mu4BufCount(%d)", mu4BufCount);
    //
    mvImgBuf.clear();
    mvImgBufIonFd.clear();
    for (size_t i = 0; i < mu4BufCount; i++)
    {
        if ( this->doAllocate(user) < 0 )
        {
            MY_LOGE("doAllocate fail[%d]", i);
            goto lbExit;
        }
    }
    //
    //
    MY_LOGD("- ret(1)");
    return true;
lbExit:
    MY_LOGD("- ret(0)");
    uninit();
    return false;
}


/******************************************************************************
 *
 ******************************************************************************/
void
RecBufManager::
uninit()
{
    MY_LOGD("+ mvImgBuf.size(%d), mu4BufCount(%d)", (int)mvImgBuf.size(), mu4BufCount);
    //
    for (size_t i = 0; i < mvImgBuf.size(); i++)
    {
        mvImgBuf.editItemAt(i) = NULL;
    }
    //
    mvImgBufIonFd.clear();
    //
    MY_LOGD("-");
}


/******************************************************************************
 *
 ******************************************************************************/
int32_t RecBufManager::
doAllocate(void* user)
{
    int32_t index = mvImgBuf.size();
    int32_t ionFd = 0;
    sp<RecImgInfo> spRecImgInfo = new RecImgInfo(
                                        mbMetaMode,
                                        mu4BufUsage,
                                        mu4ImgFormat,
                                        mu4ImgWidth,
                                        mu4ImgHeight,
                                        mu4ImgStrides,
                                        ms8ImgFormat,
                                        ms8Name,
                                        mi4BufSecu,
                                        mi4BufCohe);
    RecImgBuf* pRecImgBuf = RecImgBuf::alloc(
        mRequestMemory,
        spRecImgInfo,
        user
    );

    if  ( pRecImgBuf == 0 )
    {
        MY_LOGE("cannot allocate pRecImgBuf [%d]", index);
        return -1;
    }
    pRecImgBuf->dump();

    ionFd = pRecImgBuf->getIonFd();
    if  ( ionFd == 0 )
    {
        MY_LOGE("cannot allocate mvImgBufIonFd[%d]", index);
        return -1;
    }

    if( index != mvImgBuf.size() || mvImgBuf.size() != mvImgBufIonFd.size() )
    {
        MY_LOGE("cannot allocate[%d]: size mistach mvImgBuf(%d) mvImgBufIonFd(%d)", index, mvImgBuf.size(), mvImgBufIonFd.size());
        return -1;
    }

    mvImgBuf.push_back(pRecImgBuf);
    mvImgBufIonFd.push_back(ionFd);

    return index;
}

