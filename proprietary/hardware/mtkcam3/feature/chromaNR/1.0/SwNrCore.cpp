/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2018. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */
#define LOG_TAG "ChromaNR"

#include "SwNrCore.h"

#include <mtkcam/utils/std/Log.h> // CAM_LOGD
// Allocate working buffer. Be aware of that we use AOSP library
#include <mtkcam3/feature/utils/ImageBufferUtils.h>
//

// ISP profile
//#include <tuning_mapping/cam_idx_struct_ext.h>
#include <cutils/properties.h>

// 3A and ISP
#include <mtkcam/aaa/IHalISP.h>
#include <mtkcam/aaa/ICaptureNR.h>

#include <mtkcam/utils/TuningUtils/FileReadRule.h>
#include <mtkcam/utils/exif/DebugExifUtils.h> // for debug exif
#include <mtkcam/utils/TuningUtils/FileDumpNamingRule.h>
#include <mtkcam3/feature/utils/ImageBufferUtils.h>

// StreamInfo
#include <mtkcam3/pipeline/utils/streaminfo/ImageStreamInfo.h>

// DRV
#include <mtkcam/drv/iopipe/SImager/IImageTransform.h> // IImageTransform

// Gralloc helper
#include <mtkcam/utils/imgbuf/IGraphicImageBufferHeap.h>
#include <mtkcam/utils/gralloc/IGrallocHelper.h>

// Custom
#include <mtkcam/custom/ExifFactory.h>

// Custom debug exif
#include <debug_exif/dbg_id_param.h>
#include <debug_exif/cam/dbg_cam_param.h>

// Std
#include <unordered_map> // std::unordered_map
#include <deque> // std::deque

// AOSP
#include <utils/String8.h>
#include <cutils/properties.h>

using namespace chromaNR;
using namespace NSCam::TuningUtils;
using namespace NSCam::v3;
using namespace NSCam::v3::Utils;

using NSCam::NSIoPipe::NSSImager::IImageTransform;

using android::String8;

// ----------------------------------------------------------------------------
// MY_LOG
// ----------------------------------------------------------------------------
#define MY_LOGV(fmt, arg...)        CAM_LOGV("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("[%s] " fmt, __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( (cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( (cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)

//-----------------------------------------------------------------------------
// CNR util
//-----------------------------------------------------------------------------


BufferHolder::BufferHolder(buffer_handle_t handle) {
    if(handle == nullptr) {
        MY_LOGF("Buffer handle is null!!!");
    }

    importedBufferHandle = handle;

    auto helper = IGrallocHelper::singleton();
    if  ( ! helper || ! helper->importBuffer(importedBufferHandle) || importedBufferHandle == nullptr )
    {
        importedBufferHandle = nullptr;
        MY_LOGF("importBuffer failed");
    } else {
        MY_LOGD("Import buffer handle %p", importedBufferHandle);
    }
}

BufferHolder::~BufferHolder() {
    if  ( importedBufferHandle != nullptr )
    {
        if  ( auto helper = IGrallocHelper::singleton() ) {
            MY_LOGD("free buffer handle: %p", importedBufferHandle);
            helper->freeBuffer(importedBufferHandle);
        }
    }
    importedBufferHandle = nullptr;
}

//-----------------------------------------------------------------------------
// IChromaNR methods
//-----------------------------------------------------------------------------
std::shared_ptr<IChromaNR> IChromaNR::createInstance()
{
    std::shared_ptr<IChromaNR> cNrCore = std::make_shared<SwNrCore>();
    return cNrCore;
}

//-----------------------------------------------------------------------------
// IChromaNR implementation
//-----------------------------------------------------------------------------
SwNrCore::SwNrCore()
{
    mDump = ::property_get_int32("vendor.debug.camera.dump.nr", 0);
}
//-----------------------------------------------------------------------------
SwNrCore::~SwNrCore()
{
}

enum ChromaNRErr SwNrCore::doChromaNR (const ChromaNRParam& param)
{
    enum ChromaNRErr err = ChromaNR_Err_Ok;

    dumpParam(param);

    BufferHolder holder(param.inputHandle);
    sp<IImageBuffer> buf = createImageBuffer(param, holder);
    if(buf.get() == nullptr) {
        MY_LOGF("Create YUV IImageBuffer is nullptr");
        return ChromaNR_Err_BadArgument;
    }

