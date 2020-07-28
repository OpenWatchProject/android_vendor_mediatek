/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2012. All rights reserved.
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


#include <hardware/sensors.h>
#include <fcntl.h>
#include <errno.h>
#include <dirent.h>
#include <math.h>
#include <string.h>

#include <poll.h>
#include <pthread.h>

#include <linux/input.h>

#include <cutils/atomic.h>
#include <log/log.h>
#include <inttypes.h>

#include "SensorContext.h"
#include "SensorList.h"
#include "Acceleration.h"
#include "Magnetic.h"
#include "Proximity.h"
#include "Pressure.h"
#include "Humidity.h"
#include "Gyroscope.h"
#include "AmbienteLight.h"
#include "StepCounter.h"

#include "Activity.h"
#include "Pedometer.h"

#include "Situation.h"
#include "Fusion.h"
#include "Biometric.h"
#include "WakeUpSet.h"

#undef LOG_TAG
#define LOG_TAG "SensorContext"

static int dynamicFusionSelect(void) {
    int fd = scpfusion;

#ifdef MAG_CALIBRATION_IN_SENSORHUB
    fd = scpfusion;
#else
    fd = apfusion;
#endif
    return fd;
}

static int dynamicUncaliGyroSelect(void) {
    int fd = scpfusion;

    if (SensorList::getInstance()->hwGyroSupported())
        fd = scpfusion;
    else
        fd = apfusion;
    return fd;
}

static int dynamicUncaliMagSelect(void) {
    int fd = scpfusion;

#ifdef MAG_CALIBRATION_IN_SENSORHUB
    fd = scpfusion;
#else
    fd = apfusion;
#endif
    return fd;
}

static int handleToDriver(int handle) {
    switch (handle) {
        case ID_ACCELEROMETER:
            return accel;
        case ID_MAGNETIC:
            return magnetic;
        case ID_PROXIMITY:
            return proximity;
        case ID_LIGHT:
        case ID_RGBW:
            return light;
        case ID_GYROSCOPE:
            return gyro;
        case ID_PRESSURE:
            return pressure;
        case ID_RELATIVE_HUMIDITY:
            return humidity;
        case ID_STEP_COUNTER:
        case ID_STEP_DETECTOR:
        case ID_SIGNIFICANT_MOTION:
        case ID_FLOOR_COUNTER:
            return stepcounter;
        case ID_PEDOMETER:
            return pedometer;
        case ID_ACTIVITY:
            return activity;
        case ID_LINEAR_ACCELERATION:
        case ID_ROTATION_VECTOR:
        case ID_GAME_ROTATION_VECTOR:
        case ID_GRAVITY:
        case ID_GEOMAGNETIC_ROTATION_VECTOR:
        case ID_ORIENTATION:
        case ID_PDR:
            return dynamicFusionSelect();
        case ID_ACCELEROMETER_UNCALIBRATED:
            return scpfusion;
        case ID_GYROSCOPE_UNCALIBRATED:
        case ID_GYRO_TEMPERATURE:
            return dynamicUncaliGyroSelect();
        case ID_MAGNETIC_UNCALIBRATED:
            return dynamicUncaliMagSelect();
        case ID_FACE_DOWN:
        case ID_SHAKE:
        case ID_ANSWER_CALL:
        case ID_IN_POCKET:
        case ID_STATIONARY_DETECT:
        case ID_MOTION_DETECT:
        case ID_DEVICE_ORIENTATION:
        case ID_WAKE_GESTURE:
        case ID_PICK_UP_GESTURE:
        case ID_GLANCE_GESTURE:
        case ID_TILT_DETECTOR:
        case ID_FLAT:
        case ID_SAR:
            return situation;
        case ID_EKG:
        case ID_PPG1:
        case ID_PPG2:
            return bio;
        case ID_STEP_DETECTOR_WAKEUP:
            return wakeupset;
        default:
            break;
    }
    return -EINVAL;
}

sensors_poll_context_t *sensors_poll_context_t::contextInstance = nullptr;
sensors_poll_context_t *sensors_poll_context_t::getInstance() {
    if (contextInstance == nullptr) {
        sensors_poll_context_t *context = new sensors_poll_context_t;
        contextInstance = context;
    }
    return contextInstance;
}

sensors_poll_context_t::sensors_poll_context_t()
{
    ALOGE("sensors_poll_context_t constructor.\n");

    memset(&device, 0, sizeof(device));

    mSensors[accel] = new AccelerationSensor();
    mPollFds[accel].fd = mSensors[accel]->getFd();
    mPollFds[accel].events = POLLIN;
    mPollFds[accel].revents = 0;

    mSensors[magnetic] = new MagneticSensor();
    mPollFds[magnetic].fd = mSensors[magnetic]->getFd();
    mPollFds[magnetic].events = POLLIN;
    mPollFds[magnetic].revents = 0;

    mSensors[proximity] = new ProximitySensor();
    mPollFds[proximity].fd = mSensors[proximity]->getFd();
    mPollFds[proximity].events = POLLIN;
    mPollFds[proximity].revents = 0;

    mSensors[light] = new AmbiLightSensor();
    mPollFds[light].fd = mSensors[light]->getFd();
    mPollFds[light].events = POLLIN;
    mPollFds[light].revents = 0;

    if (SensorList::getInstance()->hwGyroSupported()) {
        mSensors[gyro] = new HwGyroscopeSensor();
        mPollFds[gyro].fd = mSensors[gyro]->getFd();
        mPollFds[gyro].events = POLLIN;
        mPollFds[gyro].revents = 0;
    } else {
        mSensors[gyro] = new VirtGyroscopeSensor();
        mPollFds[gyro].fd = mSensors[gyro]->getFd();
        mPollFds[gyro].events = POLLIN;
        mPollFds[gyro].revents = 0;
    }

    mSensors[pressure] = new PressureSensor();
    mPollFds[pressure].fd = mSensors[pressure]->getFd();
    mPollFds[pressure].events = POLLIN;
    mPollFds[pressure].revents = 0;

    mSensors[humidity] = new HumiditySensor();
    mPollFds[humidity].fd = mSensors[humidity]->getFd();
    mPollFds[humidity].events = POLLIN;
    mPollFds[humidity].revents = 0;

    mSensors[stepcounter] = new StepCounterSensor();
    mPollFds[stepcounter].fd = mSensors[stepcounter]->getFd();
    mPollFds[stepcounter].events = POLLIN;
    mPollFds[stepcounter].revents = 0;

    mSensors[pedometer] = new PedometerSensor();
    mPollFds[pedometer].fd = mSensors[pedometer]->getFd();
    mPollFds[pedometer].events = POLLIN;
    mPollFds[pedometer].revents = 0;

    mSensors[activity] = new ActivitySensor();
    mPollFds[activity].fd = mSensors[activity]->getFd();
    mPollFds[activity].events = POLLIN;
    mPollFds[activity].revents = 0;

    mSensors[situation] = new SituationSensor();
    mPollFds[situation].fd = mSensors[situation]->getFd();
    mPollFds[situation].events = POLLIN;
    mPollFds[situation].revents = 0;

    mSensors[scpfusion] = new ScpFusionSensor();
    mPollFds[scpfusion].fd = mSensors[scpfusion]->getFd();
    mPollFds[scpfusion].events = POLLIN;
    mPollFds[scpfusion].revents = 0;

    mSensors[apfusion] = new ApFusionSensor();
    mPollFds[apfusion].fd = mSensors[apfusion]->getFd();
    mPollFds[apfusion].events = POLLIN;
    mPollFds[apfusion].revents = 0;

    mSensors[bio] = new BiometricSensor();
    mPollFds[bio].fd = mSensors[bio]->getFd();
    mPollFds[bio].events = POLLIN;
    mPollFds[bio].revents = 0;

    mSensors[wakeupset] = new WakeUpSetSensor();
    mPollFds[wakeupset].fd = mSensors[wakeupset]->getFd();
    mPollFds[wakeupset].events = POLLIN;
    mPollFds[wakeupset].revents = 0;
}

sensors_poll_context_t::~sensors_poll_context_t() {
    for (int i = 0; i < numFds; i++) {
        delete mSensors[i];
    }
    contextInstance = nullptr;
}

int sensors_poll_context_t::activate(int handle, int enabled) {
    int err = 0;
    int index = handleToDriver(handle);

    if ((index >= numFds) || (index < 0)) {
        ALOGE("activate error index = %d\n", index);
        return -1;
    }
    if (NULL != mSensors[index])
        err = mSensors[index]->enable(handle, enabled);
    return err;
}

