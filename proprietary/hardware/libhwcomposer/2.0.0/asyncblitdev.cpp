#define DEBUG_LOG_TAG "ASYNCBLTDEV"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <hardware/hwcomposer_defs.h>

#include "utils/debug.h"
#include "utils/tools.h"
#include "sync.h"
#include "overlay.h"

#include "asyncblitdev.h"

enum
{
    BLIT_INVALID_SESSION = -1,
    BLIT_VIRTUAL_SESSION = 0x80002,
};

#define checkValidDpyRetNon(dpy) \
    do {if (HWC_DISPLAY_VIRTUAL != dpy){ \
            HWC_LOGW("(%d) Failed to %s", dpy, __func__); \
            return;} \
    } while(0)

#define checkValidDpyRetVal(dpy, val) \
    do {if (HWC_DISPLAY_VIRTUAL != dpy){ \
            HWC_LOGW("(%d) Failed to %s", dpy, __func__); \
            return val;} \
    } while(0)

#define checkValidSessionRetNon(dpy, session) \
    do {if (BLIT_INVALID_SESSION == session){ \
        HWC_LOGW("(%d) Failed to %s (id=0x%x)", dpy, __func__, session); \
        return;} \
    } while(0)

#define checkValidSessionRetVal(dpy, session, val) \
    do {if (BLIT_INVALID_SESSION == session){ \
        HWC_LOGW("(%d) Failed to %s (id=0x%x)", dpy, __func__, session); \
        return val;} \
    } while(0)

#define HWC_ATRACE_BUFFER_INFO(string, n1, n2, n3, n4)                            \
        if (ATRACE_ENABLED()) {                                                   \
            char ___traceBuf[1024];                                               \
            snprintf(___traceBuf, 1024, "%s(%d:%d): %u %d", (string),             \
                (n1), (n2), (n3), (n4));                                          \
            android::ScopedTrace ___bufTracer(ATRACE_TAG, ___traceBuf);           \
        }

#define ALIGN_CEIL(x,a)     (((x) + (a) - 1L) & ~((a) - 1L))
// ---------------------------------------------------------------------------

DP_PROFILE_ENUM mapColorRange(const uint32_t range)
{
    switch (range)
    {
        case GRALLOC_EXTRA_BIT_YUV_BT601_NARROW:
            return DP_PROFILE_BT601;

        case GRALLOC_EXTRA_BIT_YUV_BT601_FULL:
            return DP_PROFILE_FULL_BT601;

        case GRALLOC_EXTRA_BIT_YUV_BT709_NARROW:
            return DP_PROFILE_BT709;
    }

    return DP_PROFILE_BT601;
}

AsyncBlitDevice::AsyncBlitDevice()
    : m_session_id(BLIT_INVALID_SESSION)
    , m_state(OVL_IN_PARAM_DISABLE)
    , m_disp_session_state(HWC_DISP_INVALID_SESSION_MODE)
{
    HWC_LOGI("create AsyncBlitDevice");

    m_blit_stream.setUser(DP_BLIT_ADDITIONAL_DISPLAY);
}

AsyncBlitDevice::~AsyncBlitDevice()
{
    HWC_LOGI("~AsyncBlitDevice");


    while (!m_job_list.isEmpty())
    {
        struct MdpJobInfo& job = m_job_list.editItemAt(0);
        if (job.release_fd != -1)
        {
            protectedClose(job.release_fd);
            job.release_fd = -1;
        }
        if (job.input_ion_fd != -1)
        {
            IONDevice::getInstance().ionCloseAndSet(&job.input_ion_fd);
        }
        if (job.output_ion_fd != -1)
        {
            IONDevice::getInstance().ionCloseAndSet(&job.output_ion_fd);
        }
        m_job_list.removeAt(0);
    }

    m_ion_flush_vector.clear();
}

void AsyncBlitDevice::initOverlay()
{
}

