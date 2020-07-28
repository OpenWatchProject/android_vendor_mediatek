#define DEBUG_LOG_TAG "TOL"

#include <dlfcn.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <linux/ion_drv.h>
#include <ion/ion.h>
#include "ion.h"

#include <cutils/properties.h>

#include <utils/tools.h>

#include <ui/GraphicBufferMapper.h>

#include "grallocdev.h"

#include "transform.h"
#ifdef USE_HWC2
#include "hwc2.h"
#include <sync/sync.h>
#endif // USE_HWC2

#include "DpAsyncBlitStream.h"
#include "overlay.h"
#include "dispatcher.h"

#define UI_PQ_VIDEO_ID 0xFFFFFF00

#define LOG_MAX_SIZE 512
//#define FENCE_DEBUG

DP_COLOR_ENUM mapDpFormat(const uint32_t& fmt)
{
    switch (fmt)
    {
        case HAL_PIXEL_FORMAT_RGBA_8888:
            return DP_COLOR_RGBA8888;

        case HAL_PIXEL_FORMAT_RGBX_8888:
            return DP_COLOR_RGBA8888;

        case HAL_PIXEL_FORMAT_BGRA_8888:
            return DP_COLOR_BGRA8888;

        case HAL_PIXEL_FORMAT_BGRX_8888:
        case HAL_PIXEL_FORMAT_IMG1_BGRX_8888:
            return DP_COLOR_BGRA8888;

        case HAL_PIXEL_FORMAT_RGB_888:
            return DP_COLOR_RGB888;

        case HAL_PIXEL_FORMAT_RGB_565:
            return DP_COLOR_RGB565;

        case HAL_PIXEL_FORMAT_NV12_BLK_FCM:
            return DP_COLOR_420_BLKI;

        case HAL_PIXEL_FORMAT_NV12_BLK:
            return DP_COLOR_420_BLKP;

        case HAL_PIXEL_FORMAT_I420:
            return DP_COLOR_I420;

        case HAL_PIXEL_FORMAT_I420_DI:
            return DP_COLOR_I420;

        case HAL_PIXEL_FORMAT_YV12:
            return DP_COLOR_YV12;

        case HAL_PIXEL_FORMAT_YV12_DI:
            return DP_COLOR_YV12;

        case HAL_PIXEL_FORMAT_YUYV:
        case HAL_PIXEL_FORMAT_YCbCr_422_I:
            return DP_COLOR_YUYV;

        case HAL_PIXEL_FORMAT_UFO:
            return DP_COLOR_420_BLKP_UFO;

        case HAL_PIXEL_FORMAT_UFO_AUO:
            return DP_COLOR_420_BLKP_UFO_AUO;

        case HAL_PIXEL_FORMAT_NV12:
            return DP_COLOR_NV12;

        case HAL_PIXEL_FORMAT_YCRCB_420_SP:
            return DP_COLOR_NV21;

        case HAL_PIXEL_FORMAT_NV12_BLK_10BIT_H:
            return DP_COLOR_420_BLKP_10_H;

        case HAL_PIXEL_FORMAT_NV12_BLK_10BIT_H_JUMP:
            return DP_COLOR_420_BLKP_10_H_JUMP;

        case HAL_PIXEL_FORMAT_NV12_BLK_10BIT_V:
            return DP_COLOR_420_BLKP_10_V;

        case HAL_PIXEL_FORMAT_NV12_BLK_10BIT_V_JUMP:
            return DP_COLOR_420_BLKP_10_V_JUMP;

        case HAL_PIXEL_FORMAT_UFO_10BIT_H:
            return DP_COLOR_420_BLKP_UFO_10_H;

        case HAL_PIXEL_FORMAT_UFO_10BIT_H_JUMP:
            return DP_COLOR_420_BLKP_UFO_10_H_JUMP;

        case HAL_PIXEL_FORMAT_UFO_10BIT_V:
            return DP_COLOR_420_BLKP_UFO_10_V;

        case HAL_PIXEL_FORMAT_UFO_10BIT_V_JUMP:
            return DP_COLOR_420_BLKP_UFO_10_V_JUMP;

        default:
            HWC_LOGE("Color format for DP is invalid (0x%x)", fmt);
            return DP_COLOR_UNKNOWN;
    }
    return DP_COLOR_UNKNOWN;
}

void calculateCrop(Rect* src_crop, Rect* dst_crop, Rect& dst_buf, uint32_t xform)
{
    Rect src_base(*src_crop);
    Rect dst_base(*dst_crop);

    dst_base.intersect(dst_buf, dst_crop);

    if (dst_base == *dst_crop)
    {
        // no crop happened, skip
        *src_crop = src_base;
    }
    else
    {
        // check inverse transform
        Rect in_base(dst_base);
        Rect in_crop(*dst_crop);
        if (Transform::ROT_0 != xform && in_base != in_crop)
        {
            if (Transform::ROT_90 & xform)
                xform ^= (Transform::FLIP_H | Transform::FLIP_V);

            Transform tr(xform);
            in_base = tr.transform(in_base);
            in_crop = tr.transform(in_crop);
        }

        // map dst crop to src crop

        // calculate rectangle ratio between two rectangles
        // horizontally and vertically
        const float ratio_h = src_base.getWidth() /
            static_cast<float>(in_base.getWidth());
        const float ratio_v = src_base.getHeight() /
            static_cast<float>(in_base.getHeight());

        // get result of the corresponding crop rectangle
        // add 0.5f to round the result to the nearest whole number
        src_crop->left = src_base.left + 0.5f +
            (in_crop.left - in_base.left) * ratio_h;
        src_crop->top  = src_base.top + 0.5f +
            (in_crop.top - in_base.top) * ratio_v;
        src_crop->right = src_base.left + 0.5f +
            (in_crop.right - in_base.left) * ratio_h;
        src_crop->bottom = src_base.top + 0.5f +
            (in_crop.bottom - in_base.top) * ratio_v;
    }
}

