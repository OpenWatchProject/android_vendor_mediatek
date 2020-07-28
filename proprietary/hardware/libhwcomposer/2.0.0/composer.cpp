#define DEBUG_LOG_TAG "COMP"
#define ATRACE_TAG ATRACE_TAG_GRAPHICS

#include "hwc_priv.h"

#include "gralloc_mtk_defs.h"
#include <hardware/gralloc.h>

#include "utils/debug.h"
#include "utils/tools.h"

#include "composer.h"
#include "display.h"
#include "overlay.h"
#include "dispatcher.h"
#include "worker.h"
#include "sync.h"
#include "platform.h"
// todo: cache
// #include "cache.h"
#include "hwc2.h"
#include <sync/sync.h>

#include <ui/GraphicBuffer.h>

#define CLOGV(i, x, ...) HWC_LOGV("(%d:%d) " x, m_disp_id, i, ##__VA_ARGS__)
#define CLOGD(i, x, ...) HWC_LOGD("(%d:%d) " x, m_disp_id, i, ##__VA_ARGS__)
#define CLOGI(i, x, ...) HWC_LOGI("(%d:%d) " x, m_disp_id, i, ##__VA_ARGS__)
#define CLOGW(i, x, ...) HWC_LOGW("(%d:%d) " x, m_disp_id, i, ##__VA_ARGS__)
#define CLOGE(i, x, ...) HWC_LOGE("(%d:%d) " x, m_disp_id, i, ##__VA_ARGS__)

// ---------------------------------------------------------------------------

inline void computeBufferCrop(
    Rect& src_crop, Rect& dst_crop,
    int disp_width, int disp_height)
{
    if (dst_crop.right > disp_width)
    {
        int diff = dst_crop.right - disp_width;
        dst_crop.right = disp_width;
        src_crop.right -= diff;
    }

    if (dst_crop.bottom > disp_height)
    {
        int diff = dst_crop.bottom - disp_height;
        dst_crop.bottom = disp_height;
        src_crop.bottom -= diff;
    }

    if (dst_crop.left >= 0 && dst_crop.top >=0)
        return;

    if (dst_crop.left < 0)
    {
        src_crop.left -= dst_crop.left;
        dst_crop.left = 0;
    }

    if (dst_crop.top < 0)
    {
        src_crop.top -= dst_crop.top;
        dst_crop.top = 0;
    }
}

// ---------------------------------------------------------------------------

LayerHandler::LayerHandler(int dpy, const sp<OverlayEngine>& ovl_engine)
    : m_disp_id(dpy)
    , m_ovl_engine(ovl_engine)
    , m_sync_fence(new SyncFence(dpy))
{
}

LayerHandler::~LayerHandler()
{
    m_ovl_engine = NULL;
    m_sync_fence = NULL;
}

// ---------------------------------------------------------------------------

ComposerHandler::ComposerHandler(int dpy, const sp<OverlayEngine>& ovl_engine)
    : LayerHandler(dpy, ovl_engine)
{ }

