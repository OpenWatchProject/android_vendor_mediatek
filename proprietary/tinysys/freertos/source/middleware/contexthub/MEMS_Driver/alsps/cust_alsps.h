#ifndef __CUST_ALSPS_H__
#define __CUST_ALSPS_H__

#define C_CUST_ALS_LEVEL    16
#define C_CUST_I2C_ADDR_NUM 2

#define MAX_THRESHOLD_HIGH 0xffff
#define MIN_THRESHOLD_LOW 0x0

struct alsps_hw {
    const char *name;
    int i2c_num;                                    /*!< the i2c bus used by ALS/PS */
    int polling_mode_ps;                               /*!< 1: polling mode ; 0:interrupt mode*/
    int polling_mode_als;                               /*!< 1: polling mode ; 0:interrupt mode*/
    unsigned char   i2c_addr[C_CUST_I2C_ADDR_NUM];  /*!< i2c address list, some chip will have multiple address */
    unsigned int    als_level[C_CUST_ALS_LEVEL -
                              1]; /*!< (C_CUST_ALS_LEVEL-1) levels divides all range into C_CUST_ALS_LEVEL levels*/
    unsigned int    als_value[C_CUST_ALS_LEVEL];    /*!< the value reported in each level */
    unsigned int    ps_threshold_high;
    unsigned int    ps_threshold_low;
    unsigned int    als_threshold_high;
    unsigned int    als_threshold_low;
    unsigned int    eint_num;
#ifdef CFG_MN66233_SUPPORT
    unsigned int wait_time;
    /*als setting*/
    unsigned int als_intt;
    unsigned int als_gain;
    unsigned int als_filter;
    unsigned int als_persist;
    unsigned int als_report_type;
    unsigned int als_lpc;
    unsigned int als_dyn_intt_0;
    unsigned int als_dyn_gain_0;
    unsigned int als_dyn_intt_1;
    unsigned int als_dyn_gain_1;
    unsigned int als_offset_gain;
    unsigned int als_scale_gain;
    /*ps setting*/
    unsigned int ps_intt;
    unsigned int ps_gain;
    unsigned int ps_filter;
    unsigned int ps_persist;
    unsigned int ps_ir_drive;
    unsigned int ps_dynk_low;
    unsigned int ps_dynk_high;
    unsigned int ps_max_ct;
#endif
};
struct alsps_hw *get_cust_alsps(const char *name);
#endif
