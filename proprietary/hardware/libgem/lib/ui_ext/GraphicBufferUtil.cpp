#define LOG_TAG "GraphicBufferUtil"
#define ATRACE_TAG ATRACE_TAG_GRAPHICS

#define MTK_LOG_ENABLE 1
#include <utils/String8.h>

#include <cutils/log.h>

#include <ui/gralloc_extra.h>
#include <ui/GraphicBuffer.h>
#include <ui/GraphicBufferMapper.h>

#include <png.h>

#include <ui_ext/GraphicBufferUtil.h>
#include <graphics_mtk_defs.h>

#include <sys/stat.h>

#include <ui_ext/SkColorPriv.h>
#include <math.h>

#define ALIGN_CEIL(x,a) (((x) + (a) - 1L) & ~((a) - 1L))
#define LOCK_FOR_SW (GRALLOC_USAGE_SW_READ_RARELY | GRALLOC_USAGE_SW_WRITE_RARELY | GRALLOC_USAGE_HW_TEXTURE)
#define SYMBOL2ALIGN(x) ((x) == 0) ? 1 : ((x) << 1)

#define DUMP_FILE_PATH      "/data/SF_dump/"

namespace android {
// ---------------------------------------------------------------------------

ANDROID_SINGLETON_STATIC_INSTANCE( GraphicBufferUtil )

status_t BufferInfo::getInfo(const buffer_handle_t& handle)
{
    mHandle = handle;
#ifndef MTK_DO_NOT_USE_GPU_EXT
    int format = PIXEL_FORMAT_UNKNOWN;
    int err = NO_ERROR;
    mErr = gralloc_extra_query(handle, GRALLOC_EXTRA_GET_WIDTH, &mWidth);
    mErr |= gralloc_extra_query(handle, GRALLOC_EXTRA_GET_HEIGHT, &mHeight);
    mErr |= gralloc_extra_query(handle, GRALLOC_EXTRA_GET_STRIDE, &mStride);
    mErr |= gralloc_extra_query(handle, GRALLOC_EXTRA_GET_VERTICAL_STRIDE, &mVStride);
    mErr |= gralloc_extra_query(handle, GRALLOC_EXTRA_GET_FORMAT, &mFormat);
    mErr |= gralloc_extra_query(handle, GRALLOC_EXTRA_GET_ALLOC_SIZE, &mSize);
    mErr |= gralloc_extra_query(handle, GRALLOC_EXTRA_GET_IOCTL_ION_SF_INFO, &mExtInfo);

    if ((err = getGraphicBufferUtil().getRealFormat(handle, &format)) == GRALLOC_EXTRA_OK)
    {
        mFormat = format;
    }
    mErr |= err;
    mErr |= updateVideoInfo(handle);

    return mErr;
#else
    return NO_INIT;
#endif
}

status_t BufferInfo::getInfo(const sp<GraphicBuffer>& gb)
{
    if (NO_ERROR != getInfo(gb->handle))
    {
        ALOGD("Can't gralloc_extra handle, fallback to get info from gb!");
        mHandle = gb->handle;
        mWidth = gb->width;
        mHeight = gb->height;
        mStride = gb->stride;
        mVStride = gb->height;
        mFormat = gb->format;
        mSize = mStride * mVStride;
        mErr = NO_ERROR;
    }
    return mErr;
}

status_t BufferInfo::updateVideoInfo(const buffer_handle_t& handle)
{
#ifndef MTK_DO_NOT_USE_GPU_EXT
    int err = NO_ERROR;
    gralloc_extra_ion_sf_info_t info;

    err |= gralloc_extra_query(handle, GRALLOC_EXTRA_GET_IOCTL_ION_SF_INFO, &info);
    if (!err && info.videobuffer_status & 0x80000000)
    {
        unsigned int align = (info.videobuffer_status & 0x7FFFFFFF) >> 25;
        align = SYMBOL2ALIGN(align);
        mStride = ALIGN_CEIL(mWidth, align);

        align = (info.videobuffer_status & 0x0007FFFF) >> 13;
        align = SYMBOL2ALIGN(align);
        mVStride = ALIGN_CEIL(mHeight, align);
    }

    return err;
#else
    (void)handle;
    return NO_INIT;
#endif
}

GraphicBufferUtil::GraphicBufferUtil()
{
}

GraphicBufferUtil::~GraphicBufferUtil()
{
}

static void write565Data(uint32_t width,
                         uint32_t height,
                         uint8_t* in,
                         png_structp out)
{
    // input is RGB565 for 2 bytes, and output is RGBA8888 for 4 bytes
    const uint32_t inBPP = 2;
    const uint32_t outBPP = 4;

    png_bytep tmp = new png_byte[width * outBPP];
    if (NULL == tmp)
    {
        ALOGE("cannot create temp row buffer for RGB565 conversion");
        return;
    }

    uint8_t* wptr;
    for (uint32_t i = 0; i < height; i++)
    {
        wptr = reinterpret_cast<uint8_t*>(tmp);
        for (uint32_t j = 0; j < width; ++j)
        {
            // get 16bits pixel value and split it into 32bits storage
            const uint16_t c = *(reinterpret_cast<uint16_t*>(in));
            wptr[0] = SkPacked16ToR32(c);
            wptr[1] = SkPacked16ToG32(c);
            wptr[2] = SkPacked16ToB32(c);
            wptr[3] = 255;

            wptr += outBPP;
            in += inBPP;
        }
        png_write_rows(out, &tmp, 1);
    }

    delete [] tmp;
}

static void writeData(uint32_t width,
                      uint32_t height,
                      uint8_t* in,
                      png_structp out)
{
    // should be RGBA for 4 bytes
    const uint32_t widthBytes = width * 4;

    for (uint32_t y = 0; y < height; y++)
    {
        png_write_rows(out, (png_bytepp)&in, 1);
        in = (uint8_t *)in + widthBytes;
    }
}

static bool isSupportedFormat(const int& format)
{
    switch (format)
    {
        case HAL_PIXEL_FORMAT_RGBA_8888:
        case HAL_PIXEL_FORMAT_RGBX_8888:
        case HAL_PIXEL_FORMAT_BGRA_8888:
        case 0x1ff:                     // tricky format for SGX_COLOR_FORMAT_BGRX_8888 in fact
        case HAL_PIXEL_FORMAT_RGB_888:
        case HAL_PIXEL_FORMAT_RGB_565:
        case HAL_PIXEL_FORMAT_YUYV:
        case HAL_PIXEL_FORMAT_YV12:
        case HAL_PIXEL_FORMAT_YCbCr_420_888:
        case HAL_PIXEL_FORMAT_I420:
            return true;
        default:
            ALOGE("    unknown format: 0x%x, use default as 0.0", format);
            return false;
    }
}

void GraphicBufferUtil::dump(const sp<GraphicBuffer>& gb,
                              const char* prefix,
                              const char* dir)
{
    if (CC_UNLIKELY(gb == NULL))
    {
        ALOGE("[%s] gb is NULL", __func__);
        return;
    }

    BufferInfo i;
    i.getInfo(gb);

    dump(i, prefix, dir);
}

void GraphicBufferUtil::dump(const buffer_handle_t &handle,
                              const char* prefix,
                              const char* dir)
{
    if (CC_UNLIKELY(handle == NULL))
    {
        ALOGE("[%s] handle is NULL", __func__);
        return;
    }

    BufferInfo i;
    i.getInfo(handle);

    dump(i, prefix, dir);
}

void GraphicBufferUtil::dump( const BufferInfo &info,
                              const char* prefix,
                              const char* dir)
{
    const uint32_t& width = info.mWidth;
    const uint32_t& height = info.mHeight;
    const uint32_t& stride = info.mStride;
    const uint32_t& vstride = info.mVStride;
    const buffer_handle_t& handle = info.mHandle;
    const PixelFormat& format = info.mFormat;

    // For aee manual dump, we must create a directory to save files.
    // The step should not be completed by a script.
    struct stat sb;
    if (stat(DUMP_FILE_PATH, &sb) != 0) {
        // ths permission of /data/SF_dump must be 777,
        // or some processes cannot save files to /data/SF_dump
        mode_t mode = umask(0);
        if (mkdir(DUMP_FILE_PATH, 0777) != 0) {
            ALOGE("mkdir(%s) failed", DUMP_FILE_PATH);
        }
        umask(mode);
    }
    if (access(DUMP_FILE_PATH, R_OK | W_OK | X_OK) != 0) {
        ALOGE("The permission of %s cannot be access by this process", DUMP_FILE_PATH);
    }

    ALOGD("[%s] handle:%p +", __func__, handle);

    bool isRaw = false;
    bool isCompressed = (info.mExtInfo.status2 & GRALLOC_EXTRA_MASK2_BUF_COMPRESSION_STATUS);
    isRaw |= isCompressed;

    String8 cType;
    bool isG2G = (info.mExtInfo.status2 & GRALLOC_EXTRA_MASK2_GPU_TO_GPU);
    cType = isG2G ? ".c1" : ".c";

    ALOGD("[%s] handle:%p isRaw:%d status2:%x +", __func__, handle, isRaw,
        info.mExtInfo.status2);
    ALOGD("    prefix: %s dir:%s", prefix, dir);

    // make file name, default path to /data/[handle]_[width]_[height]_[stride]
    String8 path;
    if ((NULL == dir) || (0 == strlen(dir)))
    {
        path.setPathName("/data/");
    }
    else
    {
        path.setPathName(dir);
    }

    if ((NULL == prefix) || (0 == strlen(prefix)))
    {
        path.append(String8::format("/H%p_w%d_h%d_s%d_vs%d",
            handle, width, height, stride, vstride));
    }
    else
    {
        path.append(String8::format("/%s_H%p_w%d_h%d_s%d_vs%d",
            prefix, handle, width, height, stride, vstride));
    }
    ALOGD("      name: %s", path.string());

    bool is565 = false;
    bool stripAlpha = false;
    path += isCompressed ? cType : "";

    // switch case different pixel format process
    // only RGB? series will be saved into image file
    // others in RAW data only
    switch (format)
    {
        case HAL_PIXEL_FORMAT_RGBA_8888:
            path += isRaw ? "rgba8888": ".png";
            break;
        case HAL_PIXEL_FORMAT_RGBX_8888:
            path += isRaw ? "rgbx8888": ".png";
            stripAlpha = true;
            break;
        case HAL_PIXEL_FORMAT_BGRA_8888:
            path += isRaw ? "bgra8888": "(RBswapped).png";
            break;
        case 0x1ff:                     // tricky format for SGX_COLOR_FORMAT_BGRX_8888 in fact
            path += "(RBswapped).png";
            stripAlpha = true;
            break;
        case HAL_PIXEL_FORMAT_RGB_565:
            is565 = true;
            stripAlpha = true;
            path += isRaw ? "rgb565": ".png";
            break;
        case HAL_PIXEL_FORMAT_I420:
            isRaw = true;
            path += ".i420";
            break;
        case HAL_PIXEL_FORMAT_NV12_BLK:
            isRaw = true;
            path += ".nv12_blk";
            break;
        case HAL_PIXEL_FORMAT_NV12_BLK_FCM:
            isRaw = true;
            path += ".nv12_blk_fcm";
            break;
        case HAL_PIXEL_FORMAT_YCrCb_420_SP:
            isRaw = true;
            path += ".nv21";
            break;
        case HAL_PIXEL_FORMAT_YV12:
            isRaw = true;
            path += ".yv12";
            break;
        case HAL_PIXEL_FORMAT_YUYV:
            isRaw = true;
            path += ".yuyv";
            break;
        case HAL_PIXEL_FORMAT_RGB_888:
            isRaw = true;
            path += ".rgb888";
            break;
        case HAL_PIXEL_FORMAT_UFO:
            isRaw = true;
            path += ".ufo";
            break;
        case HAL_PIXEL_FORMAT_NV12_BLK_10BIT_H:
            isRaw = true;
            path += ".nv12_blk_10bit_h";
            break;
        case HAL_PIXEL_FORMAT_NV12_BLK_10BIT_V:
            isRaw = true;
            path += ".nv12_blk_10bit_v";
            break;
        case HAL_PIXEL_FORMAT_UFO_10BIT_H:
            isRaw = true;
            path += ".ufo_10bit_h";
            break;
        case HAL_PIXEL_FORMAT_UFO_10BIT_V:
            isRaw = true;
            path += ".ufo_10bit_v";
            break;
        default:
            isRaw = true;
            path.append(String8::format(".unknown_0x%x", format));
            ALOGE("    Try to dump unknown format(0x%x)", format);
            break;
    }
    ALOGD("      path: %s", path.string());

    void        *ptr     = NULL;
    FILE        *f       = NULL;
    png_structp png_ptr  = NULL;
    png_infop   info_ptr = NULL;

    // dump to file with final path and file type

    int err = GraphicBufferMapper::getInstance().lock(handle,
                                                      GraphicBuffer::USAGE_SW_READ_OFTEN,
                                                      Rect(width, height),
                                                      &ptr);
    if (err == NO_ERROR)
    {
        if (NULL == ptr)
        {
            ALOGE("    lock() FAILED");
            goto finalize;
        }

        f = fopen(path.string(), "wb");
        if (NULL == f)
        {
            ALOGE("    fopen() FAILED");
            goto finalize;
        }

        if (isRaw)
        {
            fwrite(ptr, info.mSize, 1, f);
        }
        else
        {
            // init write struct
            png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
            if (NULL == png_ptr)
            {
                ALOGE("    init png FAILED (1)");
                goto finalize;
            }

            // init info struct
            info_ptr = png_create_info_struct(png_ptr);
            if (NULL == info_ptr)
            {
                ALOGE("    init png FAILED (2)");
                goto finalize;
            }

            // set file to write
            png_init_io(png_ptr, f);

            // setup color format info
            if (true == stripAlpha)
            {
                png_set_IHDR(png_ptr, info_ptr,
                    stride, height, 8, PNG_COLOR_TYPE_RGB,
                    PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
            }
            else
            {
                png_set_IHDR(png_ptr, info_ptr,
                    stride, height, 8, PNG_COLOR_TYPE_RGB_ALPHA,
                    PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
            }

            // set bits layout
            png_color_8 sigBit;
            if (is565)
            {
                sigBit.red   = 5;
                sigBit.green = 6;
                sigBit.blue  = 5;
                sigBit.gray  = 0;
                sigBit.alpha = 0;
            }
            else
            {
                sigBit.red   = 8;
                sigBit.green = 8;
                sigBit.blue  = 8;
                sigBit.gray  = 0;
                sigBit.alpha = 8;
            }
            png_set_sBIT(png_ptr, info_ptr, &sigBit);

            // start
            png_write_info(png_ptr, info_ptr);

            // set filler to get rid of alpha if strip needed
            if (true == stripAlpha)
            {
                png_set_filler(png_ptr, 0x00, PNG_FILLER_AFTER);
            }

            // write data into png
            if (is565)
            {
                write565Data(stride, height, reinterpret_cast<uint8_t*>(ptr), png_ptr);
            }
            else
            {
                writeData(stride, height, reinterpret_cast<uint8_t*>(ptr), png_ptr);
            }

            // end
            png_write_end(png_ptr, info_ptr);
        }

finalize:
        // clean up
        if (NULL != f)        fclose(f);
        if (NULL != info_ptr) png_free_data(png_ptr, info_ptr, PNG_FREE_ALL, -1);
        if (NULL != png_ptr)  png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
    }
    GraphicBufferMapper::getInstance().unlock(handle);

    ALOGD("[%s] -", __func__);
}

uint32_t GraphicBufferUtil::getBitsPerPixel(int format)
{
    uint32_t bits = 0;
    switch (format)
    {
        case HAL_PIXEL_FORMAT_RGBA_8888:
        case HAL_PIXEL_FORMAT_RGBX_8888:
        case HAL_PIXEL_FORMAT_BGRA_8888:
        case 0x1ff:                     // tricky format for SGX_COLOR_FORMAT_BGRX_8888 in fact
            bits = 32;
            break;
        case HAL_PIXEL_FORMAT_RGB_888:
            bits = 24;
            break;
        case HAL_PIXEL_FORMAT_RGB_565:
        case HAL_PIXEL_FORMAT_YUYV:
            bits = 16;
            break;
        case HAL_PIXEL_FORMAT_NV12_BLK_10BIT_H:
        case HAL_PIXEL_FORMAT_NV12_BLK_10BIT_V:
        case HAL_PIXEL_FORMAT_UFO_10BIT_H:
        case HAL_PIXEL_FORMAT_UFO_10BIT_V:
        case HAL_PIXEL_FORMAT_YUV_PRIVATE_10BIT:
            bits = 15;
            break;
        case HAL_PIXEL_FORMAT_I420:
        case HAL_PIXEL_FORMAT_NV12_BLK:
        case HAL_PIXEL_FORMAT_NV12_BLK_FCM:
        case HAL_PIXEL_FORMAT_YV12:
        case HAL_PIXEL_FORMAT_YUV_PRIVATE:
        case HAL_PIXEL_FORMAT_YCbCr_420_888:
            bits = 12;
            break;
        default:
            ALOGE("    unknown format: 0x%x, use default as 0.0", format);
    }
    return bits;
}

uint32_t GraphicBufferUtil::getBitsPerPixel(buffer_handle_t handle)
{
    int32_t format = 0;
#ifndef MTK_DO_NOT_USE_GPU_EXT
    if (gralloc_extra_query(
                handle,
                GRALLOC_EXTRA_GET_FORMAT,
                static_cast<int*>(&format)) != GRALLOC_EXTRA_OK)
    {
        ALOGE("Getting format of handle failed");
        return 0;
    }
#else
    (void)handle;
#endif
    return getBitsPerPixel(format);
}

status_t GraphicBufferUtil::drawLine(const sp<GraphicBuffer>& gb, uint8_t val, int ptn_w, int ptn_h, int pos)
{
    if (CC_UNLIKELY(gb == NULL))
    {
        ALOGE("[%s] gb is NULL", __func__);
        return INVALID_OPERATION;
    }

    BufferInfo i;
    i.getInfo(gb);

    return drawLine(i, val, ptn_w, ptn_h, pos);
}

status_t GraphicBufferUtil::drawLine(const buffer_handle_t& handle, uint8_t val, int ptn_w, int ptn_h, int pos)
{
    if (CC_UNLIKELY(handle == NULL))
    {
        ALOGE("[%s] handle is NULL", __func__);
        return INVALID_OPERATION;
    }

    BufferInfo i;
    i.getInfo(handle);

    return drawLine(i, val, ptn_w, ptn_h, pos);
}

status_t GraphicBufferUtil::drawLine(const BufferInfo &info, uint8_t val, int ptn_w, int ptn_h, int pos)
{
    const uint32_t& width = info.mWidth;
    const uint32_t& height = info.mHeight;
    const uint32_t& stride = info.mStride;
    const buffer_handle_t& handle = info.mHandle;
    const PixelFormat& format = info.mFormat;

    if (ptn_w == 0)
        ptn_w = 1;
    if (ptn_h == 0)
        ptn_h = 1;

    uint32_t log_w = log2(ptn_w);
    if (log_w > 5)
        log_w = 5;
    uint32_t line_w = pow(2, log_w);

    uint32_t log_h = log2(ptn_h);
    if (log_h > 5)
        log_h = 5;
    uint32_t line_h = pow(2, log_h);

    uintptr_t ptr;
    int err = GraphicBufferMapper::getInstance().lock(handle,
                                                      LOCK_FOR_SW,
                                                      Rect(width, height),
                                                      (void**)&ptr);
    if (NO_ERROR != err)
    {
        ALOGE("[%s] buffer lock fail: %s (handle:%p)",
            __func__, strerror(err), handle);
    }
    else if (5670766 == pos)
    {
        ALOGD("[debug] drawLine, just force sync");
    }
    else
    {
        // if custom format, just regard as one-byte-plane size
        // otherwise use accurate size aggressively
        uint32_t bits = 8;
        if (format < 0x100)
        {
            bits = getBitsPerPixel(format);
        }

        if (ptn_w == 1)
        {
            uint32_t bsize; // block size, will split intrested plane to 32 parts

            pos &= (line_h - 1);    // mod count by 32

            ALOGV("[debug] drawLine, pos=%d, log_h=%d, line_h=%d", pos, log_h, line_h);

            bsize = ((stride * height * bits) >> 3) >> log_h;
            memset((void*)(ptr + (bsize * pos)), val, bsize);
        }
        else
        {
            uint32_t block_number = line_w * line_h;
            uint32_t block_w = width >> log_w;
            uint32_t block_h = height >> log_h;
            uint32_t bsize = block_w * block_h;
            uint32_t pos_x = pos & (line_w - 1);
            uint32_t pos_y = (pos / line_w) & (line_h - 1);
            uint32_t j;

            ALOGV("[debug] drawLine pos=%d, log_w=%d, line_w=%d, log_h=%d, line_h=%d, "
                  "block_number=%d, block_w=%d, block_h=%d, bsize=%d, pos_x=%d, pos_y=%d",
                pos, log_w, line_w, log_h, line_h, block_number, block_w, block_h, bsize, pos_x, pos_y);

            block_w = (block_w * bits) >> 3;
            ptr += ((stride * pos_y * block_h * bits) >> 3) + (block_w * pos_x);
            for (j = 0; j < block_h; j++)
            {
                memset((void*)ptr, val, block_w);
                ptr += (stride * bits) >> 3;
            }
        }
    }
    GraphicBufferMapper::getInstance().unlock(handle);

    return NO_ERROR;
}

int GraphicBufferUtil::getRealFormat(buffer_handle_t handle, PixelFormat* format) {
#ifndef MTK_DO_NOT_USE_GPU_EXT
    if (NULL == handle)
    {
        ALOGE("[%s] invalid argument", __func__);
        return GRALLOC_EXTRA_ERROR;
    }
    if (NULL == format)
    {
        ALOGE("[%s] invalid argument", __func__);
        return GRALLOC_EXTRA_ERROR;
    }
    int err = GRALLOC_EXTRA_OK;

    // for HAL_PIXEL_FORMAT_IMPLEMENTATION_DEFINED
    if ((err = gralloc_extra_query(
                    handle,
                    GRALLOC_EXTRA_GET_FORMAT,
                    format)) != GRALLOC_EXTRA_OK)
    {
        ALOGE("[%s] to get format failed", __func__);
        return err;
    }

    if (HAL_PIXEL_FORMAT_YUV_PRIVATE == *format ||
            HAL_PIXEL_FORMAT_YCbCr_420_888 == *format ||
            HAL_PIXEL_FORMAT_YUV_PRIVATE_10BIT == *format)
    {
        gralloc_extra_ion_sf_info_t sf_info;
        if ((err = gralloc_extra_query(handle, GRALLOC_EXTRA_GET_IOCTL_ION_SF_INFO, &sf_info))
                != GRALLOC_EXTRA_OK) {
            ALOGE("[%s] to query real format failed", __func__);
            return err;
        }
        // check real format within private format
        switch (sf_info.status & GRALLOC_EXTRA_MASK_CM)
        {
            case GRALLOC_EXTRA_BIT_CM_YV12:
                *format = HAL_PIXEL_FORMAT_YV12;
                break;
            case GRALLOC_EXTRA_BIT_CM_NV12_BLK:
                *format = HAL_PIXEL_FORMAT_NV12_BLK;
                break;
            case GRALLOC_EXTRA_BIT_CM_NV12_BLK_FCM:
                *format = HAL_PIXEL_FORMAT_NV12_BLK_FCM;
                break;
            case GRALLOC_EXTRA_BIT_CM_YUYV:
                *format = HAL_PIXEL_FORMAT_YCbCr_422_I;
                break;
            case GRALLOC_EXTRA_BIT_CM_I420:
                *format = HAL_PIXEL_FORMAT_I420;
                break;
            case GRALLOC_EXTRA_BIT_CM_UFO:
                *format = HAL_PIXEL_FORMAT_UFO;
                break;
            case GRALLOC_EXTRA_BIT_CM_NV12_BLK_10BIT_H:
                *format = HAL_PIXEL_FORMAT_NV12_BLK_10BIT_H;
                break;
            case GRALLOC_EXTRA_BIT_CM_NV12_BLK_10BIT_V:
                *format = HAL_PIXEL_FORMAT_NV12_BLK_10BIT_V;
                break;
            case GRALLOC_EXTRA_BIT_CM_UFO_10BIT_H:
                *format = HAL_PIXEL_FORMAT_UFO_10BIT_H;
                break;
            case GRALLOC_EXTRA_BIT_CM_UFO_10BIT_V:
                *format = HAL_PIXEL_FORMAT_UFO_10BIT_V;
                break;
            default:
                ALOGE("    CANNOT get real format: (format=0x%x, fillFormat=0x%x)",
                      *format,
                      static_cast<unsigned int>(sf_info.status & GRALLOC_EXTRA_MASK_CM));
                return GRALLOC_EXTRA_ERROR;
        }
    }
    return err;
#else
    (void)handle;
    (void)format;
    return NO_INIT;
#endif
}

DownSampleConfig GraphicBufferUtil::downSampleCopy(const DownSampleConfig& config,
                                                   const sp<GraphicBuffer>& srcBuf,
                                                   sp<GraphicBuffer>& dstBuf)
{
    BufferInfo srcInfo;
    srcInfo.getInfo(srcBuf);

    const int& format = srcInfo.mFormat;
    const int& stride = srcInfo.mStride;
    const int& vstride = srcInfo.mVStride;
    const uint32_t bits = getGraphicBufferUtil().getBitsPerPixel(format);
    Rect newCrop;

    // down sample config valid check
    DownSampleConfig newConfig;

    // adjust valid DownSample config
    // only non-block color format support downsample and crop copy
    newConfig.mDownSampleX = (config.mDownSampleX <= 0 || !isSupportedFormat(format)) ? 1 : config.mDownSampleX;
    newConfig.mDownSampleY = (config.mDownSampleY <= 0 || !isSupportedFormat(format)) ? 1 : config.mDownSampleY;
    if (config.mCrop.isEmpty() || !isSupportedFormat(format))
    {
        newCrop = Rect(stride, vstride);
    }
    else
    {
        newCrop.left = (config.mCrop.left >= 0 && config.mCrop.left < stride)? config.mCrop.left : 0;
        newCrop.top = (config.mCrop.top >= 0 && config.mCrop.top < vstride)? config.mCrop.top : 0;
        newCrop.right = (config.mCrop.right < stride && config.mCrop.right > newCrop.left) ? config.mCrop.right : stride;
        newCrop.bottom = (config.mCrop.bottom < vstride && config.mCrop.bottom > newCrop.top) ? config.mCrop.bottom : vstride;
    }
    newConfig.mCrop = newCrop;

    void *src = NULL;
    void *dst = NULL;
    status_t err = srcBuf->lock(GraphicBuffer::USAGE_SW_READ_OFTEN, &src);
    if (err != NO_ERROR)
    {
        ALOGE("[%s] lock GraphicBuffer failed", __func__);
        return newConfig;
    }

    // do memcpy with downSampleConfig
    if ((newConfig.mDownSampleX == 1) && (newConfig.mDownSampleY == 1) &&
        (stride == newCrop.getWidth()) && (vstride == newCrop.getHeight()))
    {
        if ((newCrop.getWidth() != dstBuf->width) || (newCrop.getHeight() != dstBuf->height)
            ||(srcBuf->format != dstBuf->format))
        {
            dstBuf = new GraphicBuffer(srcBuf->width, srcBuf->height, srcBuf->format, srcBuf->usage);
            ALOGD("[%s] setting changed, backup=(%d, %d, %d) => active=(%d, %d, %d) size = %d",
                __func__, srcBuf->width, srcBuf->height, srcBuf->format,
                dstBuf->width, dstBuf->height, dstBuf->format, (stride * vstride * bits) >> 3);
        }

        err = dstBuf->lock(GraphicBuffer::USAGE_SW_READ_OFTEN | GraphicBuffer::USAGE_SW_WRITE_OFTEN, &dst);
        if (err != NO_ERROR)
        {
            srcBuf->unlock();
            ALOGE("[%s] lock backup buffer failed", __func__);
            return newConfig;
        }

        BufferInfo dstInfo;
        dstInfo.getInfo(dstBuf);
        memcpy(dst, src, dstInfo.mSize);
    }
    else
    {
        int newWidth = (newCrop.getWidth() - 1) / newConfig.mDownSampleX + 1;
        int newHeight = (newCrop.getHeight() - 1) / newConfig.mDownSampleY + 1;
        if ((newWidth != dstBuf->width) || (newHeight != dstBuf->height)
            ||(srcBuf->format != dstBuf->format))
        {
            dstBuf = new GraphicBuffer(newWidth, newHeight, srcBuf->format, srcBuf->usage);
            ALOGD("[%s] setting changed, backup=(%d, %d, %d) => active=(%d, %d, %d) size=%d",
                __func__, srcBuf->width, srcBuf->height, srcBuf->format,
                dstBuf->width, dstBuf->height, dstBuf->format, (newWidth * newHeight * bits) >> 3);
        }

        err = dstBuf->lock(GraphicBuffer::USAGE_SW_READ_OFTEN | GraphicBuffer::USAGE_SW_WRITE_OFTEN, &dst);
        if (err != NO_ERROR)
        {
            srcBuf->unlock();
            ALOGE("[%s] lock backup buffer failed", __func__);
            return newConfig;
        }

        BufferInfo dstInfo;
        dstInfo.getInfo(dstBuf);
        const int& newStride = dstInfo.mStride;

        // use bytesPerPixel to filter uv part
        // for 12 bit(yuv format), we only handle y plane copy
        const int&& bytesPerPixel = bits >> 3;
        for (int y = newCrop.top, y0 = 0; y < newCrop.bottom; y0++, y += newConfig.mDownSampleY)
        {
            if (newConfig.mDownSampleX == 1)
            {
                memcpy((char*)dst + ((newStride * bytesPerPixel * y0)),
                       (char*)src + (((stride * y + newCrop.left) * bytesPerPixel)),
                        (newCrop.getWidth()* bytesPerPixel));
            }
            else
            {
                for (int x = newCrop.left, x0 = 0; x < newCrop.right; x0++, x += newConfig.mDownSampleX)
                {
                    memcpy((char*)dst + (((newStride * y0 + x0) * bytesPerPixel)),
                           (char*)src + (((stride * y + newCrop.left + x) * bytesPerPixel)),
                            bits >> 3);
                }
            }
        }
    }
    srcBuf->unlock();
    dstBuf->unlock();

    return newConfig;
}


// ---------------------------------------------------------------------------
}; // namespace android
