#define DEBUG_LOG_TAG "HWCLayer"
#include "hwclayer.h"

#include "overlay.h"
#include "dispatcher.h"
#include "hwcdisplay.h"
#include "platform.h"
#include "utils/transform.h"

std::atomic<int64_t> HWCLayer::id_count(0);

HWCLayer::HWCLayer(const wp<HWCDisplay>& disp, const uint64_t& disp_id, const bool& is_ct)
    : m_mtk_flags(0)
    , m_id(++id_count)
    , m_is_ct(is_ct)
    , m_disp(disp)
    , m_hwlayer_type(HWC_LAYER_TYPE_NONE)
    , m_hwlayer_type_line(-1)
    , m_hwlayer_type_file(HWC_COMP_FILE_UNK)
    , m_sf_comp_type(0)
    , m_dataspace(0)
    , m_blend(HWC2_BLEND_MODE_NONE)
    , m_plane_alpha(0.0f)
    , m_z_order(0)
    , m_transform(0)
    , m_state_changed(false)
    , m_disp_id(disp_id)
    , m_hwc_buf(new HWCBuffer(m_disp_id, m_id, is_ct))
    , m_is_visible(false)
    , m_sf_comp_type_call_from_sf(0)
    , m_last_comp_type_call_from_sf(0)
    , m_layer_caps(0)
    , m_layer_color(0)
    , m_prev_pq_enable(false)
    , m_is_override_MDP_output_format(DISP_FORMAT_UNKNOWN)
    , m_need_pq(false)
    , m_last_app_game_pq(false)
    , m_has_set_perf(false)
{
    memset(&m_damage, 0, sizeof(m_damage));
    memset(&m_display_frame, 0, sizeof(m_display_frame));
    memset(&m_source_crop, 0, sizeof(m_source_crop));
    memset(&m_visible_region, 0, sizeof(m_visible_region));
    memset(&m_mdp_dst_roi, 0, sizeof(m_mdp_dst_roi));

    if (m_hwc_buf == nullptr)
        HWC_LOGE("%s allocate HWCBuffer for m_hwc_buf fail", __func__);
}

HWCLayer::~HWCLayer()
{
    if (m_damage.rects != nullptr)
        free((void*)m_damage.rects);

    if (m_visible_region.rects != nullptr)
        free((void*)m_visible_region.rects);
}

#define SET_LINE_NUM(RTLINE, TYPE) ({ \
                            *RTLINE = __LINE__; \
                            TYPE; \
                        })

String8 HWCLayer::toString8()
{
    auto& display_frame = getDisplayFrame();
    auto& src_crop = getSourceCrop();

    String8 ret;
    ret.appendFormat("id:%" PRIu64 " v:%d acq:%d hnd:%p,%d,%" PRIu64 " w:%d,%d h:%d,%d f:%u sz:%d z:%u c:%x %s(%s,%s%d,%s,%d) s[%.1f,%.1f,%.1f,%.1f]->d[%d,%d,%d,%d] t:%d d(s%d,b%d) fbdc:%d(isG2G:%d) pq:%d",
        getId(),
        isVisible(),
        getAcquireFenceFd(),
        getHandle(),
        getPrivateHandle().ion_fd,
        getPrivateHandle().alloc_id,
        getPrivateHandle().width,
        getPrivateHandle().y_stride,
        getPrivateHandle().height,
        getPrivateHandle().vstride,
        getPrivateHandle().format,
        getPrivateHandle().size,
        getZOrder(),
        getLayerColor(),
        getCompString(getCompositionType()),
        getHWLayerString(getHwlayerType()),
        getCompString(getSFCompositionType()),
        isSFCompositionTypeCallFromSF(),
        getCompString(getLastCompTypeCallFromSF()),
        getHwlayerTypeLine(),
        src_crop.left,
        src_crop.top,
        src_crop.right,
        src_crop.bottom,
        display_frame.left,
        display_frame.top,
        display_frame.right,
        display_frame.bottom,
        getTransform(),
        isStateChanged(),
        isBufferChanged(),
        isCompressData(&getPrivateHandle()),
        isG2GCompressData(&getPrivateHandle()),
        isNeedPQ());
    return ret;
}

// return final transform rectify with prexform
uint32_t HWCLayer::getXform() const
{
    uint32_t xform = getTransform();
    const PrivateHandle& priv_hnd = getPrivateHandle();
    rectifyXformWithPrexform(&xform, priv_hnd.prexform);
    return xform;
}

