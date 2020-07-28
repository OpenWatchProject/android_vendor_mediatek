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
 * MediaTek Inc. (C) 2017. All rights reserved.
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

#ifndef _ANDROID_MEDIA_UTILS_H_
#define _ANDROID_MEDIA_UTILS_H_

#include "src/piex_types.h"
#include "src/piex.h"

#include <android_runtime/AndroidRuntime.h>
#include <camera3.h>
#include <gui/CpuConsumer.h>
#include <jni.h>
#include <JNIHelp.h>
#include <utils/KeyedVector.h>
#include <utils/String8.h>
#include <SkStream.h>

namespace android {

class AssetStream : public piex::StreamInterface {
private:
    SkStream *mStream;
    size_t mPosition;

public:
    AssetStream(SkStream* stream);
    ~AssetStream();

    // Reads 'length' amount of bytes from 'offset' to 'data'. The 'data' buffer
    // provided by the caller, guaranteed to be at least "length" bytes long.
    // On 'kOk' the 'data' pointer contains 'length' valid bytes beginning at
    // 'offset' bytes from the start of the stream.
    // Returns 'kFail' if 'offset' + 'length' exceeds the stream and does not
    // change the contents of 'data'.
    piex::Error GetData(
            const size_t offset, const size_t length, std::uint8_t* data) override;
};

class BufferedStream : public piex::StreamInterface {
private:
    SkStream *mStream;
    // Growable memory stream
    SkDynamicMemoryWStream mStreamBuffer;

    // Minimum size to read on filling the buffer.
    const size_t kMinSizeToRead = 8192;

public:
    BufferedStream(SkStream* stream);
    ~BufferedStream();

    // Reads 'length' amount of bytes from 'offset' to 'data'. The 'data' buffer
    // provided by the caller, guaranteed to be at least "length" bytes long.
    // On 'kOk' the 'data' pointer contains 'length' valid bytes beginning at
    // 'offset' bytes from the start of the stream.
    // Returns 'kFail' if 'offset' + 'length' exceeds the stream and does not
    // change the contents of 'data'.
    piex::Error GetData(
            const size_t offset, const size_t length, std::uint8_t* data) override;
};

class FileStream : public piex::StreamInterface {
private:
    FILE *mFile;
    size_t mPosition;

public:
    FileStream(const int fd);
    FileStream(const String8 filename);
    ~FileStream();

    // Reads 'length' amount of bytes from 'offset' to 'data'. The 'data' buffer
    // provided by the caller, guaranteed to be at least "length" bytes long.
    // On 'kOk' the 'data' pointer contains 'length' valid bytes beginning at
    // 'offset' bytes from the start of the stream.
    // Returns 'kFail' if 'offset' + 'length' exceeds the stream and does not
    // change the contents of 'data'.
    piex::Error GetData(
            const size_t offset, const size_t length, std::uint8_t* data) override;
    bool exists() const;
};

// Reads EXIF metadata from a given raw image via piex.
// And returns true if the operation is successful; otherwise, false.
bool GetExifFromRawImage(
        piex::StreamInterface* stream, const String8& filename, piex::PreviewImageData& image_data);

// Returns true if the conversion is successful; otherwise, false.
bool ConvertKeyValueArraysToKeyedVector(
        JNIEnv *env, jobjectArray keys, jobjectArray values,
        KeyedVector<String8, String8>* vector);

struct AMessage;
status_t ConvertMessageToMap(
        JNIEnv *env, const sp<AMessage> &msg, jobject *map);

status_t ConvertKeyValueArraysToMessage(
        JNIEnv *env, jobjectArray keys, jobjectArray values,
        sp<AMessage> *msg);

// -----------Utility functions used by ImageReader/Writer JNI-----------------

typedef CpuConsumer::LockedBuffer LockedImage;

bool usingRGBAToJpegOverride(int32_t imageFormat, int32_t containerFormat);

int32_t applyFormatOverrides(int32_t imageFormat, int32_t containerFormat);

uint32_t Image_getJpegSize(LockedImage* buffer, bool usingRGBAOverride);

bool isFormatOpaque(int format);

bool isPossiblyYUV(PixelFormat format);

status_t getLockedImageInfo(LockedImage* buffer, int idx, int32_t containerFormat,
        uint8_t **base, uint32_t *size, int *pixelStride, int *rowStride);

status_t lockImageFromBuffer(sp<GraphicBuffer> buffer, uint32_t inUsage,
        const Rect& rect, int fenceFd, LockedImage* outputImage);

status_t lockImageFromBuffer(BufferItem* bufferItem, uint32_t inUsage,
        int fenceFd, LockedImage* outputImage);

int getBufferWidth(BufferItem *buffer);

int getBufferHeight(BufferItem *buffer);

};  // namespace android

#endif //  _ANDROID_MEDIA_UTILS_H_
