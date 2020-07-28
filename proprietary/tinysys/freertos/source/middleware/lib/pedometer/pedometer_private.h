/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
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
#ifndef __PEDOMETER_PRIVATE_H__
#define __PEDOMETER_PRIVATE_H__
#include <stdio.h>
#include <stdint.h>
#include "math_method.h"
#include "pedometer.h"
#include "extract_feature.h"
// #include "activity_algorithm.h"
// #include "context.h"
#define ANTI_NOISE_ENABLE                       // Enable noise detection mechanism
#define STATEMACHINE_BUFFER_SIZE             11 // state machine total vote, must be an odd number, This is used only in main()
#define SENSOR_DOWN_SAMPLING_RATIO           1  // 1: no down sampling n: SENSOR_FREQUENCY/n//////

///////////////////////////////////
//    Special control
///////////////////////////////////
// #define DEVICE_IS_WATCH                         // If this is commented, it's Phone
// #define _STABILITY_TEST_                        // This is used to test stability with large iterations of logs
#define _STEP_DETECTOR_PARA_                    // This is used to set paramters for step_detector case
// #define _SWIP_ENABLE_                           // This is for SW IP protection
#define _NOISE_COMPENSATION_                    // This is used to compensate steps blocked by noise detection
// #define _CONTEXT_RESULT_ENABLE_                 // To access the result from context detection for SMD
// #define PC_DEV_PLOT                             // for Matlab plot
// #define PC_DEV_LOG                              // sync the log in PC and DEV
#define _EVEREST_LOG_
#define _ANTI_NOISE_TIME_CONSTAINT_
//#define PC_UTILITY                              // parameters for PC use only
///////////////////////////////////

#ifdef _SWIP_ENABLE_
#define PEDOMETER_SWIP_PERIOD           800
#endif

#ifdef _NOISE_COMPENSATION_
#define COMPENSATED_STEP                  3
#endif

#ifdef PC_UTILITY
// define DSP math
#define SHIFT_BIT                        10
// #define SHIFT_VALUE                      (1<<SHIFT_BIT)
// simulation parameters
#define IMU_BUFFER_SIZE_PEDOMETER                  100000//3850000//100000
#define NUM_IMU_VAR_PEDOMETER                      11
#define IDX_ACC_X_PEDOMETER              4
#define IDX_ACC_Y_PEDOMETER              5
#define IDX_ACC_Z_PEDOMETER              6
float imu_pedometer[IMU_BUFFER_SIZE_PEDOMETER][NUM_IMU_VAR_PEDOMETER];
int32_t imu_dsp[IMU_BUFFER_SIZE_PEDOMETER][NUM_IMU_VAR_PEDOMETER];
#endif


#define LPF_F2F(a) ((int32_t)((a)*(float)((1<<24)-1)))     //LPF: Float to Fixed Point
#define LPF_mul(a,b) ((int32_t)((long long)((long long)(a) * (b)) >> 24)) //LPF: Float to Fixed point multiply

#ifdef DEVICE_IS_WATCH
#define SAMPLE_PERIOD                16  // ms, 62.5Hz for watch
#else
#define SAMPLE_PERIOD                20  // ms, 50Hz for SP
#endif

