#define DEBUG_LOG_TAG "BLT_ASYNC"

#include "hwc_priv.h"
#include "graphics_mtk_defs.h"
#include "gralloc_mtk_defs.h"

#include <hardware/gralloc.h>

#include "utils/debug.h"
#include "utils/tools.h"

#include "utils/transform.h"

#include "bliter_async.h"
#include "display.h"
#include "platform.h"
#include "overlay.h"
#include "dispatcher.h"
#include "worker.h"
#include "queue.h"
#include "sync.h"
#include "hwc2.h"
#include "bliter_ultra.h"

#include <sync/sync.h>

#include <utils/String8.h>

#define ALIGN_FLOOR(x,a)    ((x) & ~((a) - 1L))
#define ALIGN_CEIL(x,a)     (((x) + (a) - 1L) & ~((a) - 1L))

#define NOT_PRIVATE_FORMAT -1

#define BLOGD(i, x, ...) HWC_LOGD("(%d:%d) " x, m_disp_id, i, ##__VA_ARGS__)
#define BLOGI(i, x, ...) HWC_LOGI("(%d:%d) " x, m_disp_id, i, ##__VA_ARGS__)
#define BLOGW(i, x, ...) HWC_LOGW("(%d:%d) " x, m_disp_id, i, ##__VA_ARGS__)
#define BLOGE(i, x, ...) HWC_LOGE("(%d:%d) " x, m_disp_id, i, ##__VA_ARGS__)

