/* TA Configuration file */
#include <manifest.h>
#define DRV_CORE_API_ID 0x90727
// clang-format off

DRIVER_CONFIG_BEGIN

driver_id : DRV_CORE_API_ID,
uuid : {0x020f0000, 0x0000, 0x0000, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}},
log_tag : "drv_secmem",

DRIVER_CONFIG_END
