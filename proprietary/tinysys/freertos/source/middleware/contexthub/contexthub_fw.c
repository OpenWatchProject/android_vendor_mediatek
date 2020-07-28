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
#include <sensors.h>
#include <seos.h>
#include <timer.h>
#include <util.h>
#include <math.h>
#include <plat/inc/rtc.h>
#include <cpu.h>
#include <atomic.h>
#include "scp_sem.h"
#include "dma.h"
#ifdef CFG_VCORE_DVFS_SUPPORT
#include <dvfs.h>
#endif
#include <mt_gpt.h>
#include "scp_ipi.h"

#include "contexthub_fw.h"
#include "contexthub_core.h"
#include <wakelock.h>
#include <performance.h>

#define MAX_IPI_EVENT_QUEUE_DEEPTH 16

#ifdef CFG_CHRE_256KB_SUPPORT
#define MAX_SRAM_FIFO_DEEPTH 64
#else
#define MAX_SRAM_FIFO_DEEPTH 512
#endif

#define MAX_ACCESS_DRAM_RETRY_TIMES 100
#define CONTEXTHUB_CMD(_reason, _handler, _handlerAck) \
    { .reason = _reason, .handler = _handler, .handlerAck = _handlerAck}

struct ContextHubCmd {
    uint32_t reason;
    int (*handler)(SCP_SENSOR_HUB_REQ *req, struct data_unit_t *data);
    int (*handlerAck)(SCP_SENSOR_HUB_REQ *req, SCP_SENSOR_HUB_DATA *rsp,
        struct data_unit_t *data, int errCode);
};
enum ContextHubState {
    STATE_STOP = 0,
    STATE_START,
};
static struct ContextHubTask {
    uint32_t id;
    SCP_SENSOR_HUB_REQ ipi_req;
} mTask;
/* ipi transfer event queue, when ipi transfer fail, start a timer after timer timeout we will retry this event */
static struct ContextHubIpi {
    uint8_t head;
    uint8_t tail;
    uint8_t size;
    uint32_t timerHandle;
    uint8_t isTimerRunning;
    wakelock_t wakeLock;
    SCP_SENSOR_HUB_DATA ringBuf[MAX_IPI_EVENT_QUEUE_DEEPTH];
} mContextHubIpi;
/* data transfer fifo, this fifo is used for low power, we should put data into this fifo first, when this fifo full or
 * receive flush this fifo cmd, then push data to dram, if we don't use this fifo, and directly access dram, will lead
 * low power performance worse.
 */
static struct ContextHubSramFifo {
    uint32_t head;
    uint32_t tail;
    uint32_t size;
    struct data_unit_t ringBuf[MAX_SRAM_FIFO_DEEPTH];
} mContextHubSramFifo;
/* data transfer fifo, this fifo is used for transfer data from dram to ap
 */
static struct ContextHubDramFifo {
    struct sensorFIFO *bufferBase;
    uint32_t maxEventNumber;
    uint32_t currEventNumber;
    uint32_t lastWpPointer;
    uint32_t timerHandle;
    uint8_t isDramEnabling;
    wakelock_t wakeLock;
} mContextHubDramFifo;

enum DownSamplingSensorType {
    ACC,
    GYRO,
    MAX_SENSOR_DOWN_SAMPLE,
};
struct DownSample {
    uint8_t count;
    uint8_t dropDiv;
    uint32_t hwDelay;
    uint32_t requestDelay;
};
static struct DownSampling {
    struct DownSample sample[MAX_SENSOR_DOWN_SAMPLE];
} mDownSampling;
enum ContextHubFwEvents {
    EVT_IPI_RX = EVT_APP_START + 1,
    EVT_IPI_TX,
    EVT_DRAM,
};
enum reportMode {
    continues,
    onChange,
    oneShot,
};
enum AccurancyMode {
    SENSOR_STATUS_UNRELIABLE,
    SENSOR_STATUS_ACCURACY_LOW,
    SENSOR_STATUS_ACCURACY_MEDIUM,
    SENSOR_STATUS_ACCURACY_HIGH,
};
uint32_t apGetStepCounter;
uint32_t apGetFloorCounter;
static uint8_t sensorReportingMode[SENSOR_TYPE_MAX];
static uint8_t sensorAccurancyMode[SENSOR_TYPE_MAX];
static float accBias[3];
static float magBias[3];
float gyroBias[3];
float accTemp;
int temp_status;
const struct ContextHubCmd *contextHubFindCmd(uint32_t packetReason);
/* arch counter is 13M, mult is 161319385, shift is 21 */
static inline uint64_t arch_counter_to_ns(uint64_t cyc)
{
#define ARCH_TIMER_MULT 161319385
#define ARCH_TIMER_SHIFT 21
    return (cyc * ARCH_TIMER_MULT) >> ARCH_TIMER_SHIFT;
}

#define apIdToMtkType(sensortype) (sensortype + 1)
#define mtkTypeToApId(sensortype) (sensortype - 1)
uint8_t chreTypeToMtkType(uint8_t sensortype)
{
    switch (sensortype) {
        case SENS_TYPE_ACCEL:
            return SENSOR_TYPE_ACCELEROMETER;
        case SENS_TYPE_GYRO:
            return SENSOR_TYPE_GYROSCOPE;
        case SENS_TYPE_GYRO_UNCAL:
            return SENSOR_TYPE_GYROSCOPE_UNCALIBRATED;
        case SENS_TYPE_MAG:
            return SENSOR_TYPE_MAGNETIC_FIELD;
        case SENS_TYPE_MAG_UNCAL:
            return SENSOR_TYPE_MAGNETIC_FIELD_UNCALIBRATED;
        case SENS_TYPE_BARO:
            return SENSOR_TYPE_PRESSURE;
        case SENS_TYPE_ALS:
            return SENSOR_TYPE_LIGHT;
        case SENS_TYPE_PROX:
            return SENSOR_TYPE_PROXIMITY;
        case SENS_TYPE_ROTATION_VECTOR:
            return SENSOR_TYPE_ROTATION_VECTOR;
        case SENS_TYPE_GAME_ROT_VECTOR:
            return SENSOR_TYPE_GAME_ROTATION_VECTOR;
        case SENS_TYPE_GEO_MAG_ROT_VEC:
            return SENSOR_TYPE_GEOMAGNETIC_ROTATION_VECTOR;
        case SENS_TYPE_GRAVITY:
            return SENSOR_TYPE_GRAVITY;
        case SENS_TYPE_LINEAR_ACCEL:
            return SENSOR_TYPE_LINEAR_ACCELERATION;
        case SENS_TYPE_ORIENTATION:
            return SENSOR_TYPE_ORIENTATION;
        case SENS_TYPE_STEP_COUNT:
            return SENSOR_TYPE_STEP_COUNTER;
        case SENS_TYPE_STEP_DETECT:
            return SENSOR_TYPE_STEP_DETECTOR;
        case SENS_TYPE_SIG_MOTION:
            return SENSOR_TYPE_SIGNIFICANT_MOTION;
        case SENS_TYPE_TILT:
            return SENSOR_TYPE_TILT_DETECTOR;
        case SENS_TYPE_SHAKE:
            return SENSOR_TYPE_SHAKE;
        case SENS_TYPE_PICK_UP:
            return SENSOR_TYPE_PICK_UP_GESTURE;
        case SENS_TYPE_GLANCE:
            return SENSOR_TYPE_GLANCE_GESTURE;
        case SENS_TYPE_ANSWER_CALL:
            return SENSOR_TYPE_ANSWER_CALL;
        case SENS_TYPE_STATIONARY_DETECT:
            return SENSOR_TYPE_STATIONARY_DETECT;
        case SENS_TYPE_MOTION_DETECT:
            return SENSOR_TYPE_MOTION_DETECT;
        case SENS_TYPE_ACTIVITY:
            return SENSOR_TYPE_ACTIVITY;
        case SENS_TYPE_WIN_ORIENTATION:
            return SENSOR_TYPE_DEVICE_ORIENTATION;
        case SENS_TYPE_WAKE_UP:
            return SENSOR_TYPE_WAKE_GESTURE;
        case SENS_TYPE_GEOFENCE:
            return SENSOR_TYPE_GEOFENCE;
        case SENS_TYPE_INPOCKET:
            return SENSOR_TYPE_INPOCKET;
        case SENS_TYPE_FLOOR_COUNT:
            return SENSOR_TYPE_FLOOR_COUNTER;
        case SENS_TYPE_FLAT:
            return SENSOR_TYPE_FLAT;
        case SENS_TYPE_RGBW:
            return SENSOR_TYPE_RGBW;
        case SENS_TYPE_SAR:
            return SENSOR_TYPE_SAR;
    }
    return 0;
}
uint8_t mtkTypeToChreType(uint8_t sensortype)
{
    switch (sensortype) {
        case SENSOR_TYPE_ACCELEROMETER:
            return SENS_TYPE_ACCEL;
        case SENSOR_TYPE_GYROSCOPE:
            return SENS_TYPE_GYRO;
        case SENSOR_TYPE_GYROSCOPE_UNCALIBRATED:
            return SENS_TYPE_GYRO_UNCAL;
        case SENSOR_TYPE_MAGNETIC_FIELD:
            return SENS_TYPE_MAG;
        case SENSOR_TYPE_MAGNETIC_FIELD_UNCALIBRATED:
            return SENS_TYPE_MAG_UNCAL;
        case SENSOR_TYPE_PRESSURE:
            return SENS_TYPE_BARO;
        case SENSOR_TYPE_LIGHT:
            return SENS_TYPE_ALS;
        case SENSOR_TYPE_PROXIMITY:
            return SENS_TYPE_PROX;
        case SENSOR_TYPE_ROTATION_VECTOR:
            return SENS_TYPE_ROTATION_VECTOR;
        case SENSOR_TYPE_GAME_ROTATION_VECTOR:
            return SENS_TYPE_GAME_ROT_VECTOR;
        case SENSOR_TYPE_GEOMAGNETIC_ROTATION_VECTOR:
            return SENS_TYPE_GEO_MAG_ROT_VEC;
        case SENSOR_TYPE_GRAVITY:
            return SENS_TYPE_GRAVITY;
        case SENSOR_TYPE_LINEAR_ACCELERATION:
            return SENS_TYPE_LINEAR_ACCEL;
        case SENSOR_TYPE_ORIENTATION:
            return SENS_TYPE_ORIENTATION;
        case SENSOR_TYPE_STEP_COUNTER:
            return SENS_TYPE_STEP_COUNT;
        case SENSOR_TYPE_STEP_DETECTOR:
            return SENS_TYPE_STEP_DETECT;
        case SENSOR_TYPE_SIGNIFICANT_MOTION:
            return SENS_TYPE_SIG_MOTION;
        case SENSOR_TYPE_TILT_DETECTOR:
            return SENS_TYPE_TILT;
        case SENSOR_TYPE_SHAKE:
            return SENS_TYPE_SHAKE;
        case SENSOR_TYPE_PICK_UP_GESTURE:
            return SENS_TYPE_PICK_UP;
        case SENSOR_TYPE_GLANCE_GESTURE:
            return SENS_TYPE_GLANCE;
        case SENSOR_TYPE_ANSWER_CALL:
            return SENS_TYPE_ANSWER_CALL;
        case SENSOR_TYPE_STATIONARY_DETECT:
            return SENS_TYPE_STATIONARY_DETECT;
        case SENSOR_TYPE_MOTION_DETECT:
            return SENS_TYPE_MOTION_DETECT;
        case SENSOR_TYPE_ACTIVITY:
            return SENS_TYPE_ACTIVITY;
        case SENSOR_TYPE_DEVICE_ORIENTATION:
            return SENS_TYPE_WIN_ORIENTATION;
        case SENSOR_TYPE_WAKE_GESTURE:
            return SENS_TYPE_WAKE_UP;
        case SENSOR_TYPE_GEOFENCE:
            return SENS_TYPE_GEOFENCE;
        case SENSOR_TYPE_INPOCKET:
            return SENS_TYPE_INPOCKET;
        case SENSOR_TYPE_FLOOR_COUNTER:
            return SENS_TYPE_FLOOR_COUNT;
        case SENSOR_TYPE_FLAT:
            return SENS_TYPE_FLAT;
        case SENSOR_TYPE_RGBW:
            return SENS_TYPE_RGBW;
        case SENSOR_TYPE_SAR:
            return SENS_TYPE_SAR;
    }
    return 0;
}
static void initSensorReportingInfo(void)
{
    sensorReportingMode[SENSOR_TYPE_ACCELEROMETER] = continues;
    sensorAccurancyMode[SENSOR_TYPE_ACCELEROMETER] = SENSOR_STATUS_UNRELIABLE;

    sensorReportingMode[SENSOR_TYPE_MAGNETIC_FIELD] = continues;
    sensorAccurancyMode[SENSOR_TYPE_MAGNETIC_FIELD] = SENSOR_STATUS_UNRELIABLE;

    sensorReportingMode[SENSOR_TYPE_ORIENTATION] = continues;
    sensorAccurancyMode[SENSOR_TYPE_ORIENTATION] = SENSOR_STATUS_UNRELIABLE;

    sensorReportingMode[SENSOR_TYPE_GYROSCOPE] = continues;
    sensorAccurancyMode[SENSOR_TYPE_GYROSCOPE] = SENSOR_STATUS_UNRELIABLE;

    sensorReportingMode[SENSOR_TYPE_LIGHT] = onChange;
    sensorAccurancyMode[SENSOR_TYPE_LIGHT] = SENSOR_STATUS_ACCURACY_HIGH;

    sensorReportingMode[SENSOR_TYPE_PRESSURE] = continues;
    sensorAccurancyMode[SENSOR_TYPE_PRESSURE] = SENSOR_STATUS_ACCURACY_HIGH;

    sensorReportingMode[SENSOR_TYPE_PROXIMITY] = onChange;
    sensorAccurancyMode[SENSOR_TYPE_PROXIMITY] = SENSOR_STATUS_ACCURACY_HIGH;

    sensorReportingMode[SENSOR_TYPE_GRAVITY] = continues;
    sensorAccurancyMode[SENSOR_TYPE_GRAVITY] = SENSOR_STATUS_UNRELIABLE;

    sensorReportingMode[SENSOR_TYPE_LINEAR_ACCELERATION] = continues;
    sensorAccurancyMode[SENSOR_TYPE_LINEAR_ACCELERATION] = SENSOR_STATUS_UNRELIABLE;

    sensorReportingMode[SENSOR_TYPE_ROTATION_VECTOR] = continues;
    sensorAccurancyMode[SENSOR_TYPE_ROTATION_VECTOR] = SENSOR_STATUS_UNRELIABLE;

    sensorReportingMode[SENSOR_TYPE_GAME_ROTATION_VECTOR] = continues;
    sensorAccurancyMode[SENSOR_TYPE_GAME_ROTATION_VECTOR] = SENSOR_STATUS_UNRELIABLE;

    sensorReportingMode[SENSOR_TYPE_GEOMAGNETIC_ROTATION_VECTOR] = continues;
    sensorAccurancyMode[SENSOR_TYPE_GEOMAGNETIC_ROTATION_VECTOR] = SENSOR_STATUS_UNRELIABLE;

    sensorReportingMode[SENSOR_TYPE_MAGNETIC_FIELD_UNCALIBRATED] = continues;
    sensorAccurancyMode[SENSOR_TYPE_MAGNETIC_FIELD_UNCALIBRATED] = SENSOR_STATUS_UNRELIABLE;

    sensorReportingMode[SENSOR_TYPE_GYROSCOPE_UNCALIBRATED] = continues;
    sensorAccurancyMode[SENSOR_TYPE_GYROSCOPE_UNCALIBRATED] = SENSOR_STATUS_UNRELIABLE;

    sensorReportingMode[SENSOR_TYPE_SIGNIFICANT_MOTION] = oneShot;
    sensorAccurancyMode[SENSOR_TYPE_SIGNIFICANT_MOTION] = SENSOR_STATUS_ACCURACY_HIGH;

    sensorReportingMode[SENSOR_TYPE_STEP_DETECTOR] = onChange;
    sensorAccurancyMode[SENSOR_TYPE_STEP_DETECTOR] = SENSOR_STATUS_ACCURACY_HIGH;

    sensorReportingMode[SENSOR_TYPE_STEP_COUNTER] = onChange;
    sensorAccurancyMode[SENSOR_TYPE_STEP_COUNTER] = SENSOR_STATUS_ACCURACY_HIGH;

    sensorReportingMode[SENSOR_TYPE_TILT_DETECTOR] = onChange;
    sensorAccurancyMode[SENSOR_TYPE_TILT_DETECTOR] = SENSOR_STATUS_ACCURACY_HIGH;

    sensorReportingMode[SENSOR_TYPE_ACTIVITY] = continues;
    sensorAccurancyMode[SENSOR_TYPE_ACTIVITY] = SENSOR_STATUS_ACCURACY_HIGH;

    sensorReportingMode[SENSOR_TYPE_ANSWER_CALL] = oneShot;
    sensorAccurancyMode[SENSOR_TYPE_ANSWER_CALL] = SENSOR_STATUS_ACCURACY_HIGH;

    sensorReportingMode[SENSOR_TYPE_SHAKE] = oneShot;
    sensorAccurancyMode[SENSOR_TYPE_SHAKE] = SENSOR_STATUS_ACCURACY_HIGH;

    sensorReportingMode[SENSOR_TYPE_PICK_UP_GESTURE] = oneShot;
    sensorAccurancyMode[SENSOR_TYPE_PICK_UP_GESTURE] = SENSOR_STATUS_ACCURACY_HIGH;

    sensorReportingMode[SENSOR_TYPE_GLANCE_GESTURE] = oneShot;
    sensorAccurancyMode[SENSOR_TYPE_GLANCE_GESTURE] = SENSOR_STATUS_ACCURACY_HIGH;

    sensorReportingMode[SENSOR_TYPE_STATIONARY_DETECT] = oneShot;
    sensorAccurancyMode[SENSOR_TYPE_STATIONARY_DETECT] = SENSOR_STATUS_ACCURACY_HIGH;

    sensorReportingMode[SENSOR_TYPE_MOTION_DETECT] = oneShot;
    sensorAccurancyMode[SENSOR_TYPE_MOTION_DETECT] = SENSOR_STATUS_ACCURACY_HIGH;

    sensorReportingMode[SENSOR_TYPE_DEVICE_ORIENTATION] = onChange;
    sensorAccurancyMode[SENSOR_TYPE_DEVICE_ORIENTATION] = SENSOR_STATUS_ACCURACY_HIGH;

    sensorReportingMode[SENSOR_TYPE_WAKE_GESTURE] = oneShot;
    sensorAccurancyMode[SENSOR_TYPE_WAKE_GESTURE] = SENSOR_STATUS_ACCURACY_HIGH;

    sensorReportingMode[SENSOR_TYPE_GEOFENCE] = onChange;
    sensorAccurancyMode[SENSOR_TYPE_GEOFENCE] = SENSOR_STATUS_ACCURACY_HIGH;

    sensorReportingMode[SENSOR_TYPE_INPOCKET] = oneShot;
    sensorAccurancyMode[SENSOR_TYPE_INPOCKET] = SENSOR_STATUS_ACCURACY_HIGH;

    sensorReportingMode[SENSOR_TYPE_FLOOR_COUNTER] = onChange;
    sensorAccurancyMode[SENSOR_TYPE_FLOOR_COUNTER] = SENSOR_STATUS_ACCURACY_HIGH;

    sensorReportingMode[SENSOR_TYPE_FLAT] = oneShot;
    sensorAccurancyMode[SENSOR_TYPE_FLAT] = SENSOR_STATUS_ACCURACY_HIGH;

    sensorReportingMode[SENSOR_TYPE_RGBW] = onChange;
    sensorAccurancyMode[SENSOR_TYPE_RGBW] = SENSOR_STATUS_ACCURACY_HIGH;

    sensorReportingMode[SENSOR_TYPE_SAR] = onChange;
    sensorAccurancyMode[SENSOR_TYPE_SAR] = SENSOR_STATUS_ACCURACY_HIGH;
}
static int mtkTypeToDownSample(int mtkType)
{
    int ret = -1;

    switch (mtkType) {
        case SENSOR_TYPE_ACCELEROMETER:
            ret = ACC;
            break;
        case SENSOR_TYPE_GYROSCOPE:
            ret = GYRO;
            break;
        default:
            ret = -1;
            break;
    }
    return ret;
}
static bool estimateDownSample(uint8_t mtk_type)
{
    int downSampleType = -1;
    bool needDownSample = false;
    struct DownSample *sample = NULL;

    downSampleType = mtkTypeToDownSample(mtk_type);
    if (downSampleType < 0)
        return false;
    sample = &mDownSampling.sample[downSampleType];
    if (!sample->dropDiv) {
        sample->count = 0;
        return false;
    }
    if (++sample->count / sample->dropDiv) {
        sample->count = 0;
        needDownSample = false;
    } else {
        needDownSample = true;
    }
    /*osLog(LOG_ERROR, "needDownSample=%d\n", needDownSample);*/
    return needDownSample;
}
static void calculateDownSample(int toDo)
{
    struct DownSample *sample = &mDownSampling.sample[toDo];

    sample->count = 0;
    sample->dropDiv = sample->hwDelay ?
        (sample->requestDelay / sample->hwDelay) : 0;
}
void registerDownSampleInfo(int mtkType, uint32_t rate)
{
    int toDo = -1;
    struct DownSample *sample = NULL;

    toDo = mtkTypeToDownSample(mtkType);
    if (toDo < 0)
        return;
    sample = &mDownSampling.sample[toDo];
    sample->requestDelay = (1024000000000ULL / rate);
    calculateDownSample(toDo);
    /*osLog(LOG_ERROR, "request toDo=%d,requestDelay=%d,hwDelay=%d,dropDiv=%d\n",
        toDo, sample->requestDelay, sample->hwDelay, sample->dropDiv);*/
}
void registerHwSampleInfo(int chreType, uint32_t hwDelay)
{
    int toDo = -1;
    struct DownSample *sample = NULL;

    toDo = mtkTypeToDownSample(chreTypeToMtkType(chreType));
    if (toDo < 0)
        return;
    sample = &mDownSampling.sample[toDo];
    sample->hwDelay = hwDelay;
    calculateDownSample(toDo);
    /*osLog(LOG_ERROR, "request toDo=%d,requestDelay=%d,hwDelay=%d,dropDiv=%d\n",
        toDo, sample->requestDelay, sample->hwDelay, sample->dropDiv);*/
}
static void contextHubIpiInit(void)
{
    uint8_t i = 0;

    mContextHubIpi.head = mContextHubIpi.tail = 0;
    mContextHubIpi.size = MAX_IPI_EVENT_QUEUE_DEEPTH;
    mContextHubIpi.timerHandle = 0;
    atomicWriteByte(&mContextHubIpi.isTimerRunning, STATE_STOP);
    wake_lock_init(&mContextHubIpi.wakeLock, "chre_ipi");
    for (; i < mContextHubIpi.size; ++i)
        memset(&mContextHubIpi.ringBuf[i], 0, sizeof(SCP_SENSOR_HUB_DATA));
}
static int contextHubIpiTx(SCP_SENSOR_HUB_DATA *ipiData)
{
    //osLog(LOG_DEBUG, "contextHubIpiTx\n");
    mContextHubIpi.ringBuf[mContextHubIpi.head++] = *ipiData;
    mContextHubIpi.head &= mContextHubIpi.size - 1;
    if (UNLIKELY(mContextHubIpi.head == mContextHubIpi.tail)) {
        osLog(LOG_ERROR, "dropped data due to ringbuffer is full\n");
        return -1;
    }
    return 0;
}
static void contextHubIpiTxStart(void)
{
    //osLog(LOG_DEBUG, "contextHubIpiTxStart\n");
    osEnqueuePrivateEvt(EVT_IPI_TX, NULL, NULL, mTask.id);
}
static void contextHubipiTimerCallback(uint32_t timerId, void *cookie)
{
    atomicWriteByte(&mContextHubIpi.isTimerRunning, STATE_STOP);
    osEnqueuePrivateEvt(EVT_IPI_TX, NULL, NULL, mTask.id);
}

