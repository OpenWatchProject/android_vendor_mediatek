#ifndef HWC_HWCDISPLAY_H
#define HWC_HWCDISPLAY_H

#include <vector>
#include <utils/RefBase.h>
#include <hardware/hwcomposer2.h>

#include "utils/tools.h"
#include "utils/fpscounter.h"

#include "hwcbuffer.h"
#include "hwclayer.h"
#include "hwc2_defs.h"
#include "color.h"

class HWCDisplay : public RefBase
{
public:
    HWCDisplay(const int64_t& disp_id, const int32_t& type);

    void init();

    void validate();
    void beforePresent(const int32_t num_validate_display);

    void present();

    void afterPresent();

    void clear();

    bool isConnected() const;

    bool isValidated() const;

    void getChangedCompositionTypes(
        uint32_t* out_num_elem,
        hwc2_layer_t* out_layers,
        int32_t* out_types) const;

    sp<HWCLayer> getLayer(const hwc2_layer_t& layer_id);

    const std::vector<sp<HWCLayer> >& getVisibleLayersSortedByZ();
    const std::vector<sp<HWCLayer> >& getInvisibleLayersSortedByZ();
    const std::vector<sp<HWCLayer> >& getCommittedLayers();

    sp<HWCLayer> getClientTarget();

    int32_t getWidth(hwc2_config_t config = 0) const;
    int32_t getHeight(hwc2_config_t config = 0) const;
    int32_t getVsyncPeriod(hwc2_config_t config = 0) const;
    int32_t getDpiX(hwc2_config_t config = 0) const;
    int32_t getDpiY(hwc2_config_t config = 0) const;
    uint32_t getNumConfigs() const;
    bool setActiveConfig(hwc2_config_t config);
    hwc2_config_t getActiveConfig() { return m_active_config; }
    bool isConfigChanged();
    int32_t getSecure() const;

    void setPowerMode(const int32_t& mode);
    uint32_t getPowerMode() { return m_power_mode; }

    void setVsyncEnabled(const int32_t& enabled);

    void getType(int32_t* out_type) const;

    uint64_t getId() const { return m_disp_id; }

    int32_t createLayer(hwc2_layer_t* out_layer, const bool& is_ct);
    int32_t destroyLayer(const hwc2_layer_t& layer);

    int32_t getMirrorSrc() const { return m_mir_src; }
    void setMirrorSrc(const int64_t& disp) { m_mir_src = disp; }

    void getGlesRange(int32_t* gles_head, int32_t* gles_tail) const
    {
        *gles_head = m_gles_head;
        *gles_tail = m_gles_tail;
    }

    void setGlesRange(const int32_t& gles_head, const int32_t& gles_tail);

    void updateGlesRange();

    void acceptChanges();

    int32_t getRetireFenceFd() const {
        return m_retire_fence_fd;
    }

    void setRetireFenceFd(const int32_t& retire_fence_fd, const bool& is_disp_connected);

    void clearAllFences();

    // close the retire fence, acquire fence of output buffer, and fbt fences
    void clearDisplayFencesAndFbtFences();

    const std::vector<int32_t>& getPrevCompTypes() { return m_prev_comp_types; }

    void moveChangedCompTypes(std::vector<sp<HWCLayer> >* changed_comp_types)
    {
        // todo: move ?
        m_changed_comp_types = *changed_comp_types;
    }

    void getReleaseFenceFds(
        uint32_t* out_num_elem,
        hwc2_layer_t* out_layer,
        int32_t* out_fence_fd);

    void getClientTargetSupport(
        const uint32_t& width, const uint32_t& height,
        const int32_t& format, const int32_t& dataspace);

    void setOutbuf(const buffer_handle_t& handle, const int32_t& release_fence_fd);
    sp<HWCBuffer> getOutbuf() { return m_outbuf;}

    void setMtkFlags(const int64_t& mtk_flags) {m_mtk_flags = mtk_flags; }
    int64_t getMtkFlags() const { return m_mtk_flags; }

    void dump(String8* dump_str);

    void initPrevCompTypes();

    void buildVisibleAndInvisibleLayersSortedByZ();
    void buildCommittedLayers();

    int32_t getColorTransformHint() { return m_color_transform_hint; }
    int32_t setColorTransform(const float* matrix, const int32_t& hint);

    int32_t getColorMode() { return m_color_mode; }
    void setColorMode(const int32_t& color_mode) { m_color_mode = color_mode; }

