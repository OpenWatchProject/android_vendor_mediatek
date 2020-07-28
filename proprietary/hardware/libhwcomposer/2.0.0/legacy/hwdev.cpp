#define DEBUG_LOG_TAG "DEV"
#define ATRACE_TAG ATRACE_TAG_GRAPHICS

#include "utils/tools.h"
#include <cutils/properties.h>

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <fcntl.h>
#include <sys/ioctl.h>

#include <linux/fb.h>
#include <linux/mtkfb.h>

#include "m4u_lib.h"

#include "ddp_ovl.h"

#include "hwc_priv.h"

#include <hardware/gralloc.h>

#include "utils/debug.h"

#include <utils/Trace.h>
#include "hwdev.h"
#include "display.h"
#include "overlay.h"
#include "platform.h"
#include "hwc2.h"
#include "dispatcher.h"
#include "sync.h"
#ifdef USE_SWWATCHDOG
#include "utils/swwatchdog.h"

#define WDT_IOCTL(fd, CMD, ...)                                                             \
({                                                                                          \
    int err = 0;                                                                            \
    if (Platform::getInstance().m_config.wdt_trace)                                         \
    {                                                                                       \
        ATRACE_NAME(#CMD);                                                                  \
        SWWatchDog::AutoWDT _wdt(String8::format("[DEV] ioctl(" #CMD "):%d", __LINE__), 500);   \
        err = ioctl(fd, CMD, ##__VA_ARGS__);                                                \
    }                                                                                       \
    else                                                                                    \
    {                                                                                       \
        SWWatchDog::AutoWDT _wdt(String8::format("[DEV] ioctl(" #CMD "):%d", __LINE__), 500);   \
        err = ioctl(fd, CMD, ##__VA_ARGS__);                                                \
    }                                                                                       \
    err;                                                                                    \
})
#else // USE_SWWATCHDOG
#define WDT_IOCTL(fd, CMD, ...)                                                             \
({                                                                                          \
    int err = 0;                                                                            \
    if (Platform::getInstance().m_config.wdt_trace)                                         \
    {                                                                                       \
        ATRACE_NAME(#CMD);                                                                  \
        err = ioctl(fd, CMD, ##__VA_ARGS__);                                                \
    }                                                                                       \
    else                                                                                    \
    {                                                                                       \
        err = ioctl(fd, CMD, ##__VA_ARGS__);                                                \
    }                                                                                       \
    err;                                                                                    \
})
#endif // USE_SWWATCHDOG

#define HWC_ATRACE_BUFFER_INFO(string, n1, n2, n3, n4)                       \
    if (ATRACE_ENABLED()) {                                                   \
        char ___traceBuf[1024];                                               \
        snprintf(___traceBuf, 1024, "%s(%d:%d): %u %d", (string),             \
            (n1), (n2), (n3), (n4));                                          \
        android::ScopedTrace ___bufTracer(ATRACE_TAG, ___traceBuf);           \
    }

// ---------------------------------------------------------------------------

#define DLOGD(i, x, ...) HWC_LOGD("(%d) " x " id:%x", i, ##__VA_ARGS__, m_frame_cfg[i].session_id)
#define DLOGI(i, x, ...) HWC_LOGI("(%d) " x " id:%x", i, ##__VA_ARGS__, m_frame_cfg[i].session_id)
#define DLOGW(i, x, ...) HWC_LOGW("(%d) " x " id:%x", i, ##__VA_ARGS__, m_frame_cfg[i].session_id)
#define DLOGE(i, x, ...) HWC_LOGE("(%d) " x " id:%x", i, ##__VA_ARGS__, m_frame_cfg[i].session_id)

#define IOLOGE(i, err, x, ...) HWC_LOGE("(%d) " x " id:%x err:%d, %s", i, ##__VA_ARGS__, m_frame_cfg[i].session_id, err, strerror(err))

DISP_SESSION_TYPE g_session_type[DisplayManager::MAX_DISPLAYS] = {
    DISP_SESSION_PRIMARY,
    DISP_SESSION_EXTERNAL,
    DISP_SESSION_MEMORY
};

enum {
    EXT_DEVICE_MHL = 1,
    EXT_DEVICE_EPD = 2,
    EXT_DEVICE_LCM = 3,
};

static int g_ext_device_id[] = {
    EXT_DEVICE_MHL,
    EXT_DEVICE_EPD,
    EXT_DEVICE_LCM,
};

DISP_MODE mapHwcDispMode2Disp(HWC_DISP_MODE mode)
{
    switch (mode)
    {
        case HWC_DISP_INVALID_SESSION_MODE:
            return DISP_INVALID_SESSION_MODE;

        case HWC_DISP_SESSION_DIRECT_LINK_MODE:
            return DISP_SESSION_DIRECT_LINK_MODE;

        case HWC_DISP_SESSION_DECOUPLE_MODE:
            return DISP_SESSION_DECOUPLE_MODE;

        case HWC_DISP_SESSION_DIRECT_LINK_MIRROR_MODE:
            return DISP_SESSION_DIRECT_LINK_MIRROR_MODE;

        case HWC_DISP_SESSION_DECOUPLE_MIRROR_MODE:
            return DISP_SESSION_DECOUPLE_MIRROR_MODE;

        case HWC_DISP_SESSION_RDMA_MODE:
            return DISP_SESSION_RDMA_MODE;

        case HWC_DISP_SESSION_DUAL_DIRECT_LINK_MODE:
            return DISP_SESSION_DUAL_DIRECT_LINK_MODE;

        case HWC_DISP_SESSION_DUAL_DECOUPLE_MODE:
            return DISP_SESSION_DUAL_DECOUPLE_MODE;

        case HWC_DISP_SESSION_DUAL_RDMA_MODE:
            return DISP_SESSION_DUAL_RDMA_MODE;

        case HWC_DISP_SESSION_TRIPLE_DIRECT_LINK_MODE:
            return DISP_SESSION_TRIPLE_DIRECT_LINK_MODE;

        default:
            HWC_LOGW("failed to map HWC_DISP_MODE: %d", mode);
            return DISP_INVALID_SESSION_MODE;
    }
}

static HWC_DISP_MODE mapDispMode2Hwc(DISP_MODE mode)
{
    switch (mode)
    {
        case DISP_INVALID_SESSION_MODE:
            return HWC_DISP_INVALID_SESSION_MODE;

        case DISP_SESSION_DIRECT_LINK_MODE:
            return HWC_DISP_SESSION_DIRECT_LINK_MODE;

        case DISP_SESSION_DECOUPLE_MODE:
            return HWC_DISP_SESSION_DECOUPLE_MODE;

        case DISP_SESSION_DIRECT_LINK_MIRROR_MODE:
            return HWC_DISP_SESSION_DIRECT_LINK_MIRROR_MODE;

        case DISP_SESSION_DECOUPLE_MIRROR_MODE:
            return HWC_DISP_SESSION_DECOUPLE_MIRROR_MODE;

        case DISP_SESSION_RDMA_MODE:
            return HWC_DISP_SESSION_RDMA_MODE;

        case DISP_SESSION_DUAL_DIRECT_LINK_MODE:
            return HWC_DISP_SESSION_DUAL_DIRECT_LINK_MODE;

        case DISP_SESSION_DUAL_DECOUPLE_MODE:
            return HWC_DISP_SESSION_DUAL_DECOUPLE_MODE;

        case DISP_SESSION_DUAL_RDMA_MODE:
            return HWC_DISP_SESSION_DUAL_RDMA_MODE;

        case DISP_SESSION_TRIPLE_DIRECT_LINK_MODE:
            return HWC_DISP_SESSION_TRIPLE_DIRECT_LINK_MODE;

        default:
            HWC_LOGW("failed to map DISP_MODE: %d", mode);
            return HWC_DISP_INVALID_SESSION_MODE;
    }
}

static HWC_DISP_IF_TYPE mapDispIfType2Hwc(DISP_IF_TYPE type)
{
    switch (type)
    {
        case DISP_IF_TYPE_DBI:
            return HWC_DISP_IF_TYPE_DBI;

        case DISP_IF_TYPE_DPI:
            return HWC_DISP_IF_TYPE_DPI;

        case DISP_IF_TYPE_DSI0:
            return HWC_DISP_IF_TYPE_DSI0;

        case DISP_IF_TYPE_DSI1:
            return HWC_DISP_IF_TYPE_DSI1;

        case DISP_IF_TYPE_DSIDUAL:
            return HWC_DISP_IF_TYPE_DSIDUAL;

        case DISP_IF_HDMI:
            return HWC_DISP_IF_HDMI;

        case DISP_IF_HDMI_SMARTBOOK:
            return HWC_DISP_IF_HDMI_SMARTBOOK;

        case DISP_IF_MHL:
            return HWC_DISP_IF_MHL;

        case DISP_IF_EPD:
            return HWC_DISP_IF_EPD;

        case DISP_IF_SLIMPORT:
            return HWC_DISP_IF_SLIMPORT;

        default:
            HWC_LOGW("failed to map DISP_IF_TYPE: %d", type);
            return HWC_DISP_IF_TYPE_DBI;
    }
}

static HWC_DISP_IF_MODE mapDispIfMode2Hwc(DISP_IF_MODE mode)
{
    switch (mode)
    {
        case DISP_IF_MODE_VIDEO:
            return HWC_DISP_IF_MODE_VIDEO;

        case DISP_IF_MODE_COMMAND:
            return HWC_DISP_IF_MODE_COMMAND;

        default:
            HWC_LOGW("failed to map DISP_IF_MODE: %d", mode);
            return HWC_DISP_IF_MODE_VIDEO;
    }
}

static HWC_SELF_REFRESH_TYPE mapDispSelfRefreshType2Hwc(DISP_SELF_REFRESH_TYPE type)
{
    switch (type)
    {
        case WAIT_FOR_REFRESH:
            return HWC_WAIT_FOR_REFRESH;

        case REFRESH_FOR_ANTI_LATENCY2:
            return HWC_REFRESH_FOR_ANTI_LATENCY2;

        case REFRESH_FOR_SWITCH_DECOUPLE:
            return HWC_REFRESH_FOR_SWITCH_DECOUPLE;

        case REFRESH_FOR_SWITCH_DECOUPLE_MIRROR:
            return HWC_REFRESH_FOR_SWITCH_DECOUPLE_MIRROR;

        case REFRESH_FOR_IDLE:
            return HWC_REFRESH_FOR_IDLE;

        default:
            HWC_LOGW("failed to map DISP_SELF_REFRESH_TYPE: %d", type);
            return HWC_WAIT_FOR_REFRESH;
    }
}

static void covertDispSessionInfo2SessionInfo(SessionInfo *dst, disp_session_info *src)
{
    dst->maxLayerNum = src->maxLayerNum;
    dst->isHwVsyncAvailable = src->isHwVsyncAvailable;
    dst->displayType = mapDispIfType2Hwc(src->displayType);
    dst->displayWidth = src->displayWidth;
    dst->displayHeight = src->displayHeight;
    dst->displayFormat = src->displayFormat;
    dst->displayMode = mapDispIfMode2Hwc(src->displayMode);
    dst->vsyncFPS = src->vsyncFPS;
    dst->physicalWidth = src->physicalWidth;
    dst->physicalHeight = src->physicalHeight;
    dst->physicalWidthUm = src->physicalWidthUm;
    dst->physicalHeightUm = src->physicalHeightUm;
    dst->density = src->density;
    dst->isConnected = src->isConnected;
    dst->isHDCPSupported = src->isHDCPSupported;
    dst->is3DSupport = src->is3DSupport;
}

static DISP_FORMAT mapDispOutFormat(unsigned int format)
{
    switch (format)
    {
        case HAL_PIXEL_FORMAT_RGBA_8888:
            return DISP_FORMAT_RGBA8888;

        case HAL_PIXEL_FORMAT_YV12:
            return DISP_FORMAT_YV12;

        case HAL_PIXEL_FORMAT_RGBX_8888:
            return DISP_FORMAT_RGBX8888;

        case HAL_PIXEL_FORMAT_BGRA_8888:
            return DISP_FORMAT_BGRA8888;

        case HAL_PIXEL_FORMAT_BGRX_8888:
        case HAL_PIXEL_FORMAT_IMG1_BGRX_8888:
            return DISP_FORMAT_BGRX8888;

        case HAL_PIXEL_FORMAT_RGB_888:
            return DISP_FORMAT_RGB888;

        case HAL_PIXEL_FORMAT_RGB_565:
            return DISP_FORMAT_RGB565;

        case HAL_PIXEL_FORMAT_I420:
        case HAL_PIXEL_FORMAT_NV12_BLK:
        case HAL_PIXEL_FORMAT_NV12_BLK_FCM:
        case HAL_PIXEL_FORMAT_YUV_PRIVATE:
        case HAL_PIXEL_FORMAT_YUYV:
        case HAL_PIXEL_FORMAT_YCbCr_420_888:
            return DISP_FORMAT_YUV422;

        case HAL_PIXEL_FORMAT_RGBA_1010102:
            return DISP_FORMAT_RGBA1010102;

        case HAL_PIXEL_FORMAT_RGBA_FP16:
            return DISP_FORMAT_RGBA_FP16;
    }
    HWC_LOGW("Not support output format(%d), use default RGBA8888", format);
    return DISP_FORMAT_ABGR8888;
}

static DISP_YUV_RANGE_ENUM mapDispColorRange(uint32_t range, int format)
{
    switch (format)
    {
        case HAL_PIXEL_FORMAT_I420:
        case HAL_PIXEL_FORMAT_YV12:
        case HAL_PIXEL_FORMAT_NV12_BLK:
        case HAL_PIXEL_FORMAT_NV12_BLK_FCM:
        case HAL_PIXEL_FORMAT_YUV_PRIVATE:
        case HAL_PIXEL_FORMAT_YUYV:
        case HAL_PIXEL_FORMAT_UFO:
        case HAL_PIXEL_FORMAT_UFO_AUO:
        case HAL_PIXEL_FORMAT_YCbCr_422_I:
            switch (range)
            {
                case GRALLOC_EXTRA_BIT_YUV_BT601_NARROW:
                    return DISP_YUV_BT601;

                case GRALLOC_EXTRA_BIT_YUV_BT601_FULL:
                    return DISP_YUV_BT601_FULL;

                case GRALLOC_EXTRA_BIT_YUV_BT709_NARROW:
                case GRALLOC_EXTRA_BIT_YUV_BT709_FULL:
                case GRALLOC_EXTRA_BIT_YUV_BT2020_NARROW:
                case GRALLOC_EXTRA_BIT_YUV_BT2020_FULL:
                    return DISP_YUV_BT709;
            }
            HWC_LOGW("Not support range(%#x), use default BT601", range);
            break;
    }
    return DISP_YUV_BT601;
}

static DISP_YUV_RANGE_ENUM mapDispColorRangefromDS(uint32_t ds)
{
    switch (ds & HAL_DATASPACE_STANDARD_MASK)
    {
        case HAL_DATASPACE_STANDARD_BT601_625:
        case HAL_DATASPACE_STANDARD_BT601_625_UNADJUSTED:
        case HAL_DATASPACE_STANDARD_BT601_525:
        case HAL_DATASPACE_STANDARD_BT601_525_UNADJUSTED:
            return ((ds & HAL_DATASPACE_RANGE_MASK) == HAL_DATASPACE_RANGE_FULL) ?
                DISP_YUV_BT601_FULL : DISP_YUV_BT601;

        case HAL_DATASPACE_STANDARD_BT709:
        case HAL_DATASPACE_STANDARD_DCI_P3:
        case HAL_DATASPACE_STANDARD_BT2020:
            return DISP_YUV_BT709;

        case 0:
            switch (ds & 0xffff) {
                case HAL_DATASPACE_JFIF:
                case HAL_DATASPACE_BT601_625:
                case HAL_DATASPACE_BT601_525:
                    return DISP_YUV_BT601;

                case HAL_DATASPACE_SRGB_LINEAR:
                case HAL_DATASPACE_SRGB:
                case HAL_DATASPACE_BT709:
                    return DISP_YUV_BT709;
            }
    }

    return DISP_YUV_BT601;
}

static uint32_t mapHwcFeatureFlag(uint32_t flag)
{
    uint32_t tmp = 0;

    if (flag & HWC_FEATURE_TIME_SHARING)
    {
        tmp |= DISP_FEATURE_TIME_SHARING;
    }
    if (flag & HWC_FEATURE_HRT)
    {
        tmp |= DISP_FEATURE_HRT;
    }
    if (flag & HWC_FEATURE_PARTIAL)
    {
        tmp |= DISP_FEATURE_PARTIAL;
    }
    if (flag & HWC_FEATURE_FENCE_WAIT)
    {
        tmp |= DISP_FEATURE_FENCE_WAIT;
    }
    if (flag & HWC_FEATURE_NO_PARGB)
    {
        tmp |= DISP_FEATURE_NO_PARGB;
    }
    if (flag & HWC_FEATURE_DISP_SELF_REFRESH)
    {
        tmp |= DISP_FEATURE_DISP_SELF_REFRESH;
    }
    if (flag & HWC_FEATURE_RPO)
    {
        tmp |= DISP_FEATURE_RPO;
    }
    if (flag & HWC_FEATURE_FBDC)
    {
        tmp |= DISP_FEATURE_FBDC;
    }
    if (flag & HWC_FEATURE_FORCE_DISABLE_AOD)
    {
        tmp |= DISP_FEATURE_FORCE_DISABLE_AOD;
    }

    return tmp;
}

// ---------------------------------------------------------------------------

DISP_FORMAT mapDispInFormat(unsigned int format, int mode)
{
    if (!DispDevice::getInstance().isConstantAlphaForRGBASupported())
    {
        mode = HWC2_BLEND_MODE_NONE;
    }

    switch (format)
    {
        case HAL_PIXEL_FORMAT_RGBA_8888:
            return (HWC2_BLEND_MODE_PREMULTIPLIED == mode) ? DISP_FORMAT_PRGBA8888 : DISP_FORMAT_RGBA8888;

        case HAL_PIXEL_FORMAT_RGBX_8888:
            return DISP_FORMAT_RGBX8888;

        case HAL_PIXEL_FORMAT_BGRA_8888:
            return (HWC2_BLEND_MODE_PREMULTIPLIED == mode) ? DISP_FORMAT_PBGRA8888 : DISP_FORMAT_BGRA8888;

        case HAL_PIXEL_FORMAT_BGRX_8888:
        case HAL_PIXEL_FORMAT_IMG1_BGRX_8888:
            return DISP_FORMAT_BGRX8888;

        case HAL_PIXEL_FORMAT_RGB_888:
            return DISP_FORMAT_RGB888;

        case HAL_PIXEL_FORMAT_RGB_565:
            return DISP_FORMAT_RGB565;

        case HAL_PIXEL_FORMAT_I420:
        case HAL_PIXEL_FORMAT_YV12:
        case HAL_PIXEL_FORMAT_NV12_BLK:
        case HAL_PIXEL_FORMAT_NV12_BLK_FCM:
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
            return DISP_FORMAT_YUV422;

        case HAL_PIXEL_FORMAT_RGBA_1010102:
            return (HWC2_BLEND_MODE_PREMULTIPLIED == mode) ? DISP_FORMAT_PRGBA1010102 : DISP_FORMAT_RGBA1010102;

        case HAL_PIXEL_FORMAT_RGBA_FP16:
            return (HWC2_BLEND_MODE_PREMULTIPLIED == mode) ? DISP_FORMAT_PRGBA_FP16 : DISP_FORMAT_RGBA_FP16;

        case HAL_PIXEL_FORMAT_DIM:
            return DISP_FORMAT_DIM;
    }
    HWC_LOGW("Not support input format(%d), use default RGBA8888", format);
    return (HWC2_BLEND_MODE_PREMULTIPLIED == mode) ? DISP_FORMAT_PABGR8888 : DISP_FORMAT_ABGR8888;
}

DISP_FORMAT convertFormat4Hrt(unsigned int format)
{
    DISP_FORMAT hrt_format = mapDispInFormat(format);

    // when Platform does not support 2-subsampled format with odd size of roi, we
    // should not use YUYV. If the roi size of YUYV buffer is odd, ovl shows abnormal
    // screen content. Therefore we change the format to RGB888.
    if (Platform::getInstance().m_config.support_2subsample_with_odd_size_roi == false &&
        hrt_format == DISP_FORMAT_YUV422)
    {
        hrt_format = DISP_FORMAT_RGB888;
    }
    return hrt_format;
}

ANDROID_SINGLETON_STATIC_INSTANCE(DispDevice);

DispDevice::DispDevice()
{
    char filename[256];
    sprintf(filename, "/dev/%s", DISP_SESSION_DEVICE);
    m_dev_fd = open(filename, O_RDONLY);
    if (m_dev_fd <= 0)
    {
        HWC_LOGE("Failed to open display device: %s ", strerror(errno));
        abort();
    }

    // query hw capibilities and set to m_caps_info
    if (NO_ERROR != queryCapsInfo())
    {
        abort();
    }

    // get device multi configs info
    memset(m_multi_cfgs, 0, sizeof(struct multi_configs) * DisplayManager::MAX_DISPLAYS);
    if (isMultiConfigsSupport())
        getOverlayMultiConfigs(HWC_DISPLAY_PRIMARY);

    m_ovl_input_num = getMaxOverlayInputNum();

    memset(m_frame_cfg, 0, sizeof(disp_frame_cfg_t) * DisplayManager::MAX_DISPLAYS);
    memset(m_color_transform_info, 0, sizeof(DispColorTransformInfo) * DisplayManager::MAX_DISPLAYS);

    for (int i = 0; i < DisplayManager::MAX_DISPLAYS; i++)
    {
        m_frame_cfg[i].session_id = DISP_INVALID_SESSION;
        m_frame_cfg[i].mode = DISP_INVALID_SESSION_MODE;

        // partial update - allocate dirty rect structures for ioctl to disp driver
        m_hwdev_dirty_rect[i] = (layer_dirty_roi**)malloc(sizeof(layer_dirty_roi*) * m_ovl_input_num);
        LOG_ALWAYS_FATAL_IF(m_hwdev_dirty_rect[i] == nullptr, "DispDevice() malloc(%zu) fail",
            sizeof(layer_dirty_roi*) * m_ovl_input_num);
        for (int j = 0; j < m_ovl_input_num; j++)
        {
            m_hwdev_dirty_rect[i][j] = (layer_dirty_roi*)malloc(sizeof(layer_dirty_roi) * MAX_DIRTY_RECT_CNT);
            LOG_ALWAYS_FATAL_IF(m_hwdev_dirty_rect[i][j] == nullptr, "DispDevice() malloc(%zu) fail",
                sizeof(layer_dirty_roi) * MAX_DIRTY_RECT_CNT);
        }
    }

    memset(m_layer_config_list, 0, sizeof(layer_config*) * DisplayManager::MAX_DISPLAYS);
    memset(m_layer_config_len, 0, sizeof(int) * DisplayManager::MAX_DISPLAYS);
    memset(m_input_config, 0, sizeof(disp_session_input_config) * DisplayManager::MAX_DISPLAYS);
    memset(m_output_config, 0, sizeof(disp_session_output_config) * DisplayManager::MAX_DISPLAYS);
}

DispDevice::~DispDevice()
{
    // partial update - free dirty rect structures
    for (int i = 0; i < DisplayManager::MAX_DISPLAYS; i++)
    {
        for (int j = 0; j < m_ovl_input_num; j++)
        {
            free(m_hwdev_dirty_rect[i][j]);
        }
        free(m_hwdev_dirty_rect[i]);
        if (NULL != m_layer_config_list[i])
        {
            free(m_layer_config_list[i]);
        }
    }

    protectedClose(m_dev_fd);
}

void DispDevice::initOverlay()
{
    Platform::getInstance().initOverlay();
}

status_t DispDevice::queryCapsInfo()
{
    memset(&m_caps_info, 0, sizeof(disp_caps_info));

    // query device Capabilities
    int err = WDT_IOCTL(m_dev_fd, DISP_IOCTL_GET_DISPLAY_CAPS, &m_caps_info);
    if (err < 0)
    {
        IOLOGE(0, err, "DISP_IOCTL_GET_DISPLAY_CAPS");
        return err;
    }

    HWC_LOGD("CapsInfo [%d] lcm_degree",                 m_caps_info.lcm_degree);
    HWC_LOGD("CapsInfo [%d] output_rotated",             m_caps_info.is_output_rotated);
    HWC_LOGD("CapsInfo [%d] lcm_color_mode",             m_caps_info.lcm_color_mode);
    HWC_LOGD("CapsInfo [%d] disp_feature",               m_caps_info.disp_feature);
    HWC_LOGD("CapsInfo [%d] support_frame_cfg_ioctl",    m_caps_info.is_support_frame_cfg_ioctl);
    HWC_LOGD("CapsInfo [%d,%d,%d] rpo,rsz_in_max(w,h)",
        m_caps_info.disp_feature & DISP_FEATURE_RPO,
        m_caps_info.rsz_in_max[0], m_caps_info.rsz_in_max[1]);
    HWC_LOGD("CapsInfo [%d]dispRszSupported",           isDispRszSupported());
    HWC_LOGD("CapsInfo [%d]fbdc",                       m_caps_info.disp_feature & DISP_FEATURE_FBDC);
    HWC_LOGD("CapsInfo [%d]multiConfigs",                m_caps_info.disp_feature & DISP_FEATURE_DYNFPS);
    // print supported resolutions
    if (isDispRszSupported())
    {
        for (uint32_t i = 0; i < m_caps_info.rsz_list_length; ++i)
        {
            HWC_LOGD(" (%d, %d, %d)",
                m_caps_info.rsz_in_res_list[i][0],
                m_caps_info.rsz_in_res_list[i][1],
                m_caps_info.rsz_in_res_list[i][2]);
        }
    }

    const bool isSelfRefreshSupported = isDispSelfRefreshSupported();
    HWC_LOGD("CapsInfo [%d]isDispSelfRefreshSupported", isSelfRefreshSupported);

    return NO_ERROR;
}

uint32_t DispDevice::getDisplayRotation(uint32_t dpy)
{
    uint32_t rotation = 0;
    if (HWC_DISPLAY_PRIMARY == dpy)
    {
        rotation = (uint32_t)m_caps_info.lcm_degree & 0x7;
        if (1 == m_caps_info.is_output_rotated)
        {
            rotation ^= HAL_TRANSFORM_ROT_180;
        }
    }
    return rotation;
}

bool DispDevice::isDispRszSupported()
{
    return (0 != (m_caps_info.disp_feature & DISP_FEATURE_RSZ));
}

bool DispDevice::isDispRpoSupported()
{
    return m_caps_info.disp_feature & DISP_FEATURE_RPO;
}

bool DispDevice::isDispAodForceDisable()
{
    return m_caps_info.disp_feature & DISP_FEATURE_FORCE_DISABLE_AOD;
}

bool DispDevice::isPartialUpdateSupported()
{
    if (Platform::getInstance().m_config.force_full_invalidate)
    {
        HWC_LOGW("!!!! force full invalidate !!!!");
        return false;
    }

    return (0 != (m_caps_info.disp_feature & DISP_FEATURE_PARTIAL));
}

bool DispDevice::isFenceWaitSupported()
{
    if (Platform::getInstance().m_config.wait_fence_for_display)
    {
        HWC_LOGW("!!!! force hwc wait fence for display !!!!");
        return false;
    }

    return m_caps_info.disp_feature & DISP_FEATURE_FENCE_WAIT;
}

bool DispDevice::isConstantAlphaForRGBASupported()
{
    return  (0 == (m_caps_info.disp_feature & DISP_FEATURE_NO_PARGB));
}

bool DispDevice::isDispSelfRefreshSupported()
{
    return m_caps_info.disp_feature & DISP_FEATURE_DISP_SELF_REFRESH;
}

bool DispDevice::isDisplayHrtSupport()
{
    return m_caps_info.disp_feature & DISP_FEATURE_HRT;
}

bool DispDevice::isMultiConfigsSupport()
{
    return m_caps_info.disp_feature & DISP_FEATURE_DYNFPS;

}

int32_t DispDevice::getSupportedColorMode()
{
    return m_caps_info.lcm_color_mode;
}

int DispDevice::getMaxOverlayInputNum()
{
    // TODO: deinfe in ddp_ovl.h now.
    // This's header file for kernel, not for userspace. Need to refine later
    return OVL_LAYER_NUM;
}

uint32_t DispDevice::getMaxOverlayHeight()
{
    // deinfe in ddp_ovl.h now
    return OVL_MAX_HEIGHT;
}

uint32_t DispDevice::getMaxOverlayWidth()
{
    // deinfe in ddp_ovl.h.
    return OVL_MAX_WIDTH;
}

int32_t DispDevice::getDisplayOutputRotated()
{
    return m_caps_info.is_output_rotated;
}

uint32_t DispDevice::getRszMaxWidthInput()
{
    return m_caps_info.rsz_in_max[0];
}

uint32_t DispDevice::getRszMaxHeightInput()
{
    return m_caps_info.rsz_in_max[1];
}

void DispDevice::enableDisplayFeature(uint32_t flag)
{
    m_caps_info.disp_feature |= mapHwcFeatureFlag(flag);
}

void DispDevice::disableDisplayFeature(uint32_t flag)
{
    m_caps_info.disp_feature &= ~mapHwcFeatureFlag(flag);
}

status_t DispDevice::createOverlaySession(int dpy, HWC_DISP_MODE mode)
{
    int session_id = m_frame_cfg[dpy].session_id;
    if (DISP_INVALID_SESSION != session_id)
    {
        HWC_LOGW("(%d) Failed to create existed DispSession (id=0x%x)", dpy, session_id);
        return INVALID_OPERATION;
    }

    disp_session_config config;
    memset(&config, 0, sizeof(disp_session_config));

    config.type       = g_session_type[dpy];
    config.device_id  = getDeviceId(dpy);
    config.mode       = mapHwcDispMode2Disp(mode);
    config.session_id = DISP_INVALID_SESSION;

    int err = WDT_IOCTL(m_dev_fd, DISP_IOCTL_CREATE_SESSION, &config);
    if (err < 0)
    {
        m_frame_cfg[dpy].session_id = DISP_INVALID_SESSION;
        m_frame_cfg[dpy].mode = DISP_INVALID_SESSION_MODE;

        IOLOGE(dpy, err, "DISP_IOCTL_CREATE_SESSION (%s)", getSessionModeString(mode).string());
        return BAD_VALUE;
    }

    m_frame_cfg[dpy].session_id = config.session_id;
    m_frame_cfg[dpy].mode = config.mode;

    DLOGD(dpy, "Create Session (%s)", getSessionModeString(mode).string());

    return NO_ERROR;
}

void DispDevice::destroyOverlaySession(int dpy)
{
    int session_id = m_frame_cfg[dpy].session_id;
    if (DISP_INVALID_SESSION == session_id)
    {
        HWC_LOGW("(%d) Failed to destroy invalid DispSession", dpy);
        return;
    }

    disp_session_config config;
    memset(&config, 0, sizeof(disp_session_config));

    config.type       = g_session_type[dpy];
    config.device_id  = getDeviceId(dpy);
    config.session_id = session_id;

    int err = WDT_IOCTL(m_dev_fd, DISP_IOCTL_DESTROY_SESSION, &config);
    if (err < 0)
    {
        IOLOGE(dpy, err, "DISP_IOCTL_DESTROY_SESSION");
    }

    m_frame_cfg[dpy].session_id = DISP_INVALID_SESSION;
    m_frame_cfg[dpy].mode = DISP_INVALID_SESSION_MODE;

    DLOGD(dpy, "Destroy DispSession");
}

status_t DispDevice::legacySetInputBuffer(int dpy)
{
    if (m_caps_info.is_support_frame_cfg_ioctl)
        return NO_ERROR;

    memset(&m_input_config[dpy], 0, sizeof(disp_session_input_config));

    m_input_config[dpy].session_id = m_frame_cfg[dpy].session_id;
    m_input_config[dpy].config_layer_num = m_frame_cfg[dpy].input_layer_num;
    int size = m_input_config[dpy].config_layer_num * sizeof(disp_input_config);
    memcpy(m_input_config[dpy].config, m_frame_cfg[dpy].input_cfg, size);
    memcpy(&m_input_config[dpy].ccorr_config, &m_frame_cfg[dpy].ccorr_config, sizeof(m_frame_cfg[dpy].ccorr_config));

    int err = WDT_IOCTL(m_dev_fd, DISP_IOCTL_SET_INPUT_BUFFER, &m_input_config[dpy]);
    if (err < 0)
    {
        IOLOGE(dpy, err, "DISP_IOCTL_SET_INPUT_BUFFER");
    }

    return err;
}

status_t DispDevice::legacySetOutputBuffer(int dpy)
{
    if (m_caps_info.is_support_frame_cfg_ioctl)
        return NO_ERROR;

    memset(&m_output_config[dpy], 0, sizeof(disp_session_output_config));

    m_output_config[dpy].session_id = m_frame_cfg[dpy].session_id;
    memcpy(&m_output_config[dpy].config, &m_frame_cfg[dpy].output_cfg, sizeof(disp_output_config));

    int err = WDT_IOCTL(m_dev_fd, DISP_IOCTL_SET_OUTPUT_BUFFER, &m_output_config[dpy]);
    if (err < 0)
    {
        IOLOGE(dpy, err, "DISP_IOCTL_SET_OUTPUT_BUFFER");
    }

    return err;
}

status_t DispDevice::legacyTriggerSession(int dpy, int pf_idx)
{
    if (m_caps_info.is_support_frame_cfg_ioctl)
        return NO_ERROR;

    disp_session_config config;
    memset(&config, 0, sizeof(disp_session_config));

    config.type       = g_session_type[dpy];
    config.device_id  = getDeviceId(dpy);
    config.session_id = m_frame_cfg[dpy].session_id;
    config.present_fence_idx = pf_idx;

    int err = WDT_IOCTL(m_dev_fd, DISP_IOCTL_TRIGGER_SESSION, &config);
    if (err < 0)
    {
        IOLOGE(dpy, err, "DISP_IOCTL_TRIGGER_SESSION pf_idx=%d", pf_idx);
    }
    else
    {
        DLOGD(dpy, "DISP_IOCTL_TRIGGER_SESSION pf_idx=%d", pf_idx);
    }

    return err;
}

status_t DispDevice::frameConfig(int dpy, int pf_idx, int ovlp_layer_num,
                                 int prev_present_fence_fd, hwc2_config_t config,
                                 const uint32_t& hrt_weight, const uint32_t& hrt_idx,
                                 const bool& is_HBM)
{
    if (-1 == ovlp_layer_num)
    {
        HWC_LOGW("ovlp_layer_num is not available, calculate roughly ...");
        ovlp_layer_num = 0;
        for (int i = 0; i < m_ovl_input_num; i++)
        {
            disp_input_config* input = &m_frame_cfg[dpy].input_cfg[i];
            if (1 == input->layer_enable)
                ovlp_layer_num++;
        }
    }

    m_frame_cfg[dpy].present_fence_idx = pf_idx;
    m_frame_cfg[dpy].overlap_layer_num = ovlp_layer_num;
    m_frame_cfg[dpy].tigger_mode = TRIGGER_NORMAL;
    m_frame_cfg[dpy].prev_present_fence_fd = prev_present_fence_fd;
    m_frame_cfg[dpy].active_config = config;
    m_frame_cfg[dpy].hrt_weight = hrt_weight;
    m_frame_cfg[dpy].hrt_idx = hrt_idx;
    m_frame_cfg[dpy].hbm_en = is_HBM;

    int err = WDT_IOCTL(m_dev_fd, DISP_IOCTL_FRAME_CONFIG, &m_frame_cfg[dpy]);
    HWC_ATRACE_FORMAT_NAME("active_config(%d)", config);
    if (err < 0)
    {
        IOLOGE(dpy, err, "DISP_IOCTL_FRAME_CONFIG ovlp:%d pf_idx=%d", ovlp_layer_num, pf_idx);
    }
    else
    {
        DbgLogger logger(DbgLogger::TYPE_HWC_LOG, 'D');
        logger.printf("(%d) DISP_IOCTL_FRAME_CONFIG ovlp:%d pf_idx=%d id:%x hrt:%u,%u hbm:%d",
                                    dpy, ovlp_layer_num, pf_idx, m_frame_cfg[dpy].session_id, hrt_weight, hrt_idx, is_HBM);
    }

    return err;
}

bool DispDevice::queryValidLayer(void* ptr)
{
    disp_layer_info* disp_layer = static_cast<disp_layer_info*>(ptr);
    int err = WDT_IOCTL(m_dev_fd, DISP_IOCTL_QUERY_VALID_LAYER, disp_layer);

    if (err < 0)
    {
        IOLOGE(HWC_DISPLAY_PRIMARY, err, "DISP_IOCTL_QUERY_VALID_LAYER: %d", err);
        return false;
    }

    return (disp_layer->hrt_num != -1) ? true : false;
}

status_t DispDevice::triggerOverlaySession(int dpy, int present_fence_idx, int ovlp_layer_num,
                                           int prev_present_fence_fd, hwc2_config_t config,
                                           const uint32_t& hrt_weight, const uint32_t& hrt_idx,
                                           const bool& is_HBM)
{
    if (DISP_INVALID_SESSION == static_cast<int32_t>(m_frame_cfg[dpy].session_id))
    {
        HWC_LOGW("(%d) Failed to trigger invalid DispSession", dpy);
        return INVALID_OPERATION;
    }

    HWC_ATRACE_FORMAT_NAME("TrigerOVL:%d", present_fence_idx);
    if (!m_caps_info.is_support_frame_cfg_ioctl)
        return legacyTriggerSession(dpy, present_fence_idx);
    else
       return frameConfig(dpy, present_fence_idx, ovlp_layer_num, prev_present_fence_fd, config,
                          hrt_weight, hrt_idx, is_HBM);
}

void DispDevice::disableOverlaySession(
    int dpy,  OverlayPortParam* const* params, int num)
{
    int session_id = m_frame_cfg[dpy].session_id;
    if (DISP_INVALID_SESSION == session_id)
    {
        HWC_LOGW("(%d) Failed to disable invalid DispSession", dpy);
        return;
    }

    {
        DbgLogger logger(DbgLogger::TYPE_HWC_LOG, 'D', "(%d) disableOverlaySession ", dpy);
        for (int i = 0; i < m_ovl_input_num; i++)
        {
            disp_input_config* input = &m_frame_cfg[dpy].input_cfg[i];

            if (i >= num)
            {
                input->layer_id = m_ovl_input_num + 1;
                continue;
            }

            input->layer_id     = i;
            input->layer_enable = 0;
            input->next_buff_idx = params[i]->fence_index;

            logger.printf("-%d,idx=%d/ ", i, input->next_buff_idx);
        }
        m_frame_cfg[dpy].input_layer_num = (num < m_ovl_input_num) ? num : m_ovl_input_num;
    }

    if (!m_caps_info.is_support_frame_cfg_ioctl)
        legacySetInputBuffer(dpy);

    disableOverlayOutput(dpy);
    triggerOverlaySession(dpy, -1, 0, -1, 0, 0, 0);

    DLOGD(dpy, "Disable DispSession");
}

status_t DispDevice::setOverlaySessionMode(int dpy, HWC_DISP_MODE mode)
{
    int session_id = m_frame_cfg[dpy].session_id;
    if (DISP_INVALID_SESSION == session_id)
    {
        HWC_LOGW("(%d) Failed to set invalid DispSession (mode)", dpy);
        return INVALID_OPERATION;
    }

    disp_session_config config;
    memset(&config, 0, sizeof(disp_session_config));

    config.device_id  = getDeviceId(dpy);
    config.session_id = session_id;
    config.mode       = mapHwcDispMode2Disp(mode);

    int err = WDT_IOCTL(m_dev_fd, DISP_IOCTL_SET_SESSION_MODE, &config);
    if (err < 0)
    {
        IOLOGE(dpy, err, "DISP_IOCTL_SET_SESSION_MODE %s", getSessionModeString(mode).string());
        return BAD_VALUE;
    }

    m_frame_cfg[dpy].mode = config.mode;

    DLOGD(dpy, "DispSessionMode (%s)", getSessionModeString(mode).string());
    return NO_ERROR;
}

HWC_DISP_MODE DispDevice::getOverlaySessionMode(int dpy)
{
    int session_id = m_frame_cfg[dpy].session_id;
    if (DISP_INVALID_SESSION == session_id)
    {
        HWC_LOGW("(%d) Failed to get invalid DispSession", dpy);
        return HWC_DISP_INVALID_SESSION_MODE;
    }

    return mapDispMode2Hwc(m_frame_cfg[dpy].mode);
}

status_t DispDevice::getOverlaySessionInfo(int dpy, SessionInfo* info)
{
    int session_id = m_frame_cfg[dpy].session_id;
    if (DISP_INVALID_SESSION == session_id)
    {
        HWC_LOGW("(%d) Failed to get info for invalid DispSession", dpy);
        return INVALID_OPERATION;
    }

    disp_session_info tmp_info;
    memset(&tmp_info, 0, sizeof(tmp_info));
    tmp_info.session_id = session_id;

    int err = WDT_IOCTL(m_dev_fd, DISP_IOCTL_GET_SESSION_INFO, &tmp_info);
    if (err < 0)
    {
        IOLOGE(dpy, err, "DISP_IOCTL_GET_SESSION_INFO");
    }
    else
    {
        covertDispSessionInfo2SessionInfo(info, &tmp_info);
    }

    return NO_ERROR;
}

status_t DispDevice::getOverlayMultiConfigs(int dpy)
{
    if (!isMultiConfigsSupport())
    {
        HWC_LOGW("Not support to get multi configs");
        return INVALID_OPERATION;
    }
    if (dpy != HWC_DISPLAY_PRIMARY )
    {
        HWC_LOGW("(%d) Failed to get multi configs for ", dpy);
        return INVALID_OPERATION;
    }

    struct multi_configs tmp_configs;
    memset(&tmp_configs, 0, sizeof(tmp_configs));
    int err = WDT_IOCTL(m_dev_fd, DISP_IOCTL_GET_MULTI_CONFIGS, &tmp_configs);
    if (err < 0)
    {
        IOLOGE(dpy, err, "DISP_IOCTL_GET_MULTI_CONFIGS");
    }
    else
    {
        m_multi_cfgs[dpy].config_num = tmp_configs.config_num;
        for (int i = 0; i < m_multi_cfgs[dpy].config_num; i++)
        {
            m_multi_cfgs[dpy].dyn_cfgs[i].vsyncFPS = tmp_configs.dyn_cfgs[i].vsyncFPS;
            m_multi_cfgs[dpy].dyn_cfgs[i].width = tmp_configs.dyn_cfgs[i].width;
            m_multi_cfgs[dpy].dyn_cfgs[i].height = tmp_configs.dyn_cfgs[i].height;
        }
    }

    return NO_ERROR;
}

int DispDevice::getAvailableOverlayInput(int dpy)
{
    SessionInfo info;

    getOverlaySessionInfo(dpy, &info);

    return info.maxLayerNum;
}

void DispDevice::prepareOverlayInput(
    int dpy, OverlayPrepareParam* param)
{
    int session_id = m_frame_cfg[dpy].session_id;
    if (DISP_INVALID_SESSION == session_id)
    {
        HWC_LOGW("(%d) Failed to preapre invalid DispSession (input)", dpy);
        return;
    }

    disp_buffer_info buffer;
    memset(&buffer, 0, sizeof(disp_buffer_info));

    buffer.session_id = session_id;
    buffer.layer_id   = param->id;
    buffer.layer_en   = 1;
    buffer.ion_fd     = param->ion_fd;
    buffer.cache_sync = param->is_need_flush;
    buffer.index      = -1;
    buffer.fence_fd   = -1;

    int err = WDT_IOCTL(m_dev_fd, DISP_IOCTL_PREPARE_INPUT_BUFFER, &buffer);
    if (err < 0)
    {
        IOLOGE(dpy, err, "DISP_IOCTL_PREPARE_INPUT_BUFFER");
    }
    param->fence_index = buffer.index;
    param->fence_fd    = dupCloseAndSetFd(&buffer.fence_fd);

    HWC_ATRACE_BUFFER_INFO("pre_input",
        dpy, param->id, param->fence_index, param->fence_fd);

    HWC_LOGV("DispDevice::prepareOverlayInput() dpy:%d id:%d ion:%d fence_idx:%d fence_fd:%d",
        dpy, param->id, param->ion_fd, param->fence_index, param->fence_fd);
}

void DispDevice::updateOverlayInputs(
    int dpy, OverlayPortParam* const* params, int num, sp<ColorTransform> color_transform)
{
    HWC_LOGV("+ updateOverlayInputs num:%d", num);
    int session_id = m_frame_cfg[dpy].session_id;
    if (DISP_INVALID_SESSION == session_id)
    {
        HWC_LOGW("(%d) Failed to update invalid DispSession (input)", dpy);
        return;
    }

    DbgLogger logger(DbgLogger::TYPE_HWC_LOG, 'D', "(%d) Input: ", dpy);
    DbgLogger logger_dirty(DbgLogger::TYPE_HWC_LOG, 'V', "[DIRTY] dev(xywh)");

    bool is_fbt_exist = false;
    size_t non_fbt_layer_num = 0;
    int i = 0;
    size_t layer_enable_amount = 0;
    for (; i < m_ovl_input_num; i++)
    {
        if (i >= num) break;

        HWC_LOGV("i:%d params[i]->state:%d", i, params[i]->state);

        disp_input_config* input = &m_frame_cfg[dpy].input_cfg[i];

        if (OVL_IN_PARAM_DISABLE == params[i]->state)
        {
            input->layer_id      = i;
            input->layer_enable  = 0;
            input->next_buff_idx = params[i]->fence_index;
            input->src_fence_fd = -1;

            logger.printf("-%d,idx:%d/ ", i, params[i]->fence_index);
            continue;
        }
        else
        {
            ++layer_enable_amount;
        }

        //some crop may be empty(w/h = 0), don't enable layer if crop is empty
        if (params[i]->src_crop.getWidth() < 1 ||
            params[i]->src_crop.getHeight() < 1 ||
            params[i]->dst_crop.getWidth() < 1 ||
            params[i]->dst_crop.getHeight() < 1)
        {
            input->layer_id      = i;
            input->layer_enable  = 0;
            input->next_buff_idx = params[i]->fence_index;
            input->src_fence_fd = -1;

            HWC_LOGW("(%d:%d) Layer- (idx=%d,w/h=0) ", dpy, i, input->next_buff_idx);
            continue;
        }

        input->layer_id       = i;
        input->layer_enable   = 1;

        if (params[i]->dim)
        {
            input->buffer_source = DISP_BUFFER_ALPHA;
            input->dim_color = params[i]->layer_color;
        }
        else if (params[i]->ion_fd == HWC_NO_ION_FD)
        {
            input->buffer_source = DISP_BUFFER_MVA;
        }
        else
        {
            input->buffer_source = DISP_BUFFER_ION;
        }

        // setup layer type for S3D info to driver
        if ((params[i]->is_s3d_layer) && (!HWCMediator::getInstance().m_features.hdmi_s3d_debug))
        {
            if (params[i]->s3d_type == HWC_IS_S3D_LAYER_TAB)
            {
                input->layer_type = DISP_LAYER_3D_TAB_0;
            }
            else if(params[i]->s3d_type == HWC_IS_S3D_LAYER_SBS)
            {
                input->layer_type = DISP_LAYER_3D_SBS_0;
            }
        }
        else
        {
            input->layer_type = DISP_LAYER_2D;
        }

        input->layer_rotation = DISP_ORIENTATION_0;
        input->src_base_addr  = params[i]->va;
        input->src_phy_addr   = params[i]->mva;
        input->src_pitch      = params[i]->pitch;
        input->src_v_pitch    = params[i]->v_pitch;
        input->src_fmt        = mapDispInFormat(params[i]->format, params[i]->blending);
        input->src_offset_x   = params[i]->src_crop.left;
        input->src_offset_y   = params[i]->src_crop.top;
        input->src_width      = params[i]->src_crop.getWidth();
        input->src_height     = params[i]->src_crop.getHeight();
        input->tgt_offset_x   = params[i]->dst_crop.left;
        input->tgt_offset_y   = params[i]->dst_crop.top;
        input->tgt_width      = params[i]->dst_crop.getWidth();
        input->tgt_height     = params[i]->dst_crop.getHeight();
        input->isTdshp        = params[i]->is_sharpen;
        input->next_buff_idx  = params[i]->fence_index;
        input->identity       = params[i]->identity;
        if (params[i]->identity == HWC_LAYER_TYPE_FBT)
        {
            is_fbt_exist = true;
        }
        else
        {
            ++non_fbt_layer_num;
        }

        input->connected_type = params[i]->connected_type;
        input->alpha_enable   = params[i]->alpha_enable;
        input->alpha          = params[i]->alpha;
        input->frm_sequence   = params[i]->sequence;
        input->ext_sel_layer  = params[i]->ext_sel_layer;
        input->yuv_range      = mapDispColorRange(params[i]->color_range, params[i]->format);
        // map yuv from dataspace since disp driver apply yuv range for mm layer
        if (Platform::getInstance().m_config.use_dataspace_for_yuv)
            input->yuv_range = mapDispColorRangefromDS(params[i]->dataspace);
        // widegamut series layer uses dataspace to config
        input->dataspace      = params[i]->dataspace;
        input->src_fence_fd   = params[i]->fence;

        // partial update - fill dirty rects info
        logger_dirty.printf(" (%d,n:%d)", i, params[i]->ovl_dirty_rect_cnt);
        if (0 == params[i]->ovl_dirty_rect_cnt)
        {
            input->dirty_roi_num = 0;
            logger_dirty.printf("NULL", i);
        }
        else
        {
            layer_dirty_roi* hwdev_dirty_rect = m_hwdev_dirty_rect[dpy][i];
            hwc_rect_t* ovl_dirty_rect = params[i]->ovl_dirty_rect;
            for (int j = 0; j < params[i]->ovl_dirty_rect_cnt; j++)
            {
                hwdev_dirty_rect[j].dirty_x = ovl_dirty_rect[j].left;
                hwdev_dirty_rect[j].dirty_y = ovl_dirty_rect[j].top;
                hwdev_dirty_rect[j].dirty_w = ovl_dirty_rect[j].right - ovl_dirty_rect[j].left;
                hwdev_dirty_rect[j].dirty_h = ovl_dirty_rect[j].bottom - ovl_dirty_rect[j].top;
                logger_dirty.printf("[%d,%d,%d,%d]", hwdev_dirty_rect[j].dirty_x,
                                                     hwdev_dirty_rect[j].dirty_y,
                                                     hwdev_dirty_rect[j].dirty_w,
                                                     hwdev_dirty_rect[j].dirty_h);
            }
            input->dirty_roi_addr = hwdev_dirty_rect;
            input->dirty_roi_num = params[i]->ovl_dirty_rect_cnt;
        }

        if (!isConstantAlphaForRGBASupported())
        {
            if (params[i]->blending == HWC2_BLEND_MODE_PREMULTIPLIED)
            {
                input->sur_aen = 1;
                input->src_alpha = DISP_ALPHA_ONE;
                input->dst_alpha = DISP_ALPHA_SRC_INVERT;
            }
            else
            {
                input->sur_aen = 0;
            }
        }

        if (params[i]->secure)
            input->security = DISP_SECURE_BUFFER;
        else if (params[i]->protect)
            input->security = DISP_PROTECT_BUFFER;
        else
            input->security = DISP_NORMAL_BUFFER;

        input->compress = params[i]->compress;

        logger.printf("+%d,ion:%d,idx:%d,ext:%d/ ", i,params[i]->ion_fd, params[i]->fence_index, params[i]->ext_sel_layer);

        DbgLogger* ovl_logger = Debugger::getInstance().m_logger->ovlInput[dpy][i];
        ovl_logger->printf("(%d:%d) Layer+ ("
                 "mva=%p/sec=%d/prot=%d/"
                 "alpha=%d:0x%02x/blend=%04x/dim=%d/fmt=%d/range=%x(%x)/"
                 "x=%d y=%d w=%d h=%d s=%d,%d -> x=%d y=%d w=%d h=%d/"
                 "layer_type=%d s3d=%d ext_layer=%d ds=%d fbdc=%d)",
                 dpy, i, params[i]->mva, params[i]->secure, params[i]->protect,
                 params[i]->alpha_enable, params[i]->alpha, params[i]->blending,
                 params[i]->dim, input->src_fmt, input->yuv_range, params[i]->color_range,
                 params[i]->src_crop.left, params[i]->src_crop.top,
                 params[i]->src_crop.getWidth(), params[i]->src_crop.getHeight(), params[i]->pitch, input->src_v_pitch,
                 params[i]->dst_crop.left, params[i]->dst_crop.top,
                 params[i]->dst_crop.getWidth(), params[i]->dst_crop.getHeight(),
                 input->layer_type ,params[i]->s3d_type, params[i]->ext_sel_layer, params[i]->dataspace, input->compress);

        ovl_logger->tryFlush();

        HWC_ATRACE_BUFFER_INFO("set_input",
            dpy, i, params[i]->fence_index, params[i]->if_fence_index);
    }

    if (layer_enable_amount == 0)
    {
        HWC_LOGI("[DEV] updateOverlayInputs(): No enabling layer");
    }

    m_frame_cfg[dpy].input_layer_num = i;

    if (HWCMediator::getInstance().m_features.is_support_pq &&
        Platform::getInstance().m_config.support_color_transform)
    {
        if (color_transform != nullptr && color_transform->dirty)
        {
            m_frame_cfg[dpy].ccorr_config.is_dirty = true;
            m_frame_cfg[dpy].ccorr_config.mode = color_transform->hint;

            for (int32_t i = 0; i < COLOR_MATRIX_DIM * COLOR_MATRIX_DIM ; ++i)
                m_frame_cfg[dpy].ccorr_config.color_matrix[i] =
                    static_cast<int32_t>(color_transform->matrix[i / 4][i % 4] * 1024.f + 0.5f);

            if (m_frame_cfg[dpy].ccorr_config.mode == HAL_COLOR_TRANSFORM_IDENTITY)
            {
                HWC_LOGI("[DEV] (Send identity matrix)");
            }
            else
            {
                for (int32_t i = 0; i < COLOR_MATRIX_DIM; ++i)
                {
                    DbgLogger logger(DbgLogger::TYPE_HWC_LOG, 'I', "[DEV] ");
                    logger.printf("%4d %4d %4d %4d",
                        m_frame_cfg[dpy].ccorr_config.color_matrix[i * COLOR_MATRIX_DIM],
                        m_frame_cfg[dpy].ccorr_config.color_matrix[i * COLOR_MATRIX_DIM + 1],
                        m_frame_cfg[dpy].ccorr_config.color_matrix[i * COLOR_MATRIX_DIM + 2],
                        m_frame_cfg[dpy].ccorr_config.color_matrix[i * COLOR_MATRIX_DIM + 3]);
                }
            }
        }
    }
    else
    {
        m_frame_cfg[dpy].ccorr_config.is_dirty = false;
    }

    HWC_LOGV("- updateOverlayInputs");

    if (!m_caps_info.is_support_frame_cfg_ioctl)
        legacySetInputBuffer(dpy);
}

void DispDevice::prepareOverlayOutput(int dpy, OverlayPrepareParam* param)
{
    int session_id = m_frame_cfg[dpy].session_id;
    if (DISP_INVALID_SESSION == session_id)
    {
        HWC_LOGW("(%d) Failed to prepare invalid DispSession (output)", dpy);
        return;
    }

    disp_buffer_info buffer;
    memset(&buffer, 0, sizeof(disp_buffer_info));

    buffer.session_id = session_id;
    buffer.layer_id   = param->id;
    buffer.layer_en   = 1;
    buffer.ion_fd     = param->ion_fd;
    buffer.cache_sync = param->is_need_flush;
    buffer.index      = -1;
    buffer.fence_fd   = -1;

    int err = WDT_IOCTL(m_dev_fd, DISP_IOCTL_PREPARE_OUTPUT_BUFFER, &buffer);
    if (err < 0)
    {
        IOLOGE(dpy, err, "DISP_IOCTL_PREPARE_OUTPUT_BUFFER");
    }

    param->fence_index = buffer.index;
    param->fence_fd    = dupCloseAndSetFd(&buffer.fence_fd);

    param->if_fence_index = buffer.interface_index;
    param->if_fence_fd    = dupCloseAndSetFd(&buffer.interface_fence_fd);

    HWC_ATRACE_BUFFER_INFO("pre_output",
        dpy, param->id, param->fence_index, param->fence_fd);
}

void DispDevice::disableOverlayOutput(int dpy)
{
    m_frame_cfg[dpy].output_en = false;
    m_frame_cfg[dpy].output_cfg.src_fence_fd = -1;
}

void DispDevice::enableOverlayOutput(int dpy, OverlayPortParam* param)
{
    int session_id = m_frame_cfg[dpy].session_id;
    if (DISP_INVALID_SESSION == session_id)
    {
        HWC_LOGW("(%d) Failed to update invalid DispSession (output)", dpy);
        return;
    }

    disp_output_config* out_cfg = &m_frame_cfg[dpy].output_cfg;

    out_cfg->va     = param->va;
    out_cfg->pa     = param->mva;
    out_cfg->fmt    = mapDispOutFormat(param->format);
    out_cfg->x      = param->dst_crop.left;
    out_cfg->y      = param->dst_crop.top;
    out_cfg->width  = param->dst_crop.getWidth();
    out_cfg->height = param->dst_crop.getHeight();
    out_cfg->pitch  = param->pitch;
    out_cfg->src_fence_fd = param->fence;

    if (param->secure)
        out_cfg->security = DISP_SECURE_BUFFER;
    else
        out_cfg->security = DISP_NORMAL_BUFFER;

    out_cfg->buff_idx = param->fence_index;

    out_cfg->interface_idx = param->if_fence_index;

    out_cfg->frm_sequence  = param->sequence;

    HWC_LOGD("(%d) Output+ (ion=%d/idx=%d/if_idx=%d/sec=%d/fmt=%d"
             "/x=%d y=%d w=%d h=%d s=%d ds=%d)",
             dpy, param->ion_fd, param->fence_index, param->if_fence_index, param->secure,
             out_cfg->fmt, param->dst_crop.left, param->dst_crop.top,
             param->dst_crop.getWidth(), param->dst_crop.getHeight(),
             param->pitch, param->dataspace);

    m_frame_cfg[dpy].output_en = true;

    if (!m_caps_info.is_support_frame_cfg_ioctl)
        legacySetOutputBuffer(dpy);
}

void DispDevice::prepareOverlayPresentFence(int dpy, OverlayPrepareParam* param)
{
    int session_id = m_frame_cfg[dpy].session_id;
    if (DISP_INVALID_SESSION == session_id)
    {
        HWC_LOGW("(%d) Failed to preapre invalid DispSession (present)", dpy);
        return;
    }

    {
        disp_present_fence fence;

        fence.session_id = session_id;

        int err = WDT_IOCTL(m_dev_fd, DISP_IOCTL_GET_PRESENT_FENCE, &fence);
        if (err < 0)
        {
            IOLOGE(dpy, err, "DISP_IOCTL_GET_PRESENT_FENCE");
        }

        param->fence_index = fence.present_fence_index;
        param->fence_fd    = dupCloseAndSetFd(&fence.present_fence_fd);
        HWC_LOGV("(%d) DispDevice::prepareOverlayPresentFence() idx:%d fence fd:%d",
            dpy, param->fence_index, param->fence_fd);
    }
}

status_t DispDevice::waitVSync(int dpy, nsecs_t *ts)
{
    int session_id = m_frame_cfg[dpy].session_id;
    if (DISP_INVALID_SESSION == session_id)
    {
        HWC_LOGW("Failed to wait vsync for invalid DispSession (dpy=%d)", dpy);
        return BAD_VALUE;
    }

    disp_session_vsync_config config;
    memset(&config, 0, sizeof(config));

    config.session_id = session_id;

    int err = WDT_IOCTL(m_dev_fd, DISP_IOCTL_WAIT_FOR_VSYNC, &config);
    if (err < 0)
    {
        IOLOGE(dpy, err, "DISP_IOCTL_WAIT_FOR_VSYNC");
        return BAD_VALUE;
    }

    *ts = config.vsync_ts;

    return NO_ERROR;
}

void DispDevice::setPowerMode(int dpy,int mode)
{
    if (HWCMediator::getInstance().m_features.control_fb)
    {
        HWC_LOGD("DispDevice::setPowerMode() dpy:%d mode:%d", dpy, mode);
#ifdef USE_SWWATCHDOG
        AUTO_WDT(1000);
#endif
        char filename[32] = {0};
        snprintf(filename, sizeof(filename), "/dev/graphics/fb%d", dpy);
        int fb_fd = open(filename, O_RDWR);
        if (fb_fd <= 0)
        {
            HWC_LOGE("Failed to open fb%d device: %s", dpy, strerror(errno));
            return;
        }

        int err = NO_ERROR;
        switch (mode)
        {
            case HWC_POWER_MODE_OFF:
            {
                err = WDT_IOCTL(fb_fd, FBIOBLANK, FB_BLANK_POWERDOWN);
#ifdef USE_SWWATCHDOG
                if (dpy == HWC_DISPLAY_PRIMARY)
                    SWWatchDog::suspend();
#endif
                break;
            }
            case HWC_POWER_MODE_NORMAL:
            {
#ifdef USE_SWWATCHDOG
                if (dpy == HWC_DISPLAY_PRIMARY)
                    SWWatchDog::resume();
#endif
                err = WDT_IOCTL(fb_fd, FBIOBLANK, FB_BLANK_UNBLANK);
                m_color_transform_info[dpy].resend_color_transform = true;
                break;
            }
            case HWC_POWER_MODE_DOZE:
            {
                if (HWCMediator::getInstance().m_features.aod)
                {
                    err = WDT_IOCTL(fb_fd, MTKFB_SET_AOD_POWER_MODE, MTKFB_AOD_DOZE);
                }
                else
                {
                    err = WDT_IOCTL(fb_fd, FBIOBLANK, FB_BLANK_UNBLANK);
                    HWC_LOGE("setPowerMode: receive HWC_POWER_MODE_DOZE without aod enabled");
                }
                m_color_transform_info[dpy].resend_color_transform = true;
                break;
            }
            case HWC_POWER_MODE_DOZE_SUSPEND:
            {
                if (HWCMediator::getInstance().m_features.aod)
                {
                    err = WDT_IOCTL(fb_fd, MTKFB_SET_AOD_POWER_MODE, MTKFB_AOD_DOZE_SUSPEND);
                }
                else
                {
                    err = WDT_IOCTL(fb_fd, FBIOBLANK, FB_BLANK_UNBLANK);
                    HWC_LOGE("setPowerMode: receive HWC_POWER_MODE_DOZE_SUSPEND without aod enabled ");
                }
                break;
            }

            default:
                HWC_LOGE("setPowerMode: receive unknown power mode: %d", mode);
        }

        if (err != NO_ERROR) {
            HWC_LOGE("Failed to set power(%d) to fb%d device: %s", mode, dpy, strerror(errno));
        }

        protectedClose(fb_fd);
    }
}

unsigned int DispDevice::getDeviceId(int dpy)
{
    unsigned int device_id = dpy;
    if (dpy == HWC_DISPLAY_EXTERNAL)
    {
        device_id = g_ext_device_id[HWCMediator::getInstance().m_features.dual_display];
    }
    return device_id;
}

status_t DispDevice::waitAllJobDone(const int dpy)
{
    if (isFenceWaitSupported())
    {
        if (static_cast<int32_t>(m_frame_cfg[dpy].session_id) == DISP_INVALID_SESSION)
        {
            HWC_LOGW("try to wait for an invalid display session");
            return INVALID_OPERATION;
        }

        int err = WDT_IOCTL(m_dev_fd, DISP_IOCTL_WAIT_ALL_JOBS_DONE, m_frame_cfg[dpy].session_id);
        if (err < 0)
        {
            IOLOGE(dpy, err, "DISP_IOCTL_WAIT_ALL_JOBS_DONE");
            return err;
        }
    }

    return NO_ERROR;
}

status_t DispDevice::waitRefreshRequest(unsigned int* type)
{
    // NOTE: don't use WDT_IOCTL since this DISP_IOCTL_WAIT_DISP_SELF_REFRESH will
    // blocked by driver until self refresh is needed
    DISP_SELF_REFRESH_TYPE temp = WAIT_FOR_REFRESH;
    int err = ioctl(m_dev_fd, DISP_IOCTL_WAIT_DISP_SELF_REFRESH, &temp);
    if (err < 0)
    {
        IOLOGE(HWC_DISPLAY_PRIMARY, err, "DISP_IOCTL_WAIT_DISP_SELF_REFRESH");
        return BAD_VALUE;
    }
    *type = mapDispSelfRefreshType2Hwc(temp);

    return NO_ERROR;
}

void DispDevice::setLastValidColorTransform(const int32_t& dpy, sp<ColorTransform> color_transform)
{
    m_color_transform_info[dpy].last_valid_color_transform = color_transform;
}

int32_t DispDevice::getWidth(int dpy, hwc2_config_t config)
{
    return m_multi_cfgs[dpy].dyn_cfgs[config].width;
}

int32_t DispDevice::getHeight(int dpy, hwc2_config_t config)
{
    return m_multi_cfgs[dpy].dyn_cfgs[config].height;
}

int32_t DispDevice::getRefresh(int dpy, hwc2_config_t config)
{
    // display Driver will multiply 100 for float support
    return m_multi_cfgs[dpy].dyn_cfgs[config].vsyncFPS / 100;
}

uint32_t DispDevice::getNumConfigs(int dpy)
{
    if (!isMultiConfigsSupport())
    {
        return 1;
    }
    return m_multi_cfgs[dpy].config_num;
}
