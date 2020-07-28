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
#ifndef __FLP_SERVICE_C_INCLUDED__
#define __FLP_SERVICE_C_INCLUDED__

#include "string.h"
#include "sensor_manager_fw.h"
#include "sensor_manager.h"
#ifdef CFG_CCCI_SUPPORT
#include "ccci.h"
#endif
#include <stdint.h>
#include <scp_ipi.h>
#include <malloc.h>
#include <interrupt.h>
#include "FreeRTOS.h"
#include "flp_service.h"
#ifdef CFG_GPS_SUPPORT
#include "gps_driver.h"
#endif

#define FLP_DEBUG
#ifdef FLP_DEBUG
#define FLOG(fmt, args...)    PRINTF_D("[FLP]: "fmt, ##args)
#else
#define FLOG(fmt, args...)
#endif

extern unsigned char _SCP_TO_CNN_SHARE_BUFFER_ADDR;
extern unsigned char _CNN_TO_SCP_SHARE_BUFFER_ADDR;
#define SCP_TO_CNN_SHARE_BUFFER     ((volatile unsigned int*)&_SCP_TO_CNN_SHARE_BUFFER_ADDR)
#define CNN_TO_SCP_SHARE_BUFFER     ((volatile unsigned int*)&_CNN_TO_SCP_SHARE_BUFFER_ADDR)
#define SCP_TO_CONN_INT      0x400A002C
#define CONN_TO_SCP_INT      0x400A0034
#define MPE_AUTO_STOP_CNT    4
#define SMD_REG_FREQ         1000

static UINT8 modem_init = 0, flp_pdr_init = 0, pdr_sensor_init = 0, mpe_adr_init = 0;
static UINT8 smd_sensor_int = 0, station_sensor_int = 0;
static int flp_handle = -1;
static int mdm_cnt = 0;
static int gps_in_cnt = -1;
static int current_motion = -1;
static int last_motion = -1;
modem_vec_t modem_data[10];
MTK_FLP_PDR_INPUT pdr_res = {0};
static QueueHandle_t flp_eint_queue;

#ifdef MPE_DYNAMIC_LOGGING
extern int logging_on;
#endif


void onSensorChanged_flp_srv(int sensor, struct data_t *values);
void onSensorAccChange_flp_srv(int sensor, int accuracy);
static void flp_eint_handler(void);
static void flp_eint_task(void *pvParameters);
static void flp_trigger_eint_task(UINT32 event_id);
//extern void irq_status_dump(void);

/**************************************************************************/
/* FLP service  */
/**************************************************************************/
void FlpService_init(void)
{
    int ret = -1;
    //register EINT handler
    memset(&modem_data, 0, 10 * sizeof(modem_vec_t));
    request_irq(IPC2_IRQn, flp_eint_handler, "IPC2"); // GIPC_IN_2
    flp_eint_queue = xQueueCreate(5, sizeof(UINT32));

    ret = kal_xTaskCreate(flp_eint_task, "FLP", 256, NULL, 2, NULL);

    //register CB in sensormanager
    struct CM4TaskInformationStruct flp_reg_task = {0};
    flp_reg_task.onAccuracyChanged = onSensorAccChange_flp_srv;
    flp_reg_task.onSensorChanged = onSensorChanged_flp_srv;
    flp_handle = SCP_Sensor_Manager_open(&flp_reg_task);
    FLOG("flp_handle = %d, task = %d\n", flp_handle, ret);
    return;
}

void flp_trigger_irq_to_cnn(void)
{
    *(UINT32 *)(SCP_TO_CONN_INT) = 1;
    //FLOG("int read status: %d\n", *(UINT32 *)(SCP_TO_CONN_INT));
}

void flp_clear_irq_from_cnn(void)
{
    *(UINT32 *)(CONN_TO_SCP_INT) = 0;
    //FLOG("int read status: %d\n", *(UINT32 *)(CONN_TO_SCP_INT));
}

