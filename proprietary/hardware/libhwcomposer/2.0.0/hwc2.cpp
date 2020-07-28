#define DEBUG_LOG_TAG "HWC"
#define ATRACE_TAG ATRACE_TAG_GRAPHICS
#define LOG_TAG "hwcomposer"
#include <cstring>
#include <sstream>
#include <cutils/properties.h>
#include <sync/sync.h>
#include <sw_sync.h>

#include "gralloc_mtk_defs.h"

#include "ui/Rect.h"

#include "utils/debug.h"
#include "utils/tools.h"
#include "utils/devicenode.h"
#include "utils/perfhelper.h"

#include "hwc2.h"
#include "platform.h"
#include "display.h"
#include "overlay.h"
#include "dispatcher.h"
#include "worker.h"
#include "composer.h"
#include "bliter_ultra.h"
#include "asyncblitdev.h"
#include "sync.h"
#include "pqdev.h"

#include "utils/transform.h"
#include "ui/gralloc_extra.h"
#include "ui/Region.h"
#include <utils/SortedVector.h>
#include <utils/String8.h>
// todo: cache
// #include "cache.h"

#ifdef USES_PQSERVICE
#include <vendor/mediatek/hardware/pq/2.0/IPictureQuality.h>
using android::hardware::hidl_array;
using vendor::mediatek::hardware::pq::V2_0::IPictureQuality;
using vendor::mediatek::hardware::pq::V2_0::Result;
#endif

int32_t checkMirrorPath(const vector<sp<HWCDisplay> >& displays, bool *ultra_scenario);

bool isDispConnected(const uint64_t& display)
{
    return DisplayManager::getInstance().getDisplayData(display)->connected;
}

// -----------------------------------------------------------------------------

DisplayListener::DisplayListener(
    const HWC2_PFN_HOTPLUG callback_hotplug,
    const hwc2_callback_data_t callback_hotplug_data,
    const HWC2_PFN_VSYNC callback_vsync,
    const hwc2_callback_data_t callback_vsync_data,
    const HWC2_PFN_REFRESH callback_refresh,
    const hwc2_callback_data_t callback_refresh_data)
    : m_callback_hotplug(callback_hotplug)
    , m_callback_hotplug_data(callback_hotplug_data)
    , m_callback_vsync(callback_vsync)
    , m_callback_vsync_data(callback_vsync_data)
    , m_callback_refresh(callback_refresh)
    , m_callback_refresh_data(callback_refresh_data)
{
}

void DisplayListener::onVSync(int dpy, nsecs_t timestamp, bool enabled)
{
    if (HWC_DISPLAY_PRIMARY == dpy && enabled && m_callback_vsync)
    {
        m_callback_vsync(m_callback_vsync_data, dpy, timestamp);
    }

    HWCDispatcher::getInstance().onVSync(dpy);
}

void DisplayListener::onPlugIn(int dpy)
{
    HWCDispatcher::getInstance().onPlugIn(dpy);
}

void DisplayListener::onPlugOut(int dpy)
{
    HWCDispatcher::getInstance().onPlugOut(dpy);
}

void DisplayListener::onHotPlugExt(int dpy, int connected)
{
    if (m_callback_hotplug &&
        (dpy == HWC_DISPLAY_PRIMARY ||
         dpy == HWC_DISPLAY_EXTERNAL))
    {
        m_callback_hotplug(m_callback_hotplug_data, dpy, connected);
    }
}

void DisplayListener::onRefresh(int dpy)
{
    if (m_callback_refresh)
    {
        m_callback_refresh(m_callback_refresh_data, dpy);
    }
}

void DisplayListener::onRefresh(int dpy, unsigned int /*type*/)
{
    if (m_callback_refresh) {
        HWC_LOGI("fire a callback of refresh to SF");
        m_callback_refresh(m_callback_refresh_data, dpy);
    }
    HWCMediator::getInstance().addDriverRefreshCount();
}
// -----------------------------------------------------------------------------

ANDROID_SINGLETON_STATIC_INSTANCE(HWCMediator);

HWC2Api* g_hwc2_api = &HWCMediator::getInstance();

const char* g_set_buf_from_sf_log_prefix = "[HWC] setBufFromSf";
const char* g_set_comp_from_sf_log_prefix = "[HWC] setCompFromSf";

HWCMediator::HWCMediator()
    : m_need_validate(HWC_SKIP_VALIDATE_NOT_SKIP)
    , m_last_SF_validate_num(0)
    , m_validate_seq(0)
    , m_present_seq(0)
    , m_vsync_offset_state(true)
    , m_set_buf_from_sf_log(DbgLogger::TYPE_HWC_LOG, 'D', g_set_buf_from_sf_log_prefix)
    , m_set_comp_from_sf_log(DbgLogger::TYPE_HWC_LOG, 'D', g_set_comp_from_sf_log_prefix)
    , m_driver_refresh_count(0)
    , m_is_valied(false)
    , m_is_init_disp_manager(false)
    , m_callback_hotplug(nullptr)
    , m_callback_hotplug_data(nullptr)
    , m_callback_vsync(nullptr)
    , m_callback_vsync_data(nullptr)
    , m_callback_refresh(nullptr)
    , m_callback_refresh_data(nullptr)
{
    sp<IOverlayDevice> primary_disp_dev = getHwDevice();
    sp<IOverlayDevice> virtual_disp_dev = nullptr;
    m_hrt = createHrt();
    if (Platform::getInstance().m_config.blitdev_for_virtual)
    {
        virtual_disp_dev = new AsyncBlitDevice();
    }
    else
    {
        virtual_disp_dev = primary_disp_dev;
    }
    m_disp_devs.resize(3, primary_disp_dev);
    m_disp_devs[HWC_DISPLAY_VIRTUAL] = virtual_disp_dev;

    if (primary_disp_dev->isDispAodForceDisable())
    {
        m_features.aod = 0;
        HWC_LOGI("force to disable aod feature by caps");
    }

    Debugger::getInstance();
    Debugger::getInstance().m_logger = new Debugger::LOGGER();

    m_displays.push_back(new HWCDisplay(HWC_DISPLAY_PRIMARY, HWC2_DISPLAY_TYPE_PHYSICAL));
    m_displays.push_back(new HWCDisplay(HWC_DISPLAY_EXTERNAL, HWC2_DISPLAY_TYPE_PHYSICAL));
    m_displays.push_back(new HWCDisplay(HWC_DISPLAY_VIRTUAL, HWC2_DISPLAY_TYPE_VIRTUAL));
    /*
    // check if virtual display could be composed by hwc
    status_t err = DispDevice::getInstance().createOverlaySession(HWC_DISPLAY_VIRTUAL);
    m_is_support_ext_path_for_virtual = (err == NO_ERROR);
    DispDevice::getInstance().destroyOverlaySession(HWC_DISPLAY_VIRTUAL);
    */

    m_capabilities.clear();
    char value[PROPERTY_VALUE_MAX];
    property_get("vendor.debug.hwc.is_skip_validate", value, "-1");
    if (-1 != atoi(value))
    {
        Platform::getInstance().m_config.is_skip_validate = atoi(value);
    }

    if (Platform::getInstance().m_config.is_skip_validate == 1)
    {
        m_capabilities.push_back(HWC2_CAPABILITY_SKIP_VALIDATE);
    }


    HwcHdrUtils::getInstance().setHDRFeature(m_features.hdr_display);

    // set hwc pid to property for watchdog
    char buf[16];
    snprintf(buf, sizeof(buf), "%d", getpid());
    int err = property_set("vendor.debug.sf.hwc_pid", buf);
    if (err < 0) {
        HWC_LOGI("failed to set HWC pid to debug.sf.hwc_pid");
    }

    memset(buf, '\0', 16);
    if (Platform::getInstance().m_config.latch_unsignaled_buffer)
        snprintf(buf, sizeof(buf), "%d", 1);
    else
        snprintf(buf, sizeof(buf), "%d", 0);
    err = property_set("vendor.debug.sf.latch_unsignaled", buf);
    if (err < 0) {
        HWC_LOGI("failed to set vendor.debug.sf.latch_unsignaled");
    }

    // init perfhelper to avoid blocking common flow
    PerfHelper::getInstance();
}

HWCMediator::~HWCMediator()
{
}

void HWCMediator::addHWCDisplay(const sp<HWCDisplay>& display)
{
    m_displays.push_back(display);
}

void HWCMediator::open(/*hwc_private_device_t* device*/)
{
}

void HWCMediator::close(/*hwc_private_device_t* device*/)
{
}

void HWCMediator::getCapabilities(
        uint32_t* out_count,
        int32_t* /*hwc2_capability_t*/ out_capabilities)
{
    if (out_capabilities == NULL)
    {
        *out_count = m_capabilities.size();
        return;
    }

    for(uint32_t i = 0; i < *out_count; ++i)
    {
        out_capabilities[i] = m_capabilities[i];
    }
}

bool HWCMediator::hasCapabilities(int32_t capability)
{
    for (size_t i = 0; i < m_capabilities.size(); ++i)
    {
        if (m_capabilities[i] == capability)
        {
            return true;
        }
    }

    return false;
}

void HWCMediator::createExternalDisplay()
{
    if (m_displays[HWC_DISPLAY_EXTERNAL]->isConnected())
    {
        HWC_LOGE("external display is already connected %s", __func__);
        abort();
    }
    else
    {
        m_displays[HWC_DISPLAY_EXTERNAL]->init();
    }
}

void HWCMediator::destroyExternalDisplay()
{
    if (m_displays[HWC_DISPLAY_EXTERNAL]->isConnected())
    {
        HWC_LOGE("external display is not disconnected %s", __func__);
        abort();
    }
    else
    {
        m_displays[HWC_DISPLAY_EXTERNAL]->clear();
    }
}

/* Device functions */
int32_t /*hwc2_error_t*/ HWCMediator::deviceCreateVirtualDisplay(
    hwc2_device_t* /*device*/,
    uint32_t width,
    uint32_t height,
    int32_t* /*android_pixel_format_t*/ format,
    hwc2_display_t* outDisplay)
{

    if ((NULL == format) || (NULL == outDisplay))
    {
        return HWC2_ERROR_BAD_PARAMETER;
    }

    *outDisplay = HWC_DISPLAY_VIRTUAL;

    if (*outDisplay >= m_displays.size())
    {
        HWC_LOGE("%s: this display(%" PRIu64 ") is invalid, display size is %zu",
                 __func__, *outDisplay, m_displays.size());
        return HWC2_ERROR_BAD_DISPLAY;
    }

    if (m_displays[*outDisplay]->isConnected())
    {
        return HWC2_ERROR_NO_RESOURCES;
    }

    if (Platform::getInstance().m_config.only_wfd_by_hwc &&
        !DisplayManager::getInstance().checkIsWfd())
    {
        return HWC2_ERROR_NO_RESOURCES;
    }

    if (width > HWCMediator::getInstance().getOvlDevice(HWC_DISPLAY_PRIMARY)->getMaxOverlayWidth() ||
        height > HWCMediator::getInstance().getOvlDevice(HWC_DISPLAY_PRIMARY)->getMaxOverlayHeight())
    {
        HWC_LOGI("(%" PRIu64 ") %s hwc not support width:%u x %u limit: %u x %u", *outDisplay, __func__, width, height,
            HWCMediator::getInstance().getOvlDevice(HWC_DISPLAY_PRIMARY)->getMaxOverlayWidth(),
            HWCMediator::getInstance().getOvlDevice(HWC_DISPLAY_PRIMARY)->getMaxOverlayHeight());
        return HWC2_ERROR_NO_RESOURCES;
    }

    if (Platform::getInstance().m_config.only_wfd_by_dispdev &&
        HWCMediator::getInstance().m_features.copyvds)
    {
        if (DisplayManager::getInstance().checkIsWfd())
        {
            if (getOvlDevice(*outDisplay)->getType() != OVL_DEVICE_TYPE_OVL)
            {
                m_disp_devs.pop_back();
                sp<IOverlayDevice> disp_dev = nullptr;
                disp_dev = getHwDevice();
                m_disp_devs.push_back(disp_dev);
                HWC_LOGI("virtual display change to use DispDevice");
            }
        }
        else
        {
            if (getOvlDevice(*outDisplay)->getType() != OVL_DEVICE_TYPE_BLITDEV)
            {
                m_disp_devs.pop_back();
                m_disp_devs.push_back(new AsyncBlitDevice());
                HWC_LOGI("virtual display change to use AsyncBlitDevice");
            }
        }
    }

    HWC_LOGI("(%" PRIu64 ") %s format:%d", *outDisplay, __func__, *format);
    m_displays[*outDisplay]->init();
    if (format != NULL && *format == HAL_PIXEL_FORMAT_RGBA_8888 &&
        !DisplayManager::getInstance().checkIsWfd())
    {
        m_displays[*outDisplay]->setGpuComposition(true);
    }
    DisplayManager::getInstance().hotplugVir(
        HWC_DISPLAY_VIRTUAL, true, width, height, *format);
    return HWC2_ERROR_NONE;
}

int32_t /*hwc2_error_t*/ HWCMediator::deviceDestroyVirtualDisplay(
    hwc2_device_t* /*device*/,
    hwc2_display_t display)
{
    if (display >= DisplayManager::MAX_DISPLAYS)
    {
        HWC_LOGE("%s: this display(%" PRIu64 ") is invalid", __func__, display);
        return HWC2_ERROR_BAD_DISPLAY;
    }

    if (display >= m_displays.size())
    {
        HWC_LOGE("%s: this display(%" PRIu64 ") is invalid, display size is %zu",
                 __func__, display, m_displays.size());
        return HWC2_ERROR_BAD_DISPLAY;
    }

    if (!m_displays[display]->isConnected())
    {
        return HWC2_ERROR_BAD_PARAMETER;
    }
    HWC_LOGI("(%" PRIu64 ") %s", display, __func__);
    const uint32_t width = 0, height = 0;
    const int32_t format = 0;
    DisplayManager::getInstance().hotplugVir(
        HWC_DISPLAY_VIRTUAL, false, width, height, format);
    m_displays[display]->clear();
    return HWC2_ERROR_NONE;
}

