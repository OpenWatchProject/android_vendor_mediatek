/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2018. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 */


#ifndef SCP_ERR_INFO_H
#define SCP_ERR_INFO_H


/******************************************************************************
******************************************************************************/
#include <stdint.h>     // for uint32_t
#include "scp_ipi.h"


/******************************************************************************
* The following definitions are used in error_info::case_id.
******************************************************************************/
typedef enum err_case_id_t {
    ERR_CASE_ACC_GYR_INIT     = 926006001,
    ERR_CASE_ACC_INIT         = 926006002,
    ERR_CASE_GYR_INIT         = 926006003,
    ERR_CASE_MAG_INIT         = 926006004,
    ERR_CASE_ALS_PS_INIT      = 926006005,
    ERR_CASE_ALS_INIT         = 926006006,
    ERR_CASE_PS_INIT          = 926006007,
    ERR_CASE_BARO_INIT        = 926006008,
    ERR_CASE_I2C              = 926006009,
    ERR_CASE_SPI              = 926006010,
    ERR_CASE_DEV_CHECK        = 926006011,
    ERR_CASE_UNKNOWN          = 0xffffffff
} err_case_id_t;


/******************************************************************************
* The following definitions are used in error_info::sensor_id.
******************************************************************************/
typedef enum err_sensor_id_t {
    ERR_SENSOR_ACC_GYR        = 0x00000001,
    ERR_SENSOR_ACC            = 0x00000002,
    ERR_SENSOR_GYR            = 0x00000003,
    ERR_SENSOR_MAG            = 0x00000004,
    ERR_SENSOR_ALS_PS         = 0x00000005,
    ERR_SENSOR_ALS            = 0x00000006,
    ERR_SENSOR_PS             = 0x00000007,
    ERR_SENSOR_BARO           = 0x00000008,
    ERR_SENSOR_I2C            = 0x00000009,
    ERR_SENSOR_SPI            = 0x0000000a,
    ERR_SENSOR_UNKNOWN        = 0xffffffff
} err_sensor_id_t;


/******************************************************************************
* The following definitions are used in error_info::context[].
******************************************************************************/
#define ERR_CONTEXT_LSM6DS3     "ST-LSM6DS3"


#define ERR_MAX_CONTEXT_LEN     32

typedef struct error_info {
    err_case_id_t case_id;
    err_sensor_id_t sensor_id;
    char context[ERR_MAX_CONTEXT_LEN];
} error_info;


typedef enum wait_ipi {
    WAIT_IPI_NO = 0,    /* Do not wait for IPI completion. */
    WAIT_IPI_YES = 1    /* Wait for IPI completion. */
} wait_ipi;


#ifdef CFG_ERR_INFO_SUPPORT
/******************************************************************************
* Note that this function cannot be called in the ISR context.
******************************************************************************/
ipi_status scp_send_err_to_ap(error_info *info);
#else
static inline ipi_status scp_send_err_to_ap(error_info *info)
{
    return DONE;
}
#endif  // CFG_ERR_INFO_SUPPORT


#endif  // SCP_ERR_INFO_H


