
#ifndef __BH1726_H__
#define __BH1726_H__
#include "cust_alsps.h"
#include <FreeRTOS.h>
#include <semphr.h>

#define BH1726_SLAVE_ADDR      (0x39)    /*another address=0X29 need to re-layout(hw)*/

/*BH1726 als sensor register related macro*/
#define BH1726_REG_CTRL        0X00 + 0X80
#define BH1726_REG_TIMING      0X01 + 0X80
#define BH1726_REG_INT_CONF    0X02 + 0X80
#define BH1726_REG_THDL_L      0X03 + 0X80
#define BH1726_REG_THDL_H      0X04 + 0X80
#define BH1726_REG_THDH_L      0X05 + 0X80
#define BH1726_REG_THDH_H      0X06 + 0X80
#define BH1726_REG_GAIN        0X07 + 0X80
#define BH1726_REG_ID          0X12 + 0X80
#define BH1726_REG_DATA0_L     0X14 + 0X80
#define BH1726_REG_DATA0_H     0X15 + 0X80
#define BH1726_REG_DATA1_L     0X16 + 0X80
#define BH1726_REG_DATA1_H     0X17 + 0X80
#define BH1726_REG_WAIT        0X18 + 0X80

/*BH1726 chip ID*/
#define BH1726_CHIP_ID         0X72

/*HB1726 als setting command*/
#define BH1726_SW_RESET             0XE4
#define BH1726_POWER_ON             0X01
#define BH1726_POWER_OFF            0X00
#define BH1726_INT_ENABLE           0X10
#define BH1726_INT_LATCH            0X20
#define BH1726_ADC_ENABLE           0X02
#define BH1726_DATA_VALID           0X10
#define BH1726_POWER_OFF            0X00
#define BH1726_DATA_GAIN_1x         0X00
#define BH1726_DATA_GAIN_2x         0X01
#define BH1726_DATA_GAIN_64x        0X10
#define BH1726_DATA_GAIN_128x       0X11

#define BH1726_WAIT_EN              0X01

/*BH1726 related driver tag macro*/
#define BH1726_SUCCESS                      0
#define BH1726_ERR_I2C                     -1
#define BH1726_ERR_STATUS                  -3
#define BH1726_ERR_SETUP_FAILURE           -4
#define BH1726_ERR_GETGSENSORDATA          -5
#define BH1726_ERR_IDENTIFICATION          -6

#endif