void HWCMediator::deviceDump(hwc2_device_t* /*device*/, uint32_t* outSize, char* outBuffer)
{
    static String8 m_dump_str;
    String8 dump_str;
    if (outBuffer)
    {
        size_t sizeSrc = m_dump_str.size();
        size_t sizeFinal = (*outSize >= sizeSrc) ? sizeSrc : *outSize;
        memcpy(outBuffer, const_cast<char*>(m_dump_str.string()), sizeFinal);
        outBuffer[*outSize - 1] = '\0';
    }
    else
    {
        for (auto& display : m_displays)
        {
            if (!display->isConnected())
                continue;
            display->dump(&dump_str);
        }
        char value[PROPERTY_VALUE_MAX];

        // force full invalidate
        property_get("vendor.debug.hwc.forceFullInvalidate", value, "-1");
        if (-1 != atoi(value))
            Platform::getInstance().m_config.force_full_invalidate = atoi(value);

        property_get("vendor.debug.hwc.rgba_rotate", value, "-1");
        if (-1 != atoi(value))
            Platform::getInstance().m_config.enable_rgba_rotate = atoi(value);

        property_get("vendor.debug.hwc.rgbx_scaling", value, "-1");
        if (-1 != atoi(value))
            Platform::getInstance().m_config.enable_rgbx_scaling = atoi(value);

        // check compose level
        property_get("vendor.debug.hwc.compose_level", value, "-1");
        if (-1 != atoi(value))
            Platform::getInstance().m_config.compose_level = atoi(value);

        property_get("vendor.debug.hwc.enableUBL", value, "-1");
        if (-1 != atoi(value))
            Platform::getInstance().m_config.use_async_bliter_ultra = (0 != atoi(value));

        // switch AsyncBltUltraDebug
        property_get("vendor.debug.hwc.debugUBL", value, "-1");
        if (-1 != atoi(value))
            UltraBliter::getInstance().debug(atoi(value));

        property_get("vendor.debug.hwc.prexformUI", value, "-1");
        if (-1 != atoi(value))
            Platform::getInstance().m_config.prexformUI = atoi(value);

        property_get("vendor.debug.hwc.skip_log", value, "-1");
        if (-1 != atoi(value))
            Debugger::m_skip_log = atoi(value);

        // check mirror state
        property_get("vendor.debug.hwc.mirror_state", value, "-1");
        if (-1 != atoi(value))
            Platform::getInstance().m_config.mirror_state = atoi(value);

        // dynamic change mir format for mhl_output
        property_get("vendor.debug.hwc.mhl_output", value, "-1");
        if (-1 != atoi(value))
            Platform::getInstance().m_config.format_mir_mhl = atoi(value);

        // check profile level
        property_get("vendor.debug.hwc.profile_level", value, "-1");
        if (-1 != atoi(value))
            DisplayManager::m_profile_level = atoi(value);

        // check the maximum scale ratio of mirror source
        property_get("vendor.debug.hwc.mir_scale_ratio", value, "0");
        if (!(strlen(value) == 1 && value[0] == '0'))
            Platform::getInstance().m_config.mir_scale_ratio = strtof(value, NULL);

        property_get("persist.vendor.debug.hwc.log", value, "0");
        if (!(strlen(value) == 1 && value[0] == '0'))
            Debugger::getInstance().setLogThreshold(value[0]);

        property_get("vendor.debug.hwc.ext_layer", value, "-1");
        if (-1 != atoi(value))
            Platform::getInstance().m_config.enable_smart_layer = atoi(value);

        // 0: All displays' jobs are dispatched when they are added into job queue
        // 1: Only external display's jobs are dispatched when external display's vsync is received
        // 2: external and wfd displays' jobs are dispatched when they receive VSync
        property_get("vendor.debug.hwc.trigger_by_vsync", value, "-1");
        if (-1 != atoi(value))
            m_features.trigger_by_vsync = atoi(value);

        property_get("vendor.debug.hwc.av_grouping", value, "-1");
        if (-1 != atoi(value))
        {
            Platform::getInstance().m_config.av_grouping = atoi(value);
        }

        // force hwc to wait fence for display
        property_get("vendor.debug.hwc.waitFenceForDisplay", value, "-1");
        if (-1 != atoi(value))
        {
            Platform::getInstance().m_config.wait_fence_for_display = atoi(value);
        }

        property_get("vendor.debug.hwc.always_setup_priv_hnd", value, "-1");
        if (-1 != atoi(value))
        {
            Platform::getInstance().m_config.always_setup_priv_hnd = atoi(value);
        }

        property_get("vendor.debug.hwc.disable_uipq_layer", value, "-1");
        if (-1 != atoi(value))
        {
            Platform::getInstance().m_config.uipq_debug = atoi(value);
        }

        property_get("vendor.debug.hwc.only_wfd_by_hwc", value, "-1");
        if (-1 != atoi(value))
        {
            Platform::getInstance().m_config.only_wfd_by_hwc = atoi(value);
        }

        property_get("vendor.debug.hwc.wdt_trace", value, "-1");
        if (-1 != atoi(value))
        {
            Platform::getInstance().m_config.wdt_trace = atoi(value);
        }

        property_get("vendor.debug.hwc.dump_buf", value, "-1");
        if ('-' != value[0])
        {
            if (value[0] == 'M' || value[0] == 'U' || value[0] == 'C')
            {
                Platform::getInstance().m_config.dump_buf_type = value[0];
                Platform::getInstance().m_config.dump_buf = atoi(value + 1);
            }
            else if(isdigit(value[0]))
            {
                Platform::getInstance().m_config.dump_buf_type = 'A';
                Platform::getInstance().m_config.dump_buf = atoi(value);
            }
        }
        else
        {
            Platform::getInstance().m_config.dump_buf_type = 'A';
            Platform::getInstance().m_config.dump_buf = 0;
        }

        property_get("vendor.debug.hwc.dump_buf_cont", value, "-1");
        if ('-' != value[0])
        {
            if (value[0] == 'M' || value[0] == 'U' || value[0] == 'C')
            {
                Platform::getInstance().m_config.dump_buf_cont_type = value[0];
                Platform::getInstance().m_config.dump_buf_cont = atoi(value + 1);
            }
            else if(isdigit(value[0]))
            {
                Platform::getInstance().m_config.dump_buf_cont_type = 'A';
                Platform::getInstance().m_config.dump_buf_cont = atoi(value);
            }
        }
        else
        {
            Platform::getInstance().m_config.dump_buf_cont_type = 'A';
            Platform::getInstance().m_config.dump_buf_cont = 0;
        }

        property_get("vendor.debug.hwc.dump_buf_log_enable", value, "-1");
        if (-1 != atoi(value))
        {
            Platform::getInstance().m_config.dump_buf_log_enable = atoi(value);
        }

        property_get("vendor.debug.hwc.fill_black_debug", value, "-1");
        if (-1 != atoi(value))
        {
            Platform::getInstance().m_config.fill_black_debug = atoi(value);
        }

        property_get("vendor.debug.hwc.is_skip_validate", value, "-1");
        if (-1 != atoi(value))
        {
            Platform::getInstance().m_config.is_skip_validate = atoi(value);

            if (Platform::getInstance().m_config.is_skip_validate == 0)
            {
                std::vector<int32_t>::iterator capbility = std::find(m_capabilities.begin(), m_capabilities.end(), HWC2_CAPABILITY_SKIP_VALIDATE);
                if (capbility != m_capabilities.end())
                {
                    m_capabilities.erase(capbility);
                }
            }
            else
            {
                if (hasCapabilities(HWC2_CAPABILITY_SKIP_VALIDATE) == false)
                {
                    m_capabilities.push_back(HWC2_CAPABILITY_SKIP_VALIDATE);
                }
            }
        }

        property_get("vendor.debug.hwc.color_transform", value, "-1");
        if (-1 != atoi(value))
        {
            Platform::getInstance().m_config.support_color_transform = atoi(value);
        }

        property_get("vendor.debug.hwc.enable_rpo", value, "-1");
        if (1 == atoi(value))
        {
            HWCMediator::getInstance().getOvlDevice(HWC_DISPLAY_PRIMARY)->enableDisplayFeature(HWC_FEATURE_RPO);
        }
        else if (0 == atoi(value))
        {
            HWCMediator::getInstance().getOvlDevice(HWC_DISPLAY_PRIMARY)->disableDisplayFeature(HWC_FEATURE_RPO);
        }

        property_get("vendor.debug.hwc.rpo_ui_max_src_width", value, "-1");
        if (-1 != atoi(value))
        {
            Platform::getInstance().m_config.rpo_ui_max_src_width = atoi(value);
        }

        property_get("vendor.debug.hwc.mdp_scale_percentage", value, "-1");
        const double num_double = atof(value);
        if (fabs(num_double - (-1)) > 0.05f)
        {
            Platform::getInstance().m_config.mdp_scale_percentage = num_double;
        }

        property_get("vendor.debug.hwc.extend_mdp_cap", value, "-1");
        if (-1 != atoi(value))
        {
            Platform::getInstance().m_config.extend_mdp_capacity = atoi(value);
        }

        property_get("vendor.debug.hwc.disp_support_decompress", value, "-1");
        if (-1 != atoi(value))
        {
            Platform::getInstance().m_config.disp_support_decompress = atoi(value);
        }

        property_get("vendor.debug.hwc.mdp_support_decompress", value, "-1");
        if (-1 != atoi(value))
        {
            Platform::getInstance().m_config.mdp_support_decompress = atoi(value);
        }

        property_get("vendor.debug.hwc.disable_color_transform_for_secondary_displays", value, "-1");
        if (-1 != atoi(value))
        {
            Platform::getInstance().m_config.disable_color_transform_for_secondary_displays = atoi(value);
        }

        property_get("vendor.debug.hwc.disp_support_decompress", value, "-1");
        if (-1 != atoi(value))
        {
            Platform::getInstance().m_config.disp_support_decompress = atoi(value);
        }

        property_get("vendor.debug.hwc.remove_invisible_layers", value, "-1");
        if (-1 != atoi(value))
        {
            Platform::getInstance().m_config.remove_invisible_layers = atoi(value);
        }

        property_get("vendor.debug.hwc.use_dataspace_for_yuv", value, "-1");
        if (-1 != atoi(value))
        {
            Platform::getInstance().m_config.use_dataspace_for_yuv = atoi(value);
        }

        property_get("vendor.debug.hwc.hdr", value, "-1");
        if (-1 != atoi(value))
        {
            m_features.hdr_display = atoi(value);
        }

        property_get("vendor.debug.hwc.fill_hwdec_hdr", value, "-1");
        if (-1 != atoi(value))
        {
            Platform::getInstance().m_config.fill_hwdec_hdr = atoi(value);
        }

        property_get("vendor.debug.hwc.is_support_mdp_pmqos", value, "-1");
        if (-1 != atoi(value))
        {
            Platform::getInstance().m_config.is_support_mdp_pmqos = atoi(value);
        }

        property_get("vendor.debug.hwc.is_support_mdp_pmqos_debug", value, "-1");
        if (-1 != atoi(value))
        {
            Platform::getInstance().m_config.is_support_mdp_pmqos_debug = atoi(value);
        }

        m_hrt->dump(&dump_str);
        property_get("vendor.debug.hwc.force_pq_index", value, "-1");
        if (-1 != atoi(value))
        {
            Platform::getInstance().m_config.force_pq_index = atoi(value);
        }
        else
        {
            Platform::getInstance().m_config.force_pq_index = -1;
        }

        property_get("vendor.debug.hwc.is_support_game_pq", value, "-1");
        if (-1 != atoi(value))
        {
            m_features.game_pq = atoi(value);
        }

        HWCDispatcher::getInstance().dump(&dump_str);
        dump_str.appendFormat("\n");
        Debugger::getInstance().dump(&dump_str);
        dump_str.appendFormat("\n[Driver Support]\n");
#ifndef MTK_USER_BUILD
        dump_str.appendFormat("  res_switch:%d\n", HWCMediator::getInstance().getOvlDevice(HWC_DISPLAY_PRIMARY)->isDispRszSupported());
        dump_str.appendFormat("  rpo:%d max_w,h:%d,%d ui_max_src_width:%d\n",
            HWCMediator::getInstance().getOvlDevice(HWC_DISPLAY_PRIMARY)->isDispRpoSupported(),
            HWCMediator::getInstance().getOvlDevice(HWC_DISPLAY_PRIMARY)->getRszMaxWidthInput(),
            HWCMediator::getInstance().getOvlDevice(HWC_DISPLAY_PRIMARY)->getRszMaxHeightInput(),
            Platform::getInstance().m_config.rpo_ui_max_src_width);
        dump_str.appendFormat("  partial_update:%d\n", HWCMediator::getInstance().getOvlDevice(HWC_DISPLAY_PRIMARY)->isPartialUpdateSupported());
        dump_str.appendFormat("  waits_fences:%d\n", HWCMediator::getInstance().getOvlDevice(HWC_DISPLAY_PRIMARY)->isFenceWaitSupported());
        dump_str.appendFormat("  ConstantAlphaForRGBA:%d\n", HWCMediator::getInstance().getOvlDevice(HWC_DISPLAY_PRIMARY)->isConstantAlphaForRGBASupported());
        dump_str.appendFormat("  ext_path_for_virtual:%d\n", Platform::getInstance().m_config.is_support_ext_path_for_virtual);

        dump_str.appendFormat("  self_refresh:%d\n", HWCMediator::getInstance().getOvlDevice(HWC_DISPLAY_PRIMARY)->isDispSelfRefreshSupported());
        dump_str.appendFormat("  lcm_color_mode:%d\n", DisplayManager::getInstance().getSupportedColorMode(HWC_DISPLAY_PRIMARY));
#else
        dump_str.appendFormat("  %d,%d-%d-%d-%d,%d,%d,%d,%d\n",
            HWCMediator::getInstance().getOvlDevice(HWC_DISPLAY_PRIMARY)->isDispRszSupported(),
            HWCMediator::getInstance().getOvlDevice(HWC_DISPLAY_PRIMARY)->isDispRpoSupported(),
            HWCMediator::getInstance().getOvlDevice(HWC_DISPLAY_PRIMARY)->getRszMaxWidthInput(),
            HWCMediator::getInstance().getOvlDevice(HWC_DISPLAY_PRIMARY)->getRszMaxHeightInput(),
            Platform::getInstance().m_config.rpo_ui_max_src_width,
            HWCMediator::getInstance().getOvlDevice(HWC_DISPLAY_PRIMARY)->isPartialUpdateSupported(),
            HWCMediator::getInstance().getOvlDevice(HWC_DISPLAY_PRIMARY)->isFenceWaitSupported(),
            HWCMediator::getInstance().getOvlDevice(HWC_DISPLAY_PRIMARY)->isConstantAlphaForRGBASupported(),
            Platform::getInstance().m_config.is_support_ext_path_for_virtual);

        dump_str.appendFormat("  %d,%d\n",
            HWCMediator::getInstance().getOvlDevice(HWC_DISPLAY_PRIMARY)->isDispSelfRefreshSupported(),
            DisplayManager::getInstance().getSupportedColorMode(HWC_DISPLAY_PRIMARY));

#endif
        dump_str.appendFormat("\n[HWC Property]\n");
#ifndef MTK_USER_BUILD
        dump_str.appendFormat("  force_full_invalidate(vendor.debug.hwc.forceFullInvalidate):%d\n", Platform::getInstance().m_config.force_full_invalidate);
        dump_str.appendFormat("  wait_fence_for_display(vendor.debug.hwc.waitFenceForDisplay):%d\n", Platform::getInstance().m_config.wait_fence_for_display);
        dump_str.appendFormat("  rgba_rotate(vendor.debug.hwc.rgba_rotate):%d\n", Platform::getInstance().m_config.enable_rgba_rotate);
        dump_str.appendFormat("  rgba_rotate(vendor.debug.hwc.rgbx_scaling):%d\n", Platform::getInstance().m_config.enable_rgbx_scaling);
        dump_str.appendFormat("  compose_level(vendor.debug.hwc.compose_level):%d, ", Platform::getInstance().m_config.compose_level);
        dump_str.appendFormat("  mirror_state(vendor.debug.hwc.mirror_state):%d\n", Platform::getInstance().m_config.mirror_state);

        dump_str.appendFormat("  enableUBL(vendor.debug.hwc.enableUBL):%d\n", Platform::getInstance().m_config.use_async_bliter_ultra);
        dump_str.appendFormat("  prexformUI(vendor.debug.hwc.prexformUI):%d\n", Platform::getInstance().m_config.prexformUI);
        dump_str.appendFormat("  log_level(persist.vendor.debug.hwc.log):%c, ", Debugger::getInstance().getLogThreshold());
        dump_str.appendFormat("  skip_period_log(vendor.debug.hwc.skip_log):%d\n", Debugger::m_skip_log);
        dump_str.appendFormat("  mhl_output(vendor.debug.hwc.mhl_output):%d\n", Platform::getInstance().m_config.format_mir_mhl);

        dump_str.appendFormat("  profile_level(vendor.debug.hwc.profile_level):%d\n", DisplayManager::m_profile_level);
        dump_str.appendFormat("  mir_scale_ratio(vendor.debug.hwc.mir_scale_ratio):%f\n", Platform::getInstance().m_config.mir_scale_ratio);
        dump_str.appendFormat("  ext layer:%d(vendor.debug.hwc.ext_layer)\n", Platform::getInstance().m_config.enable_smart_layer);

        dump_str.appendFormat("  trigger_by_vsync(vendor.debug.hwc.trigger_by_vsync):%d\n", m_features.trigger_by_vsync);
        dump_str.appendFormat("  AV_grouping(vendor.debug.hwc.av_grouping):%d\n", Platform::getInstance().m_config.av_grouping);
        dump_str.appendFormat("  DumpBuf(vendor.debug.hwc.dump_buf):%c-%d, DumpBufCont(debug.hwc.dump_buf_cont):%c-%d log:%d\n",
            Platform::getInstance().m_config.dump_buf_type, Platform::getInstance().m_config.dump_buf,
            Platform::getInstance().m_config.dump_buf_cont_type, Platform::getInstance().m_config.dump_buf_cont,
            Platform::getInstance().m_config.dump_buf_log_enable);

        dump_str.appendFormat("  fill_black_debug(vendor.debug.hwc.fill_black_debug):%d\n", Platform::getInstance().m_config.fill_black_debug);
        dump_str.appendFormat("  Always_Setup_Private_Handle(vendor.debug.hwc.always_setup_priv_hnd):%d\n", Platform::getInstance().m_config.always_setup_priv_hnd);
        dump_str.appendFormat("  wdt_trace(vendor.debug.hwc.wdt_trace):%d\n", Platform::getInstance().m_config.wdt_trace);
        dump_str.appendFormat("  only_wfd_by_hwc(vendor.debug.hwc.only_wfd_by_hwc):%d\n", Platform::getInstance().m_config.only_wfd_by_hwc);
        dump_str.appendFormat("  blitdev_for_virtual(vendor.debug.hwc.blitdev_for_virtual):%d\n", Platform::getInstance().m_config.blitdev_for_virtual);

        dump_str.appendFormat("  is_skip_validate(vendor.debug.hwc.is_skip_validate):%d\n", Platform::getInstance().m_config.is_skip_validate);
        dump_str.appendFormat("  support_color_transform(vendor.debug.hwc.color_transform):%d\n", Platform::getInstance().m_config.support_color_transform);
        dump_str.appendFormat("  mdp_scaling_percentage(vendor.debug.hwc.mdp_scale_percentage):%.2f\n", Platform::getInstance().m_config.mdp_scale_percentage);
        dump_str.appendFormat("  ExtendMDP(vendor.debug.hwc.extend_mdp_cap):%d\n", Platform::getInstance().m_config.extend_mdp_capacity);
        dump_str.appendFormat("  disp_support_decompress(vendor.debug.hwc.disp_support_decompress):%d\n", Platform::getInstance().m_config.disp_support_decompress);

        dump_str.appendFormat("  mdp_support_decompress(vendor.debug.hwc.mdp_support_decompress):%d\n", Platform::getInstance().m_config.mdp_support_decompress);
        dump_str.appendFormat("  mdp_support_decompress(vendor.debug.hwc.disable_color_transform_for_secondary_displays):%d\n", Platform::getInstance().m_config.disable_color_transform_for_secondary_displays);
        dump_str.appendFormat("  mdp_support_decompress(vendor.debug.hwc.remove_invisible_layers):%d\n", Platform::getInstance().m_config.remove_invisible_layers);
        dump_str.appendFormat("  use_datapace_for_yuv(vendor.debug.hwc.use_dataspace_for_yuv):%d\n", Platform::getInstance().m_config.use_dataspace_for_yuv);
        dump_str.appendFormat("  hdr_support (vendor.debug.hwc.hdr):%d\n", m_features.hdr_display);

        dump_str.appendFormat("  hdr_support (vendor.debug.hwc.fill_hwdec_hdr):%d\n", Platform::getInstance().m_config.fill_hwdec_hdr);

        dump_str.appendFormat("  is_support_mdp_pmqos(vendor.debug.hwc.is_support_mdp_pmqos):%d\n", Platform::getInstance().m_config.is_support_mdp_pmqos);
        dump_str.appendFormat("  is_support_mdp_pmqos_debug(vendor.debug.hwc.is_support_mdp_pmqos_debug):%d\n", Platform::getInstance().m_config.is_support_mdp_pmqos_debug);
        dump_str.appendFormat("  force_pq_index(vendor.debug.hwc.force_pq_index):%d\n", Platform::getInstance().m_config.force_pq_index);
        dump_str.appendFormat("  is_support_game_pq(vendor.debug.hwc.is_support_game_pq):%d\n", m_features.game_pq);
        dump_str.appendFormat("\n");
#else // MTK_USER_BUILD
        dump_str.appendFormat("  %d,%d,%d,%d,%d,%d, ",
                Platform::getInstance().m_config.force_full_invalidate,
                Platform::getInstance().m_config.wait_fence_for_display,
                Platform::getInstance().m_config.enable_rgba_rotate,
                Platform::getInstance().m_config.enable_rgbx_scaling,
                Platform::getInstance().m_config.compose_level,
                Platform::getInstance().m_config.mirror_state);

        dump_str.appendFormat("%d,%d,%c,%d,%d, ",
                Platform::getInstance().m_config.use_async_bliter_ultra,
                Platform::getInstance().m_config.prexformUI,
                Debugger::getInstance().getLogThreshold(),
                Debugger::m_skip_log,
                Platform::getInstance().m_config.format_mir_mhl);

        dump_str.appendFormat("%d,%f,%d, ",
                DisplayManager::m_profile_level,
                Platform::getInstance().m_config.mir_scale_ratio,
                Platform::getInstance().m_config.enable_smart_layer);

        dump_str.appendFormat("%d,%d,%c-%d,%c-%d,%d, ",
                m_features.trigger_by_vsync,
                Platform::getInstance().m_config.av_grouping,
                Platform::getInstance().m_config.dump_buf_type, Platform::getInstance().m_config.dump_buf,
                Platform::getInstance().m_config.dump_buf_cont_type, Platform::getInstance().m_config.dump_buf_cont,
                Platform::getInstance().m_config.dump_buf_log_enable);

        dump_str.appendFormat("%d,%d,%d,%d,%d,%d,%d, ",
                Platform::getInstance().m_config.fill_black_debug,
                Platform::getInstance().m_config.always_setup_priv_hnd,
                Platform::getInstance().m_config.wdt_trace,
                Platform::getInstance().m_config.only_wfd_by_hwc,
                Platform::getInstance().m_config.blitdev_for_virtual,
                Platform::getInstance().m_config.is_skip_validate,
                Platform::getInstance().m_config.support_color_transform);

        dump_str.appendFormat("%.2f,%d,%d,%d,%d,  ",
                Platform::getInstance().m_config.mdp_scale_percentage,
                Platform::getInstance().m_config.extend_mdp_capacity,
                Platform::getInstance().m_config.disp_support_decompress,
                Platform::getInstance().m_config.mdp_support_decompress,
                Platform::getInstance().m_config.disable_color_transform_for_secondary_displays);

        dump_str.appendFormat("%d %d %d,%d,%d,%d,%d\n\n",
                Platform::getInstance().m_config.use_dataspace_for_yuv,
                m_features.hdr_display,
                Platform::getInstance().m_config.fill_hwdec_hdr,
                Platform::getInstance().m_config.is_support_mdp_pmqos,
                Platform::getInstance().m_config.is_support_mdp_pmqos_debug,
                Platform::getInstance().m_config.force_pq_index,
                m_features.game_pq);
#endif // MTK_USER_BUILD

        property_get("vendor.debug.hwc.useColorTransformIoctl", value, "-1");
        if (atoi(value) != -1)
        {
            PqDevice::getInstance().useColorTransformIoctl(atoi(value));
        }
        dump_str.appendFormat("[PQ Support]\n");
#ifndef MTK_USER_BUILD
        dump_str.appendFormat("  useColorTransformIoctl(vendor.debug.hwc.useColorTransformIoctl):%d\n",
                PqDevice::getInstance().isColorTransformIoctl());
#else
        dump_str.appendFormat(" %d,\n",
                PqDevice::getInstance().isColorTransformIoctl());
#endif
        dump_str.appendFormat("\n");

        *outSize = dump_str.size() + 1;
        m_dump_str = dump_str;
    }
}

