#ifndef HWC_OVL_DEV_H
#define HWC_OVL_DEV_H

#include <ui/Rect.h>
#include <utils/Singleton.h>
#include <hardware/hwcomposer2.h>

#include "hwc2_defs.h"
#include "color.h"
#include "display.h"

#define DISP_NO_PRESENT_FENCE  -1

using namespace android;

struct OverlayPrepareParam;
struct OverlayPortParam;
class MTKM4UDrv;

// ---------------------------------------------------------------------------

// MAX_DIRTY_RECT_CNT hwc supports
enum
{
    MAX_DIRTY_RECT_CNT = 10,
};

enum
{
    OVL_DEVICE_TYPE_INVALID,
    OVL_DEVICE_TYPE_OVL,
    OVL_DEVICE_TYPE_BLITDEV
};

typedef enum {
    HWC_DISP_INVALID_SESSION_MODE = 0,
    HWC_DISP_SESSION_DIRECT_LINK_MODE = 1,
    HWC_DISP_SESSION_DECOUPLE_MODE = 2,
    HWC_DISP_SESSION_DIRECT_LINK_MIRROR_MODE = 3,
    HWC_DISP_SESSION_DECOUPLE_MIRROR_MODE = 4,
    HWC_DISP_SESSION_RDMA_MODE,
    HWC_DISP_SESSION_DUAL_DIRECT_LINK_MODE,
    HWC_DISP_SESSION_DUAL_DECOUPLE_MODE,
    HWC_DISP_SESSION_DUAL_RDMA_MODE,
    HWC_DISP_SESSION_TRIPLE_DIRECT_LINK_MODE,
    HWC_DISP_SESSION_MODE_NUM,
} HWC_DISP_MODE;

typedef enum {
    HWC_DISP_IF_TYPE_DBI = 0,
    HWC_DISP_IF_TYPE_DPI,
    HWC_DISP_IF_TYPE_DSI0,
    HWC_DISP_IF_TYPE_DSI1,
    HWC_DISP_IF_TYPE_DSIDUAL,
    HWC_DISP_IF_HDMI = 7,
    HWC_DISP_IF_HDMI_SMARTBOOK,
    HWC_DISP_IF_MHL,
    HWC_DISP_IF_EPD,
    HWC_DISP_IF_SLIMPORT,
} HWC_DISP_IF_TYPE;

typedef enum {
    HWC_DISP_IF_MODE_VIDEO = 0,
    HWC_DISP_IF_MODE_COMMAND,
} HWC_DISP_IF_MODE;

typedef enum {
    HWC_WAIT_FOR_REFRESH,
    HWC_REFRESH_FOR_ANTI_LATENCY2,
    HWC_REFRESH_FOR_SWITCH_DECOUPLE,
    HWC_REFRESH_FOR_SWITCH_DECOUPLE_MIRROR,
    HWC_REFRESH_FOR_IDLE,
    HWC_REFRESH_TYPE_NUM,
} HWC_SELF_REFRESH_TYPE;

enum HWC_LAYERING_CAPS {
    HWC_LAYERING_OVL_ONLY = 0x00000001,
    HWC_MDP_RSZ_LAYER = 0x00000002,
    HWC_DISP_RSZ_LAYER = 0x00000004,
    HWC_MDP_ROT_LAYER = 0x00000008,
    HWC_MDP_HDR_LAYER = 0x00000010,
    HWC_NO_FBDC       = 0x00000020,
};

enum HWC_FEATURE {
     HWC_FEATURE_TIME_SHARING = 0x00000001,
     HWC_FEATURE_HRT = 0x00000002,
     HWC_FEATURE_PARTIAL = 0x00000004,
     HWC_FEATURE_FENCE_WAIT = 0x00000008,
     HWC_FEATURE_RSZ = 0x00000010,
     HWC_FEATURE_NO_PARGB = 0x00000020,
     HWC_FEATURE_DISP_SELF_REFRESH = 0x00000040,
     HWC_FEATURE_RPO = 0x00000080,
     HWC_FEATURE_FBDC = 0x00000100,
     HWC_FEATURE_FORCE_DISABLE_AOD = 0x00000200,
};

enum HWC_DISP_SESSION_TYPE{
    HWC_DISP_SESSION_PRIMARY = 1,
    HWC_DISP_SESSION_EXTERNAL = 2,
    HWC_DISP_SESSION_MEMORY = 3
};

class SessionInfo
{
public:
    SessionInfo();

    unsigned int maxLayerNum;
    unsigned int isHwVsyncAvailable;
    HWC_DISP_IF_TYPE displayType;
    unsigned int displayWidth;
    unsigned int displayHeight;
    unsigned int displayFormat;
    HWC_DISP_IF_MODE displayMode;
    unsigned int vsyncFPS;
    unsigned int physicalWidth;
    unsigned int physicalHeight;
    unsigned int physicalWidthUm;
    unsigned int physicalHeightUm;
    unsigned int density;
    unsigned int isConnected;
    unsigned int isHDCPSupported;
    unsigned int is3DSupport;
};

class IOverlayDevice: public RefBase
{
public:
    virtual ~IOverlayDevice() {}

    virtual int32_t getType() = 0;

    // initOverlay() initializes overlay related hw setting
    virtual void initOverlay() = 0;

    // getDisplayRotation gets LCM's degree
    virtual uint32_t getDisplayRotation(uint32_t dpy) = 0;

    // isDispRszSupported() is used to query if display rsz is supported
    virtual bool isDispRszSupported() = 0;

    // isDispRszSupported() is used to query if display rsz is supported
    virtual bool isDispRpoSupported() = 0;