UINT32 flp_check_scp2cnn_irq_status(void)
{
    return (*(UINT32 *)(SCP_TO_CONN_INT));
}

static void flp_eint_handler(void)
{
    unsigned char share_buf[TOTAL_CNN_SCP_IPI_SIZE] = {0};
    MTK_FLP_MSG_T *flp_msg = NULL;
    MTK_FLP_MSG_T scp_flp_msg = {0};
#ifdef CFG_GPS_SUPPORT
    gps_input_t gps_data = {{0}};
#endif

    memcpy(share_buf, (void *)CNN_TO_SCP_SHARE_BUFFER, sizeof(share_buf));
    //flp_msg = (MTK_FLP_MSG_T *)&share_buf[4];
    flp_msg = (MTK_FLP_MSG_T *)&share_buf[0];
    FLOG("flp_eint recv interrupt, len =%d, type =%x\n", flp_msg->length, flp_msg->type);
    //FLOG("int buff = %x,%x,%x,%x,%x,%x,%x,%x\n",share_buf[0],share_buf[1],share_buf[2],share_buf[3],share_buf[4],share_buf[5],share_buf[6],share_buf[7]);

    switch (flp_msg->type) {
        case CMD_START_MPE_REQ:
            flp_trigger_eint_task(flp_msg->type);
            //mask_irq(IPC2_IRQn);
            flp_clear_irq_from_cnn();
            scp_flp_msg.length = 0;
            scp_flp_msg.type = CMD_START_MPE_RES;
            memcpy((void *)SCP_TO_CNN_SHARE_BUFFER, &scp_flp_msg, sizeof(scp_flp_msg));
            flp_trigger_irq_to_cnn();
            FLOG("int to CNN 0x%x\n", flp_msg->type);
            break;
        case CMD_FLP_START_SENSOR:
            flp_trigger_eint_task(flp_msg->type);
            //mask_irq(IPC2_IRQn);
            flp_clear_irq_from_cnn();
            scp_flp_msg.length = 0;
            scp_flp_msg.type = CMD_FLP_START_SENSOR_RES;
            memcpy((void *)SCP_TO_CNN_SHARE_BUFFER, &scp_flp_msg, sizeof(scp_flp_msg));
            flp_trigger_irq_to_cnn();
            FLOG("int to CNN 0x%x\n", flp_msg->type);
            break;
        case CMD_FLP_START_CELL:
            flp_trigger_eint_task(flp_msg->type);
            //mask_irq(IPC2_IRQn);
            flp_clear_irq_from_cnn();
#ifdef CFG_CCCI_SUPPORT
            scp_flp_msg.length = 0;
            scp_flp_msg.type = CMD_FLP_START_CELL_RES;
            memcpy((void *)SCP_TO_CNN_SHARE_BUFFER, &scp_flp_msg, sizeof(scp_flp_msg));
            flp_trigger_irq_to_cnn();
            FLOG("int to CNN 0x%x\n", flp_msg->type);
            irq_status_dump();
#endif
            break;
        case CMD_FLP_START_SMD_SENSOR:
            flp_trigger_eint_task(flp_msg->type);
            flp_clear_irq_from_cnn();
            scp_flp_msg.length = 0;
            scp_flp_msg.type = CMD_FLP_START_SMD_SENSOR_RES;
            memcpy((void *)SCP_TO_CNN_SHARE_BUFFER, &scp_flp_msg, sizeof(scp_flp_msg));
            flp_trigger_irq_to_cnn();
            FLOG("int to CNN 0x%x\n", flp_msg->type);
            break;
        case CMD_STOP_MPE_REQ:
        case CMD_FLP_STOP_SENSOR:
        case CMD_FLP_STOP_CELL:
        case CMD_FLP_STOP_SMD_SENSOR:
            flp_trigger_eint_task(flp_msg->type);
            //mask_irq(IPC2_IRQn);
            flp_clear_irq_from_cnn();
            break;
        case CMD_GET_ADR_STATUS_REQ:
#ifdef CFG_GPS_SUPPORT
            memcpy(&gps_data, (UINT8 *)flp_msg + sizeof(MTK_FLP_MSG_T), sizeof(gps_input_t));
            GPS_set_data(&gps_data);
#endif
            //mask_irq(IPC2_IRQn);
            gps_in_cnt = 0;
            if( !mpe_adr_init) {//reset gps_in_cnt if gps continue to send req after mpe auto stop, reinit mpe
                flp_trigger_eint_task(CMD_START_MPE_REQ);
                FLOG("reinit mpe\n");
            }
            flp_clear_irq_from_cnn();
            break;
#ifdef MPE_DYNAMIC_LOGGING
        case CMD_FLP_SCP_LOGGING_CTRL:
            if(flp_msg->length >= sizeof(int)) {
                memcpy(&logging_on,((signed char *)flp_msg) + sizeof(MTK_FLP_MSG_T), sizeof(int));
                FLOG("sensor logging=%d\n",logging_on);
            }
            flp_clear_irq_from_cnn();
            break;
#endif
        default:
            FLOG("unrecognize type %d \n", flp_msg->type);
            //mask_irq(IPC2_IRQn);
            flp_clear_irq_from_cnn();
            break;
    }
}

