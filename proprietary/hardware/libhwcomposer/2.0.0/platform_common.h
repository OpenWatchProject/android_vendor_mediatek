#ifndef HWC_PLATFORM_COMMON_H_
#define HWC_PLATFORM_COMMON_H_

#include <utils/Singleton.h>
#include "hwc2.h"

using namespace android;

// ---------------------------------------------------------------------------

enum PLATFORM_INFO {
    PLATFORM_NOT_DEFINE = 0,

    PLATFORM_MT8127 = 1,
    PLATFORM_MT8135 = 2,
    PLATFORM_MT8163 = 3,
    PLATFORM_MT8167 = 4,
    PLATFORM_MT8173 = 5,
    PLATFORM_MT8168 = 6,

    PLATFORM_MT6735 = 1000,
    PLATFORM_MT6739 = 1001,
    PLATFORM_MT6755 = 1002,
    PLATFORM_MT6757 = 1003,
    PLATFORM_MT6763 = 1004,
    PLATFORM_MT6771 = 1005,
    PLATFORM_MT6775 = 1006,
    PLATFORM_MT6765 = 1007,
    PLATFORM_MT6761 = 1008,
    PLATFORM_MT6797 = 1009,
    PLATFORM_MT6799 = 1010,
    PLATFORM_MT6758 = 1011,
    PLATFORM_MT3967 = 1012,
    PLATFORM_MT6580 = 1013,
    PLATFORM_MT6779 = 1014,
    PLATFORM_MT6885 = 1015,
    PLATFORM_MT6768 = 1016,
    PLATFORM_MT6785 = 1017,
};

enum HWC_MIRROR_STATE {
    MIRROR_UNDEFINED = 0, // reserved to avoid using this state by accident

    MIRROR_ENABLED   = (1 << 0),
    MIRROR_PAUSED    = (1 << 1),
    MIRROR_DISABLED  = (1 << 2),
};

enum HWC_MIR_FORMAT {
    MIR_FORMAT_UNDEFINE = 0,
    MIR_FORMAT_RGB888   = 1,
    MIR_FORMAT_YUYV     = 2,
    MIR_FORMAT_YV12     = 3,
};

// An abstract class of Platform. Each function of Platform must have a condidate in
// PlatformCommon to avoid compilation error except pure virtual functions.
class PlatformCommon
{
public:
    PlatformCommon() { };
    virtual ~PlatformCommon() { };

    // initOverlay() is used to init overlay related setting
    void initOverlay();

    // if ui layer could be handled by hwcomposer
    virtual bool isUILayerValid(const sp<HWCLayer>& layer, int32_t* line);

    // isMMLayerValid() is used to verify
    // if mm layer could be handled by hwcomposer
    virtual bool isMMLayerValid(const sp<HWCLayer>& layer, int32_t* line);

    // getUltraVideoSize() is used to return the limitation of video resolution
    // when this device connect with the maximum resolution of external display
    size_t getLimitedVideoSize();

    // getUltraDisplaySize() is used to return the limitation of external display
    // when this device play maximum resolution of video
    size_t getLimitedExternalDisplaySize();

    // check is low memory device
    virtual bool isLowMemoryDevice();

    struct PlatformConfig
    {
        PlatformConfig();

        // platform define related hw family, includes ovl and mdp engine
        int platform;

        // compose_level defines default compose level
        int compose_level;

        // mirror_state defines mirror enhancement state
        int mirror_state;

        // mir_scale_ratio defines the maxinum scale ratio of mirror source
        float mir_scale_ratio;

        // format_mir_mhl defines which color format
        // should be used as mirror result for MHL
        int format_mir_mhl;

        // hdcp checking id handled by display driver
        bool bypass_wlv1_checking;

        // can UI process prexform buffer
        int prexformUI;

        // can rdma support roi update
        int rdma_roi_update;

        // force full invalidate for partial update debug through setprop
        bool force_full_invalidate;

        // use async bliter ultra
        bool use_async_bliter_ultra;

        // force hwc to wait fence for display
        bool wait_fence_for_display;

        // Smart layer switch
        bool enable_smart_layer;

        // enable rgba rotate
        bool enable_rgba_rotate;

        // enable rgbx scaling
        bool enable_rgbx_scaling;

        bool av_grouping;

        // dump input buffers of hwc2
        char dump_buf_type;
        int32_t dump_buf;
        char dump_buf_cont_type;
        int32_t dump_buf_cont;
        bool dump_buf_log_enable;

        // debug flag for filleBalck function, this flag will fill white content
        // into outputbuffer if it no need to fill black
        bool fill_black_debug;

        bool always_setup_priv_hnd;

        // enable/disable uipq debug
        bool uipq_debug;

        bool wdt_trace;

        // If ture, only WiFi-display is composed by HWC, and
        // other virtual displays such as CTS and screenrecord are composed by GPU
        bool only_wfd_by_hwc;

        // If ture, only WiFi-display is converted by OVL, and
        // other virtual displays such as screenrecord are converted by MDP
        bool only_wfd_by_dispdev;

        // If there is only one ovl hw, virtual displays can composed by GPU, and
        // the output format is converted to YV12 by BlitDevice
        bool blitdev_for_virtual;

        // to indicate how HWC process the virtual display when only one ovl hw.
        bool is_support_ext_path_for_virtual;

        // If true, the flow will change from validate -> present to
        // 1. If skip, present
        // 2. If no skip, present -> validate -> present
        bool is_skip_validate;

        bool support_color_transform;

        double mdp_scale_percentage;

        bool extend_mdp_capacity;

        int32_t rpo_ui_max_src_width;

        bool disp_support_decompress;

        bool mdp_support_decompress;

        int32_t disp_wdma_fmt_for_vir_disp;

        bool disable_color_transform_for_secondary_displays;

        bool remove_invisible_layers;

        //Let Vendor control SF latch unsignaled buffer or not
        bool latch_unsignaled_buffer;

        // Both disp and bliter use dataspace to map color_range instead of using
        // gralloc_extra
        bool use_dataspace_for_yuv;

        bool fill_hwdec_hdr;
        bool is_support_mdp_pmqos;
        bool is_support_mdp_pmqos_debug;
        int buffer_slots;
        bool is_disp_support_RGBA1010102;

        // force layer into PQ flow by order index
        int32_t force_pq_index;

        bool is_mdp_support_RGBA1010102;

        // ovl support 2-subsampled format with odd size of roi
        bool support_2subsample_with_odd_size_roi;
    };
    PlatformConfig m_config;
};

#endif // HWC_PLATFORM_H_
