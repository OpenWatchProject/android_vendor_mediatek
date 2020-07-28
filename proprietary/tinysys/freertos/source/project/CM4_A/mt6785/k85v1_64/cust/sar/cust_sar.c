#include "cust_sar.h"

struct sar_hw cust_sar_hw[] __attribute__((section(".cust_sar"))) = {

#ifdef CFG_ADUX1050SAR_SUPPORT
    {
        .name = "adux1050",
        .i2c_num = 0,
        .direction = 0,
        .i2c_addr = {0x2C, 0},
        .reg_config = {
            0x216, 0xb78, 0xcc, 0x0, 0x0821, /* CTRL Regs */
            0x3918, 0x15e, 0x1858, 0x9A, 0xec00, /* Stage 0 Regs */
            0x3902, 0x15e, 0x1858, 0x11, 0xFB00,/* Stage 1 Regs */
            0x1800, 0x0, 0x0, 0x0, 0x0,/* Stage 2 Regs */
            0x1800, 0x0, 0x0, 0x0, 0x0,/* Stage 3 Regs */
            0x7d02, 0x8000, 0x8000, 0x8000
        },/* BS  Regs */
    },
#endif
};
