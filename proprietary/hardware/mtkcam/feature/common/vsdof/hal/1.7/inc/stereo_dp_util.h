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
#ifndef _STEREO_DP_UTIL_H_
#define _STEREO_DP_UTIL_H_

#include <mtkcam/utils/std/Format.h>
#include <DpBlitStream.h>

#include <DpAsyncBlitStream.h>
#include <pthread.h>
#include <thread>
#include <sync.h>
#include <sw_sync.h>
#include <android/log.h>

#include <mtkcam/utils/imgbuf/IGrallocImageBufferHeap.h>
#include <ui/gralloc_extra.h>
#include <vsdof/hal/ProfileUtil.h>

#include <vndk/hardware_buffer.h>    //For AHardwareBuffer

#define STEREO_DP_UTIL_TAG "StereoDpUtil"

using namespace NSCam;
#define IS_ALLOC_GB    true

class StereoDpUtil {
public:
    //Notice: caller should create dst image with rotated size
    static bool transformImage(
                        IImageBuffer *imgSrc,
                        IImageBuffer *imgDst,
                        ENUM_ROTATION eRotateDegree = eRotate_0,
                        DpRect *srcROI = NULL,
                        DpRect *dstROI = NULL
                       )
    {
        if(NULL == imgSrc ||
           NULL == imgDst)
        {
            return false;
        }

        DpBlitStream   dpStream;
        void           *pBuffer[PLANE_COUNT];
        uint32_t       size[PLANE_COUNT];

        //Set src
        pBuffer[0] = (imgSrc->getPlaneCount() > 0) ? (void *)imgSrc->getBufVA(0) : NULL;
        pBuffer[1] = (imgSrc->getPlaneCount() > 1) ? (void *)imgSrc->getBufVA(1) : NULL;
        pBuffer[2] = (imgSrc->getPlaneCount() > 2) ? (void *)imgSrc->getBufVA(2) : NULL;

        size[0] = (imgSrc->getPlaneCount() > 0) ? imgSrc->getBufSizeInBytes(0) : 0;
        size[1] = (imgSrc->getPlaneCount() > 1) ? imgSrc->getBufSizeInBytes(1) : 0;
        size[2] = (imgSrc->getPlaneCount() > 2) ? imgSrc->getBufSizeInBytes(2) : 0;
        dpStream.setSrcBuffer(pBuffer, size, imgSrc->getPlaneCount());

        //Only support YV12, RGBA, Y8, NV12, NV21
        DpColorFormat imgFormat = __getDpImageFormat(imgSrc);
        if(DP_COLOR_UNKNOWN == imgFormat) {
            __android_log_print(ANDROID_LOG_ERROR, STEREO_DP_UTIL_TAG, "Unknown src format: 0x%x", imgSrc->getImgFormat());
            return false;
        }

        dpStream.setSrcConfig(imgSrc->getImgSize().w,
                              imgSrc->getImgSize().h,
                              imgSrc->getBufStridesInBytes(0),
                              (imgSrc->getPlaneCount() <= 1) ? 0 : imgSrc->getBufStridesInBytes(1),
                              imgFormat,
                              DP_PROFILE_FULL_BT601,
                              eInterlace_None,
                              srcROI,
                              DP_SECURE_NONE,
                              DO_FLUSH);

        // Set dst
        void           *pBufferOut[PLANE_COUNT];
        uint32_t       sizeOut[PLANE_COUNT];
        pBufferOut[0] = (imgDst->getPlaneCount() > 0) ? (void *)imgDst->getBufVA(0) : NULL;
        pBufferOut[1] = (imgDst->getPlaneCount() > 1) ? (void *)imgDst->getBufVA(1) : NULL;
        pBufferOut[2] = (imgDst->getPlaneCount() > 2) ? (void *)imgDst->getBufVA(2) : NULL;

        sizeOut[0] = (imgDst->getPlaneCount() > 0) ? imgDst->getBufSizeInBytes(0) : 0;
        sizeOut[1] = (imgDst->getPlaneCount() > 1) ? imgDst->getBufSizeInBytes(1) : 0;
        sizeOut[2] = (imgDst->getPlaneCount() > 2) ? imgDst->getBufSizeInBytes(2) : 0;

        dpStream.setDstBuffer(pBufferOut, sizeOut, imgDst->getPlaneCount());

        //Only support YV12, RGBA, Y8, NV12, NV21
        imgFormat = __getDpImageFormat(imgDst);
        if(DP_COLOR_UNKNOWN == imgFormat) {
            __android_log_print(ANDROID_LOG_ERROR, STEREO_DP_UTIL_TAG, "Unknown dst format: 0x%x", imgDst->getImgFormat());
            return false;
        }

        if(dstROI) {
            dpStream.setDstConfig(dstROI->w,
                                  dstROI->h,
                                  imgDst->getBufStridesInBytes(0),
                                  (imgDst->getPlaneCount() <= 1) ? 0 : imgDst->getBufStridesInBytes(1),
                                  imgFormat,
                                  DP_PROFILE_FULL_BT601,
                                  eInterlace_None,
                                  dstROI,
                                  DP_SECURE_NONE,
                                  DO_FLUSH);
        } else {
            dpStream.setDstConfig(imgDst->getImgSize().w,
                                  imgDst->getImgSize().h,
                                  imgDst->getBufStridesInBytes(0),
                                  (imgDst->getPlaneCount() <= 1) ? 0 : imgDst->getBufStridesInBytes(1),
                                  imgFormat,
                                  DP_PROFILE_FULL_BT601,
                                  eInterlace_None,
                                  NULL,
                                  DP_SECURE_NONE,
                                  DO_FLUSH);
        }

        dpStream.setRotate(eRotateDegree);
        dpStream.invalidate();

        return true;
    }