void ComposerHandler::set(
    const sp<HWCDisplay>& display,
    DispatcherJob* job)
{
    uint32_t total_num = job->num_layers;
    auto&& layers = display->getCommittedLayers();
    HWC_LOGV("+ ComposerHandler::set() commit_layers size:%zu", layers.size());

    //DbgLogger logger(DbgLogger::TYPE_HWC_LOG, "(%d) ComposerHandler::set()", display->getId());
    for (uint32_t i = 0; i < total_num; i++)
    {
        HWLayer* hw_layer = &job->hw_layers[i];

        HWC_LOGV("hw_layers[i:%u] enable:%d", i, hw_layer->enable);
        // this layer is not enable
        if (!hw_layer->enable) continue;

        // skip mm layers
        if (HWC_LAYER_TYPE_MM == hw_layer->type) continue;

        // skip mm fbt
        if (HWC_LAYER_TYPE_MM_FBT == hw_layer->type) continue;

        //hwc_layer_1_t* layer = &list->hwLayers[hw_layer->index];
        //todo:
        HWC_LOGV("ComposerHandler::set() hw_layer->index:%d", hw_layer->index);
        sp<HWCLayer> layer = layers[hw_layer->index];
        HWC_LOGV("ComposerHandler::set() id:%" PRIu64 " is_ct:%d", layer->getId(), layer->isClientTarget());
        PrivateHandle* priv_handle = &hw_layer->priv_handle;

        if (HWC_LAYER_TYPE_DIM == hw_layer->type)
        {
            memset(&hw_layer->layer, 0, sizeof(hwc_layer_1_t));
            copyHWCLayerIntoHwcLayer_1(layer, &hw_layer->layer);

            CLOGV(i, "SET/dim");
            continue;
        }

        if (priv_handle == nullptr)
        {
            hw_layer->enable = false;
            continue;
        }

        int type = (priv_handle->ext_info.status & GRALLOC_EXTRA_MASK_TYPE);
        int is_need_flush = (type != GRALLOC_EXTRA_BIT_TYPE_GPU);

        // get release fence from display driver
        {
            OverlayPrepareParam prepare_param;
            prepare_param.id            = i;
            prepare_param.ion_fd        = priv_handle->ion_fd;
            prepare_param.is_need_flush = is_need_flush;

            status_t err = m_ovl_engine->prepareInput(prepare_param);
            if (NO_ERROR != err)
            {
                prepare_param.fence_index = 0;
                prepare_param.fence_fd = -1;
            }
            hw_layer->fence_index = prepare_param.fence_index;

            if (prepare_param.fence_fd <= 0)
            {
                CLOGE(i, "Failed to get releaseFence !!");
            }

            if (layer->isClientTarget())
            {
                protectedClose(prepare_param.fence_fd);
                prepare_param.fence_fd = -1;
            }
            else
            {
                layer->setReleaseFenceFd(prepare_param.fence_fd, display->isConnected());
            }
            // logger.printf(" [i:%u id:%" PRIu64 " ion_fd:%d rel_fence:%d is_ct:%d]",
            //        i, layer->getId(), priv_handle->ion_fd,layer->getReleaseFenceFd(), layer->isClientTarget());
        }
        memset(&hw_layer->layer, 0, sizeof(hwc_layer_1_t));
        copyHWCLayerIntoHwcLayer_1(layer, &hw_layer->layer);

        // partial update - fill dirty rects info
        hw_layer->layer.surfaceDamage = { 0, hw_layer->surface_damage_rect};
        const uint32_t& num_rect = layer->getDamage().numRects;
        if (!job->is_full_invalidate)
        {
            hwc_rect_t* job_dirty_rect = hw_layer->surface_damage_rect;
            if (num_rect == 0 || num_rect > MAX_DIRTY_RECT_CNT)
            {
                job_dirty_rect[0].left   = 0;
                job_dirty_rect[0].top    = 0;
                job_dirty_rect[0].right  = priv_handle->width;
                job_dirty_rect[0].bottom = priv_handle->height;
                hw_layer->layer.surfaceDamage.numRects = 1;
            }
            else
            {
                memcpy(job_dirty_rect, layer->getDamage().rects, sizeof(hwc_rect_t) * num_rect);
                hw_layer->layer.surfaceDamage.numRects = num_rect;
            }
        }

        CLOGV(i, "SET/rel=%d(%d)/acq=%d/handle=%p/ion=%d/flush=%d",
            layer->getReleaseFenceFd(), hw_layer->fence_index, layer->getAcquireFenceFd(),
            layer->getHandle(), priv_handle->ion_fd, is_need_flush);

        layer->setAcquireFenceFd(-1, display->isConnected());
    }
    HWC_LOGV("- ComposerHandler::set");
}

