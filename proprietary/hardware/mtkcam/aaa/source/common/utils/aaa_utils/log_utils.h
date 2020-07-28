#pragma once

#include "mtkcam/utils/std/Log.h"

static int isDebug = 0;
#if 1
#define logD(fmt, arg...) do {if (isDebug) CAM_LOGI(fmt, ##arg);} while (0)
#else
#define logD(fmt, arg...) do {if (isDebug) CAM_LOGI(fmt, ##arg); else CAM_LOGD(fmt, ##arg);} while (0)
#endif
#define logI(fmt, arg...) do {CAM_LOGI(fmt, ##arg);} while (0)
#define logE(fmt, arg...) do {CAM_LOGE(fmt, ##arg);} while (0)

