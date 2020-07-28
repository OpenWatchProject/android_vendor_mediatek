/* Copyright Statement:
 *
 * (C) 2005-2016  MediaTek Inc. All rights reserved.
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. ("MediaTek") and/or its licensors.
 * Without the prior written permission of MediaTek and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 * You may only use, reproduce, modify, or distribute (as applicable) MediaTek Software
 * if you have agreed to and been bound by the applicable license agreement with
 * MediaTek ("License Agreement") and been granted explicit permission to do so within
 * the License Agreement ("Permitted User").  If you are not a Permitted User,
 * please cease any access or use of MediaTek Software immediately.
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT MEDIATEK SOFTWARE RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES
 * ARE PROVIDED TO RECEIVER ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 */

#ifndef __EXCEPTION_HANDLER__
#define __EXCEPTION_HANDLER__


 /* Includes -----------------------------------------------------------------*/
#include "stdio.h"
#include "stdint.h"
//#include "stdarg.h"

/* Private define ------------------------------------------------------------*/
#define EXCEPTION_REGION_MAX                    10
#define EXCEPTION_CONTEXT_WORDS                 96

#define EXCEPTION_PATTERN1                      0xff00a5a5
#define EXCEPTION_AR_COUNT                      (32)
#define DEFAULT_TASK_NAME_LEN                   16

/* Private typedef -----------------------------------------------------------*/
/* Total 41 Dword, 164 Byte */
typedef struct ExceptionContextType {
    /* General core registers */
    uint32_t ar[64];
    /* Window option special registers */
    uint32_t windowbase;
    uint32_t windowstart;
    /* Shift amount special registers */
    uint32_t sar;
    /* Exception and Interrupt option special registers */
    uint32_t pc;
    uint32_t exccause;
    uint32_t excvaddr;
    uint32_t excsave1;
    uint32_t epc;
    uint32_t depc;
    uint32_t ps;
    /* Loop option special registers */
    uint32_t lbeg;
    uint32_t lend;
    uint32_t lcount;
    /* FreeRTOS Task info */
    char task_name[DEFAULT_TASK_NAME_LEN];
} ExceptionContext;

void platform_assert(char *file, unsigned int line);
void set_exception_context_pointer(void* addr, unsigned int size);
#endif /*__EXCEPTION_HANDLER__*/