    int32_t bufUsage = (eBUFFER_USAGE_SW_READ_OFTEN | \
                            eBUFFER_USAGE_SW_WRITE_OFTEN | \
                            eBUFFER_USAGE_HW_CAMERA_READWRITE);

    buf->lockBuf(LOG_TAG, bufUsage);

    // Check image buffer is correct or not
    auto format = buf->getImgFormat();
    auto size = buf->getImgSize();
    MY_LOGD("Format(%p) res(%d, %d)", format, size.w, size.h);

    for(size_t i = 0; i < buf->getPlaneCount(); i++) {
        MY_LOGD("Plane[%d] stride(%d)", i, buf->getBufStridesInBytes(i));
    }

    if(mDump) {
        dumpBuffer(buf.get(), "InputBufer");
    }

    std::unique_ptr< ISwNR, std::function<void(ISwNR*)> >
                                    swnr =
                                    std::unique_ptr< ISwNR, std::function<void(ISwNR*)> >
                                    (
                                        MAKE_SwNR(param.open_id),
                                        [](ISwNR* p) {
                                            if(p) {
                                                MY_LOGD("Destroy ISwNR instance");
                                                delete p;
                                                p = nullptr;
                                            }
                                        }
                                    );
    if(swnr.get() == nullptr) {
        MY_LOGF("swnr is nullptr");
        return ChromaNR_Err_BadArgument;
    }

    swnr->init(size);

    ISwNR::SWNRParam swnrParam;
    swnrParam.iso    = param.current_iso;
    swnrParam.isMfll = 0;

    if(format != eImgFmt_I420) {
        sp<IImageBuffer> workingBuf = nullptr;
        auto ret = ImageBufferUtils::getInstance().allocBuffer(
            workingBuf, size.w, size.h, eImgFmt_I420);
        if(CC_UNLIKELY(ret == MFALSE)) {
            MY_LOGF("allocate FeoBase buffer error!!!");
            return ChromaNR_Err_BadArgument;
        }

        // Do format convert by pure MDP
        std::unique_ptr<IImageTransform, std::function<void(IImageTransform*)>> transform(
                IImageTransform::createInstance(), // constructor
                [](IImageTransform *p){ if (p) p->destroyInstance(); } // deleter
                );

        if (transform.get() == nullptr) {
            MY_LOGE("IImageTransform is NULL, cannot generate output");
            return ChromaNR_Err_BadArgument;
        }

        IImageTransform::PQParam config;

        // apply PQ: ClearZoom | DRE

        config.enable       = MTRUE;
        config.portIdx      = 0; // DST_BUF_0
        config.sensorId     = param.open_id;
        config.iso          = param.current_iso;
        config.timestamp    = 0;
        config.frameNo      = 0;
        config.requestNo    = 0;
        // config.lv_value     = lv_value;
        config.mode = IImageTransform::Mode::Capture_Single;

        MRect rectDst1(MPoint(0, 0), size);

        MY_LOGD("format convert+");
        ret = transform->execute(
                    buf.get(),
                    workingBuf.get(),
                    nullptr,
                    rectDst1,
                    0,
                    3000);
        MY_LOGD("format convert-");

        MY_LOGD("do swnr+");
        if (!swnr->doSwNR(swnrParam, workingBuf.get())) {
            MY_LOGE("SWNR failed");
        }
        MY_LOGD("do swnr-");

        MY_LOGD("format convert+");
        ret = transform->execute(
                    workingBuf.get(),
                    buf.get(),
                    nullptr,
                    rectDst1,
                    0,
                    3000);
        MY_LOGD("format convert-");

        if(mDump) {
            dumpBuffer(workingBuf.get(), "nrInputBuf");
            dumpBuffer(buf.get(), "nrOutBuf");
        }

        ImageBufferUtils::getInstance().deallocBuffer(workingBuf.get());
        workingBuf = nullptr;
    }
    else {
        if (!swnr->doSwNR(swnrParam, buf.get())) {
            MY_LOGE("SWNR failed");
        }
    }

    buf->unlockBuf(LOG_TAG);
lbExit:
    return err;
}


/******************************************************************************
 *
 ******************************************************************************/

auto
SwNrCore::
dumpParam(
    const ChromaNRParam& param
) -> void
{
    MY_LOGD("openId(%d), iso(%d)", param.open_id, param.current_iso);
    MY_LOGD("Buf res:(%d, %d, %d) fmt(%p)", param.src_width, param.src_height, param.src_yPitch, param.src_format);
    return;
}

