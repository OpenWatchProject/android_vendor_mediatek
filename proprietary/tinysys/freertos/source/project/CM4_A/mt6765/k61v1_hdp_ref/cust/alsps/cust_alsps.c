#include "cust_alsps.h"

struct alsps_hw cust_alsps_hw[] __attribute__((section(".cust_alsps"))) = {
#ifdef CFG_MN66233_SUPPORT
    {
        .name = "epl_sensor",
        .i2c_num    = 0,
        .i2c_addr = {0x49, 0},
        .polling_mode_ps = 0,
        .polling_mode_als = 1,
        .als_level  = {0, 328, 861, 1377, 3125, 7721, 7767, 12621, 23062, 28430, 33274, 47116, 57694, 57694, 65535},
        .als_value  = {0, 200, 320, 502, 1004, 2005, 3058, 5005, 8008, 10010, 12000, 16000, 20000, 20000, 20000, 20000},
        .ps_threshold_high = 26,
        .ps_threshold_low = 21,
        .eint_num = 6,
        .wait_time = 0x0,
        //als
        .als_lpc = 275,
        .als_report_type = 0x2,
        .als_persist = 0x00,
        .als_filter = 0x4,
        .als_gain = 0x3,
        .als_intt = 0xa,
        .als_dyn_intt_0 = 0xa,
        .als_dyn_gain_0 = 0x1,
        .als_dyn_intt_1 = 0xa,
        .als_dyn_gain_1 = 0x3,
        //ps
        .ps_intt = 8,
        .ps_gain = 0x03,
        .ps_filter = 0x04,
        .ps_persist = 0x00,
        .ps_ir_drive = 0x01,
        .ps_dynk_low = 345,
        .ps_dynk_high = 910,
        .ps_max_ct = 10000,
    },
#endif
};
