/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2019. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */


/* Pre-defined definition */
typedef unsigned int 	u32;
typedef unsigned short	u16;
typedef unsigned char	u8;

typedef signed int 		s32;
typedef signed short	s16;
typedef signed char		s8;

#if 0
#define TPD_KEY_COUNT   4
#define key_1           60,850              //auto define
#define key_2           180,850
#define key_3           300,850
#define key_4           420,850

#define TPD_KEYS        {KEY_BACK, KEY_HOME, KEY_MENU, KEY_SEARCH}
#define TPD_KEYS_DIM    {{key_1,50,30},{key_2,50,30},{key_3,50,30},{key_4,50,30}}
#endif
/***************************PART1:ON/OFF define*******************************/

#define GTP_DRIVER_SEND_CFG   1	// send config to TP while initializing (for no config built in TP's flash)
#define GTP_CUSTOM_CFG        1	// customize resolution & interrupt trigger mode

#define GTP_CHANGE_X2Y        0	// exchange xy
#define GTP_WARP_X_ON         0
#define GTP_WARP_Y_ON         0

#define GTP_GESTURE_WAKEUP    0	// gesture wakeup module
#define KEY_GESTURE           KEY_F24	// customize gesture-key

#define GTP_HOTKNOT           1	// hotknot module
#define HOTKNOT_TYPE          0	// 0: hotknot in flash; 1: hotknot in driver
#define HOTKNOT_BLOCK_RW      0

#define GTP_PROXIMITY         0	// proximity module (function as the p-sensor),only supports MTK platform

#define GTP_HAVE_TOUCH_KEY    0
#define GTP_WITH_STYLUS       0
#define GTP_HAVE_STYLUS_KEY   0

#define GTP_AUTO_UPDATE       1	// auto update FW to TP FLASH while initializing
#define GTP_HEADER_FW_UPDATE  1	// firmware in gt1x_firmware.h

#define GTP_POWER_CTRL_SLEEP  1       //turn off power on suspend
#define GTP_ICS_SLOT_REPORT   0
#define GTP_CREATE_WR_NODE    1	// create the interface to support gtp_tools

#define GTP_ESD_PROTECT       0	// esd-protection module (with a cycle of 2 seconds)
#define GTP_CHARGER_SWITCH    0	// charger plugin & plugout detect

#define GTP_DEBUG_ON          1	// enable log printed by GTP_DEBUG(...)
#define GTP_DEBUG_ARRAY_ON    1
#define GTP_DEBUG_FUNC_ON     1

extern int tpd_em_log;
#define CFG_GROUP_LEN(p_cfg_grp)  (sizeof(p_cfg_grp) / sizeof(p_cfg_grp[0]))

/***************************PART2:TODO define**********************************/
/*TODO: puts the config info corresponded to your TP here, the following is just
          a sample config, send this config should cause the chip cannot work normally*/

/* TODO define your config for Sensor_ID == 0 here, if needed */

#define GTP_MAX_TOUCH    5


/****************************PART3:OTHER define*********************************/
#define GTP_DRIVER_VERSION          "V1.0<2014/09/28>"
#define GTP_I2C_NAME                "Goodix-TS"
#define GT1X_DEBUG_PROC_FILE        "gt1x_debug"
#define GTP_POLL_TIME               10
#define GTP_ADDR_LENGTH             2
#define GTP_CONFIG_MIN_LENGTH       186
#define GTP_CONFIG_MAX_LENGTH       240
#define GTP_MAX_I2C_XFER_LEN        250
#define SWITCH_OFF                  0
#define SWITCH_ON                   1

#define GTP_REG_MATRIX_DRVNUM           0x8069
#define GTP_REG_MATRIX_SENNUM           0x806A
#define GTP_REG_RQST                    0x8044
#define GTP_REG_BAK_REF                 0x90EC
#define GTP_REG_MAIN_CLK                0x8020
#define GTP_REG_HAVE_KEY                0x8057
#define GTP_REG_HN_STATE                0x8800

#define GTP_REG_WAKEUP_GESTURE         0x814C
#define GTP_REG_WAKEUP_GESTURE_DETAIL  0xA2A0	// need change

/* request type */
#define GTP_RQST_CONFIG                 0x01
#define GTP_RQST_BAK_REF                0x02
#define GTP_RQST_RESET                  0x03
#define GTP_RQST_MAIN_CLOCK             0x04
#define GTP_RQST_HOTKNOT_CODE           0x20
#define GTP_RQST_RESPONDED              0x00
#define GTP_RQST_IDLE                   0xFF

#define HN_DEVICE_PAIRED                0x80
#define HN_MASTER_DEPARTED              0x40
#define HN_SLAVE_DEPARTED               0x20
#define HN_MASTER_SEND                  0x10
#define HN_SLAVE_RECEIVED               0x08

