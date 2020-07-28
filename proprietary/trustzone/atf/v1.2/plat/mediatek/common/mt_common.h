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
 * MediaTek Inc. (C) 2010. All rights reserved.
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

/******************************************************************************
*
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of MediaTek Inc. (C) 2001
*
*******************************************************************************/

#ifndef MT_COMMON_H
#define MT_COMMON_H

typedef struct mtk_bl_param {
	uint64_t bootarg_loc;
	uint64_t bootarg_size;
	uint64_t bl33_start_addr;
	uint64_t tee_info_addr;
}mtk_bl_param_t;

typedef enum {
	BR_POWER_KEY = 0,
	BR_USB,
	BR_RTC,
	BR_WDT,
	BR_WDT_BY_PASS_PWK,
	BR_TOOL_BY_PASS_PWK,
	BR_2SEC_REBOOT,
	BR_UNKNOWN,
	BR_KERNEL_PANIC,
	BR_WDT_SW,
	BR_WDT_HW
} boot_reason_t;

/* boot reason */
#define BOOT_TAG_BOOT_REASON	0x88610001

struct boot_tag_boot_reason {
	uint32_t boot_reason;
};

struct boot_tag_plat_dbg_info {
	uint32_t info_max;
};

/* log com port information */
#define BOOT_TAG_LOG_COM         0x88610004
struct boot_tag_log_com {
	uint32_t log_port;
	uint32_t log_baudrate;
	uint32_t log_enable;
	uint32_t log_dynamic_switch;
};

/* charger type info */
#define BOOT_TAG_IS_VOLT_UP      0x8861001A
struct boot_tag_is_volt_up {
	uint32_t is_volt_up;
};

#define NO_ABNORMAL_BOOT_TAG 0xffffffff
/* Boot tag */
#define BOOT_TAG_IS_ABNORMAL_BOOT	0x8861001B
struct boot_tag_is_abnormal_boot {
	uint32_t is_abnormal_boot;
};

struct boot_tag_header {
	uint32_t size;
	uint32_t tag;
};

struct boot_tag{
	struct boot_tag_header hdr;
	union {
		struct boot_tag_boot_reason boot_reason;
		struct boot_tag_log_com log_com;
		struct boot_tag_plat_dbg_info plat_dbg_info;
		struct boot_tag_is_volt_up volt_info;
		struct boot_tag_is_abnormal_boot is_abnormal_boot;
	} u;
};

#define boot_tag_next(t)    ((struct boot_tag *)((uint32_t *)(t) + (t)->hdr.size))
#define boot_tag_size(type)	((sizeof(struct boot_tag_header) + sizeof(struct type)) >> 2)

/* bit operations */
#define SET_BIT(_arg_, _bits_)					(uint32_t)((_arg_) |=  (uint32_t)(1 << (_bits_)))
#define CLEAR_BIT(_arg_, _bits_)				((_arg_) &= ~(1 << (_bits_)))
#define TEST_BIT(_arg_, _bits_)					((uint32_t)(_arg_) & (uint32_t)(1 << (_bits_)))
#define EXTRACT_BIT(_arg_, _bits_)				((_arg_ >> (_bits_)) & 1)
#define MASK_BITS(_msb_, _lsb_)					(((1U << ((_msb_) - (_lsb_) + 1)) - 1) << (_lsb_))
#define MASK_FIELD(_field_)						MASK_BITS(_field_##_MSB, _field_##_LSB)
#define EXTRACT_BITS(_arg_, _msb_, _lsb_)		((_arg_ & MASK_BITS(_msb_, _lsb_)) >> (_lsb_))
#define EXTRACT_FIELD(_arg_, _field_)			EXTRACT_BITS(_arg_, _field_##_MSB, _field_##_LSB)
#define INSERT_BIT(_arg_, _bits_, _value_)		((_value_) ? ((_arg_) |= (1 << (_bits_))) : ((_arg_) &= ~(1 << (_bits_))))
#define INSERT_BITS(_arg_, _msb_, _lsb_, _value_) \
				((_arg_) = ((_arg_) & ~MASK_BITS(_msb_, _lsb_)) | (((_value_) << (_lsb_)) & MASK_BITS(_msb_, _lsb_)))
#define INSERT_FIELD(_arg_, _field_, _value_)	INSERT_BITS(_arg_, _field_##_MSB, _field_##_LSB, _value_)

#endif