#ifdef USE_SWWATCHDOG
#include "utils/swwatchdog.h"
#define WDT_BL_NODE(fn, ...)                                                                    \
({                                                                                              \
    if (Platform::getInstance().m_config.wdt_trace)                                             \
    {                                                                                           \
        ATRACE_NAME(#fn);                                                                       \
        SWWatchDog::AutoWDT _wdt(String8::format("[BLT_ASYNC] BliterNode." #fn "():%d", __LINE__), 500); \
        m_bliter_node->fn(__VA_ARGS__);                                                         \
    }                                                                                           \
    else                                                                                        \
    {                                                                                           \
        SWWatchDog::AutoWDT _wdt(String8::format("[BLT_ASYNC] BliterNode." #fn "():%d", __LINE__), 500); \
        m_bliter_node->fn(__VA_ARGS__);                                                         \
    }                                                                                           \
})
#else // USE_SWWATCHDOG
#define WDT_BL_NODE(fn, ...)                                                                    \
({                                                                                              \
    if (Platform::getInstance().m_config.wdt_trace)                                             \
    {                                                                                           \
        ATRACE_NAME(#fn);                                                                       \
        m_bliter_node->fn(__VA_ARGS__);                                                         \
    }                                                                                           \
    else                                                                                        \
    {                                                                                           \
        m_bliter_node->fn(__VA_ARGS__);                                                         \
    }                                                                                           \
})
#endif // USE_SWWATCHDOG

extern DP_PROFILE_ENUM mapDpColorRange(const uint32_t range);
extern unsigned int mapDpOrientation(const uint32_t transform);

int convertFormat4Bliter(const uint32_t format)
{
    int bliter_format = mapGrallocFormat(format);

    // when Platform does not support 2-subsampled format with odd size of roi, we
    // should not use YUYV. If the roi size of YUYV buffer is odd, ovl shows abnormal
    // screen content. Therefore we change the format to RGB888.
    if (Platform::getInstance().m_config.support_2subsample_with_odd_size_roi == false &&
        bliter_format == HAL_PIXEL_FORMAT_YUYV)
    {
        return HAL_PIXEL_FORMAT_RGB_888;
    }
    return bliter_format;
}

// ---------------------------------------------------------------------------

AsyncBliterHandler::AsyncBliterHandler(int dpy, const sp<OverlayEngine>& ovl_engine)
    : LayerHandler(dpy, ovl_engine)
{
    int num = m_ovl_engine->getMaxInputNum() + 1;
    m_dp_configs = (BufferConfig*)calloc(1, sizeof(BufferConfig) * num);
    LOG_ALWAYS_FATAL_IF(m_dp_configs == nullptr, "async dp_config calloc(%zu) fail",
        sizeof(BufferConfig) * num);

    m_bliter_node = new BliterNode(&m_blit_stream, m_disp_id);

    if (dpy == HWC_DISPLAY_PRIMARY)
    {
        m_blit_stream.setUser(DP_BLIT_GENERAL_USER);
        UltraBliter::getInstance().setBliter(m_bliter_node);
    }
    else
    {
        m_blit_stream.setUser(DP_BLIT_ADDITIONAL_DISPLAY);
    }
}

AsyncBliterHandler::~AsyncBliterHandler()
{
    free(m_dp_configs);

    if (NULL != m_bliter_node)
    {
        delete m_bliter_node;
        m_bliter_node = NULL;
    }

    if (HWC_DISPLAY_PRIMARY == m_disp_id)
    {
        UltraBliter::getInstance().setBliter(NULL);
    }
}

void AsyncBliterHandler::set(const sp<HWCDisplay>& display, DispatcherJob* job)
{
    if (HWC_MIRROR_SOURCE_INVALID != job->disp_mir_id)
    {
        setMirror(display, job);
        return;
    }

    if (job->is_black_job)
    {
        setBlack(display, job);
        return;
    }

    uint32_t total_num = job->num_layers;

    auto&& layers = display->getCommittedLayers();

    for (uint32_t i = 0; i < total_num; i++)
    {
        HWLayer* hw_layer = &job->hw_layers[i];

        // skip non mm layers
        if (HWC_LAYER_TYPE_MM != hw_layer->type && HWC_LAYER_TYPE_MM_FBT != hw_layer->type) continue;

        // this layer is not enable
        if (!hw_layer->enable) continue;

        sp<HWCLayer> layer = layers[hw_layer->index];
        PrivateHandle& priv_handle = hw_layer->priv_handle;

        DbgLogger logger(DbgLogger::TYPE_HWC_LOG, 'D', "[BLT_ASYNC]SET(%d,%d)", m_disp_id, i);

        if (hw_layer->dirty)
        {
            // workaround
            int release_fence_fd = -1;
            // get release fence from mdp
            m_blit_stream.createJob(hw_layer->mdp_job_id, release_fence_fd);
            layer->setReleaseFenceFd(release_fence_fd, display->isConnected());

            copyHWCLayerIntoHwcLayer_1(layer, &hw_layer->layer);
            hw_layer->layer.releaseFenceFd = dup(layer->getReleaseFenceFd());

            logger.printf("/rel=%d/acq=%d/handle=%p/job=%d/pq=%d,%d,%d,%d",
                layer->getReleaseFenceFd(), layer->getAcquireFenceFd(),
                layer->getHandle(), hw_layer->mdp_job_id,
                priv_handle.pq_enable, priv_handle.pq_pos, priv_handle.pq_orientation, priv_handle.pq_table_idx);

            if (DisplayManager::m_profile_level & PROFILE_BLT)
            {
                char atrace_tag[256];
                sprintf(atrace_tag, "mm set:%p", layer->getHandle());
                HWC_ATRACE_NAME(atrace_tag);
            }

            if (isSecure(&priv_handle))
            {
                logger.printf("/sec");
            }

            if (priv_handle.ion_fd > 0)
            {
                // workaround:
                //IONDevice::getInstance().ionImport(&*(const_cast<int32_t*>(&priv_handle.ion_fd)));
                IONDevice::getInstance().ionImport(&priv_handle.ion_fd);
            }
        }
        else
        {
            hw_layer->mdp_job_id = 0;

            if (layer->getAcquireFenceFd() != -1)
            {
                ::protectedClose(layer->getAcquireFenceFd());
                layer->setAcquireFenceFd(-1, display->isConnected());
            }

            if (layer->getReleaseFenceFd() != -1)
            {
                ::protectedClose(layer->getReleaseFenceFd());
                layer->setReleaseFenceFd(-1, display->isConnected());
            }

            copyHWCLayerIntoHwcLayer_1(layer, &hw_layer->layer);

            logger.printf("/async=bypass/acq=%d/handle=%p", layer->getAcquireFenceFd(), layer->getHandle());

            if (DisplayManager::m_profile_level & PROFILE_BLT)
            {
                char atrace_tag[256];
                sprintf(atrace_tag, "mm bypass:%p", layer->getHandle());
                HWC_ATRACE_NAME(atrace_tag);
            }
        }

        if (layer->getAcquireFenceFd() != -1)
        {
            ::protectedClose(layer->getAcquireFenceFd());
            layer->setAcquireFenceFd(-1, display->isConnected());
        }
    }
}

void AsyncBliterHandler::setMirror(
    const sp<HWCDisplay>& display,
    DispatcherJob* job)
{
    // clear all layer's acquire fence and retire fence
    if (display != nullptr)
    {
        if (HWC_DISPLAY_VIRTUAL == job->disp_ori_id)
        {
            for (auto& layer : display->getCommittedLayers())
            {
                layer->setReleaseFenceFd(-1, display->isConnected());
                if (layer->getAcquireFenceFd() != -1)
                {
                    ::protectedClose(layer->getAcquireFenceFd());
                    layer->setAcquireFenceFd(-1, display->isConnected());
                }
            }
        }
        else
        {
            display->clearAllFences();
        }
    }
    else
    {
        BLOGE(0, "setMirror, display is NULL");
        return;
    }

    // get release fence from mdp
    HWBuffer* hw_mirbuf = &job->hw_mirbuf;
    m_blit_stream.createJob(job->fill_black[0].id, job->fill_black[0].fence);
    m_blit_stream.createJob(job->fill_black[1].id, job->fill_black[1].fence);
    m_blit_stream.createJob(job->mdp_job_output_buffer, hw_mirbuf->mir_in_rel_fence_fd);
    BLOGD(0, "create mirror fence by async: fill:%d|%d  fill:%d|%d  output:%d|%d",
        job->fill_black[0].id, job->fill_black[0].fence,
        job->fill_black[1].id, job->fill_black[1].fence,
        job->mdp_job_output_buffer, hw_mirbuf->mir_in_rel_fence_fd);

    // [NOTE]
    // there are two users who uses the retireFenceFd
    // 1) SurfaceFlinger 2) HWComposer
    // hence, the fence file descriptor MUST be DUPLICATED before
    // passing to SurfaceFlinger;
    // otherwise, HWComposer may wait for the WRONG fence file descriptor that
    // has been closed by SurfaceFlinger.
    //
    // we would let bliter output to virtual display's outbuf directly.
    // So retire fence is as same as bliter's release fence
    if (HWC_DISPLAY_VIRTUAL == job->disp_ori_id)
    {
        display->setRetireFenceFd(::dup(hw_mirbuf->mir_in_rel_fence_fd), display->isConnected());
    }
}

void AsyncBliterHandler::setBlack(
    const sp<HWCDisplay>& display,
    DispatcherJob* job)
{
    // clear all layer's acquire fence and retire fence
    if (display != nullptr)
    {
        if (HWC_DISPLAY_VIRTUAL == job->disp_ori_id)
        {
            for (auto& layer : display->getCommittedLayers())
            {
                layer->setReleaseFenceFd(-1, display->isConnected());
                if (layer->getAcquireFenceFd() != -1)
                {
                    ::protectedClose(layer->getAcquireFenceFd());
                    layer->setAcquireFenceFd(-1, display->isConnected());
                }
            }
        }
        else
        {
            display->clearAllFences();
        }
    }
    else
    {
        BLOGE(0, "setBlack, display is NULL");
        return;
    }

    // get release fence from mdp
    m_blit_stream.createJob(job->fill_black[0].id, job->fill_black[0].fence);
    m_blit_stream.createJob(job->fill_black[1].id, job->fill_black[1].fence);
    BLOGD(job->disp_ori_id, "create black fence by async: fill:%d|%d  fill:%d|%d  ",
        job->fill_black[0].id, job->fill_black[0].fence,
        job->fill_black[1].id, job->fill_black[1].fence);

    // [NOTE]
    // there are two users who uses the retireFenceFd
    // 1) SurfaceFlinger 2) HWComposer
    // hence, the fence file descriptor MUST be DUPLICATED before
    // passing to SurfaceFlinger;
    // otherwise, HWComposer may wait for the WRONG fence file descriptor that
    // has been closed by SurfaceFlinger.
    //
    // we would let bliter output to virtual display's outbuf directly.
    // So retire fence is as same as bliter's release fence

    HWBuffer& dst_buf = job->hw_outbuf;
    PrivateHandle priv_handle;

    int err = getPrivateHandle(dst_buf.handle, &priv_handle);
    if (0 != err)
    {
        display->setRetireFenceFd(-1, display->isConnected());
        BLOGE(job->disp_ori_id, "SetBlack Get priv_handle fail");
        return;
    }

    bool is_sec = isSecure(&priv_handle);
    if (HWC_DISPLAY_VIRTUAL == job->disp_ori_id)
    {
        if (is_sec)
        {
            display->setRetireFenceFd(::dup(job->fill_black[1].fence), display->isConnected());
        }
        else
        {
            display->setRetireFenceFd(::dup(job->fill_black[0].fence), display->isConnected());
        }
    }
}

bool AsyncBliterHandler::bypassBlit(HWLayer* hw_layer, int ovl_in)
{
    hwc_layer_1_t* layer = &hw_layer->layer;
    int pool_id = hw_layer->priv_handle.ext_info.pool_id;

    // there's no queued frame and current frame is not dirty
    if (hw_layer->dirty)
    {
        BLOGD(ovl_in, "BLT/async=curr/pool=%d/rel=%d(%d)/acq=%d/handle=%p",
            pool_id, layer->releaseFenceFd, hw_layer->sync_marker,
            layer->acquireFenceFd, layer->handle);

        return false;
    }

    HWC_ATRACE_NAME("bypass");
    BLOGD(ovl_in, "BLT/async=nop/pool=%d/handle=%p/fence=%d", pool_id, layer->handle, layer->releaseFenceFd);

    return true;
}

sp<DisplayBufferQueue> AsyncBliterHandler::getDisplayBufferQueue(PrivateHandle* priv_handle,
      BufferConfig* config, int ovl_in, const DisplayData* disp_data, const int& assigned_format) const
{
    sp<DisplayBufferQueue> queue = m_ovl_engine->getInputQueue(ovl_in);
    if (queue == NULL)
    {
        queue = new DisplayBufferQueue(DisplayBufferQueue::QUEUE_TYPE_BLT,
                                       DisplayBufferQueue::NUM_BUFFER_SLOTS);
        queue->setSynchronousMode(false);

        // connect to OverlayEngine
        m_ovl_engine->setInputQueue(ovl_in, queue);
    }

    int format = (assigned_format != -1 ? assigned_format : priv_handle->format);
    if (disp_data->subtype == HWC_DISPLAY_EPAPER)
    {
        format = HAL_PIXEL_FORMAT_RGBA_8888;
    }
    int bpp = getBitsPerPixel(convertFormat4Bliter(format));

    uint32_t buffer_w = ALIGN_CEIL(disp_data->width, 2);
    uint32_t buffer_h = ALIGN_CEIL(disp_data->height, 2);

    // set buffer format to buffer queue
    // TODO: should refer to layer->displayFrame
    DisplayBufferQueue::BufferParam buffer_param;
    buffer_param.disp_id = m_disp_id;
    buffer_param.pool_id = priv_handle->ext_info.pool_id;
    buffer_param.width   = buffer_w;
    buffer_param.height  = buffer_h;
    buffer_param.pitch   = buffer_w;
    buffer_param.format  = convertFormat4Bliter(format);
    // TODO: should calculate the size from the information of gralloc?
    buffer_param.size    = (buffer_w * buffer_h * bpp / 8);
    buffer_param.protect = (priv_handle->usage & GRALLOC_USAGE_PROTECTED);
    if (disp_data->subtype == HWC_DISPLAY_EPAPER)
    {
        buffer_param.sw_usage = true;
    }
    queue->setBufferParam(buffer_param);

    // TODO: should refer to layer->displayFrame
    config->dst_width = buffer_w;
    config->dst_height = buffer_h;
    config->dst_pitch = buffer_w * bpp / 8;
    config->dst_pitch_uv = 0;

    return queue;
}

status_t AsyncBliterHandler::setDpConfig(
    PrivateHandle* priv_handle, BufferConfig* config, int ovl_in, const int& assigned_output_format)
{
    // check if private color format is changed
    bool private_format_change = false;
    int curr_private_format = NOT_PRIVATE_FORMAT;
    if (HAL_PIXEL_FORMAT_YUV_PRIVATE == priv_handle->format ||
        HAL_PIXEL_FORMAT_YCbCr_420_888 == priv_handle->format ||
        HAL_PIXEL_FORMAT_YUV_PRIVATE_10BIT == priv_handle->format)
    {
        curr_private_format = (priv_handle->ext_info.status & GRALLOC_EXTRA_MASK_CM);
        if (HAL_PIXEL_FORMAT_YUV_PRIVATE == config->gralloc_format ||
            HAL_PIXEL_FORMAT_YCbCr_420_888 == config->gralloc_format ||
            HAL_PIXEL_FORMAT_YUV_PRIVATE_10BIT == config->gralloc_format)
        {
            private_format_change = (config->gralloc_private_format != curr_private_format);
        }
    }

    bool ufo_align_change = false;
    int curr_ufo_align = 0;
    if ((HAL_PIXEL_FORMAT_UFO == priv_handle->format || HAL_PIXEL_FORMAT_UFO_AUO == priv_handle->format) ||
        GRALLOC_EXTRA_BIT_CM_UFO == curr_private_format)
    {
        curr_ufo_align = ((priv_handle->ext_info.status & GRALLOC_EXTRA_MASK_UFO_ALIGN) >> 2);
        ufo_align_change = (config->gralloc_ufo_align_type != curr_ufo_align);
    }

    if (config->gralloc_prexform == priv_handle->prexform &&
        config->gralloc_width  == priv_handle->width  &&
        config->gralloc_height == priv_handle->height &&
        config->gralloc_stride  == priv_handle->y_stride &&
        config->gralloc_vertical_stride == priv_handle->vstride &&
        config->gralloc_cbcr_align == priv_handle->cbcr_align &&
        config->gralloc_format == priv_handle->format &&
        config->assigned_output_format == assigned_output_format &&
        private_format_change  == false &&
        ufo_align_change == false)
    {
        // data format is not changed
        if (!config->is_valid)
        {
            BLOGW(ovl_in, "Format is not changed, but config in invalid !");
            return -EINVAL;
        }

        return NO_ERROR;
    }

    BLOGD(ovl_in, "Format Change (w=%d h=%d s:%d vs:%d f=0x%x) ->\
                   (w=%d h=%d s:%d vs:%d f=0x%x af=0x%x pf=0x%x ua=%d)",
        config->gralloc_width, config->gralloc_height, config->gralloc_stride,
        config->gralloc_vertical_stride, config->gralloc_format,
        priv_handle->width, priv_handle->height, priv_handle->y_stride,
        priv_handle->vstride, priv_handle->format, assigned_output_format,
        curr_private_format, curr_ufo_align);

    // remember current buffer data format for next comparison
    config->gralloc_prexform = priv_handle->prexform;
    config->gralloc_width  = priv_handle->width;
    config->gralloc_height = priv_handle->height;
    config->gralloc_stride = priv_handle->y_stride;
    config->gralloc_cbcr_align = priv_handle->cbcr_align;
    config->gralloc_vertical_stride = priv_handle->vstride;
    config->gralloc_format = priv_handle->format;
    config->gralloc_private_format = curr_private_format;
    config->gralloc_ufo_align_type = curr_ufo_align;
    config->assigned_output_format = assigned_output_format;

    //
    // set DpFramework configuration
    //
    unsigned int src_size_luma = 0;
    unsigned int src_size_chroma = 0;

    unsigned int width  = priv_handle->width;
    unsigned int height = priv_handle->height;
    unsigned int y_stride = priv_handle->y_stride;
    unsigned int vertical_stride = priv_handle->vstride;

    // reset uv pitch since RGB does not need it
    config->src_pitch_uv = 0;

    unsigned int input_format = grallocColor2HalColor(priv_handle->format, curr_private_format);
    if (input_format == 0)
    {
        BLOGE(ovl_in, "Private Format is invalid (0x%x)", curr_private_format);
        memset(config, 0, sizeof(BufferConfig));
        return -EINVAL;
    }

    // remember real height since height should be aligned with 32 for NV12_BLK
    config->src_width = y_stride;
    config->src_height = height;
    config->deinterlace = false;

    // get color range configuration
    config->gralloc_color_range =
        (priv_handle->ext_info.status & GRALLOC_EXTRA_MASK_YUV_COLORSPACE);

    int assigned_dst_dp_format = -1;
    switch (assigned_output_format)
    {
        case HAL_PIXEL_FORMAT_YUYV:
            assigned_dst_dp_format = DP_COLOR_YUYV;
            break;
    }

    switch (input_format)
    {
        case HAL_PIXEL_FORMAT_RGBA_8888:
            config->src_pitch = y_stride * 4;
            config->src_plane = 1;
            config->src_size[0] = config->src_pitch * height;
            config->src_dpformat = DP_COLOR_RGBA8888;
            config->dst_dpformat = assigned_dst_dp_format != -1 ?
                                   static_cast<DP_COLOR_ENUM>(assigned_dst_dp_format) :
                                   mapDpFormat(convertFormat4Bliter(input_format));
            config->dst_plane = 1;
            config->gralloc_color_range = config->gralloc_color_range != 0 ?
                config->gralloc_color_range : GRALLOC_EXTRA_BIT_YUV_BT601_FULL;
            break;

        case HAL_PIXEL_FORMAT_RGBX_8888:
            config->src_pitch = y_stride * 4;
            config->src_plane = 1;
            config->src_size[0] = config->src_pitch * height;
            config->src_dpformat = DP_COLOR_RGBA8888;
            config->dst_dpformat = assigned_dst_dp_format != -1 ?
                                   static_cast<DP_COLOR_ENUM>(assigned_dst_dp_format) :
                                   mapDpFormat(convertFormat4Bliter(input_format));
            config->dst_plane = 1;
            config->gralloc_color_range = config->gralloc_color_range != 0 ?
                config->gralloc_color_range : GRALLOC_EXTRA_BIT_YUV_BT601_FULL;
            break;

        case HAL_PIXEL_FORMAT_BGRA_8888:
            config->src_pitch = y_stride * 4;
            config->src_plane = 1;
            config->src_size[0] = config->src_pitch * height;
            config->src_dpformat = DP_COLOR_BGRA8888;
            config->dst_dpformat = mapDpFormat(convertFormat4Bliter(input_format));
            config->dst_plane    = 1;
            config->gralloc_color_range = config->gralloc_color_range != 0 ?
                config->gralloc_color_range : GRALLOC_EXTRA_BIT_YUV_BT601_FULL;
            break;

        case HAL_PIXEL_FORMAT_BGRX_8888:
        case HAL_PIXEL_FORMAT_IMG1_BGRX_8888:
            config->src_pitch = y_stride * 4;
            config->src_plane = 1;
            config->src_size[0] = config->src_pitch * height;
            config->src_dpformat = DP_COLOR_BGRA8888;
            config->dst_dpformat = mapDpFormat(convertFormat4Bliter(input_format));
            config->dst_plane    = 1;
            config->gralloc_color_range = config->gralloc_color_range != 0 ?
                config->gralloc_color_range : GRALLOC_EXTRA_BIT_YUV_BT601_FULL;
            break;

        case HAL_PIXEL_FORMAT_RGBA_1010102:
            config->src_pitch = y_stride * 4;
            config->src_plane = 1;
            config->src_size[0] = config->src_pitch * height;
            config->src_dpformat = DP_COLOR_RGBA1010102;
            config->dst_dpformat = mapDpFormat(convertFormat4Bliter(input_format));
            config->dst_plane    = 1;
            config->gralloc_color_range = config->gralloc_color_range != 0 ?
                config->gralloc_color_range : GRALLOC_EXTRA_BIT_YUV_BT601_FULL;
            break;

        case HAL_PIXEL_FORMAT_RGB_888:
            config->src_pitch = y_stride * 3;
            config->src_plane = 1;
            config->src_size[0] = config->src_pitch * height;
            config->src_dpformat = DP_COLOR_RGB888;
            config->dst_dpformat = mapDpFormat(convertFormat4Bliter(input_format));
            config->dst_plane    = 1;
            config->gralloc_color_range = config->gralloc_color_range != 0 ?
                config->gralloc_color_range : GRALLOC_EXTRA_BIT_YUV_BT601_FULL;
            break;

        case HAL_PIXEL_FORMAT_RGB_565:
            config->src_pitch = y_stride * 2;
            config->src_plane = 1;
            config->src_size[0] = config->src_pitch * height;
            config->src_dpformat = DP_COLOR_RGB565;
            config->dst_dpformat = mapDpFormat(convertFormat4Bliter(input_format));
            config->dst_plane    = 1;
            config->gralloc_color_range = config->gralloc_color_range != 0 ?
                config->gralloc_color_range : GRALLOC_EXTRA_BIT_YUV_BT601_FULL;
            break;

        case HAL_PIXEL_FORMAT_NV12_BLK_FCM:
            config->src_pitch    = y_stride * 32;
            config->src_pitch_uv = ALIGN_CEIL((y_stride / 2), priv_handle->cbcr_align ? priv_handle->cbcr_align : 16) * 2 * 16;
            config->src_plane = 2;
            config->src_height = vertical_stride;
            src_size_luma = y_stride * vertical_stride;
            config->src_size[0] = src_size_luma;
            config->src_size[1] = src_size_luma / 4 * 2;
            config->src_dpformat = DP_COLOR_420_BLKI;
            config->dst_dpformat = mapDpFormat(convertFormat4Bliter(input_format));
            config->dst_plane    = 1;
            break;

        case HAL_PIXEL_FORMAT_NV12_BLK:
            config->src_pitch    = y_stride * 32;
            config->src_pitch_uv = ALIGN_CEIL((y_stride / 2), priv_handle->cbcr_align ? priv_handle->cbcr_align : 16) * 2 * 16;
            config->src_plane = 2;
            config->src_height = vertical_stride;
            src_size_luma = y_stride * vertical_stride;
            config->src_size[0] = src_size_luma;
            config->src_size[1] = src_size_luma / 4 * 2;
            config->src_dpformat = DP_COLOR_420_BLKP;
            config->dst_dpformat = mapDpFormat(convertFormat4Bliter(input_format));
            config->dst_plane    = 1;
            break;

        case HAL_PIXEL_FORMAT_I420:
            config->src_pitch    = y_stride;
            config->src_pitch_uv = ALIGN_CEIL((y_stride / 2), priv_handle->cbcr_align ? priv_handle->cbcr_align : 1);
            config->src_plane = 3;
            src_size_luma = y_stride * vertical_stride;
            src_size_chroma = config->src_pitch_uv * vertical_stride / 2;
            config->src_size[0] = src_size_luma;
            config->src_size[1] = src_size_chroma;
            config->src_size[2] = src_size_chroma;
            config->src_dpformat = DP_COLOR_I420;
            config->dst_dpformat = mapDpFormat(convertFormat4Bliter(input_format));
            config->dst_plane    = 1;
            break;

        case HAL_PIXEL_FORMAT_I420_DI:
            config->src_pitch    = y_stride * 2;
            config->src_pitch_uv = ALIGN_CEIL(y_stride, priv_handle->cbcr_align ? priv_handle->cbcr_align : 1);
            config->src_plane = 3;
            src_size_luma = y_stride * height;
            src_size_chroma = src_size_luma / 4;
            config->src_size[0] = src_size_luma;
            config->src_size[1] = src_size_chroma;
            config->src_size[2] = src_size_chroma;
            config->src_dpformat = DP_COLOR_I420;
            config->dst_dpformat = mapDpFormat(convertFormat4Bliter(input_format));
            config->dst_plane    = 1;
            config->deinterlace = true;
            break;

        case HAL_PIXEL_FORMAT_YV12:
            config->src_pitch    = y_stride;
            config->src_pitch_uv = ALIGN_CEIL((y_stride / 2), priv_handle->cbcr_align ? priv_handle->cbcr_align : 16);
            config->src_plane = 3;
            src_size_luma = y_stride * ALIGN_CEIL(height, priv_handle->v_align ? priv_handle->v_align : 2);
            src_size_chroma = config->src_pitch_uv * ALIGN_CEIL(height, priv_handle->v_align ? priv_handle->v_align : 2) / 2;
            config->src_size[0] = src_size_luma;
            config->src_size[1] = src_size_chroma;
            config->src_size[2] = src_size_chroma;
            config->src_dpformat = DP_COLOR_YV12;
            config->dst_dpformat = mapDpFormat(convertFormat4Bliter(input_format));
            config->dst_plane    = 1;
            break;

        case HAL_PIXEL_FORMAT_YV12_DI:
            config->src_pitch    = y_stride;
            config->src_pitch_uv = ALIGN_CEIL(y_stride, priv_handle->cbcr_align ? priv_handle->cbcr_align : 16);
            config->src_plane = 3;
            src_size_luma = y_stride * height;
            src_size_chroma = src_size_luma / 4;
            config->src_size[0] = src_size_luma;
            config->src_size[1] = src_size_chroma;
            config->src_size[2] = src_size_chroma;
            config->src_dpformat = DP_COLOR_YV12;
            config->dst_dpformat = mapDpFormat(convertFormat4Bliter(input_format));
            config->dst_plane    = 1;
            config->deinterlace = true;
            break;

        case HAL_PIXEL_FORMAT_YUYV:
        case HAL_PIXEL_FORMAT_YCbCr_422_I:
            config->src_pitch    = y_stride * 2;
            config->src_pitch_uv = 0;
            config->src_plane = 1;
            config->src_size[0] = y_stride * height * 2;
            config->src_dpformat = DP_COLOR_YUYV;
            config->dst_dpformat = mapDpFormat(convertFormat4Bliter(input_format));
            config->dst_plane    = 1;
            break;

        case HAL_PIXEL_FORMAT_UFO:
        case HAL_PIXEL_FORMAT_UFO_AUO:
            width = y_stride;
            height = vertical_stride;
            config->src_height   = height;
            config->src_pitch    = width * 32;
            config->src_pitch_uv = width * 16;
            config->src_plane = 2;
            // calculate PIC_SIZE_Y, need align 512
            src_size_luma = ALIGN_CEIL(width * height, 512);
            config->src_size[0] = src_size_luma;
            config->src_size[1] = src_size_luma;
            config->src_dpformat = (input_format == HAL_PIXEL_FORMAT_UFO) ? DP_COLOR_420_BLKP_UFO : DP_COLOR_420_BLKP_UFO_AUO;
            config->dst_dpformat = mapDpFormat(convertFormat4Bliter(input_format));
            config->dst_plane    = 1;
            break;

        case HAL_PIXEL_FORMAT_NV12:
            config->src_pitch    = y_stride;
            config->src_pitch_uv = ALIGN_CEIL((y_stride), priv_handle->cbcr_align ? priv_handle->cbcr_align : 1);
            config->src_plane = 2;
            src_size_luma = y_stride * vertical_stride;
            src_size_chroma = config->src_pitch_uv * vertical_stride / 2;
            config->src_size[0] = src_size_luma;
            config->src_size[1] = src_size_chroma;
            config->src_dpformat = DP_COLOR_NV12;
            config->dst_dpformat = mapDpFormat(convertFormat4Bliter(input_format));
            config->dst_plane    = 1;
            break;

        case HAL_PIXEL_FORMAT_YCRCB_420_SP:
            config->src_pitch    = y_stride;
            config->src_pitch_uv = ALIGN_CEIL((y_stride), priv_handle->cbcr_align ? priv_handle->cbcr_align : 1);
            config->src_plane = 2;
            src_size_luma = y_stride * vertical_stride;
            src_size_chroma = config->src_pitch_uv * vertical_stride / 2;
            config->src_size[0] = src_size_luma;
            config->src_size[1] = src_size_chroma;
            config->src_dpformat = DP_COLOR_NV21;
            config->dst_dpformat = mapDpFormat(convertFormat4Bliter(input_format));
            config->dst_plane    = 1;
            break;

        case HAL_PIXEL_FORMAT_NV12_BLK_10BIT_H:
        case HAL_PIXEL_FORMAT_NV12_BLK_10BIT_H_JUMP:
            config->src_pitch    = y_stride * 40;
            config->src_pitch_uv = ALIGN_CEIL((y_stride / 2), priv_handle->cbcr_align ? priv_handle->cbcr_align : 20) * 2 * 20;
            config->src_plane = 2;
            config->src_height = vertical_stride;
            src_size_luma = ALIGN_CEIL(y_stride * vertical_stride * 5 / 4, 40);
            // Because the start address of chroma has to be a multiple of 512, we align luma size
            // with 512 to adjust chroma address.
            config->src_size[0] = ALIGN_CEIL(src_size_luma, 512);
            config->src_size[1] = src_size_luma / 4 * 2;
            config->src_dpformat = (input_format == HAL_PIXEL_FORMAT_NV12_BLK_10BIT_H) ? DP_COLOR_420_BLKP_10_H : DP_COLOR_420_BLKP_10_H_JUMP;
            config->dst_dpformat = mapDpFormat(convertFormat4Bliter(input_format));
            config->dst_plane    = 1;
            break;

        case HAL_PIXEL_FORMAT_NV12_BLK_10BIT_V:
        case HAL_PIXEL_FORMAT_NV12_BLK_10BIT_V_JUMP:
            config->src_pitch    = y_stride * 40;
            config->src_pitch_uv = ALIGN_CEIL((y_stride / 2), priv_handle->cbcr_align ? priv_handle->cbcr_align : 20) * 2 * 20;
            config->src_plane = 2;
            config->src_height = vertical_stride;
            src_size_luma = ALIGN_CEIL(y_stride * vertical_stride * 5 / 4, 40);
            // Because the start address of chroma has to be a multiple of 512, we align luma size
            // with 512 to adjust chroma address.
            config->src_size[0] = ALIGN_CEIL(src_size_luma, 512);
            config->src_size[1] = src_size_luma / 4 * 2;
            config->src_dpformat = (input_format == HAL_PIXEL_FORMAT_NV12_BLK_10BIT_V) ? DP_COLOR_420_BLKP_10_V : DP_COLOR_420_BLKP_10_V_JUMP;
            config->dst_dpformat = mapDpFormat(convertFormat4Bliter(input_format));
            config->dst_plane    = 1;
            break;

        case HAL_PIXEL_FORMAT_UFO_10BIT_H:
        case HAL_PIXEL_FORMAT_UFO_10BIT_H_JUMP:
            width = y_stride;
            height = vertical_stride;
            config->src_height   = height;
            config->src_pitch    = width * 40;
            config->src_pitch_uv = width * 20;
            config->src_plane = 2;
            src_size_luma = ALIGN_CEIL(width * height * 5 / 4, 4096);
            config->src_size[0] = src_size_luma;
            config->src_size[1] = src_size_luma;
            config->src_dpformat = (input_format == HAL_PIXEL_FORMAT_UFO_10BIT_H) ? DP_COLOR_420_BLKP_UFO_10_H : DP_COLOR_420_BLKP_UFO_10_H_JUMP;
            config->dst_dpformat = mapDpFormat(convertFormat4Bliter(input_format));
            config->dst_plane    = 1;
            break;

        case HAL_PIXEL_FORMAT_UFO_10BIT_V:
        case HAL_PIXEL_FORMAT_UFO_10BIT_V_JUMP:
            width = y_stride;
            height = vertical_stride;
            config->src_height   = height;
            config->src_pitch    = width * 40;
            config->src_pitch_uv = width * 20;
            config->src_plane = 2;
            src_size_luma = ALIGN_CEIL(width * height * 5 / 4, 4096);
            config->src_size[0] = src_size_luma;
            config->src_size[1] = src_size_luma;
            config->src_dpformat = (input_format == HAL_PIXEL_FORMAT_UFO_10BIT_V) ? DP_COLOR_420_BLKP_UFO_10_V : DP_COLOR_420_BLKP_UFO_10_V_JUMP;
            config->dst_dpformat = mapDpFormat(convertFormat4Bliter(input_format));
            config->dst_plane    = 1;
            break;

        default:
            BLOGE(ovl_in, "Color format for DP is invalid (0x%x)", input_format);
            //config->is_valid = false;
            memset(config, 0, sizeof(BufferConfig));
            return -EINVAL;
    }
    if (config->gralloc_color_range == 0) {
        BLOGD(ovl_in, "Color range is %#x, use default BT601_NARROW", config->gralloc_color_range);
        config->gralloc_color_range = GRALLOC_EXTRA_BIT_YUV_BT601_NARROW;
    }

    config->dst_size[0] = config->dst_pitch * config->dst_height;

    config->is_valid = true;
    return NO_ERROR;
}

status_t AsyncBliterHandler::setDstDpConfig(PrivateHandle& dst_priv_handle, BufferConfig* config)
{
    config->dst_width    = dst_priv_handle.width;
    config->dst_height   = dst_priv_handle.height;

    switch (dst_priv_handle.format)
    {
        case HAL_PIXEL_FORMAT_RGBA_8888:
            config->dst_dpformat = DP_COLOR_RGBA8888;
            config->dst_pitch    = dst_priv_handle.y_stride * 4;
            config->dst_pitch_uv = 0;
            config->dst_plane    = 1;
            config->dst_size[0]  = config->dst_pitch * config->dst_height;
            config->gralloc_color_range = GRALLOC_EXTRA_BIT_YUV_BT601_FULL;
            config->dst_dataspace = HAL_DATASPACE_V0_JFIF;
            // HAL_DATASPACE_V0_JFIF = ((STANDARD_BT601_625 | TRANSFER_SMPTE_170M) | RANGE_FULL)
            break;

        case HAL_PIXEL_FORMAT_RGBX_8888:
            config->dst_dpformat = DP_COLOR_RGBX8888;
            config->dst_pitch    = dst_priv_handle.y_stride * 4;
            config->dst_pitch_uv = 0;
            config->dst_plane    = 1;
            config->dst_size[0]  = config->dst_pitch * config->dst_height;
            config->gralloc_color_range = GRALLOC_EXTRA_BIT_YUV_BT601_FULL;
            config->dst_dataspace = HAL_DATASPACE_V0_JFIF;
            break;

        case HAL_PIXEL_FORMAT_YV12:
            config->dst_dpformat = DP_COLOR_YV12;
            config->dst_pitch    = dst_priv_handle.y_stride;
            config->dst_pitch_uv = ALIGN_CEIL((dst_priv_handle.y_stride / 2), 16);
            config->dst_plane    = 3;
            config->dst_size[0]  = config->dst_pitch * config->dst_height;
            config->dst_size[1]  = config->dst_pitch_uv * (config->dst_height / 2);
            config->dst_size[2]  = config->dst_size[1];

            // WORKAROUND: VENC only accpet BT601 limit range
            config->gralloc_color_range = GRALLOC_EXTRA_BIT_YUV_BT601_NARROW;
            config->dst_dataspace = HAL_DATASPACE_V0_BT601_625;
            break;

        case HAL_PIXEL_FORMAT_RGB_888:
            config->dst_dpformat = DP_COLOR_RGB888;
            config->dst_pitch    = dst_priv_handle.y_stride * 3;
            config->dst_pitch_uv = 0;
            config->dst_plane    = 1;
            config->dst_size[0]  = config->dst_pitch * config->dst_height;
            config->gralloc_color_range = GRALLOC_EXTRA_BIT_YUV_BT601_FULL;
            config->dst_dataspace = HAL_DATASPACE_V0_JFIF;
            break;

        case HAL_PIXEL_FORMAT_RGB_565:
            config->dst_dpformat = DP_COLOR_RGB565;
            config->dst_pitch    = dst_priv_handle.y_stride * 2;
            config->dst_pitch_uv = 0;
            config->dst_plane    = 1;
            config->dst_size[0]  = config->dst_pitch * config->dst_height;
            config->gralloc_color_range = GRALLOC_EXTRA_BIT_YUV_BT601_FULL;
            config->dst_dataspace = HAL_DATASPACE_V0_JFIF;
            break;

        case HAL_PIXEL_FORMAT_YUYV:
        case HAL_PIXEL_FORMAT_YCbCr_422_I:
            config->dst_dpformat = DP_COLOR_YUYV;
            config->dst_pitch    = dst_priv_handle.y_stride * 2;
            config->dst_pitch_uv = 0;
            config->dst_plane    = 1;
            config->dst_size[0]  = config->dst_pitch * config->dst_height;
            config->gralloc_color_range = GRALLOC_EXTRA_BIT_YUV_BT601_FULL;
            config->dst_dataspace = HAL_DATASPACE_V0_JFIF;
            break;

        case HAL_PIXEL_FORMAT_YCRCB_420_SP:
            config->dst_dpformat = DP_COLOR_NV21;
            config->dst_pitch    = dst_priv_handle.y_stride;
            config->dst_pitch_uv = ALIGN_CEIL((dst_priv_handle.y_stride),
                                   dst_priv_handle.cbcr_align ? dst_priv_handle.cbcr_align : 1);
            config->dst_plane    = 2;
            config->dst_size[0]  = dst_priv_handle.y_stride * ALIGN_CEIL(dst_priv_handle.height, 2);
            config->dst_size[1]  = config->dst_pitch_uv * ALIGN_CEIL(dst_priv_handle.height, 2) / 2;
            config->gralloc_color_range = GRALLOC_EXTRA_BIT_YUV_BT601_FULL;
            break;

        default:
            config->dst_size[0]  = config->dst_pitch * config->dst_height;
            HWC_LOGW("setDstDpConfig format(0x%x) unexpected", dst_priv_handle.format);
            return -EINVAL;
    }

    return NO_ERROR;
}

void AsyncBliterHandler::process(DispatcherJob* job)
{
    if (HWC_MIRROR_SOURCE_INVALID != job->disp_mir_id)
    {
        if (HWC_DISPLAY_VIRTUAL == job->disp_ori_id)
        {
            processVirMirror(job);
        }
        else
        {
            processPhyMirror(job);
        }
        return;
    }

    if (job->is_black_job)
    {
        if (HWC_DISPLAY_VIRTUAL == job->disp_ori_id)
        {
            processVirBlack(job);
        }
        return;
    }

    uint32_t total_num = job->num_layers;

    for (uint32_t i = 0; i < total_num; i++)
    {
        HWLayer* hw_layer = &job->hw_layers[i];

        // skip non mm layers
        if (HWC_LAYER_TYPE_MM != hw_layer->type && HWC_LAYER_TYPE_MM_FBT != hw_layer->type) continue;

        // this layer is not enable
        if (!hw_layer->enable) continue;

        // this layer is not dirty and there is no latest frame could be used
        if (bypassBlit(hw_layer, i))
        {
            m_ovl_engine->updateSettingOfInput(i, hw_layer->ext_sel_layer);
            continue;
        }

        // in ULTRA scenario, set src buffer for master, and cancel mdp job for slave
        if (hw_layer->is_ultra_mdp)
        {
            if (IS_MASTER(m_disp_id))
                UltraBliter::getInstance().masterSetSrcBuf(hw_layer, job->sequence);
            else
                nullop(hw_layer->mdp_job_id);
        }

        hwc_layer_1_t* layer = &hw_layer->layer;
        BufferConfig* config = &m_dp_configs[i];
        DisplayBufferQueue::DisplayBuffer disp_buffer;

        int assigned_output_format = -1;
        uint32_t pq_enhance = 0;
        const bool is_game = hw_layer->need_pq || isGameHDR(&hw_layer->priv_handle);
        setPQEnhance(m_disp_id, hw_layer->priv_handle, &pq_enhance, &assigned_output_format, is_game);

        sp<DisplayBufferQueue> queue = getDisplayBufferQueue(&hw_layer->priv_handle, config, i,
            job->disp_data, assigned_output_format);

        config->src_dataspace = hw_layer->dataspace;

        // in BT2020 case, dst will be set as BT709
        config->dst_dataspace = getDstDataspace(hw_layer->dataspace);

        status_t err = setDpConfig(&hw_layer->priv_handle, config, i, assigned_output_format);

        if (NO_ERROR == err)
        {
            err = queue->dequeueBuffer(&disp_buffer, true, isSecure(&hw_layer->priv_handle));
        }

        if (NO_ERROR == err)
        {
            Rect src_roi = getFixedRect(layer->sourceCropf);
            Rect dst_roi = hw_layer->mdp_dst_roi;
            uint32_t xform = layer->transform;

            // rectify with preXform before all configuration
            rectifyRectWithPrexform(&src_roi, &hw_layer->priv_handle);
            rectifyXformWithPrexform(&xform, hw_layer->priv_handle.prexform);

            BliterNode::Parameter param = {src_roi, dst_roi, config, xform, pq_enhance};
            Rect mdp_cal_dst_crop;
            if (hw_layer->is_ultra_mdp)
            {
                UltraBliter& ublt = UltraBliter::getInstance();
                bool is_master = IS_MASTER(m_disp_id);
                ublt.setDstBuf(is_master, &disp_buffer);
                ublt.config(is_master, &param);
                err = ublt.trig(is_master, UltraBliter::DoJob, &disp_buffer.acquire_fence);

                mdp_cal_dst_crop = dst_roi;
                mdp_cal_dst_crop.offsetTo(ALIGN_FLOOR(mdp_cal_dst_crop.left, 2), mdp_cal_dst_crop.top);
                if (disp_buffer.data_format == HAL_PIXEL_FORMAT_YUYV &&
                        WIDTH(mdp_cal_dst_crop) & 0x1)
                {
                    mdp_cal_dst_crop.right = ALIGN_FLOOR(mdp_cal_dst_crop.right, 2);
                }
            }
            else
            {
                WDT_BL_NODE(setSrc, config, hw_layer->priv_handle, &layer->acquireFenceFd, is_game);
                WDT_BL_NODE(setDst, 0, &param, disp_buffer.out_ion_fd,
                    disp_buffer.out_sec_handle,
                    &disp_buffer.release_fence);
                WDT_BL_NODE(invalidate, hw_layer->mdp_job_id, &mdp_cal_dst_crop, job->sequence);

                passFenceFd(&disp_buffer.acquire_fence, &layer->releaseFenceFd);

                if (Platform::getInstance().m_config.is_support_mdp_pmqos_debug)
                {
                    MDPFrameInfoDebugger::getInstance().setJobAcquireFenceFd(job->sequence, ::dup(disp_buffer.acquire_fence));
                }
            }

            disp_buffer.data_info.src_crop   = mdp_cal_dst_crop;
            disp_buffer.data_info.dst_crop.left   = layer->displayFrame.left;
            disp_buffer.data_info.dst_crop.top    = layer->displayFrame.top;
            disp_buffer.data_info.dst_crop.right  = layer->displayFrame.right;
            disp_buffer.data_info.dst_crop.bottom = layer->displayFrame.bottom;

            disp_buffer.data_info.is_sharpen = false;
            disp_buffer.alpha_enable         = 1;
            disp_buffer.alpha                = layer->planeAlpha;
            disp_buffer.blending             = layer->blending;
            disp_buffer.src_handle           = layer->handle;
            disp_buffer.data_color_range     = config->gralloc_color_range;
            disp_buffer.dataspace            = config->dst_dataspace;
            disp_buffer.ext_sel_layer        = hw_layer->ext_sel_layer;
            disp_buffer.sequence = job->sequence;

            queue->queueBuffer(&disp_buffer);
        }
        else
        {
            if (hw_layer->is_ultra_mdp)
            {
                UltraBliter::getInstance().trig(IS_MASTER(m_disp_id), UltraBliter::CancelJob);
            }
            else
            {
                WDT_BL_NODE(cancelJob, hw_layer->mdp_job_id);
            }

            BLOGE(i, "something wrong, cancel mdp job ...");
        }

        if (hw_layer->priv_handle.ion_fd > 0)
        {
            IONDevice::getInstance().ionClose(hw_layer->priv_handle.ion_fd);
        }

        closeFenceFd(&layer->acquireFenceFd);
        closeFenceFd(&layer->releaseFenceFd);
    }
}

void AsyncBliterHandler::nullop()
{
}

void AsyncBliterHandler::nullop(const int32_t& job_id)
{
    WDT_BL_NODE(cancelJob, job_id);
}

void AsyncBliterHandler::boost()
{
}

void AsyncBliterHandler::cancelLayers(DispatcherJob* job)
{
    // cancel mm layers for dropping job
    if (HWC_MIRROR_SOURCE_INVALID != job->disp_mir_id)
    {
        cancelMirror(job);
        return;
    }

    uint32_t total_num = job->num_layers;

    for (uint32_t i = 0; i < total_num; i++)
    {
        HWLayer* hw_layer = &job->hw_layers[i];

        // skip non mm layers
        if (HWC_LAYER_TYPE_MM != hw_layer->type && HWC_LAYER_TYPE_MM_FBT != hw_layer->type) continue;

        // this layer is not enable
        if (!hw_layer->enable) continue;

        if (bypassBlit(hw_layer, i)) continue;

        hwc_layer_1_t* layer = &hw_layer->layer;

        BLOGD(i, "CANCEL/rel=%d/acq=%d/handle=%p",
            layer->releaseFenceFd, layer->acquireFenceFd, layer->handle);

        if (DisplayManager::m_profile_level & PROFILE_BLT)
        {
            char atrace_tag[256];
            sprintf(atrace_tag, "mm cancel:%p", layer->handle);
            HWC_ATRACE_NAME(atrace_tag);
        }

        if (hw_layer->is_ultra_mdp)
        {
            UltraBliter& ublt = UltraBliter::getInstance();

            bool is_master = IS_MASTER(m_disp_id);

            if (is_master)
                ublt.masterSetSrcBuf(hw_layer, job->sequence);
            else
                nullop(hw_layer->mdp_job_id);

            ublt.trig(is_master, UltraBliter::CancelJob);
        }
        else
        {
            if (layer->acquireFenceFd != -1) ::protectedClose(layer->acquireFenceFd);
            layer->acquireFenceFd = -1;

            if (hw_layer->mdp_job_id != 0) {
                nullop(hw_layer->mdp_job_id);
            }
            protectedClose(layer->releaseFenceFd);
            layer->releaseFenceFd = -1;
        }

        if (hw_layer->priv_handle.ion_fd > 0)
        {
            IONDevice::getInstance().ionClose(hw_layer->priv_handle.ion_fd);
        }
    }
}

void AsyncBliterHandler::cancelMirror(DispatcherJob* job)
{
    // cancel mirror path output buffer for dropping job
    HWBuffer* hw_mirbuf = &job->hw_mirbuf;
    if (-1 != hw_mirbuf->mir_in_rel_fence_fd)
    {
        protectedClose(hw_mirbuf->mir_in_rel_fence_fd);
        hw_mirbuf->mir_in_rel_fence_fd = -1;
    }

    nullop(job->mdp_job_output_buffer);
    clearMdpJob(job->fill_black);

    if (hw_mirbuf->mir_in_acq_fence_fd != -1) ::protectedClose(hw_mirbuf->mir_in_acq_fence_fd);
    hw_mirbuf->mir_in_acq_fence_fd = -1;
}

static int transform_table[4][4] =
{
    { 0x00, 0x07, 0x03, 0x04 },
    { 0x04, 0x00, 0x07, 0x03 },
    { 0x03, 0x04, 0x00, 0x07 },
    { 0x07, 0x03, 0x04, 0x00 }
};


void AsyncBliterHandler::calculateMirRoiXform(
                uint32_t* xform, Rect* src_roi, Rect* dst_roi, DispatcherJob* job)
{
    // ROT   0 = 000
    // ROT  90 = 100
    // ROT 180 = 011
    // ROT 270 = 111
    // count num of set bit as index for transform_table
    int ori_rot = job->disp_ori_rot;
    ori_rot = (ori_rot & 0x1) + ((ori_rot>>1) & 0x1) + ((ori_rot>>2) & 0x1);
    int mir_rot = job->disp_mir_rot;
    mir_rot = (mir_rot & 0x1) + ((mir_rot>>1) & 0x1) + ((mir_rot>>2) & 0x1);

    const DisplayData* ori_disp_data = job->disp_data;
    const DisplayData* mir_disp_data = job->mir_disp_data;

    // correct ori_disp transform with its hwrotation
    if (0 != ori_disp_data->hwrotation)
    {
        ori_rot = (ori_rot + ori_disp_data->hwrotation) % 4;
    }

    // correct ori_disp trasform with display driver's cap: is_output_rotated
    if (1 == HWCMediator::getInstance().getOvlDevice(job->disp_mir_id)->getDisplayOutputRotated())
    {
        ori_rot = (ori_rot + 2) % 4;
    }

    // correct mir_disp transform with its hwrotation
    if (0 != mir_disp_data->hwrotation)
    {
        mir_rot = (mir_rot + mir_disp_data->hwrotation) % 4;
    }

    *xform = transform_table[ori_rot][mir_rot];

    int rect_sel = abs(ori_rot - mir_rot);
    if (rect_sel & 0x1)
    {
        *src_roi = mir_disp_data->mir_landscape;
        *dst_roi = ori_disp_data->mir_landscape;
    }
    else
    {
        *src_roi = mir_disp_data->mir_portrait;
        *dst_roi = ori_disp_data->mir_portrait;
    }
}

void AsyncBliterHandler::processPhyMirror(DispatcherJob* job)
{
    HWBuffer& src_buf = job->hw_mirbuf;
    HWBuffer& dst_buf = job->hw_outbuf;

    PrivateHandle& src_priv_handle = src_buf.priv_handle;
    PrivateHandle& dst_priv_handle = dst_buf.priv_handle;

    BufferConfig* config = &m_dp_configs[m_ovl_engine->getMaxInputNum()];
    DisplayBufferQueue::DisplayBuffer disp_buffer;

    // some platforms rdma not support roi_update
    bool full_screen_update = (0 == Platform::getInstance().m_config.rdma_roi_update) &&
                              (1 == job->num_layers);

    // get display buffer queue
    sp<DisplayBufferQueue> queue =
        getDisplayBufferQueue(&dst_priv_handle, config, 0, job->disp_data);

    config->src_dataspace = src_buf.dataspace;
    config->dst_dataspace = dst_buf.dataspace;

    status_t err = setDpConfig(&src_priv_handle, config, 0);

    if (NO_ERROR == err)
    {
        err = queue->dequeueBuffer(&disp_buffer, true);
    }

    if (NO_ERROR == err)
    {
        Rect src_roi;
        Rect dst_roi;
        uint32_t xform;

        calculateMirRoiXform(&xform, &src_roi, &dst_roi, job);

        config->gralloc_color_range = GRALLOC_EXTRA_BIT_YUV_BT601_FULL;

        if (full_screen_update)
        {
            clearBackground(disp_buffer.out_handle,
                            &dst_roi,
                            &disp_buffer.release_fence,
                            job->fill_black);
        }

        BliterNode::Parameter param = {src_roi, dst_roi, config, xform, false};
        WDT_BL_NODE(setSrc, config, src_priv_handle, &src_buf.mir_in_acq_fence_fd);
        WDT_BL_NODE(setDst, 0, &param, disp_buffer.out_ion_fd,
                disp_buffer.out_sec_handle,
                &disp_buffer.release_fence);
        WDT_BL_NODE(invalidate, job->mdp_job_output_buffer);

        passFenceFd(&disp_buffer.acquire_fence, &src_buf.mir_in_rel_fence_fd);
        if (full_screen_update)
        {
            int dst_crop_x = 0;
            int dst_crop_y = 0;
            int dst_crop_w = job->disp_data->width;
            int dst_crop_h = job->disp_data->height;
            Rect base_crop(Rect(0, 0, dst_crop_w, dst_crop_h));

            disp_buffer.data_info.src_crop   = base_crop;
            disp_buffer.data_info.dst_crop   = base_crop.offsetTo(dst_crop_x, dst_crop_y);
        }
        else
        {
            dst_roi.left = ALIGN_CEIL(dst_roi.left, 2);
            dst_roi.right = ALIGN_FLOOR(dst_roi.right, 2);
            disp_buffer.data_info.src_crop   = dst_roi;
            disp_buffer.data_info.dst_crop   = dst_roi;
        }
        disp_buffer.data_info.is_sharpen = false;
        disp_buffer.alpha_enable         = 1;
        disp_buffer.alpha                = 0xFF;
        disp_buffer.sequence             = job->sequence;
        disp_buffer.src_handle           = src_buf.handle;
        disp_buffer.data_color_range     = config->gralloc_color_range;

        queue->queueBuffer(&disp_buffer);
    }
    else
    {
        BLOGE(0, "Failed to dequeue mirror buffer...");
        WDT_BL_NODE(cancelJob, job->mdp_job_output_buffer);
    }

    closeFenceFd(&src_buf.mir_in_acq_fence_fd);
    closeFenceFd(&src_buf.mir_in_rel_fence_fd);
    clearMdpJob(job->fill_black);
}

void AsyncBliterHandler::processVirMirror(DispatcherJob* job)
{
    HWBuffer& src_buf = job->hw_mirbuf;
    HWBuffer& dst_buf = job->hw_outbuf;

    PrivateHandle& src_priv_handle = src_buf.priv_handle;
    PrivateHandle& dst_priv_handle = dst_buf.priv_handle;


    BufferConfig* config = &m_dp_configs[m_ovl_engine->getMaxInputNum()];
    config->src_dataspace = src_buf.dataspace;
    config->dst_dataspace = dst_buf.dataspace;

    status_t err = setDpConfig(&src_priv_handle, config, 0);

    if (NO_ERROR == err)
    {
        setDstDpConfig(dst_priv_handle, config);

        Rect src_roi;
        Rect dst_roi;
        uint32_t xform;

        calculateMirRoiXform(&xform, &src_roi, &dst_roi, job);

        if (dst_roi.left % 2)
        {
            --dst_roi.left;
            --dst_roi.right;
            if (dst_roi.left < 0)
            {
                ++dst_roi.left;
            }
        }

        if (WIDTH(dst_roi) == 0 || (WIDTH(dst_roi) % 2))
        {
            const DisplayData* mir_disp_data = job->disp_data;

            if (dst_roi.right + 1 <= static_cast<int32_t>(mir_disp_data->width))
                ++dst_roi.right;
            else
                --dst_roi.right;
        }

        clearBackground(dst_buf.handle,
            &dst_roi,
            &dst_buf.mir_in_rel_fence_fd,
            job->fill_black);

        BliterNode::Parameter param = {src_roi, dst_roi, config, xform, false};
        WDT_BL_NODE(setSrc, config, src_priv_handle, &src_buf.mir_in_acq_fence_fd);
        WDT_BL_NODE(setDst, 0, &param, dst_priv_handle.ion_fd,
                dst_priv_handle.sec_handle,
                &dst_buf.mir_in_rel_fence_fd);
        WDT_BL_NODE(invalidate, job->mdp_job_output_buffer);
    }
    else
    {
        BLOGE(0, "Failed to get mirror buffer info !!");
        WDT_BL_NODE(cancelJob, job->mdp_job_output_buffer);
    }

    closeFenceFd(&src_buf.mir_in_acq_fence_fd);
    closeFenceFd(&dst_buf.mir_in_rel_fence_fd);
    clearMdpJob(job->fill_black);
}

void AsyncBliterHandler::processVirBlack(DispatcherJob* job)
{
    HWBuffer& dst_buf = job->hw_outbuf;

    clearBackground(dst_buf.handle,
        nullptr,
        &dst_buf.mir_in_rel_fence_fd,
        job->fill_black);
    closeFenceFd(&dst_buf.mir_in_rel_fence_fd);
    clearMdpJob(job->fill_black);
}

int AsyncBliterHandler::dump(char* /*buff*/, int /*buff_len*/, int dump_level)
{
    if (dump_level & DUMP_SYNC)
        m_sync_fence->dump(-1);

    return 0;
}

void AsyncBliterHandler::processFillBlack(PrivateHandle* dst_priv_handle, int* fence, MdpJob &job)
{
    AutoMutex l(BlackBuffer::getInstance().m_lock);

    // get BlackBuffer handle
    buffer_handle_t src_handle = BlackBuffer::getInstance().getHandle();
    if (src_handle == 0)
    {
        HWC_LOGE("processFillBlack(BlackBuffer): get handle fail");
        return;
    }

    // check is_sec
    bool is_sec = isSecure(dst_priv_handle);
    if (is_sec)
    {
        BlackBuffer::getInstance().setSecure();
    }

    BufferConfig  config;
    memset(&config, 0, sizeof(BufferConfig));

    PrivateHandle src_priv_handle;
    status_t err = getPrivateHandle(src_handle, &src_priv_handle);

    if (NO_ERROR == err)
    {
        err = setDpConfig(&src_priv_handle, &config, 0);
    }

    if (NO_ERROR == err)
    {
        setDstDpConfig(*dst_priv_handle, &config);

        Rect src_roi(src_priv_handle.width, src_priv_handle.height);
        if (src_priv_handle.width / dst_priv_handle->width >= 20)
        {
            src_roi.left = 0;
            src_roi.right = dst_priv_handle->width * 19;
        }
        Rect dst_roi(dst_priv_handle->width, dst_priv_handle->height);

        BliterNode::Parameter param = {src_roi, dst_roi, &config, 0, false};
        WDT_BL_NODE(setSrc, &config, src_priv_handle);
        WDT_BL_NODE(setDst, 0, &param, dst_priv_handle->ion_fd, dst_priv_handle->sec_handle, fence);
        WDT_BL_NODE(invalidate, job.id);

        if (NO_ERROR == err)
        {
            if (fence != NULL)
            {
                *fence = job.fence;
                job.fence = -1;
            }
        }
    }
    else
    {
        HWC_LOGE("processFillBlack setDpConfig Fail");
        WDT_BL_NODE(cancelJob, job.id);
    }

    if (is_sec)
    {
        BlackBuffer::getInstance().setNormal();
    }
}
void AsyncBliterHandler::processFillWhite(PrivateHandle* dst_priv_handle, int* fence, MdpJob &job)
{
    AutoMutex l(WhiteBuffer::getInstance().m_lock);

    // get WhiteBuffer handle
    buffer_handle_t src_handle = WhiteBuffer::getInstance().getHandle();
    if (src_handle == 0)
    {
        HWC_LOGE("processFillWhite(WhiteBuffer): get handle fail");
        return;
    }

    // check is_sec
    bool is_sec = isSecure(dst_priv_handle);
    if (is_sec)
    {
        WhiteBuffer::getInstance().setSecure();
    }

    BufferConfig  config;
    memset(&config, 0, sizeof(BufferConfig));

    PrivateHandle src_priv_handle;
    status_t err = getPrivateHandle(src_handle, &src_priv_handle);

    if (NO_ERROR == err)
    {
        err = setDpConfig(&src_priv_handle, &config, 0);
    }

    if (NO_ERROR == err)
    {
        setDstDpConfig(*dst_priv_handle, &config);

        Rect src_roi(src_priv_handle.width, src_priv_handle.height);
        Rect dst_roi(dst_priv_handle->width, dst_priv_handle->height);

        BliterNode::Parameter param = {src_roi, dst_roi, &config, 0, false};
        WDT_BL_NODE(setSrc, &config, src_priv_handle);
        WDT_BL_NODE(setDst, 0, &param, dst_priv_handle->ion_fd, dst_priv_handle->sec_handle, fence);
        WDT_BL_NODE(invalidate, job.id);

        if (NO_ERROR == err)
        {
            if (fence != NULL)
            {
                *fence = job.fence;
                job.fence = -1;
            }
        }
    }
    else
    {
        HWC_LOGE("processFillWhite setDpConfig Fail");
        WDT_BL_NODE(cancelJob, job.id);
    }

    if (is_sec)
    {
        WhiteBuffer::getInstance().setNormal();
    }
}

void AsyncBliterHandler::fillBlack(buffer_handle_t handle, PrivateHandle* priv_handle, int* fence, MdpJob* job)
{
    int err;
    bool is_sec = isSecure(priv_handle);

    // clear normal buf
    if (is_sec)
    {
        setSecExtraSfStatus(false, handle);
        err = getPrivateHandle(handle, priv_handle);
        if (0 != err)
        {
            HWC_LOGE("fillBlack - Failed to get priv handle - normal(%p)", handle);
            return;
        }
    }
    processFillBlack(priv_handle, fence, job[0]);

    // clear sec buf if exists
    unsigned int tmp_sec_handle = 0;
    err = gralloc_extra_query(handle, GRALLOC_EXTRA_GET_SECURE_HANDLE_HWC, &tmp_sec_handle);
    if ((GRALLOC_EXTRA_OK != err) || (tmp_sec_handle == 0))
        return;

    setSecExtraSfStatus(true, handle);
    err = getPrivateHandle(handle, priv_handle);
    if (0 != err)
    {
        HWC_LOGE("fillBlack - Failed to get priv handle - secure(%p)", handle);
        return;
    }
    processFillBlack(priv_handle, fence, job[1]);

    // set back
    if (!is_sec)
        setSecExtraSfStatus(false, handle);
}

void AsyncBliterHandler::fillWhite(buffer_handle_t handle, PrivateHandle* priv_handle, int* fence, MdpJob* job)
{
    int err;
    bool is_sec = isSecure(priv_handle);

    // clear normal buf
    if (is_sec)
    {
        setSecExtraSfStatus(false, handle);
        err = getPrivateHandle(handle, priv_handle);
        if (0 != err)
        {
            HWC_LOGE("fillWhite - Failed to get priv handle - normal(%p)", handle);
            return;
        }
    }
    processFillWhite(priv_handle, fence, job[0]);

    // clear sec buf if exists
    unsigned int tmp_sec_handle = 0;
    err = gralloc_extra_query(handle, GRALLOC_EXTRA_GET_SECURE_HANDLE_HWC, &tmp_sec_handle);
    if ((GRALLOC_EXTRA_OK != err) || (tmp_sec_handle == 0))
        return;

    setSecExtraSfStatus(true, handle);
    err = getPrivateHandle(handle, priv_handle);
    if (0 != err)
    {
        HWC_LOGE("fillWhite - Failed to get priv handle - secure(%p)", handle);
        return;
    }
    processFillWhite(priv_handle, fence, job[1]);

    // set back
    if (!is_sec)
        setSecExtraSfStatus(false, handle);
}
void AsyncBliterHandler::clearBackground(buffer_handle_t handle, const Rect* current_dst_roi, int* fence, MdpJob* job)
{
    PrivateHandle priv_handle;

    int err = getPrivateHandle(handle, &priv_handle);
    if (0 != err)
    {
        HWC_LOGE("Failed to get handle(%p)", handle);
        return;
    }

    if (current_dst_roi == nullptr)
    {
        fillBlack(handle, &priv_handle, fence, job);
        HWC_LOGD("clearBufferBlack with NULL ROI");
        return;
    }

    gralloc_extra_ion_hwc_info_t* hwc_ext_info = &priv_handle.hwc_ext_info;
    _crop_t prev_crop = hwc_ext_info->mirror_out_roi;
    _crop_t current_roi;
    current_roi.x = current_dst_roi->left;
    current_roi.y = current_dst_roi->top;
    current_roi.w = current_dst_roi->getWidth();
    current_roi.h = current_dst_roi->getHeight();

    // INIT    = 0xxxb
    // ROT   0 = 1000b
    // ROT  90 = 1100b
    // ROT 180 = 1011b
    // ROT 270 = 1111b
    // USED    = 1xxxb
    if ((prev_crop.w <= 0 || prev_crop.h <=0 ) ||
        (current_roi.w <= 0 || current_roi.h <=0 ) ||
        (prev_crop.x != current_roi.x) ||
        (prev_crop.y != current_roi.y) ||
        (prev_crop.w != current_roi.w) ||
        (prev_crop.h != current_roi.h))
    {
        fillBlack(handle, &priv_handle, fence, job);
        HWC_LOGD("clearBufferBlack (%d,%d,%d,%d) (%d,%d,%d,%d)", prev_crop.x, prev_crop.y, prev_crop.w, prev_crop.h,
            current_roi.x, current_roi.y, current_roi.w, current_roi.h);

        hwc_ext_info->mirror_out_roi = current_roi;

        gralloc_extra_perform(
            handle, GRALLOC_EXTRA_SET_HWC_INFO, hwc_ext_info);
    }
    else if(Platform::getInstance().m_config.fill_black_debug)
    {
        fillWhite(handle, &priv_handle, fence, job);

        HWC_LOGD("clearBufferWhite (%d,%d,%d,%d) (%d,%d,%d,%d)", prev_crop.x, prev_crop.y, prev_crop.w, prev_crop.h,
            current_roi.x, current_roi.y, current_roi.w, current_roi.h);

        hwc_ext_info->mirror_out_roi = current_roi;
        gralloc_extra_perform(
            handle, GRALLOC_EXTRA_SET_HWC_INFO, hwc_ext_info);
    }
}

void AsyncBliterHandler::clearMdpJob(MdpJob* job)
{
    for (int i = 0; i < FILL_BLACK_JOB_SIZE; i++)
    {
        if (job[i].fence != -1)
        {
            protectedClose(job[i].fence);
            job[i].fence = -1;
        }
    }
}