static int contextHubIpiRxAck(const struct ContextHubCmd *cmd, SCP_SENSOR_HUB_REQ *req,
                              struct data_unit_t *data, int errCode)
{
    int ret = 0;
    SCP_SENSOR_HUB_DATA rsp;

    memset(&rsp, 0, sizeof(SCP_SENSOR_HUB_DATA));
    /* osLog(LOG_DEBUG, "contextHubIpiRxAck mtkType:%d, action:%d, event:%d\n",
          req->sensorType, req->action, req->event); */

    cmd->handlerAck(req, &rsp, data, errCode);
    ret = contextHubIpiTx(&rsp);
    contextHubIpiTxStart();
    return ret;
}
static int contextHubIpiNotifyAp(uint8_t sensorType, uint8_t action, uint8_t event,
                                 struct data_unit_t *data)
{
    int ret = 0;
    SCP_SENSOR_HUB_DATA rsp;

    memset(&rsp, 0, sizeof(SCP_SENSOR_HUB_DATA));
    /* osLog(LOG_DEBUG, "contextHubIpiRxAck mtkType:%d, action:%d, event:%d\n",
          sensorType, action, event); */
    switch (action) {
        case SENSOR_HUB_NOTIFY:
            switch (event) {
                case SCP_NOTIFY:
                    rsp.notify_rsp.sensorType = mtkTypeToApId(sensorType);
                    rsp.notify_rsp.action = SENSOR_HUB_NOTIFY;
                    rsp.notify_rsp.event = event;
                    memcpy(rsp.notify_rsp.int8_Data, data, SENSOR_DATA_SIZE);
                    break;
                case SCP_DIRECT_PUSH:
                case SCP_FIFO_FULL:
                    rsp.notify_rsp.sensorType = mtkTypeToApId(sensorType);
                    rsp.notify_rsp.action = SENSOR_HUB_NOTIFY;
                    rsp.notify_rsp.event = event;
                    rsp.notify_rsp.currWp = (uint32_t)data->value[0];
                    /*
                     * new timestamp sync algorithm between ap and scp
                     * report scp timestamp and ipi start time to ap, ap receive this packet
                     * will run moving window average algorithm to get new time sync offset and
                     * offset stamp in ap, new time sync offset will not stamp in scp.
                     */
                    {/*enclose variable state only in brackets*/
                        uint64_t state = cpuIntsOff();
                        rsp.notify_rsp.scp_timestamp = read_xgpt_stamp_ns();
                        rsp.notify_rsp.arch_counter = timer_get_global_timer_tick();
                        cpuIntsRestore(state);
                    }
                    break;
                case SCP_BATCH_TIMEOUT:
                    /* in chre, we don't support fifo full, use direct push instead */
                    break;
                default:
                    rsp.notify_rsp.sensorType = mtkTypeToApId(sensorType);
                    rsp.notify_rsp.action = SENSOR_HUB_NOTIFY;
                    rsp.notify_rsp.event = event;
                    break;
            }
            break;
        default:
            break;
    }
    ret = contextHubIpiTx(&rsp);
    contextHubIpiTxStart();
    return ret;
}
#ifdef DEBUG_DRAM_ADDR
//return physical memory address
static uint32_t contextHubFwGetDramWPAddr(struct sensorFIFO *dram_fifo)
{
    return (uint32_t)mTask.bufferBase + offsetof(struct sensorFIFO, wp);
}
//return physical memory address
static uint32_t contextHubFwGetDramRPAddr(struct sensorFIFO *dram_fifo)
{
    return (uint32_t)mTask.bufferBase + offsetof(struct sensorFIFO, rp);
}

//return physical memory address
static uint32_t contextHubFwGetDramRP(struct sensorFIFO *dram_fifo)
{
    return (uint32_t)mTask.bufferBase->data + (uint32_t)dram_fifo->rp;
}
//return physical memory address
static uint32_t contextHubFwGetDramWP(struct sensorFIFO *dram_fifo)
{
    return (uint32_t)mTask.bufferBase->data + (uint32_t)dram_fifo->wp;
}
#endif
static void contextHubSramFifoInit(void)
{
    uint16_t i = 0;

    mContextHubSramFifo.head = mContextHubSramFifo.tail = 0;
    mContextHubSramFifo.size = MAX_SRAM_FIFO_DEEPTH;
    for (; i < mContextHubSramFifo.size; ++i)
        memset(&mContextHubSramFifo.ringBuf[i], 0, sizeof(struct data_unit_t));
}
static void contextHubDramFifoInit(void)
{
    mContextHubDramFifo.timerHandle = 0;
    atomicWriteByte(&mContextHubDramFifo.isDramEnabling, STATE_STOP);
    wake_lock_init(&mContextHubDramFifo.wakeLock, "chre_dram");
}
/* before we cope data to dram, we only take hwsemaphore at the moment get rp and  wp pointer to
 * caculate the remain space left in dram, copy data time no need to take hw semaphore, but this
 * operation is safe enough, because we only operate the remain space in dram
 */
static int contextHubDramFifoSizeLeft(uint32_t *realSizeLeft)
{
    int ret = 0;
    uint32_t maxEventNumber = mContextHubDramFifo.maxEventNumber;
    uint32_t currEventNumber = mContextHubDramFifo.currEventNumber;

    if ((maxEventNumber - currEventNumber) < MAX_SRAM_FIFO_DEEPTH * 4) {
        ret = DRAM_FIFO_FULL;
        mContextHubDramFifo.currEventNumber = 0;
        //osLog(LOG_DEBUG, "maxEventNumber: %d, currEventNumber:%d\n", maxEventNumber, currEventNumber);
    } else if (currEventNumber > maxEventNumber)
        configASSERT(0);
    *realSizeLeft = (maxEventNumber - currEventNumber) * SENSOR_DATA_SIZE;
    return ret;
}
/* when we access dram, we only take hwsemaphore at the moment get and update wp pointer,
 * copy data time no need to take hw semaphore, but this operation is safe enough
 */
static int contextHubDramFifoWrite(const uint8_t *src, uint32_t size)
{
    struct sensorFIFO *dram_fifo = NULL;
    uint32_t size_wp2end;
    uint32_t wp = 0;
    uint32_t FIFOSize;
    uint32_t packetHead;
    DMA_RESULT ret;

#ifdef CFG_VCORE_DVFS_SUPPORT
    dvfs_enable_DRAM_resource(SENS_MEM_ID);
#endif
    get_emi_semaphore();
    dram_fifo = (struct sensorFIFO *)mContextHubDramFifo.bufferBase;
    wp = dram_fifo->wp;
    FIFOSize = dram_fifo->FIFOSize;
    packetHead = (uint32_t)dram_fifo->data;

    size_wp2end = FIFOSize - wp;
    /* secondly, push data to dram */
    if (size <= size_wp2end) {
#ifdef SCP_DMA_VERSION_03
        ret = scp_dma_transaction(packetHead + wp, (uint32_t)src, size, CONTEXT_HUB_DMA_ID, NO_RESERVED);
#else
        ret = dma_transaction(packetHead + wp, (uint32_t)src, size);
#endif
        if (ret != DMA_RESULT_DONE) {
            osLog(LOG_ERROR, "dma_transaction failed!\n");
            ret = FLUSH_TO_DRAM_ERR;
            goto err;
        }
    } else {
#ifdef SCP_DMA_VERSION_03
        ret = scp_dma_transaction(packetHead + wp, (uint32_t)src, size_wp2end, CONTEXT_HUB_DMA_ID, NO_RESERVED);
#else
        ret = dma_transaction(packetHead + wp, (uint32_t)src, size_wp2end);
#endif
        if (ret != DMA_RESULT_DONE) {
            osLog(LOG_ERROR, "dma_transaction failed!\n");
            ret = FLUSH_TO_DRAM_ERR;
            goto err;
        }

#ifdef SCP_DMA_VERSION_03
        ret = scp_dma_transaction(packetHead, (uint32_t)src + size_wp2end, size - size_wp2end, CONTEXT_HUB_DMA_ID,
                NO_RESERVED);
#else
        ret = dma_transaction(packetHead, (uint32_t)src + size_wp2end, size - size_wp2end);
#endif
        if (ret != DMA_RESULT_DONE) {
            osLog(LOG_ERROR, "dma_transaction failed!\n");
            ret = FLUSH_TO_DRAM_ERR;
            goto err;
        }
    }
    dram_fifo->wp = (dram_fifo->wp + size) % dram_fifo->FIFOSize;
    ret = FLUSH_TO_DRAM_SUCCESS;
err:
    release_emi_semaphore();
#ifdef CFG_VCORE_DVFS_SUPPORT
    dvfs_disable_DRAM_resource(SENS_MEM_ID);
#endif
    return ret;
}
/* this function read sramdfifo data to dramfifo, if sramfifo data left > dramfifo sizeleft, we
 * only copy dram sizeleft to from sramfifo to dramfifo, if sramfifo data left < dramfifo
 * sizeleft, we can copy the whole srammfifo data to dram.
 */
