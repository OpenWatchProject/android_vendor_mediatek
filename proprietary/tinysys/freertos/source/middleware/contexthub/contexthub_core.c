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
#include <atomicBitset.h>
#include <atomic.h>
#include <sensType.h>
#include <sensors.h>
#include <contexthub_core.h>
#include <contexthub_fw.h>

struct sensorCoreInfo mInfoCoreList[MAX_REGISTERED_SENSORS];
static uint8_t mSensorCoreList[SENS_TYPE_LAST_USER];
ATOMIC_BITSET_DECL(mSensorCoreUsed, MAX_REGISTERED_SENSORS, static);

int sensorCoreResetCalibration(uint8_t sensType)
{
    int32_t cali_sw[AXES_NUM] = {0};
    struct sensorCoreInfo *mCoreInfo;
    int8_t handle = mSensorCoreList[sensType];

    if (!atomicBitsetGetBit(mSensorCoreUsed, handle))
        return -1;
    mCoreInfo = &mInfoCoreList[handle];
    if (!mCoreInfo->setCalibration)
        return -1;
        mCoreInfo->setCalibration(cali_sw, AXES_NUM);
    return 0;
}

int sensorCoreWriteCalibration(uint8_t sensType, int32_t *data)
{
    int32_t cali[AXES_NUM], cali_sw[AXES_NUM];
    struct sensorCoreInfo *mCoreInfo;
    int8_t handle = mSensorCoreList[sensType];
    memset(cali, 0, sizeof(cali));
    memset(cali_sw, 0, sizeof(cali_sw));

    if (!atomicBitsetGetBit(mSensorCoreUsed, handle))
        return -1;
    mCoreInfo = &mInfoCoreList[handle];
    if (!mCoreInfo->getCalibration)
        return -1;
        mCoreInfo->getCalibration(cali_sw, AXES_NUM);
    osLog(LOG_INFO, "read calibration (%ld, %ld, %ld) (%ld, %ld, %ld)\n",
          cali_sw[AXIS_X], cali_sw[AXIS_Y], cali_sw[AXIS_Z], data[AXIS_X], data[AXIS_Y], data[AXIS_Z]);
    osLog(LOG_INFO, "sensitivity:%f, gain:%lu\n", (double)mCoreInfo->sensitivity, mCoreInfo->gain);
    data[AXIS_X] = data[AXIS_X] * mCoreInfo->sensitivity / mCoreInfo->gain;
    data[AXIS_Y] = data[AXIS_Y] * mCoreInfo->sensitivity / mCoreInfo->gain;
    data[AXIS_Z] = data[AXIS_Z] * mCoreInfo->sensitivity / mCoreInfo->gain;

    cali[mCoreInfo->cvt.map[AXIS_X]] = mCoreInfo->cvt.sign[AXIS_X] * cali_sw[AXIS_X];
    cali[mCoreInfo->cvt.map[AXIS_Y]] = mCoreInfo->cvt.sign[AXIS_Y] * cali_sw[AXIS_Y];
    cali[mCoreInfo->cvt.map[AXIS_Z]] = mCoreInfo->cvt.sign[AXIS_Z] * cali_sw[AXIS_Z];

    cali[AXIS_X] += data[AXIS_X];
    cali[AXIS_Y] += data[AXIS_Y];
    cali[AXIS_Z] += data[AXIS_Z];

    cali_sw[AXIS_X] = mCoreInfo->cvt.sign[AXIS_X] * cali[mCoreInfo->cvt.map[AXIS_X]];
    cali_sw[AXIS_Y] = mCoreInfo->cvt.sign[AXIS_Y] * cali[mCoreInfo->cvt.map[AXIS_Y]];
    cali_sw[AXIS_Z] = mCoreInfo->cvt.sign[AXIS_Z] * cali[mCoreInfo->cvt.map[AXIS_Z]];

    osLog(LOG_INFO, "write calibration (%ld, %ld, %ld)\n",
          cali_sw[AXIS_X], cali_sw[AXIS_Y], cali_sw[AXIS_Z]);
    if (!mCoreInfo->setCalibration)
        return -1;
        mCoreInfo->setCalibration(cali_sw, AXES_NUM);

    return 0;
}