bool HWCLayer::needRotate() const
{
    return getXform() != Transform::ROT_0;
}

bool HWCLayer::needScaling() const
{
    if (getXform() & HAL_TRANSFORM_ROT_90)
    {
        return (WIDTH(getSourceCrop()) != HEIGHT(getDisplayFrame())) ||
                (HEIGHT(getSourceCrop()) != WIDTH(getDisplayFrame()));
    }

    return (WIDTH(getSourceCrop()) != WIDTH(getDisplayFrame())) ||
            (HEIGHT(getSourceCrop()) != HEIGHT(getDisplayFrame()));
}

void HWCLayer::validate()
{
    const int& compose_level = Platform::getInstance().m_config.compose_level;
    int32_t line = -1;
    // isUILayerValid and isMMLayerValid should set the file value, then
    // we can know the reason. However it need to modify the interface and
    // implementation of whole platform. We will do it later.
    HWC_COMP_FILE file = HWC_COMP_FILE_NSET;

    if (getSFCompositionType() == HWC2_COMPOSITION_CLIENT)
    {
        setHwlayerType(HWC_LAYER_TYPE_INVALID, __LINE__, HWC_COMP_FILE_HWCL);
        return;
    }

    if (getSFCompositionType() == HWC2_COMPOSITION_SOLID_COLOR)
    {
        if (WIDTH(m_display_frame) <= 0 || HEIGHT(m_display_frame) <= 0)
        {
            setHwlayerType(HWC_LAYER_TYPE_INVALID, __LINE__, HWC_COMP_FILE_HWCL);
            return;
        }
        setHwlayerType(HWC_LAYER_TYPE_DIM, __LINE__, HWC_COMP_FILE_HWCL);

        return;
    }

    // checking handle cannot be placed before checking dim layer
    // because handle of dim layer is nullptr.
    if (getHwcBuffer() == nullptr || getHwcBuffer()->getHandle() == nullptr)
    {
        setHwlayerType(HWC_LAYER_TYPE_INVALID, __LINE__, HWC_COMP_FILE_HWCL);
        return;
    }

    // for drm video
    sp<HWCDisplay> disp = m_disp.promote();
    if (disp == NULL)
        HWC_LOGE("%s: HWCDisplay Promoting failed!", __func__);

    const int32_t buffer_type = (getPrivateHandle().ext_info.status & GRALLOC_EXTRA_MASK_TYPE);

    if ((getPrivateHandle().usage & GRALLOC_USAGE_PROTECTED) && !disp->getSecure())
    {
        setHwlayerType(HWC_LAYER_TYPE_INVALID, __LINE__, HWC_COMP_FILE_HWCL);
        return;
    }

    if (getPrivateHandle().usage & GRALLOC_USAGE_SECURE)
    {
        if (buffer_type == GRALLOC_EXTRA_BIT_TYPE_VIDEO ||
            buffer_type == GRALLOC_EXTRA_BIT_TYPE_CAMERA ||
            getPrivateHandle().format == HAL_PIXEL_FORMAT_YV12)
        {
            // for MM case
            if (DisplayManager::getInstance().getVideoHdcp() >
                DisplayManager::getInstance().getDisplayData(disp->getId())->hdcp_version)
            {
                setHwlayerType(HWC_LAYER_TYPE_INVALID, __LINE__, HWC_COMP_FILE_HWCL);
                return;
            }
        }
        else if (DisplayManager::getInstance().getDisplayData(disp->getId())->hdcp_version > 0 &&
                 disp->getId() != HWC_DISPLAY_PRIMARY)
        {
            // for UI case
            setHwlayerType(HWC_LAYER_TYPE_INVALID, __LINE__, HWC_COMP_FILE_HWCL);
            return;
        }
    }

    // to debug which layer has been selected as UIPQ layer
    if (HWCMediator::getInstance().m_features.global_pq &&
        (compose_level & COMPOSE_DISABLE_UI) == 0 &&
        HWC_DISPLAY_PRIMARY == m_disp_id &&
        (getPrivateHandle().ext_info.status2 & GRALLOC_EXTRA_BIT2_UI_PQ_ON) &&
        Platform::getInstance().m_config.uipq_debug)
    {
        setHwlayerType(HWC_LAYER_TYPE_UIPQ_DEBUG, __LINE__, HWC_COMP_FILE_HWCL);
        return;
    }

    // for ui layer
    if ((compose_level & COMPOSE_DISABLE_UI) == 0 &&
        Platform::getInstance().isUILayerValid(this, &line))
    {
        if (getSFCompositionType() == HWC2_COMPOSITION_CURSOR)
        {
            setHwlayerType(HWC_LAYER_TYPE_CURSOR, __LINE__, HWC_COMP_FILE_HWCL);
            return;
        }

        if (HWCMediator::getInstance().m_features.global_pq &&
            HWC_DISPLAY_PRIMARY == m_disp_id &&
            (getPrivateHandle().ext_info.status2 & GRALLOC_EXTRA_BIT2_UI_PQ_ON) &&
            WIDTH(m_display_frame) > 1 &&
            HEIGHT(m_display_frame) > 1)
        {
            setHwlayerType(HWC_LAYER_TYPE_UIPQ, __LINE__, HWC_COMP_FILE_HWCL);
            return;
        }

        setHwlayerType(HWC_LAYER_TYPE_UI, __LINE__, HWC_COMP_FILE_HWCL);
        return;
    }

    // for mdp layer
    if (compose_level & COMPOSE_DISABLE_MM)
    {
        setHwlayerType(HWC_LAYER_TYPE_INVALID, __LINE__, HWC_COMP_FILE_HWCL);
        return;
     }

    if (Platform::getInstance().isMMLayerValid(this, &line))
    {
        setHwlayerType(HWC_LAYER_TYPE_MM, __LINE__, HWC_COMP_FILE_HWCL);
        return;
    }

    setHwlayerType(HWC_LAYER_TYPE_INVALID, line == -1 ? line : line + 10000, file);
}

