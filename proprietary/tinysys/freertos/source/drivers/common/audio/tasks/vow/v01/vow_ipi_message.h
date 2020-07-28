/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2015. All rights reserved.
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


#ifndef _VOW_IPI_MSG_H
#define _VOW_IPI_MSG_H
/*
============================================================================================================
------------------------------------------------------------------------------------------------------------
||                        Header Files
------------------------------------------------------------------------------------------------------------
============================================================================================================
*/
#include "audio_task_vow_params.h"

/*
============================================================================================================
------------------------------------------------------------------------------------------------------------
||                        Definitions
------------------------------------------------------------------------------------------------------------
============================================================================================================
*/
#define VOW_IPI_DEBUG 0
#define VOW_IPI_ERR(fmt, args...)    PRINTF_I("[vow_ipi] ERR: "fmt, ##args)
#define VOW_IPI_DBG(fmt, args...)    PRINTF_D("[vow_ipi] DBG: "fmt, ##args)
#if VOW_IPI_DEBUG
#define VOW_IPI_LOG(fmt, args...)    PRINTF_I("[vow_ipi] LOG: "fmt, ##args)
#else
#define VOW_IPI_LOG(fmt, args...)
#endif


#define VOW_IPI_TIMEOUT           100  // 1ms

/* SCP/AP IPI Message ID */
#define SCP_IPI_AUDMSG_BASE       0x5F00
#define AP_IPI_AUDMSG_BASE        0x7F00
#define CLR_SPEAKER_MODEL_FLAG    0x3535
/*
============================================================================================================
------------------------------------------------------------------------------------------------------------
||                        Enumrations & Data Structures
------------------------------------------------------------------------------------------------------------
============================================================================================================
*/
typedef enum vow_ipi_result {
    VOW_IPI_SUCCESS = 0,
    VOW_IPI_CLR_SMODEL_ID_NOTMATCH,
    VOW_IPI_SET_SMODEL_NO_FREE_SLOT,
} vow_ipi_result;

typedef enum vow_ipi_msgid_t {
    // AP to SCP MSG
    AP_IPIMSG_VOW_ENABLE = AP_IPI_AUDMSG_BASE,
    AP_IPIMSG_VOW_DISABLE,
    AP_IPIMSG_VOW_SETMODE,
    AP_IPIMSG_VOW_APREGDATA_ADDR,
    AP_IPIMSG_VOW_DATAREADY_ACK,
    AP_IPIMSG_SET_VOW_MODEL,
    AP_IPIMSG_VOW_SETGAIN,
    AP_IPIMSG_VOW_SET_FLAG,
    AP_IPIMSG_VOW_RECOGNIZE_OK_ACK,
    AP_IPIMSG_VOW_CHECKREG,
    AP_IPIMSG_VOW_SET_SMART_DEVICE,

    // SCP to AP MSG
    SCP_IPIMSG_VOW_ENABLE_ACK = SCP_IPI_AUDMSG_BASE,
    SCP_IPIMSG_VOW_DISABLE_ACK,
    SCP_IPIMSG_VOW_SETMODE_ACK,
    SCP_IPIMSG_VOW_APREGDATA_ADDR_ACK,
    SCP_IPIMSG_VOW_DATAREADY,
    SCP_IPIMSG_SET_VOW_MODEL_ACK,
    SCP_IPIMSG_VOW_SETGAIN_ACK,
    SCP_IPIMSG_SET_FLAG_ACK,
    SCP_IPIMSG_VOW_RECOGNIZE_OK,
    SCP_IPIMSG_VOW_SET_SMART_DEVICE_ACK,
} vow_ipi_msgid_t;


/*
============================================================================================================
------------------------------------------------------------------------------------------------------------
||                        Macors
------------------------------------------------------------------------------------------------------------
============================================================================================================
*/

/*
============================================================================================================
------------------------------------------------------------------------------------------------------------
||                        Extern
------------------------------------------------------------------------------------------------------------
============================================================================================================
*/
void vow_ipi_init(void);
int vow_ipi_sendmsg(vow_ipi_msgid_t id, void *buf, vow_ipi_result result, unsigned int size, unsigned int wait);
void vow_ipi_handler(int id, void * data, unsigned int len);


#endif  // _VOW_IPI_MSG_H
