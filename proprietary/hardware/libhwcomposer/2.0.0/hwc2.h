#ifndef HWC_MEDIATOR_H
#define HWC_MEDIATOR_H
#include <vector>
#include <atomic>
#include <sstream>

#include <utils/Singleton.h>
#include <utils/RefBase.h>

#ifdef USES_PQSERVICE
#include <vendor/mediatek/hardware/pq/2.0/IPictureQuality.h>
using vendor::mediatek::hardware::pq::V2_0::IPictureQuality;
#endif

#include "hwc2_defs.h"
#include "color.h"
#include "hdr.h"
#include "hwc2_api.h"
#include "display.h"
#include "utils/tools.h"
#include "utils/fpscounter.h"
#include "hwcbuffer.h"
#include "hwclayer.h"
#include "hwcdisplay.h"
#include "hrt_common.h"

class HWCDisplay;
class IOverlayDevice;

class DisplayListener : public DisplayManager::EventListener
{
public:
    DisplayListener(
        const HWC2_PFN_HOTPLUG callback_hotplug,
        const hwc2_callback_data_t callback_hotplug_data,
        const HWC2_PFN_VSYNC callback_vsync,
        const hwc2_callback_data_t callback_vsync_data,
        const HWC2_PFN_REFRESH callback_refresh,
        const hwc2_callback_data_t callback_refresh_data);

        HWC2_PFN_HOTPLUG m_callback_hotplug;
        hwc2_callback_data_t m_callback_hotplug_data;

        HWC2_PFN_VSYNC   m_callback_vsync;
        hwc2_callback_data_t m_callback_vsync_data;

        HWC2_PFN_REFRESH m_callback_refresh;
        hwc2_callback_data_t m_callback_refresh_data;
private:
    virtual void onVSync(int dpy, nsecs_t timestamp, bool enabled);

    virtual void onPlugIn(int dpy);

    virtual void onPlugOut(int dpy);

    virtual void onHotPlugExt(int dpy, int connected);

    virtual void onRefresh(int dpy);

    virtual void onRefresh(int dpy, unsigned int type);
};

class HWCMediator : public HWC2Api, public android::Singleton<HWCMediator>
{
public:
    HWCMediator();
    ~HWCMediator();

    sp<HWCDisplay> getHWCDisplay(const uint64_t& disp_id)
    {
        return disp_id < m_displays.size() ? m_displays[disp_id] : nullptr;
    }

    void addHWCDisplay(const sp<HWCDisplay>& display);
    void deleteHWCDisplay(const sp<HWCDisplay>& display);
    DbgLogger& editSetBufFromSfLog() { return m_set_buf_from_sf_log; }
    DbgLogger& editSetCompFromSfLog() { return m_set_comp_from_sf_log; }
    sp<IOverlayDevice> getOvlDevice(const uint64_t& dpy) { return m_disp_devs[dpy]; }

    void addDriverRefreshCount()
    {
        AutoMutex l(m_driver_refresh_count_mutex);
        ++m_driver_refresh_count;
    }
    void decDriverRefreshCount()
    {
        AutoMutex l(m_driver_refresh_count_mutex);
        if (m_driver_refresh_count > 0)
            --m_driver_refresh_count;
    }
    int getDriverRefreshCount() const
    {
        AutoMutex l(m_driver_refresh_count_mutex);
        return m_driver_refresh_count;
    }
private:
    void updateGlesRangeForAllDisplays();

    void unpackageMtkData(const HwcValidateData& val_data);

    void adjustVsyncOffset();

    void setJobVideoTimeStamp();
/*-------------------------------------------------------------------------*/
/* Skip Validate */
    bool checkSkipValidate();

    int getValidDisplayNum();

    void buildVisibleAndInvisibleLayerForAllDisplay();

    void prepareForValidation();

    void validate(const hwc2_display_t& cur_disp = -1);

    void countdowmSkipValiRelatedNumber();

    void setValiPresentStateOfAllDisplay(const HWC_VALI_PRESENT_STATE& val, const int32_t& line);