uint32_t HWCMediator::deviceGetMaxVirtualDisplayCount(hwc2_device_t* /*device*/)
{
    return 1;
}

int32_t /*hwc2_error_t*/ HWCMediator::deviceRegisterCallback(
    hwc2_device_t* /*device*/,
    int32_t /*hwc2_callback_descriptor_t*/ descriptor,
    hwc2_callback_data_t callback_data,
    hwc2_function_pointer_t pointer)
{
    switch (descriptor)
    {
        case HWC2_CALLBACK_HOTPLUG:
            {
                m_callback_hotplug = reinterpret_cast<HWC2_PFN_HOTPLUG>(pointer);
                m_callback_hotplug_data = callback_data;
                sp<DisplayListener> listener = (DisplayListener *) DisplayManager::getInstance().getListener().get();
                HWC_LOGI("Register hotplug callback(ptr=%p)", m_callback_hotplug);
                if (listener != NULL)
                {
                    listener->m_callback_hotplug =  m_callback_hotplug;
                    listener->m_callback_hotplug_data = m_callback_hotplug_data;
                    DisplayManager::getInstance().resentCallback();
                }
            }
            break;

        case HWC2_CALLBACK_VSYNC:
            {
                m_callback_vsync = reinterpret_cast<HWC2_PFN_VSYNC>(pointer);
                m_callback_vsync_data = callback_data;
                sp<DisplayListener> listener = (DisplayListener *) DisplayManager::getInstance().getListener().get();
                HWC_LOGI("Register vsync callback(ptr=%p)", m_callback_vsync);
                if (listener != NULL)
                {
                    listener->m_callback_vsync = m_callback_vsync;
                    listener->m_callback_vsync_data =m_callback_vsync_data;
                }

                // SurfaceFlinger has removed VSync callback, so we should disable VSync thread.
                // It can avoid that SurfaceFlinger get VSync when new SurfaceFlinger registers
                // a new callback function without enable VSync.
                if (m_callback_vsync == NULL)
                {
                    for (auto& hwc_display : m_displays)
                    {
                        hwc_display->setVsyncEnabled(false);
                    }
                }
            }
            break;
        case HWC2_CALLBACK_REFRESH:
            {
                m_callback_refresh = reinterpret_cast<HWC2_PFN_REFRESH>(pointer);
                m_callback_refresh_data = callback_data;
                sp<DisplayListener> listener = (DisplayListener *) DisplayManager::getInstance().getListener().get();
                HWC_LOGI("Register refresh callback(ptr=%p)", m_callback_refresh);
                if (listener != NULL)
                {
                    listener->m_callback_refresh = m_callback_refresh;
                    listener->m_callback_refresh_data = m_callback_refresh_data;
                }
            }
            break;

        default:
            HWC_LOGE("%s: unknown descriptor(%d)", __func__, descriptor);
            return HWC2_ERROR_BAD_PARAMETER;
    }

    if (m_callback_vsync && m_callback_hotplug && m_callback_refresh && !m_is_init_disp_manager)
    {
        m_is_init_disp_manager = true;
        DisplayManager::getInstance().setListener(
            new DisplayListener(
                m_callback_hotplug,
                m_callback_hotplug_data,
                m_callback_vsync,
                m_callback_vsync_data,
                m_callback_refresh,
                m_callback_refresh_data));
        // initialize DisplayManager
        DisplayManager::getInstance().init();
    }
    return HWC2_ERROR_NONE;
}

/* Display functions */
int32_t /*hwc2_error_t*/ HWCMediator::displayAcceptChanges(
    hwc2_device_t* /*device*/,
    hwc2_display_t display)
{
    if (display >= DisplayManager::MAX_DISPLAYS)
    {
        HWC_LOGE("%s: this display(%" PRIu64 ") is invalid", __func__, display);
        return HWC2_ERROR_BAD_DISPLAY;
    }

    if (display >= m_displays.size())
    {
        HWC_LOGE("%s: this display(%" PRIu64 ") is invalid, display size is %zu",
                 __func__, display, m_displays.size());
        return HWC2_ERROR_BAD_DISPLAY;
    }

    if (!DisplayManager::getInstance().getDisplayData(display)->connected)
    {
        HWC_LOGE("%s: the display(%" PRIu64 ") is not connected", __func__, display);
        return HWC2_ERROR_BAD_DISPLAY;
    }

    m_displays[display]->acceptChanges();

    return HWC2_ERROR_NONE;
}

int32_t /*hwc2_error_t*/ HWCMediator::displayCreateLayer(
    hwc2_device_t* /*device*/,
    hwc2_display_t display,
    hwc2_layer_t* out_layer)
{
    if (display >= DisplayManager::MAX_DISPLAYS)
    {
        HWC_LOGE("%s: this display(%" PRIu64 ") is invalid", __func__, display);
        return HWC2_ERROR_BAD_DISPLAY;
    }

    if (display >= m_displays.size())
    {
        HWC_LOGE("%s: this display(%" PRIu64 ") is invalid, display size is %zu",
                 __func__, display, m_displays.size());
        return HWC2_ERROR_BAD_DISPLAY;
    }

    if (!DisplayManager::getInstance().getDisplayData(display)->connected)
    {
        HWC_LOGE("%s: the display(%" PRIu64 ") is not connected", __func__, display);
        return HWC2_ERROR_BAD_DISPLAY;
    }

    checkDisplayState();

    return m_displays[display]->createLayer(out_layer, false);
}

int32_t /*hwc2_error_t*/ HWCMediator::displayDestroyLayer(
    hwc2_device_t* /*device*/,
    hwc2_display_t display,
    hwc2_layer_t layer)
{
    if (display >= DisplayManager::MAX_DISPLAYS)
    {
        HWC_LOGE("%s: this display(%" PRIu64 ") is invalid", __func__, display);
        return HWC2_ERROR_BAD_DISPLAY;
    }

    if (display >= m_displays.size())
    {
        HWC_LOGE("%s: this display(%" PRIu64 ") is invalid, display size is %zu",
                 __func__, display, m_displays.size());
        return HWC2_ERROR_BAD_DISPLAY;
    }

    if (!DisplayManager::getInstance().getDisplayData(display)->connected)
    {
        HWC_LOGE("%s: the display(%" PRIu64 ") is not connected", __func__, display);
        return HWC2_ERROR_BAD_DISPLAY;
    }

    sp<HWCLayer> hwc_layer = m_displays[display]->getLayer(layer);
    if (hwc_layer == nullptr)
    {
        HWC_LOGE("%s: the display(%" PRIu64 ") does not contain layer(%" PRIu64 ")", __func__, display, layer);
        return HWC2_ERROR_BAD_LAYER;
    }
    else if(hwc_layer->isClientTarget())
    {
        HWC_LOGE("%s: the display(%" PRIu64 ") can't remove ClientTarget(%" PRIu64 ")", __func__, display, layer);
        return HWC2_ERROR_BAD_LAYER;
    }

    return m_displays[display]->destroyLayer(layer);
}

int32_t /*hwc2_error_t*/ HWCMediator::displayGetActiveConfig(
    hwc2_device_t* /*device*/,
    hwc2_display_t display,
    hwc2_config_t* out_config)
{
    if (display >= DisplayManager::MAX_DISPLAYS)
    {
        HWC_LOGE("%s: this display(%" PRIu64 ") is invalid", __func__, display);
        return HWC2_ERROR_BAD_DISPLAY;
    }

    if (display >= m_displays.size())
    {
        HWC_LOGE("%s: this display(%" PRIu64 ") is invalid, display size is %zu",
                 __func__, display, m_displays.size());
        return HWC2_ERROR_BAD_DISPLAY;
    }

    if (!DisplayManager::getInstance().getDisplayData(display)->connected)
    {
        HWC_LOGE("%s: the display(%" PRIu64 ") is not connected", __func__, display);
        return HWC2_ERROR_BAD_DISPLAY;
    }

    *out_config = m_displays[display]->getActiveConfig();
    HWC_LOGI("(%" PRIu64 ") %s config:%d", display, __func__, *out_config);

    return HWC2_ERROR_NONE;
}

int32_t /*hwc2_error_t*/ HWCMediator::displayGetChangedCompositionTypes(
    hwc2_device_t* /*device*/,
    hwc2_display_t display,
    uint32_t* out_num_elem,
    hwc2_layer_t* out_layers,
    int32_t* /*hwc2_composition_t*/ out_types)
{
    if (display >= DisplayManager::MAX_DISPLAYS)
    {
        HWC_LOGE("%s: this display(%" PRIu64 ") is invalid", __func__, display);
        return HWC2_ERROR_BAD_DISPLAY;
    }

    if (display >= m_displays.size())
    {
        HWC_LOGE("%s: this display(%" PRIu64 ") is invalid, display size is %zu",
                 __func__, display, m_displays.size());
        return HWC2_ERROR_BAD_DISPLAY;
    }

    if (!DisplayManager::getInstance().getDisplayData(display)->connected)
    {
        HWC_LOGE("%s: the display(%" PRIu64 ") is not connected", __func__, display);
        return HWC2_ERROR_BAD_DISPLAY;
    }

    if (!m_displays[display]->isValidated())
    {
        HWC_LOGE("%s: the display(%" PRIu64 ") does not validate yet", __func__, display);
        return HWC2_ERROR_NOT_VALIDATED;
    }

    m_displays[display]->getChangedCompositionTypes(out_num_elem, out_layers, out_types);
    return HWC2_ERROR_NONE;
}