int sensors_poll_context_t::setDelay(int handle, int64_t ns) {
    int err = 0;
    int index = handleToDriver(handle);

    if ((index >= numFds) || (index < 0)) {
        ALOGE("setDelay error index = %d\n", index);
        return -1;
    }
    if (NULL != mSensors[index])
       err = mSensors[index]->batch(handle, 0, ns, 0);
    return err;
}

int sensors_poll_context_t::batch(int handle, int flags,
    int64_t samplingPeriodNs, int64_t maxBatchReportLatencyNs) {
    int err = 0;
    int index = handleToDriver(handle);

    if ((index >= numFds) || (index < 0)) {
        ALOGE("batch error index = %d\n", index);
        return -1;
    }
    if (NULL != mSensors[index])
        err = mSensors[index]->batch(handle, flags, samplingPeriodNs, maxBatchReportLatencyNs);
    return err;
}
int sensors_poll_context_t::flush(int handle) {
    int err = 0;
    int index = handleToDriver(handle);

    if ((index >= numFds) || (index < 0)) {
        ALOGE("flush error index = %d\n", index);
        return -1;
    }
    if (NULL != mSensors[index])
        err = mSensors[index]->flush(handle);
    else
        err = -1;

    return err;
}
void sensors_poll_context_t::computeCountForEachFd(int count, int *average, int *loop) {
    int pendingFd = 0;

    for (int i = 0; i < numFds; i++) {
        SensorBase* const sensor(mSensors[i]);
        if (mPollFds[i].revents & POLLIN || sensor->pendingEvent())
            pendingFd++;
    }
    int temp = pendingFd ? count / pendingFd : count;
    *average = temp ? temp : count;
    *loop = (count < pendingFd) ? 1 : pendingFd;
}
int sensors_poll_context_t::pollEvent(sensors_event_t* data, int count) {
    int nbEvents = 0;
    int n = 0;
    int averageCount = 0, loop = 0, loopcount = 0;
    int backupcount = count, backuploop = 0;

    do {
        loopcount++;
        computeCountForEachFd(count, &averageCount, &loop);
        backuploop = loop;
        for (int i = 0; count && loop && i < numFds; i++) {
            SensorBase* const sensor(mSensors[i]);
            if (mPollFds[i].revents & POLLIN || sensor->pendingEvent()) {
                int nb = sensor->readEvents(data, averageCount);
                if (nb < 0 || nb > averageCount) {
                    ALOGE("pollEvents nb:%d, averageCount:%d, loop:%d, "
                              "backupcount:%d, loopcount:%d, backuploop:%d\n",
                        nb, averageCount, loop, backupcount, loopcount, backuploop);
                    abort();
                }
                for (int j = 0; j < nb; j++) {
                    if (data[j].type == SENSOR_TYPE_META_DATA)
                        data[j].meta_data.sensor += ID_OFFSET;
                    else
                        data[j].sensor += ID_OFFSET;
                }
                count -= nb;
                nbEvents += nb;
                data += nb;
                loop--;
                if (count < 0) {
                    ALOGE("pollEvents count:%d, nbEvents:%d, nb:%d, "
                              "averageCount:%d, loop:%d, backupcount:%d, "
                                  "loopcount:%d, backuploop:%d\n",
                        count, nbEvents, nb, averageCount, loop, backupcount,
                            loopcount, backuploop);
                    abort();
                }
            }
        }
        // try to see if we can get some events immediately or just wait if
        // we don't have anything to return, important to update fd revents
        // which sensor data pending in buffer and aviod one sensor always
        // occupy poll bandwidth.
        n = TEMP_FAILURE_RETRY(poll(mPollFds, numFds, nbEvents ? 0 : -1));
        if (n < 0) {
            ALOGE("poll() failed (%s)", strerror(errno));
            return -errno;
        }
    } while (n && count);
    return nbEvents;
}
int sensors_poll_context_t::setEvent(sensors_event_t *data, int moudle, int action) {
    int err = 0;

    if ((moudle >= numFds) || (moudle < 0)) {
        ALOGE("setEvent error index = %d\n", moudle);
        return 0;
    }
    if (NULL != mSensors[moudle]) {
        if (action == DATA_ACTION)
            err = mSensors[moudle]->setEvent(data);
        else if (action == FLUSH_ACTION)
            err = mSensors[moudle]->setFlushEvent(data);
    }

    return err;
}