int sensorCoreSetThreshold(uint8_t sensType, int32_t *data)
{
    //int err = 0;
    struct sensorCoreInfo *mCoreInfo;
    int8_t handle = mSensorCoreList[sensType];

    if (!atomicBitsetGetBit(mSensorCoreUsed, handle))
        return -1;
    mCoreInfo = &mInfoCoreList[handle];
    if (!mCoreInfo->setThreshold)
        return -1;
    mCoreInfo->setThreshold(data[1], data[0]);
    return 0;
}

int sensorCoreSetDebugTrace(uint8_t sensType, int32_t trace)
{
    //int err = 0;
    struct sensorCoreInfo *mCoreInfo;
    int8_t handle = mSensorCoreList[sensType];

    if (!atomicBitsetGetBit(mSensorCoreUsed, handle))
        return -1;
    mCoreInfo = &mInfoCoreList[handle];
    if (!mCoreInfo->setDebugTrace)
        return -1;
    mCoreInfo->setDebugTrace(trace);
    return 0;
}

int sensorCoreGetData(uint8_t sensType, struct data_unit_t *data)
{
    struct sensorCoreInfo *mCoreInfo;
    struct TripleAxisDataPoint tripleSample;
    struct SingleAxisDataPoint singleSample;
    int8_t handle = mSensorCoreList[sensType];

    if (!atomicBitsetGetBit(mSensorCoreUsed, handle))
        return -1;
    mCoreInfo = &mInfoCoreList[handle];
    switch (sensType) {
        case SENS_TYPE_ACCEL:
        case SENS_TYPE_GYRO:
        case SENS_TYPE_MAG:
        case SENS_TYPE_SAR:
            if (!mCoreInfo->getData)
            return -1;
                mCoreInfo->getData(&tripleSample);
            data->value[0] = tripleSample.ix;
            data->value[1] = tripleSample.iy;
            data->value[2] = tripleSample.iz;
            break;
        case SENS_TYPE_ALS:
        case SENS_TYPE_BARO:
        case SENS_TYPE_STEP_COUNT:
        case SENS_TYPE_FLOOR_COUNT:
        case SENS_TYPE_ACTIVITY:
            if (!mCoreInfo->getData)
            return -1;
                mCoreInfo->getData(&singleSample);
            data->value[0] = singleSample.idata;
            break;
        case SENS_TYPE_PROX:
            if (!mCoreInfo->getData)
            return -1;
                mCoreInfo->getData(&tripleSample);
            data->value[0] = tripleSample.ix; //raw data
            data->value[1] = tripleSample.iy; //status
            break;
    }
    return 0;
}
int sensorCoreGetSensorInfo(uint8_t sensType, struct sensorInfo_t *data)
{
    char *default_str = "default";
    struct sensorCoreInfo *mCoreInfo;

    int8_t handle = mSensorCoreList[sensType];

    if (!atomicBitsetGetBit(mSensorCoreUsed, handle))
        return -1;
    mCoreInfo = &mInfoCoreList[handle];

    if (!mCoreInfo->getSensorInfo)
        return -1;
    strncpy(data->name, default_str, strlen(default_str) + 1);
    mCoreInfo->getSensorInfo(data);
    if (sensType == SENS_TYPE_MAG &&
            !strncmp(data->name, default_str, strlen(default_str)))
        strncpy(data->name, data->mag_dev_info.libname,
            strlen(data->mag_dev_info.libname) + 1);
    return 0;
}

int sensorCoreRegister(struct sensorCoreInfo *mInfo)
{
    int8_t handle = atomicBitsetFindClearAndSet(mSensorCoreUsed);

    if (handle < 0)
        return -1;
    mSensorCoreList[mInfo->sensType] = handle;
    memcpy(&mInfoCoreList[handle], mInfo, sizeof(struct sensorCoreInfo));
    return 0;
}

void sensorCoreInit(void)
{
    atomicBitsetInit(mSensorCoreUsed, MAX_REGISTERED_SENSORS);
}