uint32_t AsyncBlitDevice::getDisplayRotation(uint32_t /*dpy*/)
{
    return 0;
}

bool AsyncBlitDevice::isDispRszSupported()
{
    return false;
}

bool AsyncBlitDevice::isDispRpoSupported()
{
    return false;
}

bool AsyncBlitDevice::isDispAodForceDisable()
{
    return false;
}

bool AsyncBlitDevice::isPartialUpdateSupported()
{
    return false;
}

bool AsyncBlitDevice::isFenceWaitSupported()
{
    return false;
}

bool AsyncBlitDevice::isConstantAlphaForRGBASupported()
{
    return false;
}

bool AsyncBlitDevice::isDispSelfRefreshSupported()
{
    return false;
}

bool AsyncBlitDevice::isDisplayHrtSupport()
{
    return false;
}

int32_t AsyncBlitDevice::getSupportedColorMode()
{
    return HAL_COLOR_MODE_NATIVE;
}

int AsyncBlitDevice::getMaxOverlayInputNum()
{
    return 1;
}

uint32_t AsyncBlitDevice::getMaxOverlayHeight()
{
    return 0;
}

uint32_t AsyncBlitDevice::getMaxOverlayWidth()
{
    return 0;
}

int32_t AsyncBlitDevice::getDisplayOutputRotated()
{
    return 0;
}

uint32_t AsyncBlitDevice::getRszMaxWidthInput()
{
    return 0;
}

uint32_t AsyncBlitDevice::getRszMaxHeightInput()
{
    return 0;
}

void AsyncBlitDevice::enableDisplayFeature(uint32_t /*flag*/)
{
}

void AsyncBlitDevice::disableDisplayFeature(uint32_t /*flag*/)
{
}

status_t AsyncBlitDevice::createOverlaySession(int dpy, HWC_DISP_MODE mode)
{
    checkValidDpyRetVal(dpy, INVALID_OPERATION);

    if (BLIT_INVALID_SESSION != m_session_id)
    {
        HWC_LOGW("(%d) Failed to create existed BlitSession (id=0x%x)", dpy, m_session_id);
        return INVALID_OPERATION;
    }

    {
        AutoMutex mutex(m_state_lock);
        m_session_id = BLIT_VIRTUAL_SESSION;
        memset(&m_disp_session_info, 0, sizeof(m_disp_session_info));
        m_disp_session_info.maxLayerNum = 1;
        m_disp_session_info.isHwVsyncAvailable = false;
        m_disp_session_info.isConnected = true;
        m_disp_session_info.isHDCPSupported = 0;
    }

    HWC_LOGD("(%d) Create BlitSession (id=0x%x, mode=%d)", dpy, m_session_id, mode);

    return NO_ERROR;
}

void AsyncBlitDevice::destroyOverlaySession(int dpy)
{
    checkValidDpyRetNon(dpy);

    checkValidSessionRetNon(dpy, m_session_id);

    HWC_LOGD("(%d) Destroy BlitSession (id=0x%x)", dpy, m_session_id);

    {
        AutoMutex mutex(m_state_lock);
        m_session_id = BLIT_INVALID_SESSION;
        memset(&m_disp_session_info, 0, sizeof(m_disp_session_info));
    }

    {
        AutoMutex l(m_vector_lock);
        while (!m_job_list.isEmpty())
        {
            struct MdpJobInfo& job = m_job_list.editItemAt(0);
            if (job.release_fd != -1)
            {
                protectedClose(job.release_fd);
                job.release_fd = -1;
            }
            if (job.input_ion_fd != -1)
            {
                IONDevice::getInstance().ionCloseAndSet(&job.input_ion_fd);
            }
            if (job.output_ion_fd != -1)
            {
                IONDevice::getInstance().ionCloseAndSet(&job.output_ion_fd);
            }
            m_job_list.removeAt(0);
        }
    }
}

