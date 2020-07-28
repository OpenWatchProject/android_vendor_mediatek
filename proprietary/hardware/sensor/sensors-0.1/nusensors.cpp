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

#include "nusensors.h"
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

#ifdef LOG_TAG
#undef LOG_TAG
#define LOG_TAG "Sensors"
#endif
/*****************************************************************************/

struct sensors_poll_context_t {
    struct sensors_poll_device_1 device;// must be first

        sensors_poll_context_t();
        ~sensors_poll_context_t();
    int activate(int handle, int enabled);
    int setDelay(int handle, int64_t ns);
    int pollEvents(sensors_event_t* data, int count);
    int batch(int handle, int flags, int64_t samplingPeriodNs, int64_t maxBatchReportLatencyNs);
    int flush(int handle);

private:
    enum {
        accel,
        magnetic,
        gyro,
        light,
        proximity,
        pressure,
        humidity,
        stepcounter,
        pedometer,
        activity,
        situation,
        fusion,
        bio,
        numFds,
    };

    int handleToDriver(int handle) const {
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
            case ID_GYROSCOPE_UNCALIBRATED:
            case ID_MAGNETIC_UNCALIBRATED:
                 return fusion;
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
                  return situation;
            case ID_EKG:
            case ID_PPG1:
            case ID_PPG2:
                  return bio;
            default:
                break;
        }
        return -EINVAL;
    }

    struct pollfd mPollFds[numFds];
    SensorBase* mSensors[numFds];
};
/*****************************************************************************/

sensors_poll_context_t::sensors_poll_context_t()
{
    memset(&device, 0, sizeof(device));

    mSensors[accel] = new AccelerationSensor();
    mPollFds[accel].fd = ((AccelerationSensor*)mSensors[accel])->mdata_fd;
    mPollFds[accel].events = POLLIN;
    mPollFds[accel].revents = 0;

    mSensors[magnetic] = new MagneticSensor();
    mPollFds[magnetic].fd = ((MagneticSensor*)mSensors[magnetic])->mdata_fd;
    mPollFds[magnetic].events = POLLIN;
    mPollFds[magnetic].revents = 0;

    mSensors[proximity] = new ProximitySensor();
    mPollFds[proximity].fd = ((ProximitySensor*)mSensors[proximity])->mdata_fd;
    mPollFds[proximity].events = POLLIN;
    mPollFds[proximity].revents = 0;

    mSensors[light] = new AmbiLightSensor();
    mPollFds[light].fd = ((AmbiLightSensor*)mSensors[light])->mdata_fd;
    mPollFds[light].events = POLLIN;
    mPollFds[light].revents = 0;

    mSensors[gyro] = new GyroscopeSensor();
    mPollFds[gyro].fd = ((GyroscopeSensor*)mSensors[gyro])->mdata_fd;
    mPollFds[gyro].events = POLLIN;
    mPollFds[gyro].revents = 0;

    mSensors[pressure] = new PressureSensor();
    mPollFds[pressure].fd = ((PressureSensor*)mSensors[pressure])->mdata_fd;
    mPollFds[pressure].events = POLLIN;
    mPollFds[pressure].revents = 0;

    mSensors[humidity] = new HumiditySensor();
    mPollFds[humidity].fd = ((HumiditySensor*)mSensors[humidity])->mdata_fd;
    mPollFds[humidity].events = POLLIN;
    mPollFds[humidity].revents = 0;

    mSensors[stepcounter] = new StepCounterSensor();
    mPollFds[stepcounter].fd = ((StepCounterSensor*)mSensors[stepcounter])->mdata_fd;
    mPollFds[stepcounter].events = POLLIN;
    mPollFds[stepcounter].revents = 0;

    mSensors[pedometer] = new PedometerSensor();
    mPollFds[pedometer].fd = ((PedometerSensor*)mSensors[pedometer])->mdata_fd;
    mPollFds[pedometer].events = POLLIN;
    mPollFds[pedometer].revents = 0;

    mSensors[activity] = new ActivitySensor();
    mPollFds[activity].fd = ((ActivitySensor*)mSensors[activity])->mdata_fd;
    mPollFds[activity].events = POLLIN;
    mPollFds[activity].revents = 0;

    mSensors[situation] = new SituationSensor();
    mPollFds[situation].fd = ((SituationSensor*)mSensors[situation])->mdata_fd;
    mPollFds[situation].events = POLLIN;
    mPollFds[situation].revents = 0;

    mSensors[fusion] = new FusionSensor();
    mPollFds[fusion].fd = ((FusionSensor*)mSensors[fusion])->mdata_fd;
    mPollFds[fusion].events = POLLIN;
    mPollFds[fusion].revents = 0;

    mSensors[bio] = new BiometricSensor();
    mPollFds[bio].fd = ((BiometricSensor*)mSensors[bio])->mdata_fd;
    mPollFds[bio].events = POLLIN;
    mPollFds[bio].revents = 0;
}

