#ifndef __DYNAMIC_FPS_DEFS_H__
#define __DYNAMIC_FPS_DEFS_H__

#include <stdint.h>
#include <sys/cdefs.h>
#include <hardware/hardware.h>

#include <linux/dfrc_drv.h>

//__BEGIN_DECLS

#define DFPS_HEADER_VERSION 1
#define HWC_MODULE_API_VERSION_0_1  HARDWARE_MODULE_API_VERSION(0, 1)

#define DFPS_DEVICE_API_VERSION_1_0  HARDWARE_DEVICE_API_VERSION_2(1, 0, DFPS_HEADER_VERSION)

#define DFPS_HARDWARE_MODULE_ID "dfps"

#define DFPS_DYNAMIC_FPS "dynamicFps"

typedef enum {
    DFPS_CALLBACK_INVALID = 0,
    DFPS_CALLBACK_SW_VSYNC_CHANGE = 1,
} dfps_callback_descriptor_t;

typedef struct dfps_module {
    struct hw_module_t common;
} dfps_module_t;

typedef DFRC_DRV_POLICY dfps_fps_policy_info_t;
typedef DFRC_DRV_REFRESH_RANGE dfps_fps_range_t;
typedef DFRC_DRV_PANEL_INFO dfps_panel_info_t;
typedef DFRC_DRV_VSYNC_REQUEST dfps_fps_request_t;
typedef DFRC_DRV_HWC_INFO dfps_hwc_info_t;
typedef DFRC_DRV_INPUT_WINDOW_INFO dfps_simple_input_window_info_t;

typedef enum {
    DFPS_FUNCTION_INVALID = 0,
    DFPS_FUNCTION_REG_POLICY,
    DFPS_FUNCTION_UNREG_POLICY,
    DFPS_FUNCTION_SET_FPS,
    DFPS_FUNCTION_SET_SINGLE_LAYER,
    DFPS_FUNCTION_SET_NUMBER_DISPLAY,
    DFPS_FUNCTION_SET_HWC_INFO,
    DFPS_FUNCTION_SET_INPUT_WINDOWS,
    DFPS_FUNCTION_REG_INFO,
    DFPS_FUNCTION_UNREG_INFO,
    DFPS_FUNCTION_GET_PANEL_INFO,
    DFPS_FUNCTION_SET_FOREGROUND_INFO,
    DFPS_FUNCTION_GET_FPS_RANGE,
    DFPS_FUNCTION_SET_WINDOW_FLAG,
    DFPS_FUNCTION_ENABLE_TRACKER,
    DFPS_FUNCTION_FORBID_ADJUST_VSYNC,
    DFPS_FUNCTION_DUMP,
    DFPS_FUNCTION_REGISTER_CALLBACK,
} dfps_function_descriptor_t;

static inline const char* getCallbackDescriptorName(dfps_callback_descriptor_t desc) {
    switch (desc) {
        case DFPS_CALLBACK_INVALID: return "Invalid";
        case DFPS_CALLBACK_SW_VSYNC_CHANGE: return "SwVsyncChange";
        default: return "Unknown";
    }
}

static inline const char* getFunctionDescriptorName(dfps_function_descriptor_t desc) {
    switch(desc) {
        case DFPS_FUNCTION_INVALID:
            return "Invalid";
        case DFPS_FUNCTION_REG_POLICY:
            return "RegPolicy";
        case DFPS_FUNCTION_UNREG_POLICY:
            return "UnregPolicy";
        case DFPS_FUNCTION_SET_FPS:
            return "SetFps";
        case DFPS_FUNCTION_SET_SINGLE_LAYER:
            return "SetSingleLayer";
        case DFPS_FUNCTION_SET_NUMBER_DISPLAY:
            return "SetNumberDisplay";
        case DFPS_FUNCTION_SET_HWC_INFO:
            return "SetHwcInfo";
        case DFPS_FUNCTION_SET_INPUT_WINDOWS:
            return "SetInputWindows";
        case DFPS_FUNCTION_REG_INFO:
            return "RegInfo";
        case DFPS_FUNCTION_UNREG_INFO:
            return "UnregInfo";
        case DFPS_FUNCTION_GET_PANEL_INFO:
            return "GetPanelInfo";
        case DFPS_FUNCTION_SET_FOREGROUND_INFO:
            return "SetForegroundInfo";
        case DFPS_FUNCTION_GET_FPS_RANGE:
            return "GetFpsRange";
        case DFPS_FUNCTION_SET_WINDOW_FLAG:
            return "SetWindowFlag";
        case DFPS_FUNCTION_ENABLE_TRACKER:
            return "EnableTracker";
        case DFPS_FUNCTION_FORBID_ADJUST_VSYNC:
            return "ForbidAdjustVsync";
        case DFPS_FUNCTION_DUMP:
            return "Dump";
        case DFPS_FUNCTION_REGISTER_CALLBACK:
            return "RegisterCallback";
        default:
            return "Unknow";
    }
};

