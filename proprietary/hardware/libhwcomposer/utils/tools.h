#ifndef UTILS_TOOLS_H_
#define UTILS_TOOLS_H_

#ifndef DEBUG_LOG_TAG
#define DEBUG_LOG_TAG "TOOL"
#endif // DEBUG_LOG_TAG

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>

#include <utils/Errors.h>
//#include <utils/CallStack.h>

#include "hwc_priv.h"
#include "ui/Region.h"
#include "ui/gralloc_extra.h"
#include "graphics_mtk_defs.h"
#include "gralloc_mtk_defs.h"
#include "utils/debug.h"
#include "hwc2_defs.h"
#include "dev_interface.h"
#include "DpDataType.h"

#define LOCK_FOR_HW (GRALLOC_USAGE_SW_READ_RARELY | \
                     GRALLOC_USAGE_SW_WRITE_NEVER | \
                     GRALLOC_USAGE_HW_TEXTURE)

#define SWAP(a, b) do { typeof(a) __a = (a); (a) = (b); (b) = __a; } while (0)

#define WIDTH(rect) ((rect).right - (rect).left)
#define HEIGHT(rect) ((rect).bottom - (rect).top)
#define SIZE(rect) (WIDTH(rect) * HEIGHT(rect))

#define MULPASSLOGV(label, x, ...) HWC_LOGV("    ![ %s ]   " x, label, ##__VA_ARGS__)
#define MULPASSLOGD(label, x, ...) HWC_LOGD("    ![ %s ]   " x, label, ##__VA_ARGS__)
#define MULPASSLOGE(label, x, ...) HWC_LOGE("    ![ %s ]   " x, label, ##__VA_ARGS__)

#define SVPLOGV(label, x, ...) HWC_LOGV(" #SVP [ %s ] " x, label, ##__VA_ARGS__)
#define SVPLOGD(label, x, ...) HWC_LOGD(" #SVP [ %s ] " x, label, ##__VA_ARGS__)
#define SVPLOGE(label, x, ...) HWC_LOGE(" #SVP [ %s ] " x, label, ##__VA_ARGS__)

#define RECTLOG(rect, x, ...) HWC_LOGD("RECT (%4d,%4d,%4d,%4d) - " x, rect.left, rect.top, rect.right, rect.bottom, ##__VA_ARGS__)

#define ALIGN_FLOOR(x,a)    ((x) & ~((a) - 1L))
#define ALIGN_CEIL(x,a)     (((x) + (a) - 1L) & ~((a) - 1L))

#define SYMBOL2ALIGN(x) ((x) == 0) ? 1 : ((x) << 1)

#define NOT_PRIVATE_FORMAT -1

#define HAL_PIXEL_FORMAT_DIM 0x204D4944

#define MAX_ABORT_MSG 100

#define SIGNAL_TIME_PENDING (std::numeric_limits<nsecs_t>::max())
#define SIGNAL_TIME_INVALID -1

typedef uint32_t SECHAND;

template<class T>
class ObjectWithChangedState
{
private:
    T m_obj;
    bool m_changed;

public:
    ObjectWithChangedState(const T& obj) : m_obj(obj), m_changed(false) {}

    inline void set(const T& obj)
    {
        m_changed = (obj != m_obj);
        m_obj = obj;
    }

    inline T const& get() const { return m_obj; }

    inline bool isChanged() const { return m_changed; };
};

inline void passFenceFd(int* dst_fd, int* src_fd)
{
    if (*dst_fd >= 0 && *dst_fd < 3)
    {
        ALOGE("abort! pass fence dst fd %d", *dst_fd);
        abort();
    }

    if (*src_fd >= 0 && *src_fd < 3)
    {
        ALOGE("abort! pass fence src fd %d", *src_fd);
        abort();
    }

    *dst_fd = *src_fd;
    *src_fd = -1;
}

inline int dupFenceFd(int* fd)
{
    const int32_t dup_fd = (*fd >= 0) ? dup(*fd) : -1;

    if ((3 > dup_fd && -1 < dup_fd) || (3 > *fd && -1 < *fd))
    {
        //TODO: add new way to dump CallStack
        //android::CallStack stack;
        //stack.update();
        //HWC_LOGW("dup Fence fd is zero call stack is (ori:%d dup:%d) %s",*fd ,dup_fd ,stack.toString().string());
        HWC_LOGW("dup Fence fd is zero call stack is (ori:%d dup:%d)",*fd ,dup_fd);
        abort();
    }

    close(*fd);
    *fd = -1;
    return dup_fd;
}

inline void closeFenceFd(int* fd)
{
    if (3 > *fd && -1 < *fd)
    {
        //TODO: add new way to dump CallStack
        //android::CallStack stack;
        //stack.update();
        //HWC_LOGW("Fence fd is zero call stack is %s", stack.toString().string());
        HWC_LOGW("Fence fd is zero");
        abort();
    }
    if (-1 == *fd)
        return;

    close(*fd);
    *fd = -1;
}

inline Rect getFixedRect(hwc_frect_t& src_cropf)
{
    int l = (int)(ceilf(src_cropf.left));
    int t = (int)(ceilf(src_cropf.top));
    int r = (int)(floorf(src_cropf.right));
    int b = (int)(floorf(src_cropf.bottom));
    return Rect(l, t, r, b);
}