void dupBufferHandle(buffer_handle_t input, buffer_handle_t* output)
{
    const int size = (input->numFds + input->numInts) * sizeof(int);

    native_handle_t* dup_input = static_cast<native_handle_t*>(malloc(sizeof(native_handle_t) + size));

    if (nullptr == dup_input)
    {
        ALOGE("To malloc() dup_input(%p) failed", dup_input);
        return;
    }

    memcpy(dup_input, input, sizeof(native_handle_t) + size);

    for (int i = 0; i < input->numFds; ++i)
    {
        dup_input->data[i] = ::dup(input->data[i]);
    }

    *output = dup_input;
}

void freeDuppedBufferHandle(buffer_handle_t handle)
{
    if(handle == NULL)
        return;

    for (int i = 0; i < handle->numFds; ++i)
    {
        ::protectedClose(handle->data[i]);
    }

    free(const_cast<native_handle_t*>(handle));
}

LayerInfo::LayerInfo(const hwc_layer_1* const layer)
    : layer_idx(-1)
    , type(-1)
    , bpp(-1)
    , dirty(-1)
    , src_crop(0, 0, 0, 0)
    , dst_crop(0, 0, 0, 0)
    , mdp_dst_roi(0, 0, 0, 0)
{
    const int src_crop_x = getSrcLeft(layer);
    const int src_crop_y = getSrcTop(layer);
    const int src_crop_w = getSrcWidth(layer);
    const int src_crop_h = getSrcHeight(layer);
    const int dst_crop_x = layer->displayFrame.left;
    const int dst_crop_y = layer->displayFrame.top;
    const int dst_crop_w = WIDTH(layer->displayFrame);
    const int dst_crop_h = HEIGHT(layer->displayFrame);

    src_crop.left = src_crop_x;
    src_crop.top = src_crop_y;
    src_crop.right = src_crop_x + src_crop_w;
    src_crop.bottom = src_crop_y + src_crop_h;

    dst_crop.left = dst_crop_x;
    dst_crop.top = dst_crop_y;
    dst_crop.right = dst_crop_x + dst_crop_w;
    dst_crop.bottom = dst_crop_y + dst_crop_h;

    if ((dst_crop.left > 4096 || dst_crop.left < -4096)||
        (dst_crop.top > 4096 || dst_crop.top < -4096)||
        (dst_crop.right > 4096 || dst_crop.right < -4096)||
        (dst_crop.bottom > 4096 || dst_crop.bottom < -4096))
    {
        HWC_LOGW("Rect is wrong (%d,%d,%d,%d) (w:%d,h:%d)",
                dst_crop.left,
                dst_crop.top,
                dst_crop.right,
                dst_crop.bottom,
                dst_crop_w,
                dst_crop_h);
    }
}

LayerList::LayerList()
{}

LayerList::~LayerList()
{}

Rect LayerList::getBounds()
{
    return getBounds(0, size()-1);
}

Rect LayerList::getBounds(const int head, const int tail)
{
    Region region;
    for (int i = head; i <= tail; ++i)
    {
        sp<LayerInfo> info = itemAt(i);
        if ((info->dst_crop.left > 4096 || info->dst_crop.left < -4096)||
            (info->dst_crop.top > 4096 || info->dst_crop.top < -4096)||
            (info->dst_crop.right > 4096 || info->dst_crop.right < -4096)||
            (info->dst_crop.bottom > 4096 || info->dst_crop.bottom < -4096))
        {
            HWC_LOGW("Rect is wrong index(i) (%d,%d,%d,%d)",
                    info->dst_crop.left,
                    info->dst_crop.top,
                    info->dst_crop.right,
                    info->dst_crop.bottom);
            return Rect();
        }
        region = region.orSelf(info->dst_crop);
    }
    return region.getBounds();
}

// ---------------------------------------------------------------------------
ANDROID_SINGLETON_STATIC_INSTANCE(BlackBuffer);

