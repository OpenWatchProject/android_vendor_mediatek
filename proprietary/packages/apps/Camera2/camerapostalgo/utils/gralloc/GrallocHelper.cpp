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

#define LOG_TAG "MtkCam/GrallocHelper"
//
#include "MyUtils.h"
#include <utils/gralloc/IGrallocHelper.h>
//#include <mtkcam/utils/std/Sync.h>
//
#include <stdlib.h>
#include <cutils/properties.h>
#include <cutils/atomic.h>
//
#include <utils/String8.h>
#include <utils/Mutex.h>
#include <utils/Timers.h>
#include <utils/KeyedVector.h>
#include <utils/RefBase.h>
#include <sync/sync.h>
#include <vector>
//
#define PHASE_II                (1)

//#include <libladder.h>
#include <ui/gralloc_extra.h>
#include <graphics_mtk_defs.h>
#include <gralloc_mtk_defs.h>
//
#include "mapper/mapper.h"
#include "allocator/allocator.h"
#include <hidl/HidlSupport.h>
//
using namespace android;
using namespace android::hardware;
using namespace android::hardware::graphics::mapper::V2_0;
//
using namespace NSCam;
//using namespace NSCam::Utils::Sync;


/******************************************************************************
 *
 ******************************************************************************/
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
//
#if 1
#define FUNC_START     MY_LOGD("+")
#define FUNC_END       MY_LOGD("-")
#else
#define FUNC_START
#define FUNC_END
#endif

/******************************************************************************
 *
 ******************************************************************************/
#define GRALLOC_ALIGN( value, base ) (((value) + ((base) - 1)) & ~((base) - 1))
#define ADDRESS_OFFSET(base1, base2) (intptr_t( base1 ) - intptr_t( base2 ))
/*
#define DUMP_CALLSTACK(_x_)                                 \
        do {                                                \
            std::string callstack;                          \
            UnwindCurThreadBT(&callstack);                  \
            __android_log_write(ANDROID_LOG_INFO, LOG_TAG"/"_x_, callstack.c_str()); \
        } while(0)
*/

/******************************************************************************
 *
 ******************************************************************************/
namespace {
    // mapper
    std::unique_ptr<const NSCam::GrallocMapper::Mapper> gMapper = nullptr;
    std::unique_ptr<const NSCam::GrallocAllocator::Allocator> gAllocator = nullptr;
// Use "adb shell setprop debug.mmutils.gralloc.loglevel 1" to change the log level.
static volatile int32_t gLogLevel = 0;
static void setLogLevel(int level)
{
    ::android_atomic_write(level, &gLogLevel);
}
};  //namespace


/******************************************************************************
 *
 ******************************************************************************/