static int contextHubSramFifoRead(void)
{
    int ret = 0;
    uint32_t realSizeLeft = 0, realSizeTx = 0;
    uint32_t realSizeLeftNum = 0, realSizeTxNum = 0;
    uint32_t currWp = 0;
    struct data_unit_t dummy;
    struct sensorFIFO *dram_fifo = NULL;

    memset(&dummy, 0, sizeof(struct data_unit_t));

    realSizeTxNum = mContextHubSramFifo.head - mContextHubSramFifo.tail;
    realSizeTx = (mContextHubSramFifo.head - mContextHubSramFifo.tail) * SENSOR_DATA_SIZE;
    if (realSizeTx == 0)
        return SRAM_FIFO_EMPTY;
	/* firstly, we should get how much space remain in dram, if dram is full, we fisrtly notify ap to copy data
	 */
    ret = contextHubDramFifoSizeLeft(&realSizeLeft);
    if (ret == DRAM_FIFO_FULL) {
        //osLog(LOG_DEBUG, "ContextHubDramFifo full, realSizeLeft num:%d\n", realSizeLeft / SENSOR_DATA_SIZE);
#ifdef CFG_VCORE_DVFS_SUPPORT
        dvfs_enable_DRAM_resource(SENS_MEM_ID);
#endif
        get_emi_semaphore();
        dram_fifo = (struct sensorFIFO *)mContextHubDramFifo.bufferBase;
        currWp = dram_fifo->wp;
        release_emi_semaphore();
#ifdef CFG_VCORE_DVFS_SUPPORT
        dvfs_disable_DRAM_resource(SENS_MEM_ID);
#endif
        dummy.value[0] = currWp;
        if (mContextHubDramFifo.lastWpPointer != currWp) {
            /* osLog(LOG_ERROR, "ContextHubDramFifo lastWpPointer:%d, currWpPointer:%d\n",
                mContextHubDramFifo.lastWpPointer, currWp); */
            mContextHubDramFifo.lastWpPointer = currWp;
            contextHubIpiNotifyAp(0, SENSOR_HUB_NOTIFY, SCP_FIFO_FULL, &dummy);
        } else
            ;//osLog(LOG_ERROR, "ContextHubDramFifo lastWpPointer:%d, currWpPointer:%d\n",
                //mContextHubDramFifo.lastWpPointer, currWp);
    }
    realSizeLeftNum = realSizeLeft / SENSOR_DATA_SIZE;
    //osLog(LOG_DEBUG, "contextHubSramFifoRead, realSizeLeft num:%d\n", realSizeLeftNum);
    /* secondly, we should copy data to dram */
    if (realSizeLeftNum < realSizeTxNum) {
        osLog(LOG_INFO, "realSizeLeftNum(%lu) < realSizeTxNum(%lu)\n", realSizeLeftNum, realSizeTxNum);
        ret = contextHubDramFifoWrite((const uint8_t *)mContextHubSramFifo.ringBuf, realSizeLeft);
        if (ret != FLUSH_TO_DRAM_SUCCESS) {
            return FLUSH_TO_DRAM_ERR;
        }
        mContextHubDramFifo.currEventNumber += realSizeLeftNum;
        /* this place must use memmove, if use memcpy will lead data replace, if we need move num 11 to num 511
         * event to head of the ringbuffer(location num 0 to num 500), memcpy will cpy wrong data to this buffer
         */
        memmove(mContextHubSramFifo.ringBuf, &mContextHubSramFifo.ringBuf[realSizeLeftNum],
            realSizeTx - realSizeLeft);
        /* update head pointer again, head pointer is not start at 0 at this case, the real num is
         * realSizeTxNum - realSizeLeftNum
         */
        mContextHubSramFifo.head = realSizeTxNum - realSizeLeftNum;
        mContextHubSramFifo.head &= mContextHubSramFifo.size - 1;
        mContextHubSramFifo.tail = 0;
    } else {
        /* update head pointer again, this case will copy full sram fifo data to dram, theb we should
         * let head and tail point to the buffer head
         */
        ret = contextHubDramFifoWrite((const uint8_t *)mContextHubSramFifo.ringBuf, realSizeTx);
        if (ret != FLUSH_TO_DRAM_SUCCESS) {
            return FLUSH_TO_DRAM_ERR;
        }
        mContextHubDramFifo.currEventNumber += realSizeTxNum;
        mContextHubSramFifo.head = mContextHubSramFifo.tail = 0;
    }
    /* osLog(LOG_DEBUG, "mContextHubSramFifo.head :%d tail: %d, size:%d\n",
        mContextHubSramFifo.head, mContextHubSramFifo.head, size); */
    return ret;
}
/* this function write sensordata to sramdfifo, when head + 1 == tail,
 * then the sramfifo is full, we should put data to dram fifo and clear sramfifo
 */
static int contextHubSramFifoWrite(struct data_unit_t *data)
{
    uint32_t preHead = 0;

    mContextHubSramFifo.ringBuf[mContextHubSramFifo.head++] = *data;
    mContextHubSramFifo.head &= mContextHubSramFifo.size - 1;
    preHead = mContextHubSramFifo.head + 1;
    preHead &= mContextHubSramFifo.size - 1;
    if (UNLIKELY(preHead == mContextHubSramFifo.tail)) {
        //osLog(LOG_ERROR, "ContextHubSramFifo ringbuffer is full, then push data to dram\n");
        return SRAM_FIFO_FULL;
    }
    return 0;
}

static void contextHubTransferOnChangeSensor(uint8_t mtk_type, const struct mtkActiveSensor *sensor)
{
    int ret = 0;
    uint32_t numSamples = 0, numFlushes = 0;
    struct data_unit_t dummy;
    uint64_t lastTimeStamp = 0;

    memset(&dummy, 0, sizeof(struct data_unit_t));

    /* report data to ap firstly, numSamples represent data */
    for (numSamples = 0; numSamples < sensor->buffer.firstSample.numSamples; ++numSamples) {
        dummy.sensor_type = mtkTypeToApId(mtk_type);
        dummy.flush_action = DATA_ACTION;
        if (numSamples == 0) {
            lastTimeStamp = dummy.time_stamp = sensor->buffer.referenceTime;
        } else {
            if (sensor->numAxis == NUM_AXIS_THREE)
                dummy.time_stamp = lastTimeStamp + sensor->buffer.triple[numSamples].deltaTime;
            else
                dummy.time_stamp = lastTimeStamp + sensor->buffer.single[numSamples].deltaTime;
            lastTimeStamp = dummy.time_stamp;
        }
        switch (mtk_type) {
            case SENSOR_TYPE_PROXIMITY:
                /* we don't support this sensType access dram to give data to ap by notify ap */
                dummy.proximity_t.oneshot = (int)sensor->buffer.single[numSamples].fdata;
                ret = contextHubSramFifoWrite(&dummy);
                if (ret == SRAM_FIFO_FULL)
                    contextHubSramFifoRead();
                break;
            case SENSOR_TYPE_TILT_DETECTOR:
                /* we don't support this sensType access dram to give data to ap by notify ap */
                dummy.tilt_event.state = sensor->buffer.single[numSamples].idata;
                ret = contextHubSramFifoWrite(&dummy);
                if (ret == SRAM_FIFO_FULL)
                    contextHubSramFifoRead();
                break;
            case SENSOR_TYPE_LIGHT:
                dummy.light = (uint32_t)sensor->buffer.single[numSamples].fdata;
                ret = contextHubSramFifoWrite(&dummy);
                if (ret == SRAM_FIFO_FULL)
                    contextHubSramFifoRead();
                break;
            case SENSOR_TYPE_STEP_COUNTER:
                dummy.step_counter_t.accumulated_step_count = (uint32_t)sensor->buffer.single[numSamples].idata;
                ret = contextHubSramFifoWrite(&dummy);
                if (ret == SRAM_FIFO_FULL)
                    contextHubSramFifoRead();
                break;
            case SENSOR_TYPE_STEP_DETECTOR:
                dummy.step_detector_t.step_detect = sensor->buffer.single[numSamples].idata;
                ret = contextHubSramFifoWrite(&dummy);
                if (ret == SRAM_FIFO_FULL)
                    contextHubSramFifoRead();
                break;
            case SENSOR_TYPE_DEVICE_ORIENTATION:
                /* we don't support this sensType access dram to give data to ap by notify ap */
                dummy.tilt_event.state = sensor->buffer.single[numSamples].idata;
                ret = contextHubSramFifoWrite(&dummy);
                if (ret == SRAM_FIFO_FULL)
                    contextHubSramFifoRead();
                break;
            case SENSOR_TYPE_GEOFENCE:
                /* we don't support this sensType access dram to give data to ap by notify ap */
                dummy.geofence_data_t.state = sensor->buffer.single[numSamples].idata;
                ret = contextHubSramFifoWrite(&dummy);
                if (ret == SRAM_FIFO_FULL)
                    contextHubSramFifoRead();
                break;
            case SENSOR_TYPE_FLOOR_COUNTER:
                dummy.floor_counter_t.accumulated_floor_count = (uint32_t)sensor->buffer.single[numSamples].idata;
                ret = contextHubSramFifoWrite(&dummy);
                if (ret == SRAM_FIFO_FULL)
                    contextHubSramFifoRead();
                break;
            case SENSOR_TYPE_RGBW:
                /* we don't support this sensType access dram to give data to ap by notify ap */
                tripleAxisDataToRgbwData((uint32_t *)dummy.value, &sensor->buffer.triple[numSamples]);
                ret = contextHubSramFifoWrite(&dummy);
                if (ret == SRAM_FIFO_FULL)
                    contextHubSramFifoRead();
                break;
            case SENSOR_TYPE_SAR:
                dummy.sar_event.data[0] = sensor->buffer.triple[numSamples].ix;
                dummy.sar_event.data[1] = sensor->buffer.triple[numSamples].iy;
                dummy.sar_event.data[2] = sensor->buffer.triple[numSamples].iz;

                ret = contextHubSramFifoWrite(&dummy);
                if (ret == SRAM_FIFO_FULL)
                    contextHubSramFifoRead();
                break;
        }
    }
    /* report flush to ap secondly, numFlushes represent flush */
    for (numFlushes = 0; numFlushes < sensor->buffer.firstSample.numFlushes; ++numFlushes) {
        dummy.sensor_type = mtkTypeToApId(mtk_type);
        dummy.flush_action = FLUSH_ACTION;
        osLog(LOG_INFO, "mtk_type: %d send flush action\n", mtk_type);
        ret = contextHubSramFifoWrite(&dummy);
        if (ret == SRAM_FIFO_FULL)
            contextHubSramFifoRead();
    }
}
static void contextHubTransferOneShotSensor(uint8_t mtk_type, const struct mtkActiveSensor *sensor)
{
    /* we don't support this sensType access dram to give data to ap by notify ap */
    int ret = 0;
    uint32_t numSamples = 0, numFlushes = 0;
    struct data_unit_t dummy;
    uint64_t lastTimeStamp = 0;

    memset(&dummy, 0, sizeof(struct data_unit_t));

    /* report data to ap firstly, numSamples represent data */
    for (numSamples = 0; numSamples < sensor->buffer.firstSample.numSamples; ++numSamples) {
        dummy.sensor_type = mtkTypeToApId(mtk_type);
        dummy.flush_action = DATA_ACTION;
        if (numSamples == 0) {
            lastTimeStamp = dummy.time_stamp = sensor->buffer.referenceTime;
        } else {
            if (sensor->numAxis == NUM_AXIS_THREE)
                dummy.time_stamp = lastTimeStamp + sensor->buffer.triple[numSamples].deltaTime;
            else
                dummy.time_stamp = lastTimeStamp + sensor->buffer.single[numSamples].deltaTime;
            lastTimeStamp = dummy.time_stamp;
        }
        if (sensor->oneshot) { //oneshot sensor
            switch (mtk_type) {
                case SENSOR_TYPE_SIGNIFICANT_MOTION:
                    dummy.smd_t.state = sensor->buffer.single[numSamples].idata;
                    break;
                case SENSOR_TYPE_SHAKE:
                    dummy.gesture_data_t.probability = sensor->buffer.single[numSamples].idata;
                    break;
                case SENSOR_TYPE_GLANCE_GESTURE:
                    dummy.gesture_data_t.probability = sensor->buffer.single[numSamples].idata;
                    break;
                case SENSOR_TYPE_PICK_UP_GESTURE:
                    dummy.gesture_data_t.probability = sensor->buffer.single[numSamples].idata;
                    break;
                case SENSOR_TYPE_ANSWER_CALL:
                    dummy.gesture_data_t.probability = sensor->buffer.single[numSamples].idata;
                    break;
                case SENSOR_TYPE_STATIONARY_DETECT:
                    dummy.gesture_data_t.probability = sensor->buffer.single[numSamples].idata;
                    break;
                case SENSOR_TYPE_MOTION_DETECT:
                    dummy.gesture_data_t.probability = sensor->buffer.single[numSamples].idata;
                    break;
                case SENSOR_TYPE_WAKE_GESTURE:
                    dummy.gesture_data_t.probability = sensor->buffer.single[numSamples].idata;
                    break;
                case SENSOR_TYPE_INPOCKET:
                    dummy.inpocket_event.state = sensor->buffer.single[numSamples].idata;
                    break;
                case SENSOR_TYPE_FLAT:
                    dummy.flat_t.state = sensor->buffer.single[numSamples].idata;
                    break;
            }
            ret = contextHubSramFifoWrite(&dummy);
            if (ret == SRAM_FIFO_FULL)
                contextHubSramFifoRead();
            //osLog(LOG_DEBUG, "contextHubFormateData oneshot sensor:%d!\n", mtk_type);
        }
    }
    /* report flush to ap secondly, numFlushes represent flush */
    for (numFlushes = 0; numFlushes < sensor->buffer.firstSample.numFlushes; ++numFlushes) {
        dummy.sensor_type = mtkTypeToApId(mtk_type);
        dummy.flush_action = FLUSH_ACTION;
        osLog(LOG_INFO, "mtk_type: %d send flush action\n", mtk_type);
        ret = contextHubSramFifoWrite(&dummy);
        if (ret == SRAM_FIFO_FULL)
            contextHubSramFifoRead();
    }
}
static void contextHubCalcuAccurancy(uint8_t mtk_type, const struct mtkActiveSensor *sensor)
{
    if (sensor->buffer.firstSample.biasCurrent == 1)
        sensorAccurancyMode[mtk_type] = SENSOR_STATUS_ACCURACY_HIGH;
    else
        sensorAccurancyMode[mtk_type] = SENSOR_STATUS_UNRELIABLE;
}
static float contextHubCalcuVector4(float x, float y, float z)
{
    float vector_1, vector_2, vector_3, vector_4;

    vector_1 = x;
    vector_2 = y;
    vector_3 = z;
    vector_4 = (1 - vector_1 * vector_1 - vector_2 * vector_2 - vector_3 * vector_3);
    vector_4 = (vector_4 > 0.0f) ? sqrtf(vector_4) : 0.0f;
    return vector_4;
}

static void contextHubTransferContinueSensor(uint8_t mtk_type, const struct mtkActiveSensor *sensor)
{
    /* we support continus sensType access dram to give data to ap */
    int ret = 0;
    uint32_t numSamples = 0, numFlushes = 0;
    struct data_unit_t dummy;
    uint64_t lastTimeStamp = 0;

    memset(&dummy, 0, sizeof(struct data_unit_t));

    /* report data to ap firstly, numSamples represent data */
    for (numSamples = 0; numSamples < sensor->buffer.firstSample.numSamples; ++numSamples) {
        if (numSamples == 0) {
            lastTimeStamp = dummy.time_stamp = sensor->buffer.referenceTime;
        } else {
            if (sensor->numAxis == NUM_AXIS_THREE)
                dummy.time_stamp = lastTimeStamp + sensor->buffer.triple[numSamples].deltaTime;
            else
                dummy.time_stamp = lastTimeStamp + sensor->buffer.single[numSamples].deltaTime;
            lastTimeStamp = dummy.time_stamp;
        }
        if (sensor->buffer.firstSample.biasPresent && sensor->buffer.firstSample.biasSample == numSamples) {
            contextHubCalcuAccurancy(mtk_type, sensor);
            if (mtk_type == SENSOR_TYPE_ACCELEROMETER) {
                dummy.sensor_type = mtkTypeToApId(mtk_type);
                dummy.flush_action = BIAS_ACTION;
                accBias[0] = sensor->buffer.triple[numSamples].x;
                accBias[1] = sensor->buffer.triple[numSamples].y;
                accBias[2] = sensor->buffer.triple[numSamples].z;
                dummy.accelerometer_t.x_bias = (int32_t)(accBias[0] * ACCELEROMETER_INCREASE_NUM_AP);
                dummy.accelerometer_t.y_bias = (int32_t)(accBias[1] * ACCELEROMETER_INCREASE_NUM_AP);
                dummy.accelerometer_t.z_bias = (int32_t)(accBias[2] * ACCELEROMETER_INCREASE_NUM_AP);
            } else if (mtk_type == SENSOR_TYPE_MAGNETIC_FIELD) {
                dummy.sensor_type = mtkTypeToApId(mtk_type);
                dummy.flush_action = BIAS_ACTION;
                magBias[0] = sensor->buffer.triple[numSamples].x;
                magBias[1] = sensor->buffer.triple[numSamples].y;
                magBias[2] = sensor->buffer.triple[numSamples].z;
                dummy.magnetic_t.x_bias = (int32_t)(magBias[0] * MAGNETOMETER_INCREASE_NUM_AP);
                dummy.magnetic_t.y_bias = (int32_t)(magBias[1] * MAGNETOMETER_INCREASE_NUM_AP);
                dummy.magnetic_t.z_bias = (int32_t)(magBias[2] * MAGNETOMETER_INCREASE_NUM_AP);
            } else if (mtk_type == SENSOR_TYPE_GYROSCOPE) {
                dummy.sensor_type = mtkTypeToApId(mtk_type);
                dummy.flush_action = BIAS_ACTION;
                gyroBias[0] = sensor->buffer.triple[numSamples].x;
                gyroBias[1] = sensor->buffer.triple[numSamples].y;
                gyroBias[2] = sensor->buffer.triple[numSamples].z;
                dummy.gyroscope_t.x_bias = (int32_t)(gyroBias[0] * RADIRAN_TO_DEGREE_SCALAR
                    * GYROSCOPE_INCREASE_NUM_AP);
                dummy.gyroscope_t.y_bias = (int32_t)(gyroBias[1] * RADIRAN_TO_DEGREE_SCALAR
                    * GYROSCOPE_INCREASE_NUM_AP);
                dummy.gyroscope_t.z_bias = (int32_t)(gyroBias[2] * RADIRAN_TO_DEGREE_SCALAR
                    * GYROSCOPE_INCREASE_NUM_AP);
            }
            osLog(LOG_INFO, "mtk_type:%d, status: %d, offset: [%f, %f, %f]\n",
                mtk_type, sensorAccurancyMode[mtk_type],
                (double)sensor->buffer.triple[numSamples].x,
                (double)sensor->buffer.triple[numSamples].y,
                (double)sensor->buffer.triple[numSamples].z);
            ret = contextHubSramFifoWrite(&dummy);
            if (ret == SRAM_FIFO_FULL)
                contextHubSramFifoRead();
        } else {
            // for last samples to debug latency
            if (numSamples == sensor->buffer.firstSample.numSamples - 1)
                dummy.reserve[0] = 1;
            else
                dummy.reserve[0] = 0;
            dummy.sensor_type = mtkTypeToApId(mtk_type);
            dummy.flush_action = DATA_ACTION;
            //osLog(LOG_INFO, "contextHubFormateData mtk_type: %d, numSamples:%d, time_stamp:%lld\n", mtk_type,
                //numSamples, dummy.time_stamp);
            switch (mtk_type) {
                case SENSOR_TYPE_ACCELEROMETER:
                    dummy.accelerometer_t.x = (int32_t)(sensor->buffer.triple[numSamples].x * ACCELEROMETER_INCREASE_NUM_AP);
                    dummy.accelerometer_t.y = (int32_t)(sensor->buffer.triple[numSamples].y * ACCELEROMETER_INCREASE_NUM_AP);
                    dummy.accelerometer_t.z = (int32_t)(sensor->buffer.triple[numSamples].z * ACCELEROMETER_INCREASE_NUM_AP);
                    dummy.accelerometer_t.status = sensorAccurancyMode[mtk_type];
                    break;
                case SENSOR_TYPE_MAGNETIC_FIELD:
                    dummy.magnetic_t.x = (int32_t)(sensor->buffer.triple[numSamples].x * MAGNETOMETER_INCREASE_NUM_AP);
                    dummy.magnetic_t.y = (int32_t)(sensor->buffer.triple[numSamples].y * MAGNETOMETER_INCREASE_NUM_AP);
                    dummy.magnetic_t.z = (int32_t)(sensor->buffer.triple[numSamples].z * MAGNETOMETER_INCREASE_NUM_AP);
                    dummy.magnetic_t.x_bias = (int32_t)(magBias[0] * MAGNETOMETER_INCREASE_NUM_AP);
                    dummy.magnetic_t.y_bias = (int32_t)(magBias[1] * MAGNETOMETER_INCREASE_NUM_AP);
                    dummy.magnetic_t.z_bias = (int32_t)(magBias[2] * MAGNETOMETER_INCREASE_NUM_AP);
                    dummy.magnetic_t.status = sensorAccurancyMode[mtk_type];
                    break;
                case SENSOR_TYPE_GYROSCOPE:
                    dummy.gyroscope_t.x = (int32_t)(sensor->buffer.triple[numSamples].x * RADIRAN_TO_DEGREE_SCALAR
                        * GYROSCOPE_INCREASE_NUM_AP);
                    dummy.gyroscope_t.y = (int32_t)(sensor->buffer.triple[numSamples].y * RADIRAN_TO_DEGREE_SCALAR
                        * GYROSCOPE_INCREASE_NUM_AP);
                    dummy.gyroscope_t.z = (int32_t)(sensor->buffer.triple[numSamples].z * RADIRAN_TO_DEGREE_SCALAR
                        * GYROSCOPE_INCREASE_NUM_AP);
                    dummy.gyroscope_t.x_bias = (int32_t)(gyroBias[0] * RADIRAN_TO_DEGREE_SCALAR
                        * GYROSCOPE_INCREASE_NUM_AP);
                    dummy.gyroscope_t.y_bias = (int32_t)(gyroBias[1] * RADIRAN_TO_DEGREE_SCALAR
                        * GYROSCOPE_INCREASE_NUM_AP);
                    dummy.gyroscope_t.z_bias = (int32_t)(gyroBias[2] * RADIRAN_TO_DEGREE_SCALAR
                        * GYROSCOPE_INCREASE_NUM_AP);
                    dummy.gyroscope_t.status = sensorAccurancyMode[mtk_type];
                    dummy.gyroscope_t.temperature= (int32_t)(accTemp*100);
                    dummy.accelerometer_t.temp_result = temp_status;
/*
                    osLog(LOG_ERROR, "mtk_type:%d, status: %d, offset: [%d, %d, %d]\n",
                        mtk_type, dummy.gyroscope_t.temperature,
                        dummy.gyroscope_t.x,
                        dummy.gyroscope_t.y,
                        dummy.gyroscope_t.x_bias);
*/

                    break;
                case SENSOR_TYPE_PRESSURE:
                    dummy.pressure_t.pressure = (int32_t)(sensor->buffer.single[numSamples].fdata * BAROMETER_INCREASE_NUM_AP);
                    break;
                case SENSOR_TYPE_ORIENTATION:
                    dummy.orientation_t.azimuth = (int32_t)(sensor->buffer.triple[numSamples].x * ORIENTATION_INCREASE_NUM_AP);
                    dummy.orientation_t.pitch = (int32_t)(sensor->buffer.triple[numSamples].y * ORIENTATION_INCREASE_NUM_AP);
                    dummy.orientation_t.roll = (int32_t)(sensor->buffer.triple[numSamples].z * ORIENTATION_INCREASE_NUM_AP);
                    dummy.orientation_t.status = sensorAccurancyMode[SENSOR_TYPE_MAGNETIC_FIELD];
                    break;
                case SENSOR_TYPE_ROTATION_VECTOR:
                    dummy.orientation_t.azimuth = (int32_t)(sensor->buffer.triple[numSamples].x * RV_INCREASE_NUM_AP);
                    dummy.orientation_t.pitch = (int32_t)(sensor->buffer.triple[numSamples].y * RV_INCREASE_NUM_AP);
                    dummy.orientation_t.roll = (int32_t)(sensor->buffer.triple[numSamples].z * RV_INCREASE_NUM_AP);
                    dummy.orientation_t.scalar = (int32_t)(contextHubCalcuVector4(sensor->buffer.triple[numSamples].x,
                        sensor->buffer.triple[numSamples].y, sensor->buffer.triple[numSamples].z) * RV_INCREASE_NUM_AP);
                    dummy.orientation_t.status = sensorAccurancyMode[SENSOR_TYPE_MAGNETIC_FIELD];
                    break;
                case SENSOR_TYPE_GAME_ROTATION_VECTOR:
                    dummy.orientation_t.azimuth = (int32_t)(sensor->buffer.triple[numSamples].x * GMRV_INCREASE_NUM_AP);
                    dummy.orientation_t.pitch = (int32_t)(sensor->buffer.triple[numSamples].y * GMRV_INCREASE_NUM_AP);
                    dummy.orientation_t.roll = (int32_t)(sensor->buffer.triple[numSamples].z * GMRV_INCREASE_NUM_AP);
                    dummy.orientation_t.scalar = (int32_t)(contextHubCalcuVector4(sensor->buffer.triple[numSamples].x,
                        sensor->buffer.triple[numSamples].y, sensor->buffer.triple[numSamples].z) * GMRV_INCREASE_NUM_AP);
                    dummy.orientation_t.status = sensorAccurancyMode[SENSOR_TYPE_MAGNETIC_FIELD];
                    break;
                case SENSOR_TYPE_GEOMAGNETIC_ROTATION_VECTOR:
                    dummy.magnetic_t.azimuth = (int32_t)(sensor->buffer.triple[numSamples].x * GRV_INCREASE_NUM_AP);
                    dummy.magnetic_t.pitch = (int32_t)(sensor->buffer.triple[numSamples].y * GRV_INCREASE_NUM_AP);
                    dummy.magnetic_t.roll =(int32_t)(sensor->buffer.triple[numSamples].z * GRV_INCREASE_NUM_AP);
                    dummy.magnetic_t.scalar = (int32_t)(contextHubCalcuVector4(sensor->buffer.triple[numSamples].x,
                        sensor->buffer.triple[numSamples].y, sensor->buffer.triple[numSamples].z) * GRV_INCREASE_NUM_AP);
                    dummy.magnetic_t.status = sensorAccurancyMode[SENSOR_TYPE_MAGNETIC_FIELD];
                    break;
                case SENSOR_TYPE_GRAVITY:
                    dummy.accelerometer_t.x = (int32_t)(sensor->buffer.triple[numSamples].x * GRAV_INCREASE_NUM_AP);
                    dummy.accelerometer_t.y = (int32_t)(sensor->buffer.triple[numSamples].y * GRAV_INCREASE_NUM_AP);
                    dummy.accelerometer_t.z = (int32_t)(sensor->buffer.triple[numSamples].z * GRAV_INCREASE_NUM_AP);
                    dummy.accelerometer_t.status = sensorAccurancyMode[SENSOR_TYPE_MAGNETIC_FIELD];
                    break;
                case SENSOR_TYPE_LINEAR_ACCELERATION:
                    dummy.accelerometer_t.x = (int32_t)(sensor->buffer.triple[numSamples].x * LINEARACCEL_INCREASE_NUM_AP);
                    dummy.accelerometer_t.y = (int32_t)(sensor->buffer.triple[numSamples].y * LINEARACCEL_INCREASE_NUM_AP);
                    dummy.accelerometer_t.z = (int32_t)(sensor->buffer.triple[numSamples].z * LINEARACCEL_INCREASE_NUM_AP);
                    dummy.accelerometer_t.status = sensorAccurancyMode[SENSOR_TYPE_MAGNETIC_FIELD];
                    break;
                case SENSOR_TYPE_ACTIVITY:
                    tripleAxisDataToActivityData(dummy.activity_data_t.probability,
                        ACTIVITY_MAX, &sensor->buffer.triple[numSamples]);
#if 0
                    osLog(LOG_ERROR, "activity: %d, %d, %d, %d\n",
                        dummy.activity_data_t.probability[0],
                        dummy.activity_data_t.probability[1],
                        dummy.activity_data_t.probability[2],
                        dummy.activity_data_t.probability[3]);
                    osLog(LOG_ERROR, "activity: %d, %d, %d, %d\n",
                        dummy.activity_data_t.probability[4],
                        dummy.activity_data_t.probability[5],
                        dummy.activity_data_t.probability[6],
                        dummy.activity_data_t.probability[7]);
                    osLog(LOG_ERROR, "activity: %d, %d, %d, %d\n",
                        dummy.activity_data_t.probability[8],
                        dummy.activity_data_t.probability[9],
                        dummy.activity_data_t.probability[10],
                        dummy.activity_data_t.probability[11]);
#endif
                    break;
            }
            if (!estimateDownSample(mtk_type)) {
                ret = contextHubSramFifoWrite(&dummy);
                if (ret == SRAM_FIFO_FULL)
                    contextHubSramFifoRead();
            }
        }
    }
    /* report flush to ap secondly, numFlushes represent flush */
    for (numFlushes = 0; numFlushes < sensor->buffer.firstSample.numFlushes; ++numFlushes) {
        dummy.sensor_type = mtkTypeToApId(mtk_type);
        dummy.flush_action = FLUSH_ACTION;
        osLog(LOG_INFO, "mtk_type: %d send flush action\n", mtk_type);
        ret = contextHubSramFifoWrite(&dummy);
        if (ret == SRAM_FIFO_FULL)
            contextHubSramFifoRead();
    }
}
static void contextHubFormateData(const void *src)
{
    uint8_t mtk_type = 0;

    const struct mtkActiveSensor *sensor = src;

    if (hostIntfGetInterruptMask(NANOHUB_INT_NONWAKEUP) &&
            sensor->interrupt == NANOHUB_INT_NONWAKEUP) {
        return;
    }

    mtk_type = chreTypeToMtkType(sensor->buffer.sensType);

    if (sensorReportingMode[mtk_type] == oneShot)
        contextHubTransferOneShotSensor(mtk_type, sensor);
    else if (sensorReportingMode[mtk_type] == onChange)
        contextHubTransferOnChangeSensor(mtk_type, sensor);
    else if (sensorReportingMode[mtk_type] == continues)
        contextHubTransferContinueSensor(mtk_type, sensor);
}
static void contextHubDramEnableTimerCallback(uint32_t timerId, void *cookie)
{
    osEnqueuePrivateEvt(EVT_DRAM, cookie, NULL, mTask.id);
}
static int contextHubRealTransfer(uint8_t chreType);
int32_t __attribute__((weak)) check_dram_ack_status(void)
{
    return 0;
}
int hostIntfWakeUp(void)
{
    int dramStatus = 0;

    if (atomicCmpXchgByte(&mContextHubDramFifo.isDramEnabling, STATE_STOP, STATE_START)) {
#ifdef CFG_VCORE_DVFS_SUPPORT
        sshub_enable_AP_resource_nonblock(SENS_MEM_ID);
        dramStatus = check_dram_ack_status();
#endif
        if (!dramStatus) {
            //osLog(LOG_ERROR, "start timer waiting for dram enable\n");
            /* wake lock for timer callback come, this can lower sensor data latency about 1.3ms which is exit idle time */
            wake_lock(&mContextHubDramFifo.wakeLock);
            mContextHubDramFifo.timerHandle = timTimerSet(500000, 0, 50,
                contextHubDramEnableTimerCallback,
                (void *)(uint32_t)hostIntfGetInterruptType(), true);
            if (mContextHubDramFifo.timerHandle == 0) {
                osLog(LOG_ERROR, "contexthubfw cann't alloc a chre timer for dram enable\n");
                configASSERT(0);
            }
        } else if (dramStatus > 0) {
            //osLog(LOG_ERROR, "directly access dram\n");
            contextHubRealTransfer(hostIntfGetInterruptType());
        }
    }
    return 0;
}
static int contextHubRealTransfer(uint8_t chreType)
{
    int ret = 0;
    uint32_t currWp = 0;
    struct data_unit_t dummy;
    struct sensorFIFO *dram_fifo = NULL;

    memset(&dummy, 0, sizeof(struct data_unit_t));

    /* when hostinterface notify ap to get data from dram, we first should put data to dram then notify ap
     * contextHubSramFifoRead only for continues sensType
     */
    atomicWriteByte(&mContextHubDramFifo.isDramEnabling, STATE_STOP);
#ifdef CFG_VCORE_DVFS_SUPPORT
    ret = sshub_get_AP_ack_nonblock(SENS_MEM_ID);
    if (ret < 0) {
        osLog(LOG_ERROR, "sshub_get_AP_ack_nonblock fail\n");
        return -1;
    }
#endif
    ret = contextHubSramFifoRead();
    if (ret == FLUSH_TO_DRAM_ERR){
#ifdef CFG_VCORE_DVFS_SUPPORT
        dvfs_disable_DRAM_resource(SENS_MEM_ID);
#endif
        ret = -1;
    } else {
        get_emi_semaphore();
        dram_fifo = (struct sensorFIFO *)mContextHubDramFifo.bufferBase;
        currWp = dram_fifo->wp;
        release_emi_semaphore();
#ifdef CFG_VCORE_DVFS_SUPPORT
        dvfs_disable_DRAM_resource(SENS_MEM_ID);
#endif
        dummy.value[0] = currWp;
        /* osLog(LOG_DEBUG, "hostIntfWakeUp continues sensType access dram lastWpPointer:%d, currWpPointer:%d\n",
            mContextHubDramFifo.lastWpPointer, currWp); */
        if (mContextHubDramFifo.lastWpPointer != currWp) {
            mContextHubDramFifo.lastWpPointer = currWp;
            ret = contextHubIpiNotifyAp(chreTypeToMtkType(chreType), SENSOR_HUB_NOTIFY, SCP_DIRECT_PUSH, &dummy);
        } else
            ;//osLog(LOG_ERROR, "hostIntfWakeUp continues sensType access dram lastWpPointer:%d, currWpPointer:%d\n",
                //mContextHubDramFifo.lastWpPointer, currWp);
    }
    return ret;
}

