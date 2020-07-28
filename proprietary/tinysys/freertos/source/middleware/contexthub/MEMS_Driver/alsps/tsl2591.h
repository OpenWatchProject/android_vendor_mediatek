#ifndef __CM36558_H__
#define __CM36558_H__
#include "cust_alsps.h"
#include <FreeRTOS.h>
#include <semphr.h>

/* Default LUX and Color coefficients */
#define D_Factor                 1040
#define B_Coef                   1846
#define C_Coef                   749
#define D_Coef                   1374

#define MIN_ALS_VALUE            2
#define GAIN_SWITCH_LEVEL        100
#define MAX_ALS_VALUE            0xffff

/* Register definitions */
#define TSL2591_ENABLE_REG       0x80
#define TSL2591_CONTROL_REG      0x81
#define TSL2591_AILTL_REG        0x84
#define TSL2591_AILTH_REG        0x85
#define TSL2591_AIHTL_REG        0x86
#define TSL2591_AIHTH_REG        0x87
#define TSL2591_NPAILTL_REG      0x88
#define TSL2591_NPAILTH_REG      0x89
#define TSL2591_NPAIHTL_REG      0x8A
#define TSL2591_NPAIHTH_REG      0x8B
#define TSL2591_PERSIST_REG      0x8C
#define TSL2591_PID_REG          0x91
#define TSL2591_ID_REG           0x92
#define TSL2591_STATUS_REG       0x93
#define TSL2591_CH0L_REG         0x94
#define TSL2591_CH0H_REG         0x95
#define TSL2591_CH1L_REG         0x96
#define TSL2591_CH1H_REG         0x97

/* Special function definitions */
#define TSL2591_CMD_INT_TRI      0xE4
#define TSL2591_CMD_ALSINT_CLR   0xE6
#define TSL2591_CMD_ALLINT_CLR   0xE7
#define TSL2591_CMD_NPALSINT_CLR 0xEA


/* ENABLE_REG @0x80 */
#define PON                      (0x01 << 0)
#define AEN                      (0x01 << 1)
#define AIEN                     (0x01 << 4)
#define SAI                      (0x01 << 6)
#define NPIEN                    (0x01 << 7)

/* CONTROL_REG @0x81 */
#define ATIME_SHIFT              0
#define ATIME_MASK               (0x07 << ATIME_SHIFT)
#define ATIME_100MS              (0x00 << ATIME_SHIFT)
#define ATIME_200MS              (0x01 << ATIME_SHIFT)
#define ATIME_300MS              (0x02 << ATIME_SHIFT)
#define ATIME_400MS              (0x03 << ATIME_SHIFT)
#define ATIME_500MS              (0x04 << ATIME_SHIFT)
#define ATIME_600MS              (0x05 << ATIME_SHIFT)
#define ATIME_2_73MS             (0x06 << ATIME_SHIFT)
#define AGAIN_SHIFT              4
#define AGAIN_MASK               (0x03 << AGAIN_SHIFT)
#define AGAIN_LOW                (0x00 << AGAIN_SHIFT)
#define AGAIN_MED                (0x01 << AGAIN_SHIFT)
#define AGAIN_HIGH               (0x02 << AGAIN_SHIFT)
#define AGAIN_MAX                (0x03 << AGAIN_SHIFT)
#define SRESET                   (0x01 << 7)

/* PERSIST_REG @0x0C */
#define APERS_SHIFT              0
#define APERS_MASK               (0x0F << APERS_SHIFT)

/* ID_REG @0x92 */
#define DEV_ID                   0x50
#define ID_MASK                  0xFF

/* STATUS_REG @0x13 */
#define AVALID                   (0x01 << 0)
#define AINT                     (0x01 << 4)
#define NPINTR                   (0x01 << 5)

#endif

