#ifndef __RP_AGPS_LOG_H__
#define __RP_AGPS_LOG_H__
/*****************************************************************************
 * Include
 *****************************************************************************/
#include <Log.h>

/*****************************************************************************
 * External Functions
 *****************************************************************************/
extern void agpsLogPrint(int type, const char* tag, const char *fmt, ...);

/*****************************************************************************
 * Defines
 *****************************************************************************/
#define AGPS_LOGD(...) agpsLogPrint(0, "[agps][n][RILP]", __VA_ARGS__);
#define AGPS_LOGW(...) agpsLogPrint(0, "[agps] WARNING: [RILP]", __VA_ARGS__);
#define AGPS_LOGE(...) agpsLogPrint(1, "[agps] ERR: [RILP]", __VA_ARGS__);

#endif /* __RP_AGPS_LOG_H__ */