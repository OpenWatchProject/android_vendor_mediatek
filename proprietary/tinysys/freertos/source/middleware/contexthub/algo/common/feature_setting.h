#ifndef _FEATURE_SETTING_H_
#define _FEATURE_SETTING_H_
#include <stdint.h>
/* _FEATURE_SETTING_H_ */

/*define setting buffer size*/
#define NUM_ACC_WINDOW_LEN 4
#define NUM_BARO_WINDOW_LEN 4
#define NUM_ACC_FREQ_WINDOW_LEN 1
#define NUM_FREQ_BAND 9
#define NUM_ACC_CORRCOEF_WINDOW_LEN 1
#define NUM_ACC_ADV_WINDOW_LEN 1
#ifdef ENABLE_LPF_SIGNALING
#define MAX_ACC_AXIS 12
#else
#define MAX_ACC_AXIS 6
#endif

/*define feature setting structure*/
typedef struct {
    /* number of features for verifying */
    int16_t acc_output_features;
    int16_t baro_output_features;

    /* the chosen measurements from accX, accY, accZ, accNorm, accVer, accHori */
    int32_t ennum_acc_axis_time; /* the maximum number is 12 (raw + its low pass filter) for time domain */
    int32_t acc_axis_time[MAX_ACC_AXIS]; /* accX(0), accY(1), accZ(2), accNorm(3), accVer(4), accHori(5),
                                      * accX_lpf(6), accY_lpf(7), accZ_lpf(8), accNorm_lpf(9), accVer_lpf(10),
                                      * accHori_lpf(11)
                                      * */
    int32_t ennum_acc_axis_freq; /* the maximum number is 12 (raw + its low pass filter) for frequency domain */
    int32_t acc_axis_freq_current_index; /* the current index of frequency axis */
    int32_t acc_axis_freq[MAX_ACC_AXIS]; /* accX(0), accY(1), accZ(2), accNorm(3), accVer(4), accHori(5),
                                      * accX_lpf(6), accY_lpf(7), accZ_lpf(8), accNorm_lpf(9), accVer_lpf(10),
                                      * accHori_lpf(11)
                                      * */

    int32_t ennum_acc_window_t; /* the number of acc time-feature window */
    int32_t acc_window_t[NUM_ACC_WINDOW_LEN]; /* acc feature window: 32, 64, 128, etc */

    int32_t ennum_acc_window_f; /* the number of acc freq-feature window */
    int32_t acc_window_f[NUM_ACC_FREQ_WINDOW_LEN]; /* acc freq-feature window 32, 64, 128,...*/
    int32_t ennum_acc_freq_band; /* the number of acc freq-band */
    int32_t acc_freq_band[NUM_FREQ_BAND][2]; /* acc freq-band start and end. (transform to fixed point)
                                          * e.g. {{0,10}, {5,20}}
                                          * index = (frequency_band * feature_window_length) / sampling_rate
                                          * */

    int32_t ennum_acc_window_corr; /* the number of acc correlation coefficient feature window */
    int32_t ennum_acc_axis_corrcoef; /* the number of acc corrcoef pairs */
    int32_t acc_window_corrcoef[NUM_ACC_CORRCOEF_WINDOW_LEN]; /* acc corr-coef window 32, 64, 128,...*/
    int32_t acc_axis_corrcoef[MAX_ACC_AXIS][2]; /* accelerations for corr-coef, {{0, 1}, {3, 4}, {9, 10}}. max pair number is 12 */

    int32_t ennum_acc_window_adv; /* the number of acc adv feature window */
    int32_t ennum_acc_axis_adv; /* the number of acc axis for computing adv features */
    int32_t acc_window_adv[NUM_ACC_ADV_WINDOW_LEN]; /* acc peak adv window. e.g. 64, 128, etc. */
    int32_t acc_axis_peak[MAX_ACC_AXIS]; /* the acc axis for computing peak. e.g. 0 - 11 */
    int32_t acc_axis_weightless[MAX_ACC_AXIS]; /* the acc axis for computing weightlessness. e.g. 0 - 11 */

    int32_t ennum_baro_window_t; /* the number of barometric feature window */
    int32_t baro_window_t[NUM_BARO_WINDOW_LEN]; /* barometer feature window. e.g. 150, 200 */
} Feature_Setting;

Feature_Setting *get_context_feature_setting_ptr();
Feature_Setting *get_fall_feature_setting_ptr();
Feature_Setting *get_gesture_feature_setting_ptr();

#endif