static void hostIntfTransferData(const void *src)
{
    //osLog(LOG_DEBUG, "contextHubTransferData: %p\n", src);
    contextHubFormateData(src);
}

static void hostIntfTransferCaliData(const struct mtkActiveSensor *sensor)
{
    uint8_t mtk_type = 0;
    struct data_unit_t dummy;
    const struct CalibrationData *mCalibrationData = (const struct CalibrationData *)sensor->buffer.buffer;

    if (hostIntfGetInterruptMask(NANOHUB_INT_NONWAKEUP) &&
            sensor->interrupt == NANOHUB_INT_NONWAKEUP) {
        return;
    }

    memset(&dummy, 0, sizeof(struct data_unit_t));

    mtk_type = chreTypeToMtkType(mCalibrationData->data_header.sensorType);

    if (mtk_type == SENSOR_TYPE_ACCELEROMETER) {
        if (mCalibrationData->data_header.msgId == SENSOR_APP_MSG_ID_CAL_RESULT) {
            dummy.sensor_type = mtkTypeToApId(mtk_type);
            dummy.flush_action = CALI_ACTION;
            dummy.accelerometer_t.x_bias = (int32_t)mCalibrationData->values[0];
            dummy.accelerometer_t.y_bias = (int32_t)mCalibrationData->values[1];
            dummy.accelerometer_t.z_bias = (int32_t)mCalibrationData->values[2];
            dummy.accelerometer_t.status = (uint32_t)mCalibrationData->data_header.status;
            if (contextHubSramFifoWrite(&dummy) == SRAM_FIFO_FULL)
                contextHubSramFifoRead();
        } else if (mCalibrationData->data_header.msgId == SENSOR_APP_MSG_ID_TEST_RESULT) {
            dummy.sensor_type = mtkTypeToApId(mtk_type);
            dummy.flush_action = TEST_ACTION;
            dummy.accelerometer_t.status = (uint32_t)(int8_t)mCalibrationData->data_header.status;
            if (contextHubSramFifoWrite(&dummy) == SRAM_FIFO_FULL)
                contextHubSramFifoRead();
        }
    } else if (mtk_type == SENSOR_TYPE_GYROSCOPE) {
        if (mCalibrationData->data_header.msgId == SENSOR_APP_MSG_ID_CAL_RESULT) {
            dummy.sensor_type = mtkTypeToApId(mtk_type);
            dummy.flush_action = CALI_ACTION;
            dummy.gyroscope_t.x_bias = (int32_t)mCalibrationData->values[0];
            dummy.gyroscope_t.y_bias = (int32_t)mCalibrationData->values[1];
            dummy.gyroscope_t.z_bias = (int32_t)mCalibrationData->values[2];
            dummy.gyroscope_t.status = (uint32_t)mCalibrationData->data_header.status;
            if (contextHubSramFifoWrite(&dummy) == SRAM_FIFO_FULL)
                contextHubSramFifoRead();
        } else if (mCalibrationData->data_header.msgId == SENSOR_APP_MSG_ID_TEMP_RESULT) {
            dummy.sensor_type = mtkTypeToApId(mtk_type);
            dummy.flush_action = TEMP_ACTION;
            dummy.value[0] = mCalibrationData->values[0];
            dummy.value[1] = mCalibrationData->values[1];
            dummy.value[2] = mCalibrationData->values[2];
            dummy.value[3] = mCalibrationData->values[3];
            dummy.value[4] = mCalibrationData->values[4];
            dummy.value[5] = mCalibrationData->values[5];
            if (contextHubSramFifoWrite(&dummy) == SRAM_FIFO_FULL)
                contextHubSramFifoRead();
        } else if (mCalibrationData->data_header.msgId == SENSOR_APP_MSG_ID_TEST_RESULT) {
            dummy.sensor_type = mtkTypeToApId(mtk_type);
            dummy.flush_action = TEST_ACTION;
            dummy.gyroscope_t.status = (uint32_t)(int8_t)mCalibrationData->data_header.status;
            if (contextHubSramFifoWrite(&dummy) == SRAM_FIFO_FULL)
                contextHubSramFifoRead();
        }
    } else if (mtk_type == SENSOR_TYPE_MAGNETIC_FIELD) {
        if (mCalibrationData->data_header.msgId == SENSOR_APP_MSG_ID_TEST_RESULT) {
            dummy.sensor_type = mtkTypeToApId(mtk_type);
            dummy.flush_action = TEST_ACTION;
            dummy.magnetic_t.status = (uint32_t)(int8_t)mCalibrationData->data_header.status;
            if (contextHubSramFifoWrite(&dummy) == SRAM_FIFO_FULL)
                contextHubSramFifoRead();
        }
        if (mCalibrationData->data_header.msgId == SENSOR_APP_MSG_ID_CAL_RESULT) {
            dummy.sensor_type = mtkTypeToApId(mtk_type);
            dummy.flush_action = CALI_ACTION;
            dummy.value[0] = mCalibrationData->values[0];
            dummy.value[1] = mCalibrationData->values[1];
            dummy.value[2] = mCalibrationData->values[2];
            dummy.value[3] = mCalibrationData->values[3];
            dummy.value[4] = mCalibrationData->values[4];
            dummy.value[5] = mCalibrationData->values[5];
            if (contextHubSramFifoWrite(&dummy) == SRAM_FIFO_FULL)
                contextHubSramFifoRead();
        }
    } else if (mtk_type == SENSOR_TYPE_PROXIMITY) {
        if (mCalibrationData->data_header.msgId == SENSOR_APP_MSG_ID_CAL_RESULT) {
            dummy.sensor_type = mtkTypeToApId(mtk_type);
            dummy.flush_action = CALI_ACTION;
            dummy.value[0] = mCalibrationData->values[0];
            dummy.value[1] = mCalibrationData->values[1];
            if (contextHubSramFifoWrite(&dummy) == SRAM_FIFO_FULL)
                contextHubSramFifoRead();
        }
    } else if (mtk_type == SENSOR_TYPE_LIGHT) {
        if (mCalibrationData->data_header.msgId == SENSOR_APP_MSG_ID_CAL_RESULT) {
            dummy.sensor_type = mtkTypeToApId(mtk_type);
            dummy.flush_action = CALI_ACTION;
            dummy.value[0] = mCalibrationData->values[0];
            if (contextHubSramFifoWrite(&dummy) == SRAM_FIFO_FULL)
                contextHubSramFifoRead();
        }
    } else if (mtk_type == SENSOR_TYPE_SAR) {
        if (mCalibrationData->data_header.msgId == SENSOR_APP_MSG_ID_CAL_RESULT) {
            dummy.sensor_type = mtkTypeToApId(mtk_type);
            dummy.flush_action = CALI_ACTION;
            dummy.sar_event.x_bias = mCalibrationData->values[0];
            dummy.sar_event.y_bias = mCalibrationData->values[1];
            dummy.sar_event.z_bias = mCalibrationData->values[2];
            dummy.sar_event.status = (uint32_t)(int8_t)mCalibrationData->data_header.status;

            if (contextHubSramFifoWrite(&dummy) == SRAM_FIFO_FULL)
                contextHubSramFifoRead();
        }
    }
}

