#ifndef __EM30918_H__
#define __EM30918_H__
#include "cust_alsps.h"
#include <FreeRTOS.h>
#include <semphr.h>

#define EM_TURN_ON_CALI

//#define EM_TUNE0
#define EM_LT_N_CT						35
#define EM_HT_N_CT						58
#define	EM_MAX_MIN_DIFF				60
#define	EM_H_PS						    300
#define	EM_H_LT						    5//80
#define	EM_H_HT						    10//100

#define EM_ALSCTRL_REG                 0x02

/*ALSPS REGS*/
#define EM_PDT_ID_REG                  0x00
#define EM_STATE_REG                   0x01
#define EM_INT_CTRL                    0x02
#define EM_THDL_PS_REG                0x03
#define EM_THDH_PS_REG                0x04
//#define EM_THDL1_PS_REG                0x03
//#define EM_THDL2_PS_REG                0x04
//#define EM_THDH1_PS_REG                0x05
//#define EM_THDH2_PS_REG                0x06
#define EM_THDH1_ALS_REG               0x07
//#define EM_THDH2_ALS_REG               0x08
//#define EM_THDL1_ALS_REG               0x09
//#define EM_THDL2_ALS_REG               0x0A
#define EM_OSLCAL_REG                  0x0B
#define EM_LEDCTR_REG                  0X0C
#define EM_PSCTR_REG                   0X0D
//#define EM_PS_GAIN_CTRL                0X0E
#define EM_ALS_CTRL_REG		             0X12
#define EM_SW_RESET_REG                0x0e       //0x14
#define EM_PS_OFFSET_REG		           0X24
#define EM_ALS_OFFSET_REG		           0X27
#define EM_FLAG_REG                    0x02
#define EM_DATA1_PS_REG                0x08      //0x20
//#define EM_DATA2_PS_REG                0x21
#define EM_DATA1_ALS_REG               0x09      //0x30
#define EM_DATA2_ALS_REG               0x0a      //0x31
#define EM_FLG_PSINT_MASK              0x80
#define EM_STATE_EN_ALS_MASK			     0x06      //0x01
#define EM_STATE_EN_PS_MASK			       0xb8      //0x80
#define EM_DATA_PS_OFF_REG			       0x20      //0x20

#endif
