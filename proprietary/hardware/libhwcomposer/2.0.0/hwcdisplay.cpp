#define DEBUG_LOG_TAG "HWCDisplay"
#include "hwcdisplay.h"

#include "hwc2_defs.h"
#include "platform.h"
#include "overlay.h"
#include "dispatcher.h"
#include "pqdev.h"

#include <linux/disp_session.h>

#ifdef USES_PQSERVICE
#include <vendor/mediatek/hardware/pq/2.0/IPictureQuality.h>
using android::hardware::hidl_array;
using vendor::mediatek::hardware::pq::V2_0::IPictureQuality;
using vendor::mediatek::hardware::pq::V2_0::Result;
#endif

void findGlesRange(const std::vector<sp<HWCLayer> >& layers, int32_t* head, int32_t* tail)
{
    auto&& head_iter = find_if(layers.begin(), layers.end(),
        [](const sp<HWCLayer>& layer)
        {
            return layer->getCompositionType() == HWC2_COMPOSITION_CLIENT;
        });

    auto&& tail_iter = find_if(layers.rbegin(), layers.rend(),
        [](const sp<HWCLayer>& layer)
        {
            return layer->getCompositionType() == HWC2_COMPOSITION_CLIENT;
        });

    *head = head_iter != layers.end() ? head_iter - layers.begin() : -1;
    *tail = tail_iter != layers.rend() ? layers.rend() - tail_iter - 1 : -1;
}

inline uint32_t extendMDPCapacity(
    const std::vector<sp<HWCLayer> >& /*layers*/, const uint32_t& /*mm_layer_num*/,
    const uint32_t& camera_layer_num, const uint32_t& /*video_layer_num*/)
{
    if (Platform::getInstance().m_config.extend_mdp_capacity)
    {
        // rule 1: no layer came from camera
        if (camera_layer_num != 0)
        {
            return 0;
        }

        // rule 2: Primary diaplay only
        if (DisplayManager::getInstance().getDisplayData(HWC_DISPLAY_EXTERNAL)->connected ||
            DisplayManager::getInstance().getDisplayData(HWC_DISPLAY_VIRTUAL)->connected)
        {
            return 0;
        }

        return 1;
    }

    return 0;
}

HWCDisplay::HWCDisplay(const int64_t& disp_id, const int32_t& type)
    : m_mtk_flags(0)
    , m_type(type)
    , m_outbuf(nullptr)
    , m_is_validated(false)
    , m_disp_id(disp_id)
    , m_active_config(0)
    , m_config_changed(false)
    , m_gles_head(-1)
    , m_gles_tail(-1)
    , m_retire_fence_fd(-1)
    , m_mir_src(-1)
    , m_power_mode(HWC2_POWER_MODE_ON)
    , m_color_transform_hint(HAL_COLOR_TRANSFORM_IDENTITY)
    , m_color_mode(HAL_COLOR_MODE_NATIVE)
    , m_need_disable_vsync_offset(false)
    , m_need_av_grouping(false)
    , m_use_gpu_composition(false)
    , m_color_transform_ok(true)
    , m_color_transform(new ColorTransform(HAL_COLOR_TRANSFORM_IDENTITY, false))
    , m_prev_available_input_layer_num(0)
    , m_vali_present_state(HWC_VALI_PRESENT_STATE_PRESENT_DONE)
    , m_is_visible_layer_changed(false)
    , m_unpresent_count(0)
    , m_prev_unpresent_count(0)
#ifdef MTK_IN_DISPLAY_FINGERPRINT
    , m_is_HBM(false)
#endif
{
    switch (disp_id)
    {
        case HWC_DISPLAY_PRIMARY:
            hwc2_layer_t id = -1;
            createLayer(&id, true);
            m_ct = getLayer(id);
    }
}

void HWCDisplay::init()
{
    m_unpresent_count = 0;
    m_prev_unpresent_count = 0;

    switch (getId())
    {
        case HWC_DISPLAY_EXTERNAL:
            {
                hwc2_layer_t id = -1;
                createLayer(&id, true);
                m_ct = getLayer(id);
                m_mtk_flags = 0;
                m_is_validated = false;
                m_gles_head = -1;
                m_gles_tail = -1;
                m_retire_fence_fd = -1;
                m_mir_src = -1;
                // external display will not present until SF setPowerMode ON
                m_power_mode = HWC2_POWER_MODE_OFF;
                m_color_transform_hint = HAL_COLOR_TRANSFORM_IDENTITY;
                m_color_mode = HAL_COLOR_MODE_NATIVE;
                m_use_gpu_composition = false;
                m_need_disable_vsync_offset = false;
                m_need_av_grouping = false;
                m_color_transform_ok = true;
                m_prev_available_input_layer_num = 0;
                m_vali_present_state = HWC_VALI_PRESENT_STATE_PRESENT_DONE;
                m_is_visible_layer_changed = false;
            }
            break;

        case HWC_DISPLAY_VIRTUAL:
            {
                m_outbuf = new HWCBuffer(getId(), -1, false);
                hwc2_layer_t id = -1;
                createLayer(&id, true);
                m_ct = getLayer(id);
                m_mtk_flags = 0;
                m_is_validated = false;
                m_gles_head = -1;
                m_gles_tail = -1;
                m_retire_fence_fd = -1;
                m_mir_src = -1;
                m_power_mode = HWC2_POWER_MODE_ON;
                m_color_transform_hint = HAL_COLOR_TRANSFORM_IDENTITY;
                m_color_mode = HAL_COLOR_MODE_NATIVE;
                m_use_gpu_composition = false;
                m_need_disable_vsync_offset = false;
                m_need_av_grouping = false;
                m_color_transform_ok = true;
                m_prev_available_input_layer_num = 0;
                m_vali_present_state = HWC_VALI_PRESENT_STATE_PRESENT_DONE;
                m_is_visible_layer_changed = false;
            }
            break;
    }
}

void HWCDisplay::initPrevCompTypes()
{
    auto&& layers = getVisibleLayersSortedByZ();
    m_prev_comp_types.resize(layers.size());
    for (size_t i = 0; i < m_prev_comp_types.size(); ++i)
        m_prev_comp_types[i] = layers[i]->getCompositionType();
}

int32_t HWCDisplay::setColorTransform(const float* matrix, const int32_t& hint)
{
    m_color_transform_hint = hint;
    m_color_transform = new ColorTransform(matrix, hint, true);

    if (!HWCMediator::getInstance().m_features.is_support_pq ||
        !Platform::getInstance().m_config.support_color_transform ||
        getId() == HWC_DISPLAY_VIRTUAL)
    {
        m_color_transform_ok = (hint == HAL_COLOR_TRANSFORM_IDENTITY);
        m_color_transform->dirty = false;
    }
    else
    {
#ifdef USES_PQSERVICE
        if (getId() == HWC_DISPLAY_PRIMARY)
        {
            m_color_transform_ok = PqDevice::getInstance().setColorTransform(matrix, hint);
            HWC_LOGI("(%" PRIu64 ") %s hint:%d ok:%d", getId(), __func__, hint, m_color_transform_ok);
            if (m_color_transform_ok)
            {
                HWCMediator::getInstance().getOvlDevice(HWC_DISPLAY_PRIMARY)->setLastValidColorTransform(HWC_DISPLAY_PRIMARY, m_color_transform);
            }
            else
            {
                m_color_transform = new ColorTransform(HAL_COLOR_TRANSFORM_IDENTITY, true);
            }
        }
        else
        {
            m_color_transform_ok = false;
        }
#else
        m_color_transform_ok = (hint == HAL_COLOR_TRANSFORM_IDENTITY);
#endif
    }
    return m_color_transform_ok ? HWC2_ERROR_NONE : HWC2_ERROR_UNSUPPORTED;
}

void HWCDisplay::setJobDisplayOrientation()
{
    const int32_t disp_id = static_cast<int32_t>(getId());
    auto&& layers = getVisibleLayersSortedByZ();
    DispatcherJob* job = HWCDispatcher::getInstance().getExistJob(disp_id);
    //HWC decides how MDP rotates the display WDMA output buffer to fit TV out in mirror path,
    //so orienatation of source and sink display is needed in the original solution. Unfortunately,
    //SF does NOT tell HWC about display orientation, so we must figure out the other solution without modification AOSP code.
    //The importance is how many degrees to rotate the WDMA buffer not the sigle display orientation.
    //Therefore, the single layer's transform on the source and sink display can be used in this case.
    //SF provides how a single layer should be rotated on the source and sink display to HWC,
    //and the rotation can be used for the WDMA output buffer,too.
    if (job != nullptr)
    {
        // job->disp_ori_rot has initialized with 0 when job create
        if (layers.size())
        {
            for (auto& layer : layers)
            {
                if (layer->getSFCompositionType() != HWC2_COMPOSITION_SOLID_COLOR)
                {
                    job->disp_ori_rot = layer->getTransform();
                    break;
                }
            }
        }
    }
}

void HWCDisplay::setJobDisplayData()
{
    const int32_t disp_id = static_cast<int32_t>(getId());
    DispatcherJob* job = HWCDispatcher::getInstance().getExistJob(disp_id);

    if (job != nullptr)
    {
        job->disp_data = DisplayManager::getInstance().getDisplayData(disp_id, m_active_config);
        job->active_config = m_active_config;
    }
}

