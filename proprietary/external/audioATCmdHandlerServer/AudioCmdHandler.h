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

/*****************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of MediaTek Inc. (C) 2009
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


/*******************************************************************************
 *
 * Filename:
 * ---------
 * AudioCmdHandler.h
 *
 * Project:
 * --------
 *   Android
 *
 * Description:
 * ------------
 *   The audio command handling interface API.
 *
 * Author:
 * -------
 *   Donglei Ji(mtk80823)
 *
 *******************************************************************************/

#ifndef _AUDIO_CMD_HANDLER_H_
#define _AUDIO_CMD_HANDLER_H_

/*=============================================================================
 *                              Include Files
 *===========================================================================*/

#include <pthread.h>

#include <audiocustparam/AudioCustParam.h>
using namespace android;

//<--- for speech parameters calibration
#define MAX_VOL_CATE 3
#define CUSTOM_VOL_STEP 7

#define SPH_MODE_NUM 4
#define FIR_INDEX_NUM 6
#define FIR_NUM_NB 45
#define SPH_ENHANCE_PARAM_NUM 16
#define SPH_COMMON_NUM 12

#define FIR_NUM_WB 90
#define MAX_FILE_NAME_LEN 128
//--->

typedef struct
{
    int param1;
    int param2;
    int param3;
    int param4;
    int param5;
    unsigned int recvDataLen;
} AudioCmdParam;

typedef enum
{
    ACHFailed = -3,
    ACHLoadFileFailed,
    ACHParamError,
    ACHSucceeded
} ACHStatus;

typedef enum{
    AUDIO_FLT_ACF = 0,
    AUDIO_FLT_HCF,
    AUDIO_FLT_ACF_SUB,
    AUDIO_FLT_MUSIC_DRC,
    AUDIO_FLT_RINGTONE_DRC,
    AUDIO_FLT_MAX
}AudioFltTunningType;

typedef enum
{
    Analog_MIC1_Single = 1,
    Analog_MIC2_Single = 2,
    Analog_MIC_Dual = 9,
    Audio_I2S_IN = 98,
    Audio_FM_IN = 99,

    AUDIO_SOURCE_LIST_END
} AudioSourceType;

//<--- for speech parameters calibration
typedef enum
{
    LOAD_VOLUME_POLICY =    0,
    SET_FM_SPEAKER_POLICY,

    AUDIO_POLICY_CNT
} SetPolicyParameters;

typedef enum
{
    DUAL_MIC_REC_PLAY_STOP = 0,
    DUAL_MIC_REC,
    DUAL_MIC_REC_PLAY,
    DUAL_MIC_REC_PLAY_HS,
    DUAL_MIC_REC_HF,
    DUAL_MIC_REC_PLAY_HF,
    DUAL_MIC_REC_PLAY_HS_HF,
    FIR_REC,
    FIR_REC_STOP,

    DMNR_TUNING_CMD_CNT
} TuningCmdType;

typedef enum
{
    DUAL_MIC_UL_GAIN = 0,
    DUAL_MIC_DL_GAIN,
    DUAL_MIC_HSDL_GAIN,
    DUAL_MIC_UL_GAIN_HF,

    DUAL_MIC_GAIN_CNT
} GainType;

// for speech calibration
typedef struct
{
    unsigned short sph_com_param[SPH_COMMON_NUM];
    unsigned short sph_mode_param[SPH_MODE_NUM][SPH_ENHANCE_PARAM_NUM];
    short sph_in_fir[FIR_INDEX_NUM][FIR_NUM_NB];
    short sph_out_fir[FIR_INDEX_NUM][FIR_NUM_NB];
    short sph_output_FIR_coeffs[SPH_MODE_NUM][FIR_INDEX_NUM][FIR_NUM_NB];
    short selected_FIR_output_index[SPH_MODE_NUM];
} AUD_SPH_PARAM_STRUCT;

typedef struct
{
    unsigned short nb_dl_sph_mode_param[SPH_MODE_NUM][SPH_ENHANCE_PARAM_NUM];
	unsigned short wb_ul_dl_sph_mode_param[SPH_MODE_NUM][SPH_ENHANCE_PARAM_NUM];
} AUD_SPH_AFTER_FOUR_MODE_PARAM_STRUCT;
// for WB speech calibration
typedef struct
{
    unsigned short sph_mode_wb_param[SPH_MODE_NUM][SPH_ENHANCE_PARAM_NUM]; //WB speech enhancement
    short sph_wb_in_fir[FIR_INDEX_NUM][FIR_NUM_WB]; // WB speech input FIR
    short sph_wb_out_fir[FIR_INDEX_NUM][FIR_NUM_WB];// WB speech output FIR
} AUD_SPH_WB_PARAM_STRUCT;