namespace DFPS {

enum class Callback : int32_t {
    Invalid = DFPS_CALLBACK_INVALID,
    Vsync = DFPS_CALLBACK_SW_VSYNC_CHANGE,
};

enum class FunctionDescriptor : int32_t {
    Invalid = DFPS_FUNCTION_INVALID,
};

}

enum {
    DFPS_FLAG_NONE = 0x0,
    DFPS_FLAG_BINDER_DIED = 0x1,
};

typedef void (*dfps_function_pointer_t)();
typedef void* dfps_callback_data_t;

typedef struct dfps_device {
    struct hw_device_t common;

    void (*getCapabilities)(struct dfps_device* device, uint32_t* outCount,
            int32_t* /*dfps_capability_t*/ outCapabilities);

    dfps_function_pointer_t (*getFunction)(struct dfps_device* device,
            int32_t /*dfps_function_descriptor_t*/ descriptor);
} dfps_device_t;

static inline int dfps_open(const struct hw_module_t* module,
        dfps_device_t** device) {
    return module->methods->open(module, DFPS_DYNAMIC_FPS,
            TO_HW_DEVICE_T_OPEN(device));
}

static inline int dfps_close(dfps_device_t* device) {
    return device->common.close(&device->common);
}

typedef void (*DFPS_PFN_SW_VSYNC_CHANGE)(dfps_callback_data_t callbackData, int32_t mode, int32_t fps);

typedef int32_t (*DFPS_PFN_REG_POLICY)(dfps_device_t* device, const char* name, size_t size, const dfps_fps_policy_info_t& info, int32_t *receiveFd);

typedef int32_t (*DFPS_PFN_UNREG_POLICY)(dfps_device_t* device, uint64_t sequence, int32_t flag);

typedef int32_t (*DFPS_PFN_SET_FPS)(dfps_device_t* device, const dfps_fps_policy_info_t& info);

typedef int32_t (*DFPS_PFN_SET_SINGLE_LAYER)(dfps_device_t* device, int32_t single);

typedef int32_t (*DFPS_PFN_SET_NUMBER_DISPLAY)(dfps_device_t* device, int32_t number);

typedef int32_t (*DFPS_PFN_SET_HWC_INFO)(dfps_device_t* device, const dfps_hwc_info_t& info);

typedef int32_t (*DFPS_PFN_SET_INPUT_WINDOWS)(dfps_device_t* device, const char* name, size_t size, const dfps_simple_input_window_info_t& info);

typedef int32_t (*DFPS_PFN_REG_INFO)(dfps_device_t* device, uint64_t id);

typedef int32_t (*DFPS_PFN_UNREG_INFO)(dfps_device_t* device, uint64_t id, int32_t flag);

typedef int32_t (*DFPS_PFN_GET_PANEL_INFO)(dfps_device_t* device, dfps_panel_info_t* info);

typedef int32_t (*DFPS_PFN_SET_FOREGROUND_INFO)(dfps_device_t* device, int32_t pid, const char *name, size_t size);

typedef int32_t (*DFPS_PFN_GET_FPS_RANGE)(dfps_device_t* device, dfps_fps_range_t* range);

typedef int32_t (*DFPS_PFN_SET_WINDOW_FLAG)(dfps_device_t* device, int32_t flag, int32_t mask);

typedef int32_t (*DFPS_PFN_ENABLE_TRACKER)(dfps_device_t* device, int32_t enable);

typedef int32_t (*DFPS_PFN_FORBID_ADJUST_VSYNC)(dfps_device_t* device, int32_t forbid);

typedef int32_t (*DFPS_PFN_DUMP)(dfps_device_t* device, uint32_t *outSize, char *outBuffer);

typedef int32_t (*DFPS_PFN_REGISTER_CALLBACK)(dfps_device_t* device, int32_t descriptor, dfps_callback_data_t callbackData, dfps_function_pointer_t pointer);

//__END_DECLS

#endif