void HWCDisplay::validate()
{
    if (!isConnected())
    {
        HWC_LOGE("%s: the display(%" PRId64 ") is not connected!", __func__, m_disp_id);
        return;
    }

    setValiPresentState(HWC_VALI_PRESENT_STATE_VALIDATE, __LINE__);
    m_is_validated = true;

    auto&& layers = getVisibleLayersSortedByZ();

    setIsHBM(false);
    for (auto& layer : layers)
    {
        if (layer->getPrivateHandle().layer_type == HWC_LAYER_TYPE_HBM)
        {
            setIsHBM(true);
            break;
        }
    }

    const int32_t disp_id = static_cast<int32_t>(getId());
    DispatcherJob* job = HWCDispatcher::getInstance().getExistJob(disp_id);
    const int32_t ovl_layer_max = (job != NULL) ? job->num_layers : 0;
    const bool force_gpu_compose = isForceGpuCompose();

    if (getMirrorSrc() != -1)
    {
        for (auto& layer : layers)
        {
            layer->setHwlayerType(HWC_LAYER_TYPE_WORMHOLE, __LINE__, HWC_COMP_FILE_HWCD);
        }
    }
    else if (force_gpu_compose)
    {
        for (auto& layer : layers)
            layer->setHwlayerType(HWC_LAYER_TYPE_INVALID, __LINE__, HWC_COMP_FILE_HWCD);
    }
    else
    {
        const uint32_t MAX_MM_NUM = 1;
        uint32_t mm_layer_num = 0;
        uint32_t camera_layer_num = 0;
        uint32_t video_layer_num = 0;
        for (size_t i = 0; i < layers.size(); ++i)
        {
            auto& layer = layers[i];
            // force full invalidate
            if (disp_id == HWC_DISPLAY_PRIMARY)
            {
                if (i != Platform::getInstance().m_config.force_pq_index)
                {
                    layer->setForcePQ(false);
                }
                else
                {
                    layer->setForcePQ(true);
                }
            }
            layer->validate();

            if (layer->getHwlayerType() == HWC_LAYER_TYPE_MM ||
                layer->getHwlayerType() == HWC_LAYER_TYPE_UIPQ)
                ++mm_layer_num;

            switch (layer->getPrivateHandle().ext_info.status & GRALLOC_EXTRA_MASK_TYPE)
            {
                case GRALLOC_EXTRA_BIT_TYPE_CAMERA:
                    ++camera_layer_num;
                break;

                case GRALLOC_EXTRA_BIT_TYPE_VIDEO:
                    ++video_layer_num;
                break;
            }
        }

        int mm_ui_num = 0;
        mm_ui_num = mm_layer_num - camera_layer_num - video_layer_num;

        // need to judge MDP capacity
        if (mm_layer_num > MAX_MM_NUM)
        {
            // for MM_UI layer too much case, we will set MM_UI layer into GLES
            if (mm_ui_num > 1 && mm_layer_num > 2 && layers.size() >= 4)
            {
                for (auto& layer : layers)
                {
                    if ((layer->getHwlayerType() == HWC_LAYER_TYPE_MM ||
                        layer->getHwlayerType() == HWC_LAYER_TYPE_UIPQ) &&
                        (((layer->getPrivateHandle().ext_info.status & GRALLOC_EXTRA_MASK_TYPE) != GRALLOC_EXTRA_BIT_TYPE_CAMERA) &&
                        ((layer->getPrivateHandle().ext_info.status & GRALLOC_EXTRA_MASK_TYPE) != GRALLOC_EXTRA_BIT_TYPE_VIDEO)))
                    {
                        layer->setHwlayerType(HWC_LAYER_TYPE_INVALID, __LINE__, HWC_COMP_FILE_HWCD);
                        --mm_layer_num;
                    }
                }
            }

            // calculate extend mdp capacity for Primary display.
            uint32_t EXTEND_MAX_MM_NUM = disp_id == HWC_DISPLAY_PRIMARY ?
                extendMDPCapacity(layers, mm_layer_num, camera_layer_num, video_layer_num) : 0;

            mm_layer_num = 0;
            for (auto& layer : layers)
            {
                if (layer->getHwlayerType() == HWC_LAYER_TYPE_MM &&
                    ((layer->getPrivateHandle().usage & GRALLOC_USAGE_PROTECTED) != 0 ||
                     layer->getPrivateHandle().sec_handle != 0))
                {
                    // secure MM layer, never composed by GLES
                    ++mm_layer_num;
                }
                else
                {
                    if (mm_layer_num >= (MAX_MM_NUM + EXTEND_MAX_MM_NUM) &&
                        (layer->getHwlayerType() == HWC_LAYER_TYPE_MM ||
                         layer->getHwlayerType() == HWC_LAYER_TYPE_UIPQ))
                        layer->setHwlayerType(HWC_LAYER_TYPE_INVALID, __LINE__, HWC_COMP_FILE_HWCD);

                    if (layer->getHwlayerType() == HWC_LAYER_TYPE_MM ||
                        layer->getHwlayerType() == HWC_LAYER_TYPE_UIPQ)
                        ++mm_layer_num;
                }
            }
        }
    }
/*
    auto&& print_layers = m_layers;
    for (auto& kv : print_layers)
    {
        auto& layer = kv.second;
        auto& display_frame = layer->getDisplayFrame();
        HWC_LOGD("(%d) layer id:%" PRIu64 " hnd:%x z:%d hwlayer type:%s(%s,%s) line:%d displayf:[%d,%d,%d,%d] tr:%d",
            getId(),
            layer->getId(),
            layer->getHandle(),
            layer->getZOrder(),
            getCompString(layer->getCompositionType()),
            getHWLayerString(layer->getHwlayerType()),
            getCompString(layer->getSFCompositionType()),
            layer->getHwlayerTypeLine(),
            display_frame.left,
            display_frame.top,
            display_frame.right,
            display_frame.bottom,
            layer->getTransform()
        );
    }
*/
    HWC_LOGD("(%" PRIu64 ") VAL/l:%d/max:%d/fg:%d", getId(), layers.size(), ovl_layer_max, force_gpu_compose);
}

inline static void fillHwLayer(
    const uint64_t& dpy, DispatcherJob* job, const sp<HWCLayer>& layer,
    const int& ovl_idx, const int& layer_idx, const int& ext_sel_layer)
{
    HWC_ATRACE_FORMAT_NAME("HWC(h:%p)", layer->getHandle());
    const PrivateHandle* priv_handle = &layer->getPrivateHandle();

    if (ovl_idx < 0 || ovl_idx >= job->num_layers) {
        HWC_LOGE("try to fill HWLayer with invalid index: 0x%x(0x%x)", ovl_idx, job->num_layers);
        abort();
    }
    HWLayer* hw_layer  = &job->hw_layers[ovl_idx];
    hw_layer->enable   = true;
    hw_layer->index    = layer_idx;
    hw_layer->type     = layer->getHwlayerType();
    hw_layer->dirty    = HWCDispatcher::getInstance().decideDirtyAndFlush(
        dpy, priv_handle, ovl_idx, layer->isBufferChanged() || layer->isStateChanged(),
        layer->getHwlayerType(), layer->getId(), layer->getLayerCaps(), layer->isNeedPQ());
    hw_layer->hwc2_layer_id = layer->getId();

    if (HWCMediator::getInstance().m_features.global_pq)
    {
        if (layer->getHwlayerType() == HWC_LAYER_TYPE_UIPQ_DEBUG)
        {
            hw_layer->type = HWC_LAYER_TYPE_UI;
            hw_layer->enable = false;
            job->uipq_index = layer_idx;
        }
        if (layer->getHwlayerType() == HWC_LAYER_TYPE_UIPQ)
        {
            hw_layer->type = HWC_LAYER_TYPE_MM;
            hw_layer->dirty = true;
            job->uipq_index = layer_idx;
        }
    }

    if (layer->getHwlayerType() == HWC_LAYER_TYPE_MM ||
        layer->getHwlayerType() == HWC_LAYER_TYPE_UIPQ)
    {
        hw_layer->mdp_dst_roi.left = layer->getMdpDstRoi().left;
        hw_layer->mdp_dst_roi.top = layer->getMdpDstRoi().top;
        hw_layer->mdp_dst_roi.right = layer->getMdpDstRoi().right;
        hw_layer->mdp_dst_roi.bottom = layer->getMdpDstRoi().bottom;
    }
    hw_layer->ext_sel_layer = Platform::getInstance().m_config.enable_smart_layer ? ext_sel_layer : -1;
    hw_layer->layer_caps    = layer->getLayerCaps();
    hw_layer->layer_color = layer->getLayerColor();
    hw_layer->dataspace    = layer->getDataspace();
    hw_layer->need_pq    = layer->isNeedPQ();

    memcpy(&hw_layer->priv_handle, priv_handle, sizeof(PrivateHandle));

    HWC_LOGV("(%" PRIu64 ")   layer(%" PRIu64 ") hnd:%p caps:%d z:%d hw_layer->index:%d ovl_idx:%d  isenable:%d type:%d hwlayer->priv_handle->ion_fd:%d dirty:%d",
        dpy, layer->getId(), layer->getHandle(), layer->getLayerCaps(), layer->getZOrder(), hw_layer->index, ovl_idx, hw_layer->enable ,hw_layer->type, hw_layer->priv_handle.ion_fd, hw_layer->dirty);
}

