#ifndef HWC_HWCLAYER_H
#define HWC_HWCLAYER_H

#include <utils/RefBase.h>
#include <linux/disp_session.h>
#include "utils/tools.h"

#include "hwc2_defs.h"
#include "hwcbuffer.h"
#include "hdr.h"

class HWCLayer : public android::LightRefBase<HWCLayer>
{
public:
    static std::atomic<int64_t> id_count;

    HWCLayer(const wp<HWCDisplay>& disp, const uint64_t& disp_id, const bool& is_ct);
    ~HWCLayer();

    uint64_t getId() const { return m_id; };

    bool isClientTarget() const { return m_is_ct; }

    wp<HWCDisplay> getDisplay() { return m_disp; }

    void validate();
    int32_t afterPresent(const bool& is_disp_connected);

    void setHwlayerType(const int32_t& hwlayer_type, const int32_t& line, const HWC_COMP_FILE& file)
    {
        m_hwlayer_type = hwlayer_type;
        m_hwlayer_type_line = line;
        m_hwlayer_type_file = file;
    }

    int32_t getHwlayerType() const { return m_hwlayer_type; }
    int32_t getHwlayerTypeLine() const { return m_hwlayer_type_line; }
    HWC_COMP_FILE getHwlayerTypeFile() const { return m_hwlayer_type_file; }

    int32_t getCompositionType() const;

    void setSFCompositionType(const int32_t& sf_comp_type, const bool& call_from_sf);
    int32_t getSFCompositionType() const { return m_sf_comp_type; }
    int32_t getLastCompTypeCallFromSF() const { return m_last_comp_type_call_from_sf; }
    bool isSFCompositionTypeCallFromSF() const { return m_sf_comp_type_call_from_sf; }

    void setHandle(const buffer_handle_t& hnd);
    buffer_handle_t getHandle() { return m_hwc_buf->getHandle(); }

    const PrivateHandle& getPrivateHandle() const { return m_hwc_buf->getPrivateHandle(); }

    PrivateHandle& getEditablePrivateHandle() { return m_hwc_buf->getEditablePrivateHandle(); }

    void setReleaseFenceFd(const int32_t& fence_fd, const bool& is_disp_connected);
    int32_t getReleaseFenceFd() { return m_hwc_buf->getReleaseFenceFd(); }

    void setPrevReleaseFenceFd(const int32_t& fence_fd, const bool& is_disp_connected);
    int32_t getPrevReleaseFenceFd() { return m_hwc_buf->getPrevReleaseFenceFd(); }

    void setAcquireFenceFd(const int32_t& acquire_fence_fd, const bool& is_disp_connected);
    int32_t getAcquireFenceFd() { return m_hwc_buf->getAcquireFenceFd(); }

    void setDataspace(const int32_t& dataspace);
    int32_t getDataspace() { return m_dataspace; }

    void setDamage(const hwc_region_t& damage);
    const hwc_region_t& getDamage() { return m_damage; }

    void setBlend(const int32_t& blend);
    int32_t getBlend() { return m_blend; }

    void setDisplayFrame(const hwc_rect_t& display_frame);
    const hwc_rect_t& getDisplayFrame() const { return m_display_frame; }

    void setSourceCrop(const hwc_frect_t& source_crop);
    const hwc_frect_t& getSourceCrop() const { return m_source_crop; }

    void setZOrder(const uint32_t& z_order);
    uint32_t getZOrder() { return m_z_order; }

    void setPlaneAlpha(const float& plane_alpha);
    float getPlaneAlpha() { return m_plane_alpha; }

    void setTransform(const int32_t& transform);
    int32_t getTransform() const { return m_transform; }

    void setVisibleRegion(const hwc_region_t& visible_region);
    const hwc_region_t& getVisibleRegion() { return m_visible_region; }

    void setBufferChanged(const bool& changed) { return m_hwc_buf->setBufferChanged(changed); }
    bool isBufferChanged() const { return m_hwc_buf->isBufferChanged(); }

    void setStateChanged(const bool& changed) { m_state_changed = changed; }
    bool isStateChanged() const { return m_state_changed; }

    void setForcePQ(const bool& is_force_pq) { m_need_pq = is_force_pq; }
    bool isNeedPQ();

    void setMtkFlags(const int64_t& mtk_flags) {m_mtk_flags = mtk_flags; }
    int64_t getMtkFlags() const { return m_mtk_flags; }

    void setLayerColor(const hwc_color_t& color);
    uint32_t getLayerColor() { return m_layer_color; }

    const hwc_rect_t& getMdpDstRoi() const { return m_mdp_dst_roi; }
    hwc_rect_t& editMdpDstRoi() { return m_mdp_dst_roi; }

    sp<HWCBuffer> getHwcBuffer() { return m_hwc_buf; }

    void toBeDim();

    void setupPrivateHandle()
    {
        m_hwc_buf->setupPrivateHandle();
        if (m_hwc_buf->isPrexformChanged())
            setStateChanged(true);
    }

    void setVisible(const bool& is_visible) { m_is_visible = is_visible; }
    bool isVisible() const { return m_is_visible; }

    String8 toString8();

    // return final transform rectify with prexform
    uint32_t getXform() const;
    bool needRotate() const;
    bool needScaling() const;

    void initLayerCaps();
    void setLayerCaps(const int32_t& layer_caps) { m_layer_caps = layer_caps; }
    int32_t getLayerCaps() const { return m_layer_caps; }

    void setPerFrameMetadata(uint32_t numElements, const int32_t* keys, const float* metadata);
    HwcHdrMetadata getPerFrameMetadata() { return m_metadata; }

    void setPrevIsPQEnhance(const bool& val);
    bool getPrevIsPQEnhance() const;

    void setOverrideMDPOutputFormat(const DISP_FORMAT& val) { m_is_override_MDP_output_format = val; }
    DISP_FORMAT getOverrideMDPOutputFormat() const { return m_is_override_MDP_output_format; }

    void setLastAppGamePQ(const bool& app_game_pq) { m_last_app_game_pq = app_game_pq; }
    bool getLastAppGamePQ() const { return m_last_app_game_pq; }

    void setPerfState(const bool& has_set_perf) { m_has_set_perf = has_set_perf; }
    bool getPerfState() const { return m_has_set_perf; }
private:
    int64_t m_mtk_flags;

    int64_t m_id;

    const bool m_is_ct;

    wp<HWCDisplay> m_disp;

    int32_t m_hwlayer_type;
    int32_t m_hwlayer_type_line;
    HWC_COMP_FILE m_hwlayer_type_file;

    int32_t m_sf_comp_type;

    int32_t m_dataspace;

    hwc_region_t m_damage;

    int32_t m_blend;

    hwc_rect_t m_display_frame;

    hwc_frect_t m_source_crop;

    float m_plane_alpha;

    uint32_t m_z_order;

    int32_t m_transform;

    hwc_region_t m_visible_region;

    bool m_state_changed;

    PrivateHandle m_priv_hnd;

    hwc_rect_t m_mdp_dst_roi;

    uint64_t m_disp_id;

    sp<HWCBuffer> m_hwc_buf;

    bool m_is_visible;

    bool m_sf_comp_type_call_from_sf;

    int32_t m_last_comp_type_call_from_sf;

    int32_t m_layer_caps;

    uint32_t m_layer_color;

    HwcHdrMetadata m_metadata;

    bool m_prev_pq_enable;

    DISP_FORMAT m_is_override_MDP_output_format;

    bool m_need_pq;

    bool m_last_app_game_pq;

    bool m_has_set_perf;
};

#endif