int32_t /*hwc2_error_t*/ HWCMediator::displayGetClientTargetSupport(
    hwc2_device_t* /*device*/,
    hwc2_display_t display,
    uint32_t width,
    uint32_t height,
    int32_t /*android_pixel_format_t*/ format,
    int32_t /*android_dataspace_t*/ dataspace)
{
    if (display >= DisplayManager::MAX_DISPLAYS)
    {
        HWC_LOGE("%s: this display(%" PRIu64 ") is invalid", __func__, display);
        return HWC2_ERROR_BAD_DISPLAY;
    }

    if (display >= m_displays.size())
    {
        HWC_LOGE("%s: this display(%" PRIu64 ") is invalid, display size is %zu",
                 __func__, display, m_displays.size());
        return HWC2_ERROR_BAD_DISPLAY;
    }

    sp<HWCDisplay> disp = m_displays[display];
    if (disp->getWidth() != width || disp->getHeight() != height)
    {
        HWC_LOGW("%s: this display(%" PRIu64 ") does not support the CT format(%ux%u:%d:%d)",
                 __func__, display, width, height, format, dataspace);
        return HWC2_ERROR_UNSUPPORTED;
    }

    return HWC2_ERROR_NONE;
}

int32_t /*hwc2_error_t*/ HWCMediator::displayGetColorMode(
    hwc2_device_t* /*device*/,
    hwc2_display_t display,
    uint32_t* out_num_modes,
    int32_t* out_modes)
{
    Vector<int32_t> color_mode;
    DisplayManager::getInstance().getSupportedColorModeVector(display, color_mode);
    if (out_modes == nullptr)
    {
        *out_num_modes = color_mode.size();
    }
    else
    {
        for (size_t i = 0; i < color_mode.size(); i++)
        {
            out_modes[i] = color_mode[i];
        }
    }

    return HWC2_ERROR_NONE;
}

int32_t /*hwc2_error_t*/ HWCMediator::displayGetAttribute(
    hwc2_device_t* /*device*/,
    hwc2_display_t display,
    hwc2_config_t config,
    int32_t /*hwc2_attribute_t*/ attribute,
    int32_t* out_value)
{
    if (display >= DisplayManager::MAX_DISPLAYS)
    {
        HWC_LOGE("%s: this display(%" PRIu64 ") is invalid", __func__, display);
        return HWC2_ERROR_BAD_DISPLAY;
    }

    if (display >= m_displays.size())
    {
        HWC_LOGE("%s: this display(%" PRIu64 ") is invalid, display size is %zu",
                 __func__, display, m_displays.size());
        return HWC2_ERROR_BAD_DISPLAY;
    }

    if (!DisplayManager::getInstance().getDisplayData(display)->connected)
    {
        HWC_LOGE("%s: the display(%" PRIu64 ") is not connected", __func__, display);
        return HWC2_ERROR_BAD_DISPLAY;
    }

    if (display == HWC_DISPLAY_PRIMARY ?
        config >= m_displays[display]->getNumConfigs() :
        config != 0)
    {
        HWC_LOGE("%s: unknown display config id(%d)!", __func__, config);
        return HWC2_ERROR_BAD_CONFIG;
    }

    switch (attribute)
    {
        case HWC2_ATTRIBUTE_WIDTH:
            *out_value = m_displays[display]->getWidth(config);
            break;

        case HWC2_ATTRIBUTE_HEIGHT:
            *out_value = m_displays[display]->getHeight(config);
            break;

        case HWC2_ATTRIBUTE_VSYNC_PERIOD:
            *out_value = m_displays[display]->getVsyncPeriod(config);
            break;

        case HWC2_ATTRIBUTE_DPI_X:
            *out_value = m_displays[display]->getDpiX(config);
            break;

        case HWC2_ATTRIBUTE_DPI_Y:
            *out_value = m_displays[display]->getDpiY(config);
            break;

        case HWC2_ATTRIBUTE_VALIDATE_DATA:
            unpackageMtkData(*(reinterpret_cast<HwcValidateData*>(out_value)));
            break;

        default:
            HWC_LOGE("%s: unknown attribute(%d)!", __func__, attribute);
            return HWC2_ERROR_BAD_CONFIG;
    }
    HWC_LOGI("(%" PRIu64 ") Attribute: config:%d, attr:%d, value:%d", display, config, attribute, *out_value);
    return HWC2_ERROR_NONE;
}

void HWCMediator::unpackageMtkData(const HwcValidateData& val_data)
{
    for (auto& kv : val_data.layers)
    {
        auto& layer = kv.second;
        const int64_t& layer_id = layer->id;
        const int64_t& disp_id = layer->disp_id;

        sp<HWCLayer>&& hwc_layer = m_displays[disp_id]->getLayer(layer_id);
        if (hwc_layer != nullptr)
            hwc_layer->setMtkFlags(layer->flags);
        else
            HWC_LOGE("unpackageMtkData(): invalid layer id(%" PRId64 ") disp(%" PRId64 ")", layer_id, disp_id);
    }

    for (auto& display : val_data.displays)
    {
        const int64_t& id = display.id;

        sp<HWCDisplay>& hwc_display = m_displays[id];
        if (hwc_display != nullptr)
            hwc_display->setMtkFlags(display.flags);
    }
}

int32_t /*hwc2_error_t*/ HWCMediator::displayGetConfigs(
    hwc2_device_t* /*device*/,
    hwc2_display_t display,
    uint32_t* out_num_configs,
    hwc2_config_t* out_configs)
{
    if (display >= DisplayManager::MAX_DISPLAYS)
    {
        HWC_LOGE("%s: this display(%" PRIu64 ") is invalid", __func__, display);
        return HWC2_ERROR_BAD_DISPLAY;
    }

    if (display >= m_displays.size())
    {
        HWC_LOGE("%s: this display(%" PRIu64 ") is invalid, display size is %zu",
                 __func__, display, m_displays.size());
        return HWC2_ERROR_BAD_DISPLAY;
    }

    if (!DisplayManager::getInstance().getDisplayData(display)->connected)
    {
        HWC_LOGE("%s: the display(%" PRIu64 ") is not connected", __func__, display);
        return HWC2_ERROR_BAD_DISPLAY;
    }

    if (out_configs == nullptr)
    {
        *out_num_configs = display != HWC_DISPLAY_PRIMARY ? 1 : m_displays[display]->getNumConfigs();
    }
    else
    {
        for (int i = 0; i < *out_num_configs; ++i)
        {
            out_configs[i] = i;
        }
    }

    return HWC2_ERROR_NONE;
}

int32_t /*hwc2_error_t*/ HWCMediator::displayGetName(
    hwc2_device_t* /*device*/,
    hwc2_display_t /*display*/,
    uint32_t* /*out_lens*/,
    char* /*out_name*/)
{
    return HWC2_ERROR_NONE;
}

int32_t /*hwc2_error_t*/ HWCMediator::displayGetRequests(
    hwc2_device_t* /*device*/,
    hwc2_display_t display,
    int32_t* /*hwc2_display_request_t*/ /*out_display_requests*/,
    uint32_t* out_num_elem,
    hwc2_layer_t* /*out_layer*/,
    int32_t* /*hwc2_layer_request_t*/ /*out_layer_requests*/)
{
    if (!DisplayManager::getInstance().getDisplayData(display)->connected)
    {
        HWC_LOGE("%s: the display(%" PRIu64 ") is not connected", __func__, display);
        return HWC2_ERROR_BAD_DISPLAY;
    }

    *out_num_elem = 0;

    return HWC2_ERROR_NONE;
}

int32_t /*hwc2_error_t*/ HWCMediator::displayGetType(
    hwc2_device_t* /*device*/,
    hwc2_display_t display,
    int32_t* /*hwc2_display_type_t*/ out_type)
{
    if (display >= DisplayManager::MAX_DISPLAYS)
    {
        HWC_LOGE("%s: this display(%" PRIu64 ") is invalid", __func__, display);
        return HWC2_ERROR_BAD_DISPLAY;
    }

    if (display >= m_displays.size())
    {
        HWC_LOGE("%s: this display(%" PRIu64 ") is invalid, display size is %zu",
                 __func__, display, m_displays.size());
        return HWC2_ERROR_BAD_DISPLAY;
    }

    if (!DisplayManager::getInstance().getDisplayData(display)->connected)
    {
        HWC_LOGE("%s: the display(%" PRIu64 ") is not connected", __func__, display);
        return HWC2_ERROR_BAD_DISPLAY;
    }

    m_displays[display]->getType(out_type);

    return HWC2_ERROR_NONE;
}

int32_t /*hwc2_error_t*/ HWCMediator::displayGetDozeSupport(
    hwc2_device_t* /*device*/,
    hwc2_display_t display,
    int32_t* out_support)
{
    if (display >= DisplayManager::MAX_DISPLAYS)
    {
        HWC_LOGE("%s: this display(%" PRIu64 ") is invalid", __func__, display);
        return HWC2_ERROR_BAD_DISPLAY;
    }

    if (display >= m_displays.size())
    {
        HWC_LOGE("%s: this display(%" PRIu64 ") is invalid, display size is %zu",
                 __func__, display, m_displays.size());
        return HWC2_ERROR_BAD_DISPLAY;
    }

    auto& hwc_display = m_displays[display];
    if (!hwc_display->isConnected())
    {
        HWC_LOGE("%s: the display(%" PRIu64 ") is not connected", __func__, display);
        return HWC2_ERROR_BAD_DISPLAY;
    }

    if (display == HWC_DISPLAY_PRIMARY)
    {
        *out_support = HWCMediator::getInstance().m_features.aod;
    }
    else
    {
        *out_support = false;
    }

    return HWC2_ERROR_NONE;
}

int32_t /*hwc2_error_t*/ HWCMediator::displayGetHdrCapability(
    hwc2_device_t* /*device*/,
    hwc2_display_t display,
    uint32_t* out_num_types,
    int32_t* /*android_hdr_t*/ out_types,
    float* /*out_max_luminance*/,
    float* /*out_max_avg_luminance*/,
    float* /*out_min_luminance*/)
{
    if (!DisplayManager::getInstance().getDisplayData(display)->connected)
    {
        HWC_LOGE("%s: the display(%" PRIu64 ") is not connected", __func__, display);
        return HWC2_ERROR_BAD_DISPLAY;
    }
    if (out_types == NULL)
    {
        *out_num_types = HwcHdrUtils::getInstance().m_hdr_capabilities.size();
    }
    else
    {
        for(uint32_t i = 0; i < *out_num_types; ++i)
        {
            out_types[i] = HwcHdrUtils::getInstance().m_hdr_capabilities[i];
        }
    }

    return HWC2_ERROR_NONE;
}

int32_t /*hwc2_error_t*/ HWCMediator::displayGetPerFrameMetadataKeys(
    hwc2_device_t* /*device*/,
    hwc2_display_t display,
    uint32_t* outNumKeys,
    int32_t* /*hwc2_per_frame_metadata_key_t*/ outKeys)
{
    auto& hwc_display = m_displays[display];
    if (!hwc_display->isConnected())
    {
        HWC_LOGE("%s: the display(%" PRIu64 ") is not connected", __func__, display);
        return HWC2_ERROR_BAD_DISPLAY;
    }

    if (outKeys == NULL)
    {
        *outNumKeys = HwcHdrUtils::getInstance().m_hdr_metadata_keys.size();
    }
    else
    {
        for(uint32_t i = 0; i < *outNumKeys; ++i)
        {
            outKeys[i] = HwcHdrUtils::getInstance().m_hdr_metadata_keys[i];
        }
    }
    return HWC2_ERROR_NONE;
}

int32_t /*hwc2_error_t*/ HWCMediator::displayGetReleaseFence(
    hwc2_device_t* /*device*/,
    hwc2_display_t display,
    uint32_t* out_num_elem,
    hwc2_layer_t* out_layer,
    int32_t* out_fence)
{
    if (display >= DisplayManager::MAX_DISPLAYS)
    {
        HWC_LOGE("%s: this display(%" PRIu64 ") is invalid", __func__, display);
        return HWC2_ERROR_BAD_DISPLAY;
    }

    if (display >= m_displays.size())
    {
        HWC_LOGE("%s: this display(%" PRIu64 ") is invalid, display size is %zu",
                 __func__, display, m_displays.size());
        return HWC2_ERROR_BAD_DISPLAY;
    }

    if (!DisplayManager::getInstance().getDisplayData(display)->connected)
    {
        HWC_LOGE("%s: the display(%" PRIu64 ") is not connected", __func__, display);
        return HWC2_ERROR_BAD_DISPLAY;
    }

    m_displays[display]->getReleaseFenceFds(out_num_elem, out_layer, out_fence);
    return HWC2_ERROR_NONE;
}

int32_t /*hwc2_error_t*/ HWCMediator::displayPresent(
    hwc2_device_t* /*device*/,
    hwc2_display_t display,
    int32_t* out_retire_fence)
{
    if (display >= DisplayManager::MAX_DISPLAYS)
    {
        HWC_LOGE("%s: this display(%" PRIu64 ") is invalid", __func__, display);
        return HWC2_ERROR_BAD_DISPLAY;
    }

    if (display >= m_displays.size())
    {
        HWC_LOGE("%s: this display(%" PRIu64 ") is invalid, display size is %zu",
                 __func__, display, m_displays.size());
        return HWC2_ERROR_BAD_DISPLAY;
    }

    HWC_LOGD("(%" PRIu64 ") %s s:%s", display, __func__, getPresentValiStateString(m_displays[display]->getValiPresentState()));
    AbortMessager::getInstance().printf("(%" PRIu64 ") %s s:%s=>", display, __func__, getPresentValiStateString(m_displays[display]->getValiPresentState()));
    if (!DisplayManager::getInstance().getDisplayData(display)->connected)
    {
        HWC_LOGE("%s: the display(%" PRIu64 ") is not connected", __func__, display);
        unlockRefreshThread(display);
        return HWC2_ERROR_BAD_DISPLAY;
    }

    if (hasCapabilities(HWC2_CAPABILITY_SKIP_VALIDATE) &&
        (m_displays[display]->getValiPresentState() == HWC_VALI_PRESENT_STATE_PRESENT_DONE ||
         m_displays[display]->getValiPresentState() == HWC_VALI_PRESENT_STATE_VALIDATE ||
         m_displays[display]->getValiPresentState() == HWC_VALI_PRESENT_STATE_CHECK_SKIP_VALI))
    {
        if (m_displays[display]->getValiPresentState() == HWC_VALI_PRESENT_STATE_PRESENT_DONE &&
            m_displays[display]->getUnpresentCount() == 0)
        {
            buildVisibleAndInvisibleLayerForAllDisplay();

            bool ultra_scenario = false;
            int32_t mirror_sink_dpy = -1;
            if (m_displays[HWC_DISPLAY_EXTERNAL]->isConnected() ||
                m_displays[HWC_DISPLAY_VIRTUAL]->isConnected())
            {
                mirror_sink_dpy = checkMirrorPath(m_displays, &ultra_scenario);
                HWCDispatcher::getInstance().m_ultra_scenario = ultra_scenario;
            }

            const bool use_decouple_mode = mirror_sink_dpy != -1;
            HWCDispatcher::getInstance().setSessionMode(HWC_DISPLAY_PRIMARY, use_decouple_mode);

            prepareForValidation();
            setNeedValidate(HWC_SKIP_VALIDATE_NOT_SKIP);
            setValiPresentStateOfAllDisplay(HWC_VALI_PRESENT_STATE_CHECK_SKIP_VALI, __LINE__);
            if (checkSkipValidate() == true)
                setNeedValidate(HWC_SKIP_VALIDATE_SKIP);
            else
                setNeedValidate(HWC_SKIP_VALIDATE_NOT_SKIP);
        }

        if (getNeedValidate() == HWC_SKIP_VALIDATE_NOT_SKIP)
        {
            return HWC2_ERROR_NOT_VALIDATED;
        }
        else
        {
            m_hrt->run(m_displays, true);
            updateGlesRangeForAllDisplays();
            setValiPresentStateOfAllDisplay(HWC_VALI_PRESENT_STATE_VALIDATE_DONE, __LINE__);
        }
    }

    if (m_displays[display]->getValiPresentState() == HWC_VALI_PRESENT_STATE_VALIDATE_DONE)
    {
        if (m_displays[display]->isConnected())
        {
            m_displays[display]->buildCommittedLayers();
            HWC_LOGV("(%" PRIu64 ") %s getCommittedLayers() size:%d", m_displays[display]->getId(), __func__, m_displays[display]->getCommittedLayers().size());
            m_displays[display]->beforePresent(getLastSFValidateNum());
            if (display == HWC_DISPLAY_PRIMARY)
            {
                adjustVsyncOffset();
            }
            m_displays[display]->present();
            HWCDispatcher::getInstance().trigger(display);
        }
    }

    if (display == HWC_DISPLAY_PRIMARY)
    {
        if (m_displays[display]->getRetireFenceFd() == -1)
        {
            *out_retire_fence = -1;
        }
        else
        {
#ifdef USES_FENCE_RENAME
            *out_retire_fence = sync_merge("HWC_to_SF_present", m_displays[display]->getRetireFenceFd(), m_displays[display]->getRetireFenceFd());
            if (*out_retire_fence < 0)
            {
                HWC_LOGE("(%" PRIu64 ") %s merge present fence(%d) failed", display, __func__, m_displays[display]->getRetireFenceFd());
            }
#else
            *out_retire_fence = ::dup(m_displays[display]->getRetireFenceFd());
#endif
            ::protectedClose(m_displays[display]->getRetireFenceFd());
            m_displays[display]->setRetireFenceFd(-1, isDispConnected(display));
        }
    }
    else
    {
        *out_retire_fence = dupCloseFd(m_displays[display]->getRetireFenceFd());
        m_displays[display]->setRetireFenceFd(-1, isDispConnected(display));
    }
    HWC_LOGV("(%" PRIu64 ") %s out_retire_fence:%d", display, __func__, *out_retire_fence);

    m_validate_seq = 0;
    ++m_present_seq;
    m_displays[display]->afterPresent();
    m_displays[display]->setValiPresentState(HWC_VALI_PRESENT_STATE_PRESENT_DONE, __LINE__);
    unlockRefreshThread(display);
    return HWC2_ERROR_NONE;
}

int32_t /*hwc2_error_t*/ HWCMediator::displaySetActiveConfig(
    hwc2_device_t* /*device*/,
    hwc2_display_t display,
    hwc2_config_t config_id)
{
    if (!DisplayManager::getInstance().getDisplayData(display)->connected)
    {
        HWC_LOGE("%s: the display(%" PRIu64 ") is not connected", __func__, display);
        return HWC2_ERROR_BAD_DISPLAY;
    }

    if (display == HWC_DISPLAY_PRIMARY ?
        config_id >= m_displays[display]->getNumConfigs() :
        config_id != 0)
    {
        HWC_LOGE("%s: wrong config id(%d)", __func__, config_id);
        return HWC2_ERROR_BAD_CONFIG;
    }

    if (display == HWC_DISPLAY_PRIMARY)
    {
        m_displays[display]->setActiveConfig(config_id);
    }
    HWC_LOGI("(%" PRIu64 ") %s config:%d", display, __func__, config_id);

    return HWC2_ERROR_NONE;
}

int32_t /*hwc2_error_t*/ HWCMediator::displaySetClientTarget(
    hwc2_device_t* /*device*/,
    hwc2_display_t display,
    buffer_handle_t handle,
    int32_t acquire_fence,
    int32_t /*dataspace*/,
    hwc_region_t damage)
{
    if (display >= DisplayManager::MAX_DISPLAYS)
    {
        HWC_LOGE("%s: this display(%" PRIu64 ") is invalid", __func__, display);
        return HWC2_ERROR_BAD_DISPLAY;
    }

    if (display >= m_displays.size())
    {
        HWC_LOGE("%s: this display(%" PRIu64 ") is invalid, display size is %zu",
                 __func__, display, m_displays.size());
        return HWC2_ERROR_BAD_DISPLAY;
    }

    if (!DisplayManager::getInstance().getDisplayData(display)->connected)
    {
        HWC_LOGE("%s: the display(%" PRIu64 ") is not connected", __func__, display);
        return HWC2_ERROR_BAD_DISPLAY;
    }

    sp<HWCLayer> ct = m_displays[display]->getClientTarget();

    ct->setHandle(handle);
    if (ct->getAcquireFenceFd() != -1) {
        ::protectedClose(ct->getAcquireFenceFd());
        ct->setAcquireFenceFd(-1, isDispConnected(display));
    }
    ct->setAcquireFenceFd(acquire_fence, isDispConnected(display));
    ct->setDataspace(mapColorMode2DataSpace(m_displays[display]->getColorMode()));
    ct->setDamage(damage);
    ct->setupPrivateHandle();

    if (display == HWC_DISPLAY_VIRTUAL && m_displays[display]->getMirrorSrc() == -1 &&
        !Platform::getInstance().m_config.is_support_ext_path_for_virtual &&
        !HWCMediator::getInstance().m_features.copyvds)
    {
        const int32_t dup_acq_fence_fd = ::dup(acquire_fence);
        HWC_LOGV("(%" PRIu64 ") setClientTarget() handle:%p acquire_fence:%d(%d)", display, handle, acquire_fence, dup_acq_fence_fd);
        m_displays[display]->setRetireFenceFd(dup_acq_fence_fd, true);
    }
    else
    {
        HWC_LOGV("(%" PRIu64 ") setClientTarget() handle:%p acquire_fence:%d", display, handle, acquire_fence);
    }
    return HWC2_ERROR_NONE;
}

int32_t /*hwc2_error_t*/ HWCMediator::displaySetColorMode(
    hwc2_device_t* /*device*/,
    hwc2_display_t display,
    int32_t mode)
{
    if (display >= DisplayManager::MAX_DISPLAYS)
    {
        HWC_LOGE("%s: this display(%" PRIu64 ") is invalid", __func__, display);
        return HWC2_ERROR_BAD_DISPLAY;
    }

    if (display >= m_displays.size())
    {
        HWC_LOGE("%s: this display(%" PRIu64 ") is invalid, display size is %zu",
                 __func__, display, m_displays.size());
        return HWC2_ERROR_BAD_DISPLAY;
    }

    if (!m_displays[display]->isConnected())
    {
        HWC_LOGE("%s: the display(%" PRIu64 ") is not connected", __func__, display);
        return HWC2_ERROR_BAD_DISPLAY;
    }

    HWC_LOGI("(%" PRIu64 ") %s mode:%d", display, __func__, mode);
    if (mode < 0)
    {
        return HWC2_ERROR_BAD_PARAMETER;
    }

    m_displays[display]->setColorMode(mode);

    Vector<int32_t> color_mode;
    DisplayManager::getInstance().getSupportedColorModeVector(display, color_mode);

    for (size_t i = 0; i < color_mode.size(); i++)
    {
        if (mode == color_mode[i])
        {
            return HWC2_ERROR_NONE;
        }
    }
    return HWC2_ERROR_UNSUPPORTED;
}

int32_t /*hwc2_error_t*/ HWCMediator::displaySetColorTransform(
    hwc2_device_t* /*device*/,
    hwc2_display_t display,
    const float* matrix,
    int32_t /*android_color_transform_t*/ hint)
{
    if (display >= DisplayManager::MAX_DISPLAYS)
    {
        HWC_LOGE("%s: this display(%" PRIu64 ") is invalid", __func__, display);
        return HWC2_ERROR_BAD_DISPLAY;
    }

    if (display >= m_displays.size())
    {
        HWC_LOGE("%s: this display(%" PRIu64 ") is invalid, display size is %zu",
                 __func__, display, m_displays.size());
        return HWC2_ERROR_BAD_DISPLAY;
    }

    if (!m_displays[display]->isConnected())
    {
        HWC_LOGE("%s: the display(%" PRIu64 ") is not connected", __func__, display);
        return HWC2_ERROR_BAD_DISPLAY;
    }

    if (display == HWC_DISPLAY_PRIMARY ||
        display == HWC_DISPLAY_VIRTUAL)
    {
        return m_displays[display]->setColorTransform(matrix, hint);
    }
    else
    {
        return HWC2_ERROR_UNSUPPORTED;
    }
    return HWC2_ERROR_NONE;
}

int32_t /*hwc2_error_t*/ HWCMediator::displaySetOutputBuffer(
    hwc2_device_t* /*device*/,
    hwc2_display_t display,
    buffer_handle_t buffer,
    int32_t release_fence)
{
    if (display >= DisplayManager::MAX_DISPLAYS)
    {
        HWC_LOGE("%s: this display(%" PRIu64 ") is invalid", __func__, display);
        return HWC2_ERROR_BAD_DISPLAY;
    }

    if (display >= m_displays.size())
    {
        HWC_LOGE("%s: this display(%" PRIu64 ") is invalid, display size is %zu",
                 __func__, display, m_displays.size());
        return HWC2_ERROR_BAD_DISPLAY;
    }

    if (display != HWC_DISPLAY_VIRTUAL)
    {
        HWC_LOGE("%s: invalid display(%" PRIu64 ")", __func__, display);
        return HWC2_ERROR_UNSUPPORTED;
    }

    if (!m_displays[display]->isConnected())
    {
        HWC_LOGE("%s: the display(%" PRIu64 ") is not connected", __func__, display);
        return HWC2_ERROR_BAD_DISPLAY;
    }

    const int32_t dup_fd = ::dup(release_fence);
    HWC_LOGV("(%" PRIu64 ") %s outbuf fence:%d->%d", display, __func__, release_fence, dup_fd);
    m_displays[display]->setOutbuf(buffer, dup_fd);
    return HWC2_ERROR_NONE;
}

int32_t /*hwc2_error_t*/ HWCMediator::displaySetPowerMode(
    hwc2_device_t* /*device*/,
    hwc2_display_t display,
    int32_t /*hwc2_power_mode_t*/ mode)
{
    if (display >= DisplayManager::MAX_DISPLAYS)
    {
        HWC_LOGE("%s: this display(%" PRIu64 ") is invalid", __func__, display);
        return HWC2_ERROR_BAD_DISPLAY;
    }

    if (display >= m_displays.size())
    {
        HWC_LOGE("%s: this display(%" PRIu64 ") is invalid, display size is %zu",
                 __func__, display, m_displays.size());
        return HWC2_ERROR_BAD_DISPLAY;
    }

    if (!DisplayManager::getInstance().getDisplayData(display)->connected)
    {
        HWC_LOGE("%s: the display(%" PRIu64 ") is not connected", __func__, display);
        return HWC2_ERROR_BAD_DISPLAY;
    }

    HWC_LOGD("%s display:%" PRIu64 " mode:%d", __func__, display, mode);
    if (!HWCMediator::getInstance().m_features.aod &&
        (mode == HWC2_POWER_MODE_DOZE || mode == HWC2_POWER_MODE_DOZE_SUSPEND))
    {
        return HWC2_ERROR_UNSUPPORTED;
    }

    switch (mode)
    {
        case HWC2_POWER_MODE_OFF:
        case HWC2_POWER_MODE_ON:
        case HWC2_POWER_MODE_DOZE:
        case HWC2_POWER_MODE_DOZE_SUSPEND:
            m_displays[display]->setPowerMode(mode);
            break;

        default:
            HWC_LOGE("%s: display(%" PRIu64 ") a unknown parameter(%d)!", __func__, display, mode);
            return HWC2_ERROR_BAD_PARAMETER;
    }

    return HWC2_ERROR_NONE;
}

int32_t /*hwc2_error_t*/ HWCMediator::displaySetVsyncEnabled(
    hwc2_device_t* /*device*/,
    hwc2_display_t display,
    int32_t /*hwc2_vsync_t*/ enabled)
{
    if (display >= DisplayManager::MAX_DISPLAYS)
    {
        HWC_LOGE("%s: this display(%" PRIu64 ") is invalid", __func__, display);
        return HWC2_ERROR_BAD_DISPLAY;
    }

    if (display >= m_displays.size())
    {
        HWC_LOGE("%s: this display(%" PRIu64 ") is invalid, display size is %zu",
                 __func__, display, m_displays.size());
        return HWC2_ERROR_BAD_DISPLAY;
    }

    if (!DisplayManager::getInstance().getDisplayData(display)->connected)
    {
        HWC_LOGE("%s: the display(%" PRIu64 ") is not connected", __func__, display);
        return HWC2_ERROR_BAD_DISPLAY;
    }

    switch (enabled)
    {
        case HWC2_VSYNC_ENABLE:
            m_displays[display]->setVsyncEnabled(true);
            break;

        case HWC2_VSYNC_DISABLE:
            m_displays[display]->setVsyncEnabled(false);
            break;

        default:
            HWC_LOGE("%s: display( %" PRIu64 ") a unknown parameter(%d)!", __func__, display, enabled);
            return HWC2_ERROR_BAD_PARAMETER;
    }
    return HWC2_ERROR_NONE;
}

static int32_t findLimitedVideo(const vector<sp<HWCLayer> >& layers)
{
    for (auto& layer : layers)
    {
        if (layer->getHandle() == nullptr)
            continue;

        const PrivateHandle& hnd = layer->getPrivateHandle();
        const int&& type = (hnd.ext_info.status & GRALLOC_EXTRA_MASK_TYPE);
        const size_t&& size = hnd.width * hnd.height;
        if ((type == GRALLOC_EXTRA_BIT_TYPE_VIDEO ||
            type == GRALLOC_EXTRA_BIT_TYPE_CAMERA ||
            hnd.format == HAL_PIXEL_FORMAT_YV12) &&
            size >= Platform::getInstance().getLimitedVideoSize())
        {
            return layer->getId();
        }
    }
    return -1;
}

static bool isMirrorList(const vector<sp<HWCLayer> >& src_layers,
                         const vector<sp<HWCLayer> >& sink_layers,
                         const int32_t& src_disp,
                         const int32_t& sink_disp)
{
    bool ret = false;
    DbgLogger logger(DbgLogger::TYPE_HWC_LOG | DbgLogger::TYPE_DUMPSYS, 'D',"mirror?(%d->%d): ", src_disp, sink_disp);

    if (src_disp == sink_disp)
    {
        logger.printf("E-same_dpy");
        return ret;
    }

    logger.printf("I-size(%zu|%zu) ", sink_layers.size(), src_layers.size());

    if (sink_layers.size() == 0 || src_layers.size() == 0)
    {
        // it will clearBlackground and processMirror with black src buffer
        // this means that clear black twice by MDP in this case
        // therefore, switch this case to extension path to remove redundant clear black
        logger.printf("E-null_list");
        return ret;
    }

    vector<uint64_t> src_layers_alloc_id;
    vector<uint64_t> sink_layers_alloc_id;

    for (auto& layer : src_layers)
    {
        HWC_LOGV("isMirrorList 1 layer->getHandle():%x", layer->getHandle());
        if (layer->getCompositionType() != HWC2_COMPOSITION_SIDEBAND)
        {
            auto& hnd =layer->getPrivateHandle();
            src_layers_alloc_id.push_back(hnd.alloc_id);
        }
        // todo: check sidebandStream
    }

    HWC_LOGV("src_layers_alloc_id size:%d", src_layers_alloc_id.size());

    for (auto& layer : sink_layers)
    {
        HWC_LOGV("isMirrorList 2 layer->getHandle():%x", layer->getHandle());
        if (layer->getCompositionType() != HWC2_COMPOSITION_SIDEBAND)
        {
            auto& hnd =layer->getPrivateHandle();
            sink_layers_alloc_id.push_back(hnd.alloc_id);
        }
        // todo: check sidebandStream
    }

    if (src_layers_alloc_id == sink_layers_alloc_id)
    {
        logger.printf("T2 ");
        ret = true;
    }

    logger.printf("E-%d", ret);
    return ret;
}

