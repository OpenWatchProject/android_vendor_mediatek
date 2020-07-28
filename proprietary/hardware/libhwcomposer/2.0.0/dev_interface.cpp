#define DEBUG_LOG_TAG "OvlDev"
#define ATRACE_TAG ATRACE_TAG_GRAPHICS

#include "dev_interface.h"

#include <graphics_mtk_defs.h>

#include "utils/debug.h"
#include "utils/tools.h"
#include "hwc2.h"

#ifndef MTK_USE_DRM_DEVICE
#include "legacy/hwdev.h"
#endif

SessionInfo::SessionInfo()
    : maxLayerNum(0)
    , isHwVsyncAvailable(0)
    , displayType(HWC_DISP_IF_TYPE_DBI)
    , displayWidth(0)
    , displayHeight(0)
    , displayFormat(0)
    , displayMode(HWC_DISP_IF_MODE_VIDEO)
    , vsyncFPS(0)
    , physicalWidth(0)
    , physicalHeight(0)
    , physicalWidthUm(0)
    , physicalHeightUm(0)
    , density(0)
    , isConnected(0)
    , isHDCPSupported(0)
    , is3DSupport(0)
{
}

IOverlayDevice* getHwDevice()
{
#ifndef MTK_USE_DRM_DEVICE
    return &DispDevice::getInstance();
#endif
}