void ComposerHandler::process(DispatcherJob* job)
{
#ifndef MTK_USER_BUILD
    HWC_ATRACE_CALL();
#endif

    uint32_t total_num = job->num_layers;
    uint32_t i = 0;

    OverlayPortParam* const* ovl_params = m_ovl_engine->getInputParams();

    // fill overlay engine setting
    for (i = 0; i < total_num; i++)
    {
        HWLayer* hw_layer = &job->hw_layers[i];

        // this layer is not enable
        if (!hw_layer->enable)
        {
            ovl_params[i]->state = OVL_IN_PARAM_DISABLE;
            ovl_params[i]->sequence = HWC_SEQUENCE_INVALID;
            continue;
        }

        // skip mm layers
        if (HWC_LAYER_TYPE_MM == hw_layer->type) continue;

        // skip mm fbt
        if (HWC_LAYER_TYPE_MM == hw_layer->type) continue;

        m_ovl_engine->setInputDirect(i);

        hwc_layer_1_t* layer = &hw_layer->layer;
        PrivateHandle* priv_handle = &hw_layer->priv_handle;

        int l, t, r, b;
        if (HWC_LAYER_TYPE_DIM != hw_layer->type)
        {
            // [NOTE]
            // Since OVL does not support float crop, adjust coordinate to interger
            // as what SurfaceFlinger did with hwc before version 1.2
            hwc_frect_t* src_cropf = &layer->sourceCropf;
            l = (int)(ceilf(src_cropf->left));
            t = (int)(ceilf(src_cropf->top));
            r = (int)(floorf(src_cropf->right));
            b = (int)(floorf(src_cropf->bottom));
        }
        else
        {
            l = layer->displayFrame.left;
            t = layer->displayFrame.top;
            r = layer->displayFrame.right;
            b = layer->displayFrame.bottom;
        }
        Rect src_crop(l, t, r, b);
        rectifyRectWithPrexform(&src_crop, priv_handle);

        Rect dst_crop(*(Rect *)&(layer->displayFrame));
        OverlayPortParam* param = ovl_params[i];

        param->state        = OVL_IN_PARAM_ENABLE;
        param->mva          = (void*)priv_handle->fb_mva;
        param->pitch        = priv_handle->y_stride;
        param->v_pitch      = priv_handle->vstride;
        param->format       = priv_handle->format;
        param->src_crop     = src_crop;
        param->dst_crop     = dst_crop;
        param->is_sharpen   = false;
        param->fence_index  = hw_layer->fence_index;
        // use hw layer type as identity
        param->identity     = hw_layer->type;
        param->protect      = (priv_handle->usage & GRALLOC_USAGE_PROTECTED);
        param->secure       = (priv_handle->usage & GRALLOC_USAGE_SECURE);
        param->alpha_enable = (layer->blending != HWC2_BLEND_MODE_NONE);
        param->alpha        = layer->planeAlpha;
        param->blending     = layer->blending;
        param->dim          = (hw_layer->type == HWC_LAYER_TYPE_DIM);
        param->sequence     = job->sequence;
        param->is_s3d_layer = job->has_s3d_layer;
        param->s3d_type     = job->s3d_type;
        param->ext_sel_layer = hw_layer->ext_sel_layer;
        param->color_range  = (priv_handle->ext_info.status & GRALLOC_EXTRA_MASK_YUV_COLORSPACE);
        param->dataspace    = hw_layer->dataspace;
#ifdef MTK_HWC_PROFILING
        if (HWC_LAYER_TYPE_FBT == hw_layer->type)
        {
            param->fbt_input_layers = hw_layer->fbt_input_layers;
            param->fbt_input_bytes  = hw_layer->fbt_input_bytes;
        }
#endif
        param->ion_fd      = priv_handle->ion_fd;
        param->fence       = layer->acquireFenceFd;
        param->compress    = isCompressData(priv_handle);

        // partial update - fill dirty rects info
        param->ovl_dirty_rect_cnt = layer->surfaceDamage.numRects;
        if (0 < param->ovl_dirty_rect_cnt && param->ovl_dirty_rect_cnt <= MAX_DIRTY_RECT_CNT)
        {
            const int size = sizeof(hwc_rect_t) * param->ovl_dirty_rect_cnt;
            memcpy(param->ovl_dirty_rect, layer->surfaceDamage.rects, size);
        }
        else if (param->ovl_dirty_rect_cnt != 0)
        {
            HWC_LOGW("the rect number of surfaceDamage is invalid (%d), set cnt = 0", param->ovl_dirty_rect_cnt);
            param->ovl_dirty_rect_cnt = 0;
        }

        // set solid color
        param->layer_color = hw_layer->layer_color;
    }
}

void ComposerHandler::cancelLayers(DispatcherJob* job)
{
    // cancel ui layers for dropping job
    for (int32_t i = 0; i < job->num_layers; ++i)
    {
        HWLayer* hw_layer = &job->hw_layers[i];
        if (!hw_layer->enable) continue;

        // skip mm layers
        if (HWC_LAYER_TYPE_MM == hw_layer->type) continue;

        if (hw_layer->layer.acquireFenceFd != -1) ::protectedClose(hw_layer->layer.acquireFenceFd);
        hw_layer->layer.acquireFenceFd = -1;
    }
}
