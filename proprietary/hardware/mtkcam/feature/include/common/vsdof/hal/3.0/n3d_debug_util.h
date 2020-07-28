/********************************************************************************************
 *     LEGAL DISCLAIMER
 *
 *     (Header of MediaTek Software/Firmware Release or Documentation)
 *
 *     BY OPENING OR USING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *     THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE") RECEIVED
 *     FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON AN "AS-IS" BASIS
 *     ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED,
 *     INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR
 *     A PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY
 *     WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK
 *     ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
 *     NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION
 *     OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 *
 *     BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE LIABILITY WITH
 *     RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION,
 *     TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/
#ifndef STEREO_DEBUG_UTIL_H_
#define STEREO_DEBUG_UTIL_H_

#include <string>
#include <mtkcam/def/common.h>
#include <mtkcam/utils/imgbuf/IImageBuffer.h>
#include <utils/Mutex.h>
#include <thread>

#include <mtkcam/utils/std/Log.h>
#include <mtkcam/feature/stereo/hal/stereo_common.h>

namespace StereoHAL {

#define COMPRESS_STEREO_DEBUG_BUFFER    0
#define ENABLE_COMPRESS_BUFFER_PROPERTY "vendor.STEREO.compress_debug"

struct N3DDebugBuffer_T
{
    std::string             name;

    void                   *buffer           = NULL;
    NSCam::MSize            bufferSize;
    size_t                  bufferSizeInBytes = 0;
    MINT                    format            = NSCam::eImgFmt_BLOB;
    bool                    isCompressed      = false;

    N3DDebugBuffer_T() {}

    N3DDebugBuffer_T(std::string n, NSCam::IImageBuffer *img);

    N3DDebugBuffer_T(std::string n, void *buf, size_t len, NSCam::MSize sz=NSCam::MSize(), int format=NSCam::eImgFmt_BLOB);

    N3DDebugBuffer_T(N3DDebugBuffer_T &&src)
        : name(std::move(src.name))
    {
        buffer            = src.buffer;
        bufferSize        = src.bufferSize;
        bufferSizeInBytes = src.bufferSizeInBytes;
        format            = src.format;
        isCompressed      = src.isCompressed;

        src.buffer = nullptr;
        src.reset();

        // ALOGD("XXXX Move %s from %p to %p, buffer %p, len %zu", name.c_str(), &src, this, buffer, bufferSizeInBytes);
    }

    ~N3DDebugBuffer_T()
    {
        reset();
    }

    void reset()
    {
        std::string().swap(name);

        if(buffer) {
            delete [] (MUINT8*)buffer;
            buffer = NULL;
        }

        bufferSize.w      = 0;
        bufferSize.h      = 0;
        bufferSizeInBytes = 0;
        isCompressed      = false;
    }

    bool compress();
};

class N3DDebugUtil {
public:
    static N3DDebugUtil *getInstance(ENUM_STEREO_SCENARIO eScenario);
    static void destroyInstance(ENUM_STEREO_SCENARIO eScenario);

    virtual void addBuffer(N3DDebugBuffer_T &&buffer) = 0;
    virtual size_t getDebugBuffer(void *buffer) = 0;
    virtual size_t getDebugBufferSize() = 0;    //Call after all buffers are added

protected:
    virtual ~N3DDebugUtil() {}

private:
    static android::Mutex __instanceLock;
    static N3DDebugUtil *__pvInstance;
    static N3DDebugUtil *__capInstance;
};
#endif

};