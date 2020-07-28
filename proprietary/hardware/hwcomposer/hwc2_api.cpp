#define LOG_TAG "hwcomposer"

#include "hwc2_api.h"

// to know which function causes crash
#if 0
#define API_LOGD(x, ...) ALOGD("[API] " x, ##__VA_ARGS__)
#else
#define API_LOGD(x, ...)
#endif

// ---------------------------------------------------------------------------
HWC2Api::HWC2Api()
{
    initFeatures();
}

HWC2Api::~HWC2Api()
{
}

void HWC2Api::initFeatures()
{
    memset(&m_features, 0, sizeof(hwc_feature_t));

#ifdef MTK_EXTERNAL_SUPPORT
    m_features.externals = 1;
#endif

#ifdef MTK_VIRTUAL_SUPPORT
    m_features.virtuals = 1;
#endif

#ifdef MTK_PQ_ENHANCE
    m_features.is_support_pq = 1;
#endif

#ifdef MTK_FORCE_HWC_COPY_VDS
    m_features.copyvds = 1;
#endif

#ifdef MTK_SVP_SUPPORT
    m_features.svp = 1;
#endif

#ifdef MTK_LCM_PHYSICAL_ROTATION_HW
    m_features.phy_rotation_180 = 1;
#endif

#ifdef MTK_ROTATION_OFFSET_SUPPORT
    m_features.rotation_offset = 1;
#endif

#ifdef MTK_GMO_RAM_OPTIMIZE
    m_features.gmo = 1;
#endif

#ifdef MTK_CONTROL_POWER_WITH_FRAMEBUFFER_DEVICE
    m_features.control_fb = 1;
#endif

#ifdef MTK_OD_SUPPORT
    m_features.od = 1;
#endif

#ifdef MTK_DISPLAY_120HZ_SUPPORT
    m_features.fps120 = 1;
#endif
    m_features.fbt_bound = 0;
    m_features.hdmi_s3d = 0;
    m_features.hdmi_s3d_debug = 0;
    m_features.hdmi_s3d_depth = 0;

#ifdef MTK_WITHOUT_PRIMARY_PRESENT_FENCE
    m_features.without_primary_present_fence = 1;
#endif

    m_features.dual_display = MTK_DUAL_DISPLAY;
    m_features.epaper_vendor = MTK_EPAPER_VENDOR;

#ifdef MTK_MERGE_MDP_DISPLAY
    m_features.merge_mdp_display = 1;
#endif

    m_features.trigger_by_vsync = 1;

#ifdef MTK_GLOBAL_PQ_SUPPORT
    m_features.global_pq = 1;
#endif

#ifdef MTK_AOD_SUPPORT
    m_features.aod = 1;
#endif

#ifdef MTK_HAS_HDR_DISPLAY
    m_features.hdr_display = 1;
#endif

#ifdef MTK_VIDEO_ENHANCEMENT
    m_features.video_enhancement = 1;
#endif

#ifdef MTK_VIDEO_TRANSITION
    m_features.video_transition = 1;
#endif

#ifdef MTK_GAMEPQ_SUPPORT
    m_features.game_pq = 1;
#endif
}

// -----------------------------------------------------------------------------
// Device function

static int32_t /*hwc2_error_t*/ deviceCreateVirtualDisplay(
    hwc2_device_t* device,
    uint32_t width,
    uint32_t height,
    int32_t* /*android_pixel_format_t*/ format,
    hwc2_display_t* out_display)
{
    API_LOGD("%s", __func__);
    return g_hwc2_api->deviceCreateVirtualDisplay(device, width, height, format, out_display);
}

static int32_t /*hwc2_error_t*/ deviceDestroyVirtualDisplay(hwc2_device_t* device, hwc2_display_t display)
{
    API_LOGD("%s", __func__);
    return g_hwc2_api->deviceDestroyVirtualDisplay(device, display);
}

static void deviceDump(hwc2_device_t* device, uint32_t* outSize, char* outBuffer)
{
    API_LOGD("%s", __func__);
    return g_hwc2_api->deviceDump(device, outSize, outBuffer);
}

