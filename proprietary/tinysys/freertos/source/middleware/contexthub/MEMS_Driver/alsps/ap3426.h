
#ifndef __AP3426_H__
#define __AP3426_H__
#include "cust_alsps.h"
#include <FreeRTOS.h>
#include <semphr.h>

/*ap3426 als/ps sensor register related macro*/
#define AP3426_FX                           100000
#define AP3426_ALS_MAX_DATA                 65535
#define AP3426_ALS_AUTO_GAIN_HTHRES         ((3*AP3426_ALS_MAX_DATA)/4)
#define AP3426_ALS_AUTO_GAIN_LTHRES         ((2*AP3426_ALS_MAX_DATA)/16)

#define AP3426_ENABLE                       0x00
#define AP3426_INT_STATUS                   0x01
#define AP3426_INT_CTL                      0x02
#define AP3426_ADATA_L                      0x0C
#define AP3426_ADATA_H                      0x0D
#define AP3426_PDATA_L                      0x0E
#define AP3426_PDATA_H                      0x0F
#define AP3426_ALS_GAIN                     0x10
#define AP3426_INT_ALS_LOW_THD_LOW          0x1A
#define AP3426_INT_ALS_LOW_THD_HIGH         0x1B
#define AP3426_INT_ALS_HIGH_THD_LOW         0x1C
#define AP3426_INT_ALS_HIGH_THD_HIGH        0x1D
#define AP3426_INT_PS_LOW_THD_LOW           0x2A
#define AP3426_INT_PS_LOW_THD_HIGH          0x2B
#define AP3426_INT_PS_HIGH_THD_LOW          0x2C
#define AP3426_INT_PS_HIGH_THD_HIGH         0x2D

#define AP3426_ALS_GAIN_D                   0x30
#define AP3426_ALS_GAIN_C                   0x20
#define AP3426_ALS_GAIN_B                   0x10
#define AP3426_ALS_GAIN_A                   0x00
#define AP3426_ALS_GAIN_D_RATION            (0.0078  * AP3426_FX)
#define AP3426_ALS_GAIN_C_RATION            (0.03125 * AP3426_FX)
#define AP3426_ALS_GAIN_B_RATION            (0.125   * AP3426_FX)
#define AP3426_ALS_GAIN_A_RATION            (0.5     * AP3426_FX)

#endif


