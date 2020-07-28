#ifndef BUFFERUTILS_H
#define BUFFERUTILS_H

#include <vndk/hardware_buffer.h>
#include <system/graphics-base-v1.0.h>
#include <vendor/mediatek/hardware/mms/1.2/IMms.h>
#include "utils/imgbuf/IImageBuffer.h"

using ::vendor::mediatek::hardware::mms::V1_2::IMms;
using ::vendor::mediatek::hardware::mms::V1_0::HwCopybitParam;
using android::hardware::hidl_handle;
using android::sp;
using NSCam::IImageBuffer;
using NSCam::MSize;


class BufferUtils {
public:
    static AHardwareBuffer* getAHWBuffer(IImageBuffer *buf);
    static hidl_handle getHidlHandle(IImageBuffer *buf);
    static int mdpCopyBit(const HwCopybitParam* copybitParam);
    static int mdpConvertFormatAndResize(IImageBuffer* inBuffer, IImageBuffer* outBuffer);
private:
    static sp<IMms> mIMmsService;
};
#endif // BUFFERUTILS_H