inline int mapGrallocFormat(const uint32_t& format)
{
    switch (format)
    {
        case HAL_PIXEL_FORMAT_RGBA_8888:
        case HAL_PIXEL_FORMAT_BGRA_8888:
            return HAL_PIXEL_FORMAT_RGBA_8888;
        case HAL_PIXEL_FORMAT_RGBX_8888:
        case HAL_PIXEL_FORMAT_BGRX_8888:
        case HAL_PIXEL_FORMAT_IMG1_BGRX_8888:
            return HAL_PIXEL_FORMAT_RGBX_8888;
        case HAL_PIXEL_FORMAT_RGB_888:
            return HAL_PIXEL_FORMAT_RGB_888;
        case HAL_PIXEL_FORMAT_RGB_565:
            return HAL_PIXEL_FORMAT_RGB_565;
        case HAL_PIXEL_FORMAT_RGBA_1010102:
            return HAL_PIXEL_FORMAT_RGBA_8888;
        case HAL_PIXEL_FORMAT_I420:
        case HAL_PIXEL_FORMAT_YV12:
        case HAL_PIXEL_FORMAT_NV12_BLK:
        case HAL_PIXEL_FORMAT_NV12_BLK_FCM:
        case HAL_PIXEL_FORMAT_YUV_PRIVATE:
        case HAL_PIXEL_FORMAT_UFO:
        case HAL_PIXEL_FORMAT_UFO_AUO:
        case HAL_PIXEL_FORMAT_YCbCr_420_888:
        case HAL_PIXEL_FORMAT_YUV_PRIVATE_10BIT:
        case HAL_PIXEL_FORMAT_NV12_BLK_10BIT_H:
        case HAL_PIXEL_FORMAT_NV12_BLK_10BIT_H_JUMP:
        case HAL_PIXEL_FORMAT_NV12_BLK_10BIT_V:
        case HAL_PIXEL_FORMAT_NV12_BLK_10BIT_V_JUMP:
        case HAL_PIXEL_FORMAT_UFO_10BIT_H:
        case HAL_PIXEL_FORMAT_UFO_10BIT_H_JUMP:
        case HAL_PIXEL_FORMAT_UFO_10BIT_V:
        case HAL_PIXEL_FORMAT_UFO_10BIT_V_JUMP:
        case HAL_PIXEL_FORMAT_YUYV:
        case HAL_PIXEL_FORMAT_YCRCB_420_SP:
        case HAL_PIXEL_FORMAT_YCBCR_422_SP:
        case HAL_PIXEL_FORMAT_YCBCR_422_I:
        case HAL_PIXEL_FORMAT_I420_DI:
        case HAL_PIXEL_FORMAT_YV12_DI:
        case HAL_PIXEL_FORMAT_NV12:
            return HAL_PIXEL_FORMAT_YUYV;
    }
    HWC_LOGW("mapGrallocFormat: unexpected format(%#x)", format);
    return format;
}

inline int getBitsPerPixel(const uint32_t format)
{
    switch (format)
    {
        case HAL_PIXEL_FORMAT_RGBA_8888:
        case HAL_PIXEL_FORMAT_RGBX_8888:
        case HAL_PIXEL_FORMAT_BGRA_8888:
        case HAL_PIXEL_FORMAT_BGRX_8888:
        case HAL_PIXEL_FORMAT_IMG1_BGRX_8888:
        case HAL_PIXEL_FORMAT_RGBA_1010102:
            return 32;

        case HAL_PIXEL_FORMAT_RGB_888:
            return 24;

        case HAL_PIXEL_FORMAT_RGB_565:
            return 16;

        case HAL_PIXEL_FORMAT_NV12_BLK_FCM:
        case HAL_PIXEL_FORMAT_NV12_BLK:
        case HAL_PIXEL_FORMAT_I420:
        case HAL_PIXEL_FORMAT_YV12:
        case HAL_PIXEL_FORMAT_YUV_PRIVATE:
        case HAL_PIXEL_FORMAT_YCbCr_422_I:
        case HAL_PIXEL_FORMAT_YUYV:
        case HAL_PIXEL_FORMAT_UFO:
        case HAL_PIXEL_FORMAT_UFO_AUO:
        case HAL_PIXEL_FORMAT_YCbCr_420_888:
        case HAL_PIXEL_FORMAT_YUV_PRIVATE_10BIT:
        case HAL_PIXEL_FORMAT_NV12_BLK_10BIT_H:
        case HAL_PIXEL_FORMAT_NV12_BLK_10BIT_H_JUMP:
        case HAL_PIXEL_FORMAT_NV12_BLK_10BIT_V:
        case HAL_PIXEL_FORMAT_NV12_BLK_10BIT_V_JUMP:
        case HAL_PIXEL_FORMAT_UFO_10BIT_H:
        case HAL_PIXEL_FORMAT_UFO_10BIT_H_JUMP:
        case HAL_PIXEL_FORMAT_UFO_10BIT_V:
        case HAL_PIXEL_FORMAT_UFO_10BIT_V_JUMP:
        case HAL_PIXEL_FORMAT_YCRCB_420_SP:
        case HAL_PIXEL_FORMAT_YCBCR_422_SP:
        case HAL_PIXEL_FORMAT_I420_DI:
        case HAL_PIXEL_FORMAT_YV12_DI:
        case HAL_PIXEL_FORMAT_NV12:
            return 16;

        default:
            HWC_LOGW("Not support format(%#x) for bpp", format);
            return 0;
    }
}

inline bool isTransparentFormat(const unsigned int format)
{
    switch (format)
    {
        case HAL_PIXEL_FORMAT_RGBA_8888:
        case HAL_PIXEL_FORMAT_BGRA_8888:
        case HAL_PIXEL_FORMAT_RGBA_1010102:
            return true;

        case HAL_PIXEL_FORMAT_RGBX_8888:
        case HAL_PIXEL_FORMAT_RGB_888:
        case HAL_PIXEL_FORMAT_RGB_565:
        case HAL_PIXEL_FORMAT_YV12:
        case HAL_PIXEL_FORMAT_Y8:
        case HAL_PIXEL_FORMAT_Y16:
        case HAL_PIXEL_FORMAT_RAW16:
        case HAL_PIXEL_FORMAT_RAW10:
        case HAL_PIXEL_FORMAT_RAW_OPAQUE:
        case HAL_PIXEL_FORMAT_BLOB:
        case HAL_PIXEL_FORMAT_YCbCr_420_888:
        case HAL_PIXEL_FORMAT_YCbCr_422_SP:
        case HAL_PIXEL_FORMAT_YCrCb_420_SP:
        case HAL_PIXEL_FORMAT_YCbCr_422_I:
        // MTK pixel format
#ifndef MTK_BUILD_IMG_DDK
        case HAL_PIXEL_FORMAT_BGRX_8888:
#endif
        case HAL_PIXEL_FORMAT_I420:
        case HAL_PIXEL_FORMAT_YUV_PRIVATE:
        case HAL_PIXEL_FORMAT_NV12_BLK:
        case HAL_PIXEL_FORMAT_NV12_BLK_FCM:
        case HAL_PIXEL_FORMAT_IMG1_BGRX_8888:
        case HAL_PIXEL_FORMAT_YUYV:
        case HAL_PIXEL_FORMAT_I420_DI:
        case HAL_PIXEL_FORMAT_YV12_DI:
        case HAL_PIXEL_FORMAT_UFO:
        case HAL_PIXEL_FORMAT_UFO_AUO:
        case HAL_PIXEL_FORMAT_YUV_PRIVATE_10BIT:
        case HAL_PIXEL_FORMAT_NV12_BLK_10BIT_H:
        case HAL_PIXEL_FORMAT_NV12_BLK_10BIT_H_JUMP:
        case HAL_PIXEL_FORMAT_NV12_BLK_10BIT_V:
        case HAL_PIXEL_FORMAT_NV12_BLK_10BIT_V_JUMP:
        case HAL_PIXEL_FORMAT_UFO_10BIT_H:
        case HAL_PIXEL_FORMAT_UFO_10BIT_H_JUMP:
        case HAL_PIXEL_FORMAT_UFO_10BIT_V:
        case HAL_PIXEL_FORMAT_UFO_10BIT_V_JUMP:
        case HAL_PIXEL_FORMAT_NV12:
            return false;

        default:
            HWC_LOGW("Not support format(%#x) for transparent", format);
    }
    return false;
}

