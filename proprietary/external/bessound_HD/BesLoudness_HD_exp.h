/*****************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of MediaTek Inc. (C) 2005
*
*  BY OPENING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
*  THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
*  RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON
*  AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
*  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
*  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
*  NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
*  SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
*  SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK ONLY TO SUCH
*  THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
*  NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S
*  SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
*
*  BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE
*  LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
*  AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
*  OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY BUYER TO
*  MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
*
*  THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE
*  WITH THE LAWS OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF
*  LAWS PRINCIPLES.  ANY DISPUTES, CONTROVERSIES OR CLAIMS ARISING THEREOF AND
*  RELATED THERETO SHALL BE SETTLED BY ARBITRATION IN SAN FRANCISCO, CA, UNDER
*  THE RULES OF THE INTERNATIONAL CHAMBER OF COMMERCE (ICC).
*
*****************************************************************************/

/*****************************************************************************
 *
 * Filename:
 * ---------
 * BesLoudness_HD_exp.h
 *
 * Project:
 * --------
 * SWIP
 *
 * Description:
 * ------------
 * BesLoudness_HD interface
 *
 * Author:
 * -------
 * Scholar Chang
 *
 *============================================================================
 *             HISTORY
 * Below this line, this part is controlled by PVCS VM. DO NOT MODIFY!!
 *------------------------------------------------------------------------------
 * $Revision$
 * $Modtime$
 * $Log$
 *
 *------------------------------------------------------------------------------
 * Upper this line, this part is controlled by PVCS VM. DO NOT MODIFY!!
 *============================================================================
 ****************************************************************************/

#ifndef __BESLOUDNESS_HD_EXP_H__
#define __BESLOUDNESS_HD_EXP_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "BesSound_HD_exp.h"

// Notice: All the buffer pointers are required to be
//         four-byte-aligned to avoid the potential on-target process error !!!

/*
                           +------- Main Feature Version
                           |+------ Sub Feature Version
                           ||+----- Performance Improvement Version
                           |||
    BESLOUDNESS_VERSION  0xABC
                           |||
            +--------------+||
            | +-------------+|
            | | +------------+
            | | |
    Version 1.0.0: (HP Cheng)
        First release (1 HPF, 2 BPFs, DRC, Limiter)
    Version 2.0.0: (HP Cheng)
        Add wave-shaping
    Version 3.0.0: (WN Chen)
        Support DRC curve tuning for customization
        Support wave-shaping max / min gain tuning
        Support selectable filter position (before DRC for loudness / after DRC for frequency response)
    Version 4.0.0: (Scholar Chang)
        Remove 640 sample delay (DRC 512 sample + limiter 128 sample)
        Allow attack / release profile to be adjustable at the initialization stage
        Add some 2nd-order IIR filters (+1 HPF, +1 LPF, +4 BPFs)
        Increase the filter coefficient precision for headphone compensation filter (HCF)
        Add ramp-up / ramp-down overlap-and-add mechanism
        Add AudEnh
    Version 4.1.0: (Scholar Chang)
        Support 32-bit PCM input / output
        Support dual loudspeakers with different filter coefficients
    Version 5.0.0: (Scholar Chang)
        Support multi-band dynamic range control (MBDRC)
    Version 5.1.0: (Scholar Chang)
        Add dynamic notch filter & limiter gain ramping
    Version 5.2.0: (Scholar Chang)
        Modify filter design interface
    Version 5.3.0: (Scholar Chang)
        Support 192kHz sampling rate ACF & HCF.
*/

/***************************************************************************************
|  Enumerator Definition
|**************************************************************************************/
typedef enum {
    BLOUD_HD_MONO    = 1,  // Mono
    BLOUD_HD_STEREO  = 2,  // Stereo
} BLOUD_HD_CHANNEL;

typedef enum {
    BLOUD_HD_FALSE = 0,    // False
    BLOUD_HD_TRUE  = 1,    // True
} BLOUD_HD_BOOL;

typedef enum {
    HD_FILT_MODE_NONE            = 0, // Disable Filter
    HD_FILT_MODE_COMP_FLT        = 1, // Compenastion Filter
    HD_FILT_MODE_LOUD_FLT        = 2, // Loudness Filter
    HD_FILT_MODE_COMP_HDP        = 3, // Compensation Filter for Headphone
    HD_FILT_MODE_AUD_ENH         = 4, // Enable BesAudEnh Algorithnm
    HD_FILT_MODE_LOW_LATENCY_ACF = 5  // Low latency audio compensation filter
} HD_FILT_MODE_ENUM;

