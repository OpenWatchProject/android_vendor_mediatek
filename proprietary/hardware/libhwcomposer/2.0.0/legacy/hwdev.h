#ifndef HWC_HWDEV_H_
#define HWC_HWDEV_H_

#include <linux/disp_session.h>

#include "dev_interface.h"

// ---------------------------------------------------------------------------

DISP_FORMAT mapDispInFormat(unsigned int format, int mode = HWC2_BLEND_MODE_NONE);
DISP_FORMAT convertFormat4Hrt(unsigned int format);
DISP_MODE mapHwcDispMode2Disp(HWC_DISP_MODE mode);

class DispDevice : public IOverlayDevice, public Singleton<DispDevice>
{
public:
    DispDevice();
    ~DispDevice();

    int32_t getType() { return OVL_DEVICE_TYPE_OVL; }

    // initOverlay() initializes overlay related hw setting
    void initOverlay();

    // getDisplayRotation gets LCM's degree
    uint32_t getDisplayRotation(uint32_t dpy);

    // isDispRszSupported() is used to query if display rsz is supported
    bool isDispRszSupported();

    // isDispRszSupported() is used to query if display rsz is supported
    bool isDispRpoSupported();

    // isDispAodForceDisable return display forces disable aod on hwcomposer
    bool isDispAodForceDisable();

    // isPartialUpdateSupported() is used to query if PartialUpdate is supported
    bool isPartialUpdateSupported();

    // isFenceWaitSupported() is used to query if FenceWait is supported
    bool isFenceWaitSupported();

    // isConstantAlphaForRGBASupported is used to query if hardware support constant alpha for RGBA
    bool isConstantAlphaForRGBASupported();

    // isDispSelfRefreshSupported is used to query if hardware support ioctl of self-refresh
    bool isDispSelfRefreshSupported();

    // isDisplayHrtSupport() is used to query HRT suuported or not
    bool isDisplayHrtSupport();

    // getSupportedColorMode is used to check what colormode device support
    int32_t getSupportedColorMode();

    // getMaxOverlayInputNum() gets overlay supported input amount
    int getMaxOverlayInputNum();

    // getMaxOverlayHeight() gets overlay supported height amount
    uint32_t getMaxOverlayHeight();

    // getMaxOverlayWidth() gets overlay supported width amount
    uint32_t getMaxOverlayWidth();

    // getDisplayOutputRotated() get the decouple buffer is rotated or not
    int32_t getDisplayOutputRotated();

    // getRszMaxWidthInput() get the max width of rsz input
    uint32_t getRszMaxWidthInput();

    // getRszMaxHeightInput() get the max height of rsz input
    uint32_t getRszMaxHeightInput();

    // enableDisplayFeature() is used to force hwc to enable feature
    void enableDisplayFeature(uint32_t flag);

    // disableDisplayFeature() is used to force hwc to disable feature
    void disableDisplayFeature(uint32_t flag);

    // createOverlaySession() creates overlay composition session
    status_t createOverlaySession(
        int dpy, HWC_DISP_MODE mode = HWC_DISP_SESSION_DIRECT_LINK_MODE);

    // destroyOverlaySession() destroys overlay composition session
    void destroyOverlaySession(int dpy);

    // truggerOverlaySession() used to trigger overlay engine to do composition
    status_t triggerOverlaySession(int dpy, int present_fence_idx, int ovlp_layer_num,
                                   int prev_present_fence_fd, hwc2_config_t config,
                                   const uint32_t& hrt_weight, const uint32_t& hrt_idx,
                                   const bool& is_HBM = false);

    // disableOverlaySession() usd to disable overlay session to do composition
    void disableOverlaySession(int dpy,  OverlayPortParam* const* params, int num);

    // setOverlaySessionMode() sets the overlay session mode
    status_t setOverlaySessionMode(int dpy, HWC_DISP_MODE mode);

    // getOverlaySessionMode() gets the overlay session mode
    HWC_DISP_MODE getOverlaySessionMode(int dpy);

    // getOverlaySessionInfo() gets specific display device information
    status_t getOverlaySessionInfo(int dpy, SessionInfo* info);

