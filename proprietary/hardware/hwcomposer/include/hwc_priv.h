#ifndef MTK_HWC_PRIV_H
#define MTK_HWC_PRIV_H

#include <cstdint>
#include <map>
#include <set>
#include <vector>
#include <ui/Rect.h>

#include <hardware/hwcomposer.h>

// HWC includes AOSP headers?
#include <ui/Rect.h>

__BEGIN_DECLS

/*****************************************************************************/

// HwcPrepareData should be created by SurfaceFlinger, not HWComposer!
struct HwcValidateData
{
    struct LayerData
    {
        uint64_t id;

        uint64_t disp_id;

        // in/out
        // 1 byte for isDim
        uint64_t flags;
    };

    struct DisplayData
    {
        // in
        uint64_t id;

        // in/out
        uint64_t flags;
    };
    std::map<int64_t, std::shared_ptr<LayerData> > layers;
    std::vector<DisplayData> displays;
};

// HwcPrepareData should be created by SurfaceFlinger, not HWComposer!
struct HwcPrepareData
{
    HwcPrepareData();

    struct LayerData
    {
        // defined in surfacefligner/mediatek/MtkHwc.cpp
        LayerData(const buffer_handle_t&);

        // in
        buffer_handle_t hnd;

        // out
        uint32_t hints;

        // in/out
        uint32_t flags;
    };

    struct DisplayData
    {
        // defined in surfacefligner/mediatek/MtkHwc.cpp
        DisplayData(const uint32_t&);

        // in
        uint32_t id;

        // in/out
        uint32_t flags;
    };

    std::map<int32_t, std::shared_ptr<LayerData> > layers;

    std::vector<std::shared_ptr<DisplayData> > displays;
};

typedef struct hwc_feature {
    int externals;
    int virtuals;
    int is_support_pq;
#if defined(MTK_HWC_VER_1_2)
    int stereoscopic;
#else
    int copyvds;
    int svp;
    int phy_rotation_180;
    int cache_caps;
    int cache_mode;
    int rotation_offset;
    int gmo;
    int control_fb;
    int trigger_by_vsync;
    int fbt_bound;
#endif
    int od;
    int fps120;
    int hdmi_s3d;
    int hdmi_s3d_debug;
    int hdmi_s3d_depth;
    int without_primary_present_fence;
    int dual_display;
    char *epaper_vendor;
    int legacy_mirror_rule;
    int global_pq;
    int aod;
    int resolution_switch;
    int merge_mdp_display;
    int hdr_display;
    int video_enhancement;
    int video_transition;
    int game_pq;
} hwc_feature_t;


typedef struct hwc_layer_ext_info
{
    int buffer_crop_width;
    int buffer_crop_height;
} hwc_layer_ext_info_t;

// platform support cache feature capability
enum {
    HWC_FEATURES_CACHE_CAPS_GPU_PASSIVE = 0x00000001,

    HWC_FEATURES_CACHE_CAPS_GPU_ACTIVE = 0x00000002,

    HWC_FEATURES_CACHE_CAPS_OVL_ACTIVE = 0x00000004,
};

// determine whether cache feature is on or not
enum {
    HWC_FEATURES_CACHE_MODE_GPU_PASSIVE = 0x00000001,

    HWC_FEATURES_CACHE_MODE_GPU_ACTIVE = 0x00000002,

    HWC_FEATURES_CACHE_MODE_OVL_ACTIVE = 0x00000004,
};

enum {
    // Availability: MTK specific
    // Returns MTK platform features
    HWC_FEATURES_STATE = 100,

    HWC_NUM_EXTRA_BUFFER = 101,

    HWC_VIEWPORT_HINT = 102,
};

/*
 * hwc_layer_t::hints values extension
 * Need to check if these values are conflict to original AOSP
 */
enum {
    /*
     * HWC sets HWC_HINT_SECURE_COMP to tell SurfaceFlinger that
     * this layer is handled by HWC with secure buffer
     */
    HWC_HINT_SECURE_COMP = 0x10000000,
};

/*
 * hwc_layer_1_t::flags values extension
 * Need to check if these values are conflict to original AOSP
 */
enum {
    /* HWC_SECURE_LAYER is set by SurfaceFlinger to indicat that the HAL
     * this layer is secure
     */
    HWC_SECURE_LAYER    = 0x80000000,

    /*
     * HWC_DIRTY_LAYER is set by SurfaceFlinger to indicate that the HAL
     * this layer has updated content.
     */
    HWC_DIRTY_LAYER     = 0x20000000,

    /*
     * HWC_DIM_LAYER is set by SurfaceFlinger to indicate that the HAL
     * this layer is dim
     */
    HWC_DIM_LAYER       = 0x40000000,

    /*
     * HWC_IS_S3D_LAYER is set by hwcomposer to indicate that this layer is used
     * as a S3D display, and process this layer in S3D flow
     */
    HWC_IS_S3D_LAYER     = 0x00000200,

    HWC_IS_S3D_LAYER_SBS = 0x00000600,

    HWC_IS_S3D_LAYER_TAB = 0x00000a00,

    /*
     * HWC_IS_OPAQUE is set by SurfaceFlinger to indicate that the HAL
     * SurfaceFlinger support passing opaque flags
     */
    HWC_IS_OPAQUE        = 0x00200000,

};

/*
 * hwc_display_contents_1_t::flags values extension
 * Need to check if these values are conflict to original AOSP
 */
enum {
#if defined(MTK_HWC_VER_1_2)
    HWC_MIRROR_DISPLAY        = 0x80000000,
#else
    HWC_MIRROR_DISPLAY        = 0x20000000,
#endif

    // HWC_SKIP_DISPLAY is set by SurfaceFlinger to indicate that
    // the composition can be skipped this time
    HWC_SKIP_DISPLAY          = 0x40000000,

    // HWC_MIRRORED_DISP_MASK is using second byte for checking mirror display
    HWC_MIRRORED_DISP_MASK    = 0x0000FF00,

    // HWC_ORIENTATION_MASK is using third byte for external orientation
    // Added for HWC_DEVICE_API_VERSION_1_0
    HWC_ORIENTATION_MASK      = 0x00FF0000,
};

/* more display attributes returned by getDisplayAttributes() */
enum {
    HWC_DISPLAY_SUBTYPE = 10001,
};

/* Display subtypes */
enum {
    HWC_DISPLAY_LCM       = 0,
    HWC_DISPLAY_HDMI_MHL  = 1,
    HWC_DISPLAY_SMARTBOOK = 2,
    HWC_DISPLAY_MEMORY    = 3,
    HWC_DISPLAY_WIRELESS  = 4,
    HWC_DISPLAY_EPAPER    = 5,
};

/*****************************************************************************/

enum {
    HWC2_ATTRIBUTE_VALIDATE_DATA = 10,
};

__END_DECLS

#endif /* MTK_HWC_PRIV_H */