status_t AsyncBlitDevice::triggerOverlaySession(
    int dpy, int /*present_fence_idx*/, int /*ovlp_layer_num*/, int /*prev_present_fence_fd*/,
    hwc2_config_t /*config*/, const uint32_t& /*hrt_weight*/, const uint32_t& /*hrt_idx*/, const bool& /*is_HBM*/)
{
    checkValidDpyRetVal(dpy, INVALID_OPERATION);

    checkValidSessionRetVal(dpy, m_session_id, INVALID_OPERATION);

    status_t err = NO_ERROR;

    m_blit_stream.setOrientation(0, DpAsyncBlitStream::ROT_0);

    // setup for PQ
    DpPqParam dppq_param;
    dppq_param.enable = false;
    dppq_param.scenario = MEDIA_VIDEO;
    dppq_param.u.video.id = 0;
    dppq_param.u.video.timeStamp = 0;
    m_blit_stream.setPQParameter(0, dppq_param);

    HWC_LOGD("INVALIDATE/s_flush=%x/s_range=%d/s_sec=%d"
        "/s_acq=%d/s_ion=%d/s_fmt=%x"
        "/d_flush=%x/d_range=%d/d_sec=%d"
        "/d_rel=%d/d_ion=%d/d_fmt=%x"
        "/(%d,%d,%d,%d)->(%d,%d,%d,%d)",
        m_cur_params.src_is_need_flush, m_cur_params.src_range, m_cur_params.src_is_secure,
        m_cur_params.src_fence_index, m_cur_params.src_ion_fd, m_cur_params.src_fmt,
        m_cur_params.dst_is_need_flush, m_cur_params.dst_range, m_cur_params.dst_is_secure,
        m_cur_params.dst_fence_index, m_cur_params.dst_ion_fd, m_cur_params.dst_fmt,
        m_cur_params.src_crop.top, m_cur_params.src_crop.left, m_cur_params.src_crop.right, m_cur_params.src_crop.bottom,
        m_cur_params.dst_crop.top, m_cur_params.dst_crop.left, m_cur_params.dst_crop.right, m_cur_params.dst_crop.bottom);

    m_blit_stream.setConfigEnd();

    DP_STATUS_ENUM status = m_blit_stream.invalidate();
    if (DP_STATUS_RETURN_SUCCESS != status)
    {
        HWC_LOGE("INVALIDATE/blit fail/err=%d", status);
    }

    if (m_cur_params.src_is_secure)
    {
        // TODO: must guarantee life cycle of the secure buffer
    }
    else
    {
        // TODO: should be removed after intergrating prepare hehavior to DpBlitStream
    }

    if (m_cur_params.dst_is_secure)
    {
        // TODO: must guarantee life cycle of the secure buffer
    }
    else
    {
    }

    // extension mode
    if (DisplayManager::m_profile_level & PROFILE_TRIG)
    {
        char atrace_tag[256];
        sprintf(atrace_tag, "BLT-SMS");
        HWC_ATRACE_ASYNC_END(atrace_tag, m_cur_params.job_sequence);
    }

    return err;
}

void AsyncBlitDevice::disableOverlaySession(
    int dpy,  OverlayPortParam* const* /*params*/, int /*num*/)
{
    checkValidDpyRetNon(dpy);

    checkValidSessionRetNon(dpy, m_session_id);

    HWC_LOGD("(%d) Disable BlitSession (id=0x%x)", dpy, m_session_id);
}

status_t AsyncBlitDevice::setOverlaySessionMode(int dpy, HWC_DISP_MODE mode)
{
    checkValidDpyRetVal(dpy, INVALID_OPERATION);

    checkValidSessionRetVal(dpy, m_session_id, INVALID_OPERATION);

    HWC_LOGD("(%d) Set BlitSessionMode (id=0x%x mode=%s)", dpy, m_session_id, getSessionModeString(mode).string());

    {
        AutoMutex l(m_state_lock);
        m_disp_session_state = mode;

    }

    if (m_disp_session_state != HWC_DISP_SESSION_DECOUPLE_MODE)
    {
        HWC_LOGE("AsyncBlitDevice only support decouple mode!");
        return INVALID_OPERATION;
    }

    return NO_ERROR;
}

