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
#ifndef _FREERTOS_TRACE_MACRO_H
#define _FREERTOS_TRACE_MACRO_H

#include "trace_rtos_types.h"
#include "trace.h"
#include "mt_trace.h"

#ifdef OSHOOK_TEST
extern int oshook_printb(int mid, unsigned int *buf, int len, int mode);
extern int oshook_printb_isr(int mid, unsigned int *buf, int len_unit_uint32);
extern unsigned long long oshook_timestamp(void);
#define trace_printb      oshook_printb
#define trace_printb_isr  oshook_printb_isr
#define trace_get_timestamp(ctx)  oshook_timestamp()
#endif

void traceINT_ON_LOG(unsigned int evtype, unsigned int mask);

#define traceTASK_CREATE(pxNewTCB)                                      \
    pxNewTCB->uxTaskNumber = uxTaskNumber;                              \
    if (trace_rtos_task_no < FREERTOS_MAX_TASKS) {                        \
        task_create_event[trace_rtos_task_no].taskno = pxNewTCB->uxTCBNumber; \
        task_create_event[trace_rtos_task_no].priority = pxNewTCB->uxPriority;\
        task_create_event[trace_rtos_task_no].watermark = 0;                     \
        memcpy(task_create_event[trace_rtos_task_no].name, pxNewTCB->pcTaskName, configMAX_TASK_NAME_LEN);                                                    \
        trace_rtos_task_no++;                                                 \
    }                                                                   \
    else { \
        trace_rtos_hook_error.task_no_overflow = 1; \
    }