typedef enum {
    HD_LOUD_MODE_NONE       = 0, // None DRC
    HD_LOUD_MODE_BASIC      = 1, // Basic DRC
    HD_LOUD_MODE_ENHANCED   = 2, // Enhanced DRC
    HD_LOUD_MODE_AGGRESSIVE = 3, // Aggressive DRC
} HD_LOUD_MODE_ENUM;

typedef enum {
    HD_S2M_MODE_NONE     = 0, // None
    HD_S2M_MODE_ST2MO2ST = 1, // Stereo to mono to stereo
} HD_S2M_MODE_ENUM;

typedef enum {
    BLOUDHD_IN_Q1P15_OUT_Q1P15  = 0,   // 16-bit Q1.15  input, 16-bit Q1.15 output
    BLOUDHD_IN_Q1P31_OUT_Q1P31  = 1,   // 32-bit Q1.31  input, 32-bit Q1.31 output
} BLOUDHD_PCM_FORMAT;

typedef enum {
    BLOUD_HD_TO_NORMAL_STATE = 0,  // Change to normal state
    BLOUD_HD_TO_BYPASS_STATE = 1,  // Change to bypass state
    BLOUD_HD_CHANGE_MODE     = 2,  // Change mode parameters
    BLOUD_HD_RESET           = 3,  // Clear history buffer
    BLOUD_HD_SET_OUTPUT_GAIN = 4,  // Set output gain
    BLOUD_HD_CHANGE_NOTCH    = 5,  // Change notch filter parameters
} BLOUD_HD_COMMAND;

typedef enum {
    BLOUD_HD_NORMAL_STATE    = 0,  // Normal state
    BLOUD_HD_SWITCHING_STATE = 1,  // Switching state, ignore all commands
    BLOUD_HD_BYPASS_STATE    = 2,  // Bypass state
} BLOUD_HD_STATE;

typedef enum {
    BLOUD_HD_SB_MAKEUP_GAIN = 0,    // Apply subband makeup gain
    BLOUD_HD_SB_LIMITER     = 1,    // Apply subband limiter
    BLOUD_HD_SB_BYPASS      = 2,    // Bypass subband
    BLOUD_HD_SB_MUTE        = 3     // Mute subband
} BLOUD_HD_SB_MODE;                 // Subband mode

/***************************************************************************************
|  Structure Definition
|**************************************************************************************/

/********************************************************************************|
|  STRUCTURE                                                                     |
|                                                                                |
|   BLOUD_HD_IIR_DESIGN_PARAM                                                    |
|                                                                                |
|  DESCRIPTION                                                                   |
|                                                                                |
|   IIR filter design parameters for on-target coefficient generation (MT6595)   |
|                                                                                |
|  MEMBERS                                                                       |
|                                                                                |
|   hpf_fc              HPF cutoff frequency        unit: Hz                     |
|   hpf_order           HPF order                                                |
|   lpf_fc              LPF cutoff frequency        unit: Hz                     |
|   lpf_order           LPF order                                                |
|   bpf_fc[8]           BPF center frequency array  unit: Hz                     |
|   bpf_bw[8]           BPF bandwidth array         unit: Hz                     |
|   bpf_gain[8]         BPF gain array              unit: dB (Q24.8)             |
|                                                                                |
 ********************************************************************************/

typedef struct {
    unsigned int hpf_fc;
    unsigned int hpf_order;
    unsigned int lpf_fc;
    unsigned int lpf_order;
    unsigned int bpf_fc[8];
    unsigned int bpf_bw[8];
    int bpf_gain[8];
} BLOUD_HD_IIR_Design_Param;

