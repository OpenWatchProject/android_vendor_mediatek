#ifndef _HWCOMPOSER_
#define _HWCOMPOSER_

#include <hardware/hardware.h>
#include <hardware/hwcomposer2.h>
#include <hardware/hwcomposer_defs.h>
#include "hwc_priv.h"

class HWC2Api
{
public:
    HWC2Api();

    virtual ~HWC2Api();

    hwc_feature_t m_features;

    virtual void initFeatures();

    virtual void open() = 0;

    virtual void close() = 0;

    virtual void getCapabilities(
        uint32_t* out_count,
        int32_t* /*hwc2_capability_t*/ out_capabilities) = 0;

    /* Device functions */
    virtual int32_t /*hwc2_error_t*/ deviceCreateVirtualDisplay(
        hwc2_device_t* device,
        uint32_t width,
        uint32_t height,
        int32_t* /*android_pixel_format_t*/ format,
        hwc2_display_t* outDisplay) = 0;

    virtual int32_t /*hwc2_error_t*/ deviceDestroyVirtualDisplay(
        hwc2_device_t* device,
        hwc2_display_t display) = 0;

    virtual void deviceDump(hwc2_device_t* device, uint32_t* outSize, char* outBuffer) = 0;

    virtual uint32_t deviceGetMaxVirtualDisplayCount(hwc2_device_t* device) = 0;

    virtual int32_t /*hwc2_error_t*/ deviceRegisterCallback(
        hwc2_device_t* device,
        int32_t /*hwc2_callback_descriptor_t*/ descriptor,
        hwc2_callback_data_t callbackData,
        hwc2_function_pointer_t pointer) = 0;

    /* Display functions */
    virtual int32_t /*hwc2_error_t*/ displayAcceptChanges(
        hwc2_device_t* device,
        hwc2_display_t display) = 0;

    virtual int32_t /*hwc2_error_t*/ displayCreateLayer(
        hwc2_device_t* device,
        hwc2_display_t disply,
        hwc2_layer_t* outLayer) = 0;

    virtual int32_t /*hwc2_error_t*/ displayDestroyLayer(
        hwc2_device_t* device,
        hwc2_display_t display,
        hwc2_layer_t layer) = 0;

    virtual int32_t /*hwc2_error_t*/ displayGetActiveConfig(
        hwc2_device_t* device,
        hwc2_display_t display,
        hwc2_config_t* out_config) = 0;

    virtual int32_t /*hwc2_error_t*/ displayGetChangedCompositionTypes(
        hwc2_device_t* device,
        hwc2_display_t display,
        uint32_t* out_num_elem,
        hwc2_layer_t* out_layers,
        int32_t* /*hwc2_composition_t*/ out_types) = 0;

    virtual int32_t /*hwc2_error_t*/ displayGetClientTargetSupport(
        hwc2_device_t* device,
        hwc2_display_t display,
        uint32_t width,
        uint32_t height,
        int32_t /*android_pixel_format_t*/ format,
        int32_t /*android_dataspace_t*/ dataspace) = 0;

    virtual int32_t /*hwc2_error_t*/ displayGetColorMode(
        hwc2_device_t* device,
        hwc2_display_t display,
        uint32_t* out_num_modes,
        int32_t* out_modes) = 0;

    virtual int32_t /*hwc2_error_t*/ displayGetAttribute(
        hwc2_device_t* device,
        hwc2_display_t display,
        hwc2_config_t config,
        int32_t /*hwc2_attribute_t*/ attribute,
        int32_t* out_value) = 0;

    virtual int32_t /*hwc2_error_t*/ displayGetConfigs(
        hwc2_device_t* device,
        hwc2_display_t display,
        uint32_t* out_num_configs,
        hwc2_config_t* out_configs) = 0;

    virtual int32_t /*hwc2_error_t*/ displayGetName(
        hwc2_device_t* device,
        hwc2_display_t display,
        uint32_t* out_lens,
        char* out_name) = 0;

