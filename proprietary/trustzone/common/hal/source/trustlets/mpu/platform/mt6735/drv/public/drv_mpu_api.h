/*
 * Copyright (c) 2014 - 2018 MediaTek Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files
 * (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef __MT_DRV_MPU_API_H__
#define __MT_DRV_MPU_API_H__

#include "mt_emi_mpu.h"

#define msee_emi_region_info_t		emi_region_info_t
#define msee_set_emi_mpu_protection	emi_mpu_set_protection

#define MSEE_SET_ACCESS_PERMISSION	SET_ACCESS_PERMISSION

#define MSEE_SHARED_SECURE_MEMORY_MPU_REGION_ID	SHARED_SECURE_MEMORY_MPU_REGION_ID
#define MSEE_TUI_MEMORY_MPU_REGION_ID		TUI_MEMORY_MPU_REGION_ID

#define MSEE_EMI_MPU_LOCK		EMI_MPU_LOCK
#define MSEE_EMI_MPU_UNLOCK		EMI_MPU_UNLOCK

#define MSEE_EMI_MPU_NO_PROTECTION	EMI_MPU_NO_PROTECTION
#define MSEE_EMI_MPU_SEC_RW		EMI_MPU_SEC_RW
#define MSEE_EMI_MPU_SEC_RW_NSEC_R	EMI_MPU_SEC_RW_NSEC_R
#define MSEE_EMI_MPU_SEC_RW_NSEC_W	EMI_MPU_SEC_RW_NSEC_W
#define MSEE_EMI_MPU_SEC_R_NSEC_R	EMI_MPU_SEC_R_NSEC_R
#define MSEE_EMI_MPU_FORBIDDEN		EMI_MPU_FORBIDDEN
#define MSEE_SEC_R_NSEC_RW		SEC_R_NSEC_RW

#endif //__MT_DRV_MPU_API_H__