    static bool transformImage(
                        IImageBuffer *imgSrc,
                        IImageBuffer *imgDst1,
                        IImageBuffer *imgDst2,
                        DpRect *srcROI = NULL,
                        ENUM_ROTATION eRotateDst1=eRotate_0,
                        DpRect *dstROI1 = NULL,
                        ENUM_ROTATION eRotateDst2=eRotate_0,
                        DpRect *dstROI2 = NULL
                       )
    {
        if(NULL == imgSrc ||
           NULL == imgDst1 ||
           NULL == imgDst2)
        {
            return false;
        }

        DpAsyncBlitStream dpStream;
        uint32_t          job[2];
        int32_t           fence[2];

        dpStream.createJob(job[0], fence[0]);
        dpStream.createJob(job[1], fence[1]);

        for(int i = 0; i < 2; ++i) {
            dpStream.setConfigBegin(job[i]);

            __setSrc(dpStream, imgSrc);
            __setDst(dpStream, 0, imgDst1, eRotateDst1, dstROI1);
            __setDst(dpStream, 1, imgDst2, eRotateDst2, dstROI2);
            if(srcROI) {
                dpStream.setSrcCrop(0, *srcROI);
                dpStream.setSrcCrop(1, *srcROI);
            }

            dpStream.setConfigEnd();
        }

        std::thread dpThread(
            [fence]() mutable {
            // Wake up
            sync_wait(fence[0], -1);
            close(fence[0]);

            sync_wait(fence[1], -1);
            close(fence[1]);
        }
        );

        dpStream.invalidate();
        dpStream.invalidate();

        dpThread.join();

        return true;
    }