inline String8 getSessionModeString(HWC_DISP_MODE mode)
{
    // NOTE: these string literals need to match those in linux/disp_session.h
    switch (mode)
    {
        case HWC_DISP_SESSION_DIRECT_LINK_MODE:
            return String8("DL");

        case HWC_DISP_SESSION_DECOUPLE_MODE:
            return String8("DC");

        case HWC_DISP_SESSION_DIRECT_LINK_MIRROR_MODE:
            return String8("DLM");

        case HWC_DISP_SESSION_DECOUPLE_MIRROR_MODE:
            return String8("DCM");

        case HWC_DISP_INVALID_SESSION_MODE:
            return String8("INV");

        default:
            return String8("N/A");
    }
}

inline String8 getFormatString(const uint32_t& format)
{
    switch (format)
    {
        case HAL_PIXEL_FORMAT_RGBA_8888:
            return String8("rgba");

        case HAL_PIXEL_FORMAT_RGBX_8888:
            return String8("rgbx");

        case HAL_PIXEL_FORMAT_BGRA_8888:
            return String8("bgra");

        case HAL_PIXEL_FORMAT_BGRX_8888:
            return String8("bgra");

        case HAL_PIXEL_FORMAT_IMG1_BGRX_8888:
            return String8("img_bgrx");

        case HAL_PIXEL_FORMAT_RGBA_1010102:
            return String8("rgb10");

        case HAL_PIXEL_FORMAT_RGB_888:
            return String8("rgb");

        case HAL_PIXEL_FORMAT_RGB_565:
            return String8("rgb565");

        case HAL_PIXEL_FORMAT_YUYV:
            return String8("yuyv");

        default:
            return String8::format("unknown_%d",format);
    }
}

inline int getSrcLeft(const hwc_layer_1_t* const layer)
{
    return (int)(ceilf(layer->sourceCropf.left));
}

inline int getSrcTop(const hwc_layer_1_t* const layer)
{
    return (int)(ceilf(layer->sourceCropf.top));
}

inline int getSrcWidth(const hwc_layer_1_t* const layer)
{
    const int left = (int)(ceilf(layer->sourceCropf.left));
    const int right = (int)(floorf(layer->sourceCropf.right));
    return (right - left);
}

inline int getSrcHeight(const hwc_layer_1_t* const layer)
{
    int top = (int)(ceilf(layer->sourceCropf.top));
    int bottom = (int)(floorf(layer->sourceCropf.bottom));
    return (bottom - top);
}

inline int getDstTop(const hwc_layer_1_t* const layer)
{
    return (int)(ceilf(layer->displayFrame.top));
}

inline int getDstBottom(const hwc_layer_1_t* const layer)
{
    return (int)(ceilf(layer->displayFrame.bottom));
}

inline int getDstLeft(const hwc_layer_1_t* const layer)
{
    return (int)(ceilf(layer->displayFrame.left));
}

inline int getDstRight(const hwc_layer_1_t* const layer)
{
    return (int)(ceilf(layer->displayFrame.right));
}

inline int getDstWidth(const hwc_layer_1_t* const layer)
{
    return WIDTH(layer->displayFrame);
}

inline int getDstHeight(const hwc_layer_1_t* const layer)
{
    return HEIGHT(layer->displayFrame);
}

inline bool scaled(const hwc_layer_1_t* const layer)
{
    int w = getSrcWidth(layer);
    int h = getSrcHeight(layer);

    if (layer->transform & HWC_TRANSFORM_ROT_90)
        SWAP(w, h);

    return (WIDTH(layer->displayFrame) != w || HEIGHT(layer->displayFrame) != h);
}

DP_COLOR_ENUM mapDpFormat(const uint32_t& fmt);

inline unsigned int grallocColor2HalColor(unsigned int fmt, int info_format)
{
    unsigned int hal_format = 0;
    switch (info_format)
    {
        case NOT_PRIVATE_FORMAT:
            hal_format = fmt;
            break;
        case GRALLOC_EXTRA_BIT_CM_YV12:
            hal_format = HAL_PIXEL_FORMAT_YV12;
            break;
        case GRALLOC_EXTRA_BIT_CM_NV12:
            hal_format = HAL_PIXEL_FORMAT_NV12;
            break;
        case GRALLOC_EXTRA_BIT_CM_NV12_BLK:
            hal_format = HAL_PIXEL_FORMAT_NV12_BLK;
            break;
        case GRALLOC_EXTRA_BIT_CM_NV12_BLK_FCM:
            hal_format = HAL_PIXEL_FORMAT_NV12_BLK_FCM;
            break;
        case GRALLOC_EXTRA_BIT_CM_YUYV:
            hal_format = HAL_PIXEL_FORMAT_YCbCr_422_I;
            break;
        case GRALLOC_EXTRA_BIT_CM_I420:
            hal_format = HAL_PIXEL_FORMAT_I420;
            break;
        case GRALLOC_EXTRA_BIT_CM_UFO:
            hal_format = HAL_PIXEL_FORMAT_UFO;
            break;
        case GRALLOC_EXTRA_BIT_CM_NV12_BLK_10BIT_H:
            hal_format = HAL_PIXEL_FORMAT_NV12_BLK_10BIT_H;
            break;
        case GRALLOC_EXTRA_BIT_CM_NV12_BLK_10BIT_V:
            hal_format = HAL_PIXEL_FORMAT_NV12_BLK_10BIT_V;
            break;
        case GRALLOC_EXTRA_BIT_CM_UFO_10BIT_H:
            hal_format = HAL_PIXEL_FORMAT_UFO_10BIT_H;
            break;
        case GRALLOC_EXTRA_BIT_CM_UFO_10BIT_V:
            hal_format = HAL_PIXEL_FORMAT_UFO_10BIT_V;
            break;
        default:
            HWC_LOGE("%s, Gralloc format is invalid (0x%x)", __func__, fmt);
            break;
    }

    return hal_format;
}

