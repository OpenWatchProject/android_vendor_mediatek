#ifndef _ENH_API_H
#define _ENH_API_H

#define MEMORY_MAX_PAGE 14

#define NB_VOIP              0x1
#define WB_VOIP              0x1 << 1
#define MONO_RECORD          0x1 << 2
#define STEREO_RECORD        0x1 << 3
#define SPEECH_RECOGNITION   0x1 << 4
#define MONO_AEC_RECORD      0x1 << 5
#define STEREO_AEC_RECORD    0x1 << 6
#define LOW_LATENCY_RECORD   0x1 << 7
#define DMNR_CALIBRATION     0x1 << 8
#define INCALL_RECORD        0x1 << 9
#define TWO_MIC_ASR          0x1 << 10
#define PHONE_CALL           WB_VOIP

#define LIB_VERSION_NUM "MTK_SP1 Soft. Version 1.1.17, 2019-03-04"
#define LIB_VERSION LIB_VERSION_NUM

typedef signed short Word16;
typedef unsigned short uWord16;
typedef unsigned short UWord16;
typedef int Word32;
typedef unsigned int uWord32;
typedef unsigned int UWord32;
typedef short vWord16;

#define ENH_PAR_COUNT 48
#define COM_PAR_COUNT 12
#define DMNR_CAL_DATA_NB_SIZE 120
#define DMNR_CAL_DATA_WB_SIZE 120
#define DMNR_CAL_DATA_SWB_SIZE 120
#define COMPEN_FIR_LENGTH 180
#define DC_REMOVAL_IIR_LENGTH 20
#define ENH_IIR_LENGTH 42
#define ENH_IIR_LENGTH_MIC2 32
#define COMMON_LOG_START_ADDRESS 3840
#define COMMON_LOG_START_ADDRESS_2MIC  4800
#define EPL_PCM_BANDCTRL_ADDRESS 4400
#define ENH_API_AUDIO_FORMAT_PCM_16_BIT 0
#define ENH_API_AUDIO_FORMAT_PCM_8_24_BIT 1
#define TRIGGER_DUMP_SIZE_24BIT 8640
#define TRIGGER_DUMP_SIZE_16BIT 4800
/*
#ifdef USIP_INTERFACE
    #define COMMON_LOG_START_ADDRESS 3840
    #define EPL_PCM_BANDCTRL_ADDRESS 4400
#else
    #define COMMON_LOG_START_ADDRESS 4480
    #define EPL_PCM_BANDCTRL_ADDRESS (COMMON_LOG_START_ADDRESS + 560)
#endif*/
#define EPL_PCM_BANDCTRL_ADDRESS_RECORD_24BIT 8240
#define EPL_PCM_BANDCTRL_ADDRESS_2MIC 5360

#define EPL_PCM_UL0_ADDRESS 0
#define EPL_PCM_UL00_ADDRESS 320
#define EPL_PCM_UL1_ADDRESS 1280
#define EPL_PCM_UL2_ADDRESS 1600
#define EPL_PCM_DL0_ADDRESS 1920
#define EPL_PCM_DL1_ADDRESS 2240
#define EPL_PCM_DLREF_ADDRESS 2880

#define EPL_ENH_INFO_START_ADDRESS (EPL_PCM_BANDCTRL_ADDRESS + 2 )
#define EPL_ENH_INFO_LENGTH 160