inline void setupHwcLayers(const sp<HWCDisplay>& display, DispatcherJob* job)
{
    HWC_LOGV("(%" PRIu64 ") + setupHwcLayers", display->getId());
    int32_t gles_head = -1, gles_tail = -1;
    const uint64_t disp_id = display->getId();

    display->getGlesRange(&gles_head, &gles_tail);

    job->num_ui_layers = 0;
    job->num_mm_layers = 0;
    const bool hrt_from_driver =
        (0 != HWCMediator::getInstance().getOvlDevice(disp_id)->isDisplayHrtSupport());

    auto&& layers = display->getCommittedLayers();

    int ovl_index = 0;
    for (size_t i = 0; i < layers.size(); ++i, ++ovl_index)
    {
        sp<HWCLayer> layer = layers[i];
        if (layer->isClientTarget())
            continue;

        int ext_sel_layer = -1;

        int32_t ovl_id = 0;
        if (hrt_from_driver)
        {
            const int32_t hrt_idx = (gles_head == -1 || int32_t(i) < gles_head ? i : i + (gles_tail - gles_head + 1));
            ovl_id = job->layer_info.hrt_config_list[hrt_idx].ovl_id;
            ext_sel_layer = job->layer_info.hrt_config_list[hrt_idx].ext_sel_layer;
        }
        else
        {
            ovl_id = (gles_head == -1 || static_cast<int32_t>(i) < gles_head) ? ovl_index : ovl_index + 1;
            ext_sel_layer = -1;
        }
        HWC_LOGV("(%" PRIu64 ")   setupHwcLayers i:%d ovl_id:%d hrt_from_driver:%d gles_head:%d, ovl_index:%d",
            display->getId(), i, ovl_id, hrt_from_driver, gles_head, ovl_index);

        fillHwLayer(disp_id, job, layer, ovl_id, i, ext_sel_layer);

        const PrivateHandle* priv_handle = &layer->getPrivateHandle();
        // check if any protect layer exist
        job->protect |= (priv_handle->usage & GRALLOC_USAGE_PROTECTED);

        // check if need to enable secure composition
        job->secure |= (priv_handle->usage & GRALLOC_USAGE_SECURE);

        const int32_t type = layer->getHwlayerType();
        switch (type)
        {
            case HWC_LAYER_TYPE_UI:
            case HWC_LAYER_TYPE_DIM:
            case HWC_LAYER_TYPE_CURSOR:
                ++job->num_ui_layers;
                break;

            case HWC_LAYER_TYPE_MM:
                ++job->num_mm_layers;
                break;

            case HWC_LAYER_TYPE_UIPQ_DEBUG:
                if (HWCMediator::getInstance().m_features.global_pq)
                    ++job->num_ui_layers;
                else
                    HWC_LOGE("global pq feature not support!");
                break;

            case HWC_LAYER_TYPE_UIPQ:
                if (HWCMediator::getInstance().m_features.global_pq)
                    ++job->num_mm_layers;
                else
                    HWC_LOGE("global pq feature not support!");
                break;
        }
    }
    HWC_LOGV("(%" PRIu64 ") - setupHwcLayers", display->getId());
}

void HWCDisplay::setJobVideoTimeStamp(DispatcherJob* job)
{
    if (NULL == job)
        return;

    auto&& layers = getVisibleLayersSortedByZ();

    for (size_t i = 0; i < layers.size(); ++i)
    {
        sp<HWCLayer> layer = layers[i];
        if (layer->isClientTarget())
            continue;

        const PrivateHandle* priv_handle = &layer->getPrivateHandle();
        // check if need to set video timestamp
        int buffer_type = (priv_handle->ext_info.status & GRALLOC_EXTRA_MASK_TYPE);
        if (buffer_type == GRALLOC_EXTRA_BIT_TYPE_VIDEO)
            job->timestamp = priv_handle->ext_info.timestamp;
    }
}

void HWCDisplay::setGlesRange(const int32_t& gles_head, const int32_t& gles_tail)
{
    m_gles_head = gles_head;
    m_gles_tail = gles_tail;

    if (gles_head == -1)
        return;

    HWC_LOGV("setGlesRange() gles_head:%d gles_tail:%d", gles_head, gles_tail);
    auto&& layers = getVisibleLayersSortedByZ();
    for (int32_t i = gles_head; i <= gles_tail; ++i)
    {
        auto& layer = layers[i];
        if (layer->getHwlayerType() != HWC_LAYER_TYPE_INVALID)
            layer->setHwlayerType(HWC_LAYER_TYPE_INVALID, __LINE__, HWC_COMP_FILE_HWCD);
    }
}

static void calculateFbtRoi(
    const sp<HWCLayer>& fbt_layer,
    const std::vector<sp<HWCLayer> >& layers,
    const int32_t& gles_head, const int32_t& gles_tail,
    const bool& isDisabled, const int32_t& /*disp_width*/, const int32_t& /*disp_height*/)
{
    if (!HWCMediator::getInstance().m_features.fbt_bound ||
        isDisabled)
    {
        hwc_frect_t src_crop;
        src_crop.left = src_crop.top = 0;
        src_crop.right = fbt_layer->getPrivateHandle().width;
        src_crop.bottom = fbt_layer->getPrivateHandle().height;
        fbt_layer->setSourceCrop(src_crop);

        hwc_rect_t display_frame;
        display_frame.left = display_frame.top = 0;
        display_frame.right = fbt_layer->getPrivateHandle().width;
        display_frame.bottom = fbt_layer->getPrivateHandle().height;
        fbt_layer->setDisplayFrame(display_frame);
        return;
    }

    Region fbt_roi_region;
    for (int32_t i = gles_head; i <= gles_tail; ++i)
    {
        auto& layer = layers[i];
        auto& display_frame = layer->getDisplayFrame();
        Rect gles_layer_rect(
                display_frame.left,
                display_frame.top,
                display_frame.right,
                display_frame.bottom);
        fbt_roi_region = fbt_roi_region.orSelf(gles_layer_rect);
    }
    Rect fbt_roi = fbt_roi_region.getBounds();
    if (!fbt_roi.isEmpty())
    {
        hwc_frect_t src_crop;
        src_crop.left = fbt_roi.left;
        src_crop.top = fbt_roi.top;
        src_crop.right = fbt_roi.right;
        src_crop.bottom = fbt_roi.bottom;
        fbt_layer->setSourceCrop(src_crop);

        hwc_rect_t display_frame;
        display_frame.left = fbt_roi.left;
        display_frame.right = fbt_roi.right;
        display_frame.top = fbt_roi.top;
        display_frame.bottom = fbt_roi.bottom;
        fbt_layer->setDisplayFrame(display_frame);
    }
}