void onSensorChanged_flp_srv(int sensor, struct data_t *values)
{
    int i;
    struct data_unit_t *data_start = values->data;

    if( (values == NULL) || (values->data_exist_count <1)) {
        FLOG("values error\n");
        return;
    }

    for (i = 0; i < (values->data_exist_count); i++) {
        if ((sensor == SENSOR_TYPE_MODEM_1) || (sensor == SENSOR_TYPE_MODEM_3)) {
            if ((data_start->sensor_type == SENSOR_TYPE_MODEM_1) || (data_start->sensor_type == SENSOR_TYPE_MODEM_3)) {
                memcpy(modem_data+mdm_cnt, &data_start->modem_t, sizeof(modem_vec_t));
                mdm_cnt++;
            } else {
                FLOG("mdm value = null \n");
            }
        } else if (sensor == SENSOR_TYPE_PDR) {
            if ((data_start->sensor_type == SENSOR_TYPE_PDR)) {
                gps_in_cnt++;
                pdr_res.x = data_start->pdr_event.x;
                pdr_res.y = data_start->pdr_event.y;
                pdr_res.z = data_start->pdr_event.z;

                if(flp_pdr_init) {
                    FLOG("PDR = %d, %d, %d \n", pdr_res.x, pdr_res.y, pdr_res.z);
                    if((pdr_res.x != 0) || (pdr_res.y != 0) || (pdr_res.z != 0)) {
                        flp_trigger_eint_task(CMD_SCP_SEND_ADR);
                    }
                    if((gps_in_cnt == (MPE_AUTO_STOP_CNT*5)) && (gps_in_cnt>= 0) && (mpe_adr_init==1)) {
                        FLOG("auto stop mpe, %d\n",gps_in_cnt);
                        flp_trigger_eint_task(CMD_STOP_MPE_REQ);
                    }
                } else {
                    if(gps_in_cnt == MPE_AUTO_STOP_CNT) {
                        FLOG("auto stop mpe\n");
                        flp_trigger_eint_task(CMD_STOP_MPE_REQ);
                    } else if(gps_in_cnt < MPE_AUTO_STOP_CNT) {
                        FLOG("drop PDR = %d, %d, %d \n", pdr_res.x, pdr_res.y, pdr_res.z);
                        //flp_trigger_eint_task(CMD_SCP_SEND_ADR);
                    }
                }
            }
        } else if(sensor == SENSOR_TYPE_SIGNIFICANT_MOTION) {
            if ((data_start->sensor_type == SENSOR_TYPE_SIGNIFICANT_MOTION) && (data_start->smd_t.state)) {
                current_motion = 1;
                FLOG("smd report: %d\n",current_motion);
                if(current_motion != last_motion) {
                    FLOG("smd_change: %d\n",current_motion);
                    last_motion = current_motion;
                    flp_trigger_eint_task(CMD_SCP_SEND_SMD);
                }
            }
        } else if(sensor == SENSOR_TYPE_STATIONARY_GESTURE) {
            if ((data_start->sensor_type == SENSOR_TYPE_STATIONARY_GESTURE) && (data_start->stationary_event.state)) {
                current_motion = 0;
                FLOG("stationary report: %d\n",current_motion);
                if(current_motion != last_motion) {
                    FLOG("stationary_change: %d\n",current_motion);
                    last_motion = current_motion;
                    flp_trigger_eint_task(CMD_SCP_SEND_SMD);
                }
            }
        }
        data_start++;
    }
    if(mdm_cnt != 0) {
        flp_trigger_eint_task(CMD_SCP_SEND_CELL_ID);
    }
    //irq_status_dump();
    return;
}

void onSensorAccChange_flp_srv(int sensor, int accuracy)
{
    //FLOG("mdm acc changed, %d, %d",sensor, accuracy);
}

static void flp_trigger_eint_task(UINT32 event_id)
{
    BaseType_t ret;

    ret = xQueueSendFromISR(flp_eint_queue, &event_id, NULL);
    if (ret != pdPASS) {
        FLOG("flp_trigger_eint_task error %d\n", ret);
    }
}

static void flp_eint_task(void *pvParameters)
{
    UINT32 event;
    BaseType_t queue_recv;
    MTK_FLP_MSG_T flp_msg = {0};
    UINT8 buf[64]={0};
    int status = -1;
    int i, mdm_local_cnt = 0;
    struct CM4TaskRequestStruct flp_config_req = {0};
    struct CM4TaskRequestStruct flp_act_req = {0};

    FLOG("flp_eint_task\n");
    while (1) {
        queue_recv = xQueueReceive(flp_eint_queue, &event, portMAX_DELAY);
        if (queue_recv != pdPASS) {
            FLOG("flp_eint_task xQueueReceive failed\n");
            continue;
        }
        FLOG("flp_eint_task xQueueReceive %d\n", event);
        memset(&buf, 0, sizeof(buf));

        switch (event) {
            case CMD_START_MPE_REQ:
                if (!pdr_sensor_init) {
                    //config gps
                    flp_config_req.handle = flp_handle;
                    flp_config_req.sensor_type = SENSOR_TYPE_PDR;
                    flp_config_req.command = SENSOR_SETDELAY_CMD;
                    flp_config_req.value = 1000; //1s
                    SCP_Sensor_Manager_control(&flp_config_req);

                    //activate
                    flp_act_req.handle = flp_handle;
                    flp_act_req.sensor_type = SENSOR_TYPE_PDR;
                    flp_act_req.command = SENSOR_ACTIVATE_CMD;
                    flp_act_req.value = 1; //enable
                    status = SCP_Sensor_Manager_control(&flp_act_req);
                    if (status == SM_SUCCESS) {
                        FLOG("mpe:pdr start OK\n");
                        pdr_sensor_init = 1;
                        mpe_adr_init = 1;
                    } else {
                        FLOG("mpe:pdr start failed\n");
                    }
                } else {
                    mpe_adr_init = 1;
                    FLOG("pdr is running\n");
                }
                break;
            case CMD_FLP_START_SENSOR:
                if (!pdr_sensor_init) {
                    //config pdr
                    flp_config_req.handle = flp_handle;
                    flp_config_req.sensor_type = SENSOR_TYPE_PDR;
                    flp_config_req.command = SENSOR_SETDELAY_CMD;
                    flp_config_req.value = 200; //5hz
                    SCP_Sensor_Manager_control(&flp_config_req);

                    //activate
                    flp_act_req.handle = flp_handle;
                    flp_act_req.sensor_type = SENSOR_TYPE_PDR;
                    flp_act_req.command = SENSOR_ACTIVATE_CMD;
                    flp_act_req.value = 1;//enable
                    status = SCP_Sensor_Manager_control(&flp_act_req);
                    if (status == SM_SUCCESS) {
                        FLOG("flp:pdr start OK\n");
                        pdr_sensor_init = 1;
                        flp_pdr_init = 1;
                    } else {
                        FLOG("mpe:pdr start failed\n");
                    }
                } else {
                    flp_pdr_init = 1;
                    FLOG("pdr is running\n");
                }
                break;
            case CMD_FLP_START_CELL:
                if (!modem_init) {
                    //activate
                    flp_act_req.handle = flp_handle;
                    flp_act_req.sensor_type = SENSOR_TYPE_MODEM_1;
                    flp_act_req.command = SENSOR_ACTIVATE_CMD;
                    flp_act_req.value = 1; //enable
                    status = SCP_Sensor_Manager_control(&flp_act_req);

                    if (status == SM_SUCCESS) {
                        FLOG("cell start OK\n");
                        modem_init = 1;
                    } else {
                        FLOG("cell start failed\n");
                    }
                } else {
                    FLOG("cell is running\n");
                }

                FLOG("int read status: %d\n", *(UINT32 *)(SCP_TO_CONN_INT));
                //irq_status_dump();
                break;
            case CMD_FLP_START_SMD_SENSOR:
                if(!smd_sensor_int) {
                    //config smd
                    flp_config_req.handle = flp_handle;
                    flp_config_req.sensor_type = SENSOR_TYPE_SIGNIFICANT_MOTION;
                    flp_config_req.command = SENSOR_SETDELAY_CMD;
                    flp_config_req.value = SMD_REG_FREQ; //1hz
                    SCP_Sensor_Manager_control(&flp_config_req);

                    //activate
                    flp_act_req.handle = flp_handle;
                    flp_act_req.sensor_type = SENSOR_TYPE_SIGNIFICANT_MOTION;
                    flp_act_req.command = SENSOR_ACTIVATE_CMD;
                    flp_act_req.value = 1;//enable
                    status = SCP_Sensor_Manager_control(&flp_act_req);

                    if (status == SM_SUCCESS) {
                        FLOG("smd start OK\n");
                        smd_sensor_int = 1;
                    } else {
                        FLOG("smd start failed\n");
                    }
                } else {
                    FLOG("smd already init\n");
                }
                if(!station_sensor_int) {
                    //config smd
                    flp_config_req.handle = flp_handle;
                    flp_config_req.sensor_type = SENSOR_TYPE_STATIONARY_GESTURE;
                    flp_config_req.command = SENSOR_SETDELAY_CMD;
                    flp_config_req.value = SMD_REG_FREQ; //1hz
                    SCP_Sensor_Manager_control(&flp_config_req);

                    //activate
                    flp_act_req.handle = flp_handle;
                    flp_act_req.sensor_type = SENSOR_TYPE_STATIONARY_GESTURE;
                    flp_act_req.command = SENSOR_ACTIVATE_CMD;
                    flp_act_req.value = 1;//enable
                    status = SCP_Sensor_Manager_control(&flp_act_req);

                    if (status == SM_SUCCESS) {
                        FLOG("stationary start OK\n");
                        station_sensor_int = 1;
                    } else {
                        FLOG("stationary start failed\n");
                    }
                } else {
                    FLOG("stationary already init\n");
                }
                break;
            case CMD_SCP_SEND_CELL_ID:
                if (mdm_cnt > 0) {
                    for (i = 0; i < mdm_cnt; i++) {
                        FLOG("mdm data %d,type:%d,mcc:%d,mnc:%d,lac:%d,signal:%d,cellid:%d,iscamping:%d\n", i, \
                             modem_data[i].MDM_TYPE, modem_data[i].MCC, \
                             modem_data[i].NET, modem_data[i].AREA, \
                             modem_data[i].SIGNAL_STRENGTH, modem_data[i].CELL_ID, modem_data[i].isCamping);
                    }

                    if (mdm_cnt <= 2) {
                        flp_msg.length = sizeof(int) + mdm_cnt * sizeof(modem_vec_t);
                        flp_msg.type = CMD_SCP_SEND_CELL_ID;
                        memcpy((UINT8 *)buf, &flp_msg, sizeof(MTK_FLP_MSG_T));
                        memcpy((UINT8 *)buf + sizeof(MTK_FLP_MSG_T), &mdm_cnt, sizeof(int));
                        memcpy((UINT8 *)buf + sizeof(MTK_FLP_MSG_T) + sizeof(int), &modem_data,
                               mdm_cnt * sizeof(modem_vec_t));
                        if (!flp_check_scp2cnn_irq_status()) {
                            memcpy((void *)SCP_TO_CNN_SHARE_BUFFER, buf,
                                   (sizeof(MTK_FLP_MSG_T) + sizeof(int) + mdm_cnt * sizeof(modem_vec_t)));
                            FLOG("send to cnn mdm res\n");
                            flp_trigger_irq_to_cnn();
                        }
                        mdm_cnt = 0;
                    } else {
                        flp_msg.length = sizeof(int) + 2 * sizeof(modem_vec_t);
                        flp_msg.type = CMD_SCP_SEND_CELL_ID;
                        mdm_local_cnt = 2;
                        memcpy((UINT8 *)buf, &flp_msg, sizeof(MTK_FLP_MSG_T));
                        memcpy((UINT8 *)buf + sizeof(MTK_FLP_MSG_T), &mdm_local_cnt, sizeof(modem_vec_t));
                        memcpy((UINT8 *)buf + sizeof(MTK_FLP_MSG_T) + sizeof(int), &modem_data, 2 * sizeof(modem_vec_t));
                        if (!flp_check_scp2cnn_irq_status()) {
                            memcpy((void *)SCP_TO_CNN_SHARE_BUFFER, buf,
                                   (sizeof(MTK_FLP_MSG_T) + sizeof(int) + 2 * sizeof(modem_vec_t)));
                            FLOG("send to cnn mdm res\n");
                            flp_trigger_irq_to_cnn();
                        }
                        mdm_cnt = 0;
                    }
                    memset(&modem_data, 0 , 10*sizeof(modem_vec_t));
                }
                break;
            case CMD_SCP_SEND_ADR:
                flp_msg.length = sizeof(MTK_FLP_PDR_INPUT);
                flp_msg.type = CMD_SCP_SEND_ADR;
                memcpy((UINT8 *)buf, &flp_msg, sizeof(MTK_FLP_MSG_T));
                memcpy((UINT8 *)buf + sizeof(MTK_FLP_MSG_T), &pdr_res, sizeof(MTK_FLP_PDR_INPUT));
                if (!flp_check_scp2cnn_irq_status()) {
                    memcpy((void *)SCP_TO_CNN_SHARE_BUFFER, buf, (sizeof(MTK_FLP_MSG_T) + sizeof(MTK_FLP_PDR_INPUT)));
                    FLOG("send to cnn CMD_SCP_SEND_ADR\n");
                    flp_trigger_irq_to_cnn();
                }
                break;
            case CMD_SCP_SEND_SMD:
                flp_msg.length = sizeof(int);
                flp_msg.type = CMD_SCP_SEND_SMD;
                memcpy((UINT8 *)buf, &flp_msg, sizeof(MTK_FLP_MSG_T));
                memcpy((UINT8 *)buf + sizeof(MTK_FLP_MSG_T), &current_motion, sizeof(int));
                if (!flp_check_scp2cnn_irq_status()) {
                    memcpy((void *)SCP_TO_CNN_SHARE_BUFFER, buf, (sizeof(MTK_FLP_MSG_T) + sizeof(int)));
                    flp_trigger_irq_to_cnn();
                }
                break;
            case CMD_STOP_MPE_REQ:
                if (pdr_sensor_init) {
                    if (!mpe_adr_init) { //incorrect status, ignore
                        FLOG("CMD_STOP_MPE_REQ ignore, mpe_adr_init = %d\n", mpe_adr_init);
                    } else {
                        mpe_adr_init = 0; //disable mpe adr client
                        gps_in_cnt = -1;
                        if (flp_pdr_init) { //pdr client more than one, do not deinit
                        } else { //pdr client =1, disable pdr
                            //deactivate
                            flp_act_req.handle = flp_handle;
                            flp_act_req.sensor_type = SENSOR_TYPE_PDR;
                            flp_act_req.command = SENSOR_ACTIVATE_CMD;
                            flp_act_req.value = 0; //disable
                            SCP_Sensor_Manager_control(&flp_act_req);
                            pdr_sensor_init = 0;
                            FLOG("mpe:PDR stopped OK\n");
                        }
                    }
                } else {
                    FLOG("mpe:PDR stopped\n");
                }
                break;
            case CMD_FLP_STOP_SENSOR:
                if (pdr_sensor_init) {
                    if (!flp_pdr_init) { //incorrect status, ignore
                        FLOG("CMD_FLP_STOP_SENSOR ignore, flp_pdr_init = %d\n", flp_pdr_init);
                    } else {
                        flp_pdr_init = 0; //disable mpe adr client
                        if (mpe_adr_init) { //pdr client more than one, do not deinit
                        } else { //pdr client =1, disable pdr
                            //deactivate
                            flp_act_req.handle = flp_handle;
                            flp_act_req.sensor_type = SENSOR_TYPE_PDR;
                            flp_act_req.command = SENSOR_ACTIVATE_CMD;
                            flp_act_req.value = 0; //disable
                            SCP_Sensor_Manager_control(&flp_act_req);
                            pdr_sensor_init = 0;
                            FLOG("flp:PDR stopped OK\n");
                        }
                    }
                } else {
                    FLOG("flp:PDR stopped\n");
                }
                break;
            case CMD_FLP_STOP_CELL:
                if (modem_init) {
                    //deactivate
                    flp_act_req.handle = flp_handle;
                    flp_act_req.sensor_type = SENSOR_TYPE_MODEM_1;
                    flp_act_req.command = SENSOR_ACTIVATE_CMD;
                    flp_act_req.value = 0; //disable
                    SCP_Sensor_Manager_control(&flp_act_req);
                    modem_init = 0;
                    FLOG("cell stopped OK\n");
                } else {
                    FLOG("cell stopped\n");
                }
                break;
            case CMD_FLP_STOP_SMD_SENSOR:
                if(smd_sensor_int) {
                    //deactivate
                    flp_act_req.handle = flp_handle;
                    flp_act_req.sensor_type = SENSOR_TYPE_SIGNIFICANT_MOTION;
                    flp_act_req.command = SENSOR_ACTIVATE_CMD;
                    flp_act_req.value = 0; //disable
                    SCP_Sensor_Manager_control(&flp_act_req);
                    smd_sensor_int = 0;
                    current_motion = -1;
                    last_motion = -1;
                    FLOG("smd stopped OK\n");
                } else {
                    FLOG("smd stopped\n");
                }
                if(station_sensor_int) {
                    flp_act_req.handle = flp_handle;
                    flp_act_req.sensor_type = SENSOR_TYPE_STATIONARY_GESTURE;
                    flp_act_req.command = SENSOR_ACTIVATE_CMD;
                    flp_act_req.value = 0; //disable
                    SCP_Sensor_Manager_control(&flp_act_req);
                    station_sensor_int = 0;
                    current_motion = -1;
                    last_motion = -1;
                    FLOG("stationary stopped OK\n");
                } else {
                    FLOG("stationary stopped\n");
                }
                break;
            default:
                break;
        }
    }
}