    SKIP_VALI_STATE getNeedValidate() const { return m_need_validate; }
    void setNeedValidate(SKIP_VALI_STATE val) { m_need_validate = val; }

    int32_t getLastSFValidateNum() const { return m_last_SF_validate_num; }
    void setLastSFValidateNum(int32_t val) { m_last_SF_validate_num = val; }

    void checkDisplayState();
/*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*/
/* Refresh event lock */
    bool isAllDisplayPresentDone();
    void lockRefreshThread(hwc2_display_t display);
    void unlockRefreshThread(hwc2_display_t display);
    bool isValidated() const { return m_is_valied; }
    void setValidated(bool val) { m_is_valied = val; }
public:
    void lockRefreshVali() { m_refresh_vali_lock.lock(); }
    void unlockRefreshVali() { m_refresh_vali_lock.unlock(); }
/*-------------------------------------------------------------------------*/

    std::vector<sp<HWCDisplay> > m_displays;

    sp<HrtCommon> m_hrt;

    SKIP_VALI_STATE m_need_validate;
    int32_t m_last_SF_validate_num;

    int32_t m_validate_seq;
    int32_t m_present_seq;

    bool m_vsync_offset_state;

    DbgLogger m_set_buf_from_sf_log;
    DbgLogger m_set_comp_from_sf_log;
    std::vector<sp<IOverlayDevice> > m_disp_devs;

    std::vector<int32_t> m_capabilities;

    int m_driver_refresh_count;
    mutable Mutex m_driver_refresh_count_mutex;

/*-------------------------------------------------------------------------*/
/* Refresh event lock */
    bool m_is_valied;
    mutable Mutex m_refresh_vali_lock;
/*-------------------------------------------------------------------------*/

public:
    void open(/*hwc_private_device_t* device*/);

    void close(/*hwc_private_device_t* device*/);

    void getCapabilities(
        uint32_t* out_count,
        int32_t* /*hwc2_capability_t*/ out_capabilities);

    bool hasCapabilities(int32_t capabilities);

    void createExternalDisplay();
    void destroyExternalDisplay();

    /* Device functions */
    int32_t /*hwc2_error_t*/ deviceCreateVirtualDisplay(
        hwc2_device_t* device,
        uint32_t width,
        uint32_t height,
        int32_t* /*android_pixel_format_t*/ format,
        hwc2_display_t* outDisplay);

    int32_t /*hwc2_error_t*/ deviceDestroyVirtualDisplay(
        hwc2_device_t* device,
        hwc2_display_t display);

    void deviceDump(hwc2_device_t* device, uint32_t* outSize, char* outBuffer);

    uint32_t deviceGetMaxVirtualDisplayCount(hwc2_device_t* device);

    int32_t /*hwc2_error_t*/ deviceRegisterCallback(
        hwc2_device_t* device,
        int32_t /*hwc2_callback_descriptor_t*/ descriptor,
        hwc2_callback_data_t callbackData,
        hwc2_function_pointer_t pointer);

    /* Display functions */
    int32_t /*hwc2_error_t*/ displayAcceptChanges(
        hwc2_device_t* device,
        hwc2_display_t display);

    int32_t /*hwc2_error_t*/ displayCreateLayer(
        hwc2_device_t* device,
        hwc2_display_t disply,
        hwc2_layer_t* outLayer);

    int32_t /*hwc2_error_t*/ displayDestroyLayer(
        hwc2_device_t* device,
        hwc2_display_t display,
        hwc2_layer_t layer);

    int32_t /*hwc2_error_t*/ displayGetActiveConfig(
        hwc2_device_t* device,
        hwc2_display_t display,
        hwc2_config_t* out_config);

    int32_t /*hwc2_error_t*/ displayGetChangedCompositionTypes(
        hwc2_device_t* device,
        hwc2_display_t display,
        uint32_t* out_num_elem,
        hwc2_layer_t* out_layers,
        int32_t* /*hwc2_composition_t*/ out_types);

    int32_t /*hwc2_error_t*/ displayGetClientTargetSupport(
        hwc2_device_t* device,
        hwc2_display_t display,
        uint32_t width,
        uint32_t height,
        int32_t /*android_pixel_format_t*/ format,
        int32_t /*android_dataspace_t*/ dataspace);