// checkMirrorPath() checks if mirror path exists
int32_t checkMirrorPath(const vector<sp<HWCDisplay> >& displays, bool *ultra_scenario)
{
    DbgLogger logger(DbgLogger::TYPE_HWC_LOG | DbgLogger::TYPE_DUMPSYS, 'D', "chkMir(%zu: ", displays.size());

    *ultra_scenario = false;

    // display id of mirror source
    const int32_t mir_dpy = HWC_DISPLAY_PRIMARY;
    auto&& src_layers = displays[HWC_DISPLAY_PRIMARY]->getVisibleLayersSortedByZ();
    for (int32_t i = 1; i <= HWC_DISPLAY_VIRTUAL; ++i)
    {
        const DisplayData* display_data = DisplayManager::getInstance().getDisplayData(i);
        auto& display = displays[i];
        const int32_t disp_id = static_cast<int32_t>(display->getId());
        auto&& layers = display->getVisibleLayersSortedByZ();
        if (DisplayManager::MAX_DISPLAYS <= disp_id)
            continue;

        if (!display->isConnected())
            continue;

        if (display->isGpuComposition())
            continue;

        if (HWC_DISPLAY_PRIMARY == disp_id)
        {
            display->setMirrorSrc(-1);
            logger.printf("(%d:L%d) ", i, __LINE__);
            continue;
        }

        if (listForceGPUComp(layers))
        {
            display->setMirrorSrc(-1);
            logger.printf("(%d:L%d) ", i, __LINE__);
            continue;
        }

        const bool has_limit_video = findLimitedVideo(display->getVisibleLayersSortedByZ()) >= -1;

        const bool is_mirror_list = isMirrorList(src_layers, layers, mir_dpy, disp_id);

        // if hdcp checking is handled by display driver, the extension path must be applied.
        if (Platform::getInstance().m_config.bypass_wlv1_checking && listSecure(layers))
        {
            display->setMirrorSrc(-1);
            logger.printf("(%d:L%d) ", i, __LINE__);
            continue;
        }

        // 4k mhl has 4k video with mirror mode, so need to block 4k video at primary display
        if (disp_id == HWC_DISPLAY_EXTERNAL && is_mirror_list &&
            DisplayManager::getInstance().isUltraDisplay(disp_id) && has_limit_video)
        {
            *ultra_scenario = true;
            logger.printf("(%d:L%d) ", i, __LINE__);
            continue;
        }

        if ((Platform::getInstance().m_config.mirror_state & MIRROR_DISABLED) ||
            (Platform::getInstance().m_config.mirror_state & MIRROR_PAUSED))
        {
            // disable mirror mode
            // either the mirror state is disabled or the mirror source is blanked
            display->setMirrorSrc(-1);
            logger.printf("(%d:L%d) ", disp_id, __LINE__);
            continue;
        }

        // the layer list is different with primary display
        if (!is_mirror_list)
        {
            display->setMirrorSrc(-1);
            logger.printf("(%d:L%d) ", disp_id, __LINE__);
            continue;
        }

        if (!display_data->secure && listSecure(layers))
        {
            // disable mirror mode
            // if any secure or protected layer exists in mirror source
            display->setMirrorSrc(-1);
            logger.printf("(%d:L%d) ", i, __LINE__);
            continue;
        }

        if (!HWCMediator::getInstance().m_features.copyvds &&
            layers.empty())
        {
            // disable mirror mode
            // since force copy vds is not used
            display->setMirrorSrc(-1);
            logger.printf("(%d:L%d) ", i, __LINE__);
            continue;
        }

        // check enlargement ratio (i.e. scale ratio > 0)
        if (Platform::getInstance().m_config.mir_scale_ratio > 0)
        {
            const DisplayData* mir_display_data =
                DisplayManager::getInstance().getDisplayData(mir_dpy, displays[mir_dpy]->getActiveConfig());
            float scaled_ratio = display_data->pixels / static_cast<float>(mir_display_data->pixels);

            if (scaled_ratio > Platform::getInstance().m_config.mir_scale_ratio)
            {
                // disable mirror mode
                // since scale ratio exceeds the maximum one
                display->setMirrorSrc(-1);
                logger.printf("(%d:L%d) ", i, __LINE__);
                continue;
            }
        }

        if (display_data->is_s3d_support &&
            HWC_DISPLAY_EXTERNAL == disp_id && listS3d(layers))
        {
            display->setMirrorSrc(-1);
            logger.printf("(%d:L%d) ", i, __LINE__);
            continue;
        }

        if (display_data->is_s3d_support &&
            HWC_DISPLAY_EXTERNAL == disp_id && listS3d(layers))
        {
            display->setMirrorSrc(-1);
            logger.printf("(%d:L%d) ", i, __LINE__);
            continue;
        }

        display->setMirrorSrc(mir_dpy);
        logger.printf("mir");
        return disp_id;
    }
    logger.printf("!mir");
    return -1;
}

void HWCMediator::updateGlesRangeForAllDisplays()
{
    for (auto& display : m_displays)
    {
        if (!display->isConnected())
            continue;

        display->updateGlesRange();
    }
}

int32_t /*hwc2_error_t*/ HWCMediator::displayValidateDisplay(
    hwc2_device_t* /*device*/,
    hwc2_display_t display,
    uint32_t* out_num_types,
    uint32_t* out_num_requests)
{
    if (display >= DisplayManager::MAX_DISPLAYS)
    {
        HWC_LOGE("%s: this display(%" PRIu64 ") is invalid", __func__, display);
        return HWC2_ERROR_BAD_DISPLAY;
    }

    if (display >= m_displays.size())
    {
        HWC_LOGE("%s: this display(%" PRIu64 ") is invalid, display size is %zu",
                 __func__, display, m_displays.size());
        return HWC2_ERROR_BAD_DISPLAY;
    }

    HWC_LOGD("(%" PRIu64 ") %s s:%s", display, __func__, getPresentValiStateString(m_displays[display]->getValiPresentState()));
    bool is_validate_only_one_display = false;
    AbortMessager::getInstance().printf("(%" PRIu64 ") %s s:%s", display, __func__, getPresentValiStateString(m_displays[display]->getValiPresentState()));
    lockRefreshThread(display);
    if (!DisplayManager::getInstance().getDisplayData(display)->connected)
    {
        unlockRefreshThread(display);
        return HWC2_ERROR_BAD_DISPLAY;
    }

    if (m_displays[display]->getValiPresentState() == HWC_VALI_PRESENT_STATE_PRESENT_DONE)
    {
        if (m_displays[display]->getUnpresentCount() == 0)
        {
            buildVisibleAndInvisibleLayerForAllDisplay();
            bool ultra_scenario = false;
            int32_t mirror_sink_dpy = -1;
            if (m_displays[HWC_DISPLAY_EXTERNAL]->isConnected() ||
                m_displays[HWC_DISPLAY_VIRTUAL]->isConnected())
            {
                mirror_sink_dpy = checkMirrorPath(m_displays, &ultra_scenario);
                HWCDispatcher::getInstance().m_ultra_scenario = ultra_scenario;
            }

            const bool use_decouple_mode = mirror_sink_dpy != -1;
            HWCDispatcher::getInstance().setSessionMode(HWC_DISPLAY_PRIMARY, use_decouple_mode);

            prepareForValidation();
            setNeedValidate(HWC_SKIP_VALIDATE_NOT_SKIP);
        }
        else
        {
            if (m_displays[display]->getPrevUnpresentCount() >
                m_displays[display]->getUnpresentCount())
            {
                is_validate_only_one_display = true;
                m_displays[display]->buildVisibleAndInvisibleLayer();
                HWCDispatcher::getInstance().getJob(static_cast<int32_t>(m_displays[display]->getId()));
                m_displays[display]->initPrevCompTypes();
                m_displays[display]->setJobDisplayOrientation();
            }
            else
            {
                m_displays[display]->setValiPresentState(HWC_VALI_PRESENT_STATE_VALIDATE, __LINE__);
            }
        }
    }

    if (m_displays[display]->getValiPresentState() == HWC_VALI_PRESENT_STATE_PRESENT_DONE ||
        m_displays[display]->getValiPresentState() == HWC_VALI_PRESENT_STATE_CHECK_SKIP_VALI)
    {
        if (is_validate_only_one_display)
        {
            validate(display);
        }
        else
        {
            validate();
            countdowmSkipValiRelatedNumber();
        }
    }

    vector<sp<HWCLayer> > changed_comp_types;
    const vector<int32_t>& prev_comp_types = m_displays[display]->getPrevCompTypes();
    {
        // DbgLogger logger(DbgLogger::TYPE_HWC_LOG, 'D', "(%d) validateDisplay(): ", display);

        auto&& layers = m_displays[display]->getVisibleLayersSortedByZ();

        for (size_t i = 0; i < layers.size(); ++i)
        {
            if (layers[i]->getCompositionType() != prev_comp_types[i])
            {
                //logger.printf(" [layer(%d) comp type change:(%s->%s)]",
                //        layers[i]->getZOrder(),
                //        getCompString(prev_comp_types[i]),
                //        getCompString(layers[i]->getCompositionType()));
                changed_comp_types.push_back(layers[i]);
                layers[i]->setSFCompositionType(layers[i]->getCompositionType(), false);
            }

#ifndef MTK_USER_BUILD
            HWC_LOGD("(%" PRIu64 ") val %s", display, layers[i]->toString8().string());
#else
            HWC_LOGV("(%" PRIu64 ") val %s", display, layers[i]->toString8().string());
#endif
        }
    }

    m_displays[display]->moveChangedCompTypes(&changed_comp_types);
    *out_num_types = changed_comp_types.size();
    *out_num_requests = 0;

    ++m_validate_seq;
    setLastSFValidateNum(m_validate_seq);
    m_present_seq = 0;
    m_displays[display]->setValiPresentState(HWC_VALI_PRESENT_STATE_VALIDATE_DONE, __LINE__);

    return HWC2_ERROR_NONE;
}

/* Layer functions */
int32_t /*hwc2_error_t*/ HWCMediator::layerSetCursorPosition(
    hwc2_device_t* /*device*/,
    hwc2_display_t display,
    hwc2_layer_t /*layer*/,
    int32_t /*x*/,
    int32_t /*y*/)
{
    if (!DisplayManager::getInstance().getDisplayData(display)->connected)
    {
        HWC_LOGE("%s: the display(%" PRIu64 ") is not connected", __func__, display);
        return HWC2_ERROR_BAD_DISPLAY;
    }

    return HWC2_ERROR_NONE;
}

int32_t /*hwc2_error_t*/ HWCMediator::layerSetBuffer(
    hwc2_device_t* /*device*/,
    hwc2_display_t display,
    hwc2_layer_t layer,
    buffer_handle_t buffer,
    int32_t acquire_fence)
{
    if (display >= DisplayManager::MAX_DISPLAYS)
    {
        HWC_LOGE("%s: this display(%" PRIu64 ") is invalid", __func__, display);
        return HWC2_ERROR_BAD_DISPLAY;
    }

    if (display >= m_displays.size())
    {
        HWC_LOGE("%s: this display(%" PRIu64 ") is invalid, display size is %zu",
                 __func__, display, m_displays.size());
        return HWC2_ERROR_BAD_DISPLAY;
    }

    if (!DisplayManager::getInstance().getDisplayData(display)->connected)
    {
        HWC_LOGE("%s: the display(%" PRIu64 ") is not connected", __func__, display);
        return HWC2_ERROR_BAD_DISPLAY;
    }

    sp<HWCLayer> hwc_layer = m_displays[display]->getLayer(layer);
    if (hwc_layer == nullptr)
    {
        HWC_LOGE("%s: the display(%" PRIu64 ") does not contain layer(%" PRIu64 ")", __func__, display, layer);
        return HWC2_ERROR_BAD_LAYER;
    }

    HWC_LOGD("(%" PRIu64 ") layerSetBuffer() layer id:%" PRIu64 " hnd:%p acquire_fence:%d", display, layer, buffer, acquire_fence);

    if (buffer)
    {
        editSetBufFromSfLog().printf(" %" PRIu64 ":%" PRIu64 ",%x", display, layer,
            static_cast<uint32_t>(((const intptr_t)(buffer) & 0xffff0) >> 4));
    }
    else
    {
        editSetBufFromSfLog().printf(" %" PRIu64 ":%" PRIu64 ",null", display, layer);
    }

    hwc_layer->setHandle(buffer);
    if (hwc_layer->getAcquireFenceFd() != -1) {
        int32_t old_fence = hwc_layer->getAcquireFenceFd();
        if (old_fence != acquire_fence) {
            ::protectedClose(old_fence);
            hwc_layer->setAcquireFenceFd(-1, isDispConnected(display));
        }
        String8 setBufString(editSetBufFromSfLog().getLogString(), editSetBufFromSfLog().getLen());
        HWC_LOGW("%s: redundant fence(%d,%d), dump history:%s", __func__, old_fence, acquire_fence, setBufString.string());
    }
    hwc_layer->setAcquireFenceFd(acquire_fence, isDispConnected(display));

    checkDisplayState();

    return HWC2_ERROR_NONE;
}

int32_t /*hwc2_error_t*/ HWCMediator::layerSetSurfaceDamage(
    hwc2_device_t* /*device*/,
    hwc2_display_t display,
    hwc2_layer_t layer_id,
    hwc_region_t damage)
{
    if (display >= DisplayManager::MAX_DISPLAYS)
    {
        HWC_LOGE("%s: this display(%" PRIu64 ") is invalid", __func__, display);
        return HWC2_ERROR_BAD_DISPLAY;
    }

    if (display >= m_displays.size())
    {
        HWC_LOGE("%s: this display(%" PRIu64 ") is invalid, display size is %zu",
                 __func__, display, m_displays.size());
        return HWC2_ERROR_BAD_DISPLAY;
    }

    if (!DisplayManager::getInstance().getDisplayData(display)->connected)
    {
        HWC_LOGE("%s: the display(%" PRIu64 ") is not connected", __func__, display);
        return HWC2_ERROR_BAD_DISPLAY;
    }

    sp<HWCLayer> layer = m_displays[display]->getLayer(layer_id);
    if (layer == nullptr)
    {
        HWC_LOGE("%s: the display(%" PRIu64 ") does not contain layer(%" PRIu64 ")", __func__, display, layer_id);
        return HWC2_ERROR_BAD_LAYER;
    }

    layer->setDamage(damage);

    return HWC2_ERROR_NONE;
}

/* Layer state functions */
int32_t /*hwc2_error_t*/ HWCMediator::layerStateSetBlendMode(
    hwc2_device_t* /*device*/,
    hwc2_display_t display,
    hwc2_layer_t layer_id,
    int32_t /*hwc2_blend_mode_t*/ mode)
{
    if (display >= DisplayManager::MAX_DISPLAYS)
    {
        HWC_LOGE("%s: this display(%" PRIu64 ") is invalid", __func__, display);
        return HWC2_ERROR_BAD_DISPLAY;
    }

    if (display >= m_displays.size())
    {
        HWC_LOGE("%s: this display(%" PRIu64 ") is invalid, display size is %zu",
                 __func__, display, m_displays.size());
        return HWC2_ERROR_BAD_DISPLAY;
    }

    if (!DisplayManager::getInstance().getDisplayData(display)->connected)
    {
        HWC_LOGE("%s: the display(%" PRIu64 ") is not connected", __func__, display);
        return HWC2_ERROR_BAD_DISPLAY;
    }

    sp<HWCLayer> layer = m_displays[display]->getLayer(layer_id);
    if (layer == nullptr)
    {
        HWC_LOGE("%s: the display(%" PRIu64 ") does not contain layer(%" PRIu64 ")", __func__, display, layer_id);
        return HWC2_ERROR_BAD_LAYER;
    }

    switch (mode)
    {
        case HWC2_BLEND_MODE_NONE:
        case HWC2_BLEND_MODE_PREMULTIPLIED:
        case HWC2_BLEND_MODE_COVERAGE:
            layer->setBlend(mode);
            break;
        default:
            HWC_LOGE("%s: unknown mode(%d)", __func__, mode);
            return HWC2_ERROR_BAD_PARAMETER;
    }
    return HWC2_ERROR_NONE;
}