HWC_DISP_MODE AsyncBlitDevice::getOverlaySessionMode(int dpy)
{
    checkValidSessionRetVal(dpy, m_session_id, HWC_DISP_INVALID_SESSION_MODE);

    HWC_DISP_MODE disp_mode = HWC_DISP_INVALID_SESSION_MODE;
    {
        AutoMutex mutex(m_state_lock);
        disp_mode = m_disp_session_state;
    }
    return disp_mode;
}

status_t AsyncBlitDevice::getOverlaySessionInfo(int dpy, SessionInfo* info)
{
    checkValidDpyRetVal(dpy, INVALID_OPERATION);

    checkValidSessionRetVal(dpy, m_session_id, INVALID_OPERATION);

    AutoMutex mutex(m_state_lock);
    if (m_session_id == BLIT_INVALID_SESSION)
    {
        return INVALID_OPERATION;
    }

    memcpy(info, &m_disp_session_info, sizeof(m_disp_session_info));
    return NO_ERROR;
}

int AsyncBlitDevice::getAvailableOverlayInput(int dpy)
{
    checkValidDpyRetVal(dpy, 0);

    checkValidSessionRetVal(dpy, m_session_id, 0);

    return 1;
}

void AsyncBlitDevice::prepareOverlayInput(
    int dpy, OverlayPrepareParam* param)
{
    checkValidDpyRetNon(dpy);

    checkValidSessionRetNon(dpy, m_session_id);

    if (param->id != 0)
    {
        HWC_LOGE("AsyncBlitDevice support only 1 ovl input!");
    }

    int input_ion_fd = -1;
    bool closeIon = false;
    IONDevice::getInstance().ionImport(param->ion_fd, &input_ion_fd);
    {
        AutoMutex l(m_vector_lock);
        if (!m_job_list.isEmpty())
        {
            struct MdpJobInfo& job = m_job_list.editTop();
            job.input_ion_fd = input_ion_fd;
            param->fence_fd = ::dup(job.release_fd);
            param->fence_index = job.id;
        }
        else
        {
            param->fence_fd = -1;
            param->fence_index = 0;
            closeIon = true;
            HWC_LOGE("prepareOverlayInput with no MdpJobInfo");
        }

        m_ion_flush_vector.add(param->ion_fd, param->is_need_flush);
    }
    if (closeIon) {
        IONDevice::getInstance().ionCloseAndSet(&input_ion_fd);
    }

    HWC_ATRACE_BUFFER_INFO("pre_input",
        dpy, param->id, param->fence_index, param->fence_fd);
}