struct PrivateHandle
{
    PrivateHandle()
        : ion_fd(-1)
        , fb_mva(0)
        , handle(NULL)
        , sec_handle(0)
        , roi_bytes(-1)
        , width(0)
        , height(0)
        , vstride(0)
        , v_align(0)
        , y_stride(0)
        , y_align(0)
        , cbcr_align(0)
        , deinterlace(0)
        , format(0)
        , size(0)
        , usage(0)
        , p_usage(0)
        , prexform(0)
        , alloc_id(0)
        , pq_enable(0)
        , pq_pos(0)
        , pq_orientation(0)
        , pq_table_idx(0)
        , layer_type(0)
    {
        memset(&pq_info, 0, sizeof(pq_info));
        memset(&ext_info, 0, sizeof(hdr_info));
        memset(&ext_info, 0, sizeof(ext_info));
        memset(&hwc_ext_info, 0, sizeof(hwc_ext_info));
    }
    int ion_fd;
    void* fb_mva;
    buffer_handle_t handle;
    SECHAND sec_handle;
    int roi_bytes;

    unsigned int width;
    unsigned int height;
    unsigned int vstride;
    unsigned int v_align;
    unsigned int y_stride;
    unsigned int y_align;
    unsigned int cbcr_align;
    unsigned int deinterlace;
    unsigned int format;
    int size; // total bytes allocated by gralloc
    int usage;
    uint64_t p_usage;

    uint32_t prexform;
    uint64_t alloc_id;

    uint32_t pq_enable;
    uint32_t pq_pos;
    uint32_t pq_orientation;
    uint32_t pq_table_idx;

    ge_pq_mira_vision_info_t pq_info;
    ge_hdr_info_t hdr_info;

    gralloc_extra_ion_sf_info_t ext_info;
    gralloc_extra_ion_hwc_info_t hwc_ext_info;

    // for LED HBM (High Backlight Mode) control, check is HBM or AOD layer or not
    int layer_type;
};

int setPrivateHandlePQInfo(const buffer_handle_t& handle, PrivateHandle* priv_handle);
int getPrivateHandleLayerType(const buffer_handle_t& handle, PrivateHandle* priv_handle);

inline int getPrivateHandleInfo(
    buffer_handle_t handle, PrivateHandle* priv_handle)
{
    if (NULL == handle)
    {
        HWC_LOGE("%s NULL handle !!!!!", __func__);
        return -EINVAL;
    }
    priv_handle->handle = handle;

    int err = 0;

    err |= gralloc_extra_query(handle, GRALLOC_EXTRA_GET_WIDTH, &priv_handle->width);
    err |= gralloc_extra_query(handle, GRALLOC_EXTRA_GET_HEIGHT, &priv_handle->height);
    err |= gralloc_extra_query(handle, GRALLOC_EXTRA_GET_STRIDE, &priv_handle->y_stride);
    err |= gralloc_extra_query(handle, GRALLOC_EXTRA_GET_VERTICAL_STRIDE, &priv_handle->vstride);
    err |= gralloc_extra_query(handle, GRALLOC_EXTRA_GET_FORMAT, &priv_handle->format);
    err |= gralloc_extra_query(handle, GRALLOC_EXTRA_GET_ALLOC_SIZE, &priv_handle->size);
    err |= gralloc_extra_query(handle, GRALLOC_EXTRA_GET_USAGE, &priv_handle->usage);
    err |= gralloc_extra_query(handle, GRALLOC_EXTRA_GET_PRODUCER_USAGE, &priv_handle->p_usage);

    err |= gralloc_extra_query(handle, GRALLOC_EXTRA_GET_PQ_MIRA_VISION_INFO, &priv_handle->pq_info);
    err |= gralloc_extra_query(handle, GRALLOC_EXTRA_GET_HDR_INFO, &priv_handle->hdr_info);
    err |= gralloc_extra_query(handle, GRALLOC_EXTRA_GET_IOCTL_ION_SF_INFO, &priv_handle->ext_info);
    err |= gralloc_extra_query(handle, GRALLOC_EXTRA_GET_HWC_INFO, &priv_handle->hwc_ext_info);

    if (priv_handle->vstride == 0)
    {
        priv_handle->vstride = priv_handle->height;
    }

    priv_handle->v_align = 0;
    priv_handle->y_align = 0;
    priv_handle->cbcr_align = 0;

    if (0 != (priv_handle->usage & GRALLOC_USAGE_CAMERA_ORIENTATION))
    {
        err |= gralloc_extra_query(handle, GRALLOC_EXTRA_GET_ORIENTATION, &priv_handle->prexform);

        if (0 != (HAL_TRANSFORM_ROT_90 & priv_handle->prexform))
        {
            SWAP(priv_handle->width, priv_handle->height);
            err |= gralloc_extra_query(handle, GRALLOC_EXTRA_GET_BYTE_2ND_STRIDE, &priv_handle->y_stride);
            err |= gralloc_extra_query(handle, GRALLOC_EXTRA_GET_VERTICAL_2ND_STRIDE, &priv_handle->vstride);
        }
    }
    else
    {
        priv_handle->prexform = 0;
    }

    if (err) HWC_LOGE("%s err(%x), (handle=%p)", __func__, err, handle);

    if (priv_handle->ext_info.videobuffer_status & 0x80000000)
    {
        unsigned int align = (priv_handle->ext_info.videobuffer_status & 0x7FFFFFFF) >> 25;
        align = SYMBOL2ALIGN(align);
        priv_handle->y_stride = ALIGN_CEIL(priv_handle->width, align);
        priv_handle->y_align = align;

        align = (priv_handle->ext_info.videobuffer_status & 0x01FFFFFF) >> 19;
        align = SYMBOL2ALIGN(align);
        priv_handle->cbcr_align = align;

        align = (priv_handle->ext_info.videobuffer_status & 0x0007FFFF) >> 13;
        align = SYMBOL2ALIGN(align);
        priv_handle->v_align = align;
        priv_handle->vstride = ALIGN_CEIL(priv_handle->height, align);

        priv_handle->deinterlace = (priv_handle->ext_info.videobuffer_status & 0x00001000) >> 11;
    }
    else
    {
        int format = priv_handle->format;
        if (format == HAL_PIXEL_FORMAT_YUV_PRIVATE)
        {
            format = grallocColor2HalColor(format, priv_handle->ext_info.status & GRALLOC_EXTRA_MASK_CM);
        }
        if (format == HAL_PIXEL_FORMAT_NV12_BLK  || (format == HAL_PIXEL_FORMAT_UFO || format == HAL_PIXEL_FORMAT_UFO_AUO )
            || format == HAL_PIXEL_FORMAT_YCbCr_420_888)
        {
            HWC_LOGE("%s, cannot interpret buffer layout", __func__);
        }
    }

    err |= getPrivateHandleLayerType(handle, priv_handle);

    return err;
}