    //Create IImgeBuffer from buffer, only support YV12 and mask
    static bool allocImageBuffer(const char *TAG, EImageFormat fmt, MSize size, bool isGB, sp<IImageBuffer>& retImage)
    {
        if( NULL == TAG
            || 0 == strlen(TAG)
            || ( eImgFmt_YV12     != fmt &&
                 eImgFmt_I420     != fmt &&
                 eImgFmt_Y8       != fmt &&
                 eImgFmt_RGBA8888 != fmt) )
        {
            return false;
        }

        IImageBufferAllocator* allocator = IImageBufferAllocator::getInstance();
        if( NULL == allocator ) {
            __android_log_write(ANDROID_LOG_ERROR, TAG, "cannot get allocator");
            return false;
        }

        MUINT const PLANE_COUNT = NSCam::Utils::Format::queryPlaneCount(fmt);
        MINT32 bufBoundaryInBytes[] = {0, 0, 0};
        MUINT32 bufStridesInBytes[] = {0, 0, 0};

        for (MUINT i = 0; i < PLANE_COUNT; i++)
        {
            bufStridesInBytes[i] =
                (NSCam::Utils::Format::queryPlaneWidthInPixels(fmt, i, size.w) * NSCam::Utils::Format::queryPlaneBitsPerPixel(fmt, i))>>3;
        }

        if(StereoSettingProvider::isLogEnabled()) {
            switch(fmt) {
                case eImgFmt_Y8:
                    __android_log_print(ANDROID_LOG_DEBUG, TAG, "alloc %d x %d, fmt 0x%X(eImgFmt_Y8)", size.w, size.h, fmt);
                    break;
                case eImgFmt_RGBA8888:
                    __android_log_print(ANDROID_LOG_DEBUG, TAG, "alloc %d x %d, fmt 0x%X(eImgFmt_RGBA8888)", size.w, size.h, fmt);
                    break;
                case eImgFmt_I420:
                    __android_log_print(ANDROID_LOG_DEBUG, TAG, "alloc %d x %d, fmt 0x%X(eImgFmt_I420)", size.w, size.h, fmt);
                    break;
                case eImgFmt_YV12:
                default:
                    __android_log_print(ANDROID_LOG_DEBUG, TAG, "alloc %d x %d, fmt 0x%X(eImgFmt_YV12)", size.w, size.h, fmt);
                    break;
            }
        }

        IImageBufferAllocator::ImgParam imgParam(
                fmt,
                size,
                bufStridesInBytes,
                bufBoundaryInBytes,
                PLANE_COUNT
                );

        //
        MUINT32 gbusage = eBUFFER_USAGE_HW_RENDER|eBUFFER_USAGE_HW_TEXTURE|eBUFFER_USAGE_SW_READ_OFTEN|eBUFFER_USAGE_SW_WRITE_OFTEN;
        if(isGB)
        {
            IImageBufferAllocator::ExtraParam extParam(gbusage);
            retImage = allocator->alloc_gb(TAG, imgParam, extParam);

            // Enable GPU full mode
            AHardwareBuffer* gbuf;
            IImageBufferHeap* pDstHeap  = retImage->getImageBufferHeap();
            gbuf = (AHardwareBuffer*)(pDstHeap->getHWBuffer());
            buffer_handle_t handle = AHardwareBuffer_getNativeHandle(gbuf);

            gralloc_extra_ion_sf_info_t info;
            gralloc_extra_query(handle, GRALLOC_EXTRA_GET_IOCTL_ION_SF_INFO, &info);
            gralloc_extra_sf_set_status(&info, GRALLOC_EXTRA_MASK_YUV_COLORSPACE, GRALLOC_EXTRA_BIT_YUV_BT601_FULL);
            gralloc_extra_perform(handle, GRALLOC_EXTRA_SET_IOCTL_ION_SF_INFO, &info);
        }
        else
        {
            retImage = allocator->alloc_ion(TAG, imgParam);
        }

        if( NULL == retImage.get() ) {
            return false;
        }

        if(isGB) {
            if ( !retImage->lockBuf( TAG, gbusage ) )
            {
                __android_log_write(ANDROID_LOG_ERROR, TAG, "lock GBuffer failed");
                return false;
            }
        } else {
            if ( !retImage->lockBuf( TAG, eBUFFER_USAGE_SW_MASK | eBUFFER_USAGE_HW_MASK ) )
            {
                __android_log_write(ANDROID_LOG_ERROR, TAG, "lock Buffer failed");
                return false;
            }
        }

        // if ( !retImage->syncCache( eCACHECTRL_INVALID ) )
        // {
        //     __android_log_write(ANDROID_LOG_ERROR, TAG, "syncCache failed");
        //     return false;
        // }

        return true;
    }

