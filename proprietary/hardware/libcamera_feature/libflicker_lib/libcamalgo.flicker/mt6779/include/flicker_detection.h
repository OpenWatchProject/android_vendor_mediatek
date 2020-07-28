#pragma once
#ifndef _FLICKER_DETECTION_H_
#define _FLICKER_DETECTION_H_

#include "sequential_testing.h"
#include "flicker_type.h"

typedef enum {
    NORMAL = 0,
    IMX576,
    S5K2X5
} SensorType;

typedef struct {
    int32_t flicker_freq[9];
    int32_t flicker_grad_threshold;
    int32_t flicker_search_range;
    int32_t min_past_frames;
    int32_t max_past_frames;
    FlickerStatistics ev50_l50;
    FlickerStatistics ev50_l60;
    FlickerStatistics ev60_l50;
    FlickerStatistics ev60_l60;
    int32_t ev50_thresholds[2];
    int32_t ev60_thresholds[2];
    int32_t freq_feature_index[2];
    // for zHDR raw image input
    int8_t is_zhdr;
    int8_t zhdr_se_small;
    int8_t zhdr_gle_first;
} FlkExtPara;

typedef struct {
    SensorType type;
    int32_t line_length;
    int32_t column_length;
    int32_t window_width;
    double pclk;
    int32_t block_height;
    int32_t green_only;
} FlkInitPara;

typedef struct {
    SensorType type;
    void *c_list0;
    void *c_list1;
    int32_t n_win_h;
    int32_t n_win_w;
    int32_t win_wd;
    int32_t win_ht;
    int32_t cur_freq;
    FlkSensorInfo sensor_info;
    FlkEISVector eis_vec;
    FlkAEInfo ae_info;
    int32_t *af_stat;
} FlkDetectPara;

typedef struct {
    EvTable ev;
} FlkStatePara;

typedef struct {
    int32_t log_level;
} FlkLogPara;

// status from local motion vectors: large inter-frame motion & small inter-frame motion
typedef enum LmvStatus { LARGE_MOTION, SMALL_MOTION } LmvStatus;

typedef enum FlkCmd {
    CMD_INIT = 0,
    CMD_UNINIT,
    CMD_DETECT,
    CMD_SET_EXT_PARA,
    CMD_SET_STATE,
    CMD_RESET_QUEUE,
    CMD_SET_LOG_LEVEL
} FlkCmd;

void SendCmdFlk(FlkCmd cmd, void *input, void *output);

#ifdef FLK_PC_SIMULATION
void SetFrameIdxFlk(int32_t index);
void SetFilePtrFlk(FILE* fptr);
void SetFilePtrVs(FILE* fptr);
void SetFilePtrAl(FILE* fptr);
#endif

#endif