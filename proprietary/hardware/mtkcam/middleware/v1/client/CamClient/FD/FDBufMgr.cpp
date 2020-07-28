#define LOG_TAG "MtkCam/CamClient/FDClient"
//
#include "../MyUtils.h"
using namespace android;
using namespace MtkCamUtils;
//
#include <stdlib.h>
//
#include "FDBufMgr.h"
//
#include <cutils/atomic.h>
//
/******************************************************************************
*
*******************************************************************************/
#include <mtkcam/utils/std/Log.h>
#define MY_LOGV(fmt, arg...)        CAM_LOGV("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, arg...)    if (cond) { MY_LOGV(arg); }
#define MY_LOGD_IF(cond, arg...)    if (cond) { MY_LOGD(arg); }
#define MY_LOGI_IF(cond, arg...)    if (cond) { MY_LOGI(arg); }
#define MY_LOGW_IF(cond, arg...)    if (cond) { MY_LOGW(arg); }
#define MY_LOGE_IF(cond, arg...)    if (cond) { MY_LOGE(arg); }

#define FUNCTION_IN                 MY_LOGD("+")
#define FUNCTION_OUT                MY_LOGD("-")

/******************************************************************************
*
*******************************************************************************/
void
FDBuffer::
createBuffer()
{
    FUNCTION_IN;
    //
    //mbufSize = 1280*1280*2;
    mbufSize = 640*640*2;
    if (mHeight > 640)
    {
        mbufSize = 640*mHeight*2;
    }
    mbufSize = (mbufSize + L1_CACHE_BYTES-1) & ~(L1_CACHE_BYTES-1);
    mSize = mbufSize;

    mAllocator = IImageBufferAllocator::getInstance();
    IImageBufferAllocator::ImgParam imgParam(mSize,0);
    mpImg = mAllocator->alloc("FDBuffer", imgParam);
    if ( mpImg == 0 )
    {
        MY_LOGE("FDbuffer get NULL Buffer\n");
        mVirtAddr = 0;
        return ;
    }
    if ( !mpImg->lockBuf( "FDBuffer", (eBUFFER_USAGE_HW_CAMERA_READWRITE | eBUFFER_USAGE_SW_MASK)) )
    {
        MY_LOGE("lock Buffer failed\n");
        mpImg->unlockBuf("FDBuffer");
        mAllocator->free(mpImg);
        mpImg = NULL;
        mVirtAddr = 0;
        return ;
    }

    mpImg->syncCache(eCACHECTRL_INVALID);
    int padd_h = FD_IMG_PADDING * mHeight / mWidth;
    int padd_w = FD_IMG_PADDING;
    mVirtAddr = mpImg->getBufVA(0);
    mPhyAddr  = mpImg->getBufPA(0);
    mMemID    = mpImg->getFD(0);
    mStride   = mWidth;
    //
    mWidth = mWidth - padd_w*2;
    mHeight = mHeight - padd_h*2;
    MY_LOGW_IF( mSize & (L1_CACHE_BYTES-1), "bufSize(%d) not aligned!", mSize);
    MY_LOGW_IF( mVirtAddr & (L1_CACHE_BYTES-1), "bufAddr(%p) not aligned!", (void*)mVirtAddr);

    MY_LOGD("FDClient: Virtual Addr: 0x%p, Phyical Addr: 0x%p, bufSize:%d", (void*)mVirtAddr, (void*)mPhyAddr, mSize);

    //
    FUNCTION_OUT;
}


/******************************************************************************
*
*******************************************************************************/
void
FDBuffer::
destroyBuffer()
{
    FUNCTION_IN;
    //
    if (0 == mVirtAddr)
    {
        MY_LOGD("[FD Buffer doesn't exist]");
        return;
    }

    mpImg->unlockBuf("FDBuffer");
    mAllocator->free(mpImg);
    mpImg = NULL;

    //
    FUNCTION_OUT;
}