/*Register define */
#define GTP_READ_COOR_ADDR          0x814E
#define GTP_REG_CMD                 0x8040
#define GTP_REG_SENSOR_ID           0x814A
#define GTP_REG_CONFIG_DATA         0x8050
#define GTP_REG_CONFIG_RESOLUTION   0x8051
#define GTP_REG_CONFIG_TRIGGER      0x8056
#define GTP_REG_CONFIG_CHECKSUM     0x813C
#define GTP_REG_CONFIG_UPDATE       0x813E
#define GTP_REG_VERSION             0x8140
#define GTP_REG_HW_INFO             0x4220
#define GTP_REG_REFRESH_RATE	    0x8056
#define GTP_REG_ESD_CHECK           0x8043
#define GTP_REG_FLASH_PASSBY        0x8006
#define GTP_REG_HN_PAIRED           0x81AA
#define GTP_REG_HN_MODE             0x81A8
#define GTP_REG_MODULE_SWITCH3      0x8058

#define set_reg_bit(reg,index,val)	((reg)^=(!(val)<<(index)))

/* cmd define */
#define GTP_CMD_SLEEP               0x05
#define GTP_CMD_CHARGER_ON          0x06
#define GTP_CMD_CHARGER_OFF         0x07
#define GTP_CMD_GESTURE_WAKEUP      0x08
#define GTP_CMD_CLEAR_CFG           0x10
#define GTP_CMD_ESD                 0xAA
#define GTP_CMD_HN_TRANSFER         0x22
#define GTP_CMD_HN_EXIT_SLAVE       0x28

/* define offset in the config*/
#define RESOLUTION_LOC              (GTP_REG_CONFIG_RESOLUTION - GTP_REG_CONFIG_DATA)
#define TRIGGER_LOC                 (GTP_REG_CONFIG_TRIGGER - GTP_REG_CONFIG_DATA)
#define MODULE_SWITCH3_LOC	     	(GTP_REG_MODULE_SWITCH3 - GTP_REG_CONFIG_DATA)

#define GTP_I2C_ADDRESS       		0xBA

#pragma pack(1)
struct gt1x_version_info {
	u8 product_id[5];
	u32 patch_id;
	u32 mask_id;
	u8 sensor_id;
	u8 match_opt;
};
#pragma pack()

typedef enum {
	DOZE_DISABLED = 0,
	DOZE_ENABLED = 1,
	DOZE_WAKEUP = 2,
} DOZE_T;

typedef enum {
	CHIP_TYPE_GT1X = 0,
	CHIP_TYPE_GT2X = 1,
	CHIP_TYPE_NONE = 0xFF
} CHIP_TYPE_T;

#define _ERROR(e)      ((0x01 << e) | (0x01 << (sizeof(s32) * 8 - 1)))
#define ERROR          _ERROR(1)	//for common use
//system relevant
#define ERROR_IIC      _ERROR(2)	//IIC communication error.
#define ERROR_MEM      _ERROR(3)	//memory error.

//system irrelevant
#define ERROR_HN_VER   _ERROR(10)	//HotKnot version error.
#define ERROR_CHECK    _ERROR(11)	//Compare src and dst error.
#define ERROR_RETRY    _ERROR(12)	//Too many retries.
#define ERROR_PATH     _ERROR(13)	//Mount path error
#define ERROR_FW       _ERROR(14)
#define ERROR_FILE     _ERROR(15)
#define ERROR_VALUE    _ERROR(16)	//Illegal value of variables

/* bit operation */
#define SET_BIT(data, flag)	((data) |= (flag))
#define CLR_BIT(data, flag)	((data) &= ~(flag))
#define CHK_BIT(data, flag)	((data) & (flag))

/* touch states */
#define BIT_TOUCH			0x01
#define BIT_TOUCH_KEY		0x02
#define BIT_STYLUS			0x04
#define BIT_STYLUS_KEY		0x08
#define BIT_HOVER			0x10


#define GTP_DMA_MAX_TRANSACTION_LENGTH  255   // for DMA mode
#define GTP_DMA_MAX_I2C_TRANSFER_SIZE   (GTP_DMA_MAX_TRANSACTION_LENGTH - GTP_ADDR_LENGTH)
#define MAX_TRANSACTION_LENGTH        8
#define MAX_I2C_TRANSFER_SIZE         (MAX_TRANSACTION_LENGTH - GTP_ADDR_LENGTH)
#define TPD_MAX_RESET_COUNT           3

#define MAX_DMA_TRANSACTION_LENGTH        255
#define MAX_I2C_DMA_TRANSFER_SIZE         (MAX_DMA_TRANSACTION_LENGTH - GTP_ADDR_LENGTH)

extern int gt1xx_driver_init(void);
extern uint8_t *DMA_BASE_VA;