typedef struct
{

    // Parameter Settings
    Word16 enhance_pars_NB[ENH_PAR_COUNT]; // mode parameter
    Word16 enhance_pars_WB[ENH_PAR_COUNT]; // mode parameter
    Word16 enhance_pars_SWB[ENH_PAR_COUNT]; // mode parameter, shared with 48K

    Word16 common_pars[COM_PAR_COUNT];  // common paramter
    Word32 DP2_AGC_GAIN; // MIC_DG
    Word32 MMI_ctrl; // bit 0: DMNR, bit 1: MagiConference, bit 2: TDNC
    Word32 sample_rate_codec; // enhancement sample rate for UL



    // DMNR Calibration Data: 2 sets
    Word16 DMNR_cal_data_NB_set1[DMNR_CAL_DATA_NB_SIZE];
    Word16 DMNR_cal_data_NB_set2[DMNR_CAL_DATA_NB_SIZE]; //LSPK
    Word16 DMNR_cal_data_WB_set1[DMNR_CAL_DATA_WB_SIZE];
    Word16 DMNR_cal_data_WB_set2[DMNR_CAL_DATA_WB_SIZE]; //LSPK
    Word16 DMNR_cal_data_SWB_set1[DMNR_CAL_DATA_SWB_SIZE];
    Word16 DMNR_cal_data_SWB_set2[DMNR_CAL_DATA_SWB_SIZE]; //LSPK


    // FIR Coefficients
    Word16 Compen_filter_NB[COMPEN_FIR_LENGTH];
    Word16 Compen_filter_WB[COMPEN_FIR_LENGTH];
    Word16 Compen_filter_SWB[COMPEN_FIR_LENGTH]; // shared with 48K

    // DC Removal IIR Coefs
    Word16 MIC1_IIR_COEF_UL_NB[DC_REMOVAL_IIR_LENGTH];
    Word16 MIC2_IIR_COEF_UL_NB[DC_REMOVAL_IIR_LENGTH];

    Word16 MIC1_IIR_COEF_UL_WB[DC_REMOVAL_IIR_LENGTH];
    Word16 MIC2_IIR_COEF_UL_WB[DC_REMOVAL_IIR_LENGTH];

    Word16 MIC1_IIR_COEF_UL_SWB[DC_REMOVAL_IIR_LENGTH];  // shared with 48K
    Word16 MIC2_IIR_COEF_UL_SWB[DC_REMOVAL_IIR_LENGTH];  // shared with 48K

    // ENH IIR Coefs
    Word16 ENH_IIR_COEF_UL_NB[ENH_IIR_LENGTH];
    Word16 ENH_IIR_COEF_DL_NB[ENH_IIR_LENGTH];
    Word16 ENH_IIR_COEF_UL_WB[ENH_IIR_LENGTH];
    Word16 ENH_IIR_COEF_DL_WB[ENH_IIR_LENGTH];
    Word16 ENH_IIR_COEF_UL_SWB[ENH_IIR_LENGTH];  // shared with 48K
    Word16 ENH_IIR_COEF_DL_SWB[ENH_IIR_LENGTH];  // shared with 48K
    Word16 ENH_IIR_COEF_UL_MicSelection[ENH_IIR_LENGTH_MIC2];

    // Miscellaneous
    Word32 App_table; // Fixed PHONE_CALL for MODEM
    Word32 frame_rate; // Fixed 20
    Word32 Fea_Cfg_table; // Fixed 511

    // DL Max time
    Word32 SPH_DEL_M_DL;  // 8k sample
    Word32 SPH_ENH_INTERNAL_PAR_ADDR;
    // Smart PA delay(Moved to Ref_delay)
    // bit 0-7 = delay main mic sample value
    // bit   8 = smart PA switch(Removed)
    // bit  14 = smart PA switch(Gen93_AP)

    Word32 UL_delay; // Max:64ms
    // BT mode flag
    Word16 Ref_delay; // Max:256ms

    // TDNC Control
    Word32 se_cntr;  // Enhancement Counter
    Word32 t2_cur;   // TDD Counter
    Word32 TxChannelType; // HR/FR
    Word32 SubChannelID; // HR + TDD
    Word32 DTX_flag;  // TDD
    Word16 shift_SACCh;
    Word32 t2_SACCh;  // idle frame

    /* ================ INTERNAL USE ==================== */

    Word16 PCM_buffer[1920];
    Word32 PCM_buffer_32[1920];
    Word16 *EPL_buffer;
    Word32 Device_mode; // AP: 0: Normal, 1:Earphone, 2:LSPK, 3: BT
    Word32 MMI_MIC_GAIN;
    Word32 *SCH_mem;
    Word16 SQoS_state;
    Word16 audio_format; // 	0:16bit, 1:24bit

    //Two mic ASR
    Word16 UL_sep_switch;
    Word32 Working_buf_memory_size;
} SPH_ENH_ctrl_struct;

#pragma pack(push,1)
typedef struct
{
    uWord16 LP; //16bits
    char HP; //8bits

} Word24;
#pragma pack(pop)

Word32  ENH_API_Get_Memory(SPH_ENH_ctrl_struct *SPH_ENH_ctrl_struct);
Word16  ENH_API_Get_Version(SPH_ENH_ctrl_struct *SPH_ENH_ctrl_struct);


Word16  ENH_API_Alloc(SPH_ENH_ctrl_struct *SPH_ENH_ctrl_struct, Word32 *mem_ptr);
Word16  ENH_API_Init(void *p_handler);
void ENH_API_Update_Parameter(SPH_ENH_ctrl_struct *p_SPH_ENH_ctrl_struct);
void ENH_API_Reset(void *p_handler);
Word16  ENH_API_Free(void *p_handler);
void ENH_API_Process(SPH_ENH_ctrl_struct *SPH_ENH_ctrl_struct);
void ENH_API_DL_Process(SPH_ENH_ctrl_struct *Sph_Enh_ctrl);
void ENH_API_Set_dl_enhance(const char b_enhance_on, void *p_handler);
void ENH_API_Set_ul_enhance(const char b_enhance_on, void *p_handler);
unsigned char ENH_API_Get_dl_enh_value(void *p_handler);
unsigned char ENH_API_Get_ul_enh_value(void *p_handler);
void ENH_API_Set_dl_mute(const char b_mute_on, void *p_handler);
void ENH_API_Set_ul_mute(const char b_mute_on, void *p_handler);
void *ENH_API_Get_ENH_ctrl_ptr(void *p_handler);
Word32 ENH_API_Get_UL_enh_start_flag(SPH_ENH_ctrl_struct *Sph_Enh_ctrl);
Word32 ENH_API_Get_DL_enh_start_flag(SPH_ENH_ctrl_struct *Sph_Enh_ctrl);
void ENH_API_update_AGC_GAIN(void *p_handler, SPH_ENH_ctrl_struct *enh_param);
void SET_SCH_mem_print_log(void *pp_handler, void(*debug_log_fp_t)(const char *message, ...));
Word16 ENH_API_GetSWGain(void *p_handler, Word16 mic2);
void Reverse_PlaybackGain(Word32 *input, Word16 *output, Word16 GainQ12, Word16 Framesize);

void ENH_API_IIR_SRC_32k_to_16k_UL_in1(void *p_handler, Word16 *input, Word16 *output);
void ENH_API_IIR_SRC_32k_to_16k_UL_in2(void *p_handler, Word16 *input, Word16 *output);
void ENH_API_IIR_SRC_32k_to_16k_DL_ref(void *p_handler, Word16 *input, Word16 *output);
void ENH_API_IIR_SRC_32k_to_16k_DL_in(void *p_handler, Word16 *input, Word16 *output);
void ENH_API_IIR_SRC_16k_to_32k_UL(Word16 UDL, Word16 DL_4K_Removing_Switch, void *p_handler, Word16 *input, Word16 *output);
void ENH_API_IIR_SRC_16k_to_32k_DL(Word16 UDL, void *p_handler, Word16 *input, Word16 *output);
Word16 ENH_API_Get_EPL_data_size(void *p_handler);
Word16 ENH_API_Get_frame_length(void *p_handler);
#endif
