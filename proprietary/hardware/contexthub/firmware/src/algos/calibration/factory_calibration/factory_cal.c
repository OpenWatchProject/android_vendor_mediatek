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

#include "calibration/factory_calibration/factory_cal.h"
#include "common/math/macros.h"
#include "calibration/util/cal_log.h"

#define ACC_VAR_THRESHOLD_DEFAULT 8e-3f
#define ACC_BIAS_MEAN_LIMIT_DEFAULT 0.5f
#define ACC_DEBOUNCE_TIME_DEFAULT SEC_TO_NANOS(0.5f)
#define ACC_WIN_TIME_DEFAULT SEC_TO_NANOS(2)
#define GYRO_VAR_THRESHOLD_DEFAULT 5e-5f
#define GYRO_BIAS_MEAN_LIMIT_DEFAULT 0.1f
#define GYRO_DEBOUNCE_TIME_DEFAULT SEC_TO_NANOS(0.5f)
#define GYRO_WIN_TIME_DEFAULT SEC_TO_NANOS(2)

static void factoryCalibrateParam(struct FactoryCalibrate *detect,
        const struct FactoryCalibrateParam *parameter)
{
    memset(detect, 0, sizeof(*detect));

    detect->factory_calibration_enable =
        parameter->factory_calibration_enable;
    detect->var_threshold = parameter->var_threshold;
    detect->bias_mean_limit =
        parameter->bias_mean_limit;
    detect->debounce_time_duration_nanos =
        parameter->debounce_time_duration_nanos;
    detect->window_time_duration_nanos =
        parameter->window_time_duration_nanos;
    detect->standard_x =
        parameter->standard_x;
    detect->standard_y =
        parameter->standard_y;
    detect->standard_z =
        parameter->standard_z;
}

static bool factoryCalibrateDebounce(struct FactoryCalibrate *detect,
        uint64_t sample_time)
{
    if (detect->start_debounce) {
        detect->start_debounce = false;
        detect->debounce_end_time_nanos =
            sample_time + detect->debounce_time_duration_nanos;
    } else {
        detect->stillness_debounce_ready =
            (sample_time >= detect->debounce_end_time_nanos);
    }
    return detect->stillness_debounce_ready;
}

static bool factoryCalibrateUpdate(struct FactoryCalibrate *detect,
    float x, float y, float z, uint64_t sample_time)
{
    float delta = 0;

    detect->num_samples++;

    detect->mean_x += x;
    detect->mean_y += y;
    detect->mean_z += z;

    if (detect->start_new_window) {
        detect->start_new_window = false;

        detect->assumed_mean_x = x;
        detect->assumed_mean_y = y;
        detect->assumed_mean_z = z;

        detect->num_win_samples = 0;
        detect->win_mean_x = 0;
        detect->win_mean_y = 0;
        detect->win_mean_z = 0;
        detect->var_x = 0;
        detect->var_y = 0;
        detect->var_z = 0;

        detect->window_end_time_nanos =
            sample_time + detect->window_time_duration_nanos;
    } else {
        detect->stillness_window_ready =
            (sample_time >= detect->window_end_time_nanos) &&
            (detect->num_samples > 1);
    }

    detect->num_win_samples++;

    delta = (x - detect->assumed_mean_x);
    detect->win_mean_x += delta;
    detect->var_x += delta * delta;

    delta = (y - detect->assumed_mean_y);
    detect->win_mean_y += delta;
    detect->var_y += delta * delta;

    delta = (z - detect->assumed_mean_z);
    detect->win_mean_z += delta;
    detect->var_z += delta * delta;

    return detect->stillness_window_ready;
}