void Flp_get_ADR_state (MNL_location_output_t *adr_out)
{
    MTK_FLP_MSG_T flp_msg = {0};
    UINT8 buf[64] = {0};
    if((adr_out != NULL) && (mpe_adr_init == 1)) {
        flp_msg.length = sizeof(MNL_location_output_t);
        flp_msg.type = CMD_SEND_ADR_STATUS_RES;
        memcpy((UINT8 *)buf, &flp_msg, sizeof(MTK_FLP_MSG_T));
        memcpy((UINT8 *)buf + sizeof(MTK_FLP_MSG_T), adr_out, sizeof(MNL_location_output_t));
        if (!flp_check_scp2cnn_irq_status()) {
            memcpy((void *)SCP_TO_CNN_SHARE_BUFFER, buf, (sizeof(MTK_FLP_MSG_T) + sizeof(MNL_location_output_t)));
            //FLOG("send to cnn CMD_SEND_ADR_STATUS_RES\n");
            flp_trigger_irq_to_cnn();
        }
    }
}

void FlptriggerTest(int mode)
{
    int status = -1;
    //struct CM4TaskRequestStruct flp_config_req= {0};
    struct CM4TaskRequestStruct flp_act_req = {0};

    if (mode == 1) { //init mdm
        //activate
        flp_act_req.handle = flp_handle;
        flp_act_req.sensor_type = SENSOR_TYPE_MODEM_1;
        flp_act_req.command = SENSOR_ACTIVATE_CMD;
        flp_act_req.value = 1; //enable
        status = SCP_Sensor_Manager_control(&flp_act_req);
        FLOG("FlptriggerTest start= %d\n", status);
    } else { //disable mdm
        flp_act_req.handle = flp_handle;
        flp_act_req.sensor_type = SENSOR_TYPE_MODEM_1;
        flp_act_req.command = SENSOR_ACTIVATE_CMD;
        flp_act_req.value = 0; //disable
        status = SCP_Sensor_Manager_control(&flp_act_req);
        FLOG("FlptriggerTest stop= %d\n", status);
    }
}