void AsyncBlitDevice::updateOverlayInputs(
    int dpy, OverlayPortParam* const* params, int /*num*/, sp<ColorTransform> /*color_transform*/)
{
    checkValidDpyRetNon(dpy);

    checkValidSessionRetNon(dpy, m_session_id);

    OverlayPortParam* param = params[0];

    if (OVL_IN_PARAM_ENABLE != param->state)
    {
        HWC_LOGI("updateOverlayInputs ignore state(%d)!", param->state);
        m_state = param->state;

        return;
    }

    uint32_t job_id = 0;
    int src_ion_fd = -1;
    {
        AutoMutex l(m_vector_lock);
        if (m_job_list.isEmpty()) {
            HWC_LOGE("updateOverlayInputs with no MdpJob");
            return;
        }
        struct MdpJobInfo& job = m_job_list.editItemAt(0);
        job_id = job.id;
        if (job.release_fd >= 0)
        {
            protectedClose(job.release_fd);
            job.release_fd = -1;
        }
        src_ion_fd = job.input_ion_fd;
        job.input_ion_fd = -1;
    }
    m_blit_stream.setConfigBegin(job_id);

    DpSecure is_dp_secure = DP_SECURE_NONE;
    DpColorFormat src_dpformat;
    unsigned int  src_pitch;
    bool secure = param->secure;

    {
        unsigned int  src_plane;
        unsigned int  src_size[3];

        unsigned int stride = param->pitch;
        unsigned int height = param->src_crop.bottom;

        switch (param->format)
        {
            case HAL_PIXEL_FORMAT_RGBA_8888:
            case HAL_PIXEL_FORMAT_RGBX_8888:
                src_pitch = stride * 4;
                src_plane = 1;
                src_size[0] = src_pitch * height;
                src_dpformat = DP_COLOR_RGBA8888;
                break;

            case HAL_PIXEL_FORMAT_BGRA_8888:
            case HAL_PIXEL_FORMAT_BGRX_8888:
            case HAL_PIXEL_FORMAT_IMG1_BGRX_8888:
                src_pitch = stride * 4;
                src_plane = 1;
                src_size[0] = src_pitch * height;
                src_dpformat = DP_COLOR_BGRA8888;
                break;

            case HAL_PIXEL_FORMAT_RGB_888:
                src_pitch = stride * 3;
                src_plane = 1;
                src_size[0] = src_pitch * height;
                src_dpformat = DP_COLOR_RGB888;
                break;

            case HAL_PIXEL_FORMAT_RGB_565:
                src_pitch = stride * 2;
                src_plane = 1;
                src_size[0] = src_pitch * height;
                src_dpformat = DP_COLOR_RGB565;
                break;
#if 0 // ovl input didn't support YV12
            case HAL_PIXEL_FORMAT_YV12:
                {
                    src_pitch    = stride;
                    unsigned int src_pitch_uv = ALIGN_CEIL((stride / 2), 16);
                    src_plane = 3;
                    unsigned int src_size_luma = src_pitch * height;
                    unsigned int src_size_chroma = config->src_pitch_uv * (height / 2);
                    src_size[0] = src_size_luma;
                    src_size[1] = src_size_chroma;
                    src_size[2] = src_size_chroma;
                    src_dpformat = DP_COLOR_YV12;
                }
                break;
#endif
            case HAL_PIXEL_FORMAT_YUYV:
                src_pitch = stride * 2;
                src_plane = 1;
                src_size[0] = src_pitch * height;
                src_dpformat = DP_COLOR_YUYV;
                break;

            default:
                HWC_LOGW("Input color format for DP is invalid (0x%x)", param->format);
                if (src_ion_fd != -1)
                {
                    IONDevice::getInstance().ionCloseAndSet(&src_ion_fd);
                }
                return;
        }

        if (secure)
        {
            void* src_addr[3];
            src_addr[0] = (void*)(uintptr_t)param->mva;
            src_addr[1] = (void*)(uintptr_t)param->mva;
            src_addr[2] = (void*)(uintptr_t)param->mva;

            m_blit_stream.setSrcBuffer(src_addr, src_size, src_plane);
            is_dp_secure = DP_SECURE;
        }
        else
        {
            m_blit_stream.setSrcBuffer(src_ion_fd, src_size, src_plane);
        }
    }
    //-----------------------------------------------------------

    bool is_need_flush = false;
    {
        AutoMutex l(m_vector_lock);

        is_need_flush = m_ion_flush_vector.valueFor(src_ion_fd);

        m_ion_flush_vector.removeItem(src_ion_fd);
    }

    DP_PROFILE_ENUM dp_range = mapColorRange(param->color_range);

    DpRect src_dp_roi;
    int width = param->src_crop.getWidth();
    int height = param->src_crop.getHeight();

    src_dp_roi.x = param->src_crop.left;
    src_dp_roi.y = param->src_crop.top;
#if 1
    src_dp_roi.w = width;
    src_dp_roi.h = height;
#else
    // TODO: need to align 2bytes?
    src_dp_roi.w = ALIGN_FLOOR(param->src_crop.getWidth(), 2);
    src_dp_roi.h = ALIGN_FLOOR(param->src_crop.getHeight(), 2);
#endif
    // [NOTE] setSrcConfig provides y and uv pitch configuration
    // if uv pitch is 0, DP would calculate it according to y pitch
    HWC_LOGD("(%d) SRC: size=%dx%d:%d f=%x crop=%d,%d %dx%d",
            dpy, param->pitch, height, src_pitch, src_dpformat,
            src_dp_roi.x, src_dp_roi.y, src_dp_roi.w, src_dp_roi.h);
    m_blit_stream.setSrcConfig(
        param->pitch, height,
        src_pitch, 0,
        src_dpformat, dp_range,
        eInterlace_None, is_dp_secure,
    is_need_flush);
    m_blit_stream.setSrcCrop(0, src_dp_roi);

    m_cur_params.src_ion_fd = src_ion_fd;
    m_cur_params.src_fence_index = param->fence_index;
    m_cur_params.src_fmt = param->format;
    m_cur_params.src_crop = param->src_crop;
    m_cur_params.src_range = param->color_range;
    m_cur_params.src_is_need_flush = is_need_flush;
    m_cur_params.src_is_secure = secure;

    m_state = OVL_IN_PARAM_ENABLE;

    if (src_ion_fd != -1)
    {
        IONDevice::getInstance().ionCloseAndSet(&src_ion_fd);
    }
}