inline int getPrivateHandleFBT(
    buffer_handle_t handle, PrivateHandle* priv_handle)
{
    if (NULL == handle)
    {
        HWC_LOGE("%s NULL handle !!!!!", __func__);
        return -EINVAL;
    }

    priv_handle->handle = handle;
    int err = getPrivateHandleInfo(handle, priv_handle);
    if (err != GRALLOC_EXTRA_OK)
    {
        HWC_LOGE("%s err(%x), (handle=%p)", __func__, err, handle);
        return -EINVAL;
    }

    err = gralloc_extra_query(handle, GRALLOC_EXTRA_GET_FB_MVA, &priv_handle->fb_mva);
    bool is_use_ion = (err != GRALLOC_EXTRA_OK);

    if (is_use_ion)
    {
        err = gralloc_extra_query(handle, GRALLOC_EXTRA_GET_ION_FD, &priv_handle->ion_fd);
        if (err != GRALLOC_EXTRA_OK)
        {
            HWC_LOGE("%s Failed to get ION fd, err(%x), (handle=%p) !!", __func__, err, handle);
            return -EINVAL;
        }
    }

    return 0;
}

inline int getPrivateHandleBuff(
    buffer_handle_t handle, PrivateHandle* priv_handle)
{
    if (NULL == handle)
    {
        HWC_LOGE("%s NULL handle !!!!!", __func__);
        return -EINVAL;
    }

    int err = 0;
    priv_handle->handle = handle;

    err |= gralloc_extra_query(handle, GRALLOC_EXTRA_GET_ION_FD, &priv_handle->ion_fd);

    if (priv_handle->usage & GRALLOC_USAGE_SECURE)
    {
        err |= gralloc_extra_query(handle, GRALLOC_EXTRA_GET_SECURE_HANDLE, &priv_handle->sec_handle);
    }
    else if ((priv_handle->ext_info.status & GRALLOC_EXTRA_MASK_SECURE) == GRALLOC_EXTRA_BIT_SECURE)
    {
        err |= gralloc_extra_query(handle, GRALLOC_EXTRA_GET_SECURE_HANDLE_HWC, &priv_handle->sec_handle);
    }

    if (err != GRALLOC_EXTRA_OK)
    {
        HWC_LOGE("%s err(%x), (handle=%p) !!", __func__, err, handle);
        return -EINVAL;
    }

    return 0;
}

inline int getPrivateHandle(
    buffer_handle_t handle, PrivateHandle* priv_handle)
{
    int err = getPrivateHandleInfo(handle, priv_handle);

    err |= getPrivateHandleBuff(handle, priv_handle);

    if (err != GRALLOC_EXTRA_OK)
        return -EINVAL;

    return 0;
}

inline int getIonFd(
    buffer_handle_t handle, PrivateHandle* priv_handle)
{
    priv_handle->handle = handle;
    int err = gralloc_extra_query(handle, GRALLOC_EXTRA_GET_ION_FD, &priv_handle->ion_fd);
    if (err != GRALLOC_EXTRA_OK) {
        HWC_LOGE("%s err(%x), (handle=%p) !!", __func__, err, handle);
        return -EINVAL;
    }

    return 0;
}

inline unsigned int getTimestamp(
    buffer_handle_t handle, PrivateHandle* priv_handle)
{
    if (handle && (priv_handle->ext_info.magic == 0))
        getPrivateHandleInfo(handle, priv_handle);

    return priv_handle->ext_info.timestamp;
}

inline void clearListFbt(struct hwc_display_contents_1* list)
{
    list->retireFenceFd = -1;
    if (list->outbufAcquireFenceFd != -1) closeFenceFd(&list->outbufAcquireFenceFd);
    list->outbufAcquireFenceFd = -1;

    hwc_layer_1_t* layer = &list->hwLayers[list->numHwLayers - 1];
    layer->releaseFenceFd = -1;
    if (layer->acquireFenceFd != -1) closeFenceFd(&layer->acquireFenceFd);
    layer->acquireFenceFd = -1;
}

inline void clearListAll(struct hwc_display_contents_1* list)
{
    list->retireFenceFd = -1;
    if (list->outbufAcquireFenceFd != -1) closeFenceFd(&list->outbufAcquireFenceFd);
    list->outbufAcquireFenceFd = -1;

    for (uint32_t i = 0; i < list->numHwLayers; i++)
    {
        hwc_layer_1_t* layer = &list->hwLayers[i];
        layer->releaseFenceFd = -1;
        if (layer->acquireFenceFd != -1) closeFenceFd(&layer->acquireFenceFd);
        layer->acquireFenceFd = -1;
    }
}

