#ifndef _FEATURE_STRUCT_H_
#define _FEATURE_STRUCT_H_

/* header file inclusion */

#include "FreeRTOS.h"
//#ifdef _PC_VERSION_
// #include "simu_arm_math.h"
//#else
// #include "arm_math.h"
//#endif
#include "simu_arm_math.h"
#include <stdint.h>

/*  temporary typedef  */
typedef float float32_t;
typedef double float64_t;
typedef int32_t q31_t;
typedef int64_t q63_t;

#include "feature_setting.h"
#include <stdio.h> /* for debug */
#include <stdlib.h> /* for qsort */
#include <stdint.h> /* for type define*/
#include <math.h> /* for math computation */

//#define ENABLE_LPF_SIGNALING

/*Buffer Size Definition*/
#define BARO_BUFFER_LENGTH   50   /*2Hz*20s*/
#define ACC_BUFFER_LENGTH   144   /*128 + 16 (max window length + delay 320 ms)*/

/*Define Feature set buffer struct*/
typedef struct {
    uint32_t rd_featurebuf_ptr;
    uint32_t wrt_featurebuf_ptr;
    int32_t *features;
} Feature_t;

/* Data Structure for Feature Extraction */
#define XYZ 3
typedef struct {
    uint32_t last_time;
    int32_t wrt_accbuffer_ptr;
    int32_t rd_accbuffer_ptr;
    int32_t acceleration_mean[XYZ];
    int32_t acceleration_hori[ACC_BUFFER_LENGTH];
    int32_t acceleration_verti[ACC_BUFFER_LENGTH];
    int32_t acceleration_norm[ACC_BUFFER_LENGTH];
    int32_t acceleration_x[ACC_BUFFER_LENGTH];
    int32_t acceleration_y[ACC_BUFFER_LENGTH];
    int32_t acceleration_z[ACC_BUFFER_LENGTH];
#ifdef ENABLE_LPF_SIGNALING
    int32_t acceleration_hori_lpf[ACC_BUFFER_LENGTH];
    int32_t acceleration_verti_lpf[ACC_BUFFER_LENGTH];
    int32_t acceleration_norm_lpf[ACC_BUFFER_LENGTH];
    int32_t acceleration_x_lpf[ACC_BUFFER_LENGTH];
    int32_t acceleration_y_lpf[ACC_BUFFER_LENGTH];
    int32_t acceleration_z_lpf[ACC_BUFFER_LENGTH];
    int32_t acceleration_lpf_mean[2 * XYZ]; /* [0]: x, [1]: y, [2]: z, [3]: norm, [4]: verti, [5]: hori */
#endif
} AccSignals_t;

typedef struct {
    uint32_t last_time;
    int32_t wrt_barobuffer_ptr;
    int32_t rd_barobuffer_ptr;
    int32_t baro_buff[BARO_BUFFER_LENGTH];
} BaroSignals_t;

/* define peak feature */
typedef struct {
    uint32_t peak_counts;
    int32_t sum_peak_interval;
    int32_t sum_peak_height;
    int32_t mean_peak_interval;
} Peak_t;

/* define weightless feature */
typedef struct {
    uint32_t weightless_counts;
    uint32_t sum_weightless_length;
    int32_t sum_weightless_interval;
    int32_t mean_weightless_interval;
    int32_t sum_weightless_val; /* new feature */
} Weightless_t;

typedef struct {
    int32_t flag_corrcoef;
    int32_t feature_window_length;
    int32_t max_peak;
    int32_t selected_axis_index;
    int32_t delta_p;
    int32_t rate_p;
    int32_t mean;
    int32_t max;
    int32_t sampling_rate;
    int32_t *signal_input; /* feature_window_length */
    int32_t *signal_input_sec;
    q31_t *frequency_magnitude_signal; /* feature_window_length/2 */
    int32_t *frequency_band; /* frequency bandwidth for computing energy */
    int32_t **axis_address; /* record the measurement axis address: x, y, z, norm, verti, hori */
    Peak_t peak;
    Weightless_t weightless;
} UFeatureSets_t;

/* seperate feature extraction */
typedef struct Extract_Setting {
    int max_setting_num;
    int feature_update_time; /* total update times */
    int extract_index;    /* the index of extract sequence */
    int freq_axis_index;  /* which freq axis */
    void (*extract_feature_acc_function_ptr)(AccSignals_t *, Feature_t *,
            Feature_Setting *);  /* ptr to acc extract function */
    void (*extract_feature_baro_function_ptr)(BaroSignals_t *, Feature_t *,
            Feature_Setting *);  /* ptr to baro extract function */
} Extract_Setting;

#endif