void AsyncBlitDevice::prepareOverlayOutput(int dpy, OverlayPrepareParam* param)
{
    checkValidDpyRetNon(dpy);

    checkValidSessionRetNon(dpy, m_session_id);

    struct MdpJobInfo job;
    m_blit_stream.createJob(job.id, job.release_fd);

    param->fence_fd = ::dup(job.release_fd);
    param->fence_index = job.id;
    param->if_fence_index = -1;
    param->if_fence_fd    = -1;

    IONDevice::getInstance().ionImport(param->ion_fd, &job.output_ion_fd, "AsyncBlitDevice::prepareOverlayOutput()");

    HWC_ATRACE_BUFFER_INFO("pre_output",
        dpy, param->id, param->fence_index, param->fence_fd);

    {
        AutoMutex l(m_vector_lock);
        m_job_list.push_back(job);
    }
}

void AsyncBlitDevice::enableOverlayOutput(int dpy, OverlayPortParam* param)
{
    checkValidDpyRetNon(dpy);

    checkValidSessionRetNon(dpy, m_session_id);

    int dst_ion_fd = -1;
    {
        AutoMutex l(m_vector_lock);
        if (m_job_list.isEmpty())
        {
            HWC_LOGE("updateOverlayOutput with no MdpJob");
            return;
        }
        struct MdpJobInfo& job = m_job_list.editItemAt(0);
        dst_ion_fd = job.output_ion_fd;
        job.output_ion_fd = -1;
        m_job_list.removeAt(0);
    }

    DpSecure is_dp_secure = DP_SECURE_NONE;
    DpColorFormat dst_dpformat;
    unsigned int  dst_pitch;
    unsigned int dst_pitch_uv = 0;
    bool secure = param->secure;

    {
        unsigned int  dst_plane;
        unsigned int  dst_size[3];

        unsigned int stride = param->pitch;
        unsigned int height = param->dst_crop.bottom;

        switch (param->format)
        {
            case HAL_PIXEL_FORMAT_RGBA_8888:
            case HAL_PIXEL_FORMAT_RGBX_8888:
                dst_pitch = stride * 4;
                dst_plane = 1;
                dst_size[0] = dst_pitch * height;
                dst_dpformat = DP_COLOR_RGBA8888;
                break;

            case HAL_PIXEL_FORMAT_BGRA_8888:
            case HAL_PIXEL_FORMAT_BGRX_8888:
            case HAL_PIXEL_FORMAT_IMG1_BGRX_8888:
                dst_pitch = stride * 4;
                dst_plane = 1;
                dst_size[0] = dst_pitch * height;
                dst_dpformat = DP_COLOR_BGRA8888;
                break;

            case HAL_PIXEL_FORMAT_RGB_888:
                dst_pitch = stride * 3;
                dst_plane = 1;
                dst_size[0] = dst_pitch * height;
                dst_dpformat = DP_COLOR_RGB888;
                break;

            case HAL_PIXEL_FORMAT_RGB_565:
                dst_pitch = stride * 2;
                dst_plane = 1;
                dst_size[0] = dst_pitch * height;
                dst_dpformat = DP_COLOR_RGB565;
                break;

            case HAL_PIXEL_FORMAT_YV12:
                {
                    dst_pitch    = stride;
                    dst_pitch_uv = ALIGN_CEIL((stride / 2), 16);
                    dst_plane = 3;
                    unsigned int dst_size_luma = dst_pitch * height;
                    unsigned int dst_size_chroma = dst_pitch_uv * (height / 2);
                    dst_size[0] = dst_size_luma;
                    dst_size[1] = dst_size_chroma;
                    dst_size[2] = dst_size_chroma;
                    dst_dpformat = DP_COLOR_YV12;
                }
                break;

            case HAL_PIXEL_FORMAT_YUYV:
                dst_pitch = stride * 2;
                dst_plane = 1;
                dst_size[0] = dst_pitch * height;
                dst_dpformat = DP_COLOR_YUYV;
                break;

            case HAL_PIXEL_FORMAT_YCRCB_420_SP:
                dst_pitch    = stride;
                dst_pitch_uv = ALIGN_CEIL(stride, 1);
                dst_plane = 2;
                dst_size[0] = stride * ALIGN_CEIL(height, 2);;
                dst_size[1] = dst_pitch_uv * ALIGN_CEIL(height, 2) / 2;;
                dst_dpformat = DP_COLOR_NV21;
                break;

            default:
                HWC_LOGW("Output color format for DP is invalid (0x%x)", param->format);
                if (dst_ion_fd != -1)
                {
                    IONDevice::getInstance().ionCloseAndSet(&dst_ion_fd);
                }
                return;
        }

        if (secure)
        {
            void* dst_addr[3];
            dst_addr[0] = (void*)(uintptr_t)param->mva;
            dst_addr[1] = (void*)(uintptr_t)param->mva;
            dst_addr[2] = (void*)(uintptr_t)param->mva;

            m_blit_stream.setDstBuffer(0, dst_addr, dst_size, dst_plane);
            is_dp_secure = DP_SECURE;
        }

        m_blit_stream.setDstBuffer(0, dst_ion_fd, dst_size, dst_plane);
    }
    //-----------------------------------------------------------

    DP_PROFILE_ENUM dp_range = mapColorRange(param->color_range);

    DpRect dst_dp_roi;
    int width = param->dst_crop.getWidth();
    int height = param->dst_crop.getHeight();

    dst_dp_roi.x = param->dst_crop.left;
    dst_dp_roi.y = param->dst_crop.top;
#if 1
    dst_dp_roi.w = width;
    dst_dp_roi.h = height;
#else
    // TODO: need to align 2 bytes?
    dst_dp_roi.w = ALIGN_FLOOR(param->dst_crop.getWidth(), 2);
    dst_dp_roi.h = ALIGN_FLOOR(param->dst_crop.getHeight(), 2);
#endif

    // [NOTE] setDstConfig provides y and uv pitch configuration
    // if uv pitch is 0, DP would calculate it according to y pitch
    // ROI designates the dimension and the position of the bitblited image
    m_blit_stream.setDstConfig(0,
        dst_dp_roi.w, dst_dp_roi.h,
        dst_pitch, dst_pitch_uv,
        dst_dpformat, dp_range,
        eInterlace_None, &dst_dp_roi, is_dp_secure, false);
    HWC_LOGD("(%d) DST: size=%dx%d:%d f=%x crop=%d,%d %dx%d",
            dpy, dst_dp_roi.w, dst_dp_roi.h, dst_pitch, dst_dpformat,
            dst_dp_roi.x, dst_dp_roi.y, dst_dp_roi.w, dst_dp_roi.h);

    m_cur_params.dst_ion_fd = dst_ion_fd;
    m_cur_params.dst_fence_index = param->fence_index;
    m_cur_params.dst_fmt = param->format;
    m_cur_params.dst_crop = param->dst_crop;
    m_cur_params.dst_range = param->color_range;
    m_cur_params.dst_is_need_flush = false;
    m_cur_params.dst_is_secure = secure;
    m_cur_params.job_sequence = param->sequence;
    // extension mode
    if (DisplayManager::m_profile_level & PROFILE_TRIG)
    {
        char atrace_tag[256];
        sprintf(atrace_tag, "BLT-SMS");
        HWC_ATRACE_ASYNC_BEGIN(atrace_tag, param->sequence);
    }

    if (dst_ion_fd != -1)
    {
        IONDevice::getInstance().ionCloseAndSet(&dst_ion_fd);
    }
}