/***************************************************************************************
|   STRUCTURE
|
|       BLOUD_HD_FilterCoef
|
|   MEMBERS
|
|   HPF_COEF [2][5]         High-pass filter coefficients
|             |  |                             -1        -2
|             |  |                  b0 + b1 x z  + b2 x z
|             |  |          H(z) = -------------------------
|             |  |                             -1        -2
|             |  |                   1 + a1 x z  + a2 x z
|             |  |
|             +-----------> HPF index
|                |          0: HPF1
|                |          1: HPF2
|                |
|                +--------> HPF coefficients
|                           (FILT_MODE_COMP_FLT)
|                           0:  b0, format: signed Q5.27
|                           1:  b1, format: signed Q5.27
|                           2:  b2, format: signed Q5.27
|                           3: [-a1 | -a2], format: [signed Q2.14 | signed Q2.14]
|                           4:  all zeros
|                           (FILT_MODE_COMP_HDP)
|                           0:  b0, format: signed Q5.27
|                           1:  b1, format: signed Q5.27
|                           2:  b2, format: signed Q5.27
|                           3: [-a1 | -a2], format: [signed Q2.30 upper 16-bit | signed Q2.30 upper 16-bit]
|                           4: [-a1 | -a2], format: [signed Q2.30 lower 16-bit | signed Q2.30 lower 16-bit]
|
|   BPF_COEF [8][6]         Band-pass filter coefficients
|             |  |                             -1        -2
|             |  |                  b0 + b1 x z  + b2 x z
|             |  |          H(z) = -------------------------
|             |  |                             -1        -2
|             |  |                   1 + a1 x z  + a2 x z
|             |  |
|             +-----------> BPF index
|                |          0: BPF1
|                |          1: BPF2
|                |          2: BPF3
|                |          3: BPF4
|                |          4: BPF5
|                |          5: BPF6
|                |          6: BPF7
|                |          7: BPF8
|                |
|                +--------> BPF coefficients
|                           (FILT_MODE_COMP_FLT or FILT_MODE_COMP_HDP)
|                           5: BPF format flag (0 for Q2.14, 1 for Q5.27 & Q2.30, others undefined)
|
|                           If BPF format flag is 0,
|                           0: [ b0 |  b1], format: [signed Q2.14 | signed Q2.14]
|                           1: [ b2 | -a1], format: [signed Q2.14 | signed Q2.14]
|                           2: [-a2 |   0], format: [signed Q2.14 |    all zeros]
|
|                           If BPF format flag is 1,
|                           0:  b0, format: signed Q5.27
|                           1:  b1, format: signed Q5.27
|                           2:  b2, format: signed Q5.27
|                           3: [-a1 | -a2], format: [signed Q2.30 upper 16-bit | signed Q2.30 upper 16-bit]
|                           4: [-a1 | -a2], format: [signed Q2.30 lower 16-bit | signed Q2.30 lower 16-bit]
|
|   LPF_COEF [3]            Low-pass filter coefficients
|             |                                -1        -2
|             |                     b0 + b1 x z  + b2 x z
|             |             H(z) = -------------------------
|             |                                -1        -2
|             |                      1 + a1 x z  + a2 x z
|             |
|             +-----------> LPF coefficients
|                           (FILT_MODE_COMP_FLT or FILT_MODE_COMP_HDP)
|                           0: [ b0 |  b1], format: [signed Q2.14 | signed Q2.14]
|                           1: [ b2 | -a1], format: [signed Q2.14 | signed Q2.14]
|                           2: [-a2 |   0], format: [signed Q2.14 |    all zeros]
|
|**************************************************************************************/

typedef struct {
    unsigned int HPF_COEF[2][5];
    unsigned int BPF_COEF[8][6];
    unsigned int LPF_COEF[3];
    unsigned int notch_fc;      // unit: Hz, range 100 ~ 2000
    unsigned int notch_bw;      // unit: Hz, fc / bw is limited to 0.5 ~ 7.5
    int notch_th;               // Q24.8, unit: dBFS
    int output_gain;            // Q16.16, range: 1 ~ 0x10000
    unsigned int ramp_smpl_cnt; // unit: sample, range: 0x00000000 ~ 0xFFFFFFFF
}BLOUD_HD_FilterCoef;