static void setupGlesLayers(const sp<HWCDisplay>& display, DispatcherJob* job)
{
    int32_t gles_head = -1, gles_tail = -1;
    display->getGlesRange(&gles_head, &gles_tail);
    if (gles_head == -1)
    {
        job->fbt_exist = false;
        return ;
    }

    if (gles_head != -1 && display->getClientTarget()->getHandle() == nullptr)
    {
        // SurfaceFlinger might not setClientTarget while VDS disconnect.
        if (display->getId() == HWC_DISPLAY_PRIMARY)
        {
            HWC_LOGE("(%" PRIu64 ") %s: HWC does not receive client target's handle, g[%d,%d]", display->getId(), __func__, gles_head, gles_tail);
            job->fbt_exist = false;
            return ;
        }
        else
        {
            HWC_LOGW("(%" PRIu64 ") %s: HWC does not receive client target's handle, g[%d,%d] acq_fd:%d", display->getId(), __func__, gles_head, gles_tail, display->getClientTarget()->getAcquireFenceFd());
        }
    }

    job->fbt_exist = true;

    auto&& visible_layers = display->getVisibleLayersSortedByZ();
    auto&& commit_layers = display->getCommittedLayers();

    // close acquire fence of gles layers
    {
        for (size_t i = gles_head ; int32_t(i) <= gles_tail; ++i)
        {
            auto& layer = visible_layers[i];
            const int32_t acquire_fence_fd = layer->getAcquireFenceFd();
            if (acquire_fence_fd != -1)
            {
                protectedClose(layer->getAcquireFenceFd());
                layer->setAcquireFenceFd(-1, display->isConnected());
            }
        }
    }

    const bool hrt_from_driver =
        (0 != HWCMediator::getInstance().getOvlDevice(display->getId())->isDisplayHrtSupport());
    const int32_t ovl_id = hrt_from_driver ? job->layer_info.hrt_config_list[gles_head].ovl_id : gles_head;
    const int32_t fbt_hw_layer_idx = hrt_from_driver ? ovl_id : gles_head;
    HWC_LOGV("setupGlesLayers() gles[%d,%d] fbt_hw_layer_idx:%d", gles_head, gles_tail, fbt_hw_layer_idx);

    sp<HWCLayer> fbt_layer = display->getClientTarget();

    // set roi of client target
    const bool disable_fbt_roi = job->has_s3d_layer;
    calculateFbtRoi(fbt_layer, visible_layers, gles_head, gles_tail, disable_fbt_roi, display->getWidth(), display->getHeight());

    if (fbt_hw_layer_idx < 0 || fbt_hw_layer_idx >= job->num_layers) {
        HWC_LOGE("try to fill HWLayer with invalid index for client target: 0x%x(0x%x)", fbt_hw_layer_idx, job->num_layers);
        abort();
    }
    HWLayer* fbt_hw_layer = &job->hw_layers[fbt_hw_layer_idx];
    HWC_ATRACE_FORMAT_NAME("GLES CT(h:%p)", fbt_layer->getHandle());
    fbt_hw_layer->enable  = true;
    fbt_hw_layer->index   = commit_layers.size() - 1;
    fbt_hw_layer->hwc2_layer_id = fbt_layer->getId();
    // sw tcon is too slow, so we want to use MDP to backup this framebuffer.
    // then SurfaceFlinger can use this framebuffer immediately.
    // therefore change its composition type to mm
    if (DisplayManager::getInstance().getDisplayData(display->getId())->subtype == HWC_DISPLAY_EPAPER)
    {
        fbt_hw_layer->type = HWC_LAYER_TYPE_MM_FBT;
        job->mm_fbt = true;
    }
    else
    {
        fbt_hw_layer->type = HWC_LAYER_TYPE_FBT;
    }
    fbt_hw_layer->dirty   = HWCDispatcher::getInstance().decideCtDirtyAndFlush(display->getId(), fbt_hw_layer_idx);
    fbt_hw_layer->ext_sel_layer = -1;
    fbt_hw_layer->dataspace    = fbt_layer->getDataspace();

    const PrivateHandle* priv_handle = &fbt_layer->getPrivateHandle();
    memcpy(&fbt_hw_layer->priv_handle, priv_handle, sizeof(PrivateHandle));
    if (hrt_from_driver && Platform::getInstance().m_config.enable_smart_layer)
    {
        fbt_hw_layer->ext_sel_layer = job->layer_info.hrt_config_list[gles_head].ext_sel_layer;
    }
}

void HWCDisplay::updateGlesRange()
{
    if (!isConnected())
        return;

    int32_t gles_head = -1, gles_tail = -1;
    findGlesRange(getVisibleLayersSortedByZ(), &gles_head, &gles_tail);
    HWC_LOGV("updateGlesRange() gles_head:%d , gles_tail:%d", gles_head, gles_tail);
    setGlesRange(gles_head, gles_tail);

    const int32_t disp_id = getId();
    DispatcherJob* job = HWCDispatcher::getInstance().getExistJob(disp_id);
    if (job != NULL)
    {
        job->layer_info.gles_head = gles_head;
        job->layer_info.gles_tail = gles_tail;
    }
}

void HWCDisplay::acceptChanges()
{
}

void HWCDisplay::setRetireFenceFd(const int32_t& retire_fence_fd, const bool& is_disp_connected)
{
    if (retire_fence_fd >= 0 && m_retire_fence_fd != -1)
    {
        if (is_disp_connected)
        {
            HWC_LOGE("(%" PRIu64 ") fdleak detect: %s retire_fence_fd:%d",
                getId(), __func__, m_retire_fence_fd);
            AbortMessager::getInstance().abort();
        }
        else
        {
            HWC_LOGW("(%" PRIu64 ") fdleak detect: %s retire_fence_fd:%d",
                getId(), __func__, m_retire_fence_fd);
            ::protectedClose(m_retire_fence_fd);
            m_retire_fence_fd = -1;
        }
    }
    m_retire_fence_fd = retire_fence_fd;
}

void HWCDisplay::setColorTransformForJob(DispatcherJob* const job)
{
    job->color_transform = nullptr;
    if (m_color_transform != nullptr && m_color_transform->dirty)
    {
        sp<ColorTransform> color = new ColorTransform(
            m_color_transform->matrix,
            m_color_transform->hint,
            true);
        m_color_transform->dirty = false;
        job->color_transform = color;
    }
}

void HWCDisplay::beforePresent(const int32_t num_validate_display)
{
    if (getMirrorSrc() != -1)
    {
        auto&& layers = getVisibleLayersSortedByZ();
        for (auto& layer : layers)
        {
            const int32_t acquire_fence_fd = layer->getAcquireFenceFd();
            if (acquire_fence_fd != -1)
            {
                protectedClose(acquire_fence_fd);
                layer->setAcquireFenceFd(-1, isConnected());
            }
        }
    }

    if (getId() == HWC_DISPLAY_VIRTUAL)
    {
        if (getOutbuf() == nullptr)
        {
            HWC_LOGE("(%" PRIu64 ") outbuf missing", getId());
            clearAllFences();
            return;
        }
        else if (getOutbuf()->getHandle() == nullptr)
        {
            HWC_LOGE("(%" PRIu64 ") handle of outbuf missing", getId());
            clearAllFences();
            return;
        }
    }

    updateGlesRange();

    DispatcherJob* job = HWCDispatcher::getInstance().getExistJob(getId());
    if (NULL != job)
    {
        HWCDispatcher::getInstance().initPrevHwLayers(this, job);

        getGlesRange(&job->layer_info.gles_head, &job->layer_info.gles_tail);

        if (getMirrorSrc() != -1)
        {
            // prepare job in job group
            job->fbt_exist     = false;
            job->num_ui_layers = 0;
            // [NOTE] treat mirror source as mm layer
            job->num_mm_layers = 1;
            job->hw_layers[0].enable = true;
            job->hw_layers[0].type = HWC_LAYER_TYPE_MM;
        }
        else
        {
            setupHwcLayers(this, job);
            setupGlesLayers(this, job);

            job->post_state     = job->layer_info.disp_dirty ? HWC_POST_INPUT_DIRTY : HWC_POST_INPUT_NOTDIRTY;

            // [WORKAROUND]
            // No need to force wait since UI layer does not exist
            if (job->force_wait && !job->num_ui_layers)
                job->force_wait = false;

            // NOTE: enable this profiling to observe hwc recomposition
            if (DisplayManager::m_profile_level & PROFILE_TRIG)
            {
                char tag[16];
                snprintf(tag, sizeof(tag), "HWC_COMP_%" PRIu64, getId());
                ATRACE_INT(tag, job->layer_info.disp_dirty ? 1 : 0);
            }
        }

        HWCDispatcher::getInstance().fillPrevHwLayers(this, job);

        setColorTransformForJob(job);

        if (job->layer_info.max_overlap_layer_num == -1)
        {
            job->layer_info.max_overlap_layer_num = job->num_ui_layers
                                                  + job->num_mm_layers
                                                  + (job->fbt_exist ? 1 : 0);
        }

        job->is_full_invalidate =
            HWCMediator::getInstance().getOvlDevice(getId())->isPartialUpdateSupported() ? isGeometryChanged() : true;

        if (needDoAvGrouping(num_validate_display))
            job->need_av_grouping = true;

        HWC_LOGD("(%" PRIu64 ") VAL list=%d/max=%d/fbt=%d[%d,%d:%d,%d](%s)/hrt=%d,%d/ui=%d/mm=%d/ovlp=%d/fi=%d/mir=%d",
            getId(), getVisibleLayersSortedByZ().size(), job->num_layers,
            job->fbt_exist,job->layer_info.hwc_gles_head, job->layer_info.hwc_gles_tail, job->layer_info.gles_head, job->layer_info.gles_tail, job->mm_fbt ? "MM" : "OVL",
            job->layer_info.hrt_weight, job->layer_info.hrt_idx,
            job->num_ui_layers, job->num_mm_layers, job->layer_info.max_overlap_layer_num,
            job->is_full_invalidate, job->disp_mir_id);

        // todo:
        // if (HWCDispatcher::getInstance().m_ultra_scenario &&
        //        Platform::getInstance().m_config.use_async_bliter &&
        //        Platform::getInstance().m_config.use_async_bliter_ultra)
        // {
        //    check_ultra_mdp(num_display, displays);
        // }
        setPrevAvailableInputLayerNum(job->num_layers);
    }
    else
    {
        clearAllFences();
        HWC_LOGE("(%" PRIu64 ") job is null", getId());
    }
}

void HWCDisplay::present()
{
    setValiPresentState(HWC_VALI_PRESENT_STATE_PRESENT, __LINE__);

    if (getId() == HWC_DISPLAY_VIRTUAL)
    {
        if (getOutbuf() == nullptr)
        {
            HWC_LOGE("(%" PRIu64 ") outbuf missing", getId());
            return;
        }
        else if (getOutbuf()->getHandle() == nullptr)
        {
            HWC_LOGE("(%" PRIu64 ") handle of outbuf missing", getId());
            return;
        }
    }

    updateFps();
    HWCDispatcher::getInstance().setJob(this);
}