int32_t /*hwc2_error_t*/ HWCMediator::layerStateSetColor(
    hwc2_device_t* /*device*/,
    hwc2_display_t display_id,
    hwc2_layer_t layer_id,
    hwc_color_t color)
{
    if (display_id >= DisplayManager::MAX_DISPLAYS)
    {
        HWC_LOGE("%s: this display(%" PRIu64 ") is invalid", __func__, display_id);
        return HWC2_ERROR_BAD_DISPLAY;
    }

    if (display_id >= m_displays.size())
    {
        HWC_LOGE("%s: this display(%" PRIu64 ") is invalid, display size is %zu",
                 __func__, display_id, m_displays.size());
        return HWC2_ERROR_BAD_DISPLAY;
    }

    if (!DisplayManager::getInstance().getDisplayData(display_id)->connected)
    {
        HWC_LOGE("%s: the display(%" PRIu64 ") is not connected", __func__, display_id);
        return HWC2_ERROR_BAD_DISPLAY;
    }

    sp<HWCLayer> layer = m_displays[display_id]->getLayer(layer_id);
    if (layer == nullptr)
    {
        HWC_LOGE("%s: the display(%" PRIu64 ") does not contain layer(%" PRIu64 ")", __func__, display_id, layer_id);
        return HWC2_ERROR_BAD_LAYER;
    }

    layer->setLayerColor(color);

    return HWC2_ERROR_NONE;
}

int32_t /*hwc2_error_t*/ HWCMediator::layerStateSetCompositionType(
    hwc2_device_t* /*device*/,
    hwc2_display_t display_id,
    hwc2_layer_t layer_id,
    int32_t /*hwc2_composition_t*/ type)
{
    if (display_id >= DisplayManager::MAX_DISPLAYS)
    {
        HWC_LOGE("%s: this display(%" PRIu64 ") is invalid", __func__, display_id);
        return HWC2_ERROR_BAD_DISPLAY;
    }

    if (display_id >= m_displays.size())
    {
        HWC_LOGE("%s: this display(%" PRIu64 ") is invalid, display size is %zu",
                 __func__, display_id, m_displays.size());
        return HWC2_ERROR_BAD_DISPLAY;
    }

    if (!DisplayManager::getInstance().getDisplayData(display_id)->connected)
    {
        HWC_LOGE("%s: the display(%" PRIu64 ") is not connected", __func__, display_id);
        return HWC2_ERROR_BAD_DISPLAY;
    }

    HWC_LOGV("(%" PRIu64 ") layerStateSetCompositionType() layer id:%" PRIu64 " type:%s", display_id, layer_id, getCompString(type));
    editSetCompFromSfLog().printf(" (%" PRIu64 ":%" PRIu64 ",%s)", display_id, layer_id, getCompString(type));

    auto&& layer = m_displays[display_id]->getLayer(layer_id);
    if (layer == nullptr)
    {
        HWC_LOGE("(%" PRIu64 ") %s: the display does NOT contain layer(%" PRIu64 ")", display_id, __func__, layer_id);
        return HWC2_ERROR_BAD_LAYER;
    }

    switch (type)
    {
        case HWC2_COMPOSITION_CLIENT:
            if (layer->getHwlayerType() != HWC_LAYER_TYPE_INVALID)
                layer->setStateChanged(true);
            layer->setHwlayerType(HWC_LAYER_TYPE_INVALID, __LINE__, HWC_COMP_FILE_HWC);
            break;

        case HWC2_COMPOSITION_DEVICE:
            if (layer->getHwlayerType() != HWC_LAYER_TYPE_UI)
                layer->setStateChanged(true);
            layer->setHwlayerType(HWC_LAYER_TYPE_UI, __LINE__, HWC_COMP_FILE_HWC);
            break;

        case HWC2_COMPOSITION_SIDEBAND:
            abort();

        case HWC2_COMPOSITION_SOLID_COLOR:
            if (layer->getHwlayerType() != HWC_LAYER_TYPE_DIM)
                layer->setStateChanged(true);
            layer->toBeDim();
            layer->setHwlayerType(HWC_LAYER_TYPE_DIM, __LINE__, HWC_COMP_FILE_HWC);
            break;

        case HWC2_COMPOSITION_CURSOR:
            if (layer->getHwlayerType() != HWC_LAYER_TYPE_CURSOR)
                layer->setStateChanged(true);
            layer->setHwlayerType(HWC_LAYER_TYPE_CURSOR, __LINE__, HWC_COMP_FILE_HWC);
            break;
    }
    layer->setSFCompositionType(type, true);

    return HWC2_ERROR_NONE;
}

int32_t /*hwc2_error_t*/ HWCMediator::layerStateSetDataSpace(
    hwc2_device_t* /*device*/,
    hwc2_display_t display,
    hwc2_layer_t layer,
    int32_t /*android_dataspace_t*/ dataspace)
{
    if (!DisplayManager::getInstance().getDisplayData(display)->connected)
    {
        HWC_LOGE("%s: the display(%" PRIu64 ") is not connected", __func__, display);
        return HWC2_ERROR_BAD_DISPLAY;
    }

    sp<HWCLayer> hwc_layer = m_displays[display]->getLayer(layer);
    if (hwc_layer == nullptr)
    {
        HWC_LOGE("%s: the display(%" PRIu64 ") does not contain layer(%" PRIu64 ")", __func__, display, layer);
        return HWC2_ERROR_BAD_LAYER;
    }

    HWC_LOGV("(%" PRIu64 ") layerSetDataSpace() layer id:%" PRIu64 " dataspace:%d", display, layer, dataspace);

    hwc_layer->setDataspace(dataspace);

    return HWC2_ERROR_NONE;
}

int32_t /*hwc2_error_t*/ HWCMediator::layerStateSetDisplayFrame(
    hwc2_device_t* /*device*/,
    hwc2_display_t display,
    hwc2_layer_t layer_id,
    hwc_rect_t frame)
{
    if (display >= DisplayManager::MAX_DISPLAYS)
    {
        HWC_LOGE("%s: this display(%" PRIu64 ") is invalid", __func__, display);
        return HWC2_ERROR_BAD_DISPLAY;
    }

    if (display >= m_displays.size())
    {
        HWC_LOGE("%s: this display(%" PRIu64 ") is invalid, display size is %zu",
                 __func__, display, m_displays.size());
        return HWC2_ERROR_BAD_DISPLAY;
    }

    if (!DisplayManager::getInstance().getDisplayData(display)->connected)
    {
        HWC_LOGE("%s: the display(%" PRIu64 ") is not connected", __func__, display);
        return HWC2_ERROR_BAD_DISPLAY;
    }

    sp<HWCLayer> layer = m_displays[display]->getLayer(layer_id);
    if (layer == nullptr)
    {
        HWC_LOGE("%s: the display(%" PRIu64 ") does not contain layer(%" PRIu64 ")", __func__, display, layer_id);
        return HWC2_ERROR_BAD_LAYER;
    }

    const DisplayData* disp_info = DisplayManager::getInstance().getDisplayData(display,
        m_displays[display]->getActiveConfig());

    if (frame.right > disp_info->width)
    {
        HWC_LOGW("%s: (%" PRIu64 ") layer id:%" PRIu64 " displayframe width(%d) > display device width(%d)",
            __func__, display, layer_id, WIDTH(frame), disp_info->width);
        frame.right = disp_info->width;
    }

    if (frame.bottom > disp_info->height)
    {
        HWC_LOGW("%s: (%" PRIu64 ") layer id:%" PRIu64 " displayframe height(%d) > display device height(%d)",
            __func__, display, layer_id, HEIGHT(frame), disp_info->height);
        frame.bottom = disp_info->height;
    }

    HWC_LOGV("%s: (%" PRIu64 ") layer id:%" PRIu64 " frame[%d,%d,%d,%d] ",
        __func__, display, layer_id, frame.left, frame.top, frame.right, frame.bottom);

    layer->setDisplayFrame(frame);

    return HWC2_ERROR_NONE;
}

int32_t /*hwc2_error_t*/ HWCMediator::layerStateSetPlaneAlpha(
    hwc2_device_t* /*device*/,
    hwc2_display_t display,
    hwc2_layer_t layer_id,
    float alpha)
{
    if (display >= DisplayManager::MAX_DISPLAYS)
    {
        HWC_LOGE("%s: this display(%" PRIu64 ") is invalid", __func__, display);
        return HWC2_ERROR_BAD_DISPLAY;
    }

    if (display >= m_displays.size())
    {
        HWC_LOGE("%s: this display(%" PRIu64 ") is invalid, display size is %zu",
                 __func__, display, m_displays.size());
        return HWC2_ERROR_BAD_DISPLAY;
    }

    if (!DisplayManager::getInstance().getDisplayData(display)->connected)
    {
        HWC_LOGE("%s: the display(%" PRIu64 ") is not connected", __func__, display);
        return HWC2_ERROR_BAD_DISPLAY;
    }

    sp<HWCLayer> layer = m_displays[display]->getLayer(layer_id);
    if (layer == nullptr)
    {
        HWC_LOGE("%s: the display(%" PRIu64 ") does not contain layer(%" PRIu64 ")", __func__, display, layer_id);
        return HWC2_ERROR_BAD_LAYER;
    }

    layer->setPlaneAlpha(alpha);

    return HWC2_ERROR_NONE;
}

int32_t /*hwc2_error_t*/ HWCMediator::layerStateSetSidebandStream(
    hwc2_device_t* /*device*/,
    hwc2_display_t /*display*/,
    hwc2_layer_t /*layer*/,
    const native_handle_t* /*stream*/)
{
    return HWC2_ERROR_NONE;
}

int32_t /*hwc2_error_t*/ HWCMediator::layerStateSetSourceCrop(
    hwc2_device_t* /*device*/,
    hwc2_display_t display,
    hwc2_layer_t layer_id,
    hwc_frect_t crop)
{
    if (display >= DisplayManager::MAX_DISPLAYS)
    {
        HWC_LOGE("%s: this display(%" PRIu64 ") is invalid", __func__, display);
        return HWC2_ERROR_BAD_DISPLAY;
    }

    if (display >= m_displays.size())
    {
        HWC_LOGE("%s: this display(%" PRIu64 ") is invalid, display size is %zu",
                 __func__, display, m_displays.size());
        return HWC2_ERROR_BAD_DISPLAY;
    }

    if (!DisplayManager::getInstance().getDisplayData(display)->connected)
    {
        HWC_LOGE("%s: the display(%" PRIu64 ") is not connected", __func__, display);
        return HWC2_ERROR_BAD_DISPLAY;
    }

    sp<HWCLayer> layer = m_displays[display]->getLayer(layer_id);
    if (layer == nullptr)
    {
        HWC_LOGE("%s: the display(%" PRIu64 ") does not contain layer(%" PRIu64 ")", __func__, display, layer_id);
        return HWC2_ERROR_BAD_LAYER;
    }

    layer->setSourceCrop(crop);
    return HWC2_ERROR_NONE;
}

int32_t /*hwc2_error_t*/ HWCMediator::layerStateSetTransform(
    hwc2_device_t* /*device*/,
    hwc2_display_t display,
    hwc2_layer_t layer_id,
    int32_t /*hwc_transform_t*/ transform)
{
    if (display >= DisplayManager::MAX_DISPLAYS)
    {
        HWC_LOGE("%s: this display(%" PRIu64 ") is invalid", __func__, display);
        return HWC2_ERROR_BAD_DISPLAY;
    }

    if (display >= m_displays.size())
    {
        HWC_LOGE("%s: this display(%" PRIu64 ") is invalid, display size is %zu",
                 __func__, display, m_displays.size());
        return HWC2_ERROR_BAD_DISPLAY;
    }

    if (!DisplayManager::getInstance().getDisplayData(display)->connected)
    {
        HWC_LOGE("%s: the display(%" PRIu64 ") is not connected", __func__, display);
        return HWC2_ERROR_BAD_DISPLAY;
    }

    sp<HWCLayer> layer = m_displays[display]->getLayer(layer_id);
    if (layer == nullptr)
    {
        HWC_LOGE("%s: the display(%" PRIu64 ") does not contain layer(%" PRIu64 ")", __func__, display, layer_id);
        return HWC2_ERROR_BAD_LAYER;
    }

    switch (transform)
    {
        case 0:
        case HWC_TRANSFORM_FLIP_H:
        case HWC_TRANSFORM_FLIP_V:
        case HWC_TRANSFORM_ROT_90:
        case HWC_TRANSFORM_ROT_180:
        case HWC_TRANSFORM_ROT_270:
        case HWC_TRANSFORM_FLIP_H_ROT_90:
        case HWC_TRANSFORM_FLIP_V_ROT_90:
        case Transform::ROT_INVALID:
            layer->setTransform(transform);
            break;

        default:
            HWC_LOGE("%s: unknown transform(%d)", __func__, transform);
            return HWC2_ERROR_BAD_PARAMETER;
    }
    return HWC2_ERROR_NONE;
}

int32_t /*hwc2_error_t*/ HWCMediator::layerStateSetVisibleRegion(
    hwc2_device_t* /*device*/,
    hwc2_display_t display,
    hwc2_layer_t layer_id,
    hwc_region_t visible)
{
    if (display >= DisplayManager::MAX_DISPLAYS)
    {
        HWC_LOGE("%s: this display(%" PRIu64 ") is invalid", __func__, display);
        return HWC2_ERROR_BAD_DISPLAY;
    }

    if (display >= m_displays.size())
    {
        HWC_LOGE("%s: this display(%" PRIu64 ") is invalid, display size is %zu",
                 __func__, display, m_displays.size());
        return HWC2_ERROR_BAD_DISPLAY;
    }

    if (!DisplayManager::getInstance().getDisplayData(display)->connected)
    {
        HWC_LOGE("%s: the display(%" PRIu64 ") is not connected", __func__, display);
        return HWC2_ERROR_BAD_DISPLAY;
    }

    sp<HWCLayer> layer = m_displays[display]->getLayer(layer_id);
    if (layer == nullptr)
    {
        HWC_LOGE("%s: the display(%" PRIu64 ") does not contain layer(%" PRIu64 ")", __func__, display, layer_id);
        return HWC2_ERROR_BAD_LAYER;
    }

    HWC_LOGV("(%" PRIu64 ") layerSetVisibleRegion() layer id:%" PRIu64, display, layer_id);
    layer->setVisibleRegion(visible);
    layer->setVisible(true);

    return HWC2_ERROR_NONE;
}

int32_t /*hwc2_error_t*/ HWCMediator::layerStateSetZOrder(
    hwc2_device_t* /*device*/,
    hwc2_display_t display,
    hwc2_layer_t layer_id,
    uint32_t z)
{
    if (display >= DisplayManager::MAX_DISPLAYS)
    {
        HWC_LOGE("%s: this display(%" PRIu64 ") is invalid", __func__, display);
        return HWC2_ERROR_BAD_DISPLAY;
    }

    if (display >= m_displays.size())
    {
        HWC_LOGE("%s: this display(%" PRIu64 ") is invalid, display size is %zu",
                 __func__, display, m_displays.size());
        return HWC2_ERROR_BAD_DISPLAY;
    }

    if (!DisplayManager::getInstance().getDisplayData(display)->connected)
    {
        HWC_LOGE("%s: the display(%" PRIu64 ") is not connected", __func__, display);
        return HWC2_ERROR_BAD_DISPLAY;
    }

    sp<HWCLayer> layer = m_displays[display]->getLayer(layer_id);
    if (layer == nullptr)
    {
        HWC_LOGE("%s: the display(%" PRIu64 ") does not contain layer(%" PRIu64 ")", __func__, display, layer_id);
        return HWC2_ERROR_BAD_LAYER;
    }

    layer->setZOrder(z);
    return HWC2_ERROR_NONE;
}

