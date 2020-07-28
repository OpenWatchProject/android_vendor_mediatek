#define DEBUG_LOG_TAG "PLAT"


#include <hardware/hwcomposer.h>

#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>

#include "utils/debug.h"
#include "utils/tools.h"

#include "platform.h"

#include "DpBlitStream.h"

#include "hwc2.h"
extern unsigned int mapDpOrientation(const uint32_t transform);

// ---------------------------------------------------------------------------

ANDROID_SINGLETON_STATIC_INSTANCE(Platform);

Platform::Platform()
{
    m_config.platform = PLATFORM_MT6755;

    m_config.compose_level = COMPOSE_ENABLE_ALL;

    m_config.mirror_state = MIRROR_ENABLED;

    m_config.rdma_roi_update = 1;

    m_config.use_async_bliter_ultra = true;

    m_config.enable_rgbx_scaling = false;
}

bool Platform::isUILayerValid(const sp<HWCLayer>& layer, int32_t* line)
{
    return PlatformCommon::isUILayerValid(layer, line);
}


bool Platform::isMMLayerValid(const sp<HWCLayer>& layer, int32_t* line)
{
    return PlatformCommon::isMMLayerValid(layer, line);
}
