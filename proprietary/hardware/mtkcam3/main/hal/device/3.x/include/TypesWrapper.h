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

#ifndef _MTK_HARDWARE_MTKCAM_MAIN_HAL_DEVICE_3_X_INCLUDE_TYPES_WRAPPER_H_
#define _MTK_HARDWARE_MTKCAM_MAIN_HAL_DEVICE_3_X_INCLUDE_TYPES_WRAPPER_H_
//
#include <android/hardware/camera/device/3.4/types.h>
#include <android/hardware/camera/device/3.5/types.h>
#include <typeinfo>

using namespace ::android::hardware::camera::device;

/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {
namespace v3 {

struct WrappedStream;
struct WrappedStreamConfiguration;
struct WrappedHalStream;
struct WrappedHalStreamConfiguration;
struct WrappedCaptureRequest;
struct WrappedCaptureResult;

// New in V3_4, not to wrap now
// struct WrappedPhysicalCameraSetting;
// struct WrappedPhysicalCameraMetadata;

struct WrappedStream
{
    WrappedStream(const V3_2::Stream &p)
    {
        __v34Instance.v3_2 = p;
    }

    WrappedStream(const V3_4::Stream &p)
    {
        __v34Instance = p;
    }

    operator V3_2::Stream() const { return __v34Instance.v3_2; }
    operator V3_2::Stream&() { return __v34Instance.v3_2; }

    operator V3_4::Stream() const { return __v34Instance; }
    operator V3_4::Stream&() { return __v34Instance; }

private:
    V3_4::Stream __v34Instance;
};

struct WrappedStreamConfiguration
{
public:
    WrappedStreamConfiguration(const V3_2::StreamConfiguration &p)
    {
        __v35Instance.v3_4.streams = ::android::hardware::hidl_vec<V3_4::Stream>(p.streams.size());
        int i = 0;
        for(auto &s : p.streams) {
            (V3_4::Stream)__v35Instance.v3_4.streams[i++] = (V3_4::Stream&)WrappedStream(s);
        }
        __v35Instance.v3_4.operationMode = p.operationMode;
        __v35Instance.streamConfigCounter = 0;
    }

    WrappedStreamConfiguration(const V3_4::StreamConfiguration &p)
    {
        __v35Instance.v3_4 = p;
        __v35Instance.streamConfigCounter = 0;
    }

    WrappedStreamConfiguration(const V3_5::StreamConfiguration &p)
    {
        __v35Instance = p;
    }

    operator V3_2::StreamConfiguration() const
    {
        V3_2::StreamConfiguration v32Instance;
        v32Instance.streams = ::android::hardware::hidl_vec<V3_2::Stream>(__v35Instance.v3_4.streams.size());
        int i = 0;
        for(auto &s : __v35Instance.v3_4.streams) {
            (V3_2::Stream)v32Instance.streams[i++] = (V3_2::Stream&)WrappedStream(s);
        }
        v32Instance.operationMode = __v35Instance.v3_4.operationMode;
        return v32Instance;
    }

    operator V3_2::StreamConfiguration&()
    {
        __v32Instance.streams = ::android::hardware::hidl_vec<V3_2::Stream>(__v35Instance.v3_4.streams.size());
        int i = 0;
        for(auto &s : __v35Instance.v3_4.streams) {
            (V3_2::Stream)__v32Instance.streams[i++] = (V3_2::Stream&)WrappedStream(s);
        }
        __v32Instance.operationMode = __v35Instance.v3_4.operationMode;
        return __v32Instance;
    }

    operator V3_4::StreamConfiguration() const { return __v35Instance.v3_4; }
    operator V3_4::StreamConfiguration&() { return __v35Instance.v3_4; }

    operator V3_5::StreamConfiguration() const { return __v35Instance; }
    operator V3_5::StreamConfiguration&() { return __v35Instance; }

private:
    V3_5::StreamConfiguration __v35Instance;
    V3_2::StreamConfiguration __v32Instance;
};

struct WrappedHalStream
{
    WrappedHalStream(const V3_2::HalStream &p)
    {
        __v34Instance.v3_3.v3_2 = p;

        // No value:
        // __v34Instance.v3_3.overrideDataSpace
        // __v34Instance.physicalCameraId
    }

    WrappedHalStream(const V3_4::HalStream &p)
    {
        __v34Instance = p;
    }

    operator V3_2::HalStream() const { return __v34Instance.v3_3.v3_2; }
    operator V3_2::HalStream&() { return __v34Instance.v3_3.v3_2; }

    operator V3_3::HalStream() const { return __v34Instance.v3_3; }
    operator V3_3::HalStream&() { return __v34Instance.v3_3; }

    operator V3_4::HalStream() const { return __v34Instance; }
    operator V3_4::HalStream&() { return __v34Instance; }

private:
    V3_4::HalStream __v34Instance;
};

struct WrappedHalStreamConfiguration
{
    WrappedHalStreamConfiguration() {}

    WrappedHalStreamConfiguration(const V3_2::HalStreamConfiguration &p)
    {
        __v34Instance.streams = ::android::hardware::hidl_vec<V3_4::HalStream>(p.streams.size());
        int i = 0;
        for(auto &s : p.streams) {
            (V3_4::HalStream)__v34Instance.streams[i++] = (V3_4::HalStream&)WrappedHalStream(s);
        }
    }

    WrappedHalStreamConfiguration(V3_2::HalStreamConfiguration &p)
    {
        __v34Instance.streams = ::android::hardware::hidl_vec<V3_4::HalStream>(p.streams.size());
        int i = 0;
        for(auto &s : p.streams) {
            (V3_4::HalStream)__v34Instance.streams[i++] = (V3_4::HalStream&)WrappedHalStream(s);
        }
    }

    WrappedHalStreamConfiguration(const V3_4::HalStreamConfiguration &p)
    {
        __v34Instance = p;
    }

    operator V3_2::HalStreamConfiguration() const
    {
        V3_2::HalStreamConfiguration v32Instance;
        v32Instance.streams = ::android::hardware::hidl_vec<V3_2::HalStream>(__v34Instance.streams.size());
        int i = 0;
        for(auto &s : __v34Instance.streams) {
            (V3_2::HalStream)v32Instance.streams[i++] = (V3_2::HalStream&)WrappedHalStream(s);
        }

        return v32Instance;
    }

    operator V3_2::HalStreamConfiguration&()
    {
        __v32Instance.streams = ::android::hardware::hidl_vec<V3_2::HalStream>(__v34Instance.streams.size());
        int i = 0;
        for(auto &s : __v34Instance.streams) {
            (V3_2::HalStream)__v32Instance.streams[i++] = (V3_2::HalStream&)WrappedHalStream(s);
        }

        return __v32Instance;
    }

    operator V3_3::HalStreamConfiguration() const
    {
        V3_3::HalStreamConfiguration v33Instance;
        v33Instance.streams = ::android::hardware::hidl_vec<V3_3::HalStream>(__v34Instance.streams.size());
        int i = 0;
        for(auto &s : __v34Instance.streams) {
            (V3_3::HalStream)v33Instance.streams[i++] = (V3_3::HalStream&)WrappedHalStream(s);
        }

        return v33Instance;
    }

    operator V3_3::HalStreamConfiguration&()
    {
        __v33Instance.streams = ::android::hardware::hidl_vec<V3_3::HalStream>(__v34Instance.streams.size());
        int i = 0;
        for(auto &s : __v34Instance.streams) {
            (V3_3::HalStream)__v33Instance.streams[i++] = (V3_3::HalStream&)WrappedHalStream(s);
        }

        return __v33Instance;
    }

    operator V3_4::HalStreamConfiguration() const { return __v34Instance; }
    operator V3_4::HalStreamConfiguration&() { return __v34Instance; }

private:
    V3_4::HalStreamConfiguration __v34Instance;
    V3_3::HalStreamConfiguration __v33Instance;
    V3_2::HalStreamConfiguration __v32Instance;
};

struct WrappedCaptureRequest
{
    WrappedCaptureRequest(const V3_2::CaptureRequest &p)
    {
        __v34Instance.v3_2 = p;
    }

    WrappedCaptureRequest(const V3_4::CaptureRequest &p)
    {
        __v34Instance = p;
    }

    operator V3_2::CaptureRequest() const { return __v34Instance.v3_2; }
    operator V3_2::CaptureRequest&() { return __v34Instance.v3_2; }

    operator V3_4::CaptureRequest() const { return __v34Instance; }
    operator V3_4::CaptureRequest&() { return __v34Instance; }
private:
    V3_4::CaptureRequest __v34Instance;
};

struct WrappedCaptureResult
{
    WrappedCaptureResult(const V3_2::CaptureResult &p)
    {
        __v34Instance.v3_2 = p;
    }

    WrappedCaptureResult(const V3_4::CaptureResult &p)
    {
        __v34Instance = p;
    }

    operator V3_2::CaptureResult() const { return __v34Instance.v3_2; }
    operator V3_2::CaptureResult&() { return __v34Instance.v3_2; }

    operator V3_4::CaptureResult() const { return __v34Instance; }
    operator V3_4::CaptureResult&() { return __v34Instance; }

private:
    V3_4::CaptureResult __v34Instance;
};

/******************************************************************************
 *
 ******************************************************************************/
};  //namespace v3
};  //namespace NSCam
#endif  //_MTK_HARDWARE_MTKCAM_MAIN_HAL_DEVICE_3_X_INCLUDE_TYPES_WRAPPER_H_

