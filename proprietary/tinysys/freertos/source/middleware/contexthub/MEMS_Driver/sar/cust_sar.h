#ifndef __CUST_SAR_H__
#define __CUST_SAR_H__

#define S_CUST_I2C_ADDR_NUM 2

struct sar_hw {
    const char *name;
    int i2c_num;    /*!< the i2c bus used by the chip */
    int direction;  /*!< the direction of the chip */
    unsigned char   i2c_addr[S_CUST_I2C_ADDR_NUM];
};
struct sar_hw* get_cust_sar(const char *name);
#endif