auto
SwNrCore::
dumpBuffer(
    IImageBuffer* buf,
    const char* bufName
) -> void
{
    char fileName[512];
    FILE_DUMP_NAMING_HINT dumpNamingHint;
    // Check image buffer is correct or not
    auto format = buf->getImgFormat();
    auto size = buf->getImgSize();

    dumpNamingHint.ImgFormat = format;
    dumpNamingHint.ImgWidth = size.w;
    dumpNamingHint.ImgHeight = size.h;
    genFileName_YUV(fileName, sizeof(fileName), &dumpNamingHint, YUV_PORT_NULL, bufName);
    buf->saveToFile(fileName);
}

auto
SwNrCore::
createImageStreamInfo(
    const ChromaNRParam& param
) -> IImageStreamInfo*
{
    String8 streamName = String8::format("SWNR YUV stream");
    StreamId_T streamId = 0;
    int32_t imgFormat = param.src_format;
    MSize imgSize = MSize(param.src_width, param.src_height);
    uint32_t srcStreamType = eSTREAMTYPE_IMAGE_IN;

    IImageStreamInfo::BufPlanes_t bufPlanes;
    //bufPlanes.resize(2);
    {
        // Luma plane
        IImageStreamInfo::BufPlane& yPlane = bufPlanes.planes[0];
        yPlane.sizeInBytes      = param.src_yPitch * param.src_height;
        yPlane.rowStrideInBytes = param.src_yPitch;

        // Chroma plane
        IImageStreamInfo::BufPlane& cPlane = bufPlanes.planes[1];
        cPlane.sizeInBytes = param.src_uvPitch * param.src_height / 2;
        cPlane.rowStrideInBytes = param.src_uvPitch;
    }

    MUINT64 const usageForHal = (GRALLOC_USAGE_SW_READ_OFTEN|GRALLOC_USAGE_SW_WRITE_OFTEN) |
                          GRALLOC1_PRODUCER_USAGE_CAMERA ;
    MUINT64 usageForAllocator = usageForHal;
    //
    auto pImageStreamInfo = ImageStreamInfoBuilder()
                            .setStreamName(streamName.c_str())
                            .setStreamId(streamId)
                            .setStreamType(srcStreamType)
                            .setMaxBufNum(1)
                            .setMinInitBufNum(1)
                            .setUsageForAllocator(usageForAllocator)
                            .setAllocImgFormat(imgFormat)
                            .setAllocBufPlanes(bufPlanes)
                            .setImgFormat(imgFormat)
                            .setBufPlanes(bufPlanes)
                            .setImgSize(imgSize)
                            .setBufCount(1)
                            .setBufStep(0)
                            .setStartOffset(0)
                            .build();

    return pImageStreamInfo.get();
}

auto
SwNrCore::
createImageBufferHeap(
    const std::string& bufferName,
    const android::sp<IImageStreamInfo>& streamInfo,
    buffer_handle_t bufferHandle,
    int const acquire_fence,
    int const release_fence
) -> IImageBufferHeap*
{
    int format = streamInfo->getAllocImgFormat();
    IGraphicImageBufferHeap* pImageBufferHeap =
        IGraphicImageBufferHeap::create(
            (bufferName + ":" + streamInfo->getStreamName()).c_str(),
            streamInfo->getUsageForAllocator(),
            streamInfo->getImgSize(),
            format,
            &bufferHandle,
            acquire_fence,
            release_fence
        );
    if ( pImageBufferHeap == nullptr ){
        MY_LOGF("IGraphicImageBufferHeap::create \"%s:%s\", handle: %p, fd: %d",
                 bufferName.c_str(), streamInfo->getStreamName(), bufferHandle, bufferHandle->data[0]);
    }

    return pImageBufferHeap;
}

auto
SwNrCore::
createImageBuffer(
    const ChromaNRParam& param,
    BufferHolder& holder
) -> IImageBuffer*
{
    sp<IImageStreamInfo> appStreamInfo = createImageStreamInfo(param);
    if(appStreamInfo.get() == nullptr) {
        MY_LOGF("create yuv streamInfo fail");
        return nullptr;
    }

    // Create graphic buffer heap
    buffer_handle_t handle = holder.importedBufferHandle;
    string bufferName = "appYUV";
    sp<IImageBufferHeap> heap = createImageBufferHeap(bufferName, appStreamInfo, handle, -1/*acquire_fence*/, -1/*release_fence*/);

    // Create imagebuffer
    IImageBuffer* buf = nullptr;
    buf = heap->createImageBuffer();

    return buf;
}
