/* MediaTek Inc. (C) 2016. All rights reserved.
 *
 * Copyright Statement:
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 */

#ifndef __MTK_AUDIO_COMPONENT_ENGINE__EXP_H__
#define __MTK_AUDIO_COMPONENT_ENGINE__EXP_H__

#define BCV_UP_BIT           0
#define BCV_DOWN_BIT         0x100
#define BCV_SIMPLE_SHIFT_BIT 0x200

typedef enum {
    BCV_PAR_SET_PCM_FORMAT,
    BCV_PAR_GET_PCM_FORMAT,
    BCV_PAR_SET_SAMPLE_RATE,
    BCV_PAR_GET_SAMPLE_RATE,
    BCV_PAR_SET_CHANNEL_NUMBER,
    BCV_PAR_GET_CHANNEL_NUMBER,
} BCV_PARAMETER;

typedef enum {
    BCV_IN_Q1P15_OUT_Q1P31    = BCV_UP_BIT + 0,     // 16-bit Q1.15 input, 32-bit Q1.31 output
    BCV_IN_Q1P31_OUT_Q1P15    = BCV_UP_BIT + 1,     // 32-bit Q1.31 input, 16-bit Q1.15 output
    BCV_IN_Q9P23_OUT_Q1P31    = BCV_UP_BIT + 2,     // 32-bit Q9.23 input, 32-bit Q1.31 output
    BCV_IN_Q1P31_OUT_Q9P23    = BCV_UP_BIT + 3,     // 32-bit Q1.31 input, 32-bit Q9.23 output
    BCV_IN_Q1P15_OUT_Q9P23    = BCV_UP_BIT + 4,     // 16-bit Q1.15 input, 32-bit Q9.23 output
    BCV_IN_Q9P23_OUT_Q1P15    = BCV_UP_BIT + 5,     // 32-bit Q9.23 input, 16-bit Q1.15 output
    BCV_IN_Q1P15_OUT_Q1P23    = BCV_UP_BIT + 6,     // 16-bit Q1.15 input, 24-bit Q1.23 output
    BCV_IN_Q1P23_OUT_Q1P15    = BCV_UP_BIT + 7,     // 24-bit Q1.23 input, 16-bit Q1.15 output
    BCV_IN_Q1P31_OUT_Q1P23    = BCV_UP_BIT + 8,     // 32-bit Q1.31 input, 24-bit Q1.23 output
    BCV_IN_Q1P23_OUT_Q1P31    = BCV_UP_BIT + 9,     // 24-bit Q1.23 input, 32-bit Q1.31 output
    BCV_IN_Q9P23_OUT_Q1P23    = BCV_UP_BIT + 10,    // 32-bit Q9.23 input, 24-bit Q1.23 output
    BCV_IN_Q1P23_OUT_Q9P23    = BCV_UP_BIT + 11,    // 24-bit Q1.23 input, 32-bit Q9.23 output
    BCV_UP_BIT_END            = BCV_UP_BIT + 12,    // End of up-bits. Do Not use this

    BCV_IN_Q33P31_OUT_Q1P31   = BCV_DOWN_BIT + 0,   // 64-bit Q33.31 input, 32-bit Q1.31 output
    BCV_DOWN_BIT_END          = BCV_DOWN_BIT + 1,   // End of down-bits. Do Not use this

    BCV_SIMPLE_SHIFT_BIT_END  = BCV_SIMPLE_SHIFT_BIT + 0,   // End of simple down-bits. Do Not use this
} BCV_PCM_FORMAT;

typedef enum {
    SRC_IN_Q1P15_OUT_Q1P15 = 0,   // 16-bit Q1.15 input, 16-bit Q1.15 output
    SRC_IN_Q1P15_OUT_Q1P31 = 1,   // 16-bit Q1.15 input, 32-bit Q1.31 output
    SRC_IN_Q9P23_OUT_Q1P31 = 2,   // 32-bit Q9.23 input, 32-bit Q1.31 output
    SRC_IN_Q1P31_OUT_Q1P31 = 3,   // 32-bit Q1.31 input, 32-bit Q1.31 output
    SRC_IN_END,
} SRC_PCM_FORMAT;

typedef enum {
    SRC_PAR_SET_PCM_FORMAT,
    SRC_PAR_GET_PCM_FORMAT,
    SRC_PAR_SET_INPUT_SAMPLE_RATE,
    SRC_PAR_GET_INPUT_SAMPLE_RATE,
    SRC_PAR_SET_OUTPUT_SAMPLE_RATE,
    SRC_PAR_GET_OUTPUT_SAMPLE_RATE,
    SRC_PAR_SET_INPUT_CHANNEL_NUMBER,
    SRC_PAR_GET_INPUT_CHANNEL_NUMBER,
    SRC_PAR_SET_OUTPUT_CHANNEL_NUMBER,
    SRC_PAR_GET_OUTPUT_CHANNEL_NUMBER,
} SRC_PARAMETER;


typedef enum {
    ACE_SUCCESS = 0,
    ACE_INVALIDE_PARAMETER,
    ACE_INVALIDE_OPERATION,
    ACE_NOT_INIT,
    ACE_NOT_OPEN,
} ACE_ERRID;

typedef enum {
    ACE_STATE_NONE,
    ACE_STATE_INIT,
    ACE_STATE_OPEN,
} ACE_STATE;

#endif // __MTK_AUDIO_COMPONENT_ENGINE__EXP_H__
