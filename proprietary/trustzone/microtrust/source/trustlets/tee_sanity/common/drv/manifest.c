/* Drv Configuration file */
#include <manifest.h>
#define DRV_CORE_API_ID 0x90305

DRIVER_CONFIG_BEGIN

driver_id : DRV_CORE_API_ID,
uuid : {0x05160000, 0x0000, 0x0000, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}},
log_tag : "tee_sanity_drv",

DRIVER_CONFIG_END
