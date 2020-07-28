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

#ifndef _OVERLAY_INIT_H_
#define _OVERLAY_INIT_H_

#include "main.h"

#ifdef CFG_OVERLAY_INIT_SUPPORT

/*SCP hardcode def*/
#define SCP_IMAGE_INFO_ADDR      0x00000400
#define SCP_IMAGE_FW_OFFSET      0x800

/*************************************************************************
* identify how many overlay section
* The symbol OVERLAY_WORK_00 corresponds to OVERLAY0 at system.ld.template.
* The symbol OVERLAY_WORK_01 corresponds to OVERLAY1, and so on follows.
* Do not change the mapping (unless you know what you are doing) when you
* define new meaningful symbols for sensor driver.
* so far, this platform sensor will use OVERLAY_WORK_00 to OVERLAY_WORK_05,
* any use please start from OVERLAY_WORK_06
*************************************************************************/
enum OVERLAY_WS {
    OVERLAY_WORK_00 = 0,
    OVERLAY_WORK_01,
    OVERLAY_WORK_02,
    OVERLAY_WORK_03,
    OVERLAY_WORK_04,
    OVERLAY_WORK_05,
    OVERLAY_WORK_06,
    OVERLAY_WORK_07,
    OVERLAY_WORK_08,
    OVERLAY_WORK_09,
    NUM_OVERLAY_WORK,

    /*for sensor start*/
    /*acceleremter and gyroscope sensor*/
    OVERLAY_ID_ACCGYRO         = OVERLAY_WORK_00,
    /*magnetometer sensor*/
    OVERLAY_ID_MAG             = OVERLAY_WORK_01,
    /*light and proximity sensor*/
    OVERLAY_ID_ALSPS           = OVERLAY_WORK_02,
    /*barometer sensor*/
    OVERLAY_ID_BARO            = OVERLAY_WORK_03,
    /*light and proximity sensor secondary*/
    OVERLAY_ID_ALSPS_SECONDARY = OVERLAY_WORK_04,
    /*sar sensor*/
    OVERLAY_ID_SAR             = OVERLAY_WORK_05,
    /*for sensor end*/
};

struct overlay_init_s {
    int overlay_section;
    char* name;
    char* overlay_load_start;
    char* overlay_load_end;
    void* data;
};



extern void * tinysys_overlay_select(char* name);
extern void tinysys_overlay_init();

#define OVERLAY_DECLARE(ovl_name, ovl_section, ovl_data) \
    struct overlay_init_s overlay_init_##ovl_name __attribute__((section(".overlay_init"))) = { \
        .overlay_section = ovl_section, \
        .data = ovl_data, \
    }

#define OVERLAY_INIT(ovl_name) do { \
    extern char __load_start_##ovl_name; \
    extern char __load_stop_##ovl_name; \
    extern struct overlay_init_s overlay_init_##ovl_name; \
    overlay_init_##ovl_name.name = #ovl_name; \
    overlay_init_##ovl_name.overlay_load_start = &__load_start_##ovl_name; \
    overlay_init_##ovl_name.overlay_load_end = &__load_stop_##ovl_name; \
    } while(0)


#define TINYSYS_OVERLAY_SELECT(ovl_name) \
({ \
    void* overlay_select_ret; \
    OVERLAY_INIT(ovl_name); \
    overlay_select_ret = tinysys_overlay_select(#ovl_name); \
    overlay_select_ret; \
})


#endif
#endif