inline bool isSecure(const PrivateHandle* priv_handle)
{
    if (priv_handle->usage & GRALLOC_USAGE_SECURE)
        return true;

    return (priv_handle->ext_info.status & GRALLOC_EXTRA_MASK_SECURE) == GRALLOC_EXTRA_BIT_SECURE;
}

inline bool isCompressData(const PrivateHandle* priv_handle)
{
    bool is_compressed = false;
    if ((priv_handle->usage & GRALLOC_USAGE_HW_TEXTURE) &&
        (priv_handle->p_usage & GRALLOC1_PRODUCER_USAGE_PRIVATE_19))
        is_compressed = true;

    if (priv_handle->ext_info.status2 & GRALLOC_EXTRA_MASK2_BUF_COMPRESSION_STATUS)
        is_compressed = true;

    return is_compressed;
}

inline bool isG2GCompressData(const PrivateHandle* priv_handle)
{
    bool isG2GCompressed = false;

    if ((priv_handle->ext_info.status2 & GRALLOC_EXTRA_MASK2_BUF_COMPRESSION_STATUS) &&
        (priv_handle->ext_info.status2 & GRALLOC_EXTRA_MASK2_GPU_TO_GPU))
        isG2GCompressed = true;

    return isG2GCompressed;
}

inline bool isUIPq(const PrivateHandle* priv_handle)
{
    if (priv_handle->ext_info.status2 & GRALLOC_EXTRA_BIT2_UI_PQ_ON)
        return true;
    else
        return false;
}

inline bool isGameHDR(const PrivateHandle* priv_handle)
{
    const int&& type = (priv_handle->ext_info.status & GRALLOC_EXTRA_MASK_TYPE);
    if (priv_handle->hdr_info.u4TransformCharacter == 16 &&
        priv_handle->format == HAL_PIXEL_FORMAT_RGBA_1010102 &&
        type != GRALLOC_EXTRA_BIT_TYPE_VIDEO && type != GRALLOC_EXTRA_BIT_TYPE_CAMERA)
    {
        return true;
    }
    else
        return false;
}
// set "extra sf info" for marking this buffer as secure or normal
// if the caller need to get the final ext_info (ex: for synchronizing ext_info of priv_handle)
// fill rt_ext_info
// and this function will return the ext_info when rt_ext_info != NULL
void setSecExtraSfStatus(
    bool is_secure, buffer_handle_t hand, gralloc_extra_ion_sf_info_t* rt_ext_info = NULL);

// query sec_handle in order to check if this handle is with a secure buffer or not
// if there is a secure buffer attached, return the secure handle by filling *sec_handel
// if there isn't, allocate a secure buffer and query the secure handle, then fill *sec_handle
inline int getSecureHwcBuf(buffer_handle_t hand,
                                  SECHAND* sec_handle,
                                  GRALLOC_EXTRA_SECURE_BUFFER_TYPE* option = NULL)
{
    *sec_handle = 0;
    SECHAND tmp_sec_handle = 0;
    int err = gralloc_extra_query(hand, GRALLOC_EXTRA_GET_SECURE_HANDLE_HWC, &tmp_sec_handle);
    SVPLOGV("sbuf(+)", "query (err:%d h:%x sh:%x)", err, hand, tmp_sec_handle);

    if ((GRALLOC_EXTRA_OK != err) || (tmp_sec_handle == 0))
    {
        SVPLOGD("sbuf(+)", "alloc (h:%x)", hand);
        err = gralloc_extra_perform(hand, GRALLOC_EXTRA_ALLOC_SECURE_BUFFER_HWC, option);
        if (GRALLOC_EXTRA_OK != err)
        {
            SVPLOGD("sbuf(+)", "alloc fail (err:%d)", err);
        }

        err = gralloc_extra_query(hand, GRALLOC_EXTRA_GET_SECURE_HANDLE_HWC, &tmp_sec_handle);

        SVPLOGD("sbuf(+)", "query again (err:%d sh:%x)", err, tmp_sec_handle);
        if ((GRALLOC_EXTRA_OK != err) || (tmp_sec_handle == 0))
        {
            SVPLOGE("sbuf(+)", "fail!!!! (err:%d sh:%x)", err, tmp_sec_handle);
        }

        // clear flag, let buffer be treated as just initial.
        gralloc_extra_ion_sf_info_t ext_info;
        gralloc_extra_query(hand, GRALLOC_EXTRA_GET_IOCTL_ION_SF_INFO, &ext_info);
        gralloc_extra_sf_set_status(&ext_info, GRALLOC_EXTRA_MASK_ORIENT, 0);
        gralloc_extra_perform(hand, GRALLOC_EXTRA_SET_IOCTL_ION_SF_INFO, &ext_info);
    }
    if (GRALLOC_EXTRA_OK == err)
        *sec_handle = tmp_sec_handle;

    return err;
}

inline int freeSecureHwcBuf(buffer_handle_t hand)
{
    SECHAND tmp_sec_handle = 0;
    int err = gralloc_extra_query(hand, GRALLOC_EXTRA_GET_SECURE_HANDLE_HWC, &tmp_sec_handle);
    SVPLOGV("sbuf(-)", "query (err:%d sh:%x)", err, tmp_sec_handle);

    if ((GRALLOC_EXTRA_OK == err) && (tmp_sec_handle != 0))
    {
        SVPLOGD("sbuf(-)", "free (h:%x)", hand);
        err = gralloc_extra_perform(hand, GRALLOC_EXTRA_FREE_SEC_BUFFER_HWC, NULL);
        if (GRALLOC_EXTRA_OK != err)
        {
            SVPLOGE("sbuf(-)", "free fail (err:%d)", err);
        }

        err = gralloc_extra_query(hand, GRALLOC_EXTRA_GET_SECURE_HANDLE_HWC, &tmp_sec_handle);
        SVPLOGD("sbuf(-)", "query again (err:%d sh:%x)", err, tmp_sec_handle);
        if ((GRALLOC_EXTRA_OK == err) && (tmp_sec_handle != 0))
        {
            SVPLOGE("sbuf(-)", "fail!!!! (err:%d sh:%x)", err, tmp_sec_handle);
        }
    }
    return err;
}