void HWCDisplay::afterPresent()
{
    setLastCommittedLayers(getCommittedLayers());
    setMirrorSrc(-1);

    int32_t needAbort = 0;
    for (auto& kv : m_layers)
    {
        auto& layer = kv.second;
        needAbort += layer->afterPresent(isConnected());
    }
    if (needAbort) {
        HWC_LOGE("There is %d layers which acquire fence were not closed", needAbort);
        AbortMessager::getInstance().abort();
    }

    if (getOutbuf() != nullptr)
    {
        getOutbuf()->afterPresent(isConnected());
        if (getOutbuf()->getReleaseFenceFd() != -1)
        {
            if (isConnected())
            {
                HWC_LOGE("(%" PRIu64 ") %s getReleaseFenceFd:%d", getId(), __func__, getOutbuf()->getReleaseFenceFd());
                AbortMessager::getInstance().abort();
            }
            else
            {
                HWC_LOGW("(%" PRIu64 ") %s getReleaseFenceFd:%d", getId(), __func__, getOutbuf()->getReleaseFenceFd());
                ::protectedClose(getOutbuf()->getReleaseFenceFd());
                getOutbuf()->setReleaseFenceFd(-1, isConnected());
            }
        }
    }

    // just set as -1, do not close!!!
    if (getRetireFenceFd() > -1)
    {
        if (isConnected())
        {
            HWC_LOGE("(%" PRIu64 ") %s getRetireFenceFd():%d", getId(), __func__, getRetireFenceFd());
            AbortMessager::getInstance().abort();
        }
        else
        {

            HWC_LOGW("(%" PRIu64 ") %s getRetireFenceFd():%d", getId(), __func__, getRetireFenceFd());
            ::protectedClose(getRetireFenceFd());
            setRetireFenceFd(-1, isConnected());
        }
    }

#ifdef USES_PQSERVICE
    // check if need to refresh display
    if (HWCMediator::getInstance().m_features.global_pq &&
        HWCMediator::getInstance().m_features.is_support_pq &&
        getId() == HWC_DISPLAY_PRIMARY &&
        !DisplayManager::getInstance().getDisplayData(HWC_DISPLAY_EXTERNAL)->connected &&
        !DisplayManager::getInstance().getDisplayData(HWC_DISPLAY_VIRTUAL)->connected)
    {
        sp<IPictureQuality> pq_service = IPictureQuality::getService();
        if (pq_service == nullptr)
        {
            HWC_LOGE("cannot find PQ service for pq update!");
        }
        else
        {
            bool need_refresh = false;
            // UIPQ dc effect is gradient by frame, it may need refresh display
            // because dc status has not reach stable yet. Consider uipq layer
            // is composed by device or client, we check dc status after display
            // present. 0: unstable; 1: stable
            pq_service->getGlobalPQStableStatus(
                [&] (Result retval, int32_t stable_status)
                {
                    if (retval == Result::OK)
                    {
                        need_refresh = (0 == stable_status) ? true : false;
                    }
                    else
                    {
                        HWC_LOGE("getGlobalPQStableStatus failed!");
                    }
                });
            if (need_refresh)
            {
                // clear stable flag to avoid always refresh if bypass MDP by accident.
                if (pq_service->setGlobalPQStableStatus(1) != Result::OK)
                {
                    HWC_LOGE("setGlobalPQStableStatus failed!");
                }
                DisplayManager::getInstance().refresh(HWC_DISPLAY_PRIMARY);
            }
        }
    }
#endif
    decUnpresentCount();
}

void HWCDisplay::clear()
{
    m_outbuf = nullptr;
    m_changed_comp_types.clear();
    m_layers.clear();
    m_ct = nullptr;
    m_prev_comp_types.clear();
    m_pending_removed_layers_id.clear();
    m_unpresent_count = 0;
    m_prev_unpresent_count = 0;
}

bool HWCDisplay::isConnected() const
{
    return DisplayManager::getInstance().getDisplayData(m_disp_id)->connected;
}

bool HWCDisplay::isValidated() const
{
    return m_is_validated;
}

void HWCDisplay::removePendingRemovedLayers()
{
    AutoMutex l(m_pending_removed_layers_mutex);
    for (auto& layer_id : m_pending_removed_layers_id)
    {
        if (m_layers.find(layer_id) != m_layers.end())
        {
            auto& layer = m_layers[layer_id];
            if (layer->isVisible())
            {
                HWC_LOGE("(%" PRIu64 ") false removed layer %s", getId(), layer->toString8().string());
            }
            else
            {
#ifdef MTK_USER_BUILD
                HWC_LOGV("(%" PRIu64 ") %s: destroy layer id(%" PRIu64 ")", getId(), __func__, layer_id);
#else
                HWC_LOGD("(%" PRIu64 ") %s: destroy layer id(%" PRIu64 ")", getId(), __func__, layer_id);
#endif
            }
            layer = nullptr;
            m_layers.erase(layer_id);
        }
        else
        {
            HWC_LOGE("(%" PRIu64 ") %s: destroy layer id(%" PRIu64 ") failed", getId(), __func__, layer_id);
        }
    }
    m_pending_removed_layers_id.clear();
}

void HWCDisplay::getChangedCompositionTypes(
    uint32_t* out_num_elem,
    hwc2_layer_t* out_layers,
    int32_t* out_types) const
{
    if (out_num_elem != NULL)
        *out_num_elem = m_changed_comp_types.size();

    if (out_layers != NULL)
        for (size_t i = 0; i < m_changed_comp_types.size(); ++i)
            out_layers[i] = m_changed_comp_types[i]->getId();

    if (out_types != NULL)
    {
        for (size_t i = 0; i < m_changed_comp_types.size(); ++i)
        {
            out_types[i] = m_changed_comp_types[i]->getCompositionType();
        }
    }
}

sp<HWCLayer> HWCDisplay::getLayer(const hwc2_layer_t& layer_id)
{
    const auto& iter = m_layers.find(layer_id);
    if (iter == m_layers.end())
    {
        HWC_LOGE("(%" PRIu64 ") %s %" PRIu64, getId(), __func__, layer_id);
        for (auto& kv : m_layers)
        {
            auto& layer = kv.second;
            HWC_LOGE("(%" PRIu64 ") getLayer() %s", getId(), layer->toString8().string());
        }
        if (HWC_DISPLAY_EXTERNAL == getId())
        {
            HWC_LOGE("warning!!! external display getLayer failed!");
        }
        else
        {
            abort();
        }
    }
    return (iter == m_layers.end()) ? nullptr : iter->second;
}

void HWCDisplay::checkVisibleLayerChange(const std::vector<sp<HWCLayer> > &prev_visible_layers)
{
    m_is_visible_layer_changed = false;
    if (m_visible_layers.size() != prev_visible_layers.size())
    {
        m_is_visible_layer_changed = true;
    }
    else
    {
        for(size_t i = 0; i < prev_visible_layers.size(); i++)
        {
            if (prev_visible_layers[i]->getId() != m_visible_layers[i]->getId())
            {
                m_is_visible_layer_changed = true;
                break;
            }
        }
    }

    if (isVisibleLayerChanged())
    {
        for (auto& layer : getVisibleLayersSortedByZ())
        {
            layer->setStateChanged(true);
        }
    }
}

void HWCDisplay::buildVisibleAndInvisibleLayersSortedByZ()
{
    const std::vector<sp<HWCLayer> > prev_visible_layers(m_visible_layers);
    m_visible_layers.clear();
    {
        AutoMutex l(m_pending_removed_layers_mutex);
        for(auto &kv : m_layers)
        {
            auto& layer = kv.second;
            if (m_pending_removed_layers_id.find(kv.first) == m_pending_removed_layers_id.end() &&
                !layer->isClientTarget())
            {
                m_visible_layers.push_back(kv.second);
            }
        }
    }

    sort(m_visible_layers.begin(), m_visible_layers.end(),
        [](const sp<HWCLayer>& a, const sp<HWCLayer>& b)
        {
            return a->getZOrder() < b->getZOrder();
        });

    m_invisible_layers.clear();
    if (m_visible_layers.size() > 1 &&
        Platform::getInstance().m_config.remove_invisible_layers && m_color_transform_ok &&
        !m_use_gpu_composition)
    {
        const uint32_t black_mask = 0x0;
        for (auto iter = m_visible_layers.begin(); iter != m_visible_layers.end();)
        {
            auto& layer = (*iter);
            if (layer->getSFCompositionType() == HWC2_COMPOSITION_SOLID_COLOR &&
                (((layer->getLayerColor() << 8) >> 8) | black_mask) == 0U)
            {
                m_invisible_layers.push_back(layer);
                m_visible_layers.erase(iter);
                continue;
            }
            break;
        }
    }

    checkVisibleLayerChange(prev_visible_layers);
}

const std::vector<sp<HWCLayer> >& HWCDisplay::getVisibleLayersSortedByZ()
{
    return m_visible_layers;
}

const std::vector<sp<HWCLayer> >& HWCDisplay::getInvisibleLayersSortedByZ()
{
    return m_invisible_layers;
}