int32_t HWCLayer::afterPresent(const bool& is_disp_connected)
{
    // HWCLayer should check its acquire fence first!
    // SF may give a layer with zero width or height, and the layer is not in the
    // getVisibleLayersSortedByZ(). Therefore, its acquire fence is not processed.
    // HWC2 should process the acquire fence via afterPresent(). the task should
    // be done by HWCLayer::afterPresent() because HWCBuffer does NOT have display
    // frame information.
    int32_t needAbort = 0;
    if (getAcquireFenceFd() > -1)
    {
        auto& f = getDisplayFrame();
        if (f.left == f.right || f.top == f.bottom)
        {
            ::protectedClose(getAcquireFenceFd());
            setAcquireFenceFd(-1, is_disp_connected);
        }
        else
        {
            if (is_disp_connected)
            {
                HWC_LOGE("(%" PRIu64 ") unclose acquire fd(%d) of layer(%d)", m_disp_id, getAcquireFenceFd(), getId());
                return 1;
            }
            else
            {
                HWC_LOGW("(%" PRIu64 ") unclose acquire fd(%d) of layer(%d)", m_disp_id, getAcquireFenceFd(), getId());
                ::protectedClose(getAcquireFenceFd());
                setAcquireFenceFd(-1, is_disp_connected);
            }
        }
    }

    // Careful!!! HWCBuffer::afterPresent() should be behind of the layer with zero
    // width or height checking
    if (getHwcBuffer() != nullptr)
        needAbort = getHwcBuffer()->afterPresent(is_disp_connected, isClientTarget());

    setStateChanged(false);
    setVisible(false);
    setLastAppGamePQ(isNeedPQ());
    return needAbort;
}

void HWCLayer::toBeDim()
{
    m_priv_hnd.format = HAL_PIXEL_FORMAT_DIM;
}

int32_t HWCLayer::getCompositionType() const
{
    switch (m_hwlayer_type) {
        case HWC_LAYER_TYPE_NONE:
            return HWC2_COMPOSITION_INVALID;

        case HWC_LAYER_TYPE_INVALID:
            return HWC2_COMPOSITION_CLIENT;

        case HWC_LAYER_TYPE_FBT:
        case HWC_LAYER_TYPE_UI:
        case HWC_LAYER_TYPE_MM:
        case HWC_LAYER_TYPE_DIM:
        case HWC_LAYER_TYPE_MM_FBT:
        case HWC_LAYER_TYPE_UIPQ_DEBUG:
        case HWC_LAYER_TYPE_UIPQ:
            return HWC2_COMPOSITION_DEVICE;

        case HWC_LAYER_TYPE_CURSOR:
            return HWC2_COMPOSITION_CURSOR;

        case HWC_LAYER_TYPE_WORMHOLE:
            return HWC2_COMPOSITION_DEVICE;
    }
    return HWC2_COMPOSITION_CLIENT;
};