    static bool freeImageBuffer(const char *TAG, sp<IImageBuffer>& imgBuf)
    {
        if( NULL == TAG
            || 0 == strlen(TAG)
            || NULL == imgBuf.get() )
        {
            return false;
        }

        IImageBufferAllocator* allocator = IImageBufferAllocator::getInstance();
        if( !imgBuf->unlockBuf( TAG ) )
        {
            __android_log_write(ANDROID_LOG_ERROR, TAG, "unlock Buffer failed");
            return false;
        }

        allocator->free(imgBuf.get());
        imgBuf = NULL;

        return true;
    }

    static bool rotateBuffer(const char *TAG,
                             const MUINT8 *SRC_BUFFER,
                             const MSize SRC_SIZE,
                             MUINT8 *dstBuffer,
                             ENUM_ROTATION targetRotation,
                             StereoArea targetROI=STEREO_AREA_ZERO,
                             const bool USE_MDP=false)
    {
        if(NULL == SRC_BUFFER ||
           NULL == dstBuffer ||
           0 == SRC_SIZE.w ||
           0 == SRC_SIZE.h)
        {
            return false;
        }

        char logName[100];
        sprintf(logName, "RotateBuffer: src size %dx%d, target ROI (%d, %d) %dx%d rotate %d, MDP %d",
                SRC_SIZE.w, SRC_SIZE.h,
                targetROI.startPt.x, targetROI.startPt.y, targetROI.size.w, targetROI.size.h,
                targetRotation, USE_MDP);
        AutoProfileUtil profile(TAG, logName);

        const MINT32 BUFFER_LEN = SRC_SIZE.w * SRC_SIZE.h;

        if(USE_MDP) {
            sp<IImageBuffer> srcImage;
            if(allocImageBuffer(LOG_TAG, eImgFmt_Y8, SRC_SIZE, !IS_ALLOC_GB, srcImage)) {
                //Copy Depth map to image buffer
                ::memcpy((void*)srcImage.get()->getBufVA(0), SRC_BUFFER, BUFFER_LEN);

                sp<IImageBuffer> dstImage;
                targetROI.rotate(targetRotation);

                if(allocImageBuffer(LOG_TAG, eImgFmt_Y8, targetROI.size, !IS_ALLOC_GB, dstImage)) {
                    //Rotate by MDP
                    if(targetROI.size.w != 0 ||
                       targetROI.size.h != 0)
                    {
                        MSize contentSize = targetROI.contentSize();
                        DpRect dstROI(targetROI.startPt.x, targetROI.startPt.y, contentSize.w, contentSize.h);
                        transformImage(srcImage.get(), dstImage.get(), targetRotation, NULL, &dstROI);
                    } else {
                        transformImage(srcImage.get(), dstImage.get(), targetRotation);
                    }

                    ::memcpy(dstBuffer, (void*)dstImage.get()->getBufVA(0), targetROI.size.w*targetROI.size.h);
                    freeImageBuffer(LOG_TAG, dstImage);
                } else {
                    freeImageBuffer(LOG_TAG, srcImage);
                    return false;
                }

                freeImageBuffer(LOG_TAG, srcImage);
            } else {
                return false;
            }

            return true;
        }

        //Rotate by CPU
        MINT32 writeRow = 0;
        MINT32 writeCol = 0;
        MINT32 writePos = 0;

        if(targetROI == STEREO_AREA_ZERO) {
            if(targetRotation & 0x2) {
                targetROI.size.w = SRC_SIZE.h;
                targetROI.size.h = SRC_SIZE.w;
            } else {
                targetROI.size.w = SRC_SIZE.w;
                targetROI.size.h = SRC_SIZE.h;
            }
        }

        if(eRotate_0 == targetRotation) {
            if(targetROI.size.w != SRC_SIZE.w ||
               targetROI.size.h != SRC_SIZE.h)
            {
                ::memset(dstBuffer, 0, sizeof(*dstBuffer) * targetROI.size.w * targetROI.size.h);
                dstBuffer += targetROI.startPt.y * targetROI.size.w + targetROI.startPt.x;
                for(writeRow = 0; writeRow < SRC_SIZE.h; ++writeRow) {
                    ::memcpy(dstBuffer, SRC_BUFFER, SRC_SIZE.w);
                    dstBuffer += targetROI.size.w;
                    SRC_BUFFER += SRC_SIZE.w;
                }
            } else {
                ::memcpy(dstBuffer, SRC_BUFFER, BUFFER_LEN);
            }
        } else if(eRotate_90 == targetRotation) {
            writeRow = SRC_SIZE.w;  //only for counting
            writeCol = (targetROI.startPt.y+SRC_SIZE.w-1) * targetROI.size.w + targetROI.startPt.x;
            writePos = writeCol;
            for(int i = BUFFER_LEN-1; i >= 0; --i) {
                *(dstBuffer + writePos) = *(SRC_BUFFER + i);

                writePos -= targetROI.size.w;
                --writeRow;
                if(writeRow <= 0) {
                    writeRow = SRC_SIZE.w;
                    ++writeCol;
                    writePos = writeCol;
                }
            }
        } else if(eRotate_270 == targetRotation) {
            writeRow = 0;
            writeCol = targetROI.startPt.y * targetROI.size.w + targetROI.startPt.x + SRC_SIZE.h - 1;
            writePos = writeCol;
            for(int i = BUFFER_LEN-1; i >= 0; --i) {
                *(dstBuffer + writePos) = *(SRC_BUFFER + i);

                writePos += targetROI.size.w;
                ++writeRow;
                if(writeRow >= SRC_SIZE.w) {
                    writeRow = 0;
                    --writeCol;
                    writePos = writeCol;
                }
            }
        } else if(eRotate_180 == targetRotation) {
            writeRow = targetROI.startPt.y;
            writeCol = 0;
            writePos = writeRow * targetROI.size.w + targetROI.startPt.x;
            for(int i = BUFFER_LEN-1; i >= 0; --i) {
                *(dstBuffer + writePos) = *(SRC_BUFFER + i);

                ++writePos;
                ++writeCol;
                if(writeCol >= SRC_SIZE.w) {
                    ++writeRow;
                    writeCol = 0;
                    writePos += targetROI.padding.w;
                }
            }
        }

        return true;
    }

private:
    static const MUINT32  PLANE_COUNT = 3;
    static const bool     DO_FLUSH = true;