static uint32_t deviceGetMaxVirtualDisplayCount(hwc2_device_t* device)
{
    API_LOGD("%s", __func__);
    return g_hwc2_api->deviceGetMaxVirtualDisplayCount(device);
}

static int32_t /*hwc2_error_t*/ deviceRegisterCallback(
    hwc2_device_t* device,
    int32_t /*hwc2_callback_descriptor_t*/ descriptor,
    hwc2_callback_data_t callbackData,
    hwc2_function_pointer_t pointer)
{
    API_LOGD("%s", __func__);
    return g_hwc2_api->deviceRegisterCallback(device, descriptor, callbackData, pointer);
}

// -----------------------------------------------------------------------------
// Display function

static int32_t /*hwc2_error_t*/ displayAcceptChanges(hwc2_device_t* device, hwc2_display_t display)
{
    API_LOGD("%s", __func__);
    return g_hwc2_api->displayAcceptChanges(device, display);
}

static int32_t /*hwc2_error_t*/ displayCreateLayer(hwc2_device_t* device, hwc2_display_t display, hwc2_layer_t* outLayer)
{
    API_LOGD("%s", __func__);
    return g_hwc2_api->displayCreateLayer(device, display, outLayer);
}

static int32_t /*hwc2_error_t*/ displayDestroyLayer(hwc2_device_t* device, hwc2_display_t display, hwc2_layer_t layer)
{
    API_LOGD("%s", __func__);
    return g_hwc2_api->displayDestroyLayer(device, display, layer);
}

static int32_t /*hwc2_error_t*/ displayGetActiveConfig(hwc2_device_t* device, hwc2_display_t display, hwc2_config_t* out_config)
{
    API_LOGD("%s", __func__);
    return g_hwc2_api->displayGetActiveConfig(device, display, out_config);
}

static int32_t /*hwc2_error_t*/ displayGetChangedCompositionTypes(
    hwc2_device_t* device,
    hwc2_display_t display,
    uint32_t* out_num_elem,
    hwc2_layer_t* out_layers,
    int32_t* /*hwc2_composition_t*/ out_types)
{
    API_LOGD("%s", __func__);
    return g_hwc2_api->displayGetChangedCompositionTypes(
        device, display, out_num_elem, out_layers, out_types);
}

static int32_t /*hwc2_error_t*/ displayGetClientTargetSupport(
    hwc2_device_t* device,
    hwc2_display_t display,
    uint32_t width,
    uint32_t height,
    int32_t /*android_pixel_format_t*/ format,
    int32_t /*android_dataspace_t*/ dataspace)
{
    API_LOGD("%s", __func__);
    return g_hwc2_api->displayGetClientTargetSupport(
        device, display, width, height, format, dataspace);
}

static int32_t /*hwc2_error_t*/ displayGetColorMode(
    hwc2_device_t* device,
    hwc2_display_t display,
    uint32_t* out_num_modes,
    int32_t* out_modes)
{
    API_LOGD("%s", __func__);
    return g_hwc2_api->displayGetColorMode(
        device, display, out_num_modes, out_modes);
}

static int32_t /*hwc2_error_t*/ displayGetAttribute(
    hwc2_device_t* device,
    hwc2_display_t display,
    hwc2_config_t config,
    int32_t /*hwc2_attribute_t*/ attribute,
    int32_t* out_value)
{
    API_LOGD("%s", __func__);
    return g_hwc2_api->displayGetAttribute(
        device, display, config, attribute, out_value);
}

static int32_t /*hwc2_error_t*/ displayGetConfigs(
    hwc2_device_t* device,
    hwc2_display_t display,
    uint32_t* out_num_configs,
    hwc2_config_t* out_configs)
{
    API_LOGD("%s", __func__);
    return g_hwc2_api->displayGetConfigs(device, display, out_num_configs, out_configs);
}

static int32_t /*hwc2_error_t*/ displayGetName(
    hwc2_device_t* device,
    hwc2_display_t display,
    uint32_t* out_lens,
    char* out_name)
{
    API_LOGD("%s", __func__);
    return g_hwc2_api->displayGetName(device, display, out_lens, out_name);
}

