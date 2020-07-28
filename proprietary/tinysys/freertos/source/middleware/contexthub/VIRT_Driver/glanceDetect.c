/* MediaTek Inc. (C) 2017. All rights reserved.
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

#include <stdio.h>
#include "algoConfig.h"
#include "gesture.h"
#include "gesture_setting.h"
#include "FreeRTOS.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <timer.h>
#include <plat/inc/rtc.h>
#include <hostIntf.h>
#include <nanohubPacket.h>
#include <seos.h>
#include <nanohub_math.h>
#include <sensors.h>
#include <limits.h>
#include <performance.h>

#define EVT_SENSOR_ACC  sensorGetMyEventType(SENS_TYPE_ACCEL)
#define EVT_SENSOR_ANY_MOTION    sensorGetMyEventType(SENS_TYPE_ANY_MOTION)
#define EVT_SENSOR_NO_MOTION     sensorGetMyEventType(SENS_TYPE_NO_MOTION)
//#define _GLANCE_CUSTOMIZED_CONFIG_

struct glanceDetectTask {
    uint32_t taskId;
    uint32_t handle;
    uint32_t anyMotionHandle;
    uint32_t noMotionHandle;
    uint32_t accelHandle;
    enum {
        STATE_DISABLED,
        STATE_AWAITING_ANY_MOTION,
        STATE_AWAITING_GESTURE,
        STATE_INIT
    } taskState;
};
static struct glanceDetectTask mTask;
static resampling_t mResample;
static swMotion_t mMotion;
#ifdef _GLANCE_CUSTOMIZED_CONFIG_
static mtk_gesture_config_t customized_parameter = {
    0,              //  ENABLE_NORMAL_LOG
    1,              //  ENABLE_LIFT
    1,              //  ENABLE_GLANCE
    1,              //  ENABLE_EXT_GLANCE_GLANCE
    3000,           //  LIFT_TO_PUT_HOLD_TIME
    200,            //  ORIENTATION_INTERVAL
    38220,          //  SATURATE_ACC_VAL
    60949249,       //  ABSORB_GRAV_L_BOUND
    139405249,      //  ABSORB_GRAV_U_BOUND
    3072,           //  PUT_ANGLE_SENSITIVITY
    5,              //  PUT_STATIC_INTERVAL
    10000,          //  PUT_STATIC_TH
    3,              //  PUT_STATIC_COUNT
    250000,         //  PUT_Y_SENSITIVITY
    171800,         //  PUT_FLAT_Y_VAL
    -171800,        //  PUT_DANG_FLAT_Y_VAL
    0.26794919,     //  PUT_FACEUP_YZ_RATIO
    -1.0000000,     //  PUT_FACEDOWN_YZ_RATIO
    -900000,        //  PUT_FACEDOWN_MAX_Z      (issue: expand pitch angle to 135)
    1000,           //  PUT_TWO_EVENT_INTERVAL
    10,             //  PUT_RANGE_COUNT_BOUND
    1024,           //  FLIP_ANGLE_SENSITIVITY [cos(60):1024, cos(30):3072]
    2000,           //  FLIP_ANGLE_HOLD_TIME
    5,              //  FLIP_STATIC_INTERVAL
    30000,          //  FLIP_STATIC_TH
    3,              //  FLIP_STATIC_COUNT
    -600000,        //  FLIP_RANGE_MAX_REF_Y
    0.0874886,      //  FLIP_ZX_RATIO [1/tan(30) = 1.7320508, 1/tan(80) = 0.1763269, 1/tan(85) = 0.0874886]
    300000,         //  FLIP_SUPPORT_PUT_Y
    2000,           //  FLIP_DISABLE_TIME
    6000,           //  FLIP_SMALL_Y_CHANGE
    50,             //  GLANCE_SEMIVIS_COUNT
    10,             //  GLANCE_VIS_COUNT
    10,             //  GLANCE_INVIS_COUNT
    150000,         //  GLANCE_BIGMOT_VAR
    80000,          //  GLANCE_BIGMOT_Z_VAR
    15,             //  GLANCE_BIGMOT_INTERVAL
    2000,           //  GLANCE_BIGMOT_MAX_DURATION
    1.7320508,      //  GLANCE_VIS_YX_RATIO
    0.4663077,      //  GLANCE_VIS_FACEUP_YZ_RATIO
    3.7320508,      //  GLANCE_VIS_FACEDOWN_Y_Z_RATIO
    0.0000000,      //  GLANCE_SEMIVIS_FACEUP_YZ_RATIO
    3.7320508,      //  GLANCE_SEMIVIS_FACEDOWN_YZ_RATIO
    -0.26794919,    //  GLANCE_DESK_DANG_Y_BOUND
    -693434,        //  EXT_GLANCE_MIN_X
    693434,         //  EXT_GLANCE_MAX_X
    -693434,        //  EXT_GLANCE_MIN_Y
    2147483647,     //  EXT_GLANCE_MAX_Y
    -2147483647,    //  EXT_GLANCE_MIN_Z
    0,              //  EXT_GLANCE_MAX_Z
    5,              //  EXT_GLANCE_STATIC_INTERVAL
    10000,          //  EXT_GLANCE_STATIC_COND_VAR
    3,              //  EXT_GLANCE_STATIC_COND_COUNT
    200000,         //  EXT_GLANCE_X_Y_MOTION_VAR
    2000,           //  EXT_GLANCE_Y_CH_VAL
    1000,           //  EXT_GLANCE_Y_CH_INTERVAL
    500000,         //  EXT_GLANCE_X_OSCIL_VAL
    -1480665,       //  EXT_GLANCE_Z_OSCIL_U_VAL
    -480665,        //  EXT_GLANCE_Z_OSCIL_L_VAL
    300,            //  LIFT_TWO_EVENT_INTERVAL
    250,            //  LIFT_REF_INTERVAL
    100,            //  LIFT_INTERVAL
    3364,           //  LIFT_ANGLE_SENSITIVITY
    10000,          //  LIFT_Y_SENSITIVITY
    171800,         //  LIFT_FLAT_Y_VAL
    900000,         //  LIFT_FLAT_Z_VAL
    -490332,        //  LIFT_MIN_X
    490332,         //  LIFT_MAX_X
    100000,         //  LIFT_DESK_MIN_Y
    -171800,        //  LIFT_DANG_MIN_Y
    2147483647,     //  LIFT_MAX_Y
    -900000,        //  LIFT_MIN_Z    (issue: expand pitch angle to 135)
    2147483647,     //  LIFT_MAX_Z
    600000,         //  LIFT_ABAN_REF_Y_VAL 849279: 60 degree
    490332,         //  LIFT_ABAN_REF_Z_VAL
    1393967480896,  //  LIFT_ABAN_REF_U_BOUND (G + 2m/s2)
    609436280896,   //  LIFT_ABAN_REF_L_BOUND (G - 2m/s2)
    150000,         //  LIFT_SMALLMOT_VAR
    15,             //  LIFT_SMALLMOT_INTERVAL
    10000,          //  LIFT_SMALLMOT_X_VAR
    120000,         //  LIFT_SMALLMOT_Y_VAR
    40000,          //  LIFT_SMALLMOT_Z_VAR
    1167834680896,  //  LIFT_HORIZ_MOTION_U_SQ
    0,              //  LIFT_HORIZ_MOTION_L_SQ
    6,              //  LIFT_HORIZ_MOTION_INTERVAL
    250000,         //  LIFT_HORIZ_MOTION_Z_DIFF
    2,              //  LIFT_HORIZ_MOTION_COUNT
    1.7320508,      //  LIFT_YX_RATIO
    0.4663076,      //  LIFT_DESK_YZ_RATIO [tan(15) = 0.2679491, tan(25) = 0.4663076]
    -0.2679492,     //  LIFT_DANG_YZ_RATIO
    -800000,        //  LIFT_FOUND_BETTER_Z
    -100000,        //  LIFT_DESK_DANG_Y_BOUND (-1 m/s2)
    490332,         //  LIFT_AZIMUTH_ROTATION_VAL
    2,              //  LIFT_ABSORB_GRAV_COUNT
    940000,         //  LIFT_INSTANT_Z_BOUND
    200000,         //  LIFT_FREEFALL_TH (447mm/s^2 * 447mm/s^2)
    3000,           //  LIFT_FREEFALL_SUSPEND_INTERVAL (ms)
    5,              //  LIFT_FREEFALL_COUNT
};
#endif
// *****************************************************************************
static void glanceInit()
{
#ifndef _GLANCE_CUSTOMIZED_CONFIG_
    reinit_gesture(&config, (gesture_output_t) {
        0, 1, 0, 0
    });
#else
    reinit_gesture(&customized_parameter, (gesture_output_t) {
        0, 1, 0, 0
    });
#endif
    init_data_resampling(&mResample, GESTURE_INPUT_INTERVAL);
    memset(&mMotion, false, sizeof(swMotion_t));
    osLog(LOG_INFO, "GLANCE INIT DONE\n");
}

void glanceGestureConfig(uint8_t on)
{
#ifdef _GLANCE_CUSTOMIZED_CONFIG_
    customized_parameter.ENABLE_LIFT = on;
    customized_parameter.ENABLE_EXT_GLANCE_GLANCE = on;
#endif
    config.ENABLE_LIFT = on;
    config.ENABLE_EXT_GLANCE_GLANCE = on;
}

static void glanceDetectAlgo(struct TripleAxisDataEvent *ev)
{
    struct TripleAxisDataPoint *sample;
    uint32_t input_time_stamp_ms = ev->referenceTime / TIME_NANO_TO_MILLI;
    uint32_t count = ev->samples[0].firstSample.numSamples;
    uint32_t i;

    for (i = 0; i < count; i++) {
        sample = &ev->samples[i];

        if (i > 0) {
            input_time_stamp_ms += sample->deltaTime / TIME_NANO_TO_MILLI;
        }
        sensor_input_data_resampling_preparation(&mResample, input_time_stamp_ms, EVT_SENSOR_ACCEL);
        mResample.currentAccData = (AccelData_t) {
            (int32_t)(sample->x * 1000), (int32_t)(sample->y * 1000), (int32_t)(sample->z * 1000)
        };
        while (mResample.input_count > 0) {
            algo_input_data_generation(&mResample, EVT_SENSOR_ACCEL);
            glanceGestureConfig(0);
            gesture_output_t gesture_alg_output = gesture_enter_point(mResample.algo_time_stamp, mResample.algoAccData.x,
                                                  mResample.algoAccData.y, mResample.algoAccData.z);
            glanceGestureConfig(1);
            if (gesture_alg_output.original_glance_result) {
                union EmbeddedDataPoint sample;
                sample.idata = 1;
                osEnqueueEvt(sensorGetMyEventType(SENS_TYPE_GLANCE), sample.vptr, NULL);
                osLog(LOG_INFO, "glance notify\n\n");
            }
            mResample.input_count--;
        }
    }
}

static void configAnyMotion(bool on)
{
    uint32_t i;
    const struct SensorInfo *getAnyMotionSrc;
    if (on && (!mTask.anyMotionHandle)) {
        for (i = 0; (getAnyMotionSrc = sensorFind(SENS_TYPE_ANY_MOTION, i, &mTask.anyMotionHandle))
                && (getAnyMotionSrc != NULL); i++) {
            if (sensorRequest(mTask.taskId, mTask.anyMotionHandle, SENSOR_RATE_ONCHANGE, 0)) {
                mMotion.apply_sw_anymotion = !(strcmp(getAnyMotionSrc->sensorName, (char*)SW_AnyMotion));
                osEventSubscribe(mTask.taskId, EVT_SENSOR_ANY_MOTION);
                break;
            }
        }
        if (!getAnyMotionSrc) {
            osLog(LOG_INFO, "[WARNING] Glance: anyMotion sensor is not available\n");
        }
    } else if ((!on) && mTask.anyMotionHandle) {
        sensorRelease(mTask.taskId, mTask.anyMotionHandle);
        mTask.anyMotionHandle = 0;
        osEventUnsubscribe(mTask.taskId, EVT_SENSOR_ANY_MOTION);
    }
}

static void configNoMotion(bool on)
{
    uint32_t i;
    const struct SensorInfo *getNoMotionSrc;


    if (on && (!mTask.noMotionHandle)) {
        for (i = 0; (getNoMotionSrc = sensorFind(SENS_TYPE_NO_MOTION, i, &mTask.noMotionHandle))
                && (getNoMotionSrc != NULL); i++) {
            if (sensorRequest(mTask.taskId, mTask.noMotionHandle, SENSOR_RATE_ONCHANGE, 0)) {
                mMotion.apply_sw_nomotion = !(strcmp(getNoMotionSrc->sensorName, (char*)SW_NoMotion));
                osEventSubscribe(mTask.taskId, EVT_SENSOR_NO_MOTION);
                break;
            }
        }
        if (!getNoMotionSrc) {
            osLog(LOG_INFO, "[WARNING] Glance: noMotion sensor is not available\n");
        }
    } else if ((!on) && mTask.noMotionHandle) {
        sensorRelease(mTask.taskId, mTask.noMotionHandle);
        mTask.noMotionHandle = 0;
        osEventUnsubscribe(mTask.taskId, EVT_SENSOR_NO_MOTION);
    }
}

static void configAccel(bool on, uint32_t acc_class)
{
    uint32_t i;
    bool getAccelSensor = false;
    if (on && (!mTask.accelHandle)) {
        for (i = 0; sensorFind(SENS_TYPE_ACCEL, i, &mTask.accelHandle) != NULL; i++) {
            if (acc_class == NORMAL_ACC_WINDOW) {
                if (sensorRequest(mTask.taskId, mTask.accelHandle, ACCEL_MIN_RATE, ACCEL_NORMAL_LATENCY)) {
                    getAccelSensor = true;
                    osEventSubscribe(mTask.taskId, EVT_SENSOR_ACCEL);
                    break;
                }
            } else {
                if (sensorRequest(mTask.taskId, mTask.accelHandle, ACCEL_MIN_RATE, ACCEL_EXTENDED_LATENCY)) {
                    getAccelSensor = true;
                    osEventSubscribe(mTask.taskId, EVT_SENSOR_ACCEL);
                    break;
                }
            }
        }
        if (!getAccelSensor) {
            osLog(LOG_INFO, "[WARNING] Glance: Accel sensor is not available\n");
        }
    } else if ((!on) && mTask.accelHandle) {
        sensorRelease(mTask.taskId, mTask.accelHandle);
        mTask.accelHandle = 0;
        osEventUnsubscribe(mTask.taskId, EVT_SENSOR_ACCEL);
    }
}

static bool glanceDetectPower(bool on, void *cookie)
{
    osLog(LOG_INFO, "GlanceDetectPower:%d\n", on);
    static bool prev_any_on = false;
    bool any_on = on;
    if ((!prev_any_on) && (any_on)) {
        glanceInit();
        configAccel(true, NORMAL_ACC_WINDOW);
        configAnyMotion(true);
        configNoMotion(true);
        mTask.taskState = STATE_INIT;
    } else if ((prev_any_on) && (!any_on)) {
        configAnyMotion(false);
        configNoMotion(false);
        configAccel(false, NORMAL_ACC_WINDOW);
        configAccel(false, EXTENDED_ACC_WINDOW);
        mTask.taskState = STATE_DISABLED;
    }
    prev_any_on = any_on;
    sensorSignalInternalEvt(mTask.handle, SENSOR_INTERNAL_EVT_POWER_STATE_CHG, on, 0);
    return true;
}

static bool glanceDetectSetRate(uint32_t rate, uint64_t latency, void *cookie)
{
    sensorSignalInternalEvt(mTask.handle, SENSOR_INTERNAL_EVT_RATE_CHG, rate, latency);
    return true;
}

static bool glanceDetectFirmwareUpload(void *cookie)
{
    sensorSignalInternalEvt(mTask.handle, SENSOR_INTERNAL_EVT_FW_STATE_CHG, 1, 0);
    return true;
}

static bool glanceDetectFlush(void *cookie)
{
    return osEnqueueEvt(sensorGetMyEventType(SENS_TYPE_GLANCE), SENSOR_DATA_EVENT_FLUSH, NULL);
}

static void glanceDetectHandleEvent(uint32_t evtType, const void* evtData)
{
    if (evtData == SENSOR_DATA_EVENT_FLUSH) {
        return;
    }

    switch (evtType) {
        case EVT_APP_START:
            osEventUnsubscribe(mTask.taskId, EVT_APP_START);
            osLog(LOG_INFO, "GLANCE EVT_APP_START\n");
            break;

        case EVT_SENSOR_ANY_MOTION:
            if ((mTask.taskState == STATE_AWAITING_ANY_MOTION)  || (mTask.taskState == STATE_INIT)) {
                configNoMotion(true);
                configAnyMotion(false);
                if (mMotion.apply_sw_anymotion) {
                    configAccel(false, EXTENDED_ACC_WINDOW);
                }
                configAccel(true, NORMAL_ACC_WINDOW);
                mTask.taskState = STATE_AWAITING_GESTURE;
            }
            break;

        case EVT_SENSOR_NO_MOTION:
            if ((mTask.taskState == STATE_AWAITING_GESTURE)  || (mTask.taskState == STATE_INIT)) {
                configAnyMotion(true);
                configNoMotion(false);
                configAccel(false, NORMAL_ACC_WINDOW);
                if (mMotion.apply_sw_anymotion) {
                    configAccel(true, EXTENDED_ACC_WINDOW);
                }
                mTask.taskState = STATE_AWAITING_ANY_MOTION;
            }
            break;

        case EVT_SENSOR_ACC:
            mark_timestamp(ACC_GYRO, GESTURE, ALGO_BEGIN, rtcGetTime());
            glanceDetectAlgo((struct TripleAxisDataEvent *)evtData);
            mark_timestamp(ACC_GYRO, GESTURE, ALGO_DONE, rtcGetTime());
            break;

        default:
            break;
    }
}

static const struct SensorInfo mSi = {
    .sensorName = "GLANCE",
    .sensorType = SENS_TYPE_GLANCE,
    .numAxis = NUM_AXIS_EMBEDDED,
    .interrupt = NANOHUB_INT_WAKEUP,
    .minSamples = 20
};

static const struct SensorOps mSops = {
    .sensorPower = glanceDetectPower,
    .sensorFirmwareUpload = glanceDetectFirmwareUpload,
    .sensorSetRate = glanceDetectSetRate,
    .sensorFlush = glanceDetectFlush,
};

static bool glanceDetectStart(uint32_t taskId)
{
    mTask.taskId = taskId;
    mTask.handle = sensorRegister(&mSi, &mSops, NULL, true);;
    osEventSubscribe(taskId, EVT_APP_START);
    return true;
}

static void glanceDetectEnd()
{
}
INTERNAL_APP_INIT(APP_ID_MAKE(APP_ID_VENDOR_MTK, MTK_APP_ID_WRAP(SENS_TYPE_GLANCE, 0, 0)), 0, glanceDetectStart,
                  glanceDetectEnd, glanceDetectHandleEvent);