/***************************************************************************************
|   STRUCTURE
|
|       BLOUD_HD_CustomParam
|
|   MEMBERS
|
|   WS_Gain_Max             Wave shaping maximum gain, format Q2.14, set to 0 would use the default value
|   WS_Gain_Min             Wave shaping minimum gain, format Q2.14, set to 0 would use the default value
|
|   Filter_First            Choose the position of the DRC / filter stage order
|                           0: DRC First
|                           1: Filter First
|
|   Sep_LR_Filter           Separate the filter coefficients for stereo input L / R channels
|                           0: Use same filter for both L / R
|                           1: Separate L / R filter
|
|   Num_Bands               Number of subbands for multi-band DRC
|                           Range: 1 ~ 8
|
|   Flt_Bank_Order          Filter order for filter bank
|                           Range: 3, 5, 7
|
|   Cross_Freq[7]           Subband crossover frequencies, unit: Hz
|              |
|              +--------->  Crossover frequency index
|                           0: Crossover frequency between subband 1 & 2 (Lowest)
|                           1: Crossover frequency between subband 2 & 3
|                           2: Crossover frequency between subband 3 & 4
|                           3: Crossover frequency between subband 4 & 5
|                           4: Crossover frequency between subband 5 & 6
|                           5: Crossover frequency between subband 6 & 7
|                           6: Crossover frequency between subband 7 & 8 (Highest)
|
|   DRC_Th[8][5]            DRC curve threshold for input signal, Q24.8, unit: dB
|          |  |
|          +------------->  Subband index
|             |             0: Subband 1 (Lowest)
|             |             1: Subband 2
|             |             2: Subband 3
|             |             3: Subband 4
|             |             4: Subband 5
|             |             5: Subband 6
|             |             6: Subband 7
|             |             7: Subband 8 (Highest)
|             |
|             +---------->  Input level index
|                           0: Th, noise gate threshold
|                           1: Ex, expansion end point
|                           2: Cp, compression start point
|                           3: Ed, target level
|                           4: Lm, limitation end point
|
|   DRC_Gn[8][5]            DRC curve gain, Q24.8, unit: dB
|          |  |
|          +------------->  Subband index
|             |             0: Subband 1 (Lowest)
|             |             1: Subband 2
|             |             2: Subband 3
|             |             3: Subband 4
|             |             4: Subband 5
|             |             5: Subband 6
|             |             6: Subband 7
|             |             7: Subband 8 (Highest)
|             |
|             +---------->  Input level index
|                           0: Th, noise gate threshold
|                           1: Ex, expansion end point
|                           2: Cp, compression start point
|                           3: Ed, target level
|                           4: Lm, limitation end point
|
|   SB_Gn[8]                Subband gain, Q24.8, unit: dB
|         |
|         +-------------->  Subband index
|                           0: Subband 1 (Lowest)
|                           1: Subband 2
|                           2: Subband 3
|                           3: Subband 4
|                           4: Subband 5
|                           5: Subband 6
|                           6: Subband 7
|                           7: Subband 8 (Highest)
|
|   SB_Mode[8]              Subband mode
|           |               0: makeup gain
|           |               1: subband limiter
|           |               2: bypass
|           |               3: mute
|           |
|           +------------>  Subband index
|                           0: Subband 1 (Lowest)
|                           1: Subband 2
|                           2: Subband 3
|                           3: Subband 4
|                           4: Subband 5
|                           5: Subband 6
|                           6: Subband 7
|                           7: Subband 8 (Highest)
|
|   DRC_Delay               (Reserved)
|
|   Att_Time[8][6]          Attack time of each subband, unit: 0.1 ms / 6dB
|            |  |
|            +----------->  Subband index
|               |           0: Subband 1 (Lowest)
|               |           1: Subband 2
|               |           2: Subband 3
|               |           3: Subband 4
|               |           4: Subband 5
|               |           5: Subband 6
|               |           6: Subband 7
|               |           7: Subband 8 (Highest)
|               |
|               +-------->  Input level region index
|                           0: Region          < Th
|                           1: Region >= Th && < Ex
|                           2: Region >= Ex && < Cp
|                           3: Region >= Cp && < Ed
|                           4: Region >= Ed && < Lm
|                           5: Region >= Lm
|
|   Rel_Time[8][6]          Release time of each subband, unit: 0.1 ms / 6dB
|            |  |
|            +----------->  Subband index
|               |           0: Subband 1 (Lowest)
|               |           1: Subband 2
|               |           2: Subband 3
|               |           3: Subband 4
|               |           4: Subband 5
|               |           5: Subband 6
|               |           6: Subband 7
|               |           7: Subband 8 (Highest)
|               |
|               +-------->  Input level region index
|                           0: Region          < Th
|                           1: Region >= Th && < Ex
|                           2: Region >= Ex && < Cp
|                           3: Region >= Cp && < Ed
|                           4: Region >= Ed && < Lm
|                           5: Region >= Lm
|
|   Hyst_Th[8][6]           Release hysteresis threshold, Q24.8, unit: dB
|           |  |
|           +------------>  Subband index
|              |            0: Subband 1 (Lowest)
|              |            1: Subband 2
|              |            2: Subband 3
|              |            3: Subband 4
|              |            4: Subband 5
|              |            5: Subband 6
|              |            6: Subband 7
|              |            7: Subband 8 (Highest)
|              |
|              +--------->  Input level region index
|                           0: Region          < Th
|                           1: Region >= Th && < Ex
|                           2: Region >= Ex && < Cp
|                           3: Region >= Cp && < Ed
|                           4: Region >= Ed && < Lm
|                           5: Region >= Lm
|
|   Lim_Th                  Limiter threshold, Q17.15, range: 1 ~ 0x7FFF
|
|   Lim_Gn                  Limiter gain, Q17.15, range: 1 ~ 0x7FFF
|
|   Lim_Const               Limiter recovery constant, unit: step size / sample,
|                           range: 1 ~ 100
|
|   Lim_Delay               (Reserved)
|
|   Disable_Limiter         Disable limiter for low latency
|                           0: Limiter on (64-sample latency for look-ahead buffer)
|                           1: Limiter off (directly clipping, no extra latency)
|**************************************************************************************/