void AsyncBlitDevice::disableOverlayOutput(int /*dpy*/)
{
}

void AsyncBlitDevice::prepareOverlayPresentFence(int dpy, OverlayPrepareParam* param)
{
    checkValidDpyRetNon(dpy);

    checkValidSessionRetNon(dpy, m_session_id);

    param->fence_index = -1;
    param->fence_fd    = -1;

    HWC_LOGD("(%d) Prepare Present Fence (id=0x%x)", dpy, m_session_id);
}

status_t AsyncBlitDevice::waitVSync(int /*dpy*/, nsecs_t* /*ts*/)
{
    HWC_LOGE("Do not call %s", __func__);
    return INVALID_OPERATION;
}

void AsyncBlitDevice::setPowerMode(int /*dpy*/, int /*mode*/)
{
    HWC_LOGE("setPowerMode error! (id=0x%x)", m_session_id);
}

bool AsyncBlitDevice::queryValidLayer(void* /*disp_layer*/)
{
    HWC_LOGE("Do NOT call %s", __func__);
    return false;
}

status_t AsyncBlitDevice::waitAllJobDone(const int /*dpy*/)
{
    return NO_ERROR;
}

void AsyncBlitDevice::setLastValidColorTransform(const int32_t& /*dpy*/, sp<ColorTransform> /*color_transform*/)
{
    HWC_LOGE("Do NOT call %s", __func__);
}

status_t AsyncBlitDevice::waitRefreshRequest(unsigned int* /*type*/)
{
    HWC_LOGE("Do NOT call %s", __func__);
    return NO_ERROR;
}

int32_t AsyncBlitDevice::getWidth(int /*dpy*/, hwc2_config_t /*config*/)
{
    HWC_LOGE("Do NOT call %s", __func__);
    return 0;
}

int32_t AsyncBlitDevice::getHeight(int /*dpy*/, hwc2_config_t /*config*/)
{
    HWC_LOGE("Do NOT call %s", __func__);
    return 0;
}

int32_t AsyncBlitDevice::getRefresh(int /*dpy*/, hwc2_config_t /*config*/)
{
    HWC_LOGE("Do NOT call %s", __func__);
    return 0;
}

uint32_t AsyncBlitDevice::getNumConfigs(int /*dpy*/)
{
    HWC_LOGE("Do NOT call %s", __func__);
    return 0;
}