    static DpColorFormat __getDpImageFormat(IImageBuffer *img) {
        if(img) {
            //Only support YV12, RGBA, Y8, NV12, NV21
            switch(img->getImgFormat()) {
            case eImgFmt_YV12:
                return DP_COLOR_YV12;
                break;
            case eImgFmt_RGBA8888:
                return DP_COLOR_RGBA8888;
                break;
            case eImgFmt_Y8:
                return DP_COLOR_GREY;
                break;
            case eImgFmt_NV12:
                return DP_COLOR_NV12;
                break;
            case eImgFmt_NV21:
                return DP_COLOR_NV21;
                break;
            case eImgFmt_YUY2:
                return DP_COLOR_YUY2;
            case eImgFmt_STA_2BYTE:
                // just need the 2 bytes per pixel, so use this maping: eImgFmt_STA_2BYTE -> DP_COLOR_RGB565_RAW
                return DP_COLOR_RGB565_RAW;
                break;
            case eImgFmt_I420:
                return DP_COLOR_I420;
                break;
            default:
                break;
            }
        }

        return DP_COLOR_UNKNOWN;
    }

    static bool __setSrc(DpAsyncBlitStream &dpStream, IImageBuffer *imgSrc)
    {
        if(NULL == imgSrc) {
            return false;
        }

        DpColorFormat imgFormat = __getDpImageFormat(imgSrc);
        if(DP_COLOR_UNKNOWN == imgFormat) {
            return false;
        }

        void        *pBuffer[PLANE_COUNT];
        uint32_t    size[PLANE_COUNT];

        pBuffer[0] = (imgSrc->getPlaneCount() > 0) ? (void *)imgSrc->getBufVA(0) : NULL;
        pBuffer[1] = (imgSrc->getPlaneCount() > 1) ? (void *)imgSrc->getBufVA(1) : NULL;
        pBuffer[2] = (imgSrc->getPlaneCount() > 2) ? (void *)imgSrc->getBufVA(2) : NULL;

        size[0] = (imgSrc->getPlaneCount() > 0) ? imgSrc->getBufSizeInBytes(0) : 0;
        size[1] = (imgSrc->getPlaneCount() > 1) ? imgSrc->getBufSizeInBytes(1) : 0;
        size[2] = (imgSrc->getPlaneCount() > 2) ? imgSrc->getBufSizeInBytes(2) : 0;
        dpStream.setSrcBuffer(pBuffer, size, imgSrc->getPlaneCount());

        dpStream.setSrcConfig(imgSrc->getImgSize().w,
                              imgSrc->getImgSize().h,
                              imgSrc->getBufStridesInBytes(0),
                              (imgSrc->getPlaneCount() <= 1) ? 0 : imgSrc->getBufStridesInBytes(1),
                              imgFormat,
                              DP_PROFILE_FULL_BT601,
                              eInterlace_None,
                              DP_SECURE_NONE,
                              DO_FLUSH);

        return true;
    }