    bool needDisableVsyncOffset() { return m_need_disable_vsync_offset; }

    void setupPrivateHandleOfLayers();

    const std::vector<sp<HWCLayer> >& getLastCommittedLayers() const { return m_last_committed_layers; }
    void setLastCommittedLayers(const std::vector<sp<HWCLayer> >& last_committed_layers) { m_last_committed_layers = last_committed_layers; }

    bool isGeometryChanged()
    {
#ifndef MTK_USER_BUILD
        ATRACE_CALL();
#endif
        const auto& committed_layers = getCommittedLayers();
        const auto& last_committed_layers = getLastCommittedLayers();
        if (committed_layers != last_committed_layers)
            return true;

        bool is_dirty = false;

        for (auto& layer: committed_layers)
        {
            is_dirty |= layer->isStateChanged();
        }
        return is_dirty;
    }

    void removePendingRemovedLayers();
    void buildVisibleAndInvisibleLayer();

    bool isValid()
    {
        if (!isConnected() || getPowerMode() == HWC2_POWER_MODE_OFF)
            return false;

        return true;
    }

    void setJobDisplayOrientation();
    void setJobDisplayData();
    bool isForceGpuCompose();

    int getPrevAvailableInputLayerNum() const { return m_prev_available_input_layer_num; }
    void setPrevAvailableInputLayerNum(int availInputNum) { m_prev_available_input_layer_num = availInputNum; }

    void setValiPresentState(HWC_VALI_PRESENT_STATE val, const int32_t& line);

    HWC_VALI_PRESENT_STATE getValiPresentState() const { return m_vali_present_state; }

    bool isVisibleLayerChanged() const { return m_is_visible_layer_changed; }
    void checkVisibleLayerChange(const std::vector<sp<HWCLayer> > &prev_visible_layers);
    void setColorTransformForJob(DispatcherJob* const job);

    void setOverrideMDPOutputFormatOfLayers();
    void setJobVideoTimeStamp(DispatcherJob* job);

    bool isGpuComposition() const { return m_use_gpu_composition; }
    void setGpuComposition(bool enable) { m_use_gpu_composition = enable; }

    void addUnpresentCount();
    void decUnpresentCount();
    int getUnpresentCount() { return m_unpresent_count; }
    int getPrevUnpresentCount() { return m_prev_unpresent_count; }
    // for LED HBM (High Backlight Mode) control
    bool getIsHBM() const { return m_is_HBM; }
    void setIsHBM(const bool& val) { m_is_HBM = val; }
private:
    bool needDoAvGrouping(const int32_t num_validate_display);
    void updateFps();
    void updateLayerPrevInfo();

    int64_t m_mtk_flags;

    int32_t m_type;
    sp<HWCBuffer> m_outbuf;

    bool m_is_validated;
    std::vector<sp<HWCLayer> > m_changed_comp_types;

    int64_t m_disp_id;
    hwc2_config_t m_active_config;
    bool m_config_changed;

    std::map<int64_t, sp<HWCLayer> > m_layers;
    mutable Mutex m_pending_removed_layers_mutex;
    std::set<uint64_t> m_pending_removed_layers_id;
    std::vector<sp<HWCLayer> > m_visible_layers;
    std::vector<sp<HWCLayer> > m_invisible_layers;
    std::vector<sp<HWCLayer> > m_committed_layers;

    // client target
    sp<HWCLayer> m_ct;

    int32_t m_gles_head;
    int32_t m_gles_tail;
    int32_t m_retire_fence_fd;
    int32_t m_mir_src;
    std::vector<int32_t> m_prev_comp_types;
    uint32_t m_power_mode;
    int32_t m_color_transform_hint;
    int32_t m_color_mode;

    bool m_need_disable_vsync_offset;
    bool m_need_av_grouping;
    bool m_use_gpu_composition;

    std::vector<sp<HWCLayer> > m_last_committed_layers;
    bool m_color_transform_ok;
    sp<ColorTransform> m_color_transform;
    int m_prev_available_input_layer_num;

    HWC_VALI_PRESENT_STATE m_vali_present_state;
    bool m_is_visible_layer_changed;

    FpsCounter mFpsCounter;

    // display ID map to mirror count
    int m_unpresent_count;
    int m_prev_unpresent_count;

    // for LED HBM (High Backlight Mode) control
    bool m_is_HBM;
};

#endif
