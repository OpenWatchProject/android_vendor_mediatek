#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include <log/log.h>
#include "arsi_api.h"
#include "wrapped_audio.h"

#include "enh_api.h"
#include "AudioParamParser.h"
#include "audio_task.h"


// Audio Type Declaration
#define VOIP_AUDIO_TYPE             "VoIPv2"
#define VOIPDMNR_AUDIO_TYPE         "VoIPv2DMNR"
#define VOIPGENERAL_AUDIO_TYPE      "VoIPv2General"
#define RECORD_AUDIO_TYPE           "Record"
#define RECORDFIR_AUDIO_TYPE        "RecordFIR"
#define RECORDDMNR_AUDIO_TYPE       "RecordDMNR"
// VoIPv2 AudioType Categories Path Decleration
//#define VOIP_HANDSET_DMNR_PATH      "Profile,Handset 2mic NR"
//#define VOIP_HANDSET_NO_DMNR_PATH   "Profile,Handset no 2mic NR"
#define VOIP_HANDSET_PATH           "Profile,Normal"
#define VOIP_HAC_PATH               "Profile,HAC"

//#define VOIP_3POLE_HEADSET_PATH     "Profile,3-pole headset"
#define VOIP_3POLE_HEADSET_PATH     "Profile,3_pole_Headset"
#define VOIP_4POLE_HEADSET_PATH     "Profile,4_pole_Headset"
#define VOIP_5POLE_HEADSET_PATH     "Profile,5_pole_Headset"
#define VOIP_5POLE_HEADSET_ANC_PATH "Profile,5_pole_Headset+ANC"

//#define VOIP_BT_PATH                "Profile,BT earphone"
#define VOIP_BT_NREC_PATH           "Profile,BT_Earphone"
#define VOIP_BT_NREC_OFF_PATH       "Profile,BT_NREC_Off"

//#define VOIP_HANDSFREE_NR_PATH      "Profile,Hands-free 1mic NR"
//#define VOIP_HANDSFREE_NO_NR_PATH   "Profile,Hands-free no 1mic NR"
#define VOIP_HANDSFREE_1MIC_NR_PATH "Profile,Handsfree"
#define VOIP_HANDSFREE_2MIC_NR_PATH "Profile,MagiConference"

#define VOIP_USB_PATH               "Profile,Usb_Headset"

// VoIPv2General AudioType Categories Path Decleration
#define VOIPGENERAL_COMMON_PATH     "CategoryLayer,Common"

// VoIPv2DMNR AudioType Categories Path Decleration
#define VOIP_NO_DMNR_PATH           ""
// #define VOIP_HANDSETDMNR_PATH       "Profile,Handset"
// #define VOIP_MAGICONFDMNR_PATH      "Profile,MagiConference"

// VoIPv2 Parameter Decleration
#define VOIP_PARAM                  "speech_mode_para"
#define VOIP_IN_FIR_PARAM           "sph_in_fir"
#define VOIP_OUT_FIR_PARAM          "sph_out_fir"

#define VOIP_IN_IIR_PARAM           "sph_in_iir_enh_dsp"
#define VOIP_OUT_IIR_PARAM          "sph_out_iir_enh_dsp"

#define VOIP_MIC1_DC_IIR_PARAM      "sph_in_iir_mic1_dsp"
#define VOIP_MIC2_DC_IIR_PARAM      "sph_in_iir_mic2_dsp"

// VoIPv2DMNR Parameter Decleration
#define VOIP_DMNR_PARAM_NB1             "dmnr_para_nb1"
#define VOIP_DMNR_PARAM_NB2             "dmnr_para_nb2"
#define VOIP_DMNR_PARAM_WB1             "dmnr_para"
#define VOIP_DMNR_PARAM_WB2             "dmnr_para_wb2"
#define VOIP_DMNR_PARAM_SWB1            "dmnr_para_swb1"
#define VOIP_DMNR_PARAM_SWB2            "dmnr_para_swb2"

// VoIPv2General Category & Parameter Decleration
#define VOIPGENERAL_PARAM_NAME      "speech_common_para"

// Record Categories Decleration
#define RECORD_HANDSET_PATH           "Profile,Handset"
#define RECORD_4POLE_HEADSET_PATH     "Profile,4-pole HS"
#define RECORD_5POLE_HEADSET_PATH     "Profile,5-pole HS"
#define RECORD_5POLE_HEADSET_ANC_PATH "Profile,5-pole HS+ANC"
#define RECORD_BT_PATH                "Profile,BT earphone"
#define RECORD_VR_PATH                "Application,VR"
#define RECORD_VOICE_PERFORMANCE_PATH "Application,VoicePerformance"
#define RECORD_INCALL_PATH            "Application,IncallRecord"
#define RECORD_VOICE_UNLOCK_PATH      "Application,VoiceUnLk"
#define RECORD_ASR_PATH               "Application,ASR"
#define RECORD_SND_REC_NORMAL_PATH    "Application,SndRecNormal"
#define RECORD_SND_REC_LECTURE_PATH   "Application,SndRecLecture"
#define RECORD_SND_REC_MEETING_PATH   "Application,SndRecMeeting"
#define RECORD_CAM_REC_NORMAL_PATH    "Application,CamRecNormal"
#define RECORD_CAM_REC_MEETING_PATH   "Application,CamRecMeeting"
#define RECORD_CUSTOMIZATION2_PATH    "Application,Customization2"
#define RECORD_FAST_RECORD_PATH       "Application,FastRecord"
#define RECORD_UNPROCESSED_PATH       "Application,Unprocessed"
#define DMNR_Cal_ATH                  "Application,DMNRCal"
#define RECORD_NO_DMNR_PATH           ""
#define RECORD_SND_REC_USB_PATH       "Profile,USB"
// Record param
#define RECORD_PARAM                  "record_mode_para"
#define RECORD_IN_FIR1_PARAM          "sph_in_fir1"
#define RECORD_IN_FIR2_PARAM          "sph_in_fir2"
#define RECORD_DMNR_PARAM             "dmnr_para"

#define RECORD_IN_IIR1_PARAM          "sph_in_iir_mic1_enh_dsp"
#define RECORD_IN_IIR2_PARAM          "sph_in_iir_mic2_enh_dsp"
#define RECORD_MIC1_DC_IIR_PARAM      "sph_in_iir_mic1_dsp"
#define RECORD_MIC2_DC_IIR_PARAM      "sph_in_iir_mic2_dsp"

// Feature options
#define VOIP_NORMAL_DMNR_SUPPORT_FO     "VIR_VOIP_NORMAL_DMNR_SUPPORT"
#define VOIP_HANDSFREE_DMNR_SUPPORT_FO  "VIR_VOIP_HANDSFREE_DMNR_SUPPORT"

#define CATE_PATH_LEN 100
#define DMNR_NB_REAL_SIZE 44
#define DMNR_WB_REAL_SIZE 76
#define DMNR_SWB_REAL_SIZE 120
#define SCENE_NAME_MAX_LEN 64
#define KEY_SET_AUDIO_CUSTOM_SCENE "SetAudioCustomScene="

const signed short Sph_IIR_default[ENH_IIR_LENGTH] =
{
    0x7A90,0xC524,0x8001,0x4000,0x7F54,
    0xC090,0x800A,0x4000,0x0,0x0,
    0x0,0x0,0x0,0x0,0x0,
    0x0,0x0,0x0,0x0,0x0,
    0x0,0x0,0x0,0x0,0x0,
    0x0,0x0,0x0,0x0,0x0,
    0x0,0x0,0x0,0x0,0x0,
    0x0,0x0,0x0,0x0,0x0,
    0x3BB,0x2
};

const signed short Sph_Mic_IIR_In_default[DC_REMOVAL_IIR_LENGTH] =
{
    -7358, 15521, 7644, -15288, 7644,
    -7960, 16132, 8192, -16383, 8192,
    0, 0, 0, 0, 8192,
    0, 0, 0, 0, 8192
};

const signed short Sph_DMNR_NB_default[DMNR_NB_REAL_SIZE] =
{
    0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
    0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
    0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
    0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
    0x44,0x0,0x0,0x0
};

