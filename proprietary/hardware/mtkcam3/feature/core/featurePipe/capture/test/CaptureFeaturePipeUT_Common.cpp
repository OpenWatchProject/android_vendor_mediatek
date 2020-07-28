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
 * MediaTek Inc. (C) 2018. All rights reserved.
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

// Standard C header file
#include <iostream>
#include <atomic>
#include <thread>
#include <iostream>
// Android system/core header file

// mtkcam custom header file

// mtkcam global header file
#include <mtkcam/drv/IHalSensor.h>
#include <mtkcam/utils/std/Format.h>
#include <mtkcam/utils/metadata/IMetadata.h>
#include <mtkcam/utils/metadata/client/mtk_metadata_tag.h>
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>
// Module header file
#include <featurePipe/core/include/GraphicBufferPool.h>
// Local header file
#include "CaptureFeaturePipeUT_Common.h"
// Log
#define LOG_TAG "CapturePipeUT/Common"
#include "CaptureFeaturePipeUT_log.h"


/*******************************************************************************
* Namespace start.
********************************************************************************/
namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {
namespace NSCapture {
namespace UnitTest {


/*******************************************************************************
* Alias.
********************************************************************************/
using namespace NSCam;


/*******************************************************************************
* Class Define
*******************************************************************************/


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  WorkingBufferHandle Implementation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
WorkingBufferHandle::
WorkingBufferHandle(const char* name, MINT32 format, const MSize& size, const char* path)
: mName(name)
, mPath(path)
, mFormat(format)
, mSize(size)
{
    const MUINT32 plane = NSCam::Utils::Format::queryPlaneCount(mFormat);
    size_t bufBoundaryInBytes[3] = {0, 0, 0};
    size_t bufStridesInBytes[3] = {0};

    for (MUINT32 i = 0; i < plane; i++)
    {
        bufStridesInBytes[i] = NSCam::Utils::Format::queryPlaneWidthInPixels(mFormat, i, mSize.w) *
                               NSCam::Utils::Format::queryPlaneBitsPerPixel(mFormat, i) / 8;
    }

    IImageBufferAllocator::ImgParam imgParam = IImageBufferAllocator::ImgParam(
        (EImageFormat)mFormat, mSize, bufStridesInBytes, bufBoundaryInBytes, plane);

    mpImageBufferHeap = IIonImageBufferHeap::create(mName, imgParam);
}

MERROR
WorkingBufferHandle::
acquire(MINT usage)
{
    mpImageBuffer = mpImageBufferHeap->createImageBuffer();
    if (mpImageBuffer == nullptr)
    {
        std::cerr << "working buffer: create image buffer failed" << std::endl;
        return UNKNOWN_ERROR;
    }

#if 0
    if (mPath != NULL) {
       mpImageBuffer->loadFromFile(mPath);
    }
#endif

    mpImageBuffer->setColorArrangement(SENSOR_FORMAT_ORDER_RAW_R);

    if (!(mpImageBuffer->lockBuf(mName, usage)))
    {
        std::cerr << "working buffer: lock image buffer failed" << std::endl;
        return UNKNOWN_ERROR;
    }
    return OK;
}

IImageBuffer*
WorkingBufferHandle::
native()
{
    return mpImageBuffer.get();
}

void
WorkingBufferHandle::
release()
{
    mpImageBuffer->unlockBuf(mName);
    mpImageBuffer.clear();
    mpImageBufferHeap.clear();
}

MUINT32
WorkingBufferHandle::
getTransform()
{
    return 0;
}

MVOID
WorkingBufferHandle::
dump(std::ostream& os) const
{
    os << "{"
       << "name:" << mName << ", "
       << "format:" << mFormat << ", "
       << "size:" << mSize.w << "x" << mSize.h
       << "}";

}

WorkingBufferHandle::
~WorkingBufferHandle()
{
    if (mpImageBuffer != nullptr)
    {
        mpImageBuffer->unlockBuf(mName);
        mpImageBuffer.clear();
        mpImageBufferHeap.clear();
    }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  WorkingMetadataHandle Implementation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
WorkingMetadataHandle::
WorkingMetadataHandle()
{
    {
        IMetadata exifMeta;
        IMetadata::IEntry entry_iso(MTK_3A_EXIF_AE_ISO_SPEED);
        entry_iso.push_back(1600, Type2Type< MINT32 >());
        exifMeta.update(entry_iso.tag(), entry_iso);
        trySetMetadata<IMetadata>(&mMetadata, MTK_3A_EXIF_METADATA, exifMeta);
    }
    //trySetMetadata<MINT32>(&mMetadata, MTK_JPEG_ORIENTATION, 270);
    trySetMetadata<MINT32>(&mMetadata, MTK_P1NODE_PROCESSOR_MAGICNUM, 0);

    IMetadata::IEntry entry(MTK_EIS_REGION);
    {
        entry.push_back(0, Type2Type< MINT32 >());  // EIS_REGION_INDEX_XINT
        entry.push_back(0, Type2Type< MINT32 >());  // EIS_REGION_INDEX_XFLOAT
        entry.push_back(0, Type2Type< MINT32 >());  // EIS_REGION_INDEX_YINT
        entry.push_back(0, Type2Type< MINT32 >());  // EIS_REGION_INDEX_YFLOAT
        entry.push_back(0, Type2Type< MINT32 >());  // EIS_REGION_INDEX_WIDTH
        entry.push_back(0, Type2Type< MINT32 >());  // EIS_REGION_INDEX_HEIGHT
        entry.push_back(0, Type2Type< MINT32 >());  // EIS_REGION_INDEX_MV2CENTERX
        entry.push_back(0, Type2Type< MINT32 >());  // EIS_REGION_INDEX_MV2CENTERY
        entry.push_back(0, Type2Type< MINT32 >());  // EIS_REGION_INDEX_ISFROMRZ
        entry.push_back(8, Type2Type< MINT32 >());  // EIS_REGION_INDEX_GMVX
        entry.push_back(8, Type2Type< MINT32 >());  // EIS_REGION_INDEX_GMVY
        entry.push_back(0, Type2Type< MINT32 >());  // EIS_REGION_INDEX_CONFX
        entry.push_back(0, Type2Type< MINT32 >());  // EIS_REGION_INDEX_CONFY
        entry.push_back(0, Type2Type< MINT32 >());  // EIS_REGION_INDEX_EXPTIME
        entry.push_back(0, Type2Type< MINT32 >());  // EIS_REGION_INDEX_HWTS
        entry.push_back(0, Type2Type< MINT32 >());  // EIS_REGION_INDEX_LWTS
        entry.push_back(0, Type2Type< MINT32 >());  // EIS_REGION_INDEX_MAX_GMV
        entry.push_back(0, Type2Type< MBOOL >());   // EIS_REGION_INDEX_ISFRONTBIN
    }
    mMetadata.update(MTK_EIS_REGION, entry);


    trySetMetadata<MSize>(&mMetadata, MTK_HAL_REQUEST_SENSOR_SIZE, fsRawSize);

    trySetMetadata<MRect>(&mMetadata, MTK_P1NODE_SCALAR_CROP_REGION, MRect(MPoint(), fsRawSize));
    trySetMetadata<MRect>(&mMetadata, MTK_P1NODE_DMA_CROP_REGION, MRect(MPoint(), fsRawSize));
    trySetMetadata<MSize>(&mMetadata, MTK_P1NODE_RESIZER_SIZE, MSize(1920, 1080));

    trySetMetadata<MINT32>(&mMetadata, MTK_P1NODE_SENSOR_MODE, SENSOR_SCENARIO_ID_NORMAL_CAPTURE);

    trySetMetadata<MRect>(&mMetadata, MTK_SCALER_CROP_REGION, MRect(MPoint(), fsRawSize));
    trySetMetadata<MUINT8>(&mMetadata, MTK_CONTROL_VIDEO_STABILIZATION_MODE, MTK_CONTROL_VIDEO_STABILIZATION_MODE_OFF);
}

MERROR
WorkingMetadataHandle::
acquire()
{
    return OK;
}

IMetadata*
WorkingMetadataHandle::
native()
{
    return &mMetadata;
}

void
WorkingMetadataHandle::
release()
{

}

MVOID
WorkingMetadataHandle::
dump(std::ostream& os) const
{
    os << "{"
       << "count:" << mMetadata.count()
       << "}";
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  TestCallback Implementation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MUINT32
TestCallback::
waitForDesiredCount(MUINT32 desiredCount, const std::chrono::milliseconds timeout)
{
    const auto startWaitTime = std::chrono::system_clock::now();
    std::chrono::milliseconds waitedTime(0);
    MUINT32 currentCount(0);
    while(MTRUE)
    {
        waitedTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - startWaitTime);
        currentCount = getReceivedEventCount();
        if(desiredCount == currentCount)
        {
            UT_LOGD("wait for desired count completed, count: %d (%d), waitedTime: %lld (%lld) ms.", currentCount, desiredCount, waitedTime.count(), timeout.count());
            break;
        }
        else if(waitedTime.count() < timeout.count())
        {
            UT_LOGD("wait for desired count, count: %d (%d), waitedTime: %lld (%lld) ms.", currentCount, desiredCount, waitedTime.count(), timeout.count());
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
        else // wait timeout
        {
            UT_LOGD("wait for desired count timeout, count: %d (%d), waitedTime: %lld (%lld) ms.", currentCount, desiredCount, waitedTime.count(), timeout.count());
            break;
        }
    }
    return currentCount;
}

void
TestCallback::
onMetaResultAvailable(android::sp<ICaptureFeatureRequest> requestPtr, IMetadata* partialMeta)
{
    std::cout << "request metaResultAvailable - no: "  << requestPtr->getRequestNo() << std::endl;
}

void
TestCallback::
onContinue(android::sp<ICaptureFeatureRequest> requestPtr)
{
    std::cout << "request continue - no: "  << requestPtr->getRequestNo() << std::endl;
}

void
TestCallback::
onAborted(android::sp<ICaptureFeatureRequest> requestPtr)
{
    std::cout << "request aborted - no: "  << requestPtr->getRequestNo() << std::endl;
}

void
TestCallback::
onCompleted(android::sp<ICaptureFeatureRequest> requestPtr, MERROR error)
{
    MUINT32 newCount = mCompletedCount++;
    UT_LOGD("request completed, req#:%d, comppletedCount:%d", requestPtr->getRequestNo(), newCount);
}

TestCallback::
~TestCallback()
{

}


} // UnitTest
} // NSCapture
} // namespace NSFeaturePipe
} // namespace NSCamFeature
} // namespace NSCam