void HWCDisplay::buildCommittedLayers()
{
    auto& visible_layers = getVisibleLayersSortedByZ();
    m_committed_layers.clear();
    for(auto &layer : visible_layers)
    {
        auto& f = layer->getDisplayFrame();
        HWC_LOGV("(%" PRIu64 ")  getCommittedLayers() i:%d f[%d,%d,%d,%d] is_ct:%d comp:%s, %d",
            getId(),
            layer->getId(),
            f.left,
            f.top,
            f.right,
            f.bottom,
            layer->isClientTarget(),
            getCompString(layer->getCompositionType()),
            getHWLayerString(layer->getHwlayerType()));
        if (f.left != f.right && f.top != f.bottom &&
            !layer->isClientTarget() &&
            (layer->getCompositionType() == HWC2_COMPOSITION_DEVICE ||
             layer->getCompositionType() == HWC2_COMPOSITION_CURSOR) &&
            layer->getHwlayerType() != HWC_LAYER_TYPE_WORMHOLE)
        {
            HWC_LOGV("(%" PRIu64 ")  getCommittedLayers() i:%d added",
                getId(), layer->getId());
            m_committed_layers.push_back(layer);
        }
    }

    sp<HWCLayer> ct = getClientTarget();
    HWC_LOGV("(%" PRIu64 ")  getCommittedLayers() ct handle:%p",
        getId(), ct->getHandle());
    if (ct->getHandle() != nullptr)
        m_committed_layers.push_back(ct);
}

const std::vector<sp<HWCLayer> >& HWCDisplay::getCommittedLayers()
{
    return m_committed_layers;
}

sp<HWCLayer> HWCDisplay::getClientTarget()
{
    if (m_layers.size() < 1)
    {
        HWC_LOGE("%s: there is no client target layer at display(%" PRId64 ")", __func__, m_disp_id);
        return nullptr;
    }
    return m_ct;
}

int32_t HWCDisplay::getWidth(hwc2_config_t config) const
{
    return DisplayManager::getInstance().getDisplayData(m_disp_id, config)->width;
}

int32_t HWCDisplay::getHeight(hwc2_config_t config) const
{
    return DisplayManager::getInstance().getDisplayData(m_disp_id, config)->height;
}

int32_t HWCDisplay::getVsyncPeriod(hwc2_config_t config) const
{
    return DisplayManager::getInstance().getDisplayData(m_disp_id, config)->refresh;
}

int32_t HWCDisplay::getDpiX(hwc2_config_t config) const
{
    return DisplayManager::getInstance().getDisplayData(m_disp_id, config)->xdpi;
}

int32_t HWCDisplay::getDpiY(hwc2_config_t config) const
{
    return DisplayManager::getInstance().getDisplayData(m_disp_id, config)->ydpi;
}

uint32_t HWCDisplay::getNumConfigs() const
{
    return HWCMediator::getInstance().getOvlDevice(m_disp_id)->getNumConfigs(m_disp_id);
}

bool HWCDisplay::setActiveConfig(hwc2_config_t config)
{
    if (m_disp_id == HWC_DISPLAY_PRIMARY &&
        config < getNumConfigs() &&
        m_active_config != config)
    {
        if (DisplayManager::getInstance().getDisplayData(m_disp_id, config)->refresh !=
            DisplayManager::getInstance().getDisplayData(m_disp_id, m_active_config)->refresh)
        {
            HWVSyncEstimator::getInstance().resetAvgVSyncPeriod(
                DisplayManager::getInstance().getDisplayData(m_disp_id, config)->refresh);
        }
        HWC_LOGI("(%" PRIu64 ") %s: m_config(%d),new_config(%d)", m_disp_id, __func__, m_active_config, config);
        m_active_config = config;
        m_config_changed = true;

        return true;
    }
    HWC_LOGE("(%" PRIu64 ") set active config(%d) failure", m_disp_id, config);
    return false;
}

bool HWCDisplay::isConfigChanged()
{
    bool config_changed = m_config_changed;
    m_config_changed = false;
    return config_changed;
}

int32_t HWCDisplay::getSecure() const
{
    return DisplayManager::getInstance().getDisplayData(m_disp_id)->secure;
}

void HWCDisplay::setPowerMode(const int32_t& mode)
{
    // screen blanking based on early_suspend in the kernel
    HWC_LOGI("Display(%" PRId64 ") SetPowerMode(%d)", m_disp_id, mode);
    m_power_mode = mode;
    DisplayManager::getInstance().setDisplayPowerState(m_disp_id, mode);

    HWCDispatcher::getInstance().setPowerMode(m_disp_id, mode);

    DisplayManager::getInstance().setPowerMode(m_disp_id, mode);

    // disable mirror mode when display blanks
    if ((Platform::getInstance().m_config.mirror_state & MIRROR_DISABLED) != MIRROR_DISABLED)
    {
        if (mode == HWC2_POWER_MODE_OFF || HWC2_POWER_MODE_DOZE_SUSPEND == mode)
        {
            Platform::getInstance().m_config.mirror_state |= MIRROR_PAUSED;
        }
        else
        {
            Platform::getInstance().m_config.mirror_state &= ~MIRROR_PAUSED;
        }
    }
}

void HWCDisplay::setVsyncEnabled(const int32_t& enabled)
{
    DisplayManager::getInstance().requestVSync(m_disp_id, enabled);
}

void HWCDisplay::getType(int32_t* out_type) const
{
    *out_type = m_type;
}

int32_t HWCDisplay::createLayer(hwc2_layer_t* out_layer, const bool& is_ct)
{
    sp<HWCLayer> layer = new HWCLayer(this, getId(), is_ct);
    if(layer == nullptr)
    {
        HWC_LOGE("%s: Fail to alloc a layer", __func__);
        return HWC2_ERROR_NO_RESOURCES;
    }

    m_layers[layer->getId()] = layer;
    *out_layer = layer->getId();

    if (is_ct)
    {
        layer->setPlaneAlpha(1.0f);
        layer->setBlend(HWC2_BLEND_MODE_PREMULTIPLIED);
        layer->setHwlayerType(HWC_LAYER_TYPE_FBT, __LINE__, HWC_COMP_FILE_HWCD);
    }
    HWC_LOGD("(%" PRIu64 ") %s out_layer:%" PRIu64, getId(), __func__, *out_layer);
    return HWC2_ERROR_NONE;
}

int32_t HWCDisplay::destroyLayer(const hwc2_layer_t& layer_id)
{
    HWC_LOGD("(%" PRIu64 ") %s layer:% " PRIu64, getId(), __func__, layer_id);

    AutoMutex l(m_pending_removed_layers_mutex);
    std::pair<std::set<uint64_t>::iterator, bool> ret = m_pending_removed_layers_id.insert(layer_id);
    if (ret.second == false)
    {
        HWC_LOGE("(%" PRIu64 ") To destroy layer id(%" PRIu64 ") twice", getId(), layer_id);
    }
    return HWC2_ERROR_NONE;
}

void HWCDisplay::clearAllFences()
{
    const int32_t retire_fence_fd = getRetireFenceFd();

    // if copyvds is false, if composition type is GLES only
    // HWC must take the acquire fence of client target as retire fence,
    // so retire fence cannot be closed.
    int32_t gles_head = -1, gles_tail = -1;
    getGlesRange(&gles_head, &gles_tail);
    if (retire_fence_fd != -1 &&
        !(getId() == HWC_DISPLAY_VIRTUAL && gles_head == 0 && gles_tail == static_cast<int32_t>(getVisibleLayersSortedByZ().size() - 1) &&
          !HWCMediator::getInstance().m_features.copyvds))
    {
        protectedClose(retire_fence_fd);
        setRetireFenceFd(-1, isConnected());
    }

    if (getOutbuf() != nullptr)
    {
        const int32_t outbuf_acquire_fence_fd = getOutbuf()->getAcquireFenceFd();
        if (outbuf_acquire_fence_fd != -1)
        {
            protectedClose(outbuf_acquire_fence_fd);
            getOutbuf()->setAcquireFenceFd(-1, isConnected());
        }

        const int32_t outbuf_release_fence_fd = getOutbuf()->getReleaseFenceFd();
        if (outbuf_release_fence_fd != -1)
        {
            protectedClose(outbuf_release_fence_fd);
            getOutbuf()->setReleaseFenceFd(-1, isConnected());
        }
    }

    for (auto& kv : m_layers)
    {
        auto& layer = kv.second;
        const int32_t release_fence_fd = layer->getReleaseFenceFd();
        if (release_fence_fd != -1)
        {
            protectedClose(release_fence_fd);
            layer->setReleaseFenceFd(-1, isConnected());
        }

        const int32_t acquire_fence_fd = layer->getAcquireFenceFd();
        if (acquire_fence_fd != -1) {
            protectedClose(acquire_fence_fd);
            layer->setAcquireFenceFd(-1, isConnected());
        }
    }
}