BlackBuffer::BlackBuffer()
    : m_handle(0)
{
    GrallocDevice::AllocParam param;
    param.width  = 128;
    param.height = 128;
    param.format = HAL_PIXEL_FORMAT_RGB_565;
    param.usage  = GRALLOC_USAGE_SW_WRITE_RARELY;

    // allocate
    if (NO_ERROR != GrallocDevice::getInstance().alloc(param))
    {
        HWC_LOGW("fill black buffer by bliter - allocate buf fail");
        return;
    }
    m_handle = param.handle;

    // make this buffer black
    int size;
    int32_t err = GRALLOC_EXTRA_OK;
    int32_t ion_fd = -1;
    err = gralloc_extra_query(m_handle, GRALLOC_EXTRA_GET_ALLOC_SIZE, &size);
    if (err != GRALLOC_EXTRA_OK)
    {
        HWC_LOGE("%s Failed to get alloc size, err(%x), (handle=%p)", __func__, err, m_handle);
        return ;
    }

    err = gralloc_extra_query(m_handle, GRALLOC_EXTRA_GET_ION_FD, &ion_fd);
    if (err != GRALLOC_EXTRA_OK)
    {
        HWC_LOGE("%s Failed to get ion fd, err(%x), (handle=%p)", __func__, err, m_handle);
        return;
    }

    static const int32_t dev_fd = ::open("/dev/ion", O_RDONLY);
    ion_user_handle_t ion_hnd = -1;
    if (ion_import(dev_fd, ion_fd, &ion_hnd))
    {
        HWC_LOGE("ion_import is failed: %s, ion_fd(%d)", strerror(errno), ion_fd);
        ion_hnd = -1;
    }

    int32_t shared_fd = -1;
    if (ion_hnd != -1 && ion_share(dev_fd, ion_hnd, &shared_fd))
    {
        HWC_LOGE("ion_share is failed: %s, ion_hnd(%p) ", strerror(errno), ion_hnd);
        shared_fd = -1;
    }

    void *ptr = nullptr;
    if (ion_fd != -1 && shared_fd != -1)
        ptr = ion_mmap(ion_fd, nullptr, size, PROT_READ | PROT_WRITE, MAP_SHARED, shared_fd, 0);

    if (ptr == nullptr || ptr == (void*)(-1))
        HWC_LOGE("ion mmap fail");

    if (ptr)
        memset(ptr, 0, size);

    if (ptr)
    {
        ion_munmap(ion_hnd, ptr, size);
        ptr = nullptr;
    }

    if (ion_hnd != -1 && ion_free(dev_fd, ion_hnd))
    {
        HWC_LOGE("ion_free is failed: %s, ion_hnd(%p) ", strerror(errno), ion_hnd);
        ion_hnd = -1;
    }
    if (shared_fd != -1 && ion_share_close(dev_fd, shared_fd))
    {
        HWC_LOGW("ion_share_close is failed: %s , share_fd(%d)", strerror(errno), shared_fd);
        shared_fd = -1;
    }
}

BlackBuffer::~BlackBuffer()
{
    if (m_handle != 0)
        GrallocDevice::getInstance().free(m_handle);
}

buffer_handle_t BlackBuffer::getHandle()
{
    return m_handle;
}

void BlackBuffer::setSecure()
{
    if (1 == HWCMediator::getInstance().m_features.svp)
    {
        // attach a zero-initialized secure buffer to the original buffer
        SVPLOGD("BlackBuf setSecure", "(h:%x)", m_handle);
        unsigned int sec_handle;
        GRALLOC_EXTRA_SECURE_BUFFER_TYPE option = GRALLOC_EXTRA_SECURE_BUFFER_TYPE_ZERO;
        getSecureHwcBuf(m_handle, &sec_handle, &option);
        setSecExtraSfStatus(true, m_handle);
    }
}

void BlackBuffer::setNormal()
{
    if (1 == HWCMediator::getInstance().m_features.svp)
    {
        SVPLOGD("BlackBuf setNormal", "(h:%x)", m_handle);
        freeSecureHwcBuf(m_handle);
        setSecExtraSfStatus(false, m_handle);
    }
}

// ---------------------------------------------------------------------------
ANDROID_SINGLETON_STATIC_INSTANCE(WhiteBuffer);

WhiteBuffer::WhiteBuffer()
    : m_handle(0)
{
    GrallocDevice::AllocParam param;
    param.width  = 128;
    param.height = 128;
    param.format = HAL_PIXEL_FORMAT_RGB_565;
    param.usage  = GRALLOC_USAGE_SW_WRITE_RARELY;

    // allocate
    if (NO_ERROR != GrallocDevice::getInstance().alloc(param))
    {
        HWC_LOGW("fill black buffer by bliter - allocate buf fail");
        return;
    }
    m_handle = param.handle;

    // make this buffer black
    int size;
    int32_t err = GRALLOC_EXTRA_OK;
    int32_t ion_fd = -1;
    err = gralloc_extra_query(m_handle, GRALLOC_EXTRA_GET_ALLOC_SIZE, &size);
    if (err != GRALLOC_EXTRA_OK)
    {
        HWC_LOGE("%s Failed to get alloc size, err(%x), (handle=%p)", __func__, err, m_handle);
        return ;
    }

    err = gralloc_extra_query(m_handle, GRALLOC_EXTRA_GET_ION_FD, &ion_fd);
    if (err != GRALLOC_EXTRA_OK)
    {
        HWC_LOGE("%s Failed to get ion fd, err(%x), (handle=%p)", __func__, err, m_handle);
        return;
    }

    static const int32_t dev_fd = ::open("/dev/ion", O_RDONLY);
    ion_user_handle_t ion_hnd = -1;
    if (ion_import(dev_fd, ion_fd, &ion_hnd))
    {
        HWC_LOGE("ion_import is failed: %s, ion_fd(%d)", strerror(errno), ion_fd);
        ion_hnd = -1;
    }

    int32_t shared_fd = -1;
    if (ion_hnd != -1 && ion_share(dev_fd, ion_hnd, &shared_fd))
    {
        HWC_LOGE("ion_share is failed: %s, ion_hnd(%p) ", strerror(errno), ion_hnd);
        shared_fd = -1;
    }

    void *ptr = nullptr;
    if (ion_fd != -1 && shared_fd != -1)
        ptr = ion_mmap(ion_fd, nullptr, size, PROT_READ | PROT_WRITE, MAP_SHARED, shared_fd, 0);

    if (ptr == nullptr || ptr == (void*)(-1))
        HWC_LOGE("ion mmap fail");

    if (ptr)
        memset(ptr, 255, size);

    if (ptr)
    {
        ion_munmap(ion_hnd, ptr, size);
        ptr = nullptr;
    }

    if (ion_hnd != -1 && ion_free(dev_fd, ion_hnd))
    {
        HWC_LOGE("ion_free is failed: %s, ion_hnd(%p) ", strerror(errno), ion_hnd);
        ion_hnd = -1;
    }
    if (shared_fd != -1 && ion_share_close(dev_fd, shared_fd))
    {
        HWC_LOGW("ion_share_close is failed: %s , share_fd(%d)", strerror(errno), shared_fd);
        shared_fd = -1;
    }
}

