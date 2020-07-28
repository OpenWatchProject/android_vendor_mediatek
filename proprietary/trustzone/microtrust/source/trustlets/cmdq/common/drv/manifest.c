/* TA Configuration file */
#include <manifest.h>
#define DRV_CORE_API_ID 0x90400
// clang-format off

DRIVER_CONFIG_BEGIN

driver_id : DRV_CORE_API_ID,
uuid : {0x020b0000, 0x0000, 0x0000, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}},
log_tag : "drv_cmdq",

DRIVER_CONFIG_END
