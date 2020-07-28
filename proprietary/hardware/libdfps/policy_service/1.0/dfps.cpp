#include <limits.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>

#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <cutils/log.h>
#include <cutils/atomic.h>

#include <hardware/hardware.h>

#include <dfps/dynamicfps_defs.h>
#include <dfps/dfps_priv.h>
#include <dfps/FpsPolicyService.h>

using namespace DFPS;
using namespace android;

struct dfps_context_t {
    dfps_device_t device;
};

static int32_t dfpsRegPolicy(dfps_device_t* device, const char* name, size_t size, const dfps_fps_policy_info_t& info, int32_t *receiveFd)
{
    private_device *dev = reinterpret_cast<private_device_t*>(device);
    if (dev != NULL) {
        FpsPolicyService *service = reinterpret_cast<FpsPolicyService*>(dev->private_data);
        service->regPolicy(name, size, info, receiveFd);
    }

    return 0;
}

static int32_t dfpsUnregPolicy(dfps_device_t* device, uint64_t sequence, int32_t flag)
{
    private_device *dev = reinterpret_cast<private_device_t*>(device);
    if (dev != NULL) {
        FpsPolicyService *service = reinterpret_cast<FpsPolicyService*>(dev->private_data);
        service->unregPolicy(sequence, (flag & DFPS_FLAG_BINDER_DIED)? true : false);
    }
    return 0;
}

static int32_t dfpsSetFps(dfps_device_t* device, const dfps_fps_policy_info_t& info)
{
    private_device *dev = reinterpret_cast<private_device_t*>(device);
    if (dev != NULL) {
        FpsPolicyService *service = reinterpret_cast<FpsPolicyService*>(dev->private_data);
        service->setFps(info);
    }
    return 0;
}

static int32_t dfpsSetSingleLayer(dfps_device_t* device, int32_t single)
{
    private_device *dev = reinterpret_cast<private_device_t*>(device);
    if (dev != NULL) {
        FpsPolicyService *service = reinterpret_cast<FpsPolicyService*>(dev->private_data);
        service->setSingleLayer(single);
    }
    return 0;
}

static int32_t (dfpsSetNumberDisplay)(dfps_device_t* device, int32_t number)
{
    private_device *dev = reinterpret_cast<private_device_t*>(device);
    if (dev != NULL) {
        FpsPolicyService *service = reinterpret_cast<FpsPolicyService*>(dev->private_data);
        service->setNumberDisplay(number);
    }
    return 0;
}

static int32_t dfpsSetHwcInfo(dfps_device_t* device, const dfps_hwc_info_t& info)
{
    private_device *dev = reinterpret_cast<private_device_t*>(device);
    if (dev != NULL) {
        FpsPolicyService *service = reinterpret_cast<FpsPolicyService*>(dev->private_data);
        service->setHwcInfo(info);
    }
    return 0;
}

static int32_t dfpsSetInputWindow(dfps_device_t* device, const char* name, size_t size, const dfps_simple_input_window_info_t& info)
{
    private_device *dev = reinterpret_cast<private_device_t*>(device);
    if (dev != NULL) {
        FpsPolicyService *service = reinterpret_cast<FpsPolicyService*>(dev->private_data);
        service->setInputWindows(name, size, info);
    }
    return 0;
}

static int32_t dfpsRegInfo(dfps_device_t* device, uint64_t id)
{
    private_device *dev = reinterpret_cast<private_device_t*>(device);
    if (dev != NULL) {
        FpsPolicyService *service = reinterpret_cast<FpsPolicyService*>(dev->private_data);
        service->regInfo(id);
    }
    return 0;
}

static int32_t dfpsUnregInfo(dfps_device_t* device, uint64_t id, int32_t flag)
{
    private_device *dev = reinterpret_cast<private_device_t*>(device);
    if (dev != NULL) {
        FpsPolicyService *service = reinterpret_cast<FpsPolicyService*>(dev->private_data);
        service->unregInfo(id, (flag & DFPS_FLAG_BINDER_DIED) ? true : false);
    }
    return 0;
}