// Personal Dead-reckoning(pdr) parameters
#define GLOBAL_TIME_BOUND                4294967295//100//2147483640
#define P2P_TIME_BOUND                   80//2147483550
#define MAX_PDR_FREQ                     4000   // unit: 0.001Hz
#define MAX_PDR_FREQ_RUN                 5000   // unit: 0.001Hz
#define DEFAULT_STEP_FREQ                1842   // unit: 0.001Hz
#define EXCEPTION_TIME                   2000   // ms
#define GRAVITY                          9810   // unit: 0.001m/s^2
#define STEP_DEFAULT_THRESHOLD           10000//9000   // unit: 0.001 m/s^2, default pedometer threshold
#define STEP_SENSITIVITY                 300
#define EXTREME_DIFF_THRESHOLD           2000   // unit: 0.001 m/s^2, minimum diff over pedometer threshold to ensure a step
#define LOCAL_MAX_DIFF                   1000   // unit: 0.001 m/s^2, lower bound for diff between local max accnorm and pedometer threshold
#define ACC_NORM_RUN_DIFF_THRESHOLD      10000
#define THRESHOLD_WINDOW                 125
#define THRESHOLD_NOISE_COUNTER          32//128    //16*N, N=1,2,3..., the sample count for noise state
#define ANTI_SHAKE_PERIOD                1200   // ms, 45 samples in 62.5HZ sample frequency
#define THRESHOLD_DECISION_TIME          1500   // ms, change threshold ervery 1500 ms
#define CROSS_PERIOD                     6*SAMPLE_PERIOD  // ms, should be aligned with acc_norm_extreme_diff
#define CROSS_PERIOD_RUN                 6*SAMPLE_PERIOD  // ms
#define PRE_CROSS_PERIOD                 240//320    // unit: ms
#define DOUBLE_CROSS_PERIOD              700    // unit: ms
#define DOUBLE_CROSS_UP_BOUND            1600   // unit: ms, equal to 2*ANTI_SHAKE_PERIOD
#define DOUBLE_CROSS_LOW_BOUND           650    // unit: ms
#define DOUBLE_CROSS_RUN_BOUND           460    // unit: ms
#define RUN_FREQ_THRESHOLD               2400   // unit: 0.001Hz
#define P2P_TIME_RUN_THRESHOLD           700    // unit: ms
#define P2P_TIME_THRESHOLD               770//850    // unit: ms
#define P2P_TIME_TRIPLE                  1200   // unit: ms
#define P2P_TIME_EXCLUDE                 4000   // unit: ms
#define NOISE_SENSITIVITY                  1      // could be 2(default), 3, 4, 5. The detection will prefer noise outcomes if the value
// becomes larger
#define ANTI_NOISE_TIME_BOUND            18000 // unit: ms

// array size
#define ACCNORM_BUFFER_SIZE              125
#define ACC_CALIBRATION_WINDOW           50
#define FREQ_BUFFER_SIZE                 20
#define FILTER_ORDER                     8

// window size
//#define MEAN_WINDOW_SIZE               25
#define MOTION_WINDOW_SIZE               40

// adaptive
#define RUN_DOWN_SAMPLE                  1
#define WALK_DOWN_SAMPLE                 2
#define STILL_DOWN_SAMPLE                8
#define UNKNOWN_DOWN_SAMPLE              0
#define STILL_THRESHOLD                  360000 // unit (0.001m/s^2)^2, threshold for still detection
#define PDR_WALK_STD_TH                  10000000
#define NORM_DIFF_THRESHOLD              2200
#define STILL_NUM_THRESHOLD              99
#define SMD_STEP_TH                      8

// ==================================================
// structure define for predict noise functions
// ==================================================

/*Buffer Size Definition*/
#define ACC_BUFFER_LENGTH_PEDOMETER   32  /*50Hz*2s*/
#define NUM_ACC_AXIS 4

/* user definition (temporary)*/
#define NUM_FEATURES_PDR 24

/* que data for first time predict */
#define MIN_SAMPLES 64

/*Define Feature set buffer struct*/
typedef struct {
    uint16_t rd_featurebuf_ptr;
    uint16_t wrt_featurebuf_ptr;
    int32_t features[NUM_FEATURES_PDR];
} Feature_t_pedometer;

/* Data Structure for Feature Extraction */
#define XYZ 3
typedef struct {
    uint32_t last_time;
    uint16_t wrt_accbuffer_ptr;
    uint16_t rd_accbuffer_ptr;
    int32_t acceleration[NUM_ACC_AXIS][ACC_BUFFER_LENGTH_PEDOMETER];
} AccSignals_t_pedometer;

typedef struct {
    uint16_t current_index;
    uint16_t divide_shift;
    int32_t mean;
    int32_t *signal_input; /* feature_window_length */
} UFeatureSets_t_pedometer;

typedef struct {
    uint8_t *leaf_node_class_distribution;
    uint16_t *decision_node_feature;
    int32_t *decision_node_theshold;
} Tree_pedometer;