void HWCLayer::setSFCompositionType(const int32_t& sf_comp_type, const bool& call_from_sf)
{
    m_sf_comp_type = sf_comp_type;
    m_sf_comp_type_call_from_sf = call_from_sf;

    if (call_from_sf)
    {
        m_last_comp_type_call_from_sf = sf_comp_type;
    }
}

void HWCLayer::setHandle(const buffer_handle_t& hnd)
{
    m_hwc_buf->setHandle(hnd);
}

void HWCLayer::setReleaseFenceFd(const int32_t& fence_fd, const bool& is_disp_connected)
{
    m_hwc_buf->setReleaseFenceFd(fence_fd, is_disp_connected);
}

void HWCLayer::setPrevReleaseFenceFd(const int32_t& fence_fd, const bool& is_disp_connected)
{
    m_hwc_buf->setPrevReleaseFenceFd(fence_fd, is_disp_connected);
}

void HWCLayer::setAcquireFenceFd(const int32_t& acquire_fence_fd, const bool& is_disp_connected)
{
    m_hwc_buf->setAcquireFenceFd(acquire_fence_fd, is_disp_connected);
}

void HWCLayer::setDataspace(const int32_t& dataspace)
{
    if (m_dataspace != dataspace)
    {
        setStateChanged(true);
        m_dataspace = dataspace;
    }
}

void HWCLayer::setDamage(const hwc_region_t& damage)
{
    if (!isHwcRegionEqual(m_damage, damage))
    {
        copyHwcRegion(&m_damage, damage);
    }
}

void HWCLayer::setBlend(const int32_t& blend)
{
    if (m_blend != blend)
    {
        setStateChanged(true);
        m_blend = blend;
    }
}

void HWCLayer::setDisplayFrame(const hwc_rect_t& display_frame)
{
    if (memcmp(&m_display_frame, &display_frame, sizeof(hwc_rect_t)) != 0)
    {
        setStateChanged(true);
        m_display_frame = display_frame;
    }
}

void HWCLayer::setSourceCrop(const hwc_frect_t& source_crop)
{
    if (memcmp(&m_source_crop, &source_crop, sizeof(hwc_frect_t)) != 0)
    {
        setStateChanged(true);
        m_source_crop = source_crop;
    }
}

void HWCLayer::setPlaneAlpha(const float& plane_alpha)
{
    if (m_plane_alpha != plane_alpha)
    {
        setStateChanged(true);
        m_plane_alpha = plane_alpha;
    }
}

void HWCLayer::setZOrder(const uint32_t& z_order)
{
    if (m_z_order != z_order)
    {
        setStateChanged(true);
        m_z_order = z_order;
    }
}

void HWCLayer::setTransform(const int32_t& transform)
{
    if (m_transform != transform)
    {
        setStateChanged(true);
        m_transform = transform;
    }
}

void HWCLayer::setVisibleRegion(const hwc_region_t& visible_region)
{
    if (!isHwcRegionEqual(m_visible_region, visible_region))
    {
        setStateChanged(true);
        copyHwcRegion(&m_visible_region, visible_region);
    }
}

void HWCLayer::setLayerColor(const hwc_color_t& color)
{
    uint32_t new_color = color.a << 24 | color.r << 16 | color.g << 8 | color.b;
    if (m_layer_color != new_color)
    {
        setStateChanged(true);
        m_layer_color = new_color;
    }
}

void HWCLayer::initLayerCaps()
{
    m_layer_caps = 0;
    if ((getPrivateHandle().usage & GRALLOC_USAGE_SECURE) ||
        (getPrivateHandle().usage & GRALLOC_USAGE_PROTECTED))
    {
        m_layer_caps |= HWC_LAYERING_OVL_ONLY;
    }
}

void HWCLayer::setPerFrameMetadata(uint32_t numElements, const int32_t* keys, const float* metadata)
{
    if (HwcHdrUtils::getInstance().shouldChangeMetadata(numElements, keys, metadata, m_metadata))
    {
        setStateChanged(true);
        HwcHdrUtils::getInstance().setMetadata(numElements, keys, metadata, &m_metadata);
    }
}

void HWCLayer::setPrevIsPQEnhance(const bool& val)
{
    m_prev_pq_enable = val;
}

bool HWCLayer::getPrevIsPQEnhance() const
{
    return m_prev_pq_enable;
}

bool HWCLayer::isNeedPQ()
{
    if (!HWCMediator::getInstance().m_features.game_pq)
    {
        return false;
    }

    const PrivateHandle& priv_hnd = getPrivateHandle();
    if (priv_hnd.pq_info.version > 0)
    {
        return true;
    }

    return m_need_pq;
}
