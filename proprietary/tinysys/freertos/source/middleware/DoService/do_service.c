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

#include <tinysys_config.h>
#include <stdio.h>
#include <string.h>
#include <platform_mtk.h>
#include <stddef.h>
#include <interrupt.h>
#include <utils.h>
#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>
#include "do_service.h"
#include "dynamic_object.h"
#include "mt_gpt.h"

#define MAX_RETRY       100
#define WAIT            1
#define CMD_BUFF_LEN    35

#include <verify.h>

enum DOState {
    DO_INIT = 1,
    DO_DONE,
    DO_LOAD,
    DO_UNLOAD,
    DO_SAVE_INFO,
    DO_TERMINATED = 99
};

typedef struct command {
    uint8_t type;
    char target[CMD_BUFF_LEN];
} DOCommand_t;

static QueueHandle_t cmdq;
static int retry_count = 0;
static int state;
static uint8_t *payload_buf;
static TaskHandle_t this_task;
static uint32_t scp_num = 0;

static void do_ipi_handler(int ip, void* data, unsigned int len);
static void do_save_info(DoMsgHeader_t* data);
static void do_load_module(DoMsgHeader_t* data);
static void do_unload_module(DoMsgHeader_t* data);

/************ IPI handler & related functions *************/
static void do_ipi_handler(int ip, void* data, unsigned int len)
{

    DoMsgHeader_t* header;
    header = (DoMsgHeader_t*)data;

    switch (header->type) {
        case DO_MSG_INFO:
            PRINTF_D("ipi: do_save_info, disabled now\n\r");
            break;
        case DO_MSG_DRAM_ADDR:
            PRINTF_D("ipi: get dram addr\n\r");
            do_save_info(header);
            break;
        case DO_MSG_GET_INFO:
            PRINTF_D("get DO_MSG_GET_INFO; should not receive this from AP\n\r");
            break;
        case DO_MSG_LOAD_MODULE:
            PRINTF_D("ipi: load do\n\r");
            do_load_module(header);
            break;
        case DO_MSG_UNLOAD_MODULE:
            PRINTF_D("ipi: unload do\n\r");
            do_unload_module(header);
            break;
        case DO_MSG_DO_DISABLED:
            state = DO_TERMINATED;
            break;
        default:
            PRINTF_D("ipi: unknown msg type\n\r");
            break;
    }
}

/* do_save_info:
 * called by handler
 * call DO center init function to build up DO table
 */
static inline void do_save_info(DoMsgHeader_t* data)
{
    if (state != DO_INIT) return; // prevent from duplicated init flow

    /* get scp number from DoMsgHeader assigned by AP */
    scp_num = data->scp_num;
    PRINTF_D("do_save_info: assigned scp number = %d\n", scp_num);

    memcpy(payload_buf, (uint8_t*)&(data->payload), data->len);
    PRINTF_D("do_save_info: data len = %u, data = 0x%x\n", data->len, data->payload);
    state = DO_SAVE_INFO;
}

/* do_load_module:
 * called by handler, change state only
 */
static inline void do_load_module(DoMsgHeader_t* data)
{

    DOCommand_t cmd;
    cmd.type = DO_LOAD;
    strncpy(cmd.target, (char*)&data->payload, sizeof(cmd.target));
    if (cmdq) {
        xQueueSendFromISR(cmdq, (void*)&cmd, (TickType_t)0);// don't block
    }
}

/* do_unload_module:
 * called by handler, change state only
 */
static inline void do_unload_module(DoMsgHeader_t* data)
{

    DOCommand_t cmd;
    cmd.type = DO_UNLOAD;
    strncpy(cmd.target, (char*)&data->payload, sizeof(cmd.target));
    if (cmdq) {
        xQueueSendFromISR(cmdq, (void*)&cmd, (TickType_t)0);// don't block
    }
}