static int32_t dfpsGetPanelInfo(dfps_device_t* device, dfps_panel_info_t* info)
{
    private_device *dev = reinterpret_cast<private_device_t*>(device);
    if (dev != NULL) {
        FpsPolicyService *service = reinterpret_cast<FpsPolicyService*>(dev->private_data);
        service->getPanelInfo(info);
    }
    return 0;
}

static int32_t dfpsSetForegroundInfo(dfps_device_t* device, int32_t pid, const char *name, size_t size)
{
    private_device *dev = reinterpret_cast<private_device_t*>(device);
    if (dev != NULL) {
        FpsPolicyService *service = reinterpret_cast<FpsPolicyService*>(dev->private_data);
        service->setForegroundInfo(pid, name, size);
    }
    return 0;
}

static int32_t dfpsGetFpsRange(dfps_device_t* device, dfps_fps_range_t* range )
{
    private_device *dev = reinterpret_cast<private_device_t*>(device);
    if (dev != NULL) {
        FpsPolicyService *service = reinterpret_cast<FpsPolicyService*>(dev->private_data);
        service->getFpsRange(range);
    }
    return 0;
}

static int32_t dfpsSetWindowFlag(dfps_device_t* device, int32_t flag, int32_t mask)
{
    private_device *dev = reinterpret_cast<private_device_t*>(device);
    if (dev != NULL) {
        FpsPolicyService *service = reinterpret_cast<FpsPolicyService*>(dev->private_data);
        service->setWindowFlag(flag, mask);
    }
    return 0;
}

static int32_t dfpsEnableTracker(dfps_device_t* device, int32_t enable)
{
    private_device *dev = reinterpret_cast<private_device_t*>(device);
    if (dev != NULL) {
        FpsPolicyService *service = reinterpret_cast<FpsPolicyService*>(dev->private_data);
        service->enableTracker(enable);
    }
    return 0;
}

static int32_t dfpsForbidAdjustVsync(dfps_device_t* device, int32_t forbid)
{
    private_device *dev = reinterpret_cast<private_device_t*>(device);
    if (dev != NULL) {
        FpsPolicyService *service = reinterpret_cast<FpsPolicyService*>(dev->private_data);
        service->forbidAdjustingVsync(forbid);
    }
    return 0;
}

static int32_t dfpsDump(dfps_device_t* device, uint32_t* outSize, char* outBuffer)
{
    private_device *dev = reinterpret_cast<private_device_t*>(device);
    if (dev != NULL) {
        FpsPolicyService *service = reinterpret_cast<FpsPolicyService*>(dev->private_data);
        service->dump(outSize, outBuffer);
    }
    return 0;
}

static int32_t dfpsRegisterCallback(dfps_device_t* device, int32_t descriptor, dfps_callback_data_t callbackData, dfps_function_pointer_t pointer)
{
    private_device *dev = reinterpret_cast<private_device_t*>(device);
    if (dev != NULL) {
        FpsPolicyService *service = reinterpret_cast<FpsPolicyService*>(dev->private_data);
        service->registerCallback(descriptor, callbackData, pointer);
    }
    return 0;
}

static void dfpsGetCapabilities(struct dfps_device* /*device*/, uint32_t* out_count, int32_t* out_capabilities)
{
    *out_count = 0;
    out_capabilities = NULL;
}