namespace {
/**
 *
 */
String8
gQueryPixelFormatName(int format)
{
#define _ENUM_TO_NAME_(_prefix_, _format_) \
    case _prefix_##_format_: \
        {static String8 name(#_format_); return name;}break

    switch  (format)
    {
    _ENUM_TO_NAME_(HAL_PIXEL_FORMAT_, RGBA_8888);
    _ENUM_TO_NAME_(HAL_PIXEL_FORMAT_, RGBX_8888);
    _ENUM_TO_NAME_(HAL_PIXEL_FORMAT_, RGB_888);
    _ENUM_TO_NAME_(HAL_PIXEL_FORMAT_, RGB_565);
    _ENUM_TO_NAME_(HAL_PIXEL_FORMAT_, BGRA_8888);
    _ENUM_TO_NAME_(HAL_PIXEL_FORMAT_, YV12);
    _ENUM_TO_NAME_(HAL_PIXEL_FORMAT_, Y8);
    _ENUM_TO_NAME_(HAL_PIXEL_FORMAT_, Y16);
    _ENUM_TO_NAME_(HAL_PIXEL_FORMAT_, BLOB);
    _ENUM_TO_NAME_(HAL_PIXEL_FORMAT_, IMPLEMENTATION_DEFINED);
    _ENUM_TO_NAME_(HAL_PIXEL_FORMAT_, CAMERA_OPAQUE);
    _ENUM_TO_NAME_(HAL_PIXEL_FORMAT_, YCbCr_420_888);
    _ENUM_TO_NAME_(HAL_PIXEL_FORMAT_, YCbCr_422_SP);
    _ENUM_TO_NAME_(HAL_PIXEL_FORMAT_, YCrCb_420_SP);
    _ENUM_TO_NAME_(HAL_PIXEL_FORMAT_, YCbCr_422_I);
    _ENUM_TO_NAME_(HAL_PIXEL_FORMAT_, RAW16);
    _ENUM_TO_NAME_(HAL_PIXEL_FORMAT_, RAW_OPAQUE);
    default:
        break;
    }

#undef  _ENUM_TO_NAME_
    //
    return  String8::empty();
}


/**
 *
 */
String8
gQueryGrallocUsageName(int usage)
{
#define _USAGE_TO_NAME_(_prefix_, _usage_) \
    case _prefix_##_usage_:{str += "|"#_usage_;}break

    String8 str("0");
    //
    switch  ( (usage & GRALLOC_USAGE_SW_READ_MASK) )
    {
    _USAGE_TO_NAME_(GRALLOC_USAGE_, SW_READ_RARELY);
    _USAGE_TO_NAME_(GRALLOC_USAGE_, SW_READ_OFTEN);
    default: break;
    }
    //
    switch  ( (usage & GRALLOC_USAGE_SW_WRITE_MASK) )
    {
    _USAGE_TO_NAME_(GRALLOC_USAGE_, SW_WRITE_RARELY);
    _USAGE_TO_NAME_(GRALLOC_USAGE_, SW_WRITE_OFTEN);
    default: break;
    }
    //
    switch  ( (usage & GRALLOC_USAGE_HW_CAMERA_MASK) )
    {
    _USAGE_TO_NAME_(GRALLOC_USAGE_, HW_CAMERA_WRITE);
    _USAGE_TO_NAME_(GRALLOC_USAGE_, HW_CAMERA_READ);
    _USAGE_TO_NAME_(GRALLOC_USAGE_, HW_CAMERA_ZSL);
    default: break;
    }
    //
    if  ( (usage & GRALLOC_USAGE_HW_MASK) != 0 )
    {
#define _USAGE_TO_NAME_OR_(_prefix_, _usage_) \
        if  ( (usage & _prefix_##_usage_) ) {str += "|"#_usage_;}

        _USAGE_TO_NAME_OR_(GRALLOC_USAGE_, HW_TEXTURE);
        _USAGE_TO_NAME_OR_(GRALLOC_USAGE_, HW_RENDER);
        _USAGE_TO_NAME_OR_(GRALLOC_USAGE_, HW_2D);
        _USAGE_TO_NAME_OR_(GRALLOC_USAGE_, HW_COMPOSER);
        _USAGE_TO_NAME_OR_(GRALLOC_USAGE_, HW_FB);
        _USAGE_TO_NAME_OR_(GRALLOC_USAGE_, HW_VIDEO_ENCODER);
        _USAGE_TO_NAME_OR_(GRALLOC1_USAGE_, SMVR);
#undef  _USAGE_TO_NAME_OR_
    }
    //
#undef  _USAGE_TO_NAME_
    return  str;
}


/**
 *
 */
String8
gQueryDataspaceName(int32_t dataspace)
{
    String8 str;

    switch (dataspace)
    {
#define _DS_TO_NAME_(_ds_) case (int32_t)Dataspace::_ds_: {str = #_ds_;} break;

    // Legacy dataspaces
    _DS_TO_NAME_(SRGB_LINEAR);
    _DS_TO_NAME_(V0_SRGB_LINEAR);
    _DS_TO_NAME_(V0_SCRGB_LINEAR);
    _DS_TO_NAME_(SRGB);
    _DS_TO_NAME_(V0_SRGB);
    _DS_TO_NAME_(V0_SCRGB);

    // YCbCr Colorspaces
    _DS_TO_NAME_(JFIF);
    _DS_TO_NAME_(V0_JFIF);
    _DS_TO_NAME_(BT601_625);
    _DS_TO_NAME_(V0_BT601_625);
    _DS_TO_NAME_(BT601_525);
    _DS_TO_NAME_(V0_BT601_525);
    _DS_TO_NAME_(BT709);
    _DS_TO_NAME_(V0_BT709);
    _DS_TO_NAME_(DCI_P3_LINEAR);
    _DS_TO_NAME_(DCI_P3);
    //_DS_TO_NAME_(DISPLAY_P3_LINEAR);  //the same as DCI_P3_LINEAR
    _DS_TO_NAME_(DISPLAY_P3);
    _DS_TO_NAME_(ADOBE_RGB);
    _DS_TO_NAME_(BT2020_LINEAR);
    _DS_TO_NAME_(BT2020);
    _DS_TO_NAME_(BT2020_PQ);

    // Data spaces for non-color formats
    _DS_TO_NAME_(DEPTH);
    _DS_TO_NAME_(SENSOR);

    _DS_TO_NAME_(ARBITRARY);
    _DS_TO_NAME_(UNKNOWN);
    default: {str = String8::format("%#x", dataspace);} break;

#undef  _DS_TO_NAME_
    }

#define _DATASPACE_TO_NAME_(_ds_) case (int32_t)Dataspace::_ds_: {str += "|"#_ds_;} break;
    //
    //STANDARD_MASK
    switch ( dataspace & Dataspace::STANDARD_MASK )
    {
    _DATASPACE_TO_NAME_(STANDARD_BT709);
    _DATASPACE_TO_NAME_(STANDARD_BT601_625);
    _DATASPACE_TO_NAME_(STANDARD_BT601_625_UNADJUSTED);
    _DATASPACE_TO_NAME_(STANDARD_BT601_525);
    _DATASPACE_TO_NAME_(STANDARD_BT601_525_UNADJUSTED);
    _DATASPACE_TO_NAME_(STANDARD_BT2020);
    _DATASPACE_TO_NAME_(STANDARD_BT2020_CONSTANT_LUMINANCE);
    _DATASPACE_TO_NAME_(STANDARD_BT470M);
    _DATASPACE_TO_NAME_(STANDARD_FILM);
    _DATASPACE_TO_NAME_(STANDARD_DCI_P3);
    _DATASPACE_TO_NAME_(STANDARD_ADOBE_RGB);
    default: break;
    }
    //
    //TRANSFER_MASK
    switch ( dataspace & Dataspace::TRANSFER_MASK )
    {
    _DATASPACE_TO_NAME_(TRANSFER_LINEAR);
    _DATASPACE_TO_NAME_(TRANSFER_SRGB);
    _DATASPACE_TO_NAME_(TRANSFER_SMPTE_170M);
    _DATASPACE_TO_NAME_(TRANSFER_GAMMA2_2);
    _DATASPACE_TO_NAME_(TRANSFER_GAMMA2_6);
    _DATASPACE_TO_NAME_(TRANSFER_GAMMA2_8);
    _DATASPACE_TO_NAME_(TRANSFER_ST2084);
    _DATASPACE_TO_NAME_(TRANSFER_HLG);
    default: break;
    }
    //
    //RANGE_MASK
    switch ( dataspace & Dataspace::RANGE_MASK )
    {
    _DATASPACE_TO_NAME_(RANGE_FULL);
    _DATASPACE_TO_NAME_(RANGE_LIMITED);
    _DATASPACE_TO_NAME_(RANGE_EXTENDED);
    default: break;
    }
#undef  _DATASPACE_TO_NAME_

    return  str;
}


/**
 *
 */
struct Extra {
    int     format;
    int     usage;
    size_t  widthInPixels;
    size_t  heightInPixels;
    size_t  rowStrideInPixels;
    size_t  rowNum;
    size_t  allocSize;
    uint32_t producerUsage = 0;
};
status_t
gQueryExtra(
    buffer_handle_t const bh,
    Extra& extra
)
{
    int err = GRALLOC_EXTRA_OK;

#define MY_QUERY(_extra_get_, _field_) \
    extra._field_ = 0; \
    err |= ::gralloc_extra_query(bh, GRALLOC_EXTRA_GET_##_extra_get_, &extra._field_)
        //
    MY_QUERY(FORMAT         , format);
    MY_QUERY(USAGE          , usage);
    MY_QUERY(WIDTH          , widthInPixels);
    MY_QUERY(HEIGHT         , heightInPixels);
    MY_QUERY(STRIDE         , rowStrideInPixels);
    MY_QUERY(VERTICAL_STRIDE, rowNum);
    MY_QUERY(ALLOC_SIZE     , allocSize);
    MY_QUERY(PRODUCER_USAGE , producerUsage);
        //
#undef  MY_QUERY
    //
    if  ( GRALLOC_EXTRA_OK != err) {
        MY_LOGE("Given buffer_handle_t:%p, gralloc_extra_query returns %d", bh, err);
        return UNKNOWN_ERROR;
    }
    return OK;
}


/**
 *
 */
status_t
gQueryYCbCr(
    android_ycbcr& ycbcr,
    buffer_handle_t const bh,
    size_t w,
    size_t h
)
{
    status_t status = OK;
    NSCam:: GrallocMapper::YCbCrLayout layout;
    int32_t fenceFd = -1;
    //
    auto toGrallocRect = [](int32_t w, int32_t h)
    {
        NSCam::GrallocMapper::IMapper::Rect outRect =
        {
            .left   = 0,
            .top    = 0,
            .width  = w,
            .height = h,
        };
        return outRect;
    };
    //
    Error error = gMapper->lock(bh, GRALLOC_USAGE_SW_READ_OFTEN,
                                toGrallocRect((int32_t)w, (int32_t)h), -1, &layout);
    if (error == Error::NONE)
    {
        ycbcr.y = layout.y;
        ycbcr.cb = layout.cb;
        ycbcr.cr = layout.cr;
        ycbcr.ystride = static_cast<size_t>(layout.yStride);
        ycbcr.cstride = static_cast<size_t>(layout.cStride);
        ycbcr.chroma_step = static_cast<size_t>(layout.chromaStep);
    }
    status =static_cast<status_t>(error);
    fenceFd = gMapper->unlock(bh);
    ::sync_wait(fenceFd, -1);
    ::close(fenceFd);
    //
    ssize_t const offset_cr_y = ADDRESS_OFFSET(ycbcr.cr, ycbcr.y);
    ssize_t const offset_cb_cr= ADDRESS_OFFSET(ycbcr.cb, ycbcr.cr);
    MY_LOGD_IF(
        (1),
        "cr-y:%zu cb-cr:%zu y:%p cb:%p cr:%p ystride:%zu cstride:%zu chroma_step:%zu status:%d",
        offset_cr_y, offset_cb_cr,
        ycbcr.y, ycbcr.cb, ycbcr.cr,
        ycbcr.ystride, ycbcr.cstride,
        ycbcr.chroma_step,
        status
    );
    MY_LOGF_IF(OK != status, "status:%d(%s)", -status, ::strerror(-status));
    return status;
}
};  //namespace


/******************************************************************************
 *
 ******************************************************************************/
namespace {
/**
 *
 */
struct MyStaticInfo : public GrallocStaticInfo
{
    nsecs_t             grallocTime;
    int                 usage;
    size_t              allocSize;
    uint32_t            producerUsage = 0;
                        //
                        MyStaticInfo()
                        {
                            format = 0;
                            grallocTime = 0;
                            usage = 0;
                            widthInPixels = 0;
                            heightInPixels = 0;
                            allocSize = 0;
                        }
};
};  //namespace


/******************************************************************************
 *
 ******************************************************************************/
namespace {
/**
 *
 */
inline
bool
checkPlaneSize(
    MyStaticInfo const& info,
    size_t const index,
    size_t const rowNum
)
{
    MyStaticInfo::Plane const& plane = info.planes[index];
    //
    bool isEqual = plane.sizeInBytes == plane.rowStrideInBytes * rowNum;
    MY_LOGW_IF(
        (! isEqual) && (0<gLogLevel),
        "[%s:%zu] sizeInBytes:%zu != %zu x %zu",
        gQueryPixelFormatName(info.format).string(),
        index,
        plane.sizeInBytes, plane.rowStrideInBytes, rowNum
    );
    //
    return isEqual;
}

/**
 *
 */
status_t
queryStaticInfo(buffer_handle_t const bh, MyStaticInfo& info)
{
    Extra extra;
    int err = gQueryExtra(bh, extra);
    if  ( GRALLOC_EXTRA_OK != err ) {
            MY_LOGE("Given buffer_handle_t:%p, gralloc_extra_query returns %d", bh, err);
            return UNKNOWN_ERROR;
    }
    //
    info.format         = extra.format;
    info.usage          = extra.usage;
    info.widthInPixels  = extra.widthInPixels;
    info.heightInPixels = extra.heightInPixels;
    info.allocSize      = extra.allocSize;
    info.producerUsage  = extra.producerUsage;
    //
    switch  ( info.format )
    {
    case HAL_PIXEL_FORMAT_Y8:
    case HAL_PIXEL_FORMAT_BLOB:
        {
            info.planes.resize(1);
            //
            {
                MyStaticInfo::Plane& plane = info.planes.editItemAt(0);
                plane.sizeInBytes = info.allocSize;
                plane.rowStrideInBytes = (extra.rowStrideInPixels);     //1bpp
                //
                checkPlaneSize(info, 0, (extra.rowNum));
            }
        }break;
    //
    case HAL_PIXEL_FORMAT_YCbCr_422_I:
        {
            info.planes.resize(1);
            //
            {
                MyStaticInfo::Plane& plane = info.planes.editItemAt(0);
                plane.rowStrideInBytes = (extra.rowStrideInPixels<<1);  //2bpp
                plane.sizeInBytes = info.allocSize;
                //
                checkPlaneSize(info, 0, (extra.rowNum));
                MY_LOGW_IF(plane.rowStrideInBytes%32!=0, "[YCbCr_422_I] rowStrideInBytes:%zu is not a multiple of 32", plane.rowStrideInBytes);
                MY_LOGW_IF(plane.sizeInBytes<plane.rowStrideInBytes*16, "[YCbCr_422_I] sizeInBytes:%zu<rowStrideInBytes:%zu*16", plane.sizeInBytes, plane.rowStrideInBytes);
            }
        }break;
    //
    case HAL_PIXEL_FORMAT_YCrCb_420_SP:
        {
            struct android_ycbcr ycbcr;
            if  ( OK != gQueryYCbCr(ycbcr, bh, info.widthInPixels, info.heightInPixels) ) {
                return UNKNOWN_ERROR;
            }
            //
            info.planes.resize(2);
            //
            {// Y
                MyStaticInfo::Plane& plane = info.planes.editItemAt(0);
                plane.rowStrideInBytes = ycbcr.ystride;
                plane.sizeInBytes = ADDRESS_OFFSET(ycbcr.cr, ycbcr.y);
                //
                checkPlaneSize(info, 0, (extra.rowNum));
            }
            {// CrCb
                MyStaticInfo::Plane& plane = info.planes.editItemAt(1);
                plane.rowStrideInBytes = ycbcr.cstride;
                plane.sizeInBytes = extra.allocSize - ADDRESS_OFFSET(ycbcr.cr, ycbcr.y);
                //
                checkPlaneSize(info, 1, (extra.rowNum>>1));
            }
        }break;
    //
    case HAL_PIXEL_FORMAT_YV12:
        {
            struct android_ycbcr ycbcr;
            if  ( OK != gQueryYCbCr(ycbcr, bh, info.widthInPixels, info.heightInPixels) ) {
                return UNKNOWN_ERROR;
            }
            //
            info.planes.resize(3);
            //
            {// Y
                MyStaticInfo::Plane& plane = info.planes.editItemAt(0);
                plane.rowStrideInBytes = ycbcr.ystride;
                plane.sizeInBytes = ADDRESS_OFFSET(ycbcr.cr, ycbcr.y);
                //
                checkPlaneSize(info, 0, (extra.rowNum));
            }
            {// Cr
                MyStaticInfo::Plane& plane = info.planes.editItemAt(1);
                plane.rowStrideInBytes = ycbcr.cstride;
                plane.sizeInBytes = ADDRESS_OFFSET(ycbcr.cb, ycbcr.cr);
                //
                checkPlaneSize(info, 1, (extra.rowNum>>1));
            }

            {// Cb
                MyStaticInfo::Plane& plane = info.planes.editItemAt(2);
                plane.rowStrideInBytes = ycbcr.cstride;
                plane.sizeInBytes = extra.allocSize - ADDRESS_OFFSET(ycbcr.cb, ycbcr.y);
                //
                checkPlaneSize(info, 2, (extra.rowNum>>1));

                if ( (extra.producerUsage &  GRALLOC1_USAGE_SMVR) != 0)
                {
                    size_t oriCbPlane_sizeInBytes = plane.sizeInBytes;

                    // !!NOTES: yv12 layout that fits Video-Encoder
                    plane.sizeInBytes = ((((info.widthInPixels-1)>>5)+1)<<5) * ((((info.heightInPixels-1)>>5)+1)<<5) * 3/2 - ADDRESS_OFFSET(ycbcr.cb, ycbcr.y);
                    MY_LOGD_IF(0, "Extra-info(producerUsage=%#" PRIx64 ", %dx%d, "
                        "plane-y(rsb=%ld, sib=%ld), "
                        "plane-cr(rsb=%ld, sid=%ld), "
                        "plane-cb(rsb=%ld, sib-ori=%ld, sib-adjusted=%ld), "
                        "extra.allocSize=%ld",
                       extra.producerUsage, info.widthInPixels, info.heightInPixels,
                       ycbcr.ystride, ADDRESS_OFFSET(ycbcr.cr, ycbcr.y),
                       ycbcr.cstride, ADDRESS_OFFSET(ycbcr.cb, ycbcr.cr),
                       ycbcr.cstride, oriCbPlane_sizeInBytes, plane.sizeInBytes,
                       extra.allocSize
                    );
                }
            }
        }break;
    //
    case HAL_PIXEL_FORMAT_RGBA_8888:
    case HAL_PIXEL_FORMAT_BGRA_8888:
        {
            info.planes.resize(1);
            //
            {
                MyStaticInfo::Plane& plane = info.planes.editItemAt(0);
                plane.rowStrideInBytes = (extra.rowStrideInPixels<<2);//4bpp
                plane.sizeInBytes = extra.allocSize;
                //
                checkPlaneSize(info, 0, (extra.rowNum));
            }
        }break;
    case HAL_PIXEL_FORMAT_RGB_888:
        {
            info.planes.resize(1);
            //
            {
                MyStaticInfo::Plane& plane = info.planes.editItemAt(0);
                plane.rowStrideInBytes = (extra.rowStrideInPixels * 3);//3bpp
                plane.sizeInBytes = extra.allocSize;
                //
                checkPlaneSize(info, 0, (extra.rowNum));
            }
        }break;
    case HAL_PIXEL_FORMAT_RAW16:
        {
            info.planes.resize(1);
            //
            {
                MyStaticInfo::Plane& plane = info.planes.editItemAt(0);
                plane.rowStrideInBytes = (extra.rowStrideInPixels<<1);//2bpp
                plane.sizeInBytes = extra.allocSize;
                //
                checkPlaneSize(info, 0, (extra.rowNum));
            }
        }break;
    case HAL_PIXEL_FORMAT_RAW_OPAQUE:
        {
            info.planes.resize(1);
            //
            {
                MyStaticInfo::Plane& plane = info.planes.editItemAt(0);
                plane.rowStrideInBytes = (extra.rowStrideInPixels);//1bpp
                plane.sizeInBytes = extra.allocSize;
                //
                checkPlaneSize(info, 0, (extra.rowNum));
            }
        }break;
    //
    case HAL_PIXEL_FORMAT_CAMERA_OPAQUE:
        {
            MY_LOGD(
                "only in zsl mode: %#x(%s)",
                info.format,
                gQueryPixelFormatName(info.format).string()
            );
            //
            info.planes.resize(1);
            //
            {
                MyStaticInfo::Plane& plane = info.planes.editItemAt(0);
                plane.sizeInBytes = info.allocSize;
                plane.rowStrideInBytes = (extra.rowStrideInPixels*10/8);//1.25bpp
                //
                checkPlaneSize(info, 0, (extra.rowNum));
            }
        } break;
    case HAL_PIXEL_FORMAT_IMPLEMENTATION_DEFINED:
        {
            MY_LOGD(
                "only in zsl mode: %#x(%s)",
                info.format,
                gQueryPixelFormatName(info.format).string()
            );
            //
            info.planes.resize(1);
            //
            {
                MyStaticInfo::Plane& plane = info.planes.editItemAt(0);
                plane.sizeInBytes = info.allocSize;
                plane.rowStrideInBytes = (extra.rowStrideInPixels*10/8);//1.25bpp
                //
                checkPlaneSize(info, 0, (extra.rowNum));
            }
        } break;
    case HAL_PIXEL_FORMAT_YCbCr_420_888:
        MY_LOGE(
            "format from gralloc_extra_query should not be: %#x(%s)",
            info.format,
            gQueryPixelFormatName(info.format).string()
        );
        return UNKNOWN_ERROR;
    default:
        MY_LOGE(
            "Not support format: %#x(%s)",
            info.format,
            gQueryPixelFormatName(info.format).string()
        );
        return NAME_NOT_FOUND;
    }
    //
    return OK;
}
};  //namespace


/******************************************************************************
 *
 ******************************************************************************/
namespace {
class GrallocHelperImp : public IGrallocHelper
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Definitions.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////

    /**
     *
     */
    struct  MyRequest : public GrallocRequest
    {
                                MyRequest(GrallocRequest const& rhs = GrallocRequest())
                                    : GrallocRequest(rhs)
                                {
                                }

        bool                    operator==(MyRequest const& rhs) const
                                {
                                    return  usage == rhs.usage
                                        &&  format== rhs.format
                                        &&  widthInPixels == rhs.widthInPixels
                                        &&  heightInPixels== rhs.heightInPixels
                                            ;
                                }

        bool                    operator<(MyRequest const& rhs) const
                                {
                                    /*
                                        true : if lhs < rhs
                                        false: if lhs >= rhs
                                     */
                                    if  ( format != rhs.format ) {
                                        return  format < rhs.format;
                                    }
                                    if  ( usage != rhs.usage ) {
                                        return  usage < rhs.usage;
                                    }
                                    if  ( widthInPixels != rhs.widthInPixels ) {
                                        return  widthInPixels < rhs.widthInPixels;
                                    }
                                    if  ( heightInPixels != rhs.heightInPixels ) {
                                        return  heightInPixels < rhs.heightInPixels;
                                    }
                                    //
                                    //  here: lhs >= rhs
                                    return  false;
                                }
    };
    typedef KeyedVector<MyRequest, MyStaticInfo> Map_t;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////                Data Members.

    mutable Mutex               mLock;
    mutable Map_t               mMap;
    bool                        mInitialized;
    uint32_t                    mLogLevel;

protected:  ////                Operations.
    status_t                    initialize();

    static  status_t            determine(GrallocRequest const& src, MyRequest& dst);
    static  status_t            determine(MyRequest const& src, MyStaticInfo& dst);

    ssize_t                     addToMapLocked(
                                    Map_t::key_type const& k,
                                    Map_t::value_type const& v
                                ) const;

    void                        dump(
                                    Map_t::key_type const& k,
                                    Map_t::value_type const& v
                                ) const;

public:     ////                Operations.
                                GrallocHelperImp();
                                ~GrallocHelperImp();

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IGrallocHelper Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                Operations.

    virtual int                 query(
                                    struct GrallocRequest const*    pRequest,
                                    struct GrallocStaticInfo*       pStaticInfo
                                ) const;

    virtual int                 query(
                                    buffer_handle_t                 bufHandle,
                                    struct GrallocStaticInfo*       pStaticInfo,
                                    struct GrallocDynamicInfo*      pDynamicInfo
                                ) const;

    virtual int                 setBufferStatus(
                                    buffer_handle_t                 bufHandle,
                                    int                             mask,
                                    int                             value
                                ) const;

    virtual int                 setDirtyCamera(
                                    buffer_handle_t                 bufHandle
                                ) const;

    virtual int                 setColorspace_JFIF(
                                    buffer_handle_t                 bufHandle
                                ) const;

    virtual int                 setBufferSOF(
                                     buffer_handle_t                bufHandle,
                                     uint64_t                       timestamp
                                ) const;

    virtual String8             queryPixelFormatName(int format) const
                                {
                                    return  gQueryPixelFormatName(format);
                                }

    virtual String8             queryGrallocUsageName(int usage) const
                                {
                                    return  gQueryGrallocUsageName(usage);
                                }

    virtual String8             queryDataspaceName(int32_t dataspace) const
                                {
                                    return  gQueryDataspaceName(dataspace);
                                }

    virtual void                dumpToLog() const;

    virtual bool                importBuffer(buffer_handle_t& handle);

    virtual void                freeBuffer(buffer_handle_t handle);

    virtual int                 lock(buffer_handle_t handle,
                                        uint32_t usage,
                                        int width,
                                        int height,
                                        void** vaddr);

    virtual int                 unlock(buffer_handle_t handle);

};


static GrallocHelperImp* getSingleton()
{
    static GrallocHelperImp singleton;
    return &singleton;
}


};  //namespace


/******************************************************************************
 *
 ******************************************************************************/
IGrallocHelper*
IGrallocHelper::
singleton()
{
    return getSingleton();
}


/******************************************************************************
 *
 ******************************************************************************/
GrallocHelperImp::
GrallocHelperImp()
    : mInitialized(false)
    , mLogLevel(0)
{
    FUNC_START;
    mLogLevel = ::property_get_int32("vendor.debug.camera.log", 0);
    if ( mLogLevel == 0 ) {
        mLogLevel = ::property_get_int32("debug.mtkcam.utils.gralloc.loglevel", 0);
    }
    //
    status_t err = initialize();
    MY_LOGE_IF(err, "%s initialize fail", __FUNCTION__);
    FUNC_END;
}

/******************************************************************************
 *
 ******************************************************************************/
GrallocHelperImp::
~GrallocHelperImp()
{
    FUNC_START;

    FUNC_END;
}

/******************************************************************************
 *
 ******************************************************************************/
status_t
GrallocHelperImp::
initialize()
{
    FUNC_START;
    //
    gMapper = std::make_unique<const NSCam::GrallocMapper::Mapper>();
    gAllocator = std::make_unique<NSCam::GrallocAllocator::Allocator>(*gMapper);
    if ( gMapper==nullptr || gAllocator==nullptr ) {
        return NO_INIT;
    }
    //
    mInitialized = true;
    FUNC_END;
    //
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
int
GrallocHelperImp::
setBufferStatus(
    buffer_handle_t                 bufHandle,
    int                             mask,
    int                             value
) const
{
    gralloc_extra_ion_sf_info_t info;
    gralloc_extra_query(bufHandle, GRALLOC_EXTRA_GET_IOCTL_ION_SF_INFO, &info);
    gralloc_extra_sf_set_status(&info, mask, value);
    return gralloc_extra_perform(bufHandle, GRALLOC_EXTRA_SET_IOCTL_ION_SF_INFO, &info);
}


/******************************************************************************
 *
 ******************************************************************************/
int
GrallocHelperImp::
setDirtyCamera(
    buffer_handle_t                 bufHandle
) const
{
    return setBufferStatus(
        bufHandle,
        GRALLOC_EXTRA_MASK_TYPE | GRALLOC_EXTRA_MASK_DIRTY,
        GRALLOC_EXTRA_BIT_TYPE_CAMERA | GRALLOC_EXTRA_BIT_DIRTY
    );
}


/******************************************************************************
 *
 ******************************************************************************/
int
GrallocHelperImp::
setColorspace_JFIF(
    buffer_handle_t                 bufHandle
) const
{
    return setBufferStatus(
        bufHandle,
        GRALLOC_EXTRA_MASK_YUV_COLORSPACE,
        GRALLOC_EXTRA_BIT_YUV_BT601_FULL
    );
}

/******************************************************************************
 *
 ******************************************************************************/
int
GrallocHelperImp::
setBufferSOF(
    buffer_handle_t                 bufHandle,
    uint64_t                        timestamp
) const
{
    return gralloc_extra_perform(bufHandle, GRALLOC_EXTRA_SET_TIMESTAMP, &timestamp);
}

/******************************************************************************
 *
 ******************************************************************************/
int
GrallocHelperImp::
query(
    struct GrallocRequest const*    pRequest,
    struct GrallocStaticInfo*       pStaticInfo
) const
{
    status_t status = OK;
    //
    if  ( ! pRequest ) {
        MY_LOGE("NULL pRequest");
        return  BAD_VALUE;
    }
    //
    if  ( ! pStaticInfo ) {
        MY_LOGE("NULL pStaticInfo");
        return  BAD_VALUE;
    }
    //
    MY_LOGD( "Grallc Request: u(0x%x) f(0x%x) w(%d) h(%d)",
             pRequest->usage, pRequest->format, pRequest->widthInPixels, pRequest->heightInPixels);
    MyRequest request;
    determine(*pRequest, request);
    //
    Mutex::Autolock _l(mLock);
    ssize_t const index = mMap.indexOfKey(request);
    //
    if  ( index < 0 )
    {
        MyStaticInfo staticInfo;
        status = determine(request, staticInfo);
        if  ( OK == status ) {
            addToMapLocked(request, staticInfo);
            *pStaticInfo = staticInfo;
            //
#if 1
            switch  (request.format)
            {
            case HAL_PIXEL_FORMAT_IMPLEMENTATION_DEFINED:
            case HAL_PIXEL_FORMAT_YCbCr_420_888:{
                    request.format = staticInfo.format;
                    request.usage = 0;
                    ssize_t const index = mMap.indexOfKey(request);
                    if  ( index < 0 ) {
                        addToMapLocked(request, staticInfo);
                    }
                }break;
            default:
                break;
            }
#endif
        }
        else {
            return  status;
        }
    }
    else
    {
        *pStaticInfo = mMap.valueAt(index);
    }
    //
    return  OK;
}


/******************************************************************************
 *
 ******************************************************************************/
int
GrallocHelperImp::
query(
    buffer_handle_t                 bufHandle,
    struct GrallocStaticInfo*       pStaticInfo,
    struct GrallocDynamicInfo*      pDynamicInfo
) const
{
    status_t status = OK;
    //
    if  ( ! bufHandle ) {
        MY_LOGE("NULL buffer_handle_t");
        return  BAD_VALUE;
    }
    //
    //
    Extra extra;
    if  ( OK != gQueryExtra(bufHandle, extra) ) {
        return UNKNOWN_ERROR;
    }
    //
    MyRequest request;
    request.usage = 0;
    request.format        = extra.format;
    request.widthInPixels = extra.widthInPixels;
    request.heightInPixels= extra.heightInPixels;
    //
    Mutex::Autolock _l(mLock);
#if 1
    ssize_t const index = mMap.indexOfKey(request);
    if  ( index >= 0 )
    {
        *pStaticInfo = mMap.valueAt(index);
    }
    else
#endif
    {
        MY_LOGW("Not found: %dx%d %s", request.widthInPixels, request.heightInPixels, gQueryPixelFormatName(request.format).string());
#if 1
        MyStaticInfo staticInfo;
        status = queryStaticInfo(bufHandle, staticInfo);
        if  ( OK != status ) {
            return  status;
        }
        //
        //  Add it to map if not existing.
        MyRequest request;
        request.usage = 0;
        request.format        = staticInfo.format;
        request.widthInPixels = staticInfo.widthInPixels;
        request.heightInPixels= staticInfo.heightInPixels;
        //
        ssize_t const index = mMap.indexOfKey(request);
        if  ( index < 0 )
        {
            addToMapLocked(request, staticInfo);
        }
        //
        if  ( NULL != pStaticInfo ) {
            *pStaticInfo = staticInfo;
        }
#endif
    }
    //
    if  ( NULL != pDynamicInfo ) {
        Vector<int>& ionFds = pDynamicInfo->ionFds;
        //
        int ionFd = -1;
        int err = ::gralloc_extra_query(bufHandle, GRALLOC_EXTRA_GET_ION_FD, &ionFd);
        if  ( GRALLOC_EXTRA_OK == err && 0 <= ionFd ) {
            ionFds.push_back(ionFd);
        }
        else {
            MY_LOGE("[buffer_handle_t:%p] err:%d ionFd:%d", bufHandle, err, ionFd);
            return  UNKNOWN_ERROR;
        }
    }
    //
    return status;
}


/******************************************************************************
 *
 ******************************************************************************/
status_t
GrallocHelperImp::
determine(GrallocRequest const& src, MyRequest& dst)
{
    dst = src;
    //
    switch  (src.format)
    {
    case HAL_PIXEL_FORMAT_IMPLEMENTATION_DEFINED:
    case HAL_PIXEL_FORMAT_YCbCr_420_888:
        dst.usage = src.usage;
        break;
    default:
        dst.usage = 0;
        break;
    }
    //
    return  OK;
}


/******************************************************************************
 *
 ******************************************************************************/
status_t
GrallocHelperImp::
determine(MyRequest const& src, MyStaticInfo& dst)
{
    nsecs_t const startTime = ::systemTime();
    //
    // already protected by mLock.
    // allocate null buffer for query static info.
    // free buffer handle before end of this function.
    uint32_t outStride = 0;
    buffer_handle_t handle;
    status_t err = gAllocator->allocate(src.widthInPixels, src.heightInPixels,
                        static_cast<GrallocAllocator::PixelFormat>(src.format), /*inLayerCount*/ 1,
                        static_cast<uint64_t>(src.usage|GRALLOC1_USAGE_NULL_BUFFER),
                        &handle, &outStride, /*graphicBufferId*/ 0, __FUNCTION__);
    //
    if ( OK!=err || handle==nullptr )
    {
        MY_LOGE(
            "Bad allcation handle:%p %dx%d format:%#x(%s) usage:%#x(%s)",
            handle, src.widthInPixels, src.heightInPixels,
            src.format, gQueryPixelFormatName(src.format).string(),
            src.usage, gQueryGrallocUsageName(src.usage).string()
        );
        return  NO_MEMORY;
    }
    nsecs_t const grallocTime = ::systemTime() - startTime;
    //
    dst.grallocTime = grallocTime;
    err = queryStaticInfo(handle, dst);
    //
    if (handle)
        gAllocator->free(handle);

    return err;
}


/******************************************************************************
 *
 ******************************************************************************/
ssize_t
GrallocHelperImp::
addToMapLocked(
    Map_t::key_type const& k,
    Map_t::value_type const& v
) const
{
    dump(k, v);
    return mMap.add(k, v);
}


/******************************************************************************
 *
 ******************************************************************************/
void
GrallocHelperImp::
dumpToLog() const
{
    MY_LOGD("+");
    Mutex::Autolock _l(mLock);
    for (size_t i = 0; i < mMap.size(); i++) {
        dump(mMap.keyAt(i), mMap.valueAt(i));
    }
    MY_LOGD("-");
}


/******************************************************************************
 *
 ******************************************************************************/
void
GrallocHelperImp::
dump(
    Map_t::key_type const& k,
    Map_t::value_type const& v
) const
{
    MY_LOGD("*****************************************************************************");
    MY_LOGD("gralloc time: %" PRId64 " us", ::nanoseconds_to_microseconds(v.grallocTime));
    MY_LOGD(
        "%dx%d usage:0x%08x(%s) format:0x%08x(%s) -->",
        k.widthInPixels, k.heightInPixels,
        k.usage, queryGrallocUsageName(k.usage).string(),
        k.format, queryPixelFormatName(k.format).string()
    );
    MY_LOGD(
        "%dx%d usage:0x%08x(%s) format:0x%08x(%s)",
        v.widthInPixels, v.heightInPixels,
        v.usage, queryGrallocUsageName(v.usage).string(),
        v.format, queryPixelFormatName(v.format).string()
    );
    for (size_t i = 0; i < v.planes.size(); i++) {
        MY_LOGD(
            "  [%zu] sizeInBytes:%zu rowStrideInBytes:%zu",
            i,
            v.planes[i].sizeInBytes,
            v.planes[i].rowStrideInBytes
        );
    }
}

/******************************************************************************
 *
 ******************************************************************************/
bool
GrallocHelperImp::
importBuffer(buffer_handle_t& handle)
{
    if (!handle->numFds && !handle->numInts) {
        handle = nullptr;
        return true;
    }
    //
    GrallocMapper::Error error;
    buffer_handle_t importedHandle;
    error = gMapper->importBuffer( hidl_handle(handle), &importedHandle);
    if ( error != Error::NONE )
    {
        MY_LOGE("importBuffer(%p) failed: %d", handle, error);
        return false;
    }
    handle = importedHandle;
    return true;
}

/******************************************************************************
 *
 ******************************************************************************/
void
GrallocHelperImp::
freeBuffer(buffer_handle_t handle)
{
    if (!handle) {
        return;
    }
    gMapper->freeBuffer(handle);
}

/******************************************************************************
 *
 ******************************************************************************/
int
GrallocHelperImp::
lock(
    buffer_handle_t handle,
    uint32_t usage,
    int width,
    int height,
    void** vaddr
)
{
    int fenceFd = -1;
    auto toGrallocRect = [](int32_t w, int32_t h)
    {
        NSCam::GrallocMapper::IMapper::Rect outRect =
        {
            .left   = 0,
            .top    = 0,
            .width  = w,
            .height = h,
        };
        return outRect;
    };
    //
    status_t error = static_cast<status_t>(gMapper->lock(
                            handle, static_cast<uint64_t>(usage),
                            toGrallocRect((int32_t)width, (int32_t)height), fenceFd, vaddr)
                            );
    MY_LOGE_IF( error, "lock(...) failed %d (%s)", error, strerror(-error));
    //
    return error;
}

/******************************************************************************
 *
 ******************************************************************************/
int
GrallocHelperImp::
unlock(buffer_handle_t handle)
{
    int32_t fenceFd = -1;
    fenceFd = gMapper->unlock(handle);
    ::sync_wait(fenceFd, -1);
    ::close(fenceFd);
    //
    return NO_ERROR;
}