void HWCDisplay::clearDisplayFencesAndFbtFences()
{
    const int32_t retire_fence_fd = getRetireFenceFd();

    // if copyvds is false, if composition type is GLES only
    // HWC must take the acquire fence of client target as retire fence,
    // so retire fence cannot be closed.
    if (retire_fence_fd != -1 &&
        !(getMirrorSrc() == -1 && !HWCMediator::getInstance().m_features.copyvds))
    {
        ::protectedClose(retire_fence_fd);
        setRetireFenceFd(-1, isConnected());
    }

    if (getOutbuf() != nullptr)
    {
        const int32_t outbuf_acquire_fence_fd = getOutbuf()->getAcquireFenceFd();
        if (outbuf_acquire_fence_fd != -1)
        {
            ::protectedClose(outbuf_acquire_fence_fd);
            getOutbuf()->setAcquireFenceFd(-1, isConnected());
        }

        const int32_t outbuf_release_fence_fd = getOutbuf()->getReleaseFenceFd();
        if (outbuf_release_fence_fd != -1)
        {
            protectedClose(outbuf_release_fence_fd);
            getOutbuf()->setReleaseFenceFd(-1, isConnected());
        }
    }

    auto&& ct = getClientTarget();

    const int32_t release_fence_fd = ct->getReleaseFenceFd();
    if (release_fence_fd != -1)
    {
        ::protectedClose(release_fence_fd);
        ct->setReleaseFenceFd(-1, isConnected());
    }

    const int32_t acquire_fence_fd = ct->getAcquireFenceFd();
    if (acquire_fence_fd != -1) {
        ::protectedClose(acquire_fence_fd);
        ct->setAcquireFenceFd(-1, isConnected());
    }
}

void HWCDisplay::getReleaseFenceFds(
    uint32_t* out_num_elem,
    hwc2_layer_t* out_layer,
    int32_t* out_fence_fd)
{
    static bool flip = 0;

    if (!flip)
    {
        *out_num_elem = 0;
        for (auto& kv : m_layers)
        {
            auto& layer = kv.second;

            if (layer->isClientTarget())
                continue;

            if (layer->getPrevReleaseFenceFd() != -1)
                ++(*out_num_elem);
        }
    }
    else
    {
        int32_t out_fence_fd_cnt = 0;
        for (auto& kv : m_layers)
        {
            auto& layer = kv.second;

            if (layer->isClientTarget())
                continue;

            if (layer->getPrevReleaseFenceFd() != -1)
            {
                out_layer[out_fence_fd_cnt] = layer->getId();
                const int32_t prev_rel_fd = layer->getPrevReleaseFenceFd();
#ifdef USES_FENCE_RENAME
                const int32_t hwc_to_sf_rel_fd = sync_merge("HWC_to_SF_rel", prev_rel_fd, prev_rel_fd);
                if (hwc_to_sf_rel_fd < 0)
                {
                    HWC_LOGE("(%" PRIu64 ") %s layer(% " PRId64 ") merge fence failed", getId(), __func__, layer->getId());
                }
                out_fence_fd[out_fence_fd_cnt] = hwc_to_sf_rel_fd;
#else
                out_fence_fd[out_fence_fd_cnt] = ::dup(prev_rel_fd);
#endif
                ::protectedClose(prev_rel_fd);
                layer->setPrevReleaseFenceFd(-1, isConnected());
                // just set release fence fd as -1, and do not close it!!!
                // release fences cannot close here
                ++out_fence_fd_cnt;
            }
            layer->setPrevReleaseFenceFd(layer->getReleaseFenceFd(), isConnected());
            layer->setReleaseFenceFd(-1, isConnected());
        }
        //{
        //    DbgLogger logger(DbgLogger::TYPE_HWC_LOG, 'D',"(%d) hwc2::getReleaseFenceFds() out_num_elem:%d", getId(), *out_num_elem);
        //    for (int i = 0; i < *out_num_elem; ++i)
        //        logger.printf("(layer id:%d fence fd:%d) ",
        //                out_layer[i],
        //                out_fence_fd[i]);
        //}
    }
    flip = !flip;
}

void HWCDisplay::getClientTargetSupport(
    const uint32_t& /*width*/, const uint32_t& /*height*/,
    const int32_t& /*format*/, const int32_t& /*dataspace*/)
{
    /*
    auto& layer = getClientTarget();
    if (layer != nullptr)
        layer->setFormat
    */
}

void HWCDisplay::setOutbuf(const buffer_handle_t& handle, const int32_t& release_fence_fd)
{
    HWC_LOGV("(%" PRIu64 ") HWCDisplay::setOutbuf() handle:%x release_fence_fd:%d m_outbuf:%p", getId(), handle, release_fence_fd, m_outbuf.get());
    m_outbuf->setHandle(handle);
    m_outbuf->setupPrivateHandle();
    m_outbuf->setReleaseFenceFd(release_fence_fd, isConnected());
}

void HWCDisplay::dump(String8* dump_str)
{
    dump_str->appendFormat("Display(%" PRIu64 ")\n", getId());
    dump_str->appendFormat(" visible_layers:%zu invisible_layers:%zu commit_layers:%zu",
        getVisibleLayersSortedByZ().size(),
        getInvisibleLayersSortedByZ().size(),
        getCommittedLayers().size());

    if (getClientTarget()->getHandle())
    {
        dump_str->appendFormat(" ct_handle:%p ct_fbdc:%d",
            getClientTarget()->getHandle(),
            isCompressData(&(getClientTarget()->getPrivateHandle())));
    }
    dump_str->appendFormat(" hbm:%d", getIsHBM());
    dump_str->appendFormat("\n");

    dump_str->appendFormat("+----------+--------------+-----------+-------+-------------------------------+---+------------+--------+-----------+\n");
    dump_str->appendFormat("| layer id |       handle |       fmt | blend |              comp             | tr|     ds     |c(isG2G)|    pq     |\n");
    for (auto& layer : getVisibleLayersSortedByZ())
    {
        dump_str->appendFormat("+----------+--------------+-----------+-------+-------------------------------+---+------------+--------+-----------+\n");
        dump_str->appendFormat("|%9" PRId64 " | %12p |0x%8x | %5s | %3s(%4s,%3s,%3s,%5s:%5d) | %2d| %10d |    %d(%d)|%d,%3x,%d,%d,%d|\n",
            layer->getId(),
            layer->getHandle(),
            layer->getPrivateHandle().format,
            getBlendString(layer->getBlend()),
            getCompString(layer->getCompositionType()),
            getHWLayerString(layer->getHwlayerType()),
            getCompString(layer->getSFCompositionType()),
            getCompString(layer->getLastCompTypeCallFromSF()),
            getCompFileString(layer->getHwlayerTypeFile()),
            layer->getHwlayerTypeLine(),
            layer->getTransform(),
            layer->getDataspace(),
            isCompressData(&layer->getPrivateHandle()),
            isG2GCompressData(&layer->getPrivateHandle()),
            layer->getPrivateHandle().pq_enable,
            layer->getPrivateHandle().pq_pos,
            layer->getPrivateHandle().pq_orientation,
            layer->getPrivateHandle().pq_table_idx,
            layer->isNeedPQ());
    }
    dump_str->appendFormat("+----------+--------------+-----------+-------+-------------------------------+---+------------+--------+-----------+\n");

    if (getInvisibleLayersSortedByZ().size())
    {
        dump_str->appendFormat("+----------+--------------+-----------+-------+-------------------------------+---+------------+--------+-----------+\n");
        dump_str->appendFormat("| layer id |       handle |       fmt | blend |              comp             | tr|     ds     |c(isG2G)|    pq     |\n");
        for (auto& layer : getInvisibleLayersSortedByZ())
        {
            dump_str->appendFormat("+----------+--------------+-----------+-------+-------------------------------+---+------------+--------+-----------+\n");
            dump_str->appendFormat("|%9" PRId64 " | %12p |0x%8x | %5s | %3s(%4s,%3s,%3s,%5s:%5d) | %2d| %10d |    %d(%d)|%d,%3x,%d,%d,%d|\n",
                layer->getId(),
                layer->getHandle(),
                layer->getPrivateHandle().format,
                getBlendString(layer->getBlend()),
                getCompString(layer->getCompositionType()),
                getHWLayerString(layer->getHwlayerType()),
                getCompString(layer->getSFCompositionType()),
                getCompString(layer->getLastCompTypeCallFromSF()),
                getCompFileString(layer->getHwlayerTypeFile()),
                layer->getHwlayerTypeLine(),
                layer->getTransform(),
                layer->getDataspace(),
                isCompressData(&layer->getPrivateHandle()),
                isG2GCompressData(&layer->getPrivateHandle()),
                layer->getPrivateHandle().pq_enable,
                layer->getPrivateHandle().pq_pos,
                layer->getPrivateHandle().pq_orientation,
                layer->getPrivateHandle().pq_table_idx,
                layer->isNeedPQ());
        }
        dump_str->appendFormat("+----------+--------------+-----------+-------+-------------------------------+---+------------+--------+-----------+\n");
    }

    // This code flow is used for dump all display layers
    // If need this information, please enable this flow.

    /*
    dump_str->appendFormat("+----------+--------------+---------+-------+---------------------+---+------------+-+----|\n");
    dump_str->appendFormat("| layer id |       handle |     fmt | blend |           comp      | tr|     ds     |c| ct |\n");
    for (auto& kv : m_layers)
    {
        auto& layer = kv.second;
        dump_str->appendFormat("+----------+--------------+---------+-------+---------------------+---+------------+-+----|\n");
        dump_str->appendFormat("|%9" PRId64 " | %12p |%8x | %5s | %3s(%4s,%3s,%5d) | %2d| %10d |%d|%3d|\n",
                layer->getId(),
                layer->getHandle(),
                layer->getPrivateHandle().format,
                getBlendString(layer->getBlend()),
                getCompString(layer->getCompositionType()),
                getHWLayerString(layer->getHwlayerType()),
                getCompString(layer->getSFCompositionType()),
                layer->getHwlayerTypeLine(),
                layer->getTransform(),
                layer->getDataspace(),
                isCompressData(&layer->getPrivateHandle()),
                layer->isClientTarget()
                );
    }

    dump_str->appendFormat("+----------+--------------+---------+-------+---------------------+---+------------+-+\n");
    */

    mFpsCounter.dump(dump_str, "    ");
}