typedef struct
{
    unsigned short cmdType;
    unsigned short selected_fir_index;
    unsigned short dlDGGain;
    unsigned short dlPGA;
    unsigned short phone_mode;
    unsigned short wb_mode;
    char audio_file[MAX_FILE_NAME_LEN];
} AUD_TASTE_PARAM_STRUCT;

typedef struct
{
    char input_file[MAX_FILE_NAME_LEN];
    char output_file[MAX_FILE_NAME_LEN];
} DMNRTuningFileName;

#define MAX_NAME_LEN 16
#define MAX_SUB_ITEM_NUM 4

typedef struct _PCDispSubItem
{
    char outputDevice[MAX_NAME_LEN];
	unsigned short AnalogPoint;               // PGA and Ext Amp
} PCDispSubItem;                           // size:  18
typedef struct _PCDispItem
{
    char strType[MAX_NAME_LEN];                
	unsigned short level;
	unsigned int subItemNum;
	PCDispSubItem subItem[MAX_SUB_ITEM_NUM];  //3 //3 -> 4
} PCDispItem;                              // size:  22 + 18*4 = 94


#define MAX_NAME_LEN_MIC 32
#define MAX_SUB_ITEM_NUM_MIC 32
typedef struct _PCDispMic
{
    char strType[MAX_NAME_LEN_MIC];
	unsigned int subItemNum;
	char subItem[MAX_SUB_ITEM_NUM_MIC][MAX_NAME_LEN_MIC];  //24  -> 32
} PCDispMic;                               //size:  1060

#define MAX_GAIN_POINT_NUM 12
#define MAX_GAIN_STEP 32
#define MAX_ITEM_NUM 20
typedef struct _PCDispTotolStru
{
	unsigned int itemNum;
	PCDispItem DispItem[MAX_ITEM_NUM];        // 16 -> 20
	PCDispMic DispMic;        
	unsigned short gainRangeNum;              // 9->12
	signed char gainRange[MAX_GAIN_POINT_NUM][MAX_GAIN_STEP];   // 9-> 12
} PCDispTotolStru;
static PCDispTotolStru mDispStru;
static PCDispItem mBtNrecDispItem;

/*
static char gainRangeCopy[MAX_GAIN_POINT_NUM][MAX_GAIN_STEP] = 
	{{-5,  	-3,   -1,   1,    3,    5,    7,    9,    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
	 {-21, 	-19,  -17,  -15,  -13,  -11,  -9,   -7,   -5,   -3,   -1,   -1,   3,    5,    7,    9,    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
	 {0,  	45,   0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
	 {0,  	 6,   12,   20,   0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
	 {0,   	-1,   -2,   -3,   -4,   -5,   -6,   -7,   -8,   -9,   -10,  -11,  -12,  -13,  -14,  -15,  -16,  -17,  -18,  -19,  -20,  -21,  -24,  -27,  -30,  -33,  -36,  -39,  -42,  -45,  -50,  -60 },
	 {0,   	6,    12,   20,   0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
	 {0,   	-1,   -2,   3,    -4,   -5,   -6,   -7,   -8,   -9,   -10,  -11,  -12,  -13,  -14,  -15,  -16,  -17,  -18,  -19,  -20,  -21,  -24,  -27,  -30,  -33,  -36,  -39,  -42,  -45,  -50,  -60 },
	 {0, 	32,   0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
	 {0, 	32,   0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
	 {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
	 {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
	 {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}};
*/
//--->

/*=============================================================================
 *                              Class definition
 *===========================================================================*/

class AudioCmdHandler;

class AudioCmdHandler
{
    public:
        AudioCmdHandler();
        ~AudioCmdHandler();
        AudioCmdHandler(const AudioCmdHandler &);
        AudioCmdHandler &operator=(const AudioCmdHandler &);

        // The  functions of processing audio cmds
        ACHStatus setRecorderParam(AudioCmdParam &audioCmdParams);
        ACHStatus RealTimeSetVolume(int iStreamType, int VolumeIndex,int iDeviceType);
        ACHStatus getVolume(int *VolumeIndex);