WhiteBuffer::~WhiteBuffer()
{
    if (m_handle != 0)
        GrallocDevice::getInstance().free(m_handle);
}

buffer_handle_t WhiteBuffer::getHandle()
{
    return m_handle;
}

void WhiteBuffer::setSecure()
{
    if (1 == HWCMediator::getInstance().m_features.svp)
    {
        // attach a zero-initialized secure buffer to the original buffer
        SVPLOGD("BlackBuf setSecure", "(h:%x)", m_handle);
        unsigned int sec_handle;
        GRALLOC_EXTRA_SECURE_BUFFER_TYPE option = GRALLOC_EXTRA_SECURE_BUFFER_TYPE_ZERO;
        getSecureHwcBuf(m_handle, &sec_handle, &option);
        setSecExtraSfStatus(true, m_handle);
    }
}

void WhiteBuffer::setNormal()
{
    if (1 == HWCMediator::getInstance().m_features.svp)
    {
        SVPLOGD("BlackBuf setNormal", "(h:%x)", m_handle);
        freeSecureHwcBuf(m_handle);
        setSecExtraSfStatus(false, m_handle);
    }
}
// ---------------------------------------------------------------------------
ANDROID_SINGLETON_STATIC_INSTANCE(IONDevice);

IONDevice::IONDevice()
{
    m_dev_fd = open("/dev/ion", O_RDONLY);
    if (m_dev_fd <= 0)
    {
        HWC_LOGE("Failed to open ION device: %s ", strerror(errno));
    }
}

IONDevice::~IONDevice()
{
    if (m_dev_fd > 0) close(m_dev_fd);
}

int IONDevice::getDeviceFd()
{
    return m_dev_fd;
}

int IONDevice::ionImport(int* ion_fd)
{
    int dev_fd = m_dev_fd;
    if (dev_fd <= 0) return -1;

    ion_user_handle_t ion_hnd;
    if (ion_import(dev_fd, *ion_fd, &ion_hnd))
    {
        HWC_LOGE("ion_import is failed: %s, ion_fd(%d)", strerror(errno), *ion_fd);
        return -1;
    }

    int share_fd;
    if (ion_share(dev_fd, ion_hnd, &share_fd))
    {
        HWC_LOGE("ion_share is failed: %s, ion_hnd(%p) ", strerror(errno), ion_hnd);
        return -1;
    }

    if (ion_free(dev_fd, ion_hnd))
    {
        HWC_LOGE("ion_free is failed: %s, ion_hnd(%p) ", strerror(errno), ion_hnd);
        return -1;
    }

    HWC_LOGD("[mm_ionImport] ion_fd(%d) -> share_fd(%d)", *ion_fd, share_fd);

    *ion_fd = share_fd;

    return 0;
}

int IONDevice::ionImport(const int32_t& ion_fd, int32_t* new_ion_fd, const char* dbg_name)
{
    if (m_dev_fd <= 0)
    {
        HWC_LOGE("ion_import is failed because dev_fd is not initialized");
        return -1;
    }

    ion_user_handle_t ion_hnd;
    if (ion_import(m_dev_fd, ion_fd, &ion_hnd))
    {
        HWC_LOGE("ion_import is failed: %s, ion_fd(%d)", strerror(errno), ion_fd);
        return -1;
    }

    if (ion_share(m_dev_fd, ion_hnd, new_ion_fd))
    {
        HWC_LOGE("ion_share is failed: %s, ion_hnd(%p) ", strerror(errno), ion_hnd);
        return -1;
    }

    if (ion_free(m_dev_fd, ion_hnd))
    {
        HWC_LOGE("ion_free is failed: %s, ion_hnd(%p) ", strerror(errno), ion_hnd);
        return -1;
    }

    if (dbg_name != nullptr)
    {
        HWC_LOGD("[mm_ionImport] ion_fd(%d) -> new_ion_fd(%d) when %s", ion_fd, *new_ion_fd, dbg_name);
    }
    else
    {
        HWC_LOGD("[mm_ionImport] ion_fd(%d) -> new_ion_fd(%d)", ion_fd, *new_ion_fd);
    }

    return 0;
}

int IONDevice::ionClose(int share_fd)
{
    if (m_dev_fd <= 0) return -1;

    if (share_fd <= 0)
    {
        HWC_LOGW("[mm_ionClose] Invalid Fd (%d)!", share_fd);
        abort();
        return -1;
    }

    if (ion_share_close(m_dev_fd, share_fd))
    {
        HWC_LOGW("ion_share_close is failed: %s , share_fd(%d)", strerror(errno), share_fd);
        return -1;
    }

    HWC_LOGD("[mm_ionClose] share_fd(%d)", share_fd);

    return 0;
}

int IONDevice::ionCloseAndSet(int* share_fd, const int& value)
{
    int result = ionClose(*share_fd);
    if (result == 0)
    {
        *share_fd = value;
    }
    return result;
}