    virtual int32_t /*hwc2_error_t*/ displayGetRequests(
        hwc2_device_t* device,
        hwc2_display_t display,
        int32_t* /*hwc2_display_request_t*/ out_display_requests,
        uint32_t* out_num_elem,
        hwc2_layer_t* out_layer,
        int32_t* /*hwc2_layer_request_t*/ out_layer_requests) = 0;

    virtual int32_t /*hwc2_error_t*/ displayGetType(
        hwc2_device_t* device,
        hwc2_display_t display,
        int32_t* /*hwc2_display_type_t*/ out_type) = 0;

    virtual int32_t /*hwc2_error_t*/ displayGetDozeSupport(
        hwc2_device_t* device,
        hwc2_display_t display,
        int32_t* out_support) = 0;

    virtual int32_t /*hwc2_error_t*/ displayGetHdrCapability(
        hwc2_device_t* device,
        hwc2_display_t display,
        uint32_t* out_num_types,
        int32_t* /*android_hdr_t*/ out_types,
        float* out_max_luminance,
        float* out_max_avg_luminance,
        float* out_min_luminance) = 0;

    virtual int32_t /*hwc2_error_t*/ displayGetReleaseFence(
        hwc2_device_t* device,
        hwc2_display_t display,
        uint32_t* out_num_elem,
        hwc2_layer_t* out_layer,
        int32_t* out_fence) = 0;

    virtual int32_t /*hwc2_error_t*/ displayPresent(
        hwc2_device_t* device,
        hwc2_display_t display,
        int32_t* out_retire_fence) = 0;

    virtual int32_t /*hwc2_error_t*/ displaySetActiveConfig(
        hwc2_device_t* device,
        hwc2_display_t display,
        hwc2_config_t config_id) = 0;

    virtual int32_t /*hwc2_error_t*/ displaySetClientTarget(
        hwc2_device_t* device,
        hwc2_display_t display,
        buffer_handle_t handle,
        int32_t acquire_fence,
        int32_t dataspace,
        hwc_region_t damage) = 0;

    virtual int32_t /*hwc2_error_t*/ displaySetColorMode(
        hwc2_device_t* device,
        hwc2_display_t display, int32_t mode) = 0;

    virtual int32_t /*hwc2_error_t*/ displaySetColorTransform(
        hwc2_device_t* device,
        hwc2_display_t display,
        const float* matrix,
        int32_t /*android_color_transform_t*/ hint) = 0;

    virtual int32_t /*hwc2_error_t*/ displaySetOutputBuffer(
        hwc2_device_t* device,
        hwc2_display_t display,
        buffer_handle_t buffer,
        int32_t releaseFence) = 0;

    virtual int32_t /*hwc2_error_t*/ displaySetPowerMode(
        hwc2_device_t* device,
        hwc2_display_t display,
        int32_t /*hwc2_power_mode_t*/ mode) = 0;

    virtual int32_t /*hwc2_error_t*/ displaySetVsyncEnabled(
        hwc2_device_t* device,
        hwc2_display_t display,
        int32_t /*hwc2_vsync_t*/ enabled) = 0;

    virtual int32_t /*hwc2_error_t*/ displayValidateDisplay(
        hwc2_device_t* device,
        hwc2_display_t display,
        uint32_t* outNumTypes,
        uint32_t* outNumRequests) = 0;


    virtual int32_t /*hwc2_error_t*/ displayGetPerFrameMetadataKeys(
        hwc2_device_t* device,
        hwc2_display_t display,
        uint32_t* outNumKeys,
        int32_t* /*hwc2_per_frame_metadata_key_t*/ outKeys) = 0;

    /* Layer functions */
    virtual int32_t /*hwc2_error_t*/ layerSetCursorPosition(
        hwc2_device_t* device,
        hwc2_display_t display,
        hwc2_layer_t layer,
        int32_t x,
        int32_t y) = 0;