typedef struct {
    unsigned int WS_Gain_Max;       // Q2.14
    unsigned int WS_Gain_Min;       // Q2.14
    unsigned int Filter_First;      // 0: DRC First, 1: Filter First
    unsigned int Sep_LR_Filter;     // 0: Use same filter for both L / R,
                                    // 1: Separate L / R filter
    unsigned int Num_Bands;         // range: 1 ~ 8
    unsigned int Flt_Bank_Order;    // range: 3, 5, 7
    unsigned int Cross_Freq[7];     // unit: Hz
    int DRC_Th[8][5];               // Q24.8, unit: dB
    int DRC_Gn[8][5];               // Q24.8, unit: dB
    int SB_Gn[8];                   // Q24.8, unit: dB
    unsigned int SB_Mode[8];        // 0: makeup gain, 1: subband limiter,
                                    // 2: bypass, 3: mute
    unsigned int DRC_Delay;
    unsigned int Att_Time[8][6];    // unit: 0.1 ms / 6dB
    unsigned int Rel_Time[8][6];    // unit: 0.1 ms / 6dB
    int Hyst_Th[8][6];              // Q24.8, unit: dB
    int Lim_Th;
    int Lim_Gn;
    unsigned int Lim_Const;
    unsigned int Lim_Delay;
    unsigned int Disable_Limiter;   // 0: Limiter on, 1: Limiter off (only for LLACF mode)
}BLOUD_HD_CustomParam;

/***************************************************************************************
|   STRUCTURE
|
|       BLOUD_HD_ModeParam
|
|   MEMBERS
|
|   Filter_Mode             Audio compensation filter mode selection
|                           0 (HD_FILT_MODE_NONE):
|                               Disable audio compensation filter
|                           1 (HD_FILT_MODE_COMP_FLT):
|                               Use the customized filter coefficients for loudspeaker
|                           2 (HD_FILT_MODE_LOUD_FLT):
|                               If no customized filter coefficients,
|                               SWIP would apply the default loudness filters
|                           3 (HD_FILT_MODE_COMP_HDP):
|                               Use the customized filter coefficients for headphone,
|                               which support more precise filter resolution for HPF
|                           4 (HD_FILT_MODE_AUD_ENH):
|                               Use the BesLoudness HD engine built-in audio enhancement
|                               filter coefficients to achieve the best headphone audio
|                               quality for MTK recommended audio headphone device.
|                           5 (HD_FILT_MODE_LOW_LATENCY_ACF):
|                               Use the customized filter coefficients for low latency
|                               loudspeaker mode.
|                               The block size of this mode is 64 samples.
|                               The block size of other modes are 512 samples.
|                               1 pair of L / R samples counts as 1 sample.
|
|   Loudness_Mode           Loudness mode selection
|                           0 (HD_LOUD_MODE_NONE):
|                               Disable dynamic range control (DRC)
|                           1 (HD_LOUD_MODE_BASIC):
|                               Enable DRC
|                           2 (HD_LOUD_MODE_ENHANCED):
|                               Enable DRC and basic wave shaping,
|                               only valid under filter mode: HD_FILT_MODE_LOUD_FLT
|                           3 (HD_LOUD_MODE_AGGRESSIVE):
|                               Enable DRC and aggressive wave shaping
|                               only valid under filter mode: HD_FILT_MODE_LOUD_FLT
|
|   S2M_Mode                Stereo to mono mode
|                           0 (HD_S2M_MODE_NONE):
|                               None
|                           1 (HD_S2M_MODE_ST2MO2ST):
|                               If input is stereo, then apply stereo to mono to stereo
|                               to save computational complexity and increase loudness
|
|   pCustom_Param           Pointer to the custom parameter fields
|
|   pFilter_Coef_L          Pointer to the filter coefficient fields
|                           These filter coefficients would be applied to
|                           1)  If Channel_Num == 1:
|                               Mono signal
|                           2)  If Channel_Num == 2 && Sep_LR_Filter == 0:
|                               Both L / R channel signals
|                           3)  If Channel_Num == 2 && Sep_LR_Filter == 1:
|                               L channel signal
|   pFilter_Coef_R          Pointer to the filter coefficient fields
|                           These filter coefficients would be applied to
|                           1)  If Channel_Num == 2 && Sep_LR_Filter == 1:
|                               R channel signal
|                           2)  Else:
|                               None
|
|**************************************************************************************/

