#ifndef __CUST_ACCGYRO_H__
#define __CUST_ACCGYRO_H__

#define G_CUST_I2C_ADDR_NUM 2

struct accGyro_hw {
    const char *name;
    int i2c_num;    /*!< the i2c bus used by the chip */
    int direction;  /*!< the direction of the chip */
    unsigned char   i2c_addr[G_CUST_I2C_ADDR_NUM];
    unsigned int    eint_num;
};
struct accGyro_hw* get_cust_accGyro(const char *name);
#endif