    // getAvailableOverlayInput gets available amount of overlay input
    // for different session
    int getAvailableOverlayInput(int dpy);

    // prepareOverlayInput() gets timeline index and fence fd of overlay input layer
    void prepareOverlayInput(int dpy, OverlayPrepareParam* param);

    // updateOverlayInputs() updates multiple overlay input layers
    void updateOverlayInputs(int dpy, OverlayPortParam* const* params, int num, sp<ColorTransform> color_transform);

    // prepareOverlayOutput() gets timeline index and fence fd for overlay output buffer
    void prepareOverlayOutput(int dpy, OverlayPrepareParam* param);

    // disableOverlayOutput() disables overlay output buffer
    void disableOverlayOutput(int dpy);

    // enableOverlayOutput() enables overlay output buffer
    void enableOverlayOutput(int dpy, OverlayPortParam* param);

    // prepareOverlayPresentFence() gets present timeline index and fence
    void prepareOverlayPresentFence(int dpy, OverlayPrepareParam* param);

    // waitVSync() is used to wait vsync signal for specific display device
    status_t waitVSync(int dpy, nsecs_t *ts);

    // setPowerMode() is used to switch power setting for display
    void setPowerMode(int dpy, int mode);

    // to query valid layers which can handled by OVL
    bool queryValidLayer(void* ptr);

    // waitAllJobDone() use to wait driver for processing all job
    status_t waitAllJobDone(const int dpy);

    // waitRefreshRequest() is used to wait for refresh request from driver
    status_t waitRefreshRequest(unsigned int* type);

    // setLastValidColorTransform is used to save last valid color matrix
    void setLastValidColorTransform(const int32_t& dpy, sp<ColorTransform> color_transform);

    // getWidth() gets the width from the config
    int32_t getWidth(int dpy, hwc2_config_t config);

    // getHeight() gets the height from the config
    int32_t getHeight(int dpy, hwc2_config_t config);

    // getRefresh() gets the fps from the config
    int32_t getRefresh(int dpy, hwc2_config_t config);

    // getNumConfigs gets the number of configs
    uint32_t getNumConfigs(int dpy);

private:

    // for lagacy driver API
    status_t legacySetInputBuffer(int dpy);
    status_t legacySetOutputBuffer(int dpy);
    status_t legacyTriggerSession(int dpy, int present_fence_idx);

    // for new driver API from MT6755
    status_t frameConfig(int dpy, int present_fence_idx, int ovlp_layer_num,
                         int prev_present_fence_fd, hwc2_config_t config,
                         const uint32_t& hrt_weight, const uint32_t& hrt_idx,
                         const bool& is_HBM = false);

    // query hw capabilities through ioctl and store in m_caps_info
    status_t queryCapsInfo();

    // get the correct device id for extension display when enable dual display
    unsigned int getDeviceId(int dpy);

    //get hw multi configs info
    status_t getOverlayMultiConfigs(int dpy);

    // isMultiConfigsSupport() is used to query if multi configs is supported
    bool isMultiConfigsSupport();

    enum
    {
        DISP_INVALID_SESSION = -1,
    };

    int m_dev_fd;

    int m_ovl_input_num;

    disp_frame_cfg_t m_frame_cfg[DisplayManager::MAX_DISPLAYS];

    disp_session_input_config m_input_config[DisplayManager::MAX_DISPLAYS];

    disp_session_output_config m_output_config[DisplayManager::MAX_DISPLAYS];

    disp_caps_info m_caps_info;

    layer_config* m_layer_config_list[DisplayManager::MAX_DISPLAYS];

    int m_layer_config_len[DisplayManager::MAX_DISPLAYS];

    layer_dirty_roi** m_hwdev_dirty_rect[DisplayManager::MAX_DISPLAYS];

    struct DispColorTransformInfo
    {
        sp<ColorTransform> last_valid_color_transform;
        bool prev_enable_ccorr;
        bool resend_color_transform;
    };

    DispColorTransformInfo m_color_transform_info[DisplayManager::MAX_DISPLAYS];
    struct multi_configs m_multi_cfgs[DisplayManager::MAX_DISPLAYS];
};

#endif // HWC_HWDEV_H_