static bool factoryCalibrateCompute(struct FactoryCalibrate *detect)
{
    float tmp_denom = 1.f;
    float tmp_denom_mean = 1.f;

    if (detect->num_win_samples > 1) {
        tmp_denom = 1.f / (detect->num_win_samples - 1);
        tmp_denom_mean = 1.f / detect->num_win_samples;
    } else {
        detect->stillness_confidence = false;
        return detect->stillness_confidence;
    }

    float tmp = detect->win_mean_x;
    detect->win_mean_x *= tmp_denom_mean;
    detect->win_var_x =
        (detect->var_x - detect->win_mean_x * tmp) *
        tmp_denom;

    tmp = detect->win_mean_y;
    detect->win_mean_y *= tmp_denom_mean;
    detect->win_var_y =
        (detect->var_y - detect->win_mean_y * tmp) *
        tmp_denom;

    tmp = detect->win_mean_z;
    detect->win_mean_z *= tmp_denom_mean;
    detect->win_var_z =
        (detect->var_z - detect->win_mean_z * tmp) *
        tmp_denom;

    detect->win_mean_x += detect->assumed_mean_x;
    detect->win_mean_y += detect->assumed_mean_y;
    detect->win_mean_z += detect->assumed_mean_z;

    if ((detect->win_var_x >= detect->var_threshold) ||
        (detect->win_var_y >= detect->var_threshold) ||
        (detect->win_var_z >= detect->var_threshold)) {
        detect->stillness_confidence = false;
    } else {
        detect->stillness_confidence = true;
    }
    if (detect->stillness_confidence) {
        detect->bias_x = detect->win_mean_x - detect->standard_x;
        detect->bias_y = detect->win_mean_y - detect->standard_y;
        detect->bias_z = detect->win_mean_z - detect->standard_z;
        if (fabsf(detect->bias_x) >= detect->bias_mean_limit ||
            fabsf(detect->bias_y) >= detect->bias_mean_limit ||
            fabsf(detect->bias_z) >= detect->bias_mean_limit) {
                detect->stillness_confidence = false;
                CAL_DEBUG_LOG("[FACTORY_CAL]", "device tilt, please place horizon.");
                CAL_DEBUG_LOG("[FACTORY_CAL]", "Mean|Limit: "
                    CAL_FORMAT_6DIGITS_TRIPLET " | " "%.6f",
                    CAL_ENCODE_FLOAT(detect->bias_x, 6),
                    CAL_ENCODE_FLOAT(detect->bias_y, 6),
                    CAL_ENCODE_FLOAT(detect->bias_z, 6),
                    CAL_ENCODE_FLOAT(detect->bias_mean_limit, 6));
        } else {
            CAL_DEBUG_LOG("[FACTORY_CAL]", "device still, computing bias now.");
        }
    } else {
        CAL_DEBUG_LOG("[FACTORY_CAL]", "device shake, please place stable.");
        CAL_DEBUG_LOG("[FACTORY_CAL]", "Var|Threshold: "
            CAL_FORMAT_6DIGITS_TRIPLET " | " "%.6f",
            CAL_ENCODE_FLOAT(detect->win_var_x, 6),
            CAL_ENCODE_FLOAT(detect->win_var_y, 6),
            CAL_ENCODE_FLOAT(detect->win_var_z, 6),
            CAL_ENCODE_FLOAT(detect->var_threshold, 6));
    }
    return detect->stillness_confidence;
}

static int factoryCalibrateRun(struct FactoryCalibrate *detect,
    float x, float y, float z, uint64_t sample_time)
{
    int status = 0;

    if (!detect->factory_calibration_enable)
        return UNSTABLE_DETECT;
    if (!factoryCalibrateDebounce(detect, sample_time))
        return WAITING_DEBOUNCE;
    if (!factoryCalibrateUpdate(detect, x, y, z, sample_time))
        return WAITING_DETECT;
    status = factoryCalibrateCompute(detect);
    return status ? STILL_DETECT : UNSTABLE_DETECT;
}

static void factoryCalibrateGetBias(struct FactoryCalibrate *detect,
    float *x_bias, float *y_bias, float *z_bias, int *accurancy)
{
    *x_bias = detect->bias_x;
    *y_bias = detect->bias_y;
    *z_bias = detect->bias_z;
    *accurancy = 3;
}

static void factoryCalibrateRemoveBias(struct FactoryCalibrate *detect,
    float xi, float yi, float zi, float *xo, float *yo, float *zo)
{
    if (detect->factory_calibration_enable) {
        *xo = xi - detect->bias_x;
        *yo = yi - detect->bias_y;
        *zo = zi - detect->bias_z;
    } else {
        *xo = xi;
        *yo = yi;
        *zo = zi;
    }
}