inline status_t handleSecureBuffer(bool is_secure,
                                          buffer_handle_t hand,
                                          SECHAND* sec_handle,
                                          gralloc_extra_ion_sf_info_t* rt_ext_info = NULL)
{
    *sec_handle = 0;
    if (is_secure)
    {
        SECHAND tmp_sec_handle;
        int err = getSecureHwcBuf(hand, &tmp_sec_handle);
        if (GRALLOC_EXTRA_OK != err)
        {
            SVPLOGE("handleSecureBuffer", "Failed to allocate secure memory");
            return -EINVAL;
        }
        else
        {
            *sec_handle = tmp_sec_handle;
        }
    }
    else
    {
        freeSecureHwcBuf(hand);
    }
    setSecExtraSfStatus(is_secure, hand, rt_ext_info);
    return NO_ERROR;
}

inline void rectifyRectWithPrexform(Rect* roi, PrivateHandle* priv_handle)
{
    uint32_t prexform = priv_handle->prexform;
    if (0 == prexform)
        return;

    int w = priv_handle->width;
    int h = priv_handle->height;

    if (0 != (prexform & HAL_TRANSFORM_ROT_90))
    {
        SWAP(w, h);
    }

    Rect tmp = *roi;

    if (0 != (prexform & HAL_TRANSFORM_FLIP_V))
    {
        tmp.top     = h - roi->bottom;
        tmp.bottom  = h - roi->top;
    }

    if (0 != (prexform & HAL_TRANSFORM_FLIP_H))
    {
        tmp.left    = w - roi->right;
        tmp.right   = w - roi->left;
    }

    if (0 != (prexform & HAL_TRANSFORM_ROT_90))
    {
        roi->top     = tmp.left;
        roi->bottom  = tmp.right;
        roi->right   = h - tmp.top;
        roi->left    = h - tmp.bottom;
    }
    else
    {
        *roi = tmp;
    }
}

inline void rectifyXformWithPrexform(uint32_t* transform, uint32_t prexform)
{
    if (0 == prexform)
        return;

    uint32_t tmp = 0;
    prexform ^= (*transform & HAL_TRANSFORM_ROT_180);
    if (0 != (prexform & HAL_TRANSFORM_ROT_90))
    {
        if (0 != (prexform & HAL_TRANSFORM_FLIP_H))
            tmp |= HAL_TRANSFORM_FLIP_V;
        if (0 != (prexform & HAL_TRANSFORM_FLIP_V))
            tmp |= HAL_TRANSFORM_FLIP_H;
    }
    else
    {
        tmp = prexform & HAL_TRANSFORM_ROT_180;
    }

    if ((*transform & HAL_TRANSFORM_ROT_90) != (prexform & HAL_TRANSFORM_ROT_90))
    {
        tmp |= HAL_TRANSFORM_ROT_90;
        if (0 != (prexform & HAL_TRANSFORM_ROT_90))
            tmp ^= HAL_TRANSFORM_ROT_180;
    }
    *transform = tmp;
}

void dupBufferHandle(buffer_handle_t input, buffer_handle_t* output);

void freeDuppedBufferHandle(buffer_handle_t handle);

void calculateCrop(Rect* src_crop, Rect* dst_crop, Rect& dst_buf, uint32_t xform);

struct LayerInfo : public LightRefBase<LayerInfo>
{
    LayerInfo()
        : layer_idx(-1)
        , type(-1)
        , bpp(-1)
        , dirty(-1)
        , src_crop(0, 0, 0, 0)
        , dst_crop(0, 0, 0, 0)
        , mdp_dst_roi(0, 0, 0, 0)
    { }

    LayerInfo(const hwc_layer_1* const layer);

    // layer index in layerlist
    int layer_idx;

    // mark HWLayer::type values
    int type;

    // bpp of layer
    int bpp;

    // mark layer dirty
    int dirty;

    PrivateHandle priv_handle;

    Rect src_crop;
    Rect dst_crop;

    // for passing calculated MDP destination roi to HWLayer
    Rect mdp_dst_roi;
};

// a list of DispLayerInfo and provide primitive functions
class LayerList : public Vector<sp<LayerInfo> >
{
public:
    LayerList();
    ~LayerList();

    Rect getBounds();
    Rect getBounds(const int head, const int tail);
};

// ---------------------------------------------------------------------------
// BlackBuffer is a class for clearing background
// it is a singleton with a 128x128 RGB565 black buffer in it
// clear backgound by bliting this buffer to the destination buffer
// for a secure destination,
// getSecureHandle() can attach a zeor-initialized secure buffer to the original black buffer
// we can blit this black secure buffer to the secure destination

class BlackBuffer : public Singleton<BlackBuffer>
{
public:
    BlackBuffer();
    ~BlackBuffer();

    buffer_handle_t getHandle();

    // get secure handle, allocate secure buffer when m_sec_handle == 0
    void setSecure();
    void setNormal();

    Mutex m_lock;

private:
    buffer_handle_t m_handle;
};

class WhiteBuffer : public Singleton<WhiteBuffer>
{
public:
    WhiteBuffer();
    ~WhiteBuffer();

    buffer_handle_t getHandle();

    // get secure handle, allocate secure buffer when m_sec_handle == 0
    void setSecure();
    void setNormal();

    Mutex m_lock;

private:
    buffer_handle_t m_handle;
};
// ---------------------------------------------------------------------------
class IONDevice : public Singleton<IONDevice>
{
public:
    IONDevice();
    ~IONDevice();

    int getDeviceFd();
    int ionImport(int* ion_fd);
    int ionImport(const int32_t& ion_fd, int32_t* new_ion_fd, const char* = nullptr);
    int ionClose(int share_fd);
    int ionCloseAndSet(int* share_fd, const int& value = -1);

private:
    int m_dev_fd;
};

void regToGuiExt();

class AbortMessager : public android::Singleton<AbortMessager>
{
public:
    AbortMessager();
    ~AbortMessager();
    void printf(const char* msg, ...);
    void flushOut();
    void abort();
private:
    mutable Mutex m_lock;
    String8 m_msg_arr[MAX_ABORT_MSG];
    int m_begin;
};

#ifdef USE_HWC2

class HWCLayer;

int getSrcLeft(const sp<HWCLayer>& layer);
int getSrcTop(const sp<HWCLayer>& layer);
int getSrcWidth(const sp<HWCLayer>& layer);
int getSrcHeight(const sp<HWCLayer>& layer);