dfps_function_pointer_t dfpsGetFunction(struct dfps_device* /*device*/, int32_t descriptor)
{
    switch (descriptor)
    {
        case DFPS_FUNCTION_REG_POLICY:
            return reinterpret_cast<void (*)()>(dfpsRegPolicy);

        case DFPS_FUNCTION_UNREG_POLICY:
            return reinterpret_cast<void (*)()>(dfpsUnregPolicy);

        case DFPS_FUNCTION_SET_FPS:
            return reinterpret_cast<void (*)()>(dfpsSetFps);

        case DFPS_FUNCTION_SET_SINGLE_LAYER:
            return reinterpret_cast<void (*)()>(dfpsSetSingleLayer);

        case DFPS_FUNCTION_SET_NUMBER_DISPLAY:
            return reinterpret_cast<void (*)()>(dfpsSetNumberDisplay);

        case DFPS_FUNCTION_SET_HWC_INFO:
            return reinterpret_cast<void (*)()>(dfpsSetHwcInfo);

        case DFPS_FUNCTION_SET_INPUT_WINDOWS:
            return reinterpret_cast<void (*)()>(dfpsSetInputWindow);

        case DFPS_FUNCTION_REG_INFO:
            return reinterpret_cast<void (*)()>(dfpsRegInfo);

        case DFPS_FUNCTION_UNREG_INFO:
            return reinterpret_cast<void (*)()>(dfpsUnregInfo);

        case DFPS_FUNCTION_GET_PANEL_INFO:
            return reinterpret_cast<void (*)()>(dfpsGetPanelInfo);

        case DFPS_FUNCTION_SET_FOREGROUND_INFO:
            return reinterpret_cast<void (*)()>(dfpsSetForegroundInfo);

        case DFPS_FUNCTION_GET_FPS_RANGE:
            return reinterpret_cast<void (*)()>(dfpsGetFpsRange);

        case DFPS_FUNCTION_SET_WINDOW_FLAG:
            return reinterpret_cast<void (*)()>(dfpsSetWindowFlag);

        case DFPS_FUNCTION_ENABLE_TRACKER:
            return reinterpret_cast<void (*)()>(dfpsEnableTracker);

        case DFPS_FUNCTION_FORBID_ADJUST_VSYNC:
            return reinterpret_cast<void (*)()>(dfpsForbidAdjustVsync);

        case DFPS_FUNCTION_DUMP:
            return reinterpret_cast<void (*)()>(dfpsDump);

        case DFPS_FUNCTION_REGISTER_CALLBACK:
            return reinterpret_cast<void (*)()>(dfpsRegisterCallback);
    }

    return NULL;
}

static int dfps_device_close(struct hw_device_t *dev)
{
    private_device_t* private_device = reinterpret_cast<private_device_t*>(dev);
    if (private_device != NULL) {
        delete private_device;
    }

    return 0;
}

int dfps_device_open(const hw_module_t* module, const char* name, hw_device_t** device)
{
    int status = 0;

    if (strcmp(name, DFPS_DYNAMIC_FPS))
    {
        ALOGE("try to load DFPS with wrong name: %s", name);
        return -EINVAL;
    }

    private_device_t* dev = new private_device_t;
    if (dev == NULL)
    {
        ALOGE("fail to allocate memory for DFPS module");
        return -ENOMEM;
    }

    // initialize our state here
    memset(dev, 0, sizeof(*dev));

    // initialize the procs
    dev->base.common.tag = HARDWARE_DEVICE_TAG;
    dev->base.common.version = DFPS_DEVICE_API_VERSION_1_0;
    dev->base.common.module = const_cast<hw_module_t*>(module);
    dev->base.common.close = dfps_device_close;

    dev->base.getCapabilities = dfpsGetCapabilities;
    dev->base.getFunction = dfpsGetFunction;

    FpsPolicyService *service = new FpsPolicyService();
    service->run("FpsPolicyService");
    dev->private_data = service;

    *device = &dev->base.common;

    return status;

}
static struct hw_module_methods_t dfps_module_methods = {
    .open = dfps_device_open
};

struct dfps_module HAL_MODULE_INFO_SYM = {
    .common = {
        .tag = HARDWARE_MODULE_TAG,
        .module_api_version = HWC_MODULE_API_VERSION_0_1,
        .hal_api_version = HARDWARE_HAL_API_VERSION,
        .id = DFPS_HARDWARE_MODULE_ID,
        .name = "Dynamic Fps Module",
        .author = "MediaTek Inc.",
        .methods = &dfps_module_methods,
        .dso = NULL,
        .reserved = {0},
    },
};
