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
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */
#include <stdlib.h>
#include <string.h>
#include <float.h>
#include <stdbool.h>
#include <stdint.h>
#include <eventnums.h>
#include <heap.h>
#include <hostIntf.h>
#include <nanohubPacket.h>
#include <mt_reg_base.h>
#include <sensors.h>
#include <seos.h>
#include <timer.h>
#include <util.h>
#include <math.h>
#include <plat/inc/rtc.h>
#include <slab.h>
#include <timers.h>

#include <interrupt.h>
#include <scp_ipi.h>
#include "queue.h"
#include "FreeRTOS.h"
#include "contexthub_flp.h"

#ifdef CFG_CCCI_SUPPORT
#include "ccci.h"
#endif

#define FLP_LOG
#ifdef FLP_LOG
#define FLOG(fmt, args...)    osLog(LOG_INFO, "[FLP]: "fmt, ##args)
#else
#define FLOG(fmt, args...)
#endif

extern unsigned char _SCP_TO_CNN_SHARE_BUFFER_ADDR;
extern unsigned char _CNN_TO_SCP_SHARE_BUFFER_ADDR;
#define SCP_TO_CNN_SHARE_BUFFER     ((volatile unsigned int*)&_SCP_TO_CNN_SHARE_BUFFER_ADDR)
#define CNN_TO_SCP_SHARE_BUFFER     ((volatile unsigned int*)&_CNN_TO_SCP_SHARE_BUFFER_ADDR)

#define EVT_SENSOR_GEOFENCE       sensorGetMyEventType(SENS_TYPE_GEOFENCE)

#define DEFAULT_DEAD_RECKON_RATE_HZ     SENSOR_HZ(200.0f)

static struct SlabAllocator *mDataSlab;
static int mdm1_cnt = 0;
modem_vec_t modem_data1[10];

static TimerHandle_t CnnTimer;
static bool fgGetCnnAck = true;

typedef enum {
    MTK_MDM_DISABLE = 0x0, // default
    MTK_MDM_ENABLE_CHANGE_OF_CELL_ID, // enable phase I: only report when camping cell ID has change
    MTK_MDM_ENABLE_CHANGE_OF_CELL_RSSI, // enable phase II: report under phase I + report when camping cell’s RSSI has change
    MTK_MDM_CMD_END
} MTK_MDM_CMD;

enum ContextHubFlpEvents {
    EVT_IPC_RX = EVT_APP_START + 1,
    EVT_MD_RX,
};

struct flpTask {
    uint32_t id;
    uint32_t DeadReckonHandle;
    uint32_t GeofenceHandle;
};

static struct flpTask mTask;

static void contextHubFlpTriggerIrqToCnn(void)
{
    SCP_TO_CONN_REG = 1;
    //FLOG("int read status: %d\n", SCP_TO_CONN_REG);
}

static unsigned int contextHubFlpCheckScp2CnnIrqStatus(void)
{
    return SCP_TO_CONN_REG;
}

static void dataEvtFree(void *ptr)
{
    slabAllocatorFree(mDataSlab, ptr);
}

static void contextHubFlpConfigGeofence(bool on)
{
    uint32_t i;
    if ((on == true) && (mTask.GeofenceHandle == 0)) {
        for (i = 0; sensorFind(SENS_TYPE_GEOFENCE, i, &mTask.GeofenceHandle) != NULL; i++) {
            if (sensorRequest(mTask.id, mTask.GeofenceHandle, SENSOR_RATE_ONCHANGE, 0)) {
                osEventSubscribe(mTask.id, EVT_SENSOR_GEOFENCE);
                if (CnnTimer != NULL) {
                    if (xTimerStart(CnnTimer, 0) == pdPASS ) {
                        FLOG("start cnn timer\n");
                    }
                }
                break;
            }
        }
        if (sensorFind(SENS_TYPE_GEOFENCE, i, &mTask.GeofenceHandle) == NULL) {
            FLOG("geofence sensor N.A\n");
        }
    } else if ((on == false) && (mTask.GeofenceHandle != 0)) {
        sensorRelease(mTask.id, mTask.GeofenceHandle);
        mTask.GeofenceHandle = 0;
        osEventUnsubscribe(mTask.id, EVT_SENSOR_GEOFENCE);
        if (CnnTimer != NULL) {
            FLOG("stop cnn timer\n");
            xTimerStop(CnnTimer, 0);
        }
    }
}

