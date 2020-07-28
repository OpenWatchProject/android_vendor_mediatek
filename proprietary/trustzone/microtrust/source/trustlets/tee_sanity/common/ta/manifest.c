/* TA Configuration file */
#include <manifest.h>

// clang-format off

TA_CONFIG_BEGIN

uuid :  { 0x05150000, 0x0000, 0x0000, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } },
log_tag : "tee_sanity_ta",
ipc_buf_size: 0x10000,
rpmb_size : 1024,

TA_CONFIG_END