void setSecExtraSfStatus(
    bool is_secure, buffer_handle_t hand, gralloc_extra_ion_sf_info_t* rt_ext_info)
{
    if (0 == HWCMediator::getInstance().m_features.svp)
        return;

    // rt_ext_info != NULL means that we have to return the ext_info
    // so use *rt_ext_info for getting ext_info when rt_ext_info != NULL
    // otherwise use tmp_ext_info
    gralloc_extra_ion_sf_info_t tmp_ext_info;
    gralloc_extra_ion_sf_info_t* ext_info = (NULL == rt_ext_info) ? &tmp_ext_info : rt_ext_info;
    int status = is_secure ? GRALLOC_EXTRA_BIT_SECURE : GRALLOC_EXTRA_BIT_NORMAL;

    // query  ->  set extra sf info  ->  perform
    gralloc_extra_query(hand, GRALLOC_EXTRA_GET_IOCTL_ION_SF_INFO, ext_info);
    gralloc_extra_sf_set_status(ext_info, GRALLOC_EXTRA_MASK_SECURE, status);
    gralloc_extra_perform(hand, GRALLOC_EXTRA_SET_IOCTL_ION_SF_INFO, ext_info);
}

ANDROID_SINGLETON_STATIC_INSTANCE(AbortMessager);

AbortMessager::AbortMessager()
{
    m_begin = 0;
    for(int i = 0; i < MAX_ABORT_MSG; ++i)
        m_msg_arr[i].clear();
}

AbortMessager::~AbortMessager()
{
}

void AbortMessager::printf(const char* msg, ...)
{
    Mutex::Autolock l(m_lock);

    char buf[LOG_MAX_SIZE] = {'\0'};

    if (msg != nullptr)
    {
        va_list ap;
        va_start(ap, msg);
        vsnprintf(buf, LOG_MAX_SIZE, msg, ap);
        va_end(ap);
    }

    struct timeval tv;
    struct timezone tz;
    int hh, mm, ss;
    gettimeofday(&tv, &tz);
    hh = (tv.tv_sec / 3600) % 24;
    mm = (tv.tv_sec % 3600) / 60;
    ss = tv.tv_sec % 60;

    m_msg_arr[m_begin].clear();
    m_msg_arr[m_begin].appendFormat("%02d:%02d:%02d.%06ld tid:%d %s", hh, mm, ss, tv.tv_usec, ::gettid(), buf);
    m_begin = (m_begin + 1) % MAX_ABORT_MSG;
}

void AbortMessager::flushOut()
{
    Mutex::Autolock l(m_lock);
    for (int i = 0; i < MAX_ABORT_MSG; ++i)
    {
        int index = (m_begin + i) % MAX_ABORT_MSG;
        if (m_msg_arr[index].size())
            HWC_LOGD("[%d] %s", i, m_msg_arr[index].c_str());
    }
}

void AbortMessager::abort()
{
    flushOut();
    ::abort();
}

#ifdef USE_HWC2

const char* HWC2_COMPOSITION_INVALID_STR = "INV";
const char* HWC2_COMPOSITION_CLIENT_STR = "CLI";
const char* HWC2_COMPOSITION_DEVICE_STR = "DEV";
const char* HWC2_COMPOSITION_CURSOR_STR = "CUR";
const char* HWC2_COMPOSITION_SOLID_COLOR_STR = "SOL";
const char* HWC2_COMPOSITION_UNKNOWN_STR = "UNK";

const char* getCompString(const int32_t& comp_type)
{
    switch(comp_type)
    {
        case HWC2_COMPOSITION_INVALID:
            return HWC2_COMPOSITION_INVALID_STR;

        case HWC2_COMPOSITION_CLIENT:
            return HWC2_COMPOSITION_CLIENT_STR;

        case HWC2_COMPOSITION_DEVICE:
            return HWC2_COMPOSITION_DEVICE_STR;

        case HWC2_COMPOSITION_CURSOR:
            return HWC2_COMPOSITION_CURSOR_STR;

        case HWC2_COMPOSITION_SOLID_COLOR:
            return HWC2_COMPOSITION_SOLID_COLOR_STR;

        default:
            HWC_LOGE("%s unknown composition type:%d", __func__, comp_type);
            return HWC2_COMPOSITION_UNKNOWN_STR;
    }
}

const char* HWC2_BLEND_MODE_INVALID_STR = "INV";
const char* HWC2_BLEND_MODE_NONE_STR = "NON";
const char* HWC2_BLEND_MODE_PREMULTIPLIED_STR = "PRE";
const char* HWC2_BLEND_MODE_COVERAGE_STR = "COV";
const char* HWC2_BLEND_MODE_UNKNOWN_STR = "UNK";

const char* getBlendString(const int32_t& blend)
{
    switch(blend)
    {
        case HWC2_BLEND_MODE_INVALID:
            return HWC2_BLEND_MODE_INVALID_STR;

        case HWC2_BLEND_MODE_NONE:
            return HWC2_BLEND_MODE_NONE_STR;

        case HWC2_BLEND_MODE_PREMULTIPLIED:
            return HWC2_BLEND_MODE_PREMULTIPLIED_STR;

        case HWC2_BLEND_MODE_COVERAGE:
            return HWC2_BLEND_MODE_COVERAGE_STR;

        default:
            HWC_LOGE("%s unknown blend:%d", __func__, blend);
            return HWC2_BLEND_MODE_UNKNOWN_STR;
    }
}