    virtual int32_t /*hwc2_error_t*/ layerSetBuffer(
        hwc2_device_t* device,
        hwc2_display_t display,
        hwc2_layer_t layer,
        buffer_handle_t buffer,
        int32_t acquireFence) = 0;

    virtual int32_t /*hwc2_error_t*/ layerSetSurfaceDamage(
        hwc2_device_t* device,
        hwc2_display_t display,
        hwc2_layer_t layer,
        hwc_region_t damage) = 0;

    /* Layer state functions */
    virtual int32_t /*hwc2_error_t*/ layerStateSetBlendMode(
        hwc2_device_t* device,
        hwc2_display_t display,
        hwc2_layer_t layer,
        int32_t /*hwc2_blend_mode_t*/ mode) = 0;

    virtual int32_t /*hwc2_error_t*/ layerStateSetColor(
        hwc2_device_t* device,
        hwc2_display_t display,
        hwc2_layer_t layer,
        hwc_color_t color) = 0;

    virtual int32_t /*hwc2_error_t*/ layerStateSetCompositionType(
        hwc2_device_t* device,
        hwc2_display_t display,
        hwc2_layer_t layer,
        int32_t /*hwc2_composition_t*/ type) = 0;

    virtual int32_t /*hwc2_error_t*/ layerStateSetDataSpace(
        hwc2_device_t* device,
        hwc2_display_t display,
        hwc2_layer_t layer,
        int32_t /*android_dataspace_t*/ dataspace) = 0;

    virtual int32_t /*hwc2_error_t*/ layerStateSetDisplayFrame(
        hwc2_device_t* device,
        hwc2_display_t display,
        hwc2_layer_t layer,
        hwc_rect_t frame) = 0;

    virtual int32_t /*hwc2_error_t*/ layerStateSetPlaneAlpha(
        hwc2_device_t* device,
        hwc2_display_t display,
        hwc2_layer_t layer,
        float alpha) = 0;

    virtual int32_t /*hwc2_error_t*/ layerStateSetSidebandStream(
        hwc2_device_t* device,
        hwc2_display_t display,
        hwc2_layer_t layer,
        const native_handle_t* stream) = 0;

    virtual int32_t /*hwc2_error_t*/ layerStateSetSourceCrop(
        hwc2_device_t* device,
        hwc2_display_t display,
        hwc2_layer_t layer,
        hwc_frect_t crop) = 0;

    virtual int32_t /*hwc2_error_t*/ layerStateSetTransform(
        hwc2_device_t* device,
        hwc2_display_t display,
        hwc2_layer_t layer,
        int32_t /*hwc_transform_t*/ transform) = 0;

    virtual int32_t /*hwc2_error_t*/ layerStateSetVisibleRegion(
        hwc2_device_t* device,
        hwc2_display_t display,
        hwc2_layer_t layer,
        hwc_region_t visible) = 0;

    virtual int32_t /*hwc2_error_t*/ layerStateSetZOrder(
        hwc2_device_t* device,
        hwc2_display_t display,
        hwc2_layer_t layer,
        uint32_t z) = 0;

    virtual int32_t /*hwc2_error_t*/ layerStateSetPerFrameMetadata(
        hwc2_device_t* device,
        hwc2_display_t display,
        hwc2_layer_t layer,
        uint32_t numElements,
        const int32_t* /*hw2_per_frame_metadata_key_t*/ keys,
        const float* metadata) = 0;
};

extern HWC2Api* g_hwc2_api;

void hwcGetCapabilities(
    struct hwc2_device* device,
    uint32_t* out_count,
    int32_t* /*hwc2_capability_t*/ out_capabilities);

hwc2_function_pointer_t hwcGetFunction(
    struct hwc2_device* device,
    int32_t /*hwc2_function_pointer_t*/ descriptor);

#endif // _HWCOMPOSER_