int getDstTop(const sp<HWCLayer>& layer);
int getDstBottom(const sp<HWCLayer>& layer);
int getDstLeft(const sp<HWCLayer>& layer);
int getDstRight(const sp<HWCLayer>& layer);

int getDstWidth(const sp<HWCLayer>& layer);
int getDstHeight(const sp<HWCLayer>& layer);

class HWCDisplay;

void copyHWCLayerIntoHwcLayer_1(const sp<HWCLayer>& from, hwc_layer_1_t* layer);

// listSecure() checks if there is any secure content in the display dist
bool listForceGPUComp(const std::vector<sp<HWCLayer> >& layers);
bool listSecure(const std::vector<sp<HWCLayer> >& layers);

struct DispatcherJob;
bool listS3d(const std::vector<sp<HWCLayer> >& layers, DispatcherJob* job = NULL);

#define protectedClose(fd) protectedCloseImpl(fd, __func__, __LINE__)
#define protectedDup(fd) protectedDupImpl(fd, __func__, __LINE__)
void protectedCloseImpl(const int32_t& fd, const char* str, const int32_t& line);
int protectedDupImpl(const int32_t& fd, const char* str, const int32_t& line);

const char* getCompString(const int32_t& comp_type);
const char* getBlendString(const int32_t& blend);

inline int32_t dupCloseFd(const int32_t& fd)
{
    if (fd == -1)
        return -1;

    const int32_t dup_fd = dup(fd);

    if ((3 > dup_fd && -1 < dup_fd) || (3 > fd && -1 < fd))
    {
        //TODO: add new way to dump CallStack
        //android::CallStack stack;
        //stack.update();
        //HWC_LOGW("dupAndCloseFd(): dupFence fd is zero call stack is (ori:%d dup:%d) %s",
        //    fd, dup_fd, stack.toString().string());
        HWC_LOGW("dupAndCloseFd(): dupFence fd is zero call stack is (ori:%d dup:%d)",
            fd, dup_fd);
        abort();
    }

    protectedClose(fd);
    return dup_fd;
}


inline int32_t dupCloseAndSetFd(int32_t* fd, const int value = -1)
{
    const int32_t dup_fd = dupCloseFd(*fd);
    *fd = value;
    return dup_fd;
}

void checkFd(const int32_t& fd);

inline bool isHwcRegionEqual(const hwc_region_t& lhs, const hwc_region_t& rhs)
{
    if (lhs.numRects != rhs.numRects)
    {
        if ((lhs.numRects + rhs.numRects) == 1)
        {
            const hwc_rect_t *tmp = lhs.numRects ? &lhs.rects[0] : &rhs.rects[0];
            if ((tmp->left == 0) && (tmp->top == 0) && (tmp->right == 0) && (tmp->bottom == 0))
            {
                return true;
            }
        }
        return false;
    }

    if (lhs.numRects == 0)
        return true;

    const int32_t len = sizeof(hwc_rect_t) * rhs.numRects;
    return (memcmp(lhs.rects, rhs.rects, len) == 0);
}

inline void copyHwcRegion(hwc_region_t* lhs, const hwc_region_t& rhs)
{
    const int32_t len = sizeof(hwc_rect_t) * rhs.numRects;
    if (lhs->numRects != rhs.numRects)
    {
        if (lhs->numRects != 0)
        {
            free((void*)(lhs->rects));
            lhs->rects = nullptr;
        }

        if (len != 0)
        {
            lhs->rects = (hwc_rect_t const*)malloc(len);
            if (lhs->rects == nullptr)
            {
                HWC_LOGE("malloc hwc_region failed: len=%d", len);
                lhs->numRects = 0;
            }
            else
            {
                lhs->numRects = rhs.numRects;
            }
        }
        else
        {
            lhs->rects = nullptr;
            lhs->numRects = 0;
        }
    }

    if (rhs.numRects != 0 && lhs->rects != nullptr)
        memcpy((void*)lhs->rects, (void*)rhs.rects, len);
}

inline int32_t mapColorMode2DataSpace(int32_t color_mode)
{
    int32_t color_space = HAL_DATASPACE_UNKNOWN;
    switch (color_mode)
    {
        case HAL_COLOR_MODE_NATIVE:
        case HAL_COLOR_MODE_SRGB:
            color_space = HAL_DATASPACE_V0_SRGB;
            break;

        case HAL_COLOR_MODE_DCI_P3:
        case HAL_COLOR_MODE_DISPLAY_P3:
            color_space = HAL_DATASPACE_DISPLAY_P3;
            break;

        case HAL_COLOR_MODE_ADOBE_RGB:
            color_space = HAL_DATASPACE_ADOBE_RGB;
            break;

        case HAL_COLOR_MODE_STANDARD_BT601_625:
        case HAL_COLOR_MODE_STANDARD_BT601_625_UNADJUSTED:
            color_space = HAL_DATASPACE_V0_BT601_625;
            break;

        case HAL_COLOR_MODE_STANDARD_BT601_525:
        case HAL_COLOR_MODE_STANDARD_BT601_525_UNADJUSTED:
            color_space = HAL_DATASPACE_V0_BT601_525;
            break;

        case HAL_COLOR_MODE_STANDARD_BT709:
            color_space = HAL_DATASPACE_V0_BT709;
            break;
    }

    return color_space;
}

nsecs_t getFenceSignalTime(const int32_t& fd);

void setPQEnhance(const int& dpy,
                  const PrivateHandle& priv_handle,
                  uint32_t* pq_enhance,
                  int* assigned_output_format,
                  const bool& is_game);

void setPQParam(DpPqParam* dppq_param,
                const uint32_t& is_enhance,
                const bool& is_uipq,
                const int32_t& pool_id,
                const bool& is_p3,
                const int32_t& dataspace,
                const uint32_t& time_stamp,
                const buffer_handle_t& handle,
                const uint32_t& pq_table_idx,
                const bool& is_game);
void mapP3PqParam(DpPqParam* dppq_param, const int32_t& out_ds);
int32_t getDstDataspace(const int32_t& dataspace);
bool isP3(const int32_t& dataspace);

#endif // USE_HWC2

#endif // UTILS_TOOLS_H_