typedef struct {
   unsigned int Filter_Mode;
   unsigned int Loudness_Mode;
   unsigned int S2M_Mode;
   BLOUD_HD_CustomParam *pCustom_Param;
   BLOUD_HD_FilterCoef  *pFilter_Coef_L;
   BLOUD_HD_FilterCoef  *pFilter_Coef_R;
} BLOUD_HD_ModeParam;

/***************************************************************************************
|   STRUCTURE
|
|       BLOUD_HD_InitParam
|
|   MEMBERS
|
|   Channel                 Input signal channel number
|                           1 (BLOUD_HD_MONO):
|                               Mono input signal
|                           2 (BLOUD_HD_STEREO):
|                               Stereo input signal (LRLRLR ...)
|
|   Sampling_Rate           Input signal sampling rate, unit: Hz
|                           Support 15 kinds of sampling rates:
|                           192000, 176400, 128000,
|                            96000,  88200,  64000,
|                            48000,  44100,  32000,
|                            24000,  22050,  16000,
|                            12000,  11025,   8000
|
|   PCM_Format              Input / output PCM format
|                           0 (BLOUD_HD_IN_Q1P15_OUT_Q1P15):
|                               16-bit Q1.15  input, 16-bit Q1.15 output
|                           1 (BLOUD_HD_IN_Q1P31_OUT_Q1P31):
|                               32-bit Q1.31  input, 32-bit Q1.31 output
|
|   Initial_State           Initial processing state
|                           2 (BLOUD_HD_BYPASS_STATE):
|                               Set initial state to "bypass state".
|                               The initial command is "BLOUD_HD_TO_NORMAL_STATE".
|                               The engine would enter the "switching state"
|                               and start to ramp up (fade-in)
|                               and then change to the "normal state".
|                           0 (BLOUD_HD_NORMAL_STATE) or others:
|                               Set initial state to "normal state".
|                               The initial command is "BLOUD_HD_TO_NORMAL_STATE".
|                               The engine would start processing without ramp up.
|
|   Frame_Length            Frame length setting (unit: sample count, where one pair
|                           of L / R counts as one sample)
|                           Support 2 kinds of frame length: 256 & 512 (default)
|
|   pMode_Param             Pointer to the mode parameter fields
|
|**************************************************************************************/

typedef struct {
    unsigned int Channel;
    unsigned int Sampling_Rate;
    unsigned int PCM_Format;
    unsigned int Initial_State;
    unsigned int Frame_Length;
    BLOUD_HD_ModeParam *pMode_Param;
} BLOUD_HD_InitParam;