static int32_t /*hwc2_error_t*/ displayGetRequests(
    hwc2_device_t* device,
    hwc2_display_t display,
    int32_t* /*hwc2_display_request_t*/ out_display_requests,
    uint32_t* out_num_elem,
    hwc2_layer_t* out_layer,
    int32_t* /*hwc2_layer_request_t*/ out_layer_requests)
{
    API_LOGD("%s", __func__);
    return g_hwc2_api->displayGetRequests(
        device, display, out_display_requests, out_num_elem, out_layer, out_layer_requests);
}

static int32_t /*hwc2_error_t*/ displayGetType(
    hwc2_device_t* device,
    hwc2_display_t display,
    int32_t* /*hwc2_display_type_t*/ out_type)
{
    API_LOGD("%s", __func__);
    return g_hwc2_api->displayGetType(device, display, out_type);
}

static int32_t /*hwc2_error_t*/ displayGetDozeSupport(
    hwc2_device_t* device,
    hwc2_display_t display,
    int32_t* out_support)
{
    API_LOGD("%s", __func__);
    return g_hwc2_api->displayGetDozeSupport(device, display, out_support);
}

static int32_t /*hwc2_error_t*/ displayGetHdrCapability(
    hwc2_device_t* device,
    hwc2_display_t display,
    uint32_t* out_num_types,
    int32_t* /*android_hdr_t*/ out_types,
    float* out_max_luminance,
    float* out_max_avg_luminance,
    float* out_min_luminance)
{
    API_LOGD("%s", __func__);
    return g_hwc2_api->displayGetHdrCapability(
        device, display, out_num_types, out_types,
        out_max_luminance, out_max_avg_luminance, out_min_luminance);
}

static int32_t /*hwc2_error_t*/ displayGetReleaseFence(
    hwc2_device_t* device,
    hwc2_display_t display,
    uint32_t* out_num_elem,
    hwc2_layer_t* out_layer,
    int32_t* out_fence)
{
    API_LOGD("%s", __func__);
    return g_hwc2_api->displayGetReleaseFence(
        device, display, out_num_elem, out_layer, out_fence);
}

static int32_t /*hwc2_error_t*/ displayPresent(
    hwc2_device_t* device,
    hwc2_display_t display,
    int32_t* out_retire_fence)
{
    API_LOGD("%s", __func__);
    return g_hwc2_api->displayPresent(device, display, out_retire_fence);
}

static int32_t /*hwc2_error_t*/ displaySetActiveConfig(
    hwc2_device_t* device,
    hwc2_display_t display,
    hwc2_config_t config_id)
{
    API_LOGD("%s", __func__);
    return g_hwc2_api->displaySetActiveConfig(device, display, config_id);
}

static int32_t /*hwc2_error_t*/ displaySetClientTarget(
    hwc2_device_t* device,
    hwc2_display_t display,
    buffer_handle_t handle,
    int32_t acquire_fence,
    int32_t dataspace,
    hwc_region_t damage)
{
    API_LOGD("%s handle:%x", __func__, handle);
    return g_hwc2_api->displaySetClientTarget(
        device, display, handle, acquire_fence, dataspace, damage);
}

static int32_t /*hwc2_error_t*/ displaySetColorMode(
    hwc2_device_t* device,
    hwc2_display_t display,
    int32_t mode)
{
    API_LOGD("%s", __func__);
    return g_hwc2_api->displaySetColorMode(device, display, mode);
}

static int32_t /*hwc2_error_t*/ displaySetColorTransform(
    hwc2_device_t* device,
    hwc2_display_t display,
    const float* matrix,
    int32_t /*android_color_transform_t*/ hint)
{
    API_LOGD("%s", __func__);
    return g_hwc2_api->displaySetColorTransform(device, display, matrix, hint);
}

static int32_t /*hwc2_error_t*/ displaySetOutputBuffer(
    hwc2_device_t* device,
    hwc2_display_t display,
    buffer_handle_t buffer,
    int32_t release_fence)
{
    API_LOGD("%s", __func__);
    return g_hwc2_api->displaySetOutputBuffer(
        device, display, buffer, release_fence);
}

static int32_t /*hwc2_error_t*/ displaySetPowerMode(
    hwc2_device_t* device,
    hwc2_display_t display,
    int32_t /*hwc2_power_mode_t*/ mode)
{
    API_LOGD("%s", __func__);
    return g_hwc2_api->displaySetPowerMode(device, display, mode);
}

