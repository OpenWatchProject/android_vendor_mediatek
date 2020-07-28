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

#ifndef _ALGOS_CALIBRATION_FACTORY_FACTORY_CALIBRATION_
#define _ALGOS_CALIBRATION_FACTORY_FACTORY_CALIBRATION_

#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>

enum calResult {
    STILL_DETECT = 0,
    UNSTABLE_DETECT,
    WAITING_DEBOUNCE,
    WAITING_DETECT,
};

struct FactoryCalibrateParam {
    bool factory_calibration_enable;
    float var_threshold;
    float bias_mean_limit;
    float standard_x, standard_y, standard_z;
    uint64_t debounce_time_duration_nanos;
    uint64_t window_time_duration_nanos;
};

struct FactoryCalibrate {
    float var_threshold;
    float bias_mean_limit;
    bool factory_calibration_enable;
    bool stillness_debounce_ready;
    bool start_debounce;
    bool stillness_window_ready;
    bool start_new_window;
    bool stillness_confidence;
    uint64_t debounce_time_duration_nanos;
    uint64_t debounce_end_time_nanos;
    uint64_t window_time_duration_nanos;
    uint64_t window_end_time_nanos;
    uint32_t num_samples;
    uint32_t num_win_samples;
    float mean_x, mean_y, mean_z;
    float win_mean_x, win_mean_y, win_mean_z;
    float assumed_mean_x, assumed_mean_y, assumed_mean_z;
    float var_x, var_y, var_z;
    float win_var_x, win_var_y, win_var_z;
    float bias_x, bias_y, bias_z;
    float standard_x, standard_y, standard_z;
};

struct AccFactoryCalParameters {
    struct FactoryCalibrateParam param;
    // private filed please put here
};

struct AccFactoryCal {
    struct FactoryCalibrate detect;
    // private filed please put here
};

struct GyroFactoryCalParameters {
    struct FactoryCalibrateParam param;
    // private filed please put here
};

struct GyroFactoryCal {
    struct FactoryCalibrate detect;
    // private filed please put here
};

void gyroFactoryCalibrateParam(struct GyroFactoryCal *factory_cal,
        const struct GyroFactoryCalParameters *parameter);
void gyroFactoryCalibrateInit(struct GyroFactoryCal *factory_cal);
int gyroFactoryCalibrateRun(struct GyroFactoryCal *factory_cal,
    float x, float y, float z, uint64_t sample_time);
void gyroFactoryCalibrateGetBias(struct GyroFactoryCal *factory_cal,
    float *x_bias, float *y_bias, float *z_bias, int *accurancy);
void gyroFactoryCalibrateRemoveBias(struct GyroFactoryCal *factory_cal,
    float xi, float yi, float zi, float *xo, float *yo, float *zo);
void accFactoryCalibrateParam(struct AccFactoryCal *factory_cal,
        const struct AccFactoryCalParameters *parameter);
void accFactoryCalibrateInit(struct AccFactoryCal *factory_cal);
int accFactoryCalibrateRun(struct AccFactoryCal *factory_cal,
    float x, float y, float z, uint64_t sample_time);
void accFactoryCalibrateGetBias(struct AccFactoryCal *factory_cal,
    float *x_bias, float *y_bias, float *z_bias, int *accurancy);
void accFactoryCalibrateRemoveBias(struct AccFactoryCal *factory_cal,
    float xi, float yi, float zi, float *xo, float *yo, float *zo);
#endif