    // isDispAodForceDisable return display forces disable aod on hwcomposer
    virtual bool isDispAodForceDisable() = 0;

    // isPartialUpdateSupported() is used to query if PartialUpdate is supported
    virtual bool isPartialUpdateSupported() = 0;

    // isFenceWaitSupported() is used to query if FenceWait is supported
    virtual bool isFenceWaitSupported() = 0;

    // isConstantAlphaForRGBASupported() is used to query if PRGBA is supported
    virtual bool isConstantAlphaForRGBASupported() = 0;

    // isDispSelfRefreshSupported is used to query if hardware support ioctl of self-refresh
    virtual bool isDispSelfRefreshSupported() = 0;

    // isDisplayHrtSupport() is used to query HRT suuported or not
    virtual bool isDisplayHrtSupport() = 0;

    // getMaxOverlayInputNum() gets overlay supported input amount
    virtual int getMaxOverlayInputNum() = 0;

    // getMaxOverlayHeight() gets overlay supported height amount
    virtual uint32_t getMaxOverlayHeight() = 0;

    // getMaxOverlayWidth() gets overlay supported width amount
    virtual uint32_t getMaxOverlayWidth() = 0;

    // getDisplayOutputRotated() get the decouple buffer is rotated or not
    virtual int32_t getDisplayOutputRotated() = 0;

    // getRszMaxWidthInput() get the max width of rsz input
    virtual uint32_t getRszMaxWidthInput() = 0;

    // getRszMaxHeightInput() get the max height of rsz input
    virtual uint32_t getRszMaxHeightInput() = 0;

    // enableDisplayFeature() is used to force hwc to enable feature
    virtual void enableDisplayFeature(uint32_t flag) = 0;

    // disableDisplayFeature() is used to force hwc to disable feature
    virtual void disableDisplayFeature(uint32_t flag) = 0;

    // createOverlaySession() creates overlay composition session
    virtual status_t createOverlaySession(
        int dpy, HWC_DISP_MODE mode = HWC_DISP_SESSION_DIRECT_LINK_MODE) = 0;

    // destroyOverlaySession() destroys overlay composition session
    virtual void destroyOverlaySession(int dpy) = 0;

    // truggerOverlaySession() used to trigger overlay engine to do composition
    virtual status_t triggerOverlaySession(int dpy, int present_fence_idx, int ovlp_layer_num,
                                   int prev_present_fence_fd, hwc2_config_t config,
                                   const uint32_t& hrt_weight, const uint32_t& hrt_idx,
                                   const bool& is_HBM = false) = 0;

    // disableOverlaySession() usd to disable overlay session to do composition
    virtual void disableOverlaySession(int dpy, OverlayPortParam* const* params, int num) = 0;

    // setOverlaySessionMode() sets the overlay session mode
    virtual status_t setOverlaySessionMode(int dpy, HWC_DISP_MODE mode) = 0;

    // getOverlaySessionMode() gets the overlay session mode
    virtual HWC_DISP_MODE getOverlaySessionMode(int dpy) = 0;

    // getOverlaySessionInfo() gets specific display device information
    virtual status_t getOverlaySessionInfo(int dpy, SessionInfo* info) = 0;

    // getAvailableOverlayInput gets available amount of overlay input
    // for different session
    virtual int getAvailableOverlayInput(int dpy) = 0;

    // prepareOverlayInput() gets timeline index and fence fd of overlay input layer
    virtual void prepareOverlayInput(int dpy, OverlayPrepareParam* param) = 0;

    // updateOverlayInputs() updates multiple overlay input layers
    virtual void updateOverlayInputs(int dpy, OverlayPortParam* const* params, int num, sp<ColorTransform> color_transform) = 0;

    // prepareOverlayOutput() gets timeline index and fence fd for overlay output buffer
    virtual void prepareOverlayOutput(int dpy, OverlayPrepareParam* param) = 0;

    // disableOverlayOutput() disables overlay output buffer
    virtual void disableOverlayOutput(int dpy) = 0;

    // enableOverlayOutput() enables overlay output buffer
    virtual void enableOverlayOutput(int dpy, OverlayPortParam* param) = 0;

    // prepareOverlayPresentFence() gets present timeline index and fence
    virtual void prepareOverlayPresentFence(int dpy, OverlayPrepareParam* param) = 0;

    // waitVSync() is used to wait vsync signal for specific display device
    virtual status_t waitVSync(int dpy, nsecs_t *ts) = 0;

    // setPowerMode() is used to switch power setting for display
    virtual void setPowerMode(int dpy, int mode) = 0;

    // to query valid layers which can handled by OVL
    virtual bool queryValidLayer(void* ptr) = 0;

    // waitAllJobDone() use to wait driver for processing all job
    virtual status_t waitAllJobDone(const int dpy) = 0;

    // setLastValidColorTransform is used to save last valid color matrix
    virtual void setLastValidColorTransform(const int32_t& dpy, sp<ColorTransform> color_transform) = 0;

    // getSupportedColorMode is used to check what colormode device support
    virtual int32_t getSupportedColorMode() = 0;

    // waitRefreshRequest() is used to wait for refresh request from driver
    virtual status_t waitRefreshRequest(unsigned int* type) = 0;

    // getWidth() gets the width from the config
    virtual int32_t getWidth(int dpy, hwc2_config_t config) = 0;

    // getHeight() gets the height from the config
    virtual int32_t getHeight(int dpy, hwc2_config_t config) = 0;

    // getRefresh() gets the fps from the config
    virtual int32_t getRefresh(int dpy, hwc2_config_t config) = 0;

    // getNumConfigs gets the number of configs
    virtual uint32_t getNumConfigs(int dpy) = 0;
};

IOverlayDevice* getHwDevice();

#endif
