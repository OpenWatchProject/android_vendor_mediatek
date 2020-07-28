#ifndef __CM36652_H__
#define __CM36652_H__
#include "cust_alsps.h"
#include <FreeRTOS.h>
#include <semphr.h>

#define CM36652_SLAVE_ADDR      (0x60)

/*CM36652 als/ps sensor register related macro*/
#define CM36652_REG_CS_CONF         0X00
#define CM36652_REG_ALS_THDH        0X01
#define CM36652_REG_ALS_THDL        0X02
#define CM36652_REG_PS_CONF1_2      0X03
/* reserved 0x4 */
#define CM36652_REG_PS_THD          0X05
#define CM36652_REG_PS_CANC         0X06
#define CM36652_REG_PS_DATA         0X07
#define CM36652_REG_CS_R_DATA       0X08
#define CM36652_REG_CS_G_DATA       0X09
#define CM36652_REG_CS_B_DATA       0X0A
#define CM36652_REG_White_DATA      0X0B
#define CM36652_REG_INT_FLAG        0X0C
#define CM36652_REG_ID              0X0E

/*CM36652 related driver tag macro*/
#define CM36652_SUCCESS                      0
#define CM36652_ERR_I2C                     -1
#define CM36652_ERR_STATUS                  -3
#define CM36652_ERR_SETUP_FAILURE           -4
#define CM36652_ERR_GETGSENSORDATA          -5
#define CM36652_ERR_IDENTIFICATION          -6

typedef enum {
    CM36652_NOTIFY_PROXIMITY_CHANGE = 1,
} CM36652_NOTIFY_TYPE;
struct PS_CALI_DATA_STRUCT {
    int close;
    int far_away;
    int valid;
};
typedef enum {
    CMC_BIT_ALS    = 1,
    CMC_BIT_PS     = 2,
} CMC_BIT;

typedef enum {
    CMC_TRC_ALS_DATA = 0x0001,
    CMC_TRC_PS_DATA = 0x0002,
    CMC_TRC_EINT    = 0x0004,
    CMC_TRC_IOCTL   = 0x0008,
    CMC_TRC_I2C     = 0x0010,
    CMC_TRC_CVT_ALS = 0x0020,
    CMC_TRC_CVT_PS  = 0x0040,
    CMC_TRC_DEBUG   = 0x8000,
} CMC_TRC;

#endif