bool HWCDisplay::needDoAvGrouping(const int32_t num_validate_display)
{
    if (!Platform::getInstance().m_config.av_grouping)
    {
        m_need_disable_vsync_offset = false;
        m_need_av_grouping = false;
        return m_need_av_grouping;
    }

    m_need_disable_vsync_offset = false;
    m_need_av_grouping = false;

    if ((getId() == HWC_DISPLAY_PRIMARY) && (num_validate_display == 1)) {
        auto&& layers = getVisibleLayersSortedByZ();
        int num_video_layer = 0;
        int num_secure_video_layer = 0;
        for (size_t i = 0; i < layers.size(); i++) {
            sp<HWCLayer> layer = layers[i];
            int type = layer->getPrivateHandle().ext_info.status & GRALLOC_EXTRA_MASK_TYPE;
            if (type == GRALLOC_EXTRA_BIT_TYPE_VIDEO) {
                num_video_layer++;
                const int& usage = layer->getPrivateHandle().usage;
                if (usage & GRALLOC_USAGE_SECURE)
                    num_secure_video_layer++;
            }
        }
        if (num_video_layer == 1) {
            m_need_disable_vsync_offset = true;
            if (num_secure_video_layer > 0)
                m_need_av_grouping = true;
        }
    }

    return m_need_av_grouping;
}

bool HWCDisplay::isForceGpuCompose()
{
    const int32_t disp_id = static_cast<int32_t>(getId());
    if (getColorTransformHint() != HAL_COLOR_TRANSFORM_IDENTITY &&
        !m_color_transform_ok)
    {
        return true;
    }

    if (!(Platform::getInstance().m_config.disable_color_transform_for_secondary_displays))
    {
        sp<HWCDisplay> vir_hwc_disp = HWCMediator::getInstance().getHWCDisplay(HWC_DISPLAY_VIRTUAL);
        if (getId() == HWC_DISPLAY_PRIMARY &&
            vir_hwc_disp != nullptr &&
            vir_hwc_disp->isValid() &&
            getColorTransformHint() != HAL_COLOR_TRANSFORM_IDENTITY)
        {
            return true;
        }

        sp<HWCDisplay> pri_hwc_disp = HWCMediator::getInstance().getHWCDisplay(HWC_DISPLAY_PRIMARY);
        if (getId() == HWC_DISPLAY_VIRTUAL &&
            pri_hwc_disp != nullptr &&
            pri_hwc_disp->getColorTransformHint() != HAL_COLOR_TRANSFORM_IDENTITY)
        {
            return true;
        }
    }

    if (DisplayManager::getInstance().getDisplayData(disp_id)->subtype == HWC_DISPLAY_EPAPER)
    {
        return true;
    }

    if (getId() > HWC_DISPLAY_PRIMARY &&
        !Platform::getInstance().m_config.is_support_ext_path_for_virtual)
    {
        return true;
    }

    if (m_use_gpu_composition)
    {
        return true;
    }

    bool is_AOD = false;
    auto&& layers = getVisibleLayersSortedByZ();
    for (auto& layer : layers)
    {
        if (layer->getPrivateHandle().layer_type == HWC_LAYER_TYPE_AOD)
        {
            is_AOD = true;
            break;
        }
    }

    if (getIsHBM() && !is_AOD)
    {
        return true;
    }

    return false;
}

void HWCDisplay::setupPrivateHandleOfLayers()
{
    ATRACE_CALL();
    auto layers = getVisibleLayersSortedByZ();

    if (layers.size() > 0)
    {
        // RGBA layer_0, alpha value don't care.
        auto& layer = layers[0];
        if (layer != nullptr &&
            layer->getHwcBuffer() != nullptr &&
            layer->getHwcBuffer()->getHandle() != nullptr &&
            layer->getPrivateHandle().format == HAL_PIXEL_FORMAT_RGBA_8888)
        {
            layer->getEditablePrivateHandle().format = HAL_PIXEL_FORMAT_RGBX_8888;
        }
    }

    for (auto& layer : layers)
    {
        const unsigned int prev_format = layer->getPrivateHandle().format;
#ifndef MTK_USER_BUILD
        char str[256];
        snprintf(str, 256, "setupPrivateHandle %d %d(%p)", layer->isStateChanged(), layer->isBufferChanged(), layer->getHandle());
        ATRACE_NAME(str);
#endif
        layer->setPrevIsPQEnhance(layer->getPrivateHandle().pq_enable);
        int err = setPrivateHandlePQInfo(layer->getHandle(), &(layer->getEditablePrivateHandle()));
        if (err != GRALLOC_EXTRA_OK)
        {
            HWC_LOGE("setPrivateHandlePQInfo err(%x), (handle=%p)", err, layer->getHandle());
        }

        if (Platform::getInstance().m_config.always_setup_priv_hnd ||
            layer->isStateChanged() || layer->isBufferChanged())
        {
#ifndef MTK_USER_BUILD
            ATRACE_NAME("setupPrivateHandle");
#endif
            layer->setupPrivateHandle();

            //const int type = (layer->getPrivateHandle().ext_info.status & GRALLOC_EXTRA_MASK_TYPE);
            //if (!(type == GRALLOC_EXTRA_BIT_TYPE_VIDEO && Platform::getInstance().m_config.fill_hwdec_hdr))
            //{
            //    HwcHdrUtils::getInstance().fillMetadatatoGrallocExtra(layer->getPerFrameMetadata(),
            //        layer->getPrivateHandle(), layer->getDataspace());
            //}
        }

        // opaque RGBA layer can be processed as RGBX
        if (layer->getBlend() == HWC2_BLEND_MODE_NONE &&
            layer->getPrivateHandle().format == HAL_PIXEL_FORMAT_RGBA_8888)
        {
            layer->getEditablePrivateHandle().format = HAL_PIXEL_FORMAT_RGBX_8888;
        }

        if (prev_format != layer->getPrivateHandle().format)
        {
            layer->setStateChanged(true);
        }
    }
}

void HWCDisplay::setOverrideMDPOutputFormatOfLayers()
{
    auto layers = getVisibleLayersSortedByZ();
    for (auto& layer : layers)
    {
        if (HWCMediator::getInstance().m_features.is_support_pq &&
            HWCMediator::getInstance().m_features.video_enhancement &&
            HWCMediator::getInstance().m_features.video_transition)
        {
            if ((layer->getHwlayerType() == HWC_LAYER_TYPE_MM) &&
                (getId() == HWC_DISPLAY_PRIMARY) &&
                (HWC_MIRROR_SOURCE_INVALID == getMirrorSrc()) &&
                layer->getPrivateHandle().pq_enable)
            {
                layer->setOverrideMDPOutputFormat(DISP_FORMAT_YUV422);
            }
            else
            {
                layer->setOverrideMDPOutputFormat(DISP_FORMAT_UNKNOWN);
            }
        }
    }
}

void HWCDisplay::setValiPresentState(HWC_VALI_PRESENT_STATE val, const int32_t& line)
{
    DispatcherJob* job = HWCDispatcher::getInstance().getExistJob(static_cast<int32_t>(getId()));
    AbortMessager::getInstance().printf("(%" PRIu64 ") (L%d) set s:%s jobID:%d",
        m_disp_id, line, getPresentValiStateString(val), (job) ? job->sequence : -1);
    m_vali_present_state = val;
}

void HWCDisplay::updateFps()
{
    if (mFpsCounter.update())
    {
        int32_t type = 0;
        getType(&type);
        HWC_LOGI("[Display_%" PRIu64 " (type:%d)] fps:%f,dur:%.2f,max:%.2f,min:%.2f",
                getId(), type, mFpsCounter.getFps(), mFpsCounter.getLastLogDuration() / 1e6,
                mFpsCounter.getMaxDuration() / 1e6, mFpsCounter.getMinDuration() / 1e6);
    }
}

void HWCDisplay::addUnpresentCount()
{
    m_prev_unpresent_count = m_unpresent_count;
    ++m_unpresent_count;
}

void HWCDisplay::decUnpresentCount()
{
    m_prev_unpresent_count = m_unpresent_count;
    --m_unpresent_count;
    if (m_unpresent_count < 0)
    {
        HWC_LOGE("%s error prepareFrame count(%" PRIu64 ":%d)", __func__, getId(), m_unpresent_count);
        m_unpresent_count = 0;
    }
}

void HWCDisplay::buildVisibleAndInvisibleLayer()
{
    removePendingRemovedLayers();
    buildVisibleAndInvisibleLayersSortedByZ();
    setupPrivateHandleOfLayers();
}

