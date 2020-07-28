/* TA Configuration file */
#include <manifest.h>
#define DRV_CORE_API_ID 0x90206
// clang-format off

DRIVER_CONFIG_BEGIN

driver_id : DRV_CORE_API_ID,
uuid : {0x08070000, 0x0000, 0x0000, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x82, 0x70}},
log_tag : "vp9_drv_decoder",

DRIVER_CONFIG_END