const signed short Sph_DMNR_WB_default[DMNR_WB_REAL_SIZE] =
{
    0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
    0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
    0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
    0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
    0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
    0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
    0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
    0x0,0x0,0x44,0x0,0x0,0x0
};

const signed short Sph_DMNR_SWB_default[DMNR_SWB_REAL_SIZE] =
{
    0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
    0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
    0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
    0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
    0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,

    0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
    0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
    0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
    0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
    0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,

    0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
    0x0,0x0,0x0,0x0,0x0,0x0,0x44,0x0,0x0,0x0
};

void get_voip_category_path(char *scene, char *categoryPath, SPH_ENH_ctrl_struct *parms, const arsi_task_config_t *p_arsi_task_config,
                            const arsi_lib_config_t *p_arsi_lib_config __unused, const debug_log_fp_t debug_log_fp, AppOps *pAppOps __unused)
{
    // device
    parms->App_table = WB_VOIP;
    debug_log_fp("%s(),  VoIP (scene = %s)\n", __FUNCTION__, scene);

    // Reset categoryPath
    categoryPath[0] = '\0';

    // Append scene
    if (scene)
    {
        sprintf(categoryPath, "Scene,%s,", scene);
    }

    if (p_arsi_task_config->input_device_info.devices == AUDIO_DEVICE_IN_BUILTIN_MIC)
    {
        debug_log_fp("AUDIO_DEVICE_IN_BUILTIN_MIC\n");
        if (p_arsi_task_config->output_device_info.devices == AUDIO_DEVICE_OUT_EARPIECE)
        {
            // Handset
            parms->Device_mode = 0;
            //              if (appHandleIsFeatureOptionEnabled(pAppHandle, "VOIP_NORMAL_DMNR_SUPPORT_FO"))
            if ((p_arsi_task_config->enhancement_feature_mask & ENHANCEMENT_FEATURE_EARPIECE_HAC) == 0)
            {
                strncat(categoryPath, VOIP_HANDSET_PATH, CATE_PATH_LEN-1);
                debug_log_fp("VOIP_HANDSET_PATH\n");
            }
            else
            {
                strncat(categoryPath, VOIP_HAC_PATH, CATE_PATH_LEN-1);
                debug_log_fp("Unknown Device:BUILTIN_MIC ELSE, VOIP_HAC_PATH\n");
            }
        }
        else if (p_arsi_task_config->output_device_info.devices == AUDIO_DEVICE_OUT_WIRED_HEADPHONE)
        {
            // 3 pole headset
            parms->Device_mode = 1;
            strncat(categoryPath, VOIP_3POLE_HEADSET_PATH, CATE_PATH_LEN-1);
            debug_log_fp("AUDIO_DEVICE_OUT_WIRED_HEADPHONE\n");
        }
        else if (p_arsi_task_config->output_device_info.devices == AUDIO_DEVICE_OUT_SPEAKER)
        {
            // Hands-free
            parms->Device_mode = 2;
            debug_log_fp("AUDIO_DEVICE_OUT_SPEAKER\n");
            debug_log_fp("input_device_info.num_channels = %d\n", p_arsi_task_config->input_device_info.num_channels);

#if defined(MTK_VOIP_HANDSFREE_DMNR)
            // Magic Conference cases
            strncat(categoryPath, VOIP_HANDSFREE_2MIC_NR_PATH, CATE_PATH_LEN-1);
            debug_log_fp("VOIP_HANDSFREE_2MIC_NR_PATH: %d\n", p_arsi_task_config->max_input_device_num_channels);
#else
            strncat(categoryPath, VOIP_HANDSFREE_1MIC_NR_PATH, CATE_PATH_LEN-1);
            debug_log_fp("VOIP_HANDSFREE_1MIC_NR_PATH: %d\n", p_arsi_task_config->max_input_device_num_channels);
#endif

        }
        else
        {
            parms->Device_mode = 0;
            strncat(categoryPath, VOIP_HANDSET_PATH, CATE_PATH_LEN-1);
            debug_log_fp("Unknown Device:BUILTIN_MIC, HANDSET_PATH ELSE\n");
        }
    }
    else if (p_arsi_task_config->input_device_info.devices == AUDIO_DEVICE_IN_BACK_MIC)
    {
        debug_log_fp("AUDIO_DEVICE_IN_BACK_MIC\n");
        switch (p_arsi_task_config->output_device_info.devices)
        {
        case AUDIO_DEVICE_OUT_SPEAKER:
            parms->Device_mode = 2;
            debug_log_fp("AUDIO_DEVICE_OUT_SPEAKER\n");
            debug_log_fp("input_device_info.num_channels = %d\n", p_arsi_task_config->input_device_info.num_channels);

#if defined(MTK_VOIP_HANDSFREE_DMNR)
            // Magic Conference cases
            strncat(categoryPath, VOIP_HANDSFREE_2MIC_NR_PATH, CATE_PATH_LEN-1);
            debug_log_fp("VOIP_HANDSFREE_2MIC_NR_PATH: %d\n", p_arsi_task_config->max_input_device_num_channels);
#else
            strncat(categoryPath, VOIP_HANDSFREE_1MIC_NR_PATH, CATE_PATH_LEN-1);
            debug_log_fp("VOIP_HANDSFREE_1MIC_NR_PATH: %d\n", p_arsi_task_config->max_input_device_num_channels);
#endif
            break;
        default:
            parms->Device_mode = 0;
            strncat(categoryPath, VOIP_HANDSET_PATH, CATE_PATH_LEN-1);
            debug_log_fp("Unknown Device:ELSE\n");
            break;
        }
    }
    else if (p_arsi_task_config->input_device_info.devices == AUDIO_DEVICE_IN_WIRED_HEADSET)
    {
        // Headset
        parms->Device_mode = 1;
        if (p_arsi_task_config->output_device_info.devices == AUDIO_DEVICE_OUT_WIRED_HEADSET)
        {
            debug_log_fp("AUDIO_DEVICE_OUT_WIRED_HEADSET\n");
            debug_log_fp("input_device_info.num_channels = %d\n", p_arsi_task_config->input_device_info.num_channels);
            switch (p_arsi_task_config->input_device_info.num_channels)
            {
            case 1: // 4 Pole, input_device_info.num_channels = 1
                strncat(categoryPath, VOIP_4POLE_HEADSET_PATH, CATE_PATH_LEN-1);
                break;
            case 2: // 5 Pole,
                if (p_arsi_task_config->output_device_info.hw_info_mask == INPUT_DEVICE_HW_INFO_HEADSET_POLE_5_ANC)
                {
                    strncat(categoryPath, VOIP_5POLE_HEADSET_ANC_PATH, CATE_PATH_LEN-1);
                    debug_log_fp("INPUT_DEVICE_HW_INFO_HEADSET_POLE_5_ANC\n");
                }
                else
                {
                    strncat(categoryPath, VOIP_5POLE_HEADSET_PATH, CATE_PATH_LEN-1);
                    debug_log_fp("VOIP_5POLE_HEADSET_PATH\n");
                }
                break;
            default:
                strncat(categoryPath, VOIP_HANDSET_PATH, CATE_PATH_LEN-1);
                debug_log_fp("Unknown_POLE_HEADSET_PATH\n");
                break;
            }
        }
        else
        {
            strncat(categoryPath, VOIP_4POLE_HEADSET_PATH, CATE_PATH_LEN-1);
            debug_log_fp("Unknown Device:VOIP_4POLE_HEADSET_PATH ELSE\n");
        }
    }
    else if (p_arsi_task_config->input_device_info.devices == AUDIO_DEVICE_IN_USB_DEVICE)
    {
        parms->Device_mode = 1;
        strncat(categoryPath, VOIP_USB_PATH, CATE_PATH_LEN-1);
        debug_log_fp("AUDIO_DEVICE_IN_USB_DEVICE\n");
    }
    else if (p_arsi_task_config->input_device_info.devices == AUDIO_DEVICE_IN_BLUETOOTH_SCO_HEADSET)
    {
        // BT
        parms->Device_mode = 3;
        if (p_arsi_task_config->enhancement_feature_mask & ENHANCEMENT_FEATURE_BT_NREC)
        {
            strncat(categoryPath, VOIP_BT_NREC_PATH, CATE_PATH_LEN-1);
            debug_log_fp("VOIP_BT_NREC_PATH\n");
        }
        else
        {
            strncat(categoryPath, VOIP_BT_NREC_OFF_PATH, CATE_PATH_LEN-1);
            debug_log_fp("VOIP_BT_NREC_OFF_PATH\n");
        }
    }
    else // Unknown Input device
    {
        debug_log_fp("Unknown input Device:ELSE\n");
        if (p_arsi_task_config->output_device_info.devices == AUDIO_DEVICE_OUT_EARPIECE)
        {
            // Handset
            parms->Device_mode = 0;
            //              if (appHandleIsFeatureOptionEnabled(pAppHandle, "VOIP_NORMAL_DMNR_SUPPORT_FO"))
            if ((p_arsi_task_config->enhancement_feature_mask & ENHANCEMENT_FEATURE_EARPIECE_HAC) == 0)
            {
                strncat(categoryPath, VOIP_HANDSET_PATH, CATE_PATH_LEN-1);
                debug_log_fp("VOIP_HANDSET_PATH\n");
            }
            else
            {
                strncat(categoryPath, VOIP_HAC_PATH, CATE_PATH_LEN-1);
                debug_log_fp("Unknown Device:BUILTIN_MIC ELSE, VOIP_HAC_PATH\n");
            }
        }
        else if (p_arsi_task_config->output_device_info.devices == AUDIO_DEVICE_OUT_SPEAKER)
        {
            // Hands-free
            parms->Device_mode = 2;
            debug_log_fp("AUDIO_DEVICE_OUT_SPEAKER\n");
            debug_log_fp("input_device_info.num_channels = %d\n", p_arsi_task_config->input_device_info.num_channels);
            //            if (appHandleIsFeatureOptionEnabled(pAppHandle, "VOIP_HANDSFREE_DMNR_SUPPORT_FO"))

#if defined(MTK_VOIP_HANDSFREE_DMNR)
            // Magic Conference cases
            strncat(categoryPath, VOIP_HANDSFREE_2MIC_NR_PATH, CATE_PATH_LEN-1);
            debug_log_fp("VOIP_HANDSFREE_2MIC_NR_PATH: %d\n", p_arsi_task_config->max_input_device_num_channels);
#else
            strncat(categoryPath, VOIP_HANDSFREE_1MIC_NR_PATH, CATE_PATH_LEN-1);
            debug_log_fp("VOIP_HANDSFREE_1MIC_NR_PATH: %d\n", p_arsi_task_config->max_input_device_num_channels);
#endif

        }
        else if (p_arsi_task_config->output_device_info.devices == AUDIO_DEVICE_OUT_WIRED_HEADSET)
        {
            debug_log_fp("AUDIO_DEVICE_OUT_WIRED_HEADSET\n");
            debug_log_fp("input_device_info.num_channels = %d\n", p_arsi_task_config->input_device_info.num_channels);
            switch (p_arsi_task_config->input_device_info.num_channels)
            {
            case 1: // 4 Pole, input_device_info.num_channels = 1
                strncat(categoryPath, VOIP_4POLE_HEADSET_PATH, CATE_PATH_LEN-1);
                break;
            case 2: // 5 Pole,
                if (p_arsi_task_config->output_device_info.hw_info_mask == INPUT_DEVICE_HW_INFO_HEADSET_POLE_5_ANC)
                {
                    strncat(categoryPath, VOIP_5POLE_HEADSET_ANC_PATH, CATE_PATH_LEN-1);
                    debug_log_fp("INPUT_DEVICE_HW_INFO_HEADSET_POLE_5_ANC\n");
                }
                else
                {
                    strncat(categoryPath, VOIP_5POLE_HEADSET_PATH, CATE_PATH_LEN-1);
                    debug_log_fp("VOIP_5POLE_HEADSET_PATH\n");
                }
                break;
            default:
                strncat(categoryPath, VOIP_HANDSET_PATH, CATE_PATH_LEN-1);
                debug_log_fp("Unknown_POLE_HEADSET_PATH\n");
                break;
            }
        }
        else if (p_arsi_task_config->output_device_info.devices == AUDIO_DEVICE_OUT_WIRED_HEADPHONE)
        {
            // 3 pole headset
            parms->Device_mode = 1;
            strncat(categoryPath, VOIP_3POLE_HEADSET_PATH, CATE_PATH_LEN-1);
            debug_log_fp("AUDIO_DEVICE_OUT_WIRED_HEADPHONE\n");
        }
        else
        {
            switch (p_arsi_task_config->output_device_info.devices)
            {
            case AUDIO_DEVICE_OUT_BLUETOOTH_SCO:
            case AUDIO_DEVICE_OUT_BLUETOOTH_SCO_HEADSET:
            case AUDIO_DEVICE_OUT_BLUETOOTH_SCO_CARKIT:
            case AUDIO_DEVICE_OUT_BLUETOOTH_A2DP:
            case AUDIO_DEVICE_OUT_BLUETOOTH_A2DP_HEADPHONES:
            case AUDIO_DEVICE_OUT_BLUETOOTH_A2DP_SPEAKER:
                // BT
                parms->Device_mode = 3;
                if (p_arsi_task_config->enhancement_feature_mask & ENHANCEMENT_FEATURE_BT_NREC)
                {
                    strncat(categoryPath, VOIP_BT_NREC_PATH, CATE_PATH_LEN-1);
                    debug_log_fp("VOIP_BT_NREC_PATH\n");
                }
                else
                {
                    strncat(categoryPath, VOIP_BT_NREC_OFF_PATH, CATE_PATH_LEN-1);
                    debug_log_fp("VOIP_BT_NREC_OFF_PATH\n");
                }
                break;
            default:
                parms->Device_mode = 0;
                strncat(categoryPath, VOIP_HANDSET_PATH, CATE_PATH_LEN-1);
                debug_log_fp("Unknown output Device:ELSE\n");
                break;
            }
        }
    }
}


