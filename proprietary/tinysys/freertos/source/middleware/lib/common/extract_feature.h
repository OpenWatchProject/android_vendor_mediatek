#ifndef _EXTRACT_FEATURE_H_
#define _EXTRACT_FEATURE_H_

/* header file inclusion */
#include <stdio.h> /* for debug */
#include <stdlib.h> /* for qsort, malloc */
#include <stdint.h> /* for type define*/
#include <math.h> /* for math computation */

#include "FreeRTOS.h"
#include "feature_struct.h"
#include "feature_setting.h" /* compute features based on feature setting */
#include "kiss_fftr.h" /* for real-fft */
#include "kiss_fft_cust.h" /* for customerized fft setting */
#include "simu_arm_math.h"

/* interface connecting the sensor buffer and its feature sequence */
Extract_Setting *get_context_feature_extract_setting_ptr();
Extract_Setting *get_gesture_feature_extract_setting_ptr();
void extract_feature_acc_raw(AccSignals_t *accelerations_ptr, Feature_t *features, Feature_Setting *setting);
void extract_feature_acc_time(AccSignals_t *accelerations_ptr, Feature_t *features, Feature_Setting *setting);
void extract_feature_acc_freq(AccSignals_t *accelerations_ptr, Feature_t *features, Feature_Setting *setting);
void extract_feature_acc_corrcoefadv(AccSignals_t *accelerations_ptr, Feature_t *features, Feature_Setting *setting);
void extract_feature_baro(BaroSignals_t *baro, Feature_t *features, Feature_Setting *setting);

/* Signal functions */
void fixed_norm_acc(int64_t *signal_xyz , int32_t *signal_norm);
void fixed_exp_filt(int32_t *update_value, int32_t previous_value, uint32_t alpha);

void extract_acc_spatial_info(AccSignals_t *acc, UFeatureSets_t *extract_acc);
void extract_baro_spatial_info(BaroSignals_t *baro, UFeatureSets_t *extract_baro);
void get_acc_freq_info(UFeatureSets_t *extract_acc);

/* Basic Spatial Statistical Functions (acc) */
void com_timefeature(UFeatureSets_t *u_feature_sets, Feature_t *features);
void com_max(UFeatureSets_t *u_feature_sets, Feature_t *features);
void com_mean(UFeatureSets_t *u_feature_sets, Feature_t *features);
void com_std(UFeatureSets_t *u_feature_sets, Feature_t *features);
void com_rms(UFeatureSets_t *u_feature_sets, Feature_t *features);
void com_crossrate(UFeatureSets_t *u_feature_sets, Feature_t *features);
void com_iqr(UFeatureSets_t *u_feature_sets, Feature_t *features);
void com_corrcoef(UFeatureSets_t *u_feature_sets, Feature_t *features);
int compare(const void *elem1, const void *elem2);

/* Basic Spatial Statistical Functions (baro) */
void get_dp(UFeatureSets_t *extract_baro, Feature_t *features);
void get_dp_dt(UFeatureSets_t *extract_baro, Feature_t *features);

/* Basic Frequency Functions */
void com_ac_spectrum(UFeatureSets_t *u_feature_sets);
void com_mag_spec(kiss_fft_cpx *, q31_t *, uint32_t, uint32_t);
// void com_maxmag_freq(UFeatureSets_t *u_feature_sets, Feature_t *features); /* backup */
// void com_freq_entro(UFeatureSets_t *u_feature_sets, Feature_t *features);  /* backup */
void get_firstsummit(UFeatureSets_t *u_feature_sets, Feature_t *features);
void com_esd(UFeatureSets_t *u_feature_sets, Feature_t *features);

/* Advanced Spatial Functions */
void com_peak_info(UFeatureSets_t *u_feature_sets, Feature_t *features);
void com_weightless_info(UFeatureSets_t *u_feature_sets);
void get_peak_int(UFeatureSets_t *u_feature_sets, Feature_t *features);
void get_peak_hei(UFeatureSets_t *u_feature_sets, Feature_t *features);
void get_wei_len(UFeatureSets_t *u_feature_sets, Feature_t *features);
void get_wei_int(UFeatureSets_t *u_feature_sets, Feature_t *features);
void get_weilen_feawin(UFeatureSets_t *u_feature_sets, Feature_t *features);
// void get_wei_avgval(UFeatureSets_t *u_feature_sets, Feature_t *features);  /* new feature(backup) */
/* peak v.s. weightlessness*/
void get_peakcnt_weicnt(UFeatureSets_t *u_feature_sets, Feature_t *features);
void get_weiint_peakint(UFeatureSets_t *u_feature_sets, Feature_t *features);

#endif