int32_t /*hwc2_error_t*/ HWCMediator::layerStateSetPerFrameMetadata(
    hwc2_device_t* /*device*/,
    hwc2_display_t display,
    hwc2_layer_t layer_id,
    uint32_t numElements,
    const int32_t* /*hw2_per_frame_metadata_key_t*/ keys,
    const float* metadata)
{
    if (!DisplayManager::getInstance().getDisplayData(display)->connected)
    {
        HWC_LOGE("%s: the display(%" PRIu64 ") is not connected", __func__, display);
        return HWC2_ERROR_BAD_DISPLAY;
    }

    sp<HWCLayer> layer = m_displays[display]->getLayer(layer_id);
    if (layer == nullptr)
    {
        HWC_LOGE("%s: the display(%" PRIu64 ") does not contain layer(%" PRIu64 ")", __func__, display, layer_id);
        return HWC2_ERROR_BAD_LAYER;
    }

    layer->setPerFrameMetadata(numElements, keys, metadata);
    return HWC2_ERROR_NONE;
}

void HWCMediator::adjustVsyncOffset()
{
    bool need_disable_vsync_offset = false;
    for (auto& display : m_displays)
    {
        need_disable_vsync_offset |= display->needDisableVsyncOffset();
    }
    if (m_vsync_offset_state != !need_disable_vsync_offset) {
        m_vsync_offset_state = !need_disable_vsync_offset;
        setMergeMdInfo2Ged(need_disable_vsync_offset);
    }
}

void HWCMediator::countdowmSkipValiRelatedNumber()
{
    for (size_t i = 0; i < m_displays.size(); ++i)
    {
        if (!m_displays[i]->isConnected())
            continue;

        HWCDispatcher::getInstance().decSessionModeChanged();
    }

    for (size_t i = 0; i < m_displays.size(); ++i)
    {
        if (!m_displays[i]->isValid())
            continue;

        const int32_t disp_id = static_cast<int32_t>(m_displays[i]->getId());

        HWCDispatcher::getInstance().decOvlEnginePowerModeChanged(disp_id);
    }

    HWCMediator::getInstance().decDriverRefreshCount();
}

bool HWCMediator::checkSkipValidate()
{
    DbgLogger logger(DbgLogger::TYPE_HWC_LOG | DbgLogger::TYPE_DUMPSYS, 'D', "SkipV(%zu: ", m_displays.size());
    bool has_valid_display = false;

    if (HWCMediator::getInstance().getDriverRefreshCount() > 0)
    {
        logger.printf("no skip vali(L%d) ", __LINE__);
        return false;
    }

    // If a display just change the power mode and it's session mode change,
    // we should not skip validate
    for (size_t i = 0; i < m_displays.size(); ++i)
    {
        if (!m_displays[i]->isConnected())
            continue;

        if (HWCDispatcher::getInstance().getSessionModeChanged() > 0)
        {
            logger.printf("no skip vali(%zu:L%d) ", i, __LINE__);
            return false;
        }
    }

    for (size_t i = 0; i < m_displays.size(); ++i)
    {
        if (!m_displays[i]->isValid())
        {
            continue;
        }

        const int32_t disp_id = static_cast<int32_t>(m_displays[i]->getId());
        has_valid_display = true;

        if (m_displays[i]->getId() > HWC_DISPLAY_PRIMARY)
        {
            logger.printf("no skip vali(%zu:L%d) ", i, __LINE__);
            return false;
        }

        if (m_displays[i]->isForceGpuCompose())
        {
            logger.printf("no skip vali(%zu:L%d) ", i, __LINE__);
            return false;
        }

        if (m_displays[i]->isConfigChanged())
        {
            logger.printf("no skip vali(%zu:L%d) ", i, __LINE__);
            return false;
        }

        auto&& layers = m_displays[i]->getVisibleLayersSortedByZ();

        for (size_t j = 0; j < layers.size(); ++j)
        {
            if (layers[j]->isStateChanged())
            {
                logger.printf("no skip vali(%zu:L%d) ", i, __LINE__);
                return false;
            }

            if (layers[j]->getHwlayerType() == HWC_LAYER_TYPE_INVALID)
            {
                logger.printf("no skip vali(%zu:L%d) ", i, __LINE__);
                return false;
            }

            if (layers[j]->getPrevIsPQEnhance() != layers[j]->getPrivateHandle().pq_enable)
            {
                logger.printf("no skip vali(%zu:L%d) ", i, __LINE__);
                return false;
            }

            if (layers[j]->isNeedPQ() ||
                layers[j]->isNeedPQ() != layers[j]->getLastAppGamePQ())
            {
                logger.printf("no skip vali(%zu:L%d) ", i, __LINE__);
                return false;
            }
        }

        DispatcherJob* job = HWCDispatcher::getInstance().getExistJob(disp_id);
        if (job == NULL || m_displays[i]->getPrevAvailableInputLayerNum() != job->num_layers)
        {
            logger.printf("no skip vali(%zu:L%d) ", i, __LINE__);
            return false;
        }

        if (m_displays[i]->isVisibleLayerChanged())
        {
            logger.printf("no skip vali(%zu:L%d) ", i, __LINE__);
            return false;
        }

        if (HWCDispatcher::getInstance().getOvlEnginePowerModeChanged(disp_id) > 0)
        {
            logger.printf("no skip vali(%zu:L%d) ", i, __LINE__);
            return false;
        }

        // if there exists secure layers, we do not skip validate.
        if (listSecure(layers))
        {
            logger.printf("no skip vali(%zu:L%d) ", i, __LINE__);
            return false;
        }
    }

    if (has_valid_display)
        logger.printf("do skip vali");

    return (has_valid_display)? true : false;
}

int HWCMediator::getValidDisplayNum()
{
    int count = 0;
    for (size_t i = 0; i<m_displays.size(); ++i)
    {
        if (m_displays[i]->isValid())
            count++;
    }

    return count;
}

void HWCMediator::buildVisibleAndInvisibleLayerForAllDisplay()
{
    // build visible layers
    for (auto& hwc_display : m_displays)
    {
        if (!hwc_display->isValid())
            continue;

        hwc_display->buildVisibleAndInvisibleLayer();
        hwc_display->addUnpresentCount();
    }
}

void HWCMediator::prepareForValidation()
{
    editSetBufFromSfLog().flushOut();
    editSetBufFromSfLog().printf(g_set_buf_from_sf_log_prefix);

    if (editSetCompFromSfLog().getLen() != static_cast<int>(strlen(g_set_comp_from_sf_log_prefix)))
    {
        editSetCompFromSfLog().flushOut();
        editSetCompFromSfLog().printf(g_set_comp_from_sf_log_prefix);
    }

    // validate all displays
    {
        for (auto& hwc_display : m_displays)
        {
            if (!hwc_display->isValid())
                continue;

            HWCDispatcher::getInstance().getJob(static_cast<int32_t>(hwc_display->getId()));
            hwc_display->initPrevCompTypes();
            hwc_display->setJobDisplayOrientation();
            hwc_display->setJobDisplayData();
        }

        HWCDispatcher::getInstance().prepareMirror(m_displays);
        setJobVideoTimeStamp();
    }
}

void HWCMediator::setJobVideoTimeStamp()
{
    for (auto& hwc_display : m_displays)
    {
        if (!hwc_display->isValid())
            continue;

        DispatcherJob* job = HWCDispatcher::getInstance().getExistJob(static_cast<int32_t>(hwc_display->getId()));
        if (NULL != job)
        {
            if (hwc_display->getMirrorSrc() == -1)
            {
                hwc_display->setJobVideoTimeStamp(job);
            }
        }
    }

    for (auto& hwc_display : m_displays)
    {
        if (!hwc_display->isValid())
            continue;

        if (HWC_DISPLAY_VIRTUAL != hwc_display->getId())
            continue;

        DispatcherJob* job = HWCDispatcher::getInstance().getExistJob(static_cast<int32_t>(hwc_display->getId()));
        if (NULL != job && (HWC_MIRROR_SOURCE_INVALID != hwc_display->getMirrorSrc()))
        {
            DispatcherJob* mir_src_job = HWCDispatcher::getInstance().getExistJob(job->disp_mir_id);
            if (NULL != mir_src_job)
                job->timestamp = mir_src_job->timestamp;
        }
    }
}

void HWCMediator::setValiPresentStateOfAllDisplay(const HWC_VALI_PRESENT_STATE& val, const int32_t& line)
{
    for (size_t i = 0; i < m_displays.size(); ++i)
    {
        if (!m_displays[i]->isValid())
        {
            continue;
        }
        m_displays[i]->setValiPresentState(val, line);
    }
}

void calculateMdpDstRoi(sp<HWCLayer> layer, const double& mdp_scale_percentage, const int32_t& /*z_seq*/)
{
    if (layer->getHwlayerType() != HWC_LAYER_TYPE_MM)
        return;

    if (!HWCMediator::getInstance().getOvlDevice(HWC_DISPLAY_PRIMARY)->isDispRpoSupported())
    {
        layer->editMdpDstRoi() = layer->getDisplayFrame();
        return;
    }

    const bool need_mdp_rot = (layer->getLayerCaps() & HWC_MDP_ROT_LAYER);
    const bool need_mdp_rsz = (layer->getLayerCaps() & HWC_MDP_RSZ_LAYER);

    // process rotation
    if (need_mdp_rot)
    {
        layer->editMdpDstRoi().left = 0;
        layer->editMdpDstRoi().top = 0;
        switch (layer->getTransform())
        {
            case HAL_TRANSFORM_ROT_90:
            case HAL_TRANSFORM_ROT_270:
                layer->editMdpDstRoi().right  = HEIGHT(layer->getSourceCrop());
                layer->editMdpDstRoi().bottom = WIDTH(layer->getSourceCrop());
                break;

            default:
                layer->editMdpDstRoi().right  = WIDTH(layer->getSourceCrop());
                layer->editMdpDstRoi().bottom = HEIGHT(layer->getSourceCrop());
                break;
        }
    }
    else
    {
        layer->editMdpDstRoi().left = layer->getSourceCrop().left;
        layer->editMdpDstRoi().top = layer->getSourceCrop().top;
        layer->editMdpDstRoi().right = layer->getSourceCrop().right;
        layer->editMdpDstRoi().bottom = layer->getSourceCrop().bottom;
    }

    if (need_mdp_rsz)
    {
        const int32_t src_w = WIDTH(layer->editMdpDstRoi());
        const int32_t src_h = HEIGHT(layer->editMdpDstRoi());
        const int32_t dst_w = WIDTH(layer->getDisplayFrame());
        const int32_t dst_h = HEIGHT(layer->getDisplayFrame());

        const int32_t max_src_w_of_disp_rsz = HWCMediator::getInstance().getOvlDevice(HWC_DISPLAY_PRIMARY)->getRszMaxWidthInput();

        const bool is_any_shrank = (src_w >= dst_w || src_h >= dst_h);

        if (!is_any_shrank)
        {
            const double max_width_scale_percentage_of_mdp =
                static_cast<double>(max_src_w_of_disp_rsz - src_w) / (dst_w - src_w);

            const double final_mdp_scale_percentage = min(max_width_scale_percentage_of_mdp, mdp_scale_percentage);

            const int& dst_l = layer->getDisplayFrame().left;
            const int& dst_t = layer->getDisplayFrame().top;
            layer->editMdpDstRoi().left  = dst_l;
            layer->editMdpDstRoi().top   = dst_t;
            layer->editMdpDstRoi().right = dst_l +
                src_w * (1 - final_mdp_scale_percentage) + dst_w * final_mdp_scale_percentage;
            layer->editMdpDstRoi().bottom = dst_t +
                src_h * (1 - mdp_scale_percentage) + dst_h * mdp_scale_percentage;
        }
    }
    else
    {
        layer->editMdpDstRoi().right = layer->getDisplayFrame().left + WIDTH(layer->editMdpDstRoi());
        layer->editMdpDstRoi().bottom = layer->getDisplayFrame().top + HEIGHT(layer->editMdpDstRoi());
        layer->editMdpDstRoi().left = layer->getDisplayFrame().left;
        layer->editMdpDstRoi().top = layer->getDisplayFrame().top;
    }
}

void HWCMediator::validate(const hwc2_display_t& cur_disp)
{
    // check if mirror mode exists
    {
        for (auto& hwc_display : m_displays)
        {
            if (!hwc_display->isValid())
                continue;

            if ((cur_disp != -1) && cur_disp != hwc_display->getId())
                continue;

            for (auto& hwc_layer : hwc_display->getVisibleLayersSortedByZ())
                hwc_layer->initLayerCaps();

            hwc_display->validate();
            hwc_display->setOverrideMDPOutputFormatOfLayers();
        }
        updateGlesRangeForAllDisplays();
    }

    {
        // check hrt
        for (auto& hwc_display : m_displays)
        {
            if (!hwc_display->isValid())
                continue;

            for (size_t i = 0; i < hwc_display->getVisibleLayersSortedByZ().size(); ++i)
            {
                calculateMdpDstRoi(
                    hwc_display->getVisibleLayersSortedByZ()[i],
                    Platform::getInstance().m_config.mdp_scale_percentage,
                    i);
            }

            int32_t gles_head = -1, gles_tail = -1;
            hwc_display->getGlesRange(&gles_head, &gles_tail);

            const int32_t disp_id = static_cast<int32_t>(hwc_display->getId());
            DispatcherJob* job = HWCDispatcher::getInstance().getExistJob(disp_id);
            if (job != NULL)
            {
                job->layer_info.hwc_gles_head = gles_head;
                job->layer_info.hwc_gles_tail = gles_tail;
            }
        }
        m_hrt->run(m_displays, false);
        updateGlesRangeForAllDisplays();
    }

    // for limitation of max layer number of blitdev
    {
        for (auto& hwc_display : m_displays)
        {
            if (!hwc_display->isConnected() || hwc_display->getPowerMode() == HWC2_POWER_MODE_OFF)
                continue;

            if (HWCMediator::getInstance().getOvlDevice(hwc_display->getId())->getType() != OVL_DEVICE_TYPE_BLITDEV)
                continue;

            if (hwc_display->getMirrorSrc() != -1)
                continue;

            auto& layers = hwc_display->getVisibleLayersSortedByZ();
            for (size_t i = 0; i < layers.size(); ++i)
            {
                layers[i]->setHwlayerType(HWC_LAYER_TYPE_INVALID, __LINE__, HWC_COMP_FILE_HWC);
            }
            hwc_display->updateGlesRange();
        }
    }
}

bool HWCMediator::isAllDisplayPresentDone()
{
    for(auto& hwc_display : m_displays)
    {
        if (!hwc_display->isValid())
            continue;

        if (hwc_display->getValiPresentState() != HWC_VALI_PRESENT_STATE_PRESENT_DONE)
            return false;
    }
    return true;
}

void HWCMediator::lockRefreshThread(hwc2_display_t display)
{
    if(!isValidated())
    {
        AbortMessager::getInstance().printf("(%" PRIu64 ") HWCMediator m_refresh_vali_lock lock +", display);
        lockRefreshVali();
        AbortMessager::getInstance().printf("(%" PRIu64 ") HWCMediator m_refresh_vali_lock lock -", display);
        setValidated(true);
    }
}

void HWCMediator::unlockRefreshThread(hwc2_display_t display)
{
    if(isValidated())
    {
        if(isAllDisplayPresentDone())
        {
            setValidated(false);
            unlockRefreshVali();
            AbortMessager::getInstance().printf("(%" PRIu64 ") HWCMediator m_refresh_vali_lock unlock", display);
        }
    }
}

void HWCMediator::checkDisplayState()
{
    for (auto& hwc_display : m_displays)
    {
        if (!hwc_display->isValid())
            continue;

        if (HWC_VALI_PRESENT_STATE_PRESENT_DONE != hwc_display->getValiPresentState())
        {
            HWC_LOGE("checkDisplayState fail. (%" PRIu64 ") s:%s",
                hwc_display->getId(),
                getPresentValiStateString(hwc_display->getValiPresentState()));
            AbortMessager::getInstance().abort();
        }
    }
}