/***************************************************************************************
|   STRUCTURE
|
|       BLOUD_HD_RuntimeParam
|
|   MEMBERS
|
|   Command                 Runtime process command.
|                           0 (BLOUD_HD_TO_NORMAL_STATE):
|                               [Bypass state]
|                                   The engine would enter the "switching state"
|                                   and start to ramp up
|                                   and then change to the "normal state".
|                               [Other state]
|                                   Ignore this command.
|                           1 (BLOUD_HD_TO_BYPASS_STATE):
|                               [Normal state]
|                                   The engine would enter the "switching state"
|                                   and start to ramp down
|                                   and then change to the "bypass state".
|                               [Other state]
|                                   Ignore this command.
|                           2 (BLOUD_HD_CHANGE_MODE):
|                               [Normal state]
|                                   The engine would enter the "switching state"
|                                   and automatically apply
|                                   ramp down --> switch mode parameters --> ramp up
|                                   and then back to the "normal state".
|                               [Bypass state]
|                                   The engine would enter the "switching state"
|                                   and automatically apply
|                                   ramp down --> switch mode parameters --> ramp up
|                                   and then back to the "bypass state".
|                               [Other state]
|                                   Ignore this command.
|                           3 (BLOUD_HD_RESET):
|                               [Normal state]
|                                   The engine would directly clear the history buffer.
|                                   Noise would be heard if no other external
|                                   ramp down / ramp up (ex: hardware mute)
|                                   is applied.
|                                   Usually use this command with hardware mute
|                                   to avoid the noise.
|                               [Bypass state]
|                                   The engine would directly clear the history buffer.
|                               [Other state]
|                                   Ignore this command.
|                           4 {BLOUD_HD_SET_OUTPUT_GAIN}:
|                               [Normal state]
|                                   Change output gain setting with assigned sample count.
|                               [Other state]
|                                   Ignore this command.
|                           5 {BLOUD_HD_CHANGE_NOTCH}:
|                               [Normal state]
|                                   Change notch filter.
|                               [Other state]
|                                   Ignore this command.
|
|                           Note:
|                               State switching would be protected by
|                               the overlap and add (OLA) mechanism
|                               to avoid the state-switching noise.
|                               Therefore, in-place process is not allowed
|                               since OLA would reference the input signal.
|
|   pMode_Param             Pointer to the mode parameter fields
|
|
|**************************************************************************************/

typedef struct {
    unsigned int Command;
    BLOUD_HD_ModeParam *pMode_Param;
} BLOUD_HD_RuntimeParam;

/***************************************************************************************
|   STRUCTURE
|
|       BLOUD_HD_RuntimeStatus
|
|   MEMBERS
|
|   State                   Current engine state
|                           0 (BLOUD_HD_NORMAL_STATE):
|                               The engine is in normal state.
|                           1 (BLOUD_HD_SWITCHING_STATE):
|                               The engine is in switching state.
|                               The engine would ignore all commands in this state.
|                           2 (BLOUD_HD_BYPASS_STATE):
|                               The engine is in bypass mode.
|                               Driver can now terminate this engine without noise.
|
|**************************************************************************************/

typedef struct {
   unsigned int State;
} BLOUD_HD_RuntimeStatus;

/***************************************************************************************
|  Function Definition
|**************************************************************************************/

int BLOUD_HD_SetHandle(BS_HD_Handle *p_handle);

/********************************************************************************|
|  FUNCTION                                                                      |
|                                                                                |
|   BLOUD_HD_Filter_Design                                                       |
|                                                                                |
|  DESCRIPTION                                                                   |
|                                                                                |
|   Design the filter coefficients for ACF & HCF                                 |
|                                                                                |
|  PARAMETERS                                                                    |
|                                                                                |
|   filter_mode   [i] Filter mode                                                |
|                     0: HD_FILT_MODE_NONE                                       |
|                     1: HD_FILT_MODE_COMP_FLT                                   |
|                     2: HD_FILT_MODE_LOUD_FLT                                   |
|                     3: HD_FILT_MODE_COMP_HDP                                   |
|                     4: HD_FILT_MODE_AUD_ENH                                    |
|                     5: HD_FILT_MODE_LOW_LATENCY_ACF                            |
|   sampling_rate [i] Sampling rate (unit: Hz)                                   |
|   p_param       [i] Pointer to the input filter design parameter structure     |
|   p_coef        [o] Pointer to the output filter coefficient structure         |
|                     Only HPF_COEF, BPF_COEF, and LPF_COEF would be modified    |
|                                                                                |
|  RETURNS                                                                       |
|                                                                                |
|   Return  0 if all the inputs are valid                                        |
|   Return -1 if one of p_param or p_coef is NULL pointer                        |
|   Return -2 if invalid input sampling_rate                                     |
|                                                                                |
 ********************************************************************************/
int BLOUD_HD_Filter_Design(
    unsigned int filter_mode,
    unsigned int sampling_rate,
    BLOUD_HD_IIR_Design_Param *p_param,
    BLOUD_HD_FilterCoef *p_coef);

#ifdef __cplusplus
}
#endif

#endif  // __BESLOUDNESS_HD_EXP_H__