bool SensorQueueEnqueue(struct SimpleQueue* sq, const void *data, int length, bool possiblyDiscardable)
{
    struct mtkActiveSensor *sensor = container_of(data, struct mtkActiveSensor, buffer);

    if (sensor->buffer.sensType == SENS_TYPE_INVALID &&
        sensor->buffer.dataType == HOSTINTF_DATA_TYPE_RESET_REASON) {
        contextHubIpiNotifyAp(0, SENSOR_HUB_NOTIFY, SCP_INIT_DONE, NULL);
        return true;
    } else if (sensor->buffer.sensType == SENS_TYPE_INVALID &&
        sensor->buffer.dataType == HOSTINTF_DATA_TYPE_APP_TO_HOST) {
        hostIntfTransferCaliData(sensor);
    } else {
        hostIntfTransferData(sensor);
    }
    return true;
}

bool SensorQueueDequeue(struct SimpleQueue* sq, void *data)
{
    return true;
}

static int contextHubDispatchCust(uint8_t sensType, SCP_SENSOR_HUB_SET_CUST_REQ *req)
{
    int ret = 0;
    CUST_SET_REQ_P cust_req = &req->cust_set_req;

    switch (cust_req->cust.action) {
        case CUST_ACTION_SET_CALI:
            ret = sensorCoreWriteCalibration(sensType, cust_req->setCali.int32_data);
            break;
        case CUST_ACTION_RESET_CALI:
            ret = sensorCoreResetCalibration(sensType);
            break;
        case CUST_ACTION_SET_PS_THRESHOLD:
            ret = sensorCoreSetThreshold(sensType, cust_req->setPSThreshold.threshold);
            break;
        case CUST_ACTION_SET_TRACE:
            ret = sensorCoreSetDebugTrace(sensType, cust_req->setTrace.trace);
            break;
        case CUST_ACTION_GET_SENSOR_INFO:
            ret = sensorCoreGetSensorInfo(sensType, &cust_req->getInfo.sensorInfo);
            break;
        default:
            break;
    }
    return ret;
}
static int contextHubGetData(uint8_t sensType, struct data_unit_t *data)
{
    int ret = 0;
    //osLog(LOG_DEBUG, "contextHubGetData\n");
    ret = sensorCoreGetData(sensType, data);
    return ret;
}
static int contextHubFwGetData(SCP_SENSOR_HUB_REQ *req,
                               struct data_unit_t *data)
{
    uint8_t mtkType = 0;
    int ret = 0;
    SCP_SENSOR_HUB_GET_DATA_REQ *get_data_req;

    get_data_req = ((SCP_SENSOR_HUB_GET_DATA_REQ *)req);
    mtkType = apIdToMtkType(get_data_req->sensorType);
    ret = contextHubGetData(mtkTypeToChreType(mtkType), data);

    return ret;
}
static int contextHubFwGetDataAck(SCP_SENSOR_HUB_REQ *req,
        SCP_SENSOR_HUB_DATA *rsp, struct data_unit_t *data, int errCode)
{
    rsp->get_data_rsp.sensorType = req->sensorType;
    rsp->get_data_rsp.action = req->action;
    rsp->rsp.errCode = errCode;
    memcpy(rsp->get_data_rsp.data.int8_Data, data, SENSOR_DATA_SIZE);

    return 0;
}

static int contextHubFwSetCfg(SCP_SENSOR_HUB_REQ *req,
                              struct data_unit_t *data)
{
    SCP_SENSOR_HUB_SET_CONFIG_REQ *set_cfg_req;

    set_cfg_req = (SCP_SENSOR_HUB_SET_CONFIG_REQ *)req;
#ifdef CFG_VCORE_DVFS_SUPPORT
    dvfs_enable_DRAM_resource(SENS_MEM_ID);
#endif
    get_emi_semaphore();
    mContextHubDramFifo.bufferBase = (struct sensorFIFO *)ap_to_scp((uint32_t)(set_cfg_req->bufferBase));
    mContextHubDramFifo.maxEventNumber = mContextHubDramFifo.bufferBase->FIFOSize / SENSOR_DATA_SIZE;
    osLog(LOG_INFO, "get dram phy addr=%p,size=%lu, maxEventNumber:%lu\n",
          mContextHubDramFifo.bufferBase, mContextHubDramFifo.bufferBase->FIFOSize,
          mContextHubDramFifo.maxEventNumber);
    osLog(LOG_INFO, "get dram phy rp=%lu,wp=%lu\n",
          mContextHubDramFifo.bufferBase->rp, mContextHubDramFifo.bufferBase->wp);
    release_emi_semaphore();
#ifdef CFG_VCORE_DVFS_SUPPORT
    dvfs_disable_DRAM_resource(SENS_MEM_ID);
#endif
    return 0;
}