typedef struct {
    int32_t accNorm[ACCNORM_BUFFER_SIZE];
    int32_t sensorFreqArr[FREQ_BUFFER_SIZE];
    int32_t acc_norm_reg[FILTER_ORDER + 1];
    int8_t step_detection_queue[STATEMACHINE_BUFFER_SIZE];
    uint8_t circularArrayIdx;
    uint8_t circularFreqArrayIdx;
    uint8_t pdrDetect;
    uint8_t cpsFlag;
    uint8_t sensorDownSamplingRatio;    // sensor down sampling ratio
    uint8_t preSensorDownSamplingRatio;
    uint8_t stillDetect;
    uint8_t significantMotion;
    uint8_t smdStep;
    uint8_t doubleStepFlag;
    uint8_t tripleStepFlag;
    uint8_t antiShake;
    uint8_t pre_anti_shake;
    int32_t p2p_time;
    uint32_t global_time_stamp;
    uint32_t max_peak_time_stamp;
    uint32_t pre_max_peak_time_stamp;
    int32_t preAcc;
    int32_t pedometer_threshold;
    int32_t threshold_temp;
    int32_t pre_threshold;
    uint8_t stillCounter;
    uint16_t g_mag;
    uint32_t up_cross_time_stamp;
    uint32_t pre_up_cross_time_stamp;
    uint32_t down_cross_time_stamp;
    uint8_t down_down_flag;
    uint8_t threshold_counter;
    uint8_t freq_counter;
    int16_t double_cross_period;
    uint8_t run_flag;
    int16_t pre_cross_time_period;
    uint8_t double_step_counter;
    uint8_t triple_step_counter;
    uint8_t noise_flag;
    uint32_t pdr_total_stepCount;
    uint32_t pdr_total_stepLength;
    uint32_t total_step_count_reset;
    uint32_t total_step_length_reset;
    int32_t step_frequency;
    int32_t step_length;
    int8_t noise_flag_counter;
    int8_t valid_step_prediction;
    uint8_t stateMachineIdx;
    int8_t threshold_update_pending;
    uint8_t predict_counter;
    int8_t result_label;
#ifdef _STEP_DETECTOR_PARA_
    uint8_t step_detect;
    uint8_t detected_step;
#endif
    int32_t simRnd;
#ifdef ANTI_NOISE_ENABLE
    AccSignals_t_pedometer acc_param;
    Feature_t_pedometer pdr_features;
    Tree_pedometer tree_P;
#endif
#ifdef _ANTI_NOISE_TIME_CONSTAINT_
    uint32_t anti_noise_time_start;
#endif
} PedometerAlgorithmStruct;

typedef struct {
    int32_t max;
    int32_t min;
} normExtreme;


extern PedometerAlgorithmStruct PedoAlgoData;

//
// function declaration ==================================================
#ifdef PC_UTILITY
void load_pedometer_input_file(char* fileName);
#endif
int32_t getAccNorm_dsp(int32_t x, int32_t y, int32_t z);
int32_t normMax(int32_t *ipSignal, uint8_t arraySize, uint8_t windowLength, uint8_t currentIdx);
int32_t normMin(int32_t *ipSignal, uint8_t arraySize, uint8_t windowLength, uint8_t currentIdx);
normExtreme norm_extreme_cal(int32_t *ipSignal, uint8_t arraySize, uint8_t windowLength, uint8_t currentIdx);
uint8_t index_update(uint8_t current_index, uint8_t arraySize);
int32_t LPF(int32_t originalSignal, int32_t *Reg);
void smd_still_detection(int32_t acc_var, int32_t acc_norm_diff);
void adaptiveSamplingRate(void);
PedometerAlgorithmStruct *get_pedometer_struct();

#ifdef ANTI_NOISE_ENABLE
// ==================================================
// function declare for predict noise functions
// ==================================================

// predict noise main function
int8_t predict_noise(int32_t accX, int32_t accY, int32_t accZ, int32_t accnorm);

// random forest functions
int8_t pedometer_noise_predict(Feature_t_pedometer *input, Tree_pedometer *tree);
Tree_pedometer *get_pedometer_rf_para_ptr();

// get data buf pointer function
AccSignals_t_pedometer *get_pedometer_acc_buffer_ptr();
Feature_t_pedometer *get_pedometer_feature_buffer_ptr();

void get_pedometer_signal_acc(int32_t accX, int32_t accY, int32_t accZ, int32_t accnorm);
void extract_pedometer_feature_acc(AccSignals_t_pedometer *accelerations_ptr, Feature_t_pedometer *features);
void get_pedometer_max_mean(UFeatureSets_t_pedometer *u_feature_sets, Feature_t_pedometer *features);
void get_pedometer_std_rms(UFeatureSets_t_pedometer *u_feature_sets, Feature_t_pedometer *features);
void get_pedometer_crossrate(UFeatureSets_t_pedometer *u_feature_sets, Feature_t_pedometer *features);
void get_pedometer_iqr(UFeatureSets_t_pedometer *u_feature_sets, Feature_t_pedometer *features);

//HC detect state machine, return 1 for noise, This function is used only in main()
uint8_t votefornoise(int8_t *noise_detection_queue);
#endif
#endif