    int32_t /*hwc2_error_t*/ displayGetColorMode(
        hwc2_device_t* device,
        hwc2_display_t display,
        uint32_t* out_num_modes,
        int32_t* out_modes);

    int32_t /*hwc2_error_t*/ displayGetAttribute(
        hwc2_device_t* device,
        hwc2_display_t display,
        hwc2_config_t config,
        int32_t /*hwc2_attribute_t*/ attribute,
        int32_t* out_value);

    int32_t /*hwc2_error_t*/ displayGetConfigs(
        hwc2_device_t* device,
        hwc2_display_t display,
        uint32_t* out_num_configs,
        hwc2_config_t* out_configs);

    int32_t /*hwc2_error_t*/ displayGetName(
        hwc2_device_t* device,
        hwc2_display_t display,
        uint32_t* out_lens,
        char* out_name);

    int32_t /*hwc2_error_t*/ displayGetRequests(
        hwc2_device_t* device,
        hwc2_display_t display,
        int32_t* /*hwc2_display_request_t*/ out_display_requests,
        uint32_t* out_num_elem,
        hwc2_layer_t* out_layer,
        int32_t* /*hwc2_layer_request_t*/ out_layer_requests);

    int32_t /*hwc2_error_t*/ displayGetType(
        hwc2_device_t* device,
        hwc2_display_t display,
        int32_t* /*hwc2_display_type_t*/ out_type);

    int32_t /*hwc2_error_t*/ displayGetDozeSupport(
        hwc2_device_t* device,
        hwc2_display_t display,
        int32_t* out_support);

    int32_t /*hwc2_error_t*/ displayGetHdrCapability(
        hwc2_device_t* device,
        hwc2_display_t display,
        uint32_t* out_num_types,
        int32_t* /*android_hdr_t*/ out_types,
        float* out_max_luminance,
        float* out_max_avg_luminance,
        float* out_min_luminance);

    int32_t /*hwc2_error_t*/ displayGetPerFrameMetadataKeys(
        hwc2_device_t* device,
        hwc2_display_t display,
        uint32_t* outNumKeys,
        int32_t* /*hwc2_per_frame_metadata_key_t*/ outKeys);

    int32_t /*hwc2_error_t*/ displayGetReleaseFence(
        hwc2_device_t* device,
        hwc2_display_t display,
        uint32_t* out_num_elem,
        hwc2_layer_t* out_layer,
        int32_t* out_fence);

    int32_t /*hwc2_error_t*/ displayPresent(
        hwc2_device_t* device,
        hwc2_display_t display,
        int32_t* out_retire_fence);

    int32_t /*hwc2_error_t*/ displaySetActiveConfig(
        hwc2_device_t* device,
        hwc2_display_t display,
        hwc2_config_t config_id);

    int32_t /*hwc2_error_t*/ displaySetClientTarget(
        hwc2_device_t* device,
        hwc2_display_t display,
        buffer_handle_t handle,
        int32_t acquire_fence,
        int32_t dataspace,
        hwc_region_t damage);

    int32_t /*hwc2_error_t*/ displaySetColorMode(
        hwc2_device_t* device,
        hwc2_display_t display, int32_t mode);

    int32_t /*hwc2_error_t*/ displaySetColorTransform(
        hwc2_device_t* device,
        hwc2_display_t display,
        const float* matrix,
        int32_t /*android_color_transform_t*/ hint);

    int32_t /*hwc2_error_t*/ displaySetOutputBuffer(
        hwc2_device_t* device,
        hwc2_display_t display,
        buffer_handle_t buffer,
        int32_t releaseFence);

    int32_t /*hwc2_error_t*/ displaySetPowerMode(
        hwc2_device_t* device,
        hwc2_display_t display,
        int32_t /*hwc2_power_mode_t*/ mode);

    int32_t /*hwc2_error_t*/ displaySetVsyncEnabled(
        hwc2_device_t* device,
        hwc2_display_t display,
        int32_t /*hwc2_vsync_t*/ enabled);

