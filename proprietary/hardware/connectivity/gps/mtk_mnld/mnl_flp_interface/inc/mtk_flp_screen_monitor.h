#ifndef __MNL_FLP_SCREEN_MONITOR_H__
#define __MNL_FLP_SCREEN_MONITOR_H__

#include <stdint.h>
#include <stdbool.h>
#include <time.h>
#include <sys/socket.h>
#include "mtk_gps_type.h"

#ifdef __cplusplus
extern "C" {
#endif

void flp_monitor_init();
void mtk_flp_get_wake_monitor_state(UINT8 *state);

#ifdef __cplusplus
}
#endif

#endif