sensors_poll_context_t::~sensors_poll_context_t() {
    for (int i = 0; i < numFds; i++) {
        delete mSensors[i];
    }
}

int sensors_poll_context_t::activate(int handle, int enabled) {
    int err = 0;
    int index = handleToDriver(handle);

    if ((index >= numFds) || (index < 0)) {
        ALOGE("activate error index = %d\n", index);
        return 0;
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
        return 0;
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
        return 0;
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
        return 0;
    }
    if (NULL != mSensors[index])
        err = mSensors[index]->flush(handle);
    return err;
}

int sensors_poll_context_t::pollEvents(sensors_event_t* data, int count) {
    int nbEvents = 0;
    int n = 0;

    do {
        for (int i = 0; count && i < numFds; i++) {
            SensorBase* const sensor(mSensors[i]);
            if ((mPollFds[i].revents & POLLIN) || (sensor->hasPendingEvents())) {
                int nb = sensor->readEvents(data, count);

                if (nb < count) {
                    // no more data for this sensor
                    mPollFds[i].revents = 0;
                }

                for (int j = 0; j < nb; j++) {
                    if (data[j].type == SENSOR_TYPE_META_DATA)
                        data[j].meta_data.sensor += ID_OFFSET;
                    else
                        data[j].sensor += ID_OFFSET;
                }
                if(nb < 0 || nb > count)
                    ALOGE("pollEvents count error nb:%d, count:%d, nbEvents:%d", nb, count, nbEvents);
                count -= nb;
                nbEvents += nb;
                data += nb;
                //if(nb < 0||nb > count)
                //ALOGE("sensor: %d, pollEvents count nb:%d, count:%d, nbEvents:%d",i, nb, count, nbEvents);
            }
        }
        if (count) {
            // we still have some room, so try to see if we can get
            // some events immediately or just wait if we don't have
            // anything to return
            n = TEMP_FAILURE_RETRY(poll(mPollFds, numFds, nbEvents ? 0 : -1));
            if (n < 0) {
                int err;
                err = errno;
                ALOGE("poll() failed (%s)", strerror(errno));
                return -err;
            }
        }
        // if we have events and space, go read them
    } while (n && count);

    return nbEvents;
}

/*****************************************************************************/

static int poll__close(struct hw_device_t *dev) {
    sensors_poll_context_t *ctx = (sensors_poll_context_t *)dev;
    if (ctx) {
        delete ctx;
    }
    return 0;
}

static int poll__activate(struct sensors_poll_device_t *dev,
        int handle, int enabled) {
    sensors_poll_context_t *ctx = (sensors_poll_context_t *)dev;
    return ctx->activate(handle - ID_OFFSET, enabled);
}

static int poll__setDelay(struct sensors_poll_device_t *dev,
        int handle, int64_t ns) {
    sensors_poll_context_t *ctx = (sensors_poll_context_t *)dev;
    return ctx->setDelay(handle - ID_OFFSET, ns);
}

static int poll__poll(struct sensors_poll_device_t *dev,
        sensors_event_t* data, int count) {
    sensors_poll_context_t *ctx = (sensors_poll_context_t *)dev;
    return ctx->pollEvents(data, count);
}

static int poll__batch(struct sensors_poll_device_1 *dev,
        int handle, int flags, int64_t samplingPeriodNs, int64_t maxBatchReportLatencyNs) {
    sensors_poll_context_t *ctx = (sensors_poll_context_t *)dev;
    return ctx->batch(handle - ID_OFFSET, flags, samplingPeriodNs, maxBatchReportLatencyNs);
}

static int poll__flush(struct sensors_poll_device_1 *dev,
        int handle) {
    sensors_poll_context_t *ctx = (sensors_poll_context_t *)dev;
    return ctx->flush(handle - ID_OFFSET);
}

/*****************************************************************************/

int init_nusensors(hw_module_t const* module, hw_device_t** device)
{
    int status = -EINVAL;
    sensors_poll_context_t *dev;

    dev = new sensors_poll_context_t();
    memset(&dev->device, 0, sizeof(sensors_poll_device_1));

    dev->device.common.tag      = HARDWARE_DEVICE_TAG;
    dev->device.common.version  = SENSORS_DEVICE_API_VERSION_1_3;
    dev->device.common.module   = const_cast<hw_module_t*>(module);
    dev->device.common.close    = poll__close;
    dev->device.activate        = poll__activate;
    dev->device.setDelay        = poll__setDelay;
    dev->device.poll            = poll__poll;
    dev->device.batch           = poll__batch;
    dev->device.flush           = poll__flush;

    *device = &dev->device.common;
    status = 0;
    return status;
}