#if ((INCLUDE_vTaskDelete == 1) && (INCLUDE_vTaskSuspend == 1))
#define traceTASK_SWITCHED_OUT()                  \
{                                                 \
    struct task_switch_event_t ev;                \
    List_t *pxStateList;                          \
                                                  \
if (trace_rtos.hook.task == 1) {                   \
    ev.unused = 0;                                \
    pxStateList = (List_t *)listLIST_ITEM_CONTAINER(&(pxCurrentTCB->xStateListItem));                                            \
                                                  \
    if ((pxStateList == pxDelayedTaskList) || (pxStateList == pxOverflowDelayedTaskList))                                          \
    {                                             \
        ev.evtype = EV_TASK_SWTCH_2;              \
    }                                             \
    else if (pxStateList == &xSuspendedTaskList)  \
    {                                             \
        if (listLIST_ITEM_CONTAINER(&(pxCurrentTCB->xEventListItem)) == NULL)                                                      \
        {                                         \
            ev.evtype = EV_TASK_SWTCH_3;          \
        }                                         \
        else                                      \
        {                                         \
            ev.evtype = EV_TASK_SWTCH_2;          \
        }                                         \
    }                                             \
    else if (pxStateList == &xTasksWaitingTermination) \
    {                                             \
        ev.evtype = EV_TASK_SWTCH_4;              \
    }                                             \
    else                                          \
    {                                             \
        ev.evtype = EV_TASK_SWTCH_1;              \
    }                                             \
}
#endif

#if ((INCLUDE_vTaskDelete == 1) && (INCLUDE_vTaskSuspend == 0))

#define traceTASK_SWITCHED_OUT()                       \
{                                                      \
    struct task_switch_event_t ev;                     \
    List_t *pxStateList;                               \
                                                       \
if (trace_rtos.hook.task == 1) {                        \
    ev.unused = 0;                                     \
    pxStateList = (List_t *) listLIST_ITEM_CONTAINER(&(pxCurrentTCB->xStateListItem));                                                \
                                                       \
    if ((pxStateList == pxDelayedTaskList) || (pxStateList == pxOverflowDelayedTaskList))                                               \
    {                                                  \
        ev.evtype = EV_TASK_SWTCH_2;                   \
    }                                                  \
    else if (pxStateList == &xTasksWaitingTermination) \
    {                                                  \
        ev.evtype = EV_TASK_SWTCH_4;                   \
    }                                                  \
    else                                               \
    {                                                  \
        ev.evtype = EV_TASK_SWTCH_1;                   \
    }                                                  \
}
#endif

#if ((INCLUDE_vTaskDelete == 0) && (INCLUDE_vTaskSuspend == 1))

#define traceTASK_SWITCHED_OUT()                 \
{                                                \
    struct task_switch_event_t ev;               \
    List_t *pxStateList;                         \
                                                 \
if (trace_rtos.hook.task == 1) {                  \
    ev.unused = 0;                               \
    pxStateList = (List_t *)listLIST_ITEM_CONTAINER(&(pxCurrentTCB->xStateListItem));                                           \
                                                 \
    if ((pxStateList == pxDelayedTaskList) || (pxStateList == pxOverflowDelayedTaskList))                                         \
    {                                            \
ev.evtype = EV_TASK_SWTCH_2;                     \
    }                                            \
    else if (pxStateList == &xSuspendedTaskList) \
    {                                            \
        if (listLIST_ITEM_CONTAINER(&(pxCurrentTCB->xEventListItem)) == NULL) \
        {                                        \
            ev.evtype = EV_TASK_SWTCH_3;         \
        }                                        \
        else                                     \
        {                                        \
            ev.evtype = EV_TASK_SWTCH_2;         \
        }                                        \
    }                                            \
    else                                         \
    {                                            \
        ev.evtype = EV_TASK_SWTCH_1;             \
    }                                            \
}
#endif

#if ((INCLUDE_vTaskDelete == 0) && (INCLUDE_vTaskSuspend == 0))

#define traceTASK_SWITCHED_OUT()     \
{                                    \
    struct task_switch_event_t ev;   \
    List_t *pxStateList;             \
                                     \
if (trace_rtos.hook.task == 1) {      \
    ev.unused = 0;                   \
    pxStateList = (List_t *)listLIST_ITEM_CONTAINER(&(pxCurrentTCB->xStateListItem));                               \
                                     \
    if ((pxStateList == pxDelayedTaskList) || (pxStateList == pxOverflowDelayedTaskList))                             \
    {                                \
        ev.evtype = EV_TASK_SWTCH_2; \
    }                                \
    else                             \
    {                                \
        ev.evtype = EV_TASK_SWTCH_1; \
    }                                \
}
#endif

#define traceTASK_SWITCHED_IN()                                           \
if (trace_rtos.hook.task == 1) {                                            \
    ev.taskno = pxCurrentTCB->uxTCBNumber;                                \
    trace_printb_isr(MID_RTOS|MID_RTOS_TASK_SWITCH, (unsigned int *)&ev, 1);\
}                                                                         \
trace_current_task_no = pxCurrentTCB->uxTCBNumber;                          \
}

#define traceMOVED_TASK_TO_READY_STATE(pTCB)                              \
if (trace_rtos.hook.task == 1) {                                            \
    struct task_switch_event_t ev;                                        \
    ev.evtype = EV_TASK_WAKEUP;                                           \
    ev.taskno = pTCB->uxTCBNumber;                                        \
    ev.unused = 0;                                                        \
    trace_printb_isr(MID_RTOS|MID_RTOS_TASK_SWITCH, (unsigned int *)&ev, 1);\
}

#define traceQUEUE_CREATE( pxNewQueue )                                        \
    taskENTER_CRITICAL();                                                      \
    if (trace_rtos_queue_no < FREERTOS_MAX_QUEUES) {                             \
        queue_handle[trace_rtos_queue_no] = pxNewQueue;                          \
        queue_create_event[trace_rtos_queue_no].evtype = EV_QUEUE_CREATE;        \
        queue_create_event[trace_rtos_queue_no].qno = trace_rtos_queue_no;         \
        queue_create_event[trace_rtos_queue_no].qtype = pxNewQueue->ucQueueType; \
        queue_create_event[trace_rtos_queue_no].unused = 0;                      \
        queue_create_event[trace_rtos_queue_no].qlen = pxNewQueue->uxLength;     \
        queue_create_event[trace_rtos_queue_no].dlen = 0;                        \
        queue_create_event[trace_rtos_queue_no].qitemsz = pxNewQueue->uxItemSize;\
        queue_create_event[trace_rtos_queue_no].name[0] = 0;                     \
        pxNewQueue->uxQueueNumber = trace_rtos_queue_no++;                       \
    }                                                                          \
    else {                                                                     \
        trace_rtos_hook_error.queue_no_overflow = 1;                             \
    }                                                                          \
    taskEXIT_CRITICAL();

#define traceCREATE_COUNTING_SEMAPHORE( )                                      \
    taskENTER_CRITICAL();                                                      \
    queue_create_event[((Queue_t *)xHandle)->uxQueueNumber].dlen = uxInitialCount;       \
    taskEXIT_CRITICAL();

#define traceCREATE_MUTEX( pxNewQueue )                                        \
    taskENTER_CRITICAL();                                                      \
    if (trace_rtos_queue_no < FREERTOS_MAX_QUEUES) {                             \
        queue_handle[trace_rtos_queue_no] = pxNewQueue;                          \
        queue_create_event[trace_rtos_queue_no].evtype = EV_MUTEX_CREATE;        \
        queue_create_event[trace_rtos_queue_no].qno = trace_rtos_queue_no;         \
        queue_create_event[trace_rtos_queue_no].qtype = pxNewQueue->ucQueueType; \
        queue_create_event[trace_rtos_queue_no].unused = 0;                      \
        queue_create_event[trace_rtos_queue_no].qlen = pxNewQueue->uxLength;     \
        queue_create_event[trace_rtos_queue_no].dlen = 1;                        \
        queue_create_event[trace_rtos_queue_no].qitemsz = pxNewQueue->uxItemSize;\
        queue_create_event[trace_rtos_queue_no].name[0] = 0;                     \
        pxNewQueue->uxQueueNumber = trace_rtos_queue_no++;                       \
    }                                                                          \
    else {                                                                     \
        trace_rtos_hook_error.queue_no_overflow = 1;                             \
    }                                                                          \
    taskEXIT_CRITICAL();

#define traceQUEUE_REGISTRY_ADD( xQueue, pcQueueName )                         \
{                                                                              \
    int idx = ((Queue_t*)xQueue)->uxQueueNumber;                               \
    if (idx < FREERTOS_MAX_QUEUES) {                                           \
        memcpy(queue_create_event[idx].name, pcQueueName, configMAX_TASK_NAME_LEN); \
    }                                                                          \
}

#define traceQUEUE_RECEIVE( pxQueue )                                                    \
if (trace_rtos.hook.queue == 1)                                                            \
{                                                                                        \
    struct queue_op_event_t ev;                                                          \
    ev.evtype = EV_QUEUE_RECV;                                                           \
    ev.taskno = trace_current_task_no;                                                     \
    ev.qno = pxQueue->uxQueueNumber;                                                     \
    ev.dlen = pxQueue->uxMessagesWaiting;                                                \
    taskENTER_CRITICAL(); \
    trace_printb_isr(MID_RTOS|MID_RTOS_QUEUE_OP, (unsigned int *)&ev, 1);  \
    taskEXIT_CRITICAL(); \
}

#define traceQUEUE_RECEIVE_FAILED( pxQueue )                                             \
if (trace_rtos.hook.queue == 1)                                                            \
{                                                                                        \
    struct queue_op_event_t ev;                                                          \
    ev.evtype = EV_QUEUE_RECV_F;                                                         \
    ev.taskno = trace_current_task_no;                                                     \
    ev.qno = pxQueue->uxQueueNumber;                                                     \
    ev.dlen = pxQueue->uxMessagesWaiting;                                                \
    taskENTER_CRITICAL(); \
    trace_printb_isr(MID_RTOS|MID_RTOS_QUEUE_OP, (unsigned int *)&ev, 1);  \
    taskEXIT_CRITICAL(); \
}

#define traceBLOCKING_ON_QUEUE_RECEIVE(pxQueue)                                          \
if (trace_rtos.hook.queue == 1)                                                            \
{                                                                                        \
    struct queue_op_event_t ev;                                                          \
    ev.evtype = EV_QUEUE_RECV_B;                                                         \
    ev.taskno = trace_current_task_no;                                                     \
    ev.qno = pxQueue->uxQueueNumber;                                                     \
    ev.dlen = pxQueue->uxMessagesWaiting;                                                \
    taskENTER_CRITICAL(); \
    trace_printb_isr(MID_RTOS|MID_RTOS_QUEUE_OP, (unsigned int *)&ev, 1);  \
    taskEXIT_CRITICAL(); \
}

#define traceQUEUE_SEND( pxQueue )                                                       \
if (trace_rtos.hook.queue == 1)                                                            \
{                                                                                        \
    struct queue_op_event_t ev;                                                          \
    ev.evtype = EV_QUEUE_SEND;                                                           \
    ev.taskno = trace_current_task_no;                                                     \
    ev.qno = pxQueue->uxQueueNumber;                                                     \
    ev.dlen = pxQueue->uxMessagesWaiting;                                                \
    taskENTER_CRITICAL(); \
    trace_printb_isr(MID_RTOS|MID_RTOS_QUEUE_OP, (unsigned int *)&ev, 1);  \
    taskEXIT_CRITICAL(); \
}

/* To discuss: binary semaphore with send wait time 0 (multiple signals but no action) */
#define traceQUEUE_SEND_FAILED( pxQueue )                                                \
if (trace_rtos.hook.queue == 1)                                            \
{                                                                                        \
    struct queue_op_event_t ev;                                                          \
    ev.evtype = EV_QUEUE_SEND_F;                                                         \
    ev.taskno = trace_current_task_no;                                                     \
    ev.qno = pxQueue->uxQueueNumber;                                                     \
    ev.dlen = pxQueue->uxMessagesWaiting;                                                \
    taskENTER_CRITICAL(); \
    trace_printb_isr(MID_RTOS|MID_RTOS_QUEUE_OP, (unsigned int *)&ev, 1);  \
    taskEXIT_CRITICAL(); \
}

#define traceBLOCKING_ON_QUEUE_SEND(pxQueue)                                             \
if (trace_rtos.hook.queue == 1)                                                            \
{                                                                                        \
    struct queue_op_event_t ev;                                                          \
    ev.evtype = EV_QUEUE_SEND_B;                                                         \
    ev.taskno = trace_current_task_no;                                                     \
    ev.qno = pxQueue->uxQueueNumber;                                                     \
    ev.dlen = pxQueue->uxMessagesWaiting;                                                \
    taskENTER_CRITICAL(); \
    trace_printb_isr(MID_RTOS|MID_RTOS_QUEUE_OP, (unsigned int *)&ev, 1);  \
    taskEXIT_CRITICAL(); \
}

#define traceQUEUE_SEND_FROM_ISR( pxQueue )                                              \
if (trace_rtos.hook.queue == 1)                                                            \
{                                                                                        \
    struct queue_op_event_t ev;                                                          \
    ev.evtype = EV_QUEUE_SEND_ISR;                                                       \
    ev.taskno = read_taken_INT();                                                    \
    ev.qno = pxQueue->uxQueueNumber;                                                     \
    ev.dlen = pxQueue->uxMessagesWaiting;                                                \
    trace_printb_isr(MID_RTOS|MID_RTOS_QUEUE_OP, (unsigned int *)&ev, 1);                  \
}

#define traceQUEUE_SEND_FROM_ISR_FAILED( pxQueue )                                       \
if (trace_rtos.hook.queue == 1)                                                            \
{                                                                                        \
    struct queue_op_event_t ev;                                                          \
    ev.evtype = EV_QUEUE_SEND_ISR_F;                                                     \
    ev.taskno = read_taken_INT();                                                   \
    ev.qno = pxQueue->uxQueueNumber;                                                     \
    ev.dlen = pxQueue->uxMessagesWaiting;                                                \
    trace_printb_isr(MID_RTOS|MID_RTOS_QUEUE_OP, (unsigned int *)&ev, 1);                  \
}

#define traceSYS_SUSPEND() \
if (trace_rtos.hook.sys == 1){ \
        struct sleep_event_t ev; \
        ev.evtype = EV_SYS_SUSPEND; \
    trace_printb_isr(MID_RTOS|MID_RTOS_SYS_SLEEP, (unsigned int *)&ev, 1);     \
}

#define traceSYS_RESUME() \
if (trace_rtos.hook.sys == 1){ \
    struct sleep_event_t ev; \
    ev.evtype = EV_SYS_RESUME; \
    trace_printb_isr(MID_RTOS|MID_RTOS_SYS_SLEEP, (unsigned int *)&ev, 1);     \
}


// use PGT6 timer
#ifndef configTRACE_INT_MASK_TIME_BOUND_NS
#define configTRACE_INT_MASK_TIME_BOUND_NS (50*1000) // 50 us
#endif

#if 1 /* INT */
#define traceINT_OFF() \
if (trace_rtos.hook.irq == 1){ \
    trace_rtos_ts = trace_get_timestamp(CTX_ISR); \
}

#define traceINT_OFF_FROM_ISR() \
if (trace_rtos.hook.irq == 1){ \
trace_rtos_ts_nesting++; \
    if (trace_rtos_ts_nesting == 1) { \
        trace_rtos_ts = trace_get_timestamp(CTX_ISR); \
    } \
}

#define traceINT_ON() \
if ((trace_rtos.hook.irq == 1) && (trace_rtos_ts != 0)) { \
    traceINT_ON_LOG(EV_INT_ON, 0xffffffff); \
}

#define traceINT_ON_FROM_ISR() \
if ((trace_rtos.hook.irq == 1) && (trace_rtos_ts != 0)) { \
    trace_rtos_ts_nesting--; \
    if (trace_rtos_ts_nesting == 0) { \
        traceINT_ON_LOG(EV_INT_ON_FROM_ISR, ulNewMaskValue); \
    } \
}

#endif /* INT */

#define traceISR_ENTER() \
struct isr_event_t ev; \
if (trace_rtos.hook.irq == 1){ \
    ev.evtype = EV_ISR_ENTER; \
    ev.vec = read_taken_INT(); \
    trace_printb_isr(MID_RTOS|MID_RTOS_ISR, (unsigned int *)&ev, 1);     \
}

#define traceISR_EXIT() \
if (trace_rtos.hook.irq == 1){ \
    ev.evtype = EV_IST_EXIT; \
    ev.vec = read_taken_INT(); \
    trace_printb_isr(MID_RTOS|MID_RTOS_ISR, (unsigned int *)&ev, 1);     \
}

#define traceTIMER_CREATE( pxNewTimer ) \
    taskENTER_CRITICAL(); \
    if (trace_rtos_swtimer_no < FREERTOS_MAX_TIMER) { \
        timer_create_event[trace_rtos_swtimer_no].evtype = EV_SWTIMER_CREATE; \
        timer_create_event[trace_rtos_swtimer_no].timerno = trace_rtos_swtimer_no; \
        timer_create_event[trace_rtos_swtimer_no].unused = 0; \
        memcpy(timer_create_event[trace_rtos_swtimer_no].name, pcTimerName, configMAX_TASK_NAME_LEN); \
        pxNewTimer->uxTimerNumber = trace_rtos_swtimer_no++; \
    }      \
    else { \
        trace_rtos_hook_error.timer_no_overflow = 1; \
    }      \
    taskEXIT_CRITICAL();

#define traceTIMER_CALLBACK_START(pxTimer) \
if (trace_rtos.hook.swtimer == 1){ \
    struct timer_event_t ev; \
    ev.evtype = EV_SWTIMER_CB_START; \
    ev.timerno = pxTimer->uxTimerNumber; \
    taskENTER_CRITICAL(); \
    trace_printb_isr(MID_RTOS|MID_RTOS_SWTIMER, (unsigned int *)&ev, 1); \
    taskEXIT_CRITICAL(); \
}

#define traceTIMER_CALLBACK_END(pxTimer) \
if (trace_rtos.hook.swtimer == 1){ \
    struct timer_event_t ev; \
    ev.evtype = EV_SWTIMER_CB_END; \
    ev.timerno = pxTimer->uxTimerNumber; \
    taskENTER_CRITICAL(); \
    trace_printb_isr(MID_RTOS|MID_RTOS_SWTIMER, (unsigned int *)&ev, 1); \
    taskEXIT_CRITICAL(); \
}

#define traceTIMER_COMMAND_SEND( xTimer, xMessageID, xMessageValueValue, xReturn ) \
if (trace_rtos.hook.swtimer == 1) { \
    struct timer_cmd_event_t ev; \
    ev.evtype = EV_SWTIMER_CMD_SEND; \
    ev.timerno = ((Timer_t *)xTimer)->uxTimerNumber; \
    ev.cmd = xMessageID; \
    ev.value = xMessageValueValue ; \
    if (xMessageID >= tmrFIRST_FROM_ISR_COMMAND) { \
    ev.vec = read_taken_INT(); \
    ev.taskno = 0xff;\
    trace_printb_isr(MID_RTOS|MID_RTOS_SWTIMER, (unsigned int *)&ev, 3); \
    } \
    else { \
    ev.taskno = trace_current_task_no;\
    ev.vec = 0; \
    taskENTER_CRITICAL(); \
    trace_printb_isr(MID_RTOS|MID_RTOS_SWTIMER, (unsigned int *)&ev, 3); \
    taskEXIT_CRITICAL(); \
    } \
}

#define traceTIMER_COMMAND_RECEIVED( pxTimer, xMessageID, xMessageValue ) \
if (trace_rtos.hook.swtimer == 1){ \
    struct timer_event_t ev; \
    ev.evtype = EV_SWTIMER_CMD_RECV; \
    ev.timerno = pxTimer->uxTimerNumber; \
    taskENTER_CRITICAL(); \
    trace_printb_isr(MID_RTOS|MID_RTOS_SWTIMER, (unsigned int *)&ev, 1); \
    taskEXIT_CRITICAL(); \
}

#ifndef configTRACE_TICK_PERIOD
#define configTRACE_TICK_PERIOD 1000
#endif

#define traceTASK_INCREMENT_TICK(xTickCount) \
if (trace_rtos.hook.tick == 1) { \
    static TickType_t xTickCountLast = 0; \
    struct tick_event_t ev; \
    ev.evtype = EV_OSTICK; \
    if (xTickCount - xTickCountLast >= configTRACE_TICK_PERIOD) { \
        trace_printb_isr(MID_RTOS|MID_RTOS_TICK, (unsigned int *)&ev, 1); \
        xTickCountLast = xTickCount; \
    } \
}
#endif //_FREERTOS_TRACE_MACRO_H