int32_t do_ipi_send(uint8_t type, void* buf, uint32_t bufsize)
{
    //PRINTF_D("do_ipi_send: %d\n", type);
    ipi_status status;
    DoMsgHeader_t* header;
    uint8_t ipibuf[IPI_BUFF_SIZE];

    if (bufsize > IPI_BUFF_SIZE - 1) {
        PRINTF_E("payload size exceeds ipi buffer size: %u\n\r", bufsize);
        return -1;
    }

    header = (DoMsgHeader_t*) & (ipibuf[0]);
    header->scp_num = scp_num;
    header->type = (uint32_t)type;
    header->len = (uint32_t)bufsize;
    if (bufsize != 0 && buf != NULL) {
        memcpy((void*) & (header->payload), buf, bufsize);
    }

    status = scp_ipi_send(IPI_DO_SCP_MSG, ipibuf, bufsize + sizeof(DoMsgHeader_t) - 1, WAIT, IPI_SCP2AP);
    if (status == BUSY) {
        retry_count ++;
        if (retry_count > MAX_RETRY) {
            PRINTF_E("ERR: retry time out, max = %d\n\r", MAX_RETRY);
            retry_count = 0;
            return -1;
        } else {
            udelay(1000); // delay 1 ms
            return 0;
        }
    } else if (status == ERROR) {
        PRINTF_E("ERR: ipi error\n\r");
        return -1;
    }

    retry_count = 0;
    return 1;
}

void do_notify_current_do(void)
{
    //PRINTF_D("update_current_do:\n");
    char* current_do = mt_do_get_loaded_do();
    if (current_do) {
        while (!do_ipi_send(DO_MSG_UPDATE_CURRENT_DO,
                            current_do,
                            strlen(current_do) + 1) // include '\0'
              );
    } else {
        while (!do_ipi_send(DO_MSG_UPDATE_CURRENT_DO,
                            NULL,
                            0) // include '\0'
              );
    }
}

void do_request_obj_info(void)
{
    if (state != DO_INIT) return; // prevent from duplicated init flow

    PRINTF_D("do_request_obj_info\n");
    while (!do_ipi_send(DO_MSG_GET_INFO, NULL, 0));
}

extern const uint32_t end;

/******** Free RTOS testdo task *********/
void do_service_task(void* pvParameters)
{
    DOCommand_t cmd;
    int ret, count = 0;

    cmdq = xQueueCreate(5, sizeof(DOCommand_t));
    payload_buf = (uint8_t*) pvPortMalloc(IPI_BUFF_SIZE);

    PRINTF_E("symbol 'end' address: %p\n", end);

    while (1) {

        switch (state) {
            case DO_INIT: /* wait for dram addr sent by kernel */
                mdelay(100); // 100 ms
                if (count > 200) { // 20 s
                    PRINTF_E("do init timeout\n");
                    state = DO_TERMINATED;
                }
                else count++;
                break;
            case DO_SAVE_INFO:

                ret = mt_do_parse_headers((uint32_t*)payload_buf);
                if (ret) {
                    mt_do_send_obj_info();
                    vPortFree(payload_buf);

                    /* init: load the first DO by default */
                    mt_do_load_first_do();
                    do_notify_current_do();

                    state = DO_DONE;
                } else {
                    PRINTF_E("save DO info failed\n");
                    vPortFree(payload_buf);
                    state = DO_TERMINATED;
                }
                break;
            case DO_DONE:

                if (!cmdq) break;
                if (xQueueReceive(cmdq, &cmd, (TickType_t)portMAX_DELAY)) {
                    switch (cmd.type) {
                        case DO_LOAD:
                            ret = mt_do_load_do(cmd.target);
                            do_notify_current_do();
                            if (ret) {

                                // FOR TESTING
                                if (!strcmp(cmd.target, "SAMPLE1")) {
                                    verify_do();
                                }

                            }
                            break;
                        case DO_UNLOAD:
                            mt_do_unload_do(cmd.target);
                            do_notify_current_do();
                            break;
                        default:
                            break;
                    }
                }
                break;
            case DO_TERMINATED:
                do_service_deinit();
            default:
                //do nothing
                break;
        }
    }
}

/******** FreeRTOS entry point *********/
void do_service_init()
{
    state = DO_INIT;

    // init ipi
    scp_ipi_registration(IPI_DO_AP_MSG, do_ipi_handler, "do_service");
    scp_ipi_wakeup_ap_registration(IPI_DO_SCP_MSG);


    // create task
    kal_xTaskCreate(
        (TaskFunction_t)do_service_task,
        "do_service",
        256,
        (void*)NULL, 3, &this_task);
}

void do_service_deinit()
{
    vTaskDelete(this_task);
}