void protectedCloseImpl(const int32_t& fd, const char* /*str*/, const int32_t& /*line*/)
{
    if (fd >= 0 && fd < 3)
    {
        ALOGE("abort! close fd %d", fd);
        abort();
    }
#ifdef FENCE_DEBUG
    android::CallStack stack;
    stack.update();
    HWC_LOGW("close fd %d backtrace: %s", fd, stack.toString().string());

    HWC_LOGW("close fd %d(%s)", fd, str3);
#endif
    if (fd >= 0)
    {
        ::close(fd);
    }
    else if (fd < -1)
    {
        HWC_LOGW("A unknown fd(-1) to be freed");
    }
}

int protectedDupImpl(const int32_t& fd, const char* /*str*/, const int32_t& /*line*/)
{
    int dupFd = -1;
    if (fd >= 0 && fd < 3)
    {
        ALOGE("abort! dup fd %d", fd);
        abort();
    }
    dupFd = dup(fd);

    if (dupFd >= 0 && dupFd < 3)
    {
        ALOGE("abort! dupped fd %d", dupFd);
        abort();
    }

    return dupFd;
}


int getSrcLeft(const sp<HWCLayer>& layer)
{
    return (int)(ceilf(layer->getSourceCrop().left));
}

int getSrcTop(const sp<HWCLayer>& layer)
{
    return (int)(ceilf(layer->getSourceCrop().top));
}

int getSrcWidth(const sp<HWCLayer>& layer)
{
    const int left = (int)(ceilf(layer->getSourceCrop().left));
    const int right = (int)(floorf(layer->getSourceCrop().right));
    return (right - left);
}

int getSrcHeight(const sp<HWCLayer>& layer)
{
    const int top = (int)(ceilf(layer->getSourceCrop().top));
    const int bottom = (int)(floorf(layer->getSourceCrop().bottom));
    return (bottom - top);
}

int getDstTop(const sp<HWCLayer>& layer)
{
    return (int)(ceilf(layer->getDisplayFrame().top));
}

int getDstBottom(const sp<HWCLayer>& layer)
{
    return (int)(ceilf(layer->getDisplayFrame().bottom));
}

int getDstLeft(const sp<HWCLayer>& layer)
{
    return (int)(ceilf(layer->getDisplayFrame().left));
}

int getDstRight(const sp<HWCLayer>& layer)
{
    return (int)(ceilf(layer->getDisplayFrame().right));
}

int getDstWidth(const sp<HWCLayer>& layer)
{
    return WIDTH(layer->getDisplayFrame());
}

int getDstHeight(const sp<HWCLayer>& layer)
{
    return HEIGHT(layer->getDisplayFrame());
}

void copyHWCLayerIntoHwcLayer_1(const sp<HWCLayer>& from, hwc_layer_1_t* to)
{
    to->handle = from->getHandle();
    to->transform = from->getTransform();
    to->compositionType = from->getCompositionType();
    to->displayFrame = from->getDisplayFrame();
    to->visibleRegionScreen = from->getVisibleRegion();
    to->blending = from->getBlend();
    //todo:joen sourcecdrop f ? sourcecropi?
    to->sourceCropf = from->getSourceCrop();
    to->acquireFenceFd = from->getAcquireFenceFd();
    from->setAcquireFenceFd(-1, true);
    to->releaseFenceFd = from->getReleaseFenceFd();
    to->planeAlpha = static_cast<uint8_t>(from->getPlaneAlpha() * 0xff);
    to->surfaceDamage = from->getDamage();

    HWC_LOGV("copyHWCLayerIntoHwcLayer_1() id:%" PRIu64 " acq fence:%d src_crop(%f,%f,%f,%f) dst_crop(%d,%d,%d,%d)",
        from->getId(), to->acquireFenceFd, to->sourceCropf.left, to->sourceCropf.top, to->sourceCropf.right, to->sourceCropf.bottom,
        to->displayFrame.left, to->displayFrame.top, to->displayFrame.right, to->displayFrame.bottom);
}

bool listForceGPUComp(const std::vector<sp<HWCLayer> >& layers)
{
    for (auto& layer : layers)
    {
        if (layer->getSFCompositionType() == HWC2_COMPOSITION_CLIENT)
            return true;
    }
    return false;
}

// listSecure() checks if there is any secure content in the display dist
bool listSecure(const std::vector<sp<HWCLayer> >& layers)
{
    for (auto& layer : layers)
    {
        if (layer->getHandle() == nullptr)
            continue;

        const int& usage = layer->getPrivateHandle().usage;
        if (usage & (GRALLOC_USAGE_PROTECTED | GRALLOC_USAGE_SECURE))
            return true;
    }
    return false;
}

bool listS3d(const std::vector<sp<HWCLayer> >& layers, DispatcherJob* job)
{
    if (!HWCMediator::getInstance().m_features.hdmi_s3d)
    {
        return false;
    }

    for (auto& layer : layers)
    {
        if (layer->getHandle() == nullptr)
            continue;

        const PrivateHandle& priv_handle = layer->getPrivateHandle();

        int bit_S3D = (priv_handle.ext_info.status & GRALLOC_EXTRA_MASK_S3D);

        if (bit_S3D == GRALLOC_EXTRA_BIT_S3D_SBS || bit_S3D == GRALLOC_EXTRA_BIT_S3D_TAB)
        {
            // bool is_sbs = (bit_S3D == GRALLOC_EXTRA_BIT_S3D_SBS) ? true : false ;
            // bool is_tab = (bit_S3D == GRALLOC_EXTRA_BIT_S3D_TAB) ? true : false ;

            if (job == NULL)
                return true;

            /* todo: joen s3d
            layer->flags |= HWC_IS_S3D_LAYER;
            if (is_sbs)
            {
                layer->flags |= HWC_IS_S3D_LAYER_SBS;
                job->s3d_type = HWC_IS_S3D_LAYER_SBS;
            }
            else if (is_tab)
            {
                layer->flags |= HWC_IS_S3D_LAYER_TAB;
                job->s3d_type = HWC_IS_S3D_LAYER_TAB;
            }
            else
            {
                job->s3d_type = 0;
            }

            HWC_LOGI("[S3D] hasS3Dbit=%d isSBS=%d isTAB=%d status=%08x",
                bit_S3D, is_sbs, is_tab,
                priv_handle.ext_info.status);
            */

            return true;
        }
    }

    return false;
}

