/* Copyright Statement:
 *
 * @2015 MediaTek Inc. All rights reserved.
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek Inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
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
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE.
 */

#ifndef _TRACE_RTOS_TYPES_H
#define _TRACE_RTOS_TYPES_H

#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include "FreeRTOSConfig.h"

/*Definition of Event Type*/
#define EV_REC_START        0x81     /*start recording*/
#define EV_REC_STOP         0x82     /*stop recording*/
#define EV_DTS              0x83    /*Just DTS*/
#define EV_DTS_H            0x84    /*High part of DTS to the next event*/
#define EV_TAG_BEGIN        0x87
#define EV_TAG_END          0x88
#define EV_TAG_ONESHOT      0x89
#define EV_TASK_SWTCH_1     0x01     /*Switch from a running task*/
#define EV_TASK_SWTCH_2     0x02     /*Switch from a blocked task*/
#define EV_TASK_SWTCH_3     0x03     /*Switch from a suspended task*/
#define EV_TASK_SWTCH_4     0x04     /*Switch from a deleted task*/
#define EV_TASK_CREATE      0x05
#define EV_TASK_DELETE      0x06
#define EV_TASK_WAKEUP      0x07
#define EV_MUTEX_CREATE     0x10
#define EV_QUEUE_CREATE     0x11
#define EV_QUEUE_RECV       0x12     /* queue receive successful */
#define EV_QUEUE_RECV_F     0x13     /* queue receive failed */
#define EV_QUEUE_RECV_B     0x14     /* blocking on queue receive */
#define EV_QUEUE_SEND       0x15     /* queue send successful */
#define EV_QUEUE_SEND_F     0x16     /* queue send failed */
#define EV_QUEUE_SEND_B     0x17     /* blocking on queue send */
#define EV_QUEUE_SEND_ISR   0x18     /* queue send from isr successful */
#define EV_QUEUE_SEND_ISR_F 0x19     /* queue send from isr failed */
#define EV_SWTIMER_CREATE   0x20
#define EV_SWTIMER_CB_START 0x21
#define EV_SWTIMER_CB_END   0x22
#define EV_SWTIMER_CMD_SEND 0x23
#define EV_SWTIMER_CMD_RECV 0x24
#define EV_INT_OFF          0x30
#define EV_INT_OFF_FROM_ISR 0x31
#define EV_INT_ON           0x32
#define EV_INT_ON_FROM_ISR  0x33
#define EV_ISR_ENTER        0x34
#define EV_IST_EXIT         0x35
#define EV_SYS_SUSPEND      0x40
#define EV_SYS_RESUME       0x41
#define EV_OSTICK           0x50

struct task_switch_event_t {
    unsigned int evtype: 8,
             taskno: 8,
             unused: 16;
};

struct task_create_event_t {
    unsigned int priority: 8,
             taskno: 8,
             watermark: 16;
    char name[(configMAX_TASK_NAME_LEN + 0x3) & (~0x3)];
};

struct queue_create_event_t {
    unsigned int evtype: 8,
             qno: 8,
             qtype: 8,
             unused: 8;
    unsigned int qlen: 8,
             dlen: 8,
             qitemsz: 16;
    char name[(configMAX_TASK_NAME_LEN + 0x3) & (~0x3)];
};

struct queue_op_event_t {
    unsigned int evtype: 8,
             taskno: 8,
             qno: 8,
             dlen: 8;
};

struct sleep_event_t {
    unsigned int evtype: 8,
             unused: 24;
};

struct int_event_t {
    unsigned int evtype: 8,
             unused: 24;
    unsigned int mask;
    unsigned int duration_high;
    unsigned int duration_low;
};

struct isr_event_t {
    unsigned int evtype: 8,
             vec: 24;
};

struct timer_create_event_t {
    unsigned int evtype: 8,
             timerno: 8,
             unused: 16;
    char name[(configMAX_TASK_NAME_LEN + 0x3) & (~0x3)];
};

struct timer_cmd_event_t {
    unsigned int evtype: 8,
             timerno: 8,
             taskno: 8,
             cmd: 8;
    unsigned int value;
    unsigned int vec;
};

struct timer_event_t {
    unsigned int evtype: 8,
             timerno: 8,
             unused: 16;
};

struct tick_event_t {
    unsigned int evtype: 8,
             unused: 24;
};

/* this structure should be sync with AP/Linux side of */
struct trace_rtos_hook_t {
    unsigned short task: 2,
             queue: 2,
             swtimer: 2,
             irq: 2,
             sys: 2,
             tick: 2,
             task_no_overflow: 1,
             timer_no_overflow: 1,
             queue_no_overflow: 1,
             unused: 1;
};

union trace_hook_mode {
    unsigned short all;
    struct trace_rtos_hook_t hook;
};

#ifndef FREERTOS_MAX_TASKS
#define FREERTOS_MAX_TASKS 20
#endif
#ifndef FREERTOS_MAX_QUEUES
#define FREERTOS_MAX_QUEUES 50
#endif
#ifndef FREERTOS_MAX_TIMER
#define FREERTOS_MAX_TIMER 10
#endif

extern struct task_create_event_t task_create_event[];
extern struct queue_create_event_t queue_create_event[];
extern struct timer_create_event_t timer_create_event[];
extern void *queue_handle[];
extern int trace_rtos_task_no;
extern int trace_rtos_queue_no;
extern int trace_rtos_swtimer_no;
extern int trace_current_task_no;
extern union trace_hook_mode trace_rtos;
extern struct trace_rtos_hook_t trace_rtos_hook_error;
extern unsigned short rtos_flags;
extern unsigned long long trace_rtos_ts;
extern unsigned long long trace_rtos_ts_diff;
extern int trace_rtos_ts_nesting;

#define MID_RTOS_TASK_CREATE           0x100
#define MID_RTOS_TASK_SWITCH           0x200
#define MID_RTOS_TASKINFO              0x300
#define MID_RTOS_SWTIMER_INFO          0x400
#define MID_RTOS_SWTIMER_CREATE        0x500
#define MID_RTOS_SWTIMER               0x600
#define MID_RTOS_QUEUE_INFO            0x700
#define MID_RTOS_QUEUE_CREATE          0x800
#define MID_RTOS_QUEUE_OP              0x900
#define MID_RTOS_INT                   0xa00
#define MID_RTOS_ISR                   0xb00
#define MID_RTOS_SYS_SLEEP             0xc00
#define MID_RTOS_TICK                  0xd00


#endif /*_TRACE_RTOS_TYPES_H*/