static void factoryCalibrateInit(struct FactoryCalibrate *detect)
{
    detect->stillness_debounce_ready = false;
    detect->start_debounce = true;
    detect->stillness_window_ready = false;
    detect->start_new_window = true;
    detect->num_samples = 0;
    detect->mean_x = 0;
    detect->mean_y = 0;
    detect->mean_z = 0;
    detect->var_x = 0;
    detect->var_y = 0;
    detect->var_z = 0;
    detect->win_var_x = 0;
    detect->win_var_y = 0;
    detect->win_var_z = 0;
    detect->bias_x = 0;
    detect->bias_y = 0;
    detect->bias_z = 0;
}

void gyroFactoryCalibrateParam(struct GyroFactoryCal *factory_cal,
        const struct GyroFactoryCalParameters *parameter)
{
    factoryCalibrateParam(&factory_cal->detect, &parameter->param);
}

void gyroFactoryCalibrateInit(struct GyroFactoryCal *factory_cal)
{
    if (!factory_cal->detect.var_threshold)
        factory_cal->detect.var_threshold =
            GYRO_VAR_THRESHOLD_DEFAULT;
    if (!factory_cal->detect.bias_mean_limit)
        factory_cal->detect.bias_mean_limit =
            GYRO_BIAS_MEAN_LIMIT_DEFAULT;
    if (!factory_cal->detect.debounce_time_duration_nanos)
        factory_cal->detect.debounce_time_duration_nanos =
            GYRO_DEBOUNCE_TIME_DEFAULT;
    if (!factory_cal->detect.window_time_duration_nanos)
        factory_cal->detect.window_time_duration_nanos =
            GYRO_WIN_TIME_DEFAULT;
    factoryCalibrateInit(&factory_cal->detect);
}

int gyroFactoryCalibrateRun(struct GyroFactoryCal *factory_cal,
    float x, float y, float z, uint64_t sample_time)
{
    return factoryCalibrateRun(&factory_cal->detect, x, y, z, sample_time);
}

void gyroFactoryCalibrateGetBias(struct GyroFactoryCal *factory_cal,
    float *x_bias, float *y_bias, float *z_bias, int *accurancy)
{
    factoryCalibrateGetBias(&factory_cal->detect,
        x_bias, y_bias, z_bias, accurancy);
}

void gyroFactoryCalibrateRemoveBias(struct GyroFactoryCal *factory_cal,
    float xi, float yi, float zi, float *xo, float *yo, float *zo)
{
    factoryCalibrateRemoveBias(&factory_cal->detect, xi, yi, zi, xo, yo, zo);
}

void accFactoryCalibrateParam(struct AccFactoryCal *factory_cal,
        const struct AccFactoryCalParameters *parameter)
{
    factoryCalibrateParam(&factory_cal->detect, &parameter->param);
}

void accFactoryCalibrateInit(struct AccFactoryCal *factory_cal)
{
    if (!factory_cal->detect.var_threshold)
        factory_cal->detect.var_threshold =
            ACC_VAR_THRESHOLD_DEFAULT;
    if (!factory_cal->detect.bias_mean_limit)
        factory_cal->detect.bias_mean_limit =
            ACC_BIAS_MEAN_LIMIT_DEFAULT;
    if (!factory_cal->detect.debounce_time_duration_nanos)
        factory_cal->detect.debounce_time_duration_nanos =
            ACC_DEBOUNCE_TIME_DEFAULT;
    if (!factory_cal->detect.window_time_duration_nanos)
        factory_cal->detect.window_time_duration_nanos =
            ACC_WIN_TIME_DEFAULT;
    factoryCalibrateInit(&factory_cal->detect);
}

int accFactoryCalibrateRun(struct AccFactoryCal *factory_cal,
    float x, float y, float z, uint64_t sample_time)
{
    return factoryCalibrateRun(&factory_cal->detect, x, y, z, sample_time);
}

void accFactoryCalibrateGetBias(struct AccFactoryCal *factory_cal,
    float *x_bias, float *y_bias, float *z_bias, int *accurancy)
{
    factoryCalibrateGetBias(&factory_cal->detect,
        x_bias, y_bias, z_bias, accurancy);
}

void accFactoryCalibrateRemoveBias(struct AccFactoryCal *factory_cal,
    float xi, float yi, float zi, float *xo, float *yo, float *zo)
{
    factoryCalibrateRemoveBias(&factory_cal->detect, xi, yi, zi, xo, yo, zo);
}

