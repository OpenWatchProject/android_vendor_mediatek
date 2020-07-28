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

#define MTK_LOG_ENABLE 1
#include "jni.h"
#include <log/log.h>

#undef LOG_NDEBUG
#undef NDEBUG

#ifdef LOG_TAG
#undef LOG_TAG
#define LOG_TAG "SENSOR-JNI"
#endif

extern "C" {
#include "libhwm.h"


JNIEXPORT jint JNICALL Java_com_mediatek_sensorhub_sensor_EmSensor_getPsensorData(JNIEnv *, jobject)
{
    ALOGD("Enter getPsensorData()\n");
    int ret = get_psensor_data();
    ALOGD("getPsensorData() returned %d\n", ret);

    return ret;
}

JNIEXPORT jint JNICALL Java_com_mediatek_sensorhub_sensor_EmSensor_getPsensorThreshold(JNIEnv *env, jclass, jintArray result) {
    int values[2] = { 0 };
    ALOGD("Enter getPsensorThreshold()\n");
    values[0] = get_psensor_threshold(0);
    values[1] = get_psensor_threshold(1);
    ALOGD("getPsensorThreshold() returned %d, %d\n", values[0], values[1]);

    env->SetIntArrayRegion(result, 0, 2, (jint*) values);
    return 1;
}

JNIEXPORT jint JNICALL Java_com_mediatek_sensorhub_sensor_EmSensor_setPsensorThreshold(JNIEnv *, jclass, jint high, jint low) {
    ALOGD("Enter getPsensorThreshold()\n");
    int ret = set_psensor_threshold(high, low);
    ALOGD("getPsensorThreshold() returned %d\n", ret);

    return ret;
}

JNIEXPORT jint JNICALL Java_com_mediatek_sensorhub_sensor_EmSensor_calculatePsensorMinValue(JNIEnv *, jclass) {
    ALOGD("Enter calculatePsensorMinValue()\n");
    int ret = calculate_psensor_min_value();
    ALOGD("calculatePsensorMinValue() returned %d\n", ret);

    return ret;
}

JNIEXPORT jint JNICALL Java_com_mediatek_sensorhub_sensor_EmSensor_getPsensorMinValue(JNIEnv *, jclass) {
    ALOGD("Enter getPsensorMinValue()\n");
    int ret = get_psensor_min_value();
    ALOGD("getPsensorMinValue() returned %d\n", ret);

    return ret;
}

JNIEXPORT jint JNICALL Java_com_mediatek_sensorhub_sensor_EmSensor_calculatePsensorMaxValue(JNIEnv *, jclass) {
    ALOGD("Enter calculatePsensorMaxValue()\n");
    int ret = calculate_psensor_max_value();
    ALOGD("calculatePsensorMaxValue() returned %d\n", ret);

    return ret;
}

JNIEXPORT jint JNICALL Java_com_mediatek_sensorhub_sensor_EmSensor_getPsensorMaxValue(JNIEnv *, jclass) {
    ALOGD("Enter getPsensorMaxValue()\n");
    int ret = get_psensor_max_value();
    ALOGD("getPsensorMaxValue() returned %d\n", ret);

    return ret;
}

JNIEXPORT jint JNICALL Java_com_mediatek_sensorhub_sensor_EmSensor_doPsensorCalibration(JNIEnv *, jclass, jint min, jint max) {
    ALOGD("Enter doPsensorCalibration()\n");
    int ret = do_calibration(min, max);
    ALOGD("doPsensorCalibration() returned %d\n", ret);

    return ret;
}

JNIEXPORT jint JNICALL Java_com_mediatek_sensorhub_sensor_EmSensor_clearPsensorCalibration(JNIEnv *, jclass) {
    ALOGD("Enter clear_psensor_calibration()\n");
    int ret = clear_psensor_calibration();
    ALOGD("clear_psensor_calibration() returned %d\n", ret);
    return ret;
}

JNIEXPORT jint JNICALL Java_com_mediatek_sensorhub_sensor_EmSensor_startGsensorCalibration(JNIEnv *, jclass) {
    ALOGD("Enter gsensor_start_static_calibration()\n");
    int ret = gsensor_start_static_calibration();
    ALOGD("gsensor_start_static_calibration() returned %d\n", ret);

    return ret;
}

JNIEXPORT jint JNICALL Java_com_mediatek_sensorhub_sensor_EmSensor_getGsensorStaticCalibration(JNIEnv *env, jclass, jfloatArray result) {
    ALOGD("Enter gsensor_get_static_calibration()\n");
    struct caliData caliData;
    int ret = gsensor_get_static_calibration(&caliData);
    ALOGD("get_gyroscope_calibration() returned %d, %f, %f, %f\n", ret,
        caliData.data[0], caliData.data[1] ,caliData.data[2]);

    env->SetFloatArrayRegion(result, 0, 3, (jfloat*) caliData.data);
    return ret;
}

JNIEXPORT jint JNICALL Java_com_mediatek_sensorhub_sensor_EmSensor_startGyroscopeCalibration(JNIEnv *, jclass) {
    ALOGD("Enter gyroscope_start_static_calibration()\n");
    int ret = gyroscope_start_static_calibration();
    ALOGD("gyroscope_start_static_calibration() returned %d\n", ret);
    return ret;
}

JNIEXPORT jint JNICALL Java_com_mediatek_sensorhub_sensor_EmSensor_getGyroscopeStaticCalibration(JNIEnv *env, jclass, jfloatArray result) {
    ALOGD("Enter gyroscope_get_static_calibration()\n");
    struct caliData caliData;
    int ret = gyroscope_get_static_calibration(&caliData);
    ALOGD("gyroscope_get_static_calibration() returned %d, %f, %f, %f\n", ret,
        caliData.data[0], caliData.data[1] ,caliData.data[2]);

    env->SetFloatArrayRegion(result, 0, 3, (jfloat*) caliData.data);
    return ret;
}

JNIEXPORT jint JNICALL Java_com_mediatek_sensorhub_sensor_EmSensor_startLightCalibration(JNIEnv *, jclass) {
    ALOGD("Enter als_start_static_calibration()\n");
    int ret = als_start_static_calibration();
    ALOGD("als_start_static_calibration() returned %d\n", ret);

    return ret;
}

JNIEXPORT jint JNICALL Java_com_mediatek_sensorhub_sensor_EmSensor_getLightStaticCalibration(JNIEnv *env, jclass, jfloatArray result) {
    ALOGD("Enter als_get_static_calibration()\n");
    struct caliData caliData;
    int ret = als_get_static_calibration(&caliData);
    ALOGD("als_get_static_calibration() returned %d, %f\n", ret, caliData.data[0]);

    env->SetFloatArrayRegion(result, 0, 3, (jfloat*) caliData.data);
    return ret;
}
}