void get_record_category_path(char *scene, char *categoryPath, SPH_ENH_ctrl_struct *parms, const arsi_task_config_t *p_arsi_task_config,
                              const arsi_lib_config_t  *p_arsi_lib_config, const debug_log_fp_t debug_log_fp, AppOps *pAppOps)
{
    debug_log_fp("%s(), Record (scene = %s)", __FUNCTION__, scene);

    // Reset categoryPath
    categoryPath[0] = '\0';

    // Append scene
    if (scene)
    {
        sprintf(categoryPath, "Scene,%s,", scene);
    }

    if (p_arsi_lib_config->frame_size_ms < 20)
    {
        if (p_arsi_task_config->input_source == AUDIO_SOURCE_VOICE_PERFORMANCE)  // VOICE_PERFORMANCE
        {
            strncat(categoryPath, RECORD_VOICE_PERFORMANCE_PATH",", CATE_PATH_LEN-1);
            parms->App_table = LOW_LATENCY_RECORD;
            debug_log_fp("AUDIO_SOURCE_VOICE_PERFORMANCE \n");
        }
        else
        {
            // fast Record
            strncat(categoryPath, RECORD_FAST_RECORD_PATH",", CATE_PATH_LEN-1);
            parms->App_table = LOW_LATENCY_RECORD;
            debug_log_fp("LOW_LATENCY_RECORD \n");
        }
    }
    else
    {
        debug_log_fp("TASK_SCENE_RECORD.num_channels = %d\n", p_arsi_task_config->input_device_info.num_channels);
        if (p_arsi_task_config->input_device_info.num_channels == 1)
        {
            parms->App_table = MONO_RECORD;
        }
        else
        {
            parms->App_table = STEREO_RECORD;
        }
        if (p_arsi_task_config->input_source == AUDIO_SOURCE_MIC)
        {
            /*          switch (p_arsi_task_config->reserve2)
            {
            case 0: // Sound Record, Normal
            strcpy(categoryPath, RECORD_SND_REC_NORMAL_PATH",");
            break;
            case 1: // Sound Record, Lecture
            strcpy(categoryPath, RECORD_SND_REC_LECTURE_PATH",");
            break;
            case 2: // Sound Record, Meeting
            strcpy(categoryPath, RECORD_SND_REC_MEETING_PATH",");
            break;
            case 3: // Cam Record, Normal
            strcpy(categoryPath, RECORD_CAM_REC_NORMAL_PATH",");
            break;
            case 4: // Cam Record, Meeting
            strcpy(categoryPath, RECORD_CAM_REC_MEETING_PATH",");
            break;
            //case 5: // Fast Record
            //strcpy(categoryPath, RECORD_FAST_RECORD_PATH",");
            //break;
            default:
            strcpy(categoryPath, RECORD_SND_REC_NORMAL_PATH",");
            break;
            }*/
            strncat(categoryPath, RECORD_SND_REC_NORMAL_PATH",", CATE_PATH_LEN-1);
            debug_log_fp("RECORD_SND_REC_NORMAL_PATH \n");
        }
        else if (p_arsi_task_config->input_source == AUDIO_SOURCE_VOICE_CALL)
        {
            strncat(categoryPath, RECORD_INCALL_PATH",", CATE_PATH_LEN-1);
            debug_log_fp("AUDIO_SOURCE_VOICE_CALL\n");
            parms->App_table = INCALL_RECORD;
            parms->Device_mode = 0;
        }
        else if (p_arsi_task_config->input_source == AUDIO_SOURCE_VOICE_UPLINK)
        {
            strncat(categoryPath, RECORD_INCALL_PATH",", CATE_PATH_LEN-1);
            debug_log_fp("AUDIO_SOURCE_VOICE_UPLINK\n");
            parms->App_table = INCALL_RECORD;
            parms->Device_mode = 2;
        }
        else if (p_arsi_task_config->input_source == AUDIO_SOURCE_VOICE_DOWNLINK)
        {
            strncat(categoryPath, RECORD_INCALL_PATH",", CATE_PATH_LEN-1);
            debug_log_fp("AUDIO_SOURCE_VOICE_DOWNLINK\n");
            parms->App_table = INCALL_RECORD;
            parms->Device_mode = 1;
        }
        else if (p_arsi_task_config->input_source == AUDIO_SOURCE_CAMCORDER)
        {
            strncat(categoryPath, RECORD_CAM_REC_NORMAL_PATH",", CATE_PATH_LEN-1);
            debug_log_fp("AUDIO_SOURCE_CAMCORDER \n");
        }
        else if (p_arsi_task_config->input_source == AUDIO_SOURCE_CUSTOMIZATION1)  // ASR
        {
            strncat(categoryPath, RECORD_ASR_PATH",", CATE_PATH_LEN-1);
            parms->App_table = SPEECH_RECOGNITION;
            debug_log_fp("ASR \n");
        }
        else if (p_arsi_task_config->input_source == AUDIO_SOURCE_VOICE_RECOGNITION)  // VR
        {
            strncat(categoryPath, RECORD_VR_PATH",", CATE_PATH_LEN-1);
            parms->App_table = MONO_RECORD;
            debug_log_fp("AUDIO_SOURCE_VOICE_RECOGNITION \n");
        }
        else if (p_arsi_task_config->input_source == AUDIO_SOURCE_CUSTOMIZATION2) // AEC_RECORD
        {
            strncat(categoryPath, RECORD_CUSTOMIZATION2_PATH",", CATE_PATH_LEN-1);
            if (p_arsi_task_config->input_device_info.num_channels == 1)
            {
                parms->App_table = MONO_AEC_RECORD;
            }
            else
            {
                parms->App_table = STEREO_AEC_RECORD;
            }
            debug_log_fp("AEC_RECORD \n");
        }  // Record mode
        else if (p_arsi_task_config->input_source == AUDIO_SOURCE_UNPROCESSED) // Unprocessed
        {
            strncat(categoryPath, RECORD_UNPROCESSED_PATH",", CATE_PATH_LEN-1);
            parms->App_table = MONO_RECORD;
            debug_log_fp("AUDIO_SOURCE_UNPROCESSED \n");
        }
        else if (p_arsi_task_config->input_source == 83) // DMNR Calibration
        {
            parms->App_table = SPEECH_RECOGNITION;
            strncat(categoryPath, RECORD_ASR_PATH",", CATE_PATH_LEN-1);
            debug_log_fp("DMNR_Cal_ATH \n");
        }
        else
        {
            strncat(categoryPath, RECORD_SND_REC_NORMAL_PATH",", CATE_PATH_LEN-1);
            debug_log_fp("ELSE TASK_SCENE_RECORD\n");
        }
    }

    // device
    if (p_arsi_task_config->input_device_info.devices == AUDIO_DEVICE_IN_BUILTIN_MIC)
    {
        debug_log_fp("AUDIO_DEVICE_IN_BUILTIN_MIC\n");
        if (p_arsi_task_config->output_device_info.devices == AUDIO_DEVICE_OUT_WIRED_HEADPHONE)
        {
            // 3 pole headset
            if (parms->App_table != INCALL_RECORD)
            {
                parms->Device_mode = 1;
            }
            strncat(categoryPath, "Profile,3-pole HS", CATE_PATH_LEN-1);
            debug_log_fp("AUDIO_DEVICE_OUT_WIRED_HEADPHONE \n");
        }
        else //if (p_arsi_task_config->output_device_info.devices == AUDIO_DEVICE_OUT_EARPIECE)
        {
            // Handset
            if (parms->App_table != INCALL_RECORD)
            {
                parms->Device_mode = 0;
            }
            strncat(categoryPath, RECORD_HANDSET_PATH, CATE_PATH_LEN-1);
            debug_log_fp("RECORD_HANDSET_PATH \n");
        }
    }
    else if (p_arsi_task_config->input_device_info.devices == AUDIO_DEVICE_IN_WIRED_HEADSET)
    {
        // Headset
        if (parms->App_table != INCALL_RECORD)
        {
            parms->Device_mode = 1;
        }
        debug_log_fp("AUDIO_DEVICE_IN_WIRED_HEADSET\n");
        if (p_arsi_task_config->output_device_info.devices == AUDIO_DEVICE_OUT_WIRED_HEADSET)
        {
            debug_log_fp("AUDIO_DEVICE_OUT_WIRED_HEADSET\n");
            debug_log_fp("input_device_info.num_channels = %d\n", p_arsi_task_config->input_device_info.num_channels);
            switch (p_arsi_task_config->input_device_info.num_channels)
            {
            case 1: // 4 Pole, input_device_info.num_channels = 1
                strncat(categoryPath, RECORD_4POLE_HEADSET_PATH, CATE_PATH_LEN-1);
                break;
            case 2:
                if (p_arsi_task_config->input_source == AUDIO_SOURCE_VOICE_CALL)
                {
                    strncat(categoryPath, RECORD_4POLE_HEADSET_PATH, CATE_PATH_LEN-1);
                }
                else if (pAppOps->appHandleIsFeatureOptionEnabled(pAppOps->appHandleGetInstance(), "MTK_HEADSET_ACTIVE_NOISE_CANCELLATION_SUPPORT"))
                {
                    strncat(categoryPath, RECORD_5POLE_HEADSET_ANC_PATH, CATE_PATH_LEN-1);
                    debug_log_fp("RECORD_5POLE_HEADSET_ANC_PATH\n");
                }
                else
                {
                    strncat(categoryPath, RECORD_5POLE_HEADSET_PATH, CATE_PATH_LEN-1);
                    debug_log_fp("RECORD_5POLE_HEADSET_PATH\n");
                }
                break;
            }
        }
        else
        {
            strncat(categoryPath, RECORD_4POLE_HEADSET_PATH, CATE_PATH_LEN-1);
            debug_log_fp("Unknown Device:RECORD_4POLE_HEADSET_PATH ELSE\n");
        }
    }
    else if (p_arsi_task_config->input_device_info.devices == AUDIO_DEVICE_IN_USB_DEVICE)
    {
        // USB
        if (parms->App_table != INCALL_RECORD)
        {
            parms->Device_mode = 3;
        }
        debug_log_fp("AUDIO_DEVICE_IN_USB_DEVICE\n");
        strncat(categoryPath, RECORD_SND_REC_USB_PATH, CATE_PATH_LEN-1);
    }
    else if (p_arsi_task_config->input_device_info.devices == AUDIO_DEVICE_IN_BLUETOOTH_SCO_HEADSET)
    {
        // BT
        if (parms->App_table != INCALL_RECORD)
        {
            parms->Device_mode = 3;
        }
        debug_log_fp("AUDIO_DEVICE_IN_BLUETOOTH_SCO_HEADSET\n");
        strncat(categoryPath, RECORD_BT_PATH, CATE_PATH_LEN-1);
    }
    else
    {
        if (parms->App_table != INCALL_RECORD)
        {
            parms->Device_mode = 0;
        }
        strncat(categoryPath, RECORD_HANDSET_PATH, CATE_PATH_LEN-1);
        debug_log_fp("Unknown Device:RECORD_HANDSET_PATH ELSE\n");
    }
}

