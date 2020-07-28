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


#ifndef _SENSOR_MANAGER_H_

#define _SENSOR_MANAGER_H_

#include <hardware/sensors.h>
#include <utils/Log.h>
#include <utils/Mutex.h>
#include <utils/SortedVector.h>
#include <utils/KeyedVector.h>
#include <linux/hwmsensor.h>

#include <memory>
#include <mutex>
#include <map>

#include "SensorContext.h"
#include "SensorEventReader.h"

class SensorConnection {
public:
    SensorConnection();
    ~SensorConnection();
    bool addActiveHandle(uint32_t handle);
    bool removeActiveHandle(uint32_t handle);
    bool hasHandleIndexOf(uint32_t handle);
    size_t getNumActiveHandles() const { return mActiveHandles.size(); }
    void setMoudle(int moudle);
    int getMoudle(void);
    void flushCountAdd(int32_t sensor_handle);
    bool flushCountDec(int32_t sensor_handle);

private:
    android::SortedVector<uint32_t> mActiveHandles;
    int mMoudle;
    std::map<int32_t, int32_t> mFlushCnt;
    std::mutex mFlushMutex;
};

struct SensorManager {
public:
    ~SensorManager();
    static SensorManager *getInstance();
    SensorConnection* createSensorConnection(int mSensorMoudle);
    void removeSensorConnection(SensorConnection* connection);
    void addSensorsList(sensor_t const *list, size_t count);
    int activate(SensorConnection *connection, int32_t sensor_handle, bool enabled);
    int batch(SensorConnection *connection, int32_t sensor_handle,
            int64_t sampling_period_ns,
            int64_t max_report_latency_ns);
    int flush(SensorConnection *connection, int32_t sensor_handle);
    int pollEvent(sensors_event_t* data, int count);
    int setEvent(sensors_event_t* data, int moudle, int action);
    void setNativeConnection(SensorConnection *connection);
    void setSensorContext(sensors_poll_context_t *context);
    void setDirectConnection(SensorConnection *connection);

protected:
    SensorManager();
    SensorManager(const SensorManager& other);
    SensorManager& operator = (const SensorManager& other);
    int64_t getSensorMinDelayNs(int handle);
    int64_t getSensorMaxDelayNs(int handle);
    int32_t getSensorReportMode(int handle);
    void autoDisable(sensors_event_t *data, size_t count);
    size_t parsePollData(sensors_event_t *data,
        sensors_event_t *pollData, size_t pollCount);

private:
    static constexpr int pollMaxBufferSize = 128;
    std::unique_ptr<sensors_event_t[]> pollBuffer;
    SensorConnection *mNativeConnection;
    SensorConnection *mDirectConnection;
    android::SortedVector<SensorConnection *> mActiveConnections;
    android::Mutex mActiveConnectionsLock;
    std::recursive_mutex mLock;
    //android::Mutex mBatchParamsLock;
    struct BatchParams {
      int flags;
      nsecs_t batchDelay, batchTimeout;
      BatchParams() : flags(0), batchDelay(0), batchTimeout(0) {}
      BatchParams(int flag, nsecs_t delay, nsecs_t timeout): flags(flag), batchDelay(delay),
          batchTimeout(timeout) { }
      bool operator != (const BatchParams& other) {
          return other.batchDelay != batchDelay || other.batchTimeout != batchTimeout ||
                 other.flags != flags;
      }
    };
    struct Info {
        BatchParams bestBatchParams;
        android::KeyedVector<SensorConnection *, BatchParams> batchParams;
        bool isActive = false;

        Info() : bestBatchParams(0, -1, -1) {}
        int setBatchParamsForIdent(SensorConnection *connection, int flags, int64_t samplingPeriodNs,
                                        int64_t maxBatchReportLatencyNs);
        void selectBatchParams();
        ssize_t removeBatchParamsForIdent(SensorConnection *connection);

        int numActiveClients();
    };
    android::DefaultKeyedVector<int, Info> mActivationCount;
    android::Vector<sensor_t> mSensorList;
    sensors_poll_context_t *mSensorContext;
    static SensorManager *SensorManagerInstance;
};

#endif
