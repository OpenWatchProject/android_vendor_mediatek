/* Copyright Statement:
*
* This oftware/firmware and related documentation ("MediaTek Software") are
* proteed under relevant copyright laws. The information contained herein
* is confidential and proprietary to MediaTek Inc. and/or its licensors.
* Without the prior written permission of MedTek inc. and/or its licensors,
* any reproduction, modification, use or disclosure of MediaTek Software,
* and information contained herein, in whole or in part, shall be strictly prohibited.
*/
/* MediaTek Inc. (C) 2015. All rights reserved.
*
* BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
* THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
* RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
* AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESLY DISCLAIMS ANY AND ALL WARRANTIES,
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
* NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESCT TO THE
* SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
* SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
* THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
* THAT IT IS RECER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
* CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALLALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
* SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
* STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
* CUMULATIVE LIABILITY WITH RESPECT TO THEEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
* AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE T ISSUE,
* OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
* MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
*/

#ifndef __DYNAMIC_OBJECT_H__
#define __DYNAMIC_OBJECT_H__

#include "do_cmd.h"
#include <FreeRTOS.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DO_MAGIC_0 0x616e7944
#define DO_MAGIC_1 0x4f63696d
#define DO_MAGIC_2 0x63656a62
#define DO_MAGIC_3 0xffff0074

typedef struct do_sections_info {
    uint32_t magic[4];
    uint32_t do_features_start;
    uint32_t do_features_end;
    uint32_t do_exidx_start;
    uint32_t do_exidx_end;
    uint32_t do_bss_start;
    uint32_t do_bss_end;
    uint32_t reserved[2];
} doSectionsInfo_t;

typedef struct do_feature_func {
    int (*init_func)(void);
    void (*deinit_func)(void);
} doFeatureFunc_t;

#define FEATURE_DECLARE(name, _init, _deinit)  \
    static const doFeatureFunc_t __used __attribute__((__section__(".do_features"))) feature_func_##name = {  \
    .init_func = (_init),  \
    .deinit_func = (_deinit)  \
};

int32_t mt_do_parse_headers(uint32_t *addr);
uint32_t mt_do_send_obj_info(void);
void do_notify_current_do(void);

/*************** Public APIs (not opened yet) **************/
uint32_t mt_do_load_do(char* do_name);
uint32_t mt_do_unload_do(char* do_name);
char* mt_do_get_loaded_do(void);
uint32_t mt_do_load_first_do(void);

#ifdef __cplusplus
}
#endif

#endif