char *getSceneName(AudioType *audioType, char *scene)
{
    AppOps *appOps = appOpsGetInstance();
    CategoryType *categoryType = appOps->audioTypeGetCategoryTypeByName(audioType, "Scene");
    if (categoryType)
    {
        Category *category = appOps->categoryTypeGetCategoryByName(categoryType, scene);
        if (category)
        {
            return scene;
        }
        else
        {
            return "Default";
        }
    }

    return NULL;
}

status_t mtk_arsi_parsing_param_file_by_custom_info(
    const arsi_task_config_t *p_arsi_task_config,
    const arsi_lib_config_t  *p_arsi_lib_config,
    const string_buf_t       *product_info,
    const string_buf_t       *param_file_path,
    const string_buf_t       *custom_info,
    data_buf_t               *p_param_buf,
    const debug_log_fp_t      debug_log_fp)
{
    if (p_arsi_task_config == NULL ||
        p_arsi_lib_config == NULL ||
        product_info == NULL ||
        param_file_path == NULL ||
        custom_info == NULL ||
        p_param_buf == NULL ||
        debug_log_fp == NULL) {
        return BAD_VALUE;
    }

    SPH_ENH_ctrl_struct *parms = (SPH_ENH_ctrl_struct *)p_param_buf->p_buffer;

    Word16 *filter;
    uWord16 i;
    char custom_scene[SCENE_NAME_MAX_LEN] = {0};

    if (strstr(custom_info->p_string, KEY_SET_AUDIO_CUSTOM_SCENE) != NULL) {
        char *value = custom_info->p_string + strlen(KEY_SET_AUDIO_CUSTOM_SCENE);
        if (!strcmp(value, "")) {
            strncpy(custom_scene, "Default", SCENE_NAME_MAX_LEN - 1);
        } else {
            strncpy(custom_scene, value, SCENE_NAME_MAX_LEN - 1);
        }
        debug_log_fp("%s(), Scene = %s (%s)", __FUNCTION__, custom_scene, custom_info->p_string);
    }
    // Word16 *BesRecordDMNRParam = parms->DMNR_cal_data_SWB_set1; //DMNR_cal_data;


    AppOps *pAppOps = appOpsGetInstance();
    AppHandle *pAppHandle = pAppOps->appHandleGetInstance();
    AudioType *VoIPAudioType = pAppOps->appHandleGetAudioTypeByName(pAppHandle, VOIP_AUDIO_TYPE);
    AudioType *VoIPDmnrAudioType = pAppOps->appHandleGetAudioTypeByName(pAppHandle, VOIPDMNR_AUDIO_TYPE);
    AudioType *VoIPGeneralAudioType = pAppOps->appHandleGetAudioTypeByName(pAppHandle, VOIPGENERAL_AUDIO_TYPE);
    AudioType *RecordAudioType = pAppOps->appHandleGetAudioTypeByName(pAppHandle, RECORD_AUDIO_TYPE);
    AudioType *RecordFirAudioType = pAppOps->appHandleGetAudioTypeByName(pAppHandle, RECORDFIR_AUDIO_TYPE);
    AudioType *RecordDmnrAudioType = pAppOps->appHandleGetAudioTypeByName(pAppHandle, RECORDDMNR_AUDIO_TYPE);
    ParamUnit *pParamUnit;
    Param     *pSpeciParam;
    Param     *pCommonParam;
    Param     *pInFirParam;
    Param     *pOutFirParam;
    Param     *pInFir1Param;
    Param     *pInFir2Param;
    Param     *pDmnrParam;
    char categoryPath[CATE_PATH_LEN];  // std::string categoryPath = "";

    debug_log_fp("Param Parser: task_scene = %d, input_source = %d", p_arsi_task_config->task_scene, p_arsi_task_config->input_source);
    debug_log_fp("input_device_info.devices = %d,output_device_info.devices = %d, input_device_info.audio_format=%d", p_arsi_task_config->input_device_info.devices, p_arsi_task_config->output_device_info.devices, p_arsi_task_config->input_device_info.audio_format);
    parms->Device_mode = 100; // Impossible Value

    // Get Record category path
    if (p_arsi_task_config->task_scene == TASK_SCENE_VOIP)
    {
        char *sceneName = getSceneName(VoIPAudioType, custom_scene);
        get_voip_category_path(sceneName, categoryPath, parms, p_arsi_task_config, p_arsi_lib_config, debug_log_fp, pAppOps);
    }
    else if (p_arsi_task_config->task_scene == TASK_SCENE_RECORD)
    {
        char *sceneName = getSceneName(RecordAudioType, custom_scene);
        get_record_category_path(sceneName, categoryPath, parms, p_arsi_task_config, p_arsi_lib_config, debug_log_fp, pAppOps);
    }
    ALOGD("categoryPath = %s, custom_scene = %s", categoryPath, custom_scene);

    pAppOps->audioTypeReadLock(VoIPAudioType, __FUNCTION__);
    pAppOps->audioTypeReadLock(VoIPDmnrAudioType, __FUNCTION__);
    pAppOps->audioTypeReadLock(VoIPGeneralAudioType, __FUNCTION__);
    pAppOps->audioTypeReadLock(RecordAudioType, __FUNCTION__);
    pAppOps->audioTypeReadLock(RecordFirAudioType, __FUNCTION__);
    pAppOps->audioTypeReadLock(RecordDmnrAudioType, __FUNCTION__);

    // set speech parameters+++ (xml type is uint)
    if (p_arsi_task_config->task_scene == TASK_SCENE_VOIP)
    {

        pParamUnit = pAppOps->audioTypeGetParamUnit(VoIPAudioType, categoryPath);
        pSpeciParam = pAppOps->paramUnitGetParamByName(pParamUnit, VOIP_PARAM);
        for (i = 0; i < pSpeciParam->arraySize; i++)
        {
            parms->enhance_pars_WB[i] = (uWord16)(*((uWord32 *)(pSpeciParam->data) + i));
        }
        // memcpy(parms->enhance_pars_WB, (uWord16*)pSpeciParam->data, pSpeciParam->arraySize * sizeof(uWord16));
    }
    else    // record case
    {
        pParamUnit = pAppOps->audioTypeGetParamUnit(RecordAudioType, categoryPath);
        pSpeciParam = pAppOps->paramUnitGetParamByName(pParamUnit, RECORD_PARAM);
        for (i = 0; i < pSpeciParam->arraySize; i++)
        {
            parms->enhance_pars_SWB[i] = (uWord16)(*((uWord32 *)(pSpeciParam->data) + i));
            parms->enhance_pars_WB[i] = (uWord16)(*((uWord32 *)(pSpeciParam->data) + i));
        }
        // memcpy(parms->enhance_pars_SWB, (uWord16*)pSpeciParam->data, pSpeciParam->arraySize * sizeof(uWord16));
    }


    //common parameters as same as VoIP's
    pParamUnit = pAppOps->audioTypeGetParamUnit(VoIPGeneralAudioType, VOIPGENERAL_COMMON_PATH);
    pCommonParam = pAppOps->paramUnitGetParamByName(pParamUnit, VOIPGENERAL_PARAM_NAME);
    for (i = 0; i < pCommonParam->arraySize; i++)
    {
        parms->common_pars[i] = (uWord16)(*((uWord32 *)(pCommonParam->data) + i));
    }

    //FIR parameters+++
    if (p_arsi_task_config->task_scene == TASK_SCENE_VOIP ||
            p_arsi_task_config->input_source == AUDIO_SOURCE_CUSTOMIZATION2)
    {
        filter = parms->Compen_filter_WB;

        pParamUnit = pAppOps->audioTypeGetParamUnit(VoIPAudioType, categoryPath);
        pInFirParam = pAppOps->paramUnitGetParamByName(pParamUnit, VOIP_IN_FIR_PARAM);
        pOutFirParam = pAppOps->paramUnitGetParamByName(pParamUnit, VOIP_OUT_FIR_PARAM);

        // VoIP FIR parameter have 3 FIR parameter, but 1 FIR parameter work for SWIP limitation.
        memcpy(filter, (Word16 *)pInFirParam->data, pInFirParam->arraySize * sizeof(Word16));  // UL1
        // memcpy(&filter[pInFirParam->arraySize], (Word16 *)pInFirParam->data, pInFirParam->arraySize * sizeof(Word16));  // UL2
        memcpy(&filter[pInFirParam->arraySize], (Word16 *)pOutFirParam->data, pOutFirParam->arraySize * sizeof(Word16));  // DL


        // IIR parameter
        pInFirParam = pAppOps->paramUnitGetParamByName(pParamUnit, VOIP_IN_IIR_PARAM);
        if(NULL != pInFirParam)
        {
            memcpy(parms->ENH_IIR_COEF_UL_WB, (Word16*)pInFirParam->data, pInFirParam->arraySize * sizeof(Word16));
        }
        else
        {
            debug_log_fp("[DefaultParam] sph_in_iir_enh_dsp");
            memcpy(parms->ENH_IIR_COEF_UL_WB, Sph_IIR_default, ENH_IIR_LENGTH * sizeof(Word16));

        }

        pOutFirParam = pAppOps->paramUnitGetParamByName(pParamUnit, VOIP_OUT_IIR_PARAM);
        if(NULL != pOutFirParam)
        {
            memcpy(parms->ENH_IIR_COEF_DL_WB, (Word16*)pOutFirParam->data, pOutFirParam->arraySize * sizeof(Word16));
        }
        else
        {
            debug_log_fp("[DefaultParam] sph_out_iir_enh_dsp");
            memcpy(parms->ENH_IIR_COEF_DL_WB, Sph_IIR_default, ENH_IIR_LENGTH * sizeof(Word16));
        }

        // MIC1 IIR
        pInFirParam = pAppOps->paramUnitGetParamByName(pParamUnit, VOIP_MIC1_DC_IIR_PARAM);
        if(NULL != pInFirParam)
        {
            memcpy(parms->MIC1_IIR_COEF_UL_WB, (Word16*)pInFirParam->data, pInFirParam->arraySize * sizeof(Word16));
        }
        else
        {
            debug_log_fp("[DefaultParam] sph_in_iir_mic1_dsp");
            memcpy(parms->MIC1_IIR_COEF_UL_WB,  Sph_Mic_IIR_In_default, DC_REMOVAL_IIR_LENGTH * sizeof(Word16));
        }
        // MIC2 IIR
        pInFirParam = pAppOps->paramUnitGetParamByName(pParamUnit, VOIP_MIC2_DC_IIR_PARAM);
        if(NULL != pInFirParam)
        {
            memcpy(parms->MIC2_IIR_COEF_UL_WB, (Word16*)pInFirParam->data, pInFirParam->arraySize * sizeof(Word16));
        }
        else
        {
            debug_log_fp("[DefaultParam] sph_in_iir_mic2_dsp");
            memcpy(parms->MIC2_IIR_COEF_UL_WB, Sph_Mic_IIR_In_default, DC_REMOVAL_IIR_LENGTH * sizeof(Word16));
        }

    }
    else
    {
        // Record 2 FIR param is work
        pParamUnit = pAppOps->audioTypeGetParamUnit(RecordFirAudioType, categoryPath);
        pInFir1Param = pAppOps->paramUnitGetParamByName(pParamUnit, RECORD_IN_FIR1_PARAM);
        pInFir2Param = pAppOps->paramUnitGetParamByName(pParamUnit, RECORD_IN_FIR2_PARAM);

        // for normal record (48k)
        filter = parms->Compen_filter_SWB;
        memcpy(filter, (Word16*)pInFir1Param->data, pInFir1Param->arraySize * sizeof(Word16));   // UL1
        memcpy(&filter[pInFir1Param->arraySize], (Word16*)pInFir2Param->data, pInFir2Param->arraySize * sizeof(Word16)); // UL2
        // for in-call record (16k)
        filter = parms->Compen_filter_WB;
        memcpy(filter, (Word16*)pInFir1Param->data, pInFir1Param->arraySize * sizeof(Word16)); // UL1
        memcpy(&filter[pInFir1Param->arraySize], (Word16*)pInFir2Param->data, pInFir2Param->arraySize * sizeof(Word16)); // UL2

        // Get IIR for work
        pParamUnit = pAppOps->audioTypeGetParamUnit(RecordAudioType, categoryPath);
        pInFir1Param = pAppOps->paramUnitGetParamByName(pParamUnit, RECORD_IN_IIR1_PARAM);
        if( NULL != pInFir1Param )
        {
            memcpy(parms->ENH_IIR_COEF_UL_SWB, (Word16*)pInFir1Param->data, pInFir1Param->arraySize * sizeof(Word16));
            memcpy(parms->ENH_IIR_COEF_UL_WB, (Word16*)pInFir1Param->data, pInFir1Param->arraySize * sizeof(Word16));
        }
        else
        {
            debug_log_fp("[DefaultParam] sph_in_iir_mic1_enh_dsp");
            memcpy(parms->ENH_IIR_COEF_UL_SWB, Sph_IIR_default, ENH_IIR_LENGTH * sizeof(Word16));
            memcpy(parms->ENH_IIR_COEF_UL_WB, Sph_IIR_default, ENH_IIR_LENGTH * sizeof(Word16));
        }

        pInFir2Param = pAppOps->paramUnitGetParamByName(pParamUnit, RECORD_IN_IIR2_PARAM);
        if( NULL != pInFir2Param )
        {
            memcpy(parms->ENH_IIR_COEF_DL_SWB, (Word16*)pInFir2Param->data, pInFir2Param->arraySize * sizeof(Word16));
            memcpy(parms->ENH_IIR_COEF_DL_WB, (Word16*)pInFir2Param->data, pInFir2Param->arraySize * sizeof(Word16));
        }
        else
        {
            debug_log_fp("[DefaultParam] sph_in_iir_mic2_enh_dsp");
            memcpy(parms->ENH_IIR_COEF_DL_SWB, Sph_IIR_default, ENH_IIR_LENGTH * sizeof(Word16));
            memcpy(parms->ENH_IIR_COEF_DL_WB, Sph_IIR_default, ENH_IIR_LENGTH * sizeof(Word16));
        }

        // MIC 1 & 2 IIR
        pInFir1Param = pAppOps->paramUnitGetParamByName(pParamUnit, RECORD_MIC1_DC_IIR_PARAM);
        if( NULL != pInFir1Param )
        {
            memcpy(parms->MIC1_IIR_COEF_UL_SWB, (Word16*)pInFir1Param->data, pInFir1Param->arraySize * sizeof(Word16));
            memcpy(parms->MIC1_IIR_COEF_UL_WB, (Word16*)pInFir1Param->data, pInFir1Param->arraySize * sizeof(Word16));
        }
        else
        {
            debug_log_fp("[DefaultParam] sph_in_iir_mic1_dsp");
            memcpy(parms->MIC1_IIR_COEF_UL_SWB, Sph_Mic_IIR_In_default, DC_REMOVAL_IIR_LENGTH * sizeof(Word16));
            memcpy(parms->MIC1_IIR_COEF_UL_WB, Sph_Mic_IIR_In_default, DC_REMOVAL_IIR_LENGTH * sizeof(Word16));
        }

        pInFir2Param = pAppOps->paramUnitGetParamByName(pParamUnit, RECORD_MIC2_DC_IIR_PARAM);
        if( NULL != pInFir2Param )
        {
            memcpy(parms->MIC2_IIR_COEF_UL_SWB, (Word16*)pInFir2Param->data, pInFir2Param->arraySize * sizeof(Word16));
            memcpy(parms->MIC2_IIR_COEF_UL_WB, (Word16*)pInFir2Param->data, pInFir2Param->arraySize * sizeof(Word16));
        }
        else
        {
            debug_log_fp("[DefaultParam] sph_in_iir_mic2_dsp");
            memcpy(parms->MIC2_IIR_COEF_UL_SWB, Sph_Mic_IIR_In_default, DC_REMOVAL_IIR_LENGTH * sizeof(Word16));
            memcpy(parms->MIC2_IIR_COEF_UL_WB, Sph_Mic_IIR_In_default, DC_REMOVAL_IIR_LENGTH * sizeof(Word16));
        }

    }

    //DMNR parameters+++
    {
        //DMNR parameters
        //google default input source AUDIO_SOURCE_VOICE_RECOGNITION not using DMNR (on/off by parameters)
        if (p_arsi_task_config->task_scene == TASK_SCENE_VOIP ||
                p_arsi_task_config->input_source == AUDIO_SOURCE_CUSTOMIZATION2)
        {
            pParamUnit = pAppOps->audioTypeGetParamUnit(VoIPDmnrAudioType, categoryPath);
            pDmnrParam = pAppOps->paramUnitGetParamByName(pParamUnit, VOIP_DMNR_PARAM_NB1);

            if(NULL!=pDmnrParam && DMNR_NB_REAL_SIZE == pDmnrParam->arraySize)
            {
                memcpy(parms->DMNR_cal_data_NB_set1, (Word16 *)pDmnrParam->data, pDmnrParam->arraySize * sizeof(Word16));
            }
            else
            {
                debug_log_fp("[DefaultParam] dmnr_para_nb1 for %s", categoryPath);
                memcpy(parms->DMNR_cal_data_NB_set1, Sph_DMNR_NB_default, DMNR_NB_REAL_SIZE * sizeof(Word16));
            }

            // NB set2
            pParamUnit = pAppOps->audioTypeGetParamUnit(VoIPDmnrAudioType, categoryPath);
            pDmnrParam = pAppOps->paramUnitGetParamByName(pParamUnit, VOIP_DMNR_PARAM_NB2);

            if(NULL!=pDmnrParam && DMNR_NB_REAL_SIZE == pDmnrParam->arraySize)
            {
                memcpy(parms->DMNR_cal_data_NB_set2, (Word16 *)pDmnrParam->data, pDmnrParam->arraySize * sizeof(Word16));
            }
            else
            {
                debug_log_fp("[DefaultParam] dmnr_para_nb2 for %s", categoryPath);
                memcpy(parms->DMNR_cal_data_NB_set2, Sph_DMNR_NB_default, DMNR_NB_REAL_SIZE * sizeof(Word16));
            }

            // WB set1
            pParamUnit = pAppOps->audioTypeGetParamUnit(VoIPDmnrAudioType, categoryPath);
            pDmnrParam = pAppOps->paramUnitGetParamByName(pParamUnit, VOIP_DMNR_PARAM_WB1);

            // size should be DMNR_WB_REAL_SIZE(76) same as default
            if(NULL!=pDmnrParam && DMNR_WB_REAL_SIZE == pDmnrParam->arraySize)
            {
                memcpy(parms->DMNR_cal_data_WB_set1, (Word16 *)pDmnrParam->data, pDmnrParam->arraySize * sizeof(Word16));
            }
            else
            {
                debug_log_fp("[DefaultParam] dmnr_para_wb1 for %s", categoryPath);
                memcpy(parms->DMNR_cal_data_WB_set1, Sph_DMNR_WB_default, DMNR_WB_REAL_SIZE * sizeof(Word16));
            }

            // WB set2
            pParamUnit = pAppOps->audioTypeGetParamUnit(VoIPDmnrAudioType, categoryPath);
            pDmnrParam = pAppOps->paramUnitGetParamByName(pParamUnit, VOIP_DMNR_PARAM_WB2);

            // size should be DMNR_WB_REAL_SIZE(76) same as default
            if(NULL!=pDmnrParam && DMNR_WB_REAL_SIZE == pDmnrParam->arraySize)
            {
                memcpy(parms->DMNR_cal_data_WB_set2, (Word16 *)pDmnrParam->data, pDmnrParam->arraySize * sizeof(Word16));
            }
            else
            {
                debug_log_fp("[DefaultParam] dmnr_para_wb2 for %s", categoryPath);
                memcpy(parms->DMNR_cal_data_WB_set2, Sph_DMNR_WB_default, DMNR_WB_REAL_SIZE * sizeof(Word16));
            }

            // SWB set1
            pParamUnit = pAppOps->audioTypeGetParamUnit(VoIPDmnrAudioType, categoryPath);
            pDmnrParam = pAppOps->paramUnitGetParamByName(pParamUnit, VOIP_DMNR_PARAM_SWB1);

            if(NULL!=pDmnrParam && DMNR_SWB_REAL_SIZE == pDmnrParam->arraySize)
            {
                memcpy(parms->DMNR_cal_data_SWB_set1, (Word16 *)pDmnrParam->data, pDmnrParam->arraySize * sizeof(Word16));
            }
            else
            {
                debug_log_fp("[DefaultParam] dmnr_para_swb1 for %s", categoryPath);
                memcpy(parms->DMNR_cal_data_SWB_set1, Sph_DMNR_SWB_default, DMNR_SWB_REAL_SIZE * sizeof(Word16));
            }

            // SWB set2
            pParamUnit = pAppOps->audioTypeGetParamUnit(VoIPDmnrAudioType, categoryPath);
            pDmnrParam = pAppOps->paramUnitGetParamByName(pParamUnit, VOIP_DMNR_PARAM_SWB2);

            if(NULL!=pDmnrParam && DMNR_SWB_REAL_SIZE == pDmnrParam->arraySize)
            {
                memcpy(parms->DMNR_cal_data_SWB_set2, (Word16 *)pDmnrParam->data, pDmnrParam->arraySize * sizeof(Word16));
            }
            else
            {
                debug_log_fp("[DefaultParam] dmnr_para_swb2 for %s", categoryPath);
                memcpy(parms->DMNR_cal_data_SWB_set2, Sph_DMNR_SWB_default, DMNR_SWB_REAL_SIZE * sizeof(Word16));
            }

        }
        else
        {
            pParamUnit = pAppOps->audioTypeGetParamUnit(RecordDmnrAudioType, categoryPath);
            pDmnrParam = pAppOps->paramUnitGetParamByName(pParamUnit, RECORD_DMNR_PARAM);

            memcpy(parms->DMNR_cal_data_SWB_set1, (Word16 *)pDmnrParam->data, pDmnrParam->arraySize * sizeof(Word16));
        }
    }
    pAppOps->audioTypeUnlock(VoIPAudioType);
    pAppOps->audioTypeUnlock(VoIPDmnrAudioType);
    pAppOps->audioTypeUnlock(VoIPGeneralAudioType);
    pAppOps->audioTypeUnlock(RecordAudioType);
    pAppOps->audioTypeUnlock(RecordFirAudioType);
    pAppOps->audioTypeUnlock(RecordDmnrAudioType);


    if (p_arsi_task_config->input_source == 83) // DMNR Calibration
    {
        parms->enhance_pars_WB[47] |= 32768;
    }
    else if (p_arsi_task_config->task_scene == TASK_SCENE_VOIP ||
             p_arsi_task_config->input_source == AUDIO_SOURCE_CUSTOMIZATION2) // SmartPA Pilot Tone
    {
        if (p_arsi_task_config->output_device_info.hw_info_mask == OUTPUT_DEVICE_HW_INFO_SMARTPA_SPEAKER)
        {
            parms->enhance_pars_WB[12] |= 64;
        }
    }

    //AEC off
    if ((p_arsi_task_config->input_source == AUDIO_SOURCE_VOICE_COMMUNICATION) &&
        (p_arsi_task_config->input_device_info.devices != AUDIO_DEVICE_IN_ALL_SCO) &&
        (p_arsi_task_config->enhancement_feature_mask & ENHANCEMENT_FEATURE_EC) == 0) {
        parms->enhance_pars_WB[0] = 0;
        parms->enhance_pars_WB[1] = 479;
    }

    //NS off
    if ((p_arsi_task_config->input_source == AUDIO_SOURCE_VOICE_COMMUNICATION) &&
        (p_arsi_task_config->input_device_info.devices != AUDIO_DEVICE_IN_ALL_SCO) &&
        (p_arsi_task_config->enhancement_feature_mask & ENHANCEMENT_FEATURE_NS) == 0) {
        //Dual mic
        parms->enhance_pars_WB[13] = 0;
        parms->enhance_pars_WB[14] = 0;
        //1-mic
        parms->enhance_pars_WB[4] &= ~1;
    }

    //AGC off
    if ((p_arsi_task_config->input_source == AUDIO_SOURCE_VOICE_COMMUNICATION) &&
        (p_arsi_task_config->input_device_info.devices != AUDIO_DEVICE_IN_ALL_SCO) &&
        (p_arsi_task_config->enhancement_feature_mask & ENHANCEMENT_FEATURE_AGC) == 0) {
        parms->enhance_pars_WB[3] |= 0x4000;
    }

    debug_log_fp("%s", categoryPath);
    debug_log_fp("Device_mode = %d", parms->Device_mode);
    for (i = 0; i < 48; i++)
    {
        debug_log_fp("parm_NB[%d]=%hu\n", i, parms->enhance_pars_NB[i]);
    }
    ALOGD("parm_WB[ 0]=%hu, %hu, %hu, %hu, %hu, %hu, %hu, %hu, %hu, %hu\n,parm_WB[10]=%hu, %hu, %hu, %hu, %hu, %hu, %hu, %hu, %hu\n,parm_WB[32]=%hu, parm_WB[47]=%hu\n",
                               parms->enhance_pars_WB[ 0]
                             , parms->enhance_pars_WB[ 1]
                             , parms->enhance_pars_WB[ 2]
                             , parms->enhance_pars_WB[ 3]
                             , parms->enhance_pars_WB[ 4]
                             , parms->enhance_pars_WB[ 5]
                             , parms->enhance_pars_WB[ 6]
                             , parms->enhance_pars_WB[ 7]
                             , parms->enhance_pars_WB[ 8]
                             , parms->enhance_pars_WB[ 9]
                             , parms->enhance_pars_WB[10]
                             , parms->enhance_pars_WB[11]
                             , parms->enhance_pars_WB[12]
                             , parms->enhance_pars_WB[13]
                             , parms->enhance_pars_WB[14]
                             , parms->enhance_pars_WB[15]
                             , parms->enhance_pars_WB[16]
                             , parms->enhance_pars_WB[17]
                             , parms->enhance_pars_WB[18]
                             , parms->enhance_pars_WB[32]
                             , parms->enhance_pars_WB[47]);
    for (i = 0; i < 48; i++)
    {
        debug_log_fp("parm_SWB[%d]=%hu\n", i, parms->enhance_pars_SWB[i]);
    }
    ALOGD("comm_parm[ 0]=%hu, %hu, %hu, %hu, %hu, %hu, %hu, %hu, %hu, %hu, %hu, %hu\n",
                                 parms->common_pars[ 0]
                               , parms->common_pars[ 1]
                               , parms->common_pars[ 2]
                               , parms->common_pars[ 3]
                               , parms->common_pars[ 4]
                               , parms->common_pars[ 5]
                               , parms->common_pars[ 6]
                               , parms->common_pars[ 7]
                               , parms->common_pars[ 8]
                               , parms->common_pars[ 9]
                               , parms->common_pars[10]
                               , parms->common_pars[11]);

    filter = parms->Compen_filter_NB;
    debug_log_fp("[=coef=]Compen_filter_NB: p[0]=%hu, p[1]=%hu, p[2]=%hu, p[3]=%hu, p[4]=%hu",
                 filter[0], filter[1], filter[2], filter[3], filter[4]);
    filter = parms->Compen_filter_WB;
    ALOGD("[=coef=]Compen_filter_WB: p[0]=%hu, p[1]=%hu, p[2]=%hu, p[3]=%hu, p[4]=%hu",
                 filter[0], filter[1], filter[2], filter[3], filter[4]);
    filter = parms->Compen_filter_SWB;
    debug_log_fp("[=coef=]Compen_filter_SWB: p[0]=%hu, p[1]=%hu, p[2]=%hu, p[3]=%hu, p[4]=%hu",
                 filter[0], filter[1], filter[2], filter[3], filter[4]);

    filter = parms->MIC1_IIR_COEF_UL_NB;
    debug_log_fp("[=coef=]MIC1_IIR_COEF_UL_NB: p[0]=%hu, p[1]=%hu, p[2]=%hu, p[3]=%hu, p[4]=%hu",
                 filter[0], filter[1], filter[2], filter[3], filter[4]);
    filter = parms->MIC2_IIR_COEF_UL_NB;
    debug_log_fp("[=coef=]MIC2_IIR_COEF_UL_NB: p[0]=%hu, p[1]=%hu, p[2]=%hu, p[3]=%hu, p[4]=%hu",
                 filter[0], filter[1], filter[2], filter[3], filter[4]);
    filter = parms->MIC1_IIR_COEF_UL_WB;
    ALOGD("[=coef=]MIC1_IIR_COEF_UL_WB: p[0]=%hu, p[1]=%hu, p[2]=%hu, p[3]=%hu, p[4]=%hu",
                 filter[0], filter[1], filter[2], filter[3], filter[4]);
    filter = parms->MIC2_IIR_COEF_UL_WB;
    ALOGD("[=coef=]MIC2_IIR_COEF_UL_WB: p[0]=%hu, p[1]=%hu, p[2]=%hu, p[3]=%hu, p[4]=%hu",
                 filter[0], filter[1], filter[2], filter[3], filter[4]);
    filter = parms->MIC1_IIR_COEF_UL_SWB;
    debug_log_fp("[=coef=]MIC1_IIR_COEF_UL_SWB: p[0]=%hu, p[1]=%hu, p[2]=%hu, p[3]=%hu, p[4]=%hu",
                 filter[0], filter[1], filter[2], filter[3], filter[4]);
    filter = parms->MIC2_IIR_COEF_UL_SWB;
    debug_log_fp("[=coef=]MIC2_IIR_COEF_UL_SWB: p[0]=%hu, p[1]=%hu, p[2]=%hu, p[3]=%hu, p[4]=%hu",
                 filter[0], filter[1], filter[2], filter[3], filter[4]);

    filter = parms->ENH_IIR_COEF_UL_NB;
    debug_log_fp("[=coef=]ENH_IIR_COEF_UL_NB: p[0]=%hu, p[1]=%hu, p[2]=%hu, p[3]=%hu, p[4]=%hu",
                 filter[0], filter[1], filter[2], filter[3], filter[4]);
    filter = parms->ENH_IIR_COEF_DL_NB;
    debug_log_fp("[=coef=]ENH_IIR_COEF_DL_NB: p[0]=%hu, p[1]=%hu, p[2]=%hu, p[3]=%hu, p[4]=%hu",
                 filter[0], filter[1], filter[2], filter[3], filter[4]);
    filter = parms->ENH_IIR_COEF_UL_WB;
    ALOGD("[=coef=]ENH_IIR_COEF_UL_WB: p[0]=%hu, p[1]=%hu, p[2]=%hu, p[3]=%hu, p[4]=%hu",
                 filter[0], filter[1], filter[2], filter[3], filter[4]);
    filter = parms->ENH_IIR_COEF_DL_WB;
    ALOGD("[=coef=]ENH_IIR_COEF_DL_WB: p[0]=%hu, p[1]=%hu, p[2]=%hu, p[3]=%hu, p[4]=%hu",
                 filter[0], filter[1], filter[2], filter[3], filter[4]);
    filter = parms->ENH_IIR_COEF_UL_SWB;
    debug_log_fp("[=coef=]ENH_IIR_COEF_UL_SWB: p[0]=%hu, p[1]=%hu, p[2]=%hu, p[3]=%hu, p[4]=%hu",
                 filter[0], filter[1], filter[2], filter[3], filter[4]);
    filter = parms->ENH_IIR_COEF_DL_SWB;
    debug_log_fp("[=coef=]ENH_IIR_COEF_DL_SWB: p[0]=%hu, p[1]=%hu, p[2]=%hu, p[3]=%hu, p[4]=%hu",
                 filter[0], filter[1], filter[2], filter[3], filter[4]);

    p_param_buf->data_size = sizeof(SPH_ENH_ctrl_struct);
    debug_log_fp("Set Param done\n");
    return 0;
} //  arsi_parsing_param_file

