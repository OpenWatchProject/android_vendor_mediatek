/* TA Configuration file */
#include <manifest.h>
#define DRV_CORE_API_ID 0x93419
// clang-format off

DRIVER_CONFIG_BEGIN

driver_id : DRV_CORE_API_ID,
uuid : {0x08050000, 0x0000, 0x0000, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x34, 0x19}},
log_tag : "drv_pmem",

DRIVER_CONFIG_END