static int32_t /*hwc2_error_t*/ displaySetVsyncEnabled(
    hwc2_device_t* device,
    hwc2_display_t display,
    int32_t /*hwc2_vsync_t*/ enabled)
{
    API_LOGD("%s", __func__);
    return g_hwc2_api->displaySetVsyncEnabled(device, display, enabled);
}

static int32_t /*hwc2_error_t*/ displayValidateDisplay(
    hwc2_device_t* device,
    hwc2_display_t display,
    uint32_t* out_num_types,
    uint32_t* out_num_requests)
{
    API_LOGD("%s", __func__);
    return g_hwc2_api->displayValidateDisplay(
        device, display, out_num_types, out_num_requests);
}

// -----------------------------------------------------------------------------
// Layer function

static int32_t /*hwc2_error_t*/ layerSetCursorPosition(
    hwc2_device_t* device,
    hwc2_display_t display,
    hwc2_layer_t layer,
    int32_t x,
    int32_t y)
{
    API_LOGD("%s", __func__);
    return g_hwc2_api->layerSetCursorPosition(
        device, display, layer, x ,y);
}

static int32_t /*hwc2_error_t*/ layerSetBuffer(
    hwc2_device_t* device,
    hwc2_display_t display,
    hwc2_layer_t layer,
    buffer_handle_t buffer,
    int32_t acquire_fence)
{
    API_LOGD("%s layer(%d), handle(%x)", __func__, layer, buffer);
    return g_hwc2_api->layerSetBuffer(
        device, display, layer, buffer, acquire_fence);
}

static int32_t /*hwc2_error_t*/ layerSetSurfaceDamage(
    hwc2_device_t* device,
    hwc2_display_t display,
    hwc2_layer_t layer,
    hwc_region_t damage)
{
    API_LOGD("%s", __func__);
    return g_hwc2_api->layerSetSurfaceDamage(
        device, display, layer, damage);
}

static int32_t /*hwc2_error_t*/ displayGetPerFrameMetadataKeys(
    hwc2_device_t* device,
    hwc2_display_t display,
    uint32_t* outNumKeys,
    int32_t* /*hwc2_per_frame_metadata_key_t*/ outKeys)
{
    API_LOGD("%s", __func__);
    return g_hwc2_api->displayGetPerFrameMetadataKeys(
        device, display, outNumKeys, outKeys);
}

// -----------------------------------------------------------------------------
// Layer State function

static int32_t /*hwc2_error_t*/ layerStateSetBlendMode(
    hwc2_device_t* device,
    hwc2_display_t display,
    hwc2_layer_t layer,
    int32_t /*hwc2_blend_mode_t*/ mode)
{
    API_LOGD("%s", __func__);
    return g_hwc2_api->layerStateSetBlendMode(device, display, layer, mode);
}

static int32_t /*hwc2_error_t*/ layerStateSetColor(
    hwc2_device_t* device,
    hwc2_display_t display,
    hwc2_layer_t layer,
    hwc_color_t color)
{
    API_LOGD("%s", __func__);
    return g_hwc2_api->layerStateSetColor(
        device, display, layer, color);
}

static int32_t /*hwc2_error_t*/ layerStateSetCompositionType(
    hwc2_device_t* device,
    hwc2_display_t display,
    hwc2_layer_t layer,
    int32_t /*hwc2_composition_t*/ type)
{
    API_LOGD("%s", __func__);
    return g_hwc2_api->layerStateSetCompositionType(
        device, display, layer, type);
}

static int32_t /*hwc2_error_t*/ layerStateSetDataSpace(
    hwc2_device_t* device,
    hwc2_display_t display,
    hwc2_layer_t layer,
    int32_t /*android_dataspace_t*/ dataspace)
{
    API_LOGD("%s", __func__);
    return g_hwc2_api->layerStateSetDataSpace(
        device, display, layer, dataspace);
}

static int32_t /*hwc2_error_t*/ layerStateSetDisplayFrame(
    hwc2_device_t* device,
    hwc2_display_t display,
    hwc2_layer_t layer,
    hwc_rect_t frame)
{
    API_LOGD("%s", __func__);
    return g_hwc2_api->layerStateSetDisplayFrame(
        device, display, layer, frame);
}