static void contextHubFlpSendCellId()
{
    MTK_FLP_MSG_T flp_msg = {0};
    int i, mdm_local_cnt = 0;
    unsigned char buf[TOTAL_CNN_SCP_IPI_SIZE]= {0};

    if (mdm1_cnt > 0) {
        for (i = 0; i < mdm1_cnt; i++) {
            FLOG("mdm data %d,type:%d,mcc:%d,mnc:%d,lac:%d,signal:%d,cellid:%d,iscamping:%d\n", i, \
                 modem_data1[i].MDM_TYPE, modem_data1[i].MCC, \
                 modem_data1[i].NET, modem_data1[i].AREA, \
                 modem_data1[i].SIGNAL_STRENGTH, modem_data1[i].CELL_ID, modem_data1[i].isCamping);
        }

        if (mdm1_cnt <= 2) {
            flp_msg.length = sizeof(int) + mdm1_cnt * sizeof(modem_vec_t);
            flp_msg.type = CMD_SCP_SEND_CELL_ID;
            memcpy((unsigned char *)buf, &flp_msg, sizeof(MTK_FLP_MSG_T));
            memcpy((unsigned char *)buf + sizeof(MTK_FLP_MSG_T), &mdm1_cnt, sizeof(int));
            memcpy((unsigned char *)buf + sizeof(MTK_FLP_MSG_T) + sizeof(int), &modem_data1,
                   mdm1_cnt * sizeof(modem_vec_t));
            if (!contextHubFlpCheckScp2CnnIrqStatus()) {
                memcpy((void *)SCP_TO_CNN_SHARE_BUFFER, buf,
                       (sizeof(MTK_FLP_MSG_T) + sizeof(int) + mdm1_cnt * sizeof(modem_vec_t)));
                FLOG("send to cnn mdm res\n");
                contextHubFlpTriggerIrqToCnn();
            }
            mdm1_cnt = 0;
        } else {
            flp_msg.length = sizeof(int) + 2 * sizeof(modem_vec_t);
            flp_msg.type = CMD_SCP_SEND_CELL_ID;
            mdm_local_cnt = 2;
            memcpy((unsigned char *)buf, &flp_msg, sizeof(MTK_FLP_MSG_T));
            memcpy((unsigned char *)buf + sizeof(MTK_FLP_MSG_T), &mdm_local_cnt, sizeof(modem_vec_t));
            memcpy((unsigned char *)buf + sizeof(MTK_FLP_MSG_T) + sizeof(int), &modem_data1, 2 * sizeof(modem_vec_t));
            if (!contextHubFlpCheckScp2CnnIrqStatus()) {
                memcpy((void *)SCP_TO_CNN_SHARE_BUFFER, buf,
                       (sizeof(MTK_FLP_MSG_T) + sizeof(int) + 2 * sizeof(modem_vec_t)));
                FLOG("send to cnn mdm res\n");
                contextHubFlpTriggerIrqToCnn();
            }
            mdm1_cnt = 0;
        }
        memset(&modem_data1, 0 , 10*sizeof(modem_vec_t));
    }

}

static void contextHubFlpSendGeofence(const void* evtData)
{
    MTK_FLP_MSG_T flp_msg = {0};
    unsigned char buf[TOTAL_CNN_SCP_IPI_SIZE] = {0};
    union EmbeddedDataPoint geofence_data;

    geofence_data.idata = (uint32_t)evtData;
    FLOG("Geofence source: %u state: %u op: %u\n",
            ((geofence_data.idata>>8)&0xf), ((geofence_data.idata>>4)&0xf), ((geofence_data.idata)&0xf));
    flp_msg.type = CMD_SCP_SEND_SMD;
    flp_msg.length = sizeof(uint32_t);
    memcpy((unsigned char *)buf, &flp_msg, sizeof(MTK_FLP_MSG_T));
    memcpy((unsigned char *)buf + sizeof(MTK_FLP_MSG_T), &(geofence_data.idata), sizeof(uint32_t));
    memcpy((void *)SCP_TO_CNN_SHARE_BUFFER, buf, (sizeof(MTK_FLP_MSG_T) + sizeof(uint32_t)));
    FLOG("send to cnn CMD_SCP_SEND_SMD\n");
    contextHubFlpTriggerIrqToCnn();
}

static void contextHubFlpIpcRxHandler(const void* evtData)
{
    MTK_FLP_MSG_T *flp_msg = NULL;
    MTK_FLP_MSG_T scp_flp_msg = {0};
    bool on = true;

    flp_msg = (MTK_FLP_MSG_T *)evtData;
    FLOG("flp_eint recv interrupt, len =%d, type =%x\n", flp_msg->length, flp_msg->type);

    fgGetCnnAck = true;
    switch (flp_msg->type) {
        case CMD_FLP_START_SMD_SENSOR:
            on = true;
            contextHubFlpConfigGeofence(on);
            scp_flp_msg.length = 0;
            scp_flp_msg.type = CMD_FLP_START_SMD_SENSOR_RES;
            memcpy((void *)SCP_TO_CNN_SHARE_BUFFER, &scp_flp_msg, sizeof(scp_flp_msg));
            contextHubFlpTriggerIrqToCnn();
            FLOG("int to CNN 0x%x\n", flp_msg->type);
            break;
        case CMD_FLP_STOP_SMD_SENSOR:
            on = false;
            contextHubFlpConfigGeofence(on);
            break;
        case CMD_FLP_SEND_ACK:
            break;
        default:
            FLOG("unrecognize type %d \n", flp_msg->type);
            break;
    }
}