    int32_t /*hwc2_error_t*/ displayValidateDisplay(
        hwc2_device_t* device,
        hwc2_display_t display,
        uint32_t* outNumTypes,
        uint32_t* outNumRequests);

    /* Layer functions */
    int32_t /*hwc2_error_t*/ layerSetCursorPosition(
        hwc2_device_t* device,
        hwc2_display_t display,
        hwc2_layer_t layer,
        int32_t x,
        int32_t y);

    int32_t /*hwc2_error_t*/ layerSetBuffer(
        hwc2_device_t* device,
        hwc2_display_t display,
        hwc2_layer_t layer,
        buffer_handle_t buffer,
        int32_t acquireFence);

    int32_t /*hwc2_error_t*/ layerSetSurfaceDamage(
        hwc2_device_t* device,
        hwc2_display_t display,
        hwc2_layer_t layer,
        hwc_region_t damage);

    /* Layer state functions */
    int32_t /*hwc2_error_t*/ layerStateSetBlendMode(
        hwc2_device_t* device,
        hwc2_display_t display,
        hwc2_layer_t layer,
        int32_t /*hwc2_blend_mode_t*/ mode);

    int32_t /*hwc2_error_t*/ layerStateSetColor(
        hwc2_device_t* device,
        hwc2_display_t display,
        hwc2_layer_t layer,
        hwc_color_t color);

    int32_t /*hwc2_error_t*/ layerStateSetCompositionType(
        hwc2_device_t* device,
        hwc2_display_t display,
        hwc2_layer_t layer,
        int32_t /*hwc2_composition_t*/ type);

    int32_t /*hwc2_error_t*/ layerStateSetDataSpace(
        hwc2_device_t* device,
        hwc2_display_t display,
        hwc2_layer_t layer,
        int32_t /*android_dataspace_t*/ dataspace);

    int32_t /*hwc2_error_t*/ layerStateSetDisplayFrame(
        hwc2_device_t* device,
        hwc2_display_t display,
        hwc2_layer_t layer,
        hwc_rect_t frame);

    int32_t /*hwc2_error_t*/ layerStateSetPlaneAlpha(
        hwc2_device_t* device,
        hwc2_display_t display,
        hwc2_layer_t layer,
        float alpha);

    int32_t /*hwc2_error_t*/ layerStateSetSidebandStream(
        hwc2_device_t* device,
        hwc2_display_t display,
        hwc2_layer_t layer,
        const native_handle_t* stream);

    int32_t /*hwc2_error_t*/ layerStateSetSourceCrop(
        hwc2_device_t* device,
        hwc2_display_t display,
        hwc2_layer_t layer,
        hwc_frect_t crop);

    int32_t /*hwc2_error_t*/ layerStateSetTransform(
        hwc2_device_t* device,
        hwc2_display_t display,
        hwc2_layer_t layer,
        int32_t /*hwc_transform_t*/ transform);

    int32_t /*hwc2_error_t*/ layerStateSetVisibleRegion(
        hwc2_device_t* device,
        hwc2_display_t display,
        hwc2_layer_t layer,
        hwc_region_t visible);

    int32_t /*hwc2_error_t*/ layerStateSetZOrder(
        hwc2_device_t* device,
        hwc2_display_t display,
        hwc2_layer_t layer,
        uint32_t z);

    int32_t /*hwc2_error_t*/ layerStateSetPerFrameMetadata(
        hwc2_device_t* device,
        hwc2_display_t display,
        hwc2_layer_t layer,
        uint32_t numElements,
        const int32_t* /*hw2_per_frame_metadata_key_t*/ keys,
        const float* metadata);

private:
    bool m_is_init_disp_manager;

    HWC2_PFN_HOTPLUG m_callback_hotplug;
    hwc2_callback_data_t m_callback_hotplug_data;

    HWC2_PFN_VSYNC   m_callback_vsync;
    hwc2_callback_data_t m_callback_vsync_data;

    HWC2_PFN_REFRESH   m_callback_refresh;
    hwc2_callback_data_t m_callback_refresh_data;
};

#endif // HWC_MEDIATOR_H