void checkFd(const int32_t& fd)
{
#if 1
    HWC_LOGD("%s rel fd:%d", __func__, fd);
#else
    android::CallStack stack;
    stack.update();
    HWC_LOGD("%s rel fd:%d bt:%s", __func__, fd, stack.toString().string());
#endif
    struct sync_file_info* finfo = sync_file_info(fd);
    if (fd < 0 || finfo == NULL) {
        //TODO: add new way to dump CallStack
        //android::CallStack stack;
        //stack.update();
        //ALOGE("sync_fence_info returned NULL for fd %d bt:%s", fd, stack.toString().string());
        ALOGE("sync_file_info is invalid fd=%d finfo=%p", fd, finfo);
    }
    else {
        ALOGD("checkFd(): fence(%s) status(%d)\n", finfo->name, finfo->status);

        // iterate all sync points
        //struct sync_pt_info *pt_info = NULL;
        //while ((pt_info = sync_pt_info(finfo, pt_info)))
        //{
        //    if (NULL != pt_info)
        //    {
        //        int ts_sec = pt_info->timestamp_ns / 1000000000LL;
        //        int ts_usec = (pt_info->timestamp_ns % 1000000000LL) / 1000LL;

        //        HWC_LOGW("sync point: timeline(%s) drv(%s) status(%d) sync_drv(%u) timestamp(%d.%06d)",
        //                pt_info->obj_name,
        //                pt_info->driver_name,
        //                pt_info->status,
        //                *(uint32_t *)pt_info->driver_data,
        //                ts_sec, ts_usec);
        //    }
        //}
        sync_file_info_free(finfo);

    }
}

nsecs_t getFenceSignalTime(const int32_t& fd) {
    if (fd == -1) {
        return SIGNAL_TIME_INVALID;
    }

    struct sync_file_info* finfo = sync_file_info(fd);
    if (finfo == NULL) {
        return SIGNAL_TIME_INVALID;
    }
    if (finfo->status != 1) {
        sync_file_info_free(finfo);
        return SIGNAL_TIME_PENDING;
    }

    uint64_t timestamp = 0;
    struct sync_fence_info* pinfo = sync_get_fence_info(finfo);
    for (size_t i = 0; i < finfo->num_fences; i++) {
        if (pinfo[i].timestamp_ns > timestamp) {
            timestamp = pinfo[i].timestamp_ns;
        }
    }

    sync_file_info_free(finfo);
    return nsecs_t(timestamp);
}

int setPrivateHandlePQInfo(const buffer_handle_t& handle, PrivateHandle* priv_handle)
{
    int err = 0;

    if (HWCMediator::getInstance().m_features.is_support_pq &&
        HWCMediator::getInstance().m_features.video_enhancement &&
        HWCMediator::getInstance().m_features.video_transition)
    {
        ge_pq_scltm_info_t pq_info;
        err |= gralloc_extra_query(handle, GRALLOC_EXTRA_GET_PQ_SCLTM_INFO, &pq_info);
        if (!err)
        {
            priv_handle->pq_enable = pq_info.scltmEnable;
            priv_handle->pq_pos = pq_info.scltmPosition;
            priv_handle->pq_orientation = pq_info.scltmOrientation;
            priv_handle->pq_table_idx = pq_info.scltmTableIndex;

            // error handling
            priv_handle->pq_pos = (priv_handle->pq_pos > 1024 ? 1024 : priv_handle->pq_pos);

            if (isCompressData(priv_handle) && priv_handle->pq_enable)
            {
                HWC_LOGW("MDP PQ cannot be applied for compression buffer: handle:%p pq_enable:%d", handle, priv_handle->pq_enable);
                priv_handle->pq_enable = priv_handle->pq_pos = priv_handle->pq_orientation = priv_handle->pq_table_idx = 0;
            }

            if (!priv_handle->pq_enable && priv_handle->pq_pos)
            {
                HWC_LOGW("The PQ position(%d) should be zero because PQ is not enabled", priv_handle->pq_pos);
                priv_handle->pq_enable = priv_handle->pq_pos = priv_handle->pq_orientation = priv_handle->pq_table_idx = 0;
            }
            else if (priv_handle->pq_enable && !priv_handle->pq_pos)
            {
                HWC_LOGW("The PQ position(%d) should not zero because PQ is enabled", priv_handle->pq_pos);
                priv_handle->pq_enable = priv_handle->pq_pos = priv_handle->pq_orientation = priv_handle->pq_table_idx = 0;
            }
        }
    }
    else
    {
        priv_handle->pq_enable = priv_handle->pq_pos = priv_handle->pq_orientation = priv_handle->pq_table_idx = 0;
    }

    return err;
}