        //<--- for speech parameters calibration
        ACHStatus ULCustSPHParamFromNV(char *pParam, int *len, int block);
        ACHStatus DLCustSPHParamToNV(char *pParam, int block);

		ACHStatus DLCustSPHMagiParamToNV(char *pParam);
		ACHStatus ULCustSPHMagiParamFromNV(char *pParam, int *len);
		ACHStatus DLCustSPHHACParamToNV(char *pParam);
		ACHStatus ULCustSPHHACParamFromNV(char *pParam, int *len);
		ACHStatus DLCustSPHLPBKParamToNV(char *pParam);
		ACHStatus ULCustSPHLPBKParamFromNV(char *pParam, int *len);
        ACHStatus ULCustSPHVolumeParamFromNV(char *pParam, int *len);
        ACHStatus DLCustSPHVolumeParamToNV(char *pParam);

        ACHStatus ULCustSPHWBParamFromNV(char *pParam, int *len, int block);
        ACHStatus DLCustSPHWBParamToNV(char *pParam, int block);

        ACHStatus DLCustAudioFLTParamToNV(char *pParam,AudioFltTunningType eFLTID);
        ACHStatus ULCustAudioFLTParamFromNV(char *pParam, int *len,AudioFltTunningType eFLTID);

		ACHStatus DLCustHFPParamToNV(char *pParam);
		ACHStatus ULCustHFPParamFromNV(char *pParam, int *len);

        ACHStatus AudioTasteRunning(unsigned short cmdType, char *pParam = NULL);

        ACHStatus ULCustDualMicParamFromNV(char *pParam, int *len, int block);
        ACHStatus DLCustDualMicParamToNV(char *pParam, int block);

        ACHStatus getDMNRGain(unsigned short cmdType, char *pParam, int *len);
        ACHStatus setDMNRGain(unsigned short cmdType, unsigned short gain);

        ACHStatus AudioDMNRTuning(unsigned short cmdType, bool bWB, char *pParam);

#ifdef MTK_AUDIO_GAIN_TABLE
		ACHStatus getGainInfoForDisp(char *pParam, int *len, int block);
		void printGainInfo(PCDispTotolStru *dispStru, int block);
		void printCustGainTable(AUDIO_GAIN_TABLE_STRUCT *gainTable, int block);
		ACHStatus ULCustGainParamFromNV(char *pParam, int *len, int block);
		ACHStatus DLCustGainParamToNV(char *pParam, int block);
		ACHStatus getBtNrecInfoForDisp(char *pParam, int *len);
		ACHStatus DLCustBtNrecParamToNV(char *pParam);
		ACHStatus ULCustBtNrecParamFromNV(char *pParam, int *len);
#endif
        ACHStatus ULCustHDRecParamFromNV(char *pParam, int *len, int block);
        ACHStatus DLCustHDRecParamToNV(char *pParam, int block);

        ACHStatus ULCustHDRecSceTableFromNV(char *pParam, int *len);
        ACHStatus DLCustHDRecSceTableToNV(char *pParam);

        ACHStatus ULCustHDRecHSParamFromNV(char *pParam, int *len);
        ACHStatus DLCustHDRecHSParamToNV(char *pParam);
        ACHStatus HDRecording(int enable, char *pParam);

        ACHStatus getPhoneSupportInfo(unsigned int *supportInfo);
        void enableHALHDRecTunning(bool enbale);

        ACHStatus ULCustVOIPParamFromNV(char *pParam, int *len);
        ACHStatus DLCustVOIPParamToNV(char *pParam);
        //--->

        void enableAurisysMtkLibDump(bool enable);
        bool generateVmDumpByEpl(const char* eplPath);

    private:
        int m_RecordMaxDur;
        int m_RecordChns;
        int m_RecordSampleRate;
        int m_fd;
        bool m_bHDRecTunning;
        bool m_bArsiDumpEnabled;
        bool GainInfoFlag;

        status_t SetAudioData(int par1,size_t len, void *ptr);
        status_t GetAudioData(const char* keystr,size_t ptrlen,void *ptr);
        status_t setParameters(const String8 &keyValuePaires);
        String8 getParameters(const String8 &keys);
};

#endif  //_AUDIO_COMP_FLT_CUST_PARAM_H_