void FlptriggerPDRTest(int mode)
{
    int status = -1;
    struct CM4TaskRequestStruct flp_config_req = {0};
    struct CM4TaskRequestStruct flp_act_req = {0};

    if (mode == 1) { //init mdm
        flp_config_req.handle = flp_handle;
        flp_config_req.sensor_type = SENSOR_TYPE_PDR;
        flp_config_req.command = SENSOR_SETDELAY_CMD;
        flp_config_req.value = 1000;
        SCP_Sensor_Manager_control(&flp_config_req);

        //activate
        flp_act_req.handle = flp_handle;
        flp_act_req.sensor_type = SENSOR_TYPE_PDR;
        flp_act_req.command = SENSOR_ACTIVATE_CMD;
        flp_act_req.value = 1; //enable
        status = SCP_Sensor_Manager_control(&flp_act_req);
        FLOG("FlptriggerPDRTest start= %d\n", status);
    } else { //disable mdm
        flp_act_req.handle = flp_handle;
        flp_act_req.sensor_type = SENSOR_TYPE_PDR;
        flp_act_req.command = SENSOR_ACTIVATE_CMD;
        flp_act_req.value = 0; //disable
        status = SCP_Sensor_Manager_control(&flp_act_req);
        FLOG("FlptriggerPDRTest stop= %d\n", status);
    }
}

#endif //__FLP_SERVICE_C_INCLUDED__