    static bool __setDst(DpAsyncBlitStream &dpStream,
                         const int PORT_INDEX,
                         IImageBuffer *imgDst,
                         ENUM_ROTATION eRotateDegree = eRotate_0,
                         DpRect *dstROI = NULL)
    {
        if(NULL == imgDst) {
            return false;
        }

        // Set dst
        DpColorFormat imgFormat = __getDpImageFormat(imgDst);
        if(DP_COLOR_UNKNOWN == imgFormat) {
            return false;
        }

        void        *pBuffer[PLANE_COUNT];
        uint32_t    size[PLANE_COUNT];

        pBuffer[0] = (imgDst->getPlaneCount() > 0) ? (void *)imgDst->getBufVA(0) : NULL;
        pBuffer[1] = (imgDst->getPlaneCount() > 1) ? (void *)imgDst->getBufVA(1) : NULL;
        pBuffer[2] = (imgDst->getPlaneCount() > 2) ? (void *)imgDst->getBufVA(2) : NULL;

        size[0] = (imgDst->getPlaneCount() > 0) ? imgDst->getBufSizeInBytes(0) : 0;
        size[1] = (imgDst->getPlaneCount() > 1) ? imgDst->getBufSizeInBytes(1) : 0;
        size[2] = (imgDst->getPlaneCount() > 2) ? imgDst->getBufSizeInBytes(2) : 0;

        dpStream.setDstBuffer(PORT_INDEX, pBuffer, size, imgDst->getPlaneCount());

        if(dstROI) {
            dpStream.setDstConfig(PORT_INDEX,
                                  dstROI->w,
                                  dstROI->h,
                                  imgDst->getBufStridesInBytes(0),
                                  (imgDst->getPlaneCount() <= 1) ? 0 : imgDst->getBufStridesInBytes(1),
                                  imgFormat,
                                  DP_PROFILE_FULL_BT601,
                                  eInterlace_None,
                                  dstROI,
                                  DP_SECURE_NONE,
                                  DO_FLUSH);
        } else {
            dpStream.setDstConfig(PORT_INDEX,
                                  imgDst->getImgSize().w,
                                  imgDst->getImgSize().h,
                                  imgDst->getBufStridesInBytes(0),
                                  (imgDst->getPlaneCount() <= 1) ? 0 : imgDst->getBufStridesInBytes(1),
                                  imgFormat,
                                  DP_PROFILE_FULL_BT601,
                                  eInterlace_None,
                                  NULL,
                                  DP_SECURE_NONE,
                                  DO_FLUSH);
        }

        dpStream.setRotate(PORT_INDEX, eRotateDegree);
        return true;
    }
};
#endif