static void contextHubFlpHandleEvent(uint32_t evtType, const void* evtData)
{
    if (evtData == SENSOR_DATA_EVENT_FLUSH)
        return;

    FLOG("FLP handle event type %u\n", evtType);
    switch (evtType) {
        case EVT_IPC_RX:
            contextHubFlpIpcRxHandler(evtData);
            break;
        case EVT_MD_RX:
            contextHubFlpSendCellId();
            break;
        case EVT_SENSOR_GEOFENCE:
            contextHubFlpSendGeofence(evtData);
            break;
    }
}

void contextHubFlpIpcHandler()
{
    unsigned int *share_buf;
    if (mDataSlab == NULL) {
        FLOG("FLP mDataSlab NULL\n");
    } else {
        share_buf = slabAllocatorAlloc(mDataSlab);
        if (share_buf == NULL) {
            FLOG("FLP SLAB ALLOCATION FAILED\n");
        } else {
            memcpy(share_buf, (void *)CNN_TO_SCP_SHARE_BUFFER, TOTAL_CNN_SCP_IPI_SIZE);
            osEnqueuePrivateEvt(EVT_IPC_RX, (void *)share_buf, dataEvtFree, mTask.id);
            extern TaskHandle_t CHRE_TaskHandle;
            if (xTaskResumeFromISR(CHRE_TaskHandle) == pdTRUE) {
                portYIELD_WITHIN_API();
            }
        }
    }
}

static void vCnnTimerCallback(TimerHandle_t xTimer)
{
    MTK_FLP_MSG_T flp_msg = {0};
    unsigned int *share_buf;
    unsigned char buf[TOTAL_CNN_SCP_IPI_SIZE] = {0};

    FLOG("FLP Timer Expire %d\n", fgGetCnnAck);
    if (fgGetCnnAck == true) {
        flp_msg.type = CMD_SCP_SEND_SYN;
        flp_msg.length = 0;
        memcpy((unsigned char *)buf, &flp_msg, sizeof(MTK_FLP_MSG_T));
        memcpy((void *)SCP_TO_CNN_SHARE_BUFFER, buf, sizeof(MTK_FLP_MSG_T));
        FLOG("send to cnn CMD_SCP_SYN\n");
        contextHubFlpTriggerIrqToCnn();
        fgGetCnnAck = false;
    } else {
        flp_msg.type = CMD_FLP_STOP_SMD_SENSOR;
        flp_msg.length = 0;
        FLOG("send to flp CMD_STOP_SMD\n");
        if (mDataSlab == NULL) {
            FLOG("FLP mDataSlab NULL\n");
        } else {
            share_buf = slabAllocatorAlloc(mDataSlab);
            if (share_buf == NULL) {
                FLOG("FLP SLAB ALLOCATION FAILED\n");
            } else {
                memcpy(share_buf, &flp_msg, sizeof(MTK_FLP_MSG_T));
                osEnqueuePrivateEvt(EVT_IPC_RX, (void *)share_buf, dataEvtFree, mTask.id);
            }
        }
    }
}

static bool contextHubFlpStart(uint32_t tid)
{
    mTask.id = tid;
    FLOG("FLP CHRE Task start %u\n", tid);
    request_ipc(IPC4, contextHubFlpIpcHandler, "FLP"); // from CNN

    mDataSlab = slabAllocatorNew(TOTAL_CNN_SCP_IPI_SIZE, 4, 4);
    if (!mDataSlab) {
        FLOG("FLP SLAB New FAILED\n");
        return false;
    }

    CnnTimer = xTimerCreate("CnnTimer", pdMS_TO_TICKS(60000), pdTRUE, (void * )0, vCnnTimerCallback);

    return true;
}

static void contextHubFlpEnd(void)
{
    slabAllocatorDestroy(mDataSlab);
}

INTERNAL_APP_INIT(
    APP_ID_MAKE(APP_ID_VENDOR_MTK, MTK_APP_ID_WRAP(SENS_TYPE_INVALID, 0, 1)),
    0,
    contextHubFlpStart,
    contextHubFlpEnd,
    contextHubFlpHandleEvent);

