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

// The implementation was cloned from P1Node
// Only small modification for easily review & code sync

#define LOG_TAG     "CamIoDummy"

#include <mtkcam/utils/std/Log.h>
#include <mtkcam/utils/std/ULog.h>
#include <mtkcam/utils/imgbuf/IIonImageBufferHeap.h>
#include <mtkcam/utils/imgbuf/IDummyImageBufferHeap.h>
#include "CamIoDummyBuffer.h"

CAM_ULOG_DECLARE_MODULE_ID(MOD_DRV_NORMAL_PIPE);


#define MY_LOGD(fmt, arg...)        CAM_ULOGMD("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_ULOGMI("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_ULOGME("[%s] " fmt, __FUNCTION__, ##arg)

#define FUNCTION_IN
#define FUNCTION_OUT


using namespace android;


namespace NSCam {
namespace NSIoPipe {
namespace NSCamIOPipe {
namespace Wrapper {

/******************************************************************************
 *
 ******************************************************************************/
MERROR
DummyBuffer::
init(MUINT32 bufferSize, int numHardwareBuffer)
{
    FUNCTION_IN

    MERROR err = OK;
    mSize = bufferSize;
    mSlot = numHardwareBuffer;
    mTotal = mSize + (sizeof(MINTPTR) * (mSlot - 1));
    IImageBufferAllocator::ImgParam imgParam(mTotal, 0);
    sp<IIonImageBufferHeap> pHeap =
        IIonImageBufferHeap::create(mName.string(), imgParam);
    if (pHeap == NULL) {
        MY_LOGE("[%s] image buffer heap create fail", mName.string());
        return BAD_VALUE;
    }
    mDummyBufferHeap = pHeap;
    MUINT const usage = (GRALLOC_USAGE_SW_READ_OFTEN |
                        GRALLOC_USAGE_HW_CAMERA_READ |
                        GRALLOC_USAGE_HW_CAMERA_WRITE);
    sp<IImageBuffer> pImageBuffer = pHeap->createImageBuffer();
    if (pImageBuffer == NULL) {
        MY_LOGE("[%s] image buffer create fail", mName.string());
        return BAD_VALUE;
    }
    if (!(pImageBuffer->lockBuf(mName.string(), usage))) {
        MY_LOGE("[%s] image buffer lock fail", mName.string());
        return BAD_VALUE;
    }
    mVAddr = pHeap->getBufVA(0);
    mPAddr = pHeap->getBufPA(0);
    mHeapId = pHeap->getHeapID();
    mDummyBuffer = pImageBuffer;
    mUsingCount = 0;
    MY_LOGI("DummyBufferHeap[%s] Len(%d, %d, %d) VA(0x%zx) PA(0x%zx) ID(%d)",
        mName.string(), mSize, mSlot, mTotal, mVAddr, mPAddr, mHeapId);
    //
    FUNCTION_OUT;
    return err;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
DummyBuffer::
uninit(void)
{
    FUNCTION_IN

    MERROR err = OK;
    // Use exact dummy buffer number to call count(false), in order to
    // let mUsingCount == 0 to pass following check is meaningless
#if 0
    //
    if (mUsingCount > 0) {
        MY_LOGE("dummy buffer [%s] is using (%d) - should not uninit",
            mName.string(), mUsingCount);
        return BAD_VALUE;
    }
#else
    MY_LOGD("dummy buffer [%s]: %d", mName.string(), mUsingCount);
#endif

    if (mDummyBuffer != NULL) {
        mDummyBuffer->unlockBuf(mName.string());
        mDummyBuffer = NULL;
        mDummyBufferHeap = NULL;
        mVAddr = 0;
        mPAddr = 0;
        mSize = 0;
        mSlot = 0;
        mTotal = 0;
        mUsingCount = 0;
    }

    FUNCTION_OUT
    return err;
}


MERROR
DummyBuffer::
acquire(
    size_t index,
    char const* szName,
    sp<IImageBuffer> & imageBuffer
)
{
    FUNCTION_IN

    MERROR err = OK;
    MUINT const usage = (eBUFFER_USAGE_SW_READ_OFTEN |
                        eBUFFER_USAGE_HW_CAMERA_READ |
                        eBUFFER_USAGE_HW_CAMERA_WRITE);

    //
    if (index >= mSlot) {
        MY_LOGE("acquire [%s] - dummy buffer index out of range (%zu >= %d)",
            szName, index, mSlot);
        return BAD_VALUE;
    }
    // use eImgFmt_BLOB for dummy buffer, plane count is 1
    MUINTPTR const cVAddr = mVAddr + (sizeof(MINTPTR) * (index));
    MUINTPTR const virtAddr[] = {cVAddr, 0, 0};
    MUINTPTR const cPAddr = mPAddr + (sizeof(MINTPTR) * (index));
    MUINTPTR const phyAddr[] = {cPAddr, 0, 0};
    IImageBufferAllocator::ImgParam imgParam =
                        IImageBufferAllocator::ImgParam(mSize, 0);
    PortBufInfo_dummy portBufInfo = PortBufInfo_dummy(
                                        mHeapId,
                                        virtAddr,
                                        phyAddr,
                                        1);
    sp<IImageBufferHeap> imgBufHeap = IDummyImageBufferHeap::create(
                                        szName,
                                        imgParam,
                                        portBufInfo,
                                        (1 <= mLogLevel) ? true : false);
    if (imgBufHeap == NULL) {
        MY_LOGE("acquire [%s] - image buffer heap create fail", szName);
        return UNKNOWN_ERROR;
    }
    sp<IImageBuffer> imgBuf = imgBufHeap->createImageBuffer();
    if (imgBuf == NULL) {
        MY_LOGE("acquire [%s] - image buffer create fail", szName);
        return BAD_VALUE;
    }
    if (!(imgBuf->lockBuf(szName, usage))) {
        MY_LOGE("acquire [%s] - image buffer lock fail", szName);
        return BAD_VALUE;
    }
    imageBuffer = imgBuf;
    count(true);

    FUNCTION_OUT
    return err;
}


MERROR
DummyBuffer::
release(
    char const* szName,
    sp<IImageBuffer> & imageBuffer
)
{
    FUNCTION_IN

    count(false);
    imageBuffer->unlockBuf(szName);
    imageBuffer = NULL;

    FUNCTION_OUT
    return OK;
}

}
}
}
}