void mapP3PqParam(DpPqParam* dppq_param, const int32_t& out_ds)
{
    switch (out_ds & HAL_DATASPACE_STANDARD_MASK)
    {
        case HAL_DATASPACE_STANDARD_BT601_625:
        case HAL_DATASPACE_STANDARD_BT601_625_UNADJUSTED:
        case HAL_DATASPACE_STANDARD_BT601_525:
        case HAL_DATASPACE_STANDARD_BT601_525_UNADJUSTED:
            dppq_param->dstGamut = DP_GAMUT_BT601;
            break;

        case HAL_DATASPACE_STANDARD_BT709:
            dppq_param->dstGamut = DP_GAMUT_BT709;
            break;

        case HAL_DATASPACE_STANDARD_DCI_P3:
            dppq_param->dstGamut = DP_GAMUT_DISPLAY_P3;
            break;

        case 0:
            switch (out_ds & 0xffff) {
                case HAL_DATASPACE_JFIF:
                case HAL_DATASPACE_BT601_625:
                case HAL_DATASPACE_BT601_525:
                    dppq_param->dstGamut = DP_GAMUT_BT601;
                    break;

                case HAL_DATASPACE_SRGB_LINEAR:
                case HAL_DATASPACE_SRGB:
                case HAL_DATASPACE_BT709:
                    dppq_param->dstGamut = DP_GAMUT_BT709;
                    break;
            }
            break;

        default:
            dppq_param->dstGamut = DP_GAMUT_BT601;
            HWC_LOGW("Not support color range(%#x) for PQ , use default BT601", out_ds);
    }
    dppq_param->enable = PQ_CCORR_EN;
    dppq_param->scenario = MEDIA_VIDEO;
    dppq_param->srcGamut = DP_GAMUT_DISPLAY_P3;
}

void setPQEnhance(const int& dpy,
                  const PrivateHandle& priv_handle,
                  uint32_t* pq_enhance,
                  int* assigned_output_format,
                  const bool& is_game)
{
    if (HWCMediator::getInstance().m_features.is_support_pq)
    {
        if ((dpy == HWC_DISPLAY_PRIMARY))
        {
            if (HWCMediator::getInstance().m_features.video_enhancement)
            {
                if (priv_handle.pq_enable)
                {
                    if (NULL != pq_enhance)
                        *pq_enhance = PQ_DEFAULT_EN;

                    if (NULL != assigned_output_format)
                        *assigned_output_format = HAL_PIXEL_FORMAT_YUYV;
                }
                else if ((GRALLOC_EXTRA_BIT_TYPE_VIDEO == (priv_handle.ext_info.status & GRALLOC_EXTRA_MASK_TYPE)) ||
                         (HWCMediator::getInstance().m_features.global_pq && isUIPq(&priv_handle)))
                {
                    if (NULL != pq_enhance)
                        *pq_enhance = PQ_ULTRARES_EN | PQ_COLOR_EN | PQ_DYN_CONTRAST_EN;
                }
            }
            else
            {
                if ((GRALLOC_EXTRA_BIT_TYPE_VIDEO == (priv_handle.ext_info.status & GRALLOC_EXTRA_MASK_TYPE)) ||
                    (HWCMediator::getInstance().m_features.global_pq && isUIPq(&priv_handle)))
                {
                    if (NULL != pq_enhance)
                         *pq_enhance = PQ_DEFAULT_EN;
                }
            }
            if (NULL != pq_enhance)
                *pq_enhance |= is_game;
        }
    }
}

void setPQParam(DpPqParam* dppq_param,
                const uint32_t& is_enhance,
                const bool& is_uipq,
                const int32_t& pool_id,
                const bool& is_p3,
                const int32_t& dataspace,
                const uint32_t& time_stamp,
                const buffer_handle_t& handle,
                const uint32_t& pq_table_idx,
                const bool& is_game)
{
    dppq_param->enable = is_enhance;
    dppq_param->scenario = MEDIA_VIDEO;
    if (HWCMediator::getInstance().m_features.global_pq &&
        is_uipq)
    {
        dppq_param->u.video.id = UI_PQ_VIDEO_ID;
    }
    else
    {
        dppq_param->u.video.id = pool_id;
    }
    if (is_game)
    {
        dppq_param->u.video.id = UI_PQ_VIDEO_ID;
    }

    if (is_p3)
    {
        mapP3PqParam(dppq_param, dataspace);
    }

    dppq_param->u.video.timeStamp = time_stamp;
    dppq_param->u.video.grallocExtraHandle = handle;
    dppq_param->u.video.videoScenario = is_game ? INFO_GAME : INFO_VIDEO;
    dppq_param->u.video.userScenario = INFO_HWC;
    dppq_param->u.video.isHDR2SDR = 0;
    dppq_param->u.video.paramTable = pq_table_idx;
}

int32_t getDstDataspace(const int32_t& dataspace)
{
    return ((dataspace & HAL_DATASPACE_STANDARD_MASK) != HAL_DATASPACE_STANDARD_BT2020) ?
            dataspace : HAL_DATASPACE_BT709;
}

bool isP3(const int32_t& dataspace)
{
    return (dataspace == HAL_DATASPACE_DISPLAY_P3);
}

int getPrivateHandleLayerType(const buffer_handle_t& handle, PrivateHandle* priv_handle)
{
    int err = 0;
    gralloc_extra_ion_debug_t info;
    err |= gralloc_extra_query(handle, GRALLOC_EXTRA_GET_IOCTL_ION_DEBUG, &info);

    if (!err)
    {
        priv_handle->layer_type = HWC_LAYER_TYPE_INVALID;
        if (!strncmp(info.name, "OnScreenFingerprintDimLayer", strlen("OnScreenFingerprintDimLayer")))
        {
            priv_handle->layer_type = HWC_LAYER_TYPE_HBM;
        }
    }

    return err;
}

#endif // USE_HWC2