static int contextHubFwSetCfgAck(SCP_SENSOR_HUB_REQ *req,
        SCP_SENSOR_HUB_DATA *rsp, struct data_unit_t *data, int errCode)
{
    rsp->get_data_rsp.sensorType = req->sensorType;
    rsp->get_data_rsp.action = req->action;
    rsp->rsp.errCode = errCode;
    return 0;
}

static int contextHubFwSetCust(SCP_SENSOR_HUB_REQ *req,
                               struct data_unit_t *data)
{
    uint8_t mtkType = 0;
    int ret = 0;
    SCP_SENSOR_HUB_SET_CUST_REQ *set_cust_req;

    set_cust_req = (SCP_SENSOR_HUB_SET_CUST_REQ *)req;
    mtkType = apIdToMtkType(set_cust_req->sensorType);
    ret = contextHubDispatchCust(mtkTypeToChreType(mtkType), set_cust_req);
    return ret;
}

static int contextHubFwSetCustAck(SCP_SENSOR_HUB_REQ *req,
        SCP_SENSOR_HUB_DATA *rsp, struct data_unit_t *data, int errCode)
{
    SCP_SENSOR_HUB_SET_CUST_REQ *set_cust_req;
    set_cust_req = (SCP_SENSOR_HUB_SET_CUST_REQ *)req;


    rsp->set_cust_rsp.sensorType = req->sensorType;
    rsp->set_cust_rsp.action = req->action;
    rsp->set_cust_rsp.cust_set_rsp.getInfo.action =
                            set_cust_req->cust_set_req.getInfo.action;
    rsp->rsp.errCode = errCode;
    memcpy(&rsp->set_cust_rsp.cust_set_rsp.getInfo.sensorInfo,
        &set_cust_req->cust_set_req.getInfo.sensorInfo, sizeof(struct sensorInfo_t));
    return 0;
}
static int contextHubFwSyncTimestamp(SCP_SENSOR_HUB_REQ *req,
                                     struct data_unit_t *data)
{
    SCP_SENSOR_HUB_SET_CONFIG_REQ *set_cfg_req;
    uint64_t scp_now_time = 0, ap_now_time = 0;
    uint64_t arch_counter = 0, counter_elapse = 0, ipi_transfer_time = 0;
    uint64_t state = 0;

    set_cfg_req = (SCP_SENSOR_HUB_SET_CONFIG_REQ *)req;

    state = cpuIntsOff();
    scp_now_time = read_xgpt_stamp_ns();
    arch_counter = timer_get_global_timer_tick();
    cpuIntsRestore(state);

    counter_elapse = arch_counter - set_cfg_req->arch_counter;
    ipi_transfer_time = arch_counter_to_ns(counter_elapse);
    ap_now_time = set_cfg_req->ap_timestamp + ipi_transfer_time;
    /*
     * old timestamp sync algorithm between ap and scp
     * correct time_stamp_offset to avoid timestamp jump too large, then
     * lead timestamp out of order, now this feature is only for debug, we
     * have a new time sync algo for ap and scp timestamp sync.
     */
    osLog(LOG_INFO, "sync time scp:%lld, ap:%lld, offset:%lld\n",
        scp_now_time, set_cfg_req->ap_timestamp, ap_now_time - scp_now_time);
    return 0;
}
static int contextHubFwSyncTimestampAck(SCP_SENSOR_HUB_REQ *req,
        SCP_SENSOR_HUB_DATA *rsp, struct data_unit_t *data, int errCode)
{
    rsp->get_data_rsp.sensorType = req->sensorType;
    rsp->get_data_rsp.action = req->action;
    rsp->rsp.errCode = errCode;
    return 0;
}

const static struct ContextHubCmd mContextHubCmds[] = {
    CONTEXTHUB_CMD(SENSOR_HUB_GET_DATA,
        contextHubFwGetData, contextHubFwGetDataAck),
    CONTEXTHUB_CMD(SENSOR_HUB_SET_CONFIG,
        contextHubFwSetCfg, contextHubFwSetCfgAck),
    CONTEXTHUB_CMD(SENSOR_HUB_SET_CUST,
        contextHubFwSetCust, contextHubFwSetCustAck),
    CONTEXTHUB_CMD(SENSOR_HUB_SET_TIMESTAMP,
        contextHubFwSyncTimestamp, contextHubFwSyncTimestampAck),
};
const struct ContextHubCmd *contextHubFindCmd(uint32_t packetReason)
{
    int i;

    for (i = 0; i < ARRAY_SIZE(mContextHubCmds); i++) {
        const struct ContextHubCmd *cmd = &mContextHubCmds[i];
        if (cmd->reason == packetReason)
            return cmd;
    }
    return NULL;
}

static void contextHubIpiHandler(int id, void *data, unsigned int len)
{
    do {
        memcpy(&mTask.ipi_req, (char*)data, sizeof(SCP_SENSOR_HUB_REQ));
    } while (memcmp(data, &mTask.ipi_req, sizeof(SCP_SENSOR_HUB_REQ)));
    osEnqueuePrivateEvt(EVT_IPI_RX, NULL, NULL, mTask.id);
    extern TaskHandle_t CHRE_TaskHandle;
    if (xTaskResumeFromISR(CHRE_TaskHandle) == pdTRUE) {
        portYIELD_WITHIN_API();
    }
}

static int contextHubHandleIpiTxEvent(void)
{
    ipi_status ipi_ret;
    uint8_t *wakeup;
    uint64_t time_now = 0;
    static uint32_t direct_push_count = 0;
    static uint64_t direct_push_time_last= 0;

    //osLog(LOG_DEBUG, "contextHubHandleIpiTxEvent\n");
    if (UNLIKELY(mContextHubIpi.tail == mContextHubIpi.head))
        return 0;
    ipi_ret = scp_ipi_send(IPI_SENSOR,
        (void *)&mContextHubIpi.ringBuf[mContextHubIpi.tail], SENSOR_IPI_SIZE, 0, IPI_SCP2AP);
    if (ipi_ret != DONE) {
        if (atomicCmpXchgByte(&mContextHubIpi.isTimerRunning, STATE_STOP, STATE_START)) {
            osLog(LOG_INFO, "scp_ipi_send failed, enable a oneshot timer\n");
            /* wake lock for timer callback come, this can lower sensor data latency about 1.3ms which is exit idle time */
            wake_lock(&mContextHubIpi.wakeLock);
            mContextHubIpi.timerHandle = timTimerSet(1000000, 0, 50, contextHubipiTimerCallback, NULL, true);
            if (mContextHubIpi.timerHandle == 0) {
                osLog(LOG_ERROR, "contexthubfw cann't alloc a chre timer\n");
                configASSERT(0);
            }
        }
    } else {
        if (hostIntfGetInterruptMask(NANOHUB_INT_NONWAKEUP)) {
            wakeup = (uint8_t *)&mContextHubIpi.ringBuf[mContextHubIpi.tail];
            if (wakeup[1] == SENSOR_HUB_NOTIFY && wakeup[2] == SCP_DIRECT_PUSH) {
                time_now = rtcGetTime();
                direct_push_count++;
                if (time_now - direct_push_time_last > 5000000000) {
                    osLog(LOG_INFO, "scp==>ap(%d, %d, %d, %d), duration:%lld, times:%lu, wlk:0x%llx\n",
                        wakeup[0], wakeup[1], wakeup[2], wakeup[3],
                        time_now - direct_push_time_last,
                        direct_push_count, timer_get_global_timer_tick());
                    direct_push_time_last = time_now;
                    direct_push_count = 0;
                }
            } else
                osLog(LOG_INFO, "scp==>ap(%d, %d, %d, %d), wlk:0x%llx\n", wakeup[0],
                    wakeup[1], wakeup[2], wakeup[3], timer_get_global_timer_tick());
        }
        /*
         * only timer callback come we can wake unlock, if always wake unlock,
         * timer callback may block by idle time for 1.3ms, sensor data latency
         * increase 1.3ms
         */
        if (atomicReadByte(&mContextHubIpi.isTimerRunning) == STATE_STOP)
            wake_unlock(&mContextHubIpi.wakeLock);
        mContextHubIpi.tail++;
        mContextHubIpi.tail &= mContextHubIpi.size - 1;
        if (mContextHubIpi.tail != mContextHubIpi.head)
            contextHubIpiTxStart();
    }
    return 0;
}
static void contextHubHandleIpiRxEvent(void)
{
    int ret = 0;
    struct data_unit_t data;
    const struct ContextHubCmd *cmd;

    cmd = contextHubFindCmd(mTask.ipi_req.action);
    if (cmd == NULL) {
        osLog(LOG_ERROR, "contextHubHandleIpiRxEvent cannot find cmd\n");
        return;
    }
    ret = cmd->handler(&mTask.ipi_req, &data);
    ret = contextHubIpiRxAck(cmd, &mTask.ipi_req, &data, ret);
    if (ret < 0)
        osLog(LOG_ERROR, "ERR: contextHubIpiRxAck failed!\n");
}

static void contextHubFwHandleEvent(uint32_t evtType, const void* evtData)
{
    if (evtData == SENSOR_DATA_EVENT_FLUSH)
        return;

    switch (evtType) {
        case EVT_IPI_RX: {
            contextHubHandleIpiRxEvent();
            break;
        }

        case EVT_IPI_TX: {
            contextHubHandleIpiTxEvent();
            break;
        }

        case EVT_DRAM: {
            wake_unlock(&mContextHubDramFifo.wakeLock);
            contextHubRealTransfer((uint32_t)evtData);
            mark_timestamp(SENS_TYPE_ACCEL, SENS_TYPE_ACCEL, HOSTWAKEUP, rtcGetTime());
            mark_timestamp(SENS_TYPE_GYRO, SENS_TYPE_GYRO, HOSTWAKEUP, rtcGetTime());
            break;
        }
    }
}
//#define CONTEXTHUB_TESTSUITE_CASE


static bool contextHubFwStart(uint32_t tid)
{
    ipi_status ipi_ret;

    osLog(LOG_INFO, "contexthub_fw_start tid: %lu\n", tid);
    if (sizeof(struct data_unit_t) != SENSOR_DATA_SIZE || sizeof(SCP_SENSOR_HUB_DATA) != SENSOR_IPI_SIZE) {
        osLog(LOG_ERROR, "data_unit_t size: %d, SCP_SENSOR_HUB_DATA size: %d\n",
            sizeof(struct data_unit_t), sizeof(SCP_SENSOR_HUB_DATA));
        configASSERT(0);
    }
    ipi_ret = scp_ipi_registration(IPI_SENSOR, contextHubIpiHandler, "chre_fw_ipi");
    scp_ipi_wakeup_ap_registration(IPI_SENSOR);
    if (ipi_ret != DONE)
        return false;

    mTask.id = tid;

    sensorCoreInit();
    extern void virtualSensorCoreInit();
    virtualSensorCoreInit();
    contextHubIpiInit();
    contextHubSramFifoInit();
    contextHubDramFifoInit();
    initSensorReportingInfo();
    return true;
}
static void contextHubFwEnd(void)
{
}

INTERNAL_APP_INIT(
    APP_ID_MAKE(APP_ID_VENDOR_MTK, MTK_APP_ID_WRAP(SENS_TYPE_INVALID, 0, 0)),
    0,
    contextHubFwStart,
    contextHubFwEnd,
    contextHubFwHandleEvent);