static int32_t /*hwc2_error_t*/ layerStateSetPlaneAlpha(
    hwc2_device_t* device,
    hwc2_display_t display,
    hwc2_layer_t layer,
    float alpha)
{
    API_LOGD("%s", __func__);
    return g_hwc2_api->layerStateSetPlaneAlpha(
        device, display, layer, alpha);
}

static int32_t /*hwc2_error_t*/ layerStateSetSidebandStream(
    hwc2_device_t* device,
    hwc2_display_t display,
    hwc2_layer_t layer,
    const native_handle_t* stream)
{
    API_LOGD("%s", __func__);
    return g_hwc2_api->layerStateSetSidebandStream(
        device, display, layer, stream);
}

static int32_t /*hwc2_error_t*/ layerStateSetSourceCrop(
    hwc2_device_t* device,
    hwc2_display_t display,
    hwc2_layer_t layer,
    hwc_frect_t crop)
{
    API_LOGD("%s", __func__);
    return g_hwc2_api->layerStateSetSourceCrop(
        device, display, layer, crop);
}

static int32_t /*hwc2_error_t*/ layerStateSetTransform(
    hwc2_device_t* device,
    hwc2_display_t display,
    hwc2_layer_t layer,
    int32_t /*hwc_transform_t*/ transform)
{
    API_LOGD("%s", __func__);
    return g_hwc2_api->layerStateSetTransform(
        device, display, layer, transform);
}

static int32_t /*hwc2_error_t*/ layerStateSetVisibleRegion(
    hwc2_device_t* device,
    hwc2_display_t display,
    hwc2_layer_t layer,
    hwc_region_t visible)
{
    API_LOGD("%s", __func__);
    return g_hwc2_api->layerStateSetVisibleRegion(
        device, display, layer, visible);
}

static int32_t /*hwc2_error_t*/ layerStateSetZOrder(
    hwc2_device_t* device,
    hwc2_display_t display,
    hwc2_layer_t layer,
    uint32_t z)
{
    API_LOGD("%s", __func__);
    return g_hwc2_api->layerStateSetZOrder(
        device, display, layer, z);
}

static int32_t /*hwc2_error_t*/ layerStateSetPerFrameMetadata(
    hwc2_device_t* device,
    hwc2_display_t display,
    hwc2_layer_t layer,
    uint32_t numElements,
    const int32_t* /*hw2_per_frame_metadata_key_t*/ keys,
    const float* metadata)
{
    API_LOGD("%s", __func__);
    return g_hwc2_api->layerStateSetPerFrameMetadata(
        device, display, layer, numElements, keys, metadata);
}


// ----------------------------------------------------------------------------

typedef struct hwc_private_device
{
    hwc2_device_t base;

    /* our private state goes below here */
    uint32_t tag;
    hwc_procs_t* procs;
} hwc_private_device_t;

static int hwc_device_open(
    const struct hw_module_t* module,
    const char* name,
    struct hw_device_t** device);

static struct hw_module_methods_t hwc_module_methods = {
    .open = hwc_device_open
};

hwc_module_t HAL_MODULE_INFO_SYM = {
    .common = {
        .tag                = HARDWARE_MODULE_TAG,
        .module_api_version = HWC_MODULE_API_VERSION_0_1,
        .hal_api_version    = HARDWARE_HAL_API_VERSION,
        .id                 = HWC_HARDWARE_MODULE_ID,
        .name               = "MediaTek Hardware Composer HAL",
        .author             = "MediaTek Inc.",
        .methods            = &hwc_module_methods,
        .dso                = NULL,
        .reserved           = {0},
    }
};


hwc2_function_pointer_t hwcGetFunction(struct hwc2_device* /*device*/,
    int32_t /*hwc2_function_pointer_t*/ descriptor)
{
    API_LOGD("%s descriptor:%d", __func__, descriptor);
    switch (descriptor)
    {
        case HWC2_FUNCTION_ACCEPT_DISPLAY_CHANGES:
            return reinterpret_cast<void (*)()>(displayAcceptChanges);

        case HWC2_FUNCTION_CREATE_LAYER:
            return reinterpret_cast<void (*)()>(displayCreateLayer);

        case HWC2_FUNCTION_CREATE_VIRTUAL_DISPLAY:
            return reinterpret_cast<void (*)()>(deviceCreateVirtualDisplay);

        case HWC2_FUNCTION_DESTROY_LAYER:
            return reinterpret_cast<void (*)()>(displayDestroyLayer);

        case HWC2_FUNCTION_DESTROY_VIRTUAL_DISPLAY:
            return reinterpret_cast<void (*)()>(deviceDestroyVirtualDisplay);

        case HWC2_FUNCTION_DUMP:
            return reinterpret_cast<void (*)()>(deviceDump);

        case HWC2_FUNCTION_GET_ACTIVE_CONFIG:
            return reinterpret_cast<void (*)()>(displayGetActiveConfig);

        case HWC2_FUNCTION_GET_CHANGED_COMPOSITION_TYPES:
            return reinterpret_cast<void (*)()>(displayGetChangedCompositionTypes);

        case HWC2_FUNCTION_GET_CLIENT_TARGET_SUPPORT:
            return reinterpret_cast<void (*)()>(displayGetClientTargetSupport);

        case HWC2_FUNCTION_GET_COLOR_MODES:
            return reinterpret_cast<void (*)()>(displayGetColorMode);

        case HWC2_FUNCTION_GET_DISPLAY_ATTRIBUTE:
            return reinterpret_cast<void (*)()>(displayGetAttribute);

        case HWC2_FUNCTION_GET_DISPLAY_CONFIGS:
            return reinterpret_cast<void (*)()>(displayGetConfigs);

        case HWC2_FUNCTION_GET_DISPLAY_NAME:
            return reinterpret_cast<void (*)()>(displayGetName);

        case HWC2_FUNCTION_GET_DISPLAY_REQUESTS:
            return reinterpret_cast<void (*)()>(displayGetRequests);

        case HWC2_FUNCTION_GET_DISPLAY_TYPE:
            return reinterpret_cast<void (*)()>(displayGetType);

        case HWC2_FUNCTION_GET_DOZE_SUPPORT:
            return reinterpret_cast<void (*)()>(displayGetDozeSupport);

        case HWC2_FUNCTION_GET_HDR_CAPABILITIES:
            return reinterpret_cast<void (*)()>(displayGetHdrCapability);

        case HWC2_FUNCTION_GET_MAX_VIRTUAL_DISPLAY_COUNT:
            return reinterpret_cast<void (*)()>(deviceGetMaxVirtualDisplayCount);

        case HWC2_FUNCTION_GET_RELEASE_FENCES:
            return reinterpret_cast<void (*)()>(displayGetReleaseFence);

        case HWC2_FUNCTION_PRESENT_DISPLAY:
            return reinterpret_cast<void (*)()>(displayPresent);

        case HWC2_FUNCTION_REGISTER_CALLBACK:
            return reinterpret_cast<void (*)()>(deviceRegisterCallback);

        case HWC2_FUNCTION_SET_ACTIVE_CONFIG:
            return reinterpret_cast<void (*)()>(displaySetActiveConfig);

        case HWC2_FUNCTION_SET_CLIENT_TARGET:
            return reinterpret_cast<void (*)()>(displaySetClientTarget);

        case HWC2_FUNCTION_SET_COLOR_MODE:
            return reinterpret_cast<void (*)()>(displaySetColorMode);

        case HWC2_FUNCTION_SET_COLOR_TRANSFORM:
            return reinterpret_cast<void (*)()>(displaySetColorTransform);

        case HWC2_FUNCTION_SET_CURSOR_POSITION:
            return reinterpret_cast<void (*)()>(layerSetCursorPosition);

        case HWC2_FUNCTION_SET_LAYER_BLEND_MODE:
            return reinterpret_cast<void (*)()>(layerStateSetBlendMode);

        case HWC2_FUNCTION_SET_LAYER_BUFFER:
            return reinterpret_cast<void (*)()>(layerSetBuffer);

        case HWC2_FUNCTION_SET_LAYER_COLOR:
            return reinterpret_cast<void (*)()>(layerStateSetColor);

        case HWC2_FUNCTION_SET_LAYER_COMPOSITION_TYPE:
            return reinterpret_cast<void (*)()>(layerStateSetCompositionType);

        case HWC2_FUNCTION_SET_LAYER_DATASPACE:
            return reinterpret_cast<void (*)()>(layerStateSetDataSpace);

        case HWC2_FUNCTION_SET_LAYER_DISPLAY_FRAME:
            return reinterpret_cast<void (*)()>(layerStateSetDisplayFrame);

        case HWC2_FUNCTION_SET_LAYER_PLANE_ALPHA:
            return reinterpret_cast<void (*)()>(layerStateSetPlaneAlpha);

        case HWC2_FUNCTION_SET_LAYER_SIDEBAND_STREAM:
            return reinterpret_cast<void (*)()>(layerStateSetSidebandStream);

        case HWC2_FUNCTION_SET_LAYER_SOURCE_CROP:
            return reinterpret_cast<void (*)()>(layerStateSetSourceCrop);

        case HWC2_FUNCTION_SET_LAYER_SURFACE_DAMAGE:
            return reinterpret_cast<void (*)()>(layerSetSurfaceDamage);

        case HWC2_FUNCTION_SET_LAYER_TRANSFORM:
            return reinterpret_cast<void (*)()>(layerStateSetTransform);

        case HWC2_FUNCTION_SET_LAYER_VISIBLE_REGION:
            return reinterpret_cast<void (*)()>(layerStateSetVisibleRegion);

        case HWC2_FUNCTION_SET_LAYER_Z_ORDER:
            return reinterpret_cast<void (*)()>(layerStateSetZOrder);

        case HWC2_FUNCTION_SET_OUTPUT_BUFFER:
            return reinterpret_cast<void (*)()>(displaySetOutputBuffer);

        case HWC2_FUNCTION_SET_POWER_MODE:
            return reinterpret_cast<void (*)()>(displaySetPowerMode);

        case HWC2_FUNCTION_SET_VSYNC_ENABLED:
            return reinterpret_cast<void (*)()>(displaySetVsyncEnabled);

        case HWC2_FUNCTION_VALIDATE_DISPLAY:
            return reinterpret_cast<void (*)()>(displayValidateDisplay);

        case HWC2_FUNCTION_SET_LAYER_PER_FRAME_METADATA:
            return reinterpret_cast<void (*)()>(layerStateSetPerFrameMetadata);

        case HWC2_FUNCTION_GET_PER_FRAME_METADATA_KEYS:
            return reinterpret_cast<void (*)()>(displayGetPerFrameMetadataKeys);
    }
    return nullptr;
}

void hwcGetCapabilities(struct hwc2_device* /*device*/, uint32_t* out_count,
    int32_t* /*hwc2_capability_t*/ out_capabilities)
{
    API_LOGD("%s", __func__);
    g_hwc2_api->getCapabilities(out_count, out_capabilities);
}

static int hwc_device_close(struct hw_device_t* device)
{
    hwc_private_device_t* dev = (hwc_private_device_t*) device;
    if (dev)
    {
        g_hwc2_api->close();
        delete dev;
    }
    return 0;
}

static int hwc_device_open(
    const struct hw_module_t* module,
    const char* name,
    struct hw_device_t** device)
{
    API_LOGD("+ %s", __func__);
    hwc_private_device_t* dev;

    if (strcmp(name, HWC_HARDWARE_COMPOSER))
        return -EINVAL;

    dev = new hwc_private_device_t;
    if (dev == NULL)
        return -ENOMEM;

    // initialize our state here
    memset(dev, 0, sizeof(*dev));

    // initialize the procs
    dev->base.common.tag             = HARDWARE_DEVICE_TAG;
    dev->base.common.version         = HWC_DEVICE_API_VERSION_2_0;
    dev->base.common.module          = const_cast<hw_module_t*>(module);
    dev->base.common.close           = hwc_device_close;

    dev->base.getCapabilities        = hwcGetCapabilities;
    dev->base.getFunction            = hwcGetFunction;

    *device = &dev->base.common;
    g_hwc2_api->open(/*dev*/);

    API_LOGD("- %s", __func__);

    return 0;
}
