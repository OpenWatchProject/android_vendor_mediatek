#include "AudioALSAHardware.h"
#include "AudioALSAHardwareResourceManager.h"
#include <cutils/atomic.h>
#include <math.h>

#include "AudioALSAStreamManager.h"

#include "AudioFtm.h" // TODO(Harvey): remove it
#include "LoopbackManager.h" // TODO(Harvey): remove it
#include "AudioALSAStreamIn.h"
#include "AudioALSAStreamOut.h"
#include "AudioMTKHardwareCommand.h"
//#include "AudioALSAVolumeController.h"
#include "AudioVolumeFactory.h"


#include "AudioALSAFMController.h"

#include "AudioALSAParamTuner.h"

#include "SpeechDriverFactory.h"

#include <SpeechEnhancementController.h>
#include "AudioALSADeviceParser.h"
#ifdef MTK_VOW_SUPPORT
#include "AudioALSAVoiceWakeUpController.h"
#endif
#include "WCNChipController.h"
#include "AudioBTCVSDControl.h"

#include "AudioALSAANCController.h"

#include "AudioSmartPaController.h"

#if defined(MTK_AUDIO_SMARTPASCP_SUPPORT)
#include "AudioSmartPaParam.h"
#endif


#ifdef MTK_AUDIO_SCP_SUPPORT
#include <AudioMessengerIPI.h>
#endif

#if defined(MTK_AUDIO_DSP_RECOVERY_SUPPORT)
#include <audio_dsp_controller.h>
#endif

#ifdef MTK_AURISYS_FRAMEWORK_SUPPORT
#include <aurisys_controller.h>
#endif

#ifdef MTK_USB_PHONECALL
#include "AudioUSBPhoneCallController.h"
#endif

#if defined(MTK_AUDIO_HIERARCHICAL_PARAM_SUPPORT)
#include <SpeechConfig.h>
#endif

#include <AudioToolkit.h>

#ifdef MTK_SPEECH_ENCRYPTION_SUPPORT
#include <SpeechDataEncrypter.h>
#endif

#ifdef MTK_LATENCY_DETECT_PULSE
#include "AudioDetectPulse.h"
#endif
#include "SpeechVMRecorder.h"

#if defined(MTK_AUDIO_KS)
#include "AudioBitTrueTest.h"
#endif

#include <SpeechUtility.h>
#include <SpeechPcmMixerBGSPlayer.h>

#ifdef MTK_AUDIODSP_SUPPORT
#include "AudioALSAHandlerKtv.h"
#endif

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "AudioALSAHardware"

namespace android {

/*==============================================================================
 *                     setParameters keys // TODO(Harvey): Move to a specific class for setParameters
 *============================================================================*/
// Phone Call Related
static String8 keySetVTSpeechCall       = String8("SetVTSpeechCall");
static String8 keyBtHeadsetNrec         = String8("bt_headset_nrec");
static String8 keyBtHeadsetName         = String8("bt_headset_name");
static String8 keySetFlightMode         = String8("SetFlightMode");
static String8 keyBtscoOnOff            = String8("BT_SCO");

// Set BGS Mute
static String8 keySet_BGS_DL_Mute       = String8("Set_BGS_DL_Mute");
static String8 keySet_BGS_UL_Mute       = String8("Set_BGS_UL_Mute");
static String8 keyMuteBGS_DLVoice       = String8("MuteBGS_DLVoice");

// Set Phone Call Mute
static String8 keySet_SpeechCall_DL_Mute = String8("Set_SpeechCall_DL_Mute");
static String8 keySet_SpeechCall_UL_Mute = String8("Set_SpeechCall_UL_Mute");

// BT WB
static String8 keySetBTMode     = String8("bt_wbs");

// FM Rx Related
static String8 keySetFmEnable           = String8("AudioSetFmDigitalEnable");
static String8 keyGetFmEnable           = String8("GetFmEnable");
static String8 keySetFmVolume           = String8("SetFmVolume");

// FM Tx Related
static String8 keyGetFmTxEnable         = String8("GetFmTxEnable");
static String8 keySetFmTxEnable         = String8("SetFmTxEnable");

#if defined(MTK_AUDIODSP_SUPPORT)
static String8 keyEarFeedbackInDSP = String8("EarFeedbackInDSP");
#endif

static String8 keyFMRXForceDisableFMTX  = String8("FMRXForceDisableFMTX");

// TDM record Related
static String8 keySetTDMRecEnable       = String8("SetTDMRecEnable");


//record left/right channel switch
//only support on dual MIC for switch LR input channel for video record when the device rotate
static String8 keyLR_ChannelSwitch = String8("LRChannelSwitch");

// BesRecord Related
static String8 keyHDREC_SET_VOICE_MODE = String8("HDREC_SET_VOICE_MODE");
static String8 keyHDREC_SET_VIDEO_MODE = String8("HDREC_SET_VIDEO_MODE");

static String8 keyGET_AUDIO_VOLUME_VER = String8("GET_AUDIO_VOLUME_VERSION");

// TTY
static String8 keySetTtyMode = String8("tty_mode");

// RTT
#if defined(MTK_RTT_SUPPORT)
static String8 keySetRttCallType = String8("rtt_call_type");
#endif

//AudioCmdHandlerService
#define MAX_BYTE_AUDIO_BUFFER 3000
static String8 keySetBuffer = String8("SetBuffer");
static String8 keySetCmd = String8("SetCmd");

//AudioPolicy Gain Table access
static String8 keyGetAudioGainFromNvRam = String8("GetAudioGainFromNvRam");
static String8 keyGetAudioGainFromXML = String8("GetAudioGainFromXML");
static String8 keyGetGainTableSceneTable = String8("GetGainTableSceneTable");
static String8 keyGetGainTableNonSceneTable = String8("GetGainTableNonSceneTable");
static String8 keyGetGainTableSceneCount = String8("GetGainTableSceneCount");
static String8 keyGetGainTableSceneIndex = String8("GetGainTableSceneIndex");
static String8 keyGetGainTableCurrentSceneIndex = String8("GetGainTableCurrentSceneIndex");

//Audio Tuning Tool  access
static String8 keyGetATTDisplayInfoTc1 = String8("GetATTDisplayInfoTc1");
static String8 keyGetATTDisplayInfoTc1BtNrec = String8("GetATTDisplayInfoTc1BtNrec");
static String8 keyGetAudioCustomParamFromNvRam = String8("GetAudioCustomParamFromNvRam");
static String8 keyGetVolumeVer1ParamFromNvRam = String8("GetVolumeVer1ParamFromNvRam");
static String8 keyGetNBSpeechParamFromNvRam = String8("GetNBSpeechParamFromNvRam");
static String8 keyGetWBSpeechParamFromNvRam = String8("GetWBSpeechParamFromNvRam");
static String8 keyGetMagiConSpeechParamFromNvRam = String8("GetMagiConSpeechParamFromNvRam");
static String8 keyGetHACSpeechParamFromNvRam = String8("GetHACSpeechParamFromNvRam");
static String8 keyGetAudioCustomDataSize = String8("GetAudioCustomDataSize");
static String8 keyGetSpeechOutFirIdxFromNvRam = String8("GetSpeechOutFirIdxFromNvRam");
static String8 keyGetSpeechNormalOutFirIdxFromNvRam = String8("GetSpeechNormalOutFirIdxFromNvRam");
static String8 keyGetSpeechHeadsetOutFirIdxFromNvRam = String8("GetSpeechHeadsetOutFirIdxFromNvRam");
static String8 keyGetSpeechHandfreeOutFirIdxFromNvRam = String8("GetSpeechHandfreeOutFirIdxFromNvRam");
static String8 keyQueryFeatureSupportInfo = String8("QueryFeatureSupportInfo");
static String8 keyGetDualMicParamFromNvRam = String8("GetDualMicParamFromNvRam");
static String8 keyGetAudioVoIPParamFromNvRam = String8("GetAudioVoIPParamFromNvRam");
static String8 keyGetNBSpeechLpbkParamFromNvRam = String8("GetNBSpeechLpbkParamFromNvRam");
static String8 keyGetMedParamFromNvRam = String8("GetMedParamFromNvRam");
static String8 keyGetAudioHFPParamFromNvRam = String8("GetAudioHFPParamFromNvRam");
static String8 keyGetHdRecordParamFromNvRam = String8("GetHdRecordParamFromNvRam");
static String8 keyGetHdRecordSceneTableFromNvRam = String8("GetHdRecordSceneTableFromNvRam");

//Engineer Mode access
static String8 keyGetDumpAEECheck = String8("GetDumpAEECheck");
static String8 keyGetDumpAudioStreamOut = String8("GetDumpAudioStreamOut");
static String8 keyGetDumpAudioMixerBuf = String8("GetDumpAudioMixerBuf");
static String8 keyGetDumpAudioTrackBuf = String8("GetDumpAudioTrackBuf");
static String8 keyGetDumpA2DPStreamOut = String8("GetDumpA2DPStreamOut");
static String8 keyGetDumpAudioStreamIn = String8("GetDumpAudioStreamIn");
static String8 keyGetDumpIdleVM = String8("GetDumpIdleVM");
static String8 keyGetDumpApSpeechEPL = String8("GetDumpApSpeechEPL");
static String8 keyGetMagiASRTestEnable = String8("GetMagiASRTestEnable");
static String8 keyGetAECRecTestEnable = String8("GetAECRecTestEnable");


//Modem Audio DVT and Debug
static String8 keySpeechDVT_SampleRate = String8("SpeechDVT_SampleRate");
static String8 keySpeechDVT_GetSampleRate = String8("SpeechDVT_GetSampleRate");
static String8 keySpeechDVT_MD_IDX = String8("SpeechDVT_MD_IDX");
static String8 keySpeechDVT_GetMD_IDX = String8("SpeechDVT_GetMD_IDX");
static String8 keySpeechDVT_MD_LPBK = String8("SpeechDVT_MD_LPBK");

// Aurisys dump control
static String8 keyGenerateVmByEpl = String8("generateVmDumpByEpl");

// Audio Tool related
//<---for audio tool(speech/ACF/HCF/DMNR/HD/Audiotaste calibration) and HQA
static String8 keySpeechParams_Update = String8("UpdateSpeechParameter");
static String8 keySpeechVolume_Update = String8("UpdateSphVolumeParameter");
static String8 keyDualMicParams_Update = String8("UpdateDualMicParameters");
static String8 keyDualMicRecPly = String8("DUAL_MIC_REC_PLAY");
static String8 keyDUALMIC_IN_FILE_NAME = String8("DUAL_MIC_IN_FILE_NAME");
static String8 keyDUALMIC_OUT_FILE_NAME = String8("DUAL_MIC_OUT_FILE_NAME");
static String8 keyDUALMIC_GET_GAIN = String8("DUAL_MIC_GET_GAIN#");
static String8 keyDUALMIC_SET_UL_GAIN = String8("DUAL_MIC_SET_UL_GAIN");
static String8 keyDUALMIC_SET_DL_GAIN = String8("DUAL_MIC_SET_DL_GAIN");
static String8 keyDUALMIC_SET_HSDL_GAIN = String8("DUAL_MIC_SET_HSDL_GAIN");
static String8 keyDUALMIC_SET_UL_GAIN_HF = String8("DUAL_MIC_SET_UL_GAIN_HF");
static String8 keyBesRecordParams_Update = String8("UpdateBesRecordParameters");
static String8 keyMagiConParams_Update = String8("UpdateMagiConParameters");
static String8 keyHACParams_Update = String8("UpdateHACParameters");
static String8 keyMusicPlusSet      = String8("SetMusicPlusStatus");
static String8 keyMusicPlusGet      = String8("GetMusicPlusStatus");
static String8 keyHDRecTunningEnable    = String8("HDRecTunningEnable");
static String8 keyHDRecVMFileName   = String8("HDRecVMFileName");
static String8 keyACFHCF_Update = String8("UpdateACFHCFParameters");
static String8 keyBesLoudnessSet      = String8("SetBesLoudnessStatus");
static String8 keyBesLoudnessGet      = String8("GetBesLoudnessStatus");
static String8 keySetAudioCustomScene     = String8("SetAudioCustomScene");
static String8 keyGetAudioCustomScene     = String8("GetAudioCustomScene");
static String8 keySmartPAandWithoutDSP     = String8("SmartPAandWithoutDSP");

static String8 kayA2dpDelayReport = String8("A2dpDelayReport");

static String8 keySetDSM = String8("DSM");

#ifdef MTK_AUDIO_TUNNELING_SUPPORT
static String8 keySetMP3PCM_DUMP = String8("MP3_PCMDUMP");
static String8 kKeySampleRate   = String8("music_offload_sample_rate");
static String8 kKeyBitRate   = String8("music_offload_avg_bit_rate");
#endif

static String8 keyGetSpeakerProtection = String8("GetSpeakerProtection");

static String8 keyANC_CMD = String8("ANC_CMD");
static String8 keyANC_UI = String8("ANC_UI");
static String8 keyANC_Enabled = String8("ANC_Enabled");

static String8 keyAudioSampleRate = String8(AudioParameter::keySamplingRate);

static const char *SOUND_ENHANCEMENT_AUDIO_TYPE = "SoundEnhancement";
static const char *SOUND_ENHANCEMENT_CATEGORY_PATH = "SoundEnhancement,Common";
static const char *SOUND_ENHANCEMENT_PARAM_HIFI_AUDIO = "hifi_audio";
static const char *SOUND_ENHANCEMENT_PARAM_BESLOUDNESS = "besloudness";

#ifdef MTK_HIFIAUDIO_SUPPORT
static String8 keyHiFiState = String8("hifi_state");
//static const char *PROPERTY_KEY_HIFI_DAC_STATE = "persist.vendor.audiohal.hifi_dac_state";
#endif

#ifdef MTK_AUDIO_SMARTPASCP_SUPPORT
static char aurisys_set_key_prefix_hal_smartpa[] = "AURISYS_SET_PARAM,HAL,SMARTPA";
#endif

#if defined(MTK_COMBO_MODEM_SUPPORT)
static char keySpeechParserSetParam[] = "SPH_PARSER_SET_PARAM,PHONE_CALL";
static char keySpeechParserGetParam[] = "SPH_PARSER_GET_PARAM,PHONE_CALL";
#endif

#ifdef MTK_AURISYS_FRAMEWORK_SUPPORT
static char aurisys_set_key_prefix[] = "AURISYS_SET_PARAM";
static char aurisys_get_key_prefix[] = "AURISYS_GET_PARAM";

static String8 keySET_AURISYS_ON = String8("SET_AURISYS_ON");

static String8 keyAurisysLibDumpDisable = String8("AURISYS_SET_PARAM,HAL,ALL,MTKSE,ENABLE_LIB_DUMP,0=SET");
static String8 keyAurisysLibDumpEnable = String8("AURISYS_SET_PARAM,HAL,ALL,MTKSE,ENABLE_LIB_DUMP,1=SET");

#endif
static String8 keySET_BypassProcess = String8("SET_BypassProcess");

#if defined(MTK_AURISYS_FRAMEWORK_SUPPORT)
static String8 aurisys_set_param_result = String8("AURISYS_SET_PARAM_RESULT");
#endif


//--->
//TDM record
static AudioMTKHardwareInterface *gAudioHardware = NULL;
static AudioMTKStreamInInterface *gAudioStreamIn = NULL;
static uint32 gTDMsetSampleRate = 44100;
static bool gTDMsetExit = false;
device_parameters_callback_t AudioALSAHardware::mHwParametersCbk = NULL;
void *AudioALSAHardware::mHwParametersCbkCookie = NULL;


#ifdef AUDIO_HQA_SUPPORT
// for Audio HQA use
static String8 keyHQA_RDMIC_P1   = String8("HQA_RDMIC_P1");
static String8 keyHQA_PGAGAIN_P1 = String8("HQA_PGAGAIN_P1");
static String8 keyHQA_PGAGAIN_P2 = String8("HQA_PGAGAIN_P2");
static String8 keyHQA_VDPG_P1 = String8("HQA_VDPG_P1");
static String8 keyHQA_VDPG_P2 = String8("HQA_VDPG_P2");
static String8 keyHQA_VDPG_P3 = String8("HQA_VDPG_P3");
static String8 keyHQA_AUDLINEPG_P1 = String8("HQA_AUDLINEPG_P1");
static String8 keyHQA_MICVUPG_P1 = String8("HQA_MICVUPG_P1");
static String8 keyHQA_MICVUPG_P2 = String8("HQA_MICVUPG_P2");
static String8 keyHQA_MEDPLY_P1 = String8("HQA_MEDPLY_P1");
static String8 keyHQA_I2SREC = String8("HQA_I2SREC");

static String8 keyHQA_AMP_MODESEL  = String8("HQA_AMP_MODESEL");
static String8 keyHQA_AMP_AMPEN    = String8("HQA_AMP_AMPEN");
static String8 keyHQA_AMP_AMPVOL   = String8("HQA_AMP_AMPVOL");
static String8 keyHQA_AMP_RECEIVER = String8("HQA_AMP_RECEIVER");
static String8 keyHQA_AMP_RECGAIN  = String8("HQA_AMP_RECGAIN");

static String8 keyHQA_I2S_OUTPUT_PLAY = String8("HQA_I2S_OUTPUT_PLAY");
static String8 keyHQA_I2S_OUTPUT_STOP = String8("HQA_I2S_OUTPUT_STOP");

#endif


// Dual Mic Noise Reduction, DMNR for Receiver
static String8 keyEnable_Dual_Mic_Setting = String8("Enable_Dual_Mic_Setting");
static String8 keyGet_Dual_Mic_Setting    = String8("Get_Dual_Mic_Setting");

// Dual Mic Noise Reduction, DMNR for Loud Speaker
static String8 keySET_LSPK_DMNR_ENABLE = String8("SET_LSPK_DMNR_ENABLE");
static String8 keyGET_LSPK_DMNR_ENABLE = String8("GET_LSPK_DMNR_ENABLE");

// Voice Clarity Engine, VCE
static String8 keySET_VCE_ENABLE = String8("SET_VCE_ENABLE");
static String8 keyGET_VCE_ENABLE = String8("GET_VCE_ENABLE");
static String8 keyGET_VCE_STATUS = String8("GET_VCE_STATUS"); // old name, rename to GET_VCE_ENABLE, but still reserve it

// Magic Conference Call
static String8 keyGET_MAGI_CONFERENCE_SUPPORT = String8("GET_MAGI_CONFERENCE_SUPPORT");
static String8 keySET_MAGI_CONFERENCE_ENABLE = String8("SET_MAGI_CONFERENCE_ENABLE");
static String8 keyGET_MAGI_CONFERENCE_ENABLE = String8("GET_MAGI_CONFERENCE_ENABLE");

//SPEECH_DATA_ENCRYPT
static String8 keyGET_SPEECH_DATA_ENCRYPT_SUPPORT = String8("GET_SPEECH_DATA_ENCRYPT_SUPPORT");
static String8 keySET_SPEECH_DATA_ENCRYPT_CONFIG = String8("SET_SPEECH_DATA_ENCRYPT_CONFIG");
static String8 keyGET_SPEECH_DATA_ENCRYPT_STATUS = String8("GET_SPEECH_DATA_ENCRYPT_STATUS");

// HAC
static String8 keyGET_HAC_SUPPORT = String8("GET_HAC_SUPPORT");
static String8 keyHAC_SETTING = String8("HACSetting");
static String8 keyGET_HAC_ENABLE = String8("GET_HAC_ENABLE");

// Super Volume
static String8 keyGET_SUPER_VOLUME_SUPPORT = String8("GET_SUPER_VOLUME_SUPPORT");
static String8 keySET_SUPER_VOLUME_ENABLE = String8("SET_SUPER_VOLUME");
static String8 keyGET_SUPER_VOLUME_ENABLE = String8("GET_SUPER_VOLUME");

// VM Log
static String8 keySET_VMLOG_CONFIG = String8("SET_VMLOG_CONFIG");
static String8 keyGET_VMLOG_CONFIG = String8("GET_VMLOG_CONFIG");

// Cust XML
static String8 keySET_CUST_XML_ENABLE = String8("SET_CUST_XML_ENABLE");
static String8 keyGET_CUST_XML_ENABLE = String8("GET_CUST_XML_ENABLE");

// Magic Conference Direction of Arrow
static String8 keySET_MAGI_CONFERENCE_DOA_ENABLE = String8("SET_MAGI_CONFERENCE_DOA_ENABLE");
static String8 keyGET_MAGI_CONFERENCE_DOA_ENABLE = String8("GET_MAGI_CONFERENCE_DOA_ENABLE");

//VoIP
//VoIP Dual Mic Noise Reduction, DMNR for Receiver
static String8 keySET_VOIP_RECEIVER_DMNR_ENABLE = String8("SET_VOIP_RECEIVER_DMNR_ENABLE");
static String8 keyGET_VOIP_RECEIVER_DMNR_ENABLE    = String8("GET_VOIP_RECEIVER_DMNR_ENABLE");

//VoIP Dual Mic Noise Reduction, DMNR for Loud Speaker
static String8 keySET_VOIP_LSPK_DMNR_ENABLE = String8("SET_VOIP_LSPK_DMNR_ENABLE");
static String8 keyGET_VOIP_LSPK_DMNR_ENABLE = String8("GET_VOIP_LSPK_DMNR_ENABLE");

//Vibration Speaker usage
static String8 keySET_VIBSPK_ENABLE = String8("SET_VIBSPK_ENABLE");
static String8 keySET_VIBSPK_RAMPDOWN = String8("SET_VIBSPK_RAMPDOWN");

#ifdef MTK_VOW_SUPPORT
// Voice Wake up
static String8 keyMTK_VOW_ENABLE = String8("MTK_VOW_ENABLE");
static String8 keyMTK_VOW_MIC_TYPE = String8("MTK_VOW_MIC_TYPE");
static String8 keyMTK_VOW_TRAINING = String8("MTK_VOW_TRAINING");
#endif

// Voice UnLock
static String8 keyGetCaptureDropTime = String8("GetCaptureDropTime");

// low latency
static String8 keySCREEN_STATE = String8("screen_state");

// Loopbacks
static String8 keySET_LOOPBACK_USE_LOUD_SPEAKER = String8("SET_LOOPBACK_USE_LOUD_SPEAKER");
static String8 keySET_LOOPBACK_TYPE = String8("SET_LOOPBACK_TYPE");
static String8 keySET_LOOPBACK_MODEM_DELAY_FRAMES = String8("SET_LOOPBACK_MODEM_DELAY_FRAMES");

static String8 keyMTK_AUDENH_SUPPORT = String8("MTK_AUDENH_SUPPORT");
static String8 keyMTK_TTY_SUPPORT = String8("MTK_TTY_SUPPORT");
static String8 keyMTK_WB_SPEECH_SUPPORT = String8("MTK_WB_SPEECH_SUPPORT");
static String8 keyMTK_DUAL_MIC_SUPPORT = String8("MTK_DUAL_MIC_SUPPORT");
static String8 keyMTK_AUDIO_HD_REC_SUPPORT = String8("MTK_AUDIO_HD_REC_SUPPORT");
static String8 keyMTK_BESLOUDNESS_SUPPORT = String8("MTK_BESLOUDNESS_SUPPORT");
static String8 keyMTK_BESSURROUND_SUPPORT = String8("MTK_BESSURROUND_SUPPORT");
static String8 keyMTK_AUDIO_HYBRID_NLE_SUPPORT = String8("MTK_AUDIO_HYBRID_NLE_SUPPORT");
static String8 keyMTK_HDMI_MULTI_CHANNEL_SUPPORT = String8("MTK_HDMI_MULTI_CHANNEL_SUPPORT");
static String8 keyMTK_VOW_SUPPORT = String8("MTK_VOW_SUPPORT");
static String8 keyMTK_VOW_2E2K_SUPPORT = String8("MTK_VOW_2E2K_SUPPORT");
static String8 keyMTK_VOW_COMMAND = String8("MTK_VOW_COMMAND");
static String8 keyMTK_VOW_COMMAND_2ND = String8("MTK_VOW_COMMAND_2ND");
static String8 keyMTK_VOW_COMMAND_3RD = String8("MTK_VOW_COMMAND_3RD");
static String8 keyMTK_BESLOUDNESS_RUN_WITH_HAL = String8("MTK_BESLOUDNESS_RUN_WITH_HAL");
static String8 keyMTK_BESLOUDNESS_MUSIC_PARAM = String8("MTK_BESLOUDNESS_MUSIC_PARAM");
static String8 keyMTK_BESLOUDNESS_RINGTONE_PARAM = String8("MTK_BESLOUDNESS_RINGTONE_PARAM");

static String8 keyNUM_HEADSET_POLE = String8("num_hs_pole");
#if defined(MTK_HYBRID_NLE_SUPPORT)
static String8 keyHYBRID_NLE_BYPASS = String8("hybrid_nle_bypass");
static String8 keyHYBRID_NLE_DUMP = String8("hybrid_nle_dump");
static String8 keyHYBRID_NLE_ENABLE = String8("AudioEnableHybridNLE");
static String8 keyHYBRID_NLE_EOP_SET = String8("AudioHybridNLEEOP");
#endif


#if defined(MTK_AUDIO_HIERARCHICAL_PARAM_SUPPORT)||defined(MTK_AUDIO_GAIN_TABLE)
static String8 keyVolumeStreamType    = String8("volumeStreamType");;
static String8 keyVolumeDevice        = String8("volumeDevice");
static String8 keyVolumeIndex         = String8("volumeIndex");
#endif

#ifdef MTK_AUDIO_GAIN_TABLE
static String8 keySpeechBand          = String8("getSpeechBand");

static String8 keySET_AT_ACS = String8("SET_AT_ACS");  //for TC1 AT%ACS
static String8 keySET_AT_ACSVolume = String8("SET_AT_ACSVolume");  //for ACS volume

static String8 keyGainTable_Update = String8("UpdateHALCustGainTable");
static String8 keyMicGain_Update = String8("UpdateMicGain");

static String8 keyBtSupportVolume = String8("bt_headset_vgs");
#endif

#define APP_SEPERATOR "#"
static String8 keyAPP_GET_FO = String8("APP_GET_FO");

#ifdef MTK_AUDIO_HIERARCHICAL_PARAM_SUPPORT
static String8 keyAPP_GET_CATEGORY = String8("APP_GET_CATEGORY");
static String8 keyAPP_GET_PARAM = String8("APP_GET_PARAM");
static String8 keyAPP_SET_PARAM = String8("APP_SET_PARAM");
static String8 keyAPP_GET_FIELD = String8("APP_GET_FIELD");
static String8 keyAPP_SET_FIELD = String8("APP_SET_FIELD");
static String8 keyAPP_SAVE_XML = String8("APP_SAVE_XML");
static String8 keyAPP_GET_CHECKLIST = String8("APP_GET_CHECKLIST");
#endif

//Telephony/Telecom voice call control
//Refresh Modem
static String8 keyRefreshModem = String8("RefreshModem");
//get audio mode
static String8 keyGetMode = String8("getMode");
//get phone call status
static String8 keyGetPhoneCallStatus = String8("getPhoneCallStatus");
static String8 keyPhone1Modem = String8("Phone1Modem");
static String8 keyPhone2Modem = String8("Phone2Modem");
static String8 keyForegroundPhoneId = String8("ForegroundPhoneId");

static String8 keyLowLatencyDebugEnable = String8("LowLatencyDebugEnable");

static String8 keyDetectPulseEnable = String8("DetectPulseEnable");
static const char PROPERTY_KEY_BTSCO_WB_ON[PROPERTY_KEY_MAX] = "vendor.streamout.btscowb";
static String8 keyAEETimeoutEnable = String8("AEELockTimeoutEnable");

// HAL dump management
static String8 keyMTK_GET_AUDIO_DUMP_FILE_LIST = String8("MTK_GET_AUDIO_DUMP_FILE_LIST");
static String8 keyMTK_GET_AUDIO_DUMP_FILE_CONTENT = String8("MTK_GET_AUDIO_DUMP_FILE_CONTENT");
static String8 keyMTK_DEL_AUDIO_DUMP_FILE = String8("MTK_DEL_AUDIO_DUMP_FILE");

// Update Rotation
static String8 keyRotation = String8("rotation");

/* BT SCO HFP client */
static String8 keyHfpEnable = String8("hfp_enable");
static String8 keyHfpSetSamplingRate = String8("hfp_set_sampling_rate");
static String8 keyHfpVolume = String8("hfp_volume");

// a2dp offload
static String8 keyA2dpCodec = String8("A2dpCodec");
static String8 keyA2dpSuspended = String8("A2dpSuspended");

#if defined(SUPPORT_FM_AUDIO_BY_PROPRIETARY_PARAMETER_CONTROL)
static String8 keyFM_AUDIO_SWITCH = String8("handle_fm"); //
static String8 keyFM_AUDIO_VOLUME = String8("fm_volume"); // 0~1.0
#define FM_AUDIO_SWITCH_OFF (0)
#define FM_AUDIO_SWITCH_ON  (1048580)
#define FM_AUDIO_VOLUME_MIN (0)
#define FM_AUDIO_VOLUME_MAX (1.0)
/*
Turn on:       handle_fm = FM_AUDIO_SWITCH_ON, and then fm_volume = x.xxx
Turn off:       handle_fm = FM_AUDIO_SWITCH_OFF
Change device:  fm_volume = 0, new stream out routing (follow the latest device of latest set streamout), fm_volume = x.xxx
No consider record case
*/

#if 0 // O version
static String8 keyFM_Mode = String8("fm_mode"); //on or off
static String8 keyFM_Radio_Volume = String8("fm_radio_volume"); //on or off
static String8 keyFM_Radio_Mute = String8("fm_radio_mute"); // 1 or 0
static String8 keyFMRadioVol = String8("FMRadioVol");   // 0~1.0
/*
Turn on:        fm_radio_mute=1 => fm_mode=on => fm_radio_volume=on => fm_radio_mute=1 => fm_radio_mute=0
Turn off:       fm_mode=off => fm_radio_volume=off
Record on:      startinput (Routing device/input source first) => FMRadioVol=xxx
Record off:     stopinput => FMRadioVol=xxx
Change device:  fm_radio_mute=1 => Routing => FMRadioVol=xxx
*/
#endif
// P version
static String8 keyFM_Radio_Mode = String8("fm_radio_mode"); //on or off or ready
static String8 keyFM_Radio_Mute = String8("fm_radio_mute"); // 1 or 0
static String8 keyFM_Radio_Volume = String8("fm_radio_volume"); // 0~1.0

#endif
enum {
    Normal_Coef_Index,
    Headset_Coef_Index,
    Handfree_Coef_Index,
    VOIPBT_Coef_Index,
    VOIPNormal_Coef_Index,
    VOIPHandfree_Coef_Index,
    AUX1_Coef_Index,
    AuX2_Coef_Index
};

class AudioParameterChangedHidlCallback {
public:
    AudioParameterChangedHidlCallback(device_audio_parameter_changed_callback_t callback, void *cookie) {
        mHidlCallback = callback;
        mHidlCookie = cookie;
    }

    device_audio_parameter_changed_callback_t mHidlCallback;
    void *mHidlCookie;
};

String8 getAudioFO(String8 queryStr) {
    char *restOfStr = NULL;
    const char *foValStr = NULL;
    char *str = strdup(queryStr.string());
    char *foName = strtok_r(str, APP_SEPERATOR, &restOfStr);

    String8 foVal = String8("");
    if (foName) {
        AppOps *appOps = appOpsGetInstance();
        if (appOps != NULL) {
            AppHandle *appHandle = appOps->appHandleGetInstance();
            foValStr = appOps->appHandleGetFeatureOptionValue(appHandle, foName);
            if (foValStr) {
                foVal = String8(foValStr);
            }
        } else {
            ALOGE("%s(), Error: AppOps == NULL", __FUNCTION__);
            ASSERT(0);
        }
    }

    ALOGD("%s(), queryStr = %s, foVal = %s\n",
          __FUNCTION__,
          queryStr.string(),
          foVal.string());

    free(str);

    return foVal;
}

#ifdef MTK_AUDIO_HIERARCHICAL_PARAM_SUPPORT
String8 getAudioParam(String8 queryStr) {
    char *paramName = NULL;
    char *categoryPath = NULL;
    char *paramDataStr = NULL;
    char *restOfStr = NULL;
    char *str = strdup(queryStr.string());
    char *audioTypeName = strtok_r(str, APP_SEPERATOR, &restOfStr);

    if (audioTypeName) {
        categoryPath = strtok_r(NULL, APP_SEPERATOR, &restOfStr);
    }

    if (categoryPath) {
        paramName = strtok_r(NULL, APP_SEPERATOR, &restOfStr);
    }

    String8 param = String8("");
    if (audioTypeName && categoryPath && paramName) {
        AppOps *appOps = appOpsGetInstance();
        if (appOps != NULL) {
            paramDataStr = appOps->utilNativeGetParam(audioTypeName, categoryPath, paramName);
            if (paramDataStr) {
                param = String8(paramDataStr);
                free(paramDataStr);
            }
        } else {
            ALOGE("%s(), Error: AppOps == NULL", __FUNCTION__);
            ASSERT(0);
        }
    }

    ALOGD("%s(), queryStr = %s, audioTypeName = %s, categoryPath = %s, paramName = %s\n",
          __FUNCTION__,
          queryStr.string(),
          audioTypeName ? audioTypeName : "",
          categoryPath ? categoryPath : "",
          paramName ? paramName : "");

    free(str);

    return param;
}

String8 getAudioField(String8 queryStr) {
    char *fieldName = NULL;
    char *paramName = NULL;
    char *categoryPath = NULL;
    char *restOfStr = NULL;
    unsigned int fieldValue = 0;
    String8 value = String8("");
    char *str = strdup(queryStr.string());
    char *audioTypeName = strtok_r(str, APP_SEPERATOR, &restOfStr);

    if (audioTypeName) {
        categoryPath = strtok_r(NULL, APP_SEPERATOR, &restOfStr);
    }

    if (categoryPath) {
        paramName = strtok_r(NULL, APP_SEPERATOR, &restOfStr);
    }

    if (paramName) {
        fieldName = strtok_r(NULL, APP_SEPERATOR, &restOfStr);
    }

    if (audioTypeName && categoryPath && paramName && fieldName) {
        AppOps *appOps = appOpsGetInstance();
        if (appOps != NULL) {
            unsigned int fieldValue = appOps->utilNativeGetField(audioTypeName, categoryPath, paramName, fieldName);
            char buf[32];
            snprintf(buf, sizeof(buf), "%d", fieldValue);
            value = String8(buf);
        } else {
            ALOGE("%s(), Error: AppOps == NULL", __FUNCTION__);
            ASSERT(0);
        }
    }
    free(str);

    return value;
}

String8 getAudioCategory(String8 queryStr) {
    bool firstCategory = true;
    String8 value = String8("");
    char *restOfStr = NULL;
    char *categoryTypeName = NULL;
    char *str = strdup(queryStr.string());
    char *audioTypeName = strtok_r(str, APP_SEPERATOR, &restOfStr);

    if (audioTypeName) {
        categoryTypeName = strtok_r(NULL, APP_SEPERATOR, &restOfStr);
    }

    if (audioTypeName && categoryTypeName) {
        AppOps *appOps = appOpsGetInstance();
        if (appOps != NULL) {
            char *categories = appOps->utilNativeGetCategory(audioTypeName, categoryTypeName);
            if (categories) {
                value = String8(categories);
                free(categories);
            }
        } else {
            ALOGE("%s(), Error: AppOps == NULL", __FUNCTION__);
            ASSERT(0);
        }
    }
    free(str);

    return value;
}

String8 getAudioCheckList(String8 queryStr) {
    char *fieldName = NULL;
    char *paramName = NULL;
    char *restOfStr = NULL;
    String8 value = String8("");
    char *str = strdup(queryStr.string());
    char *audioTypeName = strtok_r(str, APP_SEPERATOR, &restOfStr);

    if (audioTypeName) {
        paramName = strtok_r(NULL, APP_SEPERATOR, &restOfStr);
    }

    if (paramName) {
        fieldName = strtok_r(NULL, APP_SEPERATOR, &restOfStr);
    }

    if (audioTypeName && paramName && fieldName) {
        AppOps *appOps = appOpsGetInstance();
        if (appOps != NULL) {
            const char *checkList = appOps->utilNativeGetChecklist(audioTypeName, paramName, fieldName);
            if (checkList) {
                value = String8(checkList);
            }
        } else {
            ALOGE("%s(), Error: AppOps == NULL", __FUNCTION__);
            ASSERT(0);
        }
    }
    free(str);

    return value;
}

void setAudioParam(String8 queryStr) {
    char *paramName = NULL;
    char *categoryPath = NULL;
    char *paramDataStr = NULL;
    char *restOfStr = NULL;
    char *str = strdup(queryStr.string());
    char *audioTypeName = strtok_r(str, APP_SEPERATOR, &restOfStr);

    if (audioTypeName) {
        categoryPath = strtok_r(NULL, APP_SEPERATOR, &restOfStr);
    }

    if (categoryPath) {
        paramName = strtok_r(NULL, APP_SEPERATOR, &restOfStr);
    }

    if (paramName) {
        paramDataStr = strtok_r(NULL, APP_SEPERATOR, &restOfStr);
    }

    AppOps *appOps = appOpsGetInstance();
    if (appOps != NULL) {
        appOps->utilNativeSetParam(audioTypeName, categoryPath, paramName, paramDataStr);
    } else {
        ALOGE("%s(), Error: AppOps == NULL", __FUNCTION__);
        ASSERT(0);
    }
    free(str);
}

void setAudioField(String8 queryStr) {
    char *fieldValueStr = NULL;
    char *fieldName = NULL;
    char *paramName = NULL;
    char *categoryPath = NULL;
    char *restOfStr = NULL;
    char *str = strdup(queryStr.string());
    char *audioTypeName = strtok_r(str, APP_SEPERATOR, &restOfStr);

    if (audioTypeName) {
        categoryPath = strtok_r(NULL, APP_SEPERATOR, &restOfStr);
    }

    if (categoryPath) {
        paramName = strtok_r(NULL, APP_SEPERATOR, &restOfStr);
    }

    if (paramName) {
        fieldName = strtok_r(NULL, APP_SEPERATOR, &restOfStr);
    }

    if (fieldName) {
        fieldValueStr = strtok_r(NULL, APP_SEPERATOR, &restOfStr);
    }

    AppOps *appOps = appOpsGetInstance();
    if (appOps != NULL) {
        appOps->utilNativeSetField(audioTypeName, categoryPath, paramName, fieldName, fieldValueStr);
    } else {
        ALOGE("%s(), Error: AppOps == NULL", __FUNCTION__);
        ASSERT(0);
    }
    free(str);
}

void saveAudioParam(String8 audioTypeName) {
    AppOps *appOps = appOpsGetInstance();
    if (appOps != NULL) {
        appOps->utilNativeSaveXml(audioTypeName.string());
    } else {
        ALOGE("%s(), Error: AppOps == NULL", __FUNCTION__);
        ASSERT(0);
    }
}

#endif

/*==============================================================================
 *                     setParameters() keys for common
 *============================================================================*/

AudioALSAHardware::AudioALSAHardware() :
#ifdef MTK_AUDIO_SCP_SUPPORT
    mAudioMessengerIPI(AudioMessengerIPI::getInstance()),
#else
    mAudioMessengerIPI(NULL),
#endif
    mStreamManager(NULL),
    mAudioSpeechEnhanceInfoInstance(AudioSpeechEnhanceInfo::getInstance()),
    mAudioALSAVolumeController(AudioVolumeFactory::CreateAudioVolumeController()),
    mAudioALSAParamTunerInstance(NULL),
    mSpeechPhoneCallController(NULL),
    mAudioAlsaDeviceInstance(AudioALSADeviceParser::getInstance()),
#if defined(MTK_ANC_SUPPORT)
    mANCController(AudioALSAANCController::getInstance()),
#else
    mANCController(NULL),
#endif
    mFmTxEnable(false),
    mBtscoOn(false) {
    ALOGD("%s()", __FUNCTION__);

#ifdef CONFIG_MT_ENG_BUILD
    mLogEnable = 1;
#else
    mLogEnable = __android_log_is_loggable(ANDROID_LOG_DEBUG, LOG_TAG, ANDROID_LOG_INFO);
#endif


#ifdef MTK_AURISYS_FRAMEWORK_SUPPORT
    init_aurisys_controller();
#endif
    mStreamManager = AudioALSAStreamManager::getInstance();
    mSpeechPhoneCallController = AudioALSASpeechPhoneCallController::getInstance();
    mAudioALSAParamTunerInstance = AudioALSAParamTuner::getInstance();

    mNextUniqueId = 1;
    mUseTuningVolume = false;

    // Use Audio Patch For Fm
    char property_value[PROPERTY_VALUE_MAX];
    property_get("persist.vendor.audiohal.audio_patch_fm", property_value, "1");
    mUseAudioPatchForFm = (bool)atoi(property_value);
    valAudioCmd = 0;
    pAudioBuffer = NULL;
    pAudioBuffer = new char [MAX_BYTE_AUDIO_BUFFER];
    memset(pAudioBuffer, 0, MAX_BYTE_AUDIO_BUFFER);
    // for bybpass audio hw
    property_get("vendor.audio.hw.bypass", property_value, "0");
    mAudioHWBypass = atoi(property_value);
    if (mAudioHWBypass) {
        mStreamManager->setAllOutputStreamsSuspend(true, true);
    }

    memset((void *)&VolCache, 0, sizeof(VolCache));

    mAudioCustParamClient = NULL;
    mAudioCustParamClient = AudioCustParamClient::GetInstance();

#ifdef MTK_LATENCY_DETECT_PULSE
    (void) AudioDetectPulse::getInstance();
#endif

#ifdef MTK_AUDIO_HIERARCHICAL_PARAM_SUPPORT
    /* Register xml changed callback to notify HIDL callback */
    AppOps *appOps = appOpsGetInstance();
    if (appOps != NULL) {
        appOps->appHandleRegXmlChangedCb(appOps->appHandleGetInstance(), onXmlChangedCallback);
    } else {
        ALOGE("%s(), Cannot getAppOps!", __FUNCTION__);
        ASSERT(0);
    }
#endif
#ifdef MTK_AUDIO_SMARTPASCP_SUPPORT
    AudioSmartPaParam *pSmartpaParam =
        static_cast<AudioSmartPaParam *>(AudioSmartPaParam::getInstance());
#endif

    mAudioHalBtscoWB = (bool)get_uint32_from_mixctrl(PROPERTY_KEY_BTSCO_WB_ON);
    ALOGD("%s(), mAudioHalBtscoWB = %d", __FUNCTION__, mAudioHalBtscoWB);
    if (mAudioHalBtscoWB == true) {
        WCNChipController::GetInstance()->SetBTCurrentSamplingRateNumber(16000);
        AudioBTCVSDControl::getInstance()->BT_SCO_SetMode(true);
        mSpeechPhoneCallController->setBTMode(true);
    } else {
        WCNChipController::GetInstance()->SetBTCurrentSamplingRateNumber(8000);
        AudioBTCVSDControl::getInstance()->BT_SCO_SetMode(false);
        mSpeechPhoneCallController->setBTMode(false);
    }

#ifdef UL_NON_AEE_LOCK_TIMEOUT
    setNeedAEETimeoutFlg(false);
#else
    setNeedAEETimeoutFlg(true);
#endif

    memset((void *)&mBluetoothAudioOffloadCodecConfig, 0, sizeof(mBluetoothAudioOffloadCodecConfig));

#if defined(MTK_AUDIO_KS)
    if (mixer_ctl_set_value(mixer_get_ctl_by_name(AudioALSADriverUtility::getInstance()->getMixer(),
                                                  "aaudio_ion"), 0, 1)) {
        ALOGW("%s(), aaudio_ion enable fail", __FUNCTION__);
    }
#endif

}

AudioALSAHardware::~AudioALSAHardware() {
    ALOGD("%s()", __FUNCTION__);

    if (mStreamManager != NULL) { delete mStreamManager; }
    if (pAudioBuffer != NULL) { delete pAudioBuffer; pAudioBuffer = NULL;}
    mHwParametersCbk = NULL;
    mHwParametersCbkCookie = NULL;
    mAudioALSAHardware = NULL;
}

status_t AudioALSAHardware::initCheck() {
    ALOGD("%s()", __FUNCTION__);
    return NO_ERROR;
}

status_t AudioALSAHardware::setVoiceVolume(float volume) {
    ALOGD("%s(), volume = %f, mUseTuningVolume = %d", __FUNCTION__, volume, mUseTuningVolume);

    if (volume < 0.0f || volume > 1.0f || isnan(volume)) {
        ALOGE("-%s(), strange volume level %f, something wrong!!", __FUNCTION__, volume);
        return BAD_VALUE;
    }

#ifndef MTK_AUDIO_GAIN_TABLE
    if (mUseTuningVolume == false) {
        int MapVolume = AudioALSAVolumeController::logToLinear(volume);
        int degradeDB = ((256 - MapVolume) / VOICE_ONEDB_STEP) * 2;
        if (degradeDB > (VOICE_VOLUME_MAX / VOICE_ONEDB_STEP) + 1) {
            degradeDB = VOICE_VOLUME_MAX / VOICE_ONEDB_STEP;
        }
        MapVolume = 256 - VOICE_ONEDB_STEP * degradeDB;
        volume = AudioALSAVolumeController::linearToLog(MapVolume);
    }
#endif

    return mStreamManager->setVoiceVolume(volume);
}

status_t AudioALSAHardware::setMasterVolume(float volume) {
    return mStreamManager->setMasterVolume(volume);
}

status_t AudioALSAHardware::getMasterVolume(float *volume __unused) {
    *volume = mStreamManager->getMasterVolume();
    return NO_ERROR;
}

#ifdef MTK_SPEAKER_MONITOR_SPEECH_SUPPORT
status_t AudioALSAHardware::CheckDisableSpeechStream(audio_mode_t mode) {
    status_t status_change = NO_ERROR;
    bool sph_stream_support = mStreamManager->IsSphStrmSupport();
    if (sph_stream_support) {
        status_change = mStreamManager->DisableSphStrm(static_cast<audio_mode_t>(mode));
    }
    return status_change;
}
#endif

#ifdef MTK_SPEAKER_MONITOR_SPEECH_SUPPORT
status_t AudioALSAHardware::CheckEnableSpeechStream(audio_mode_t mode, status_t status_change) {

    bool sph_stream_support = mStreamManager->IsSphStrmSupport();
    ALOGD("%s mode = %d sph_stream_support = %d status_change = %d", __FUNCTION__, mode, sph_stream_support, status_change);
    if (sph_stream_support == true && status_change == NO_ERROR) {
        mStreamManager->EnableSphStrm(static_cast<audio_mode_t>(mode));
    }
    return NO_ERROR;
}
#endif

status_t AudioALSAHardware::setMode(int mode) {
    ALOGV("%s(), setMode = %d", __FUNCTION__, mode);
    status_t status = NO_ERROR;
    audio_mode_t new_mode = static_cast<audio_mode_t>(mode);

#ifdef MTK_SPEAKER_MONITOR_SPEECH_SUPPORT
    status_t status_change = CheckDisableSpeechStream(new_mode);
#endif

#if defined(MTK_AUDIO_DSP_RECOVERY_SUPPORT)
    add_audio_dsp_recovery_lock_cnt();
    LOCK_ALOCK_MS(get_adsp_recovery_lock(), MAX_RECOVERY_LOCK_TIMEOUT_MS);
    dec_audio_dsp_recovery_lock_cnt();

    mStreamManager->updateAudioModePolicy(new_mode); // always keep policy mode
    if (get_audio_dsp_recovery_mode()) {
        ALOGD("%s(), ADSP recovery, keep mode %d", __FUNCTION__, new_mode);
        status = NO_ERROR;
    } else {
        status = mStreamManager->setMode(new_mode);
    }
    UNLOCK_ALOCK(get_adsp_recovery_lock());
#else
    mStreamManager->updateAudioModePolicy(new_mode); // always keep policy mode
    status = mStreamManager->setMode(new_mode);
#endif

#ifdef MTK_SPEAKER_MONITOR_SPEECH_SUPPORT
    CheckEnableSpeechStream(new_mode, status_change);
#endif
    return status;
}

status_t AudioALSAHardware::setMicMute(bool state) {
    return mStreamManager->setMicMute(state);
}

status_t AudioALSAHardware::getMicMute(bool *state) {
    if (state != NULL) { *state = mStreamManager->getMicMute(); }
    return NO_ERROR;
}

#ifdef  MTK_HIFIAUDIO_SUPPORT
status_t AudioALSAHardware::setHiFiStateToXML(bool value) {
    AppOps *appOps = appOpsGetInstance();
    if (appOps != NULL) {
        appOps->utilNativeSetParam(
            SOUND_ENHANCEMENT_AUDIO_TYPE,
            SOUND_ENHANCEMENT_CATEGORY_PATH,
            SOUND_ENHANCEMENT_PARAM_HIFI_AUDIO,
            (value) ? "1" : "0");
        // save to file: SoundEnhancement_AudioParam.xml
        appOps->utilNativeSaveXml(SOUND_ENHANCEMENT_AUDIO_TYPE);

        ALOGD("%s(), Set HiFi state to xml = %d", __FUNCTION__, value);
    } else {
        ALOGE("%s(), Error: AppOps == NULL", __FUNCTION__);
        ASSERT(0);
        return BAD_VALUE;
    }
    return NO_ERROR;
}

int AudioALSAHardware::getHiFiStateFromXML() {
    AppOps *appOps = appOpsGetInstance();
    char *paramDataStr = NULL;
    int ret_value = 0;

    if (appOps != NULL) {
        paramDataStr = appOps->utilNativeGetParam(
                           SOUND_ENHANCEMENT_AUDIO_TYPE,
                           SOUND_ENHANCEMENT_CATEGORY_PATH,
                           SOUND_ENHANCEMENT_PARAM_HIFI_AUDIO);
        if (paramDataStr) {
            ALOGD("%s(), get HiFi state from xml = %s", __FUNCTION__, paramDataStr);
            ret_value = atoi(paramDataStr);
            free(paramDataStr);
        }
    } else {
        ALOGE("%s(), Error: AppOps == NULL", __FUNCTION__);
        ASSERT(0);
    }
    return ret_value;
}
#endif

status_t AudioALSAHardware::setParameters(const String8 &keyValuePairs) {
    ALOGD("+%s(): %s", __FUNCTION__, keyValuePairs.string());
    AudioParameter param = AudioParameter(keyValuePairs);

    /// parse key value pairs
    status_t status = NO_ERROR;
    int value = 0;
    float value_float = 0.0;
    String8 value_str;

#ifdef MTK_AUDIO_SMARTPASCP_SUPPORT
    if (strncmp(smartpa_aurisys_set_param_prefix,
                keyValuePairs.string(),
                strlen(smartpa_aurisys_set_param_prefix)) == 0) {
        /* set parameters to smartpa class to dsp */
        AudioSmartPaParam *pSmartpaParam =
            static_cast<AudioSmartPaParam *>(AudioSmartPaParam::getInstance());
        pSmartpaParam->setParameter(keyValuePairs.string());
        goto SET_PARAM_EXIT;
    }
#endif

#if defined(MTK_COMBO_MODEM_SUPPORT)
    if (strncmp(keySpeechParserSetParam,
                keyValuePairs.string(),
                strlen(keySpeechParserSetParam)) == 0) {
        status = SpeechDriverFactory::GetInstance()->GetSpeechDriver()->setParameter(keyValuePairs.string());
        goto SET_PARAM_EXIT;
    }
#endif

#ifdef MTK_AURISYS_FRAMEWORK_SUPPORT
    if (strncmp(aurisys_set_key_prefix,
                keyValuePairs.string(),
                strlen(aurisys_set_key_prefix)) == 0) {
        aurisys_set_parameter(keyValuePairs.string() + strlen(aurisys_set_key_prefix) + 1); // 1: ','
        goto SET_PARAM_EXIT;
    }

    // enable/disable aurisys
    if (param.getInt(keySET_AURISYS_ON, value) == NO_ERROR) {
        param.remove(keySET_AURISYS_ON);
        set_aurisys_on((bool)value);
    }

    if (param.get(keyGenerateVmByEpl, value_str) == NO_ERROR) {
        param.remove(keyGenerateVmByEpl);

        char vmFilePath[512];
        mAudioSpeechEnhanceInfoInstance->GetBesRecVMFileName(vmFilePath, 512);

        ASSERT(vmFilePath[0] != '\0');

        generateVmDumpByEpl(value_str.string(), vmFilePath);
    }
#endif


    if (param.getFloat(String8("setMasterVolume"), value_float) == NO_ERROR) {
        param.remove(String8("setMasterVolume"));
        mStreamManager->setMasterVolume(value_float);
    }

    //Tina temp for 6595 eccci fake
    if (param.getInt(String8("ECCCI_Test"), value) == NO_ERROR) {
        param.remove(String8("ECCCI_Test"));
        SpeechDriverFactory::GetInstance()->GetSpeechDriver()->SetEnh1DownlinkGain(value);
    }
#if defined(MTK_ANC_SUPPORT)
    if (param.getInt(keyANC_CMD, value) == NO_ERROR) {
        param.remove(keyANC_CMD);
        ALOGD("get ANC_CMD");
        mANCController->setCMD(value);
    }
    if (param.get(keyANC_UI, value_str) == NO_ERROR) {
        param.remove(keyANC_UI);
        mANCController->setANCSwitch(value_str == "on");
    }
#endif
    if (param.getInt(String8("TDM_Record"), value) == NO_ERROR) {
        param.remove(String8("TDM_Record"));
        switch (value) {
        case 0: {
            setTDMRecord(false);
            break;
        }
        case 1: {
            setTDMRecord(44100);
            break;
        }
        default: {
            setTDMRecord(value);
            break;
        }
        }
    }


    // TODO(Harvey): test code, remove it later
    if (param.getInt(String8("SET_MODE"), value) == NO_ERROR) {
        param.remove(String8("SET_MODE"));

        switch (value) {
        case 0: {
            mStreamManager->getStreamOut(0)->setParameters(String8("routing=2"));
            setMode(AUDIO_MODE_NORMAL);
            break;
        }
        case 1: {
            setMode(AUDIO_MODE_RINGTONE);
            break;
        }
        case 2: {
            setMode(AUDIO_MODE_IN_CALL);
            mStreamManager->getStreamOut(0)->setParameters(String8("routing=1"));
            break;
        }
        case 3: {
            setMode(AUDIO_MODE_IN_COMMUNICATION);
            break;
        }
        default: {
            break;
        }
        }
    }

#if defined(MTK_AUDIO_KS)
    if (param.getInt(String8("SET_BIT_TRUE"), value) == NO_ERROR) {
        param.remove(String8("SET_BIT_TRUE"));
        AudioBitTrueTest::getInstance()->setTestType(value);
    }
#endif

    if (param.getInt(keyAudioSampleRate, value) == NO_ERROR) {
        param.remove(keyAudioSampleRate);
        mStreamManager->setParametersToStreamOut(keyValuePairs);
    }

    if (param.getInt(keySET_BypassProcess, value) == NO_ERROR) {
        param.remove(keySET_BypassProcess);

        mStreamManager->setBypassDLProcess((bool)value);
    }

#ifdef MTK_HIFIAUDIO_SUPPORT
    if (param.getInt(keyHiFiState, value) == NO_ERROR) {
        param.remove(keyHiFiState);
        ALOGD("%s(), Set HiFi state = %d", __FUNCTION__, value);
#ifdef MTK_AUDIO_HIERARCHICAL_PARAM_SUPPORT
        // save to xml: SoundEnhancement_AudioParam.xml
        setHiFiStateToXML(value);
#endif
        // save to mHiFiState in AudioALSAHardwareResourceManager
        AudioALSAHardwareResourceManager::getInstance()->setHiFiStatus(value);
    }
#endif

    if (param.getInt(String8("HAHA"), value) == NO_ERROR) {
        param.remove(String8("HAHA"));

        AudioFtm *mAudioFtm = AudioFtm::getInstance();
        static LoopbackManager *pLoopbackManager = LoopbackManager::GetInstance();
        loopback_output_device_t loopback_output_device = LOOPBACK_OUTPUT_RECEIVER; // default use receiver here

        static AudioALSAHardwareResourceManager *mResourceManager = AudioALSAHardwareResourceManager::getInstance();

        switch (value) {
        case 0: {
            setMode(AUDIO_MODE_NORMAL);
            break;
        }
        case 1: {
            setMode(AUDIO_MODE_IN_CALL);
            mStreamManager->getStreamOut(0)->setParameters(String8("routing=1"));
            break;
        }
        case 2: {
            mAudioFtm->RecieverTest(false);
            break;
        }
        case 3: {
            mAudioFtm->RecieverTest(true);
            break;
        }
        case 4: {
            mAudioFtm->LouderSPKTest(false, false);
            break;
        }
        case 5: {
            mAudioFtm->LouderSPKTest(true, true);
            break;
        }
        case 6: {
            mAudioFtm->EarphoneTest(false);
            break;
        }
        case 7: {
            mAudioFtm->EarphoneTest(true);
            break;
        }
        case 8: {
            mAudioFtm->PhoneMic_Receiver_Loopback(MIC1_OFF);
            break;
        }
        case 9: {
            mAudioFtm->PhoneMic_Receiver_Loopback(MIC1_ON);
            break;
        }
        case 10: {
            mAudioFtm->PhoneMic_EarphoneLR_Loopback(MIC1_OFF);
            break;
        }
        case 11: {
            mAudioFtm->PhoneMic_EarphoneLR_Loopback(MIC1_ON);
            break;
        }
        case 12: {
            mAudioFtm->HeadsetMic_EarphoneLR_Loopback(false, false);
            break;
        }
        case 13: {
            mAudioFtm->HeadsetMic_EarphoneLR_Loopback(true, true);
            break;
        }
        case 14: {
            mAudioFtm->PhoneMic_EarphoneLR_Loopback(MIC2_OFF);
            break;
        }
        case 15: {
            mAudioFtm->PhoneMic_EarphoneLR_Loopback(MIC2_ON);
            break;
        }
        case 16: {
            mAudioFtm->PhoneMic_EarphoneLR_Loopback(MIC3_OFF);
            break;
        }
        case 17: {
            mAudioFtm->PhoneMic_EarphoneLR_Loopback(MIC3_ON);
            break;
        }
        case 18: {
            mAudioFtm->PhoneMic_Receiver_Loopback(MIC2_OFF);
            break;
        }
        case 19: {
            mAudioFtm->PhoneMic_Receiver_Loopback(MIC2_ON);
            break;
        }
        case 20: {
            mAudioFtm->PhoneMic_Receiver_Loopback(MIC3_OFF);
            break;
        }
        case 21: {
            mAudioFtm->PhoneMic_Receiver_Loopback(MIC3_ON);
            break;
        }
        case 22:
            // close single mic acoustic loopback
            pLoopbackManager->SetLoopbackOff();
            break;
        case 23:
            // open dual mic acoustic loopback (w/o DMNR)
            pLoopbackManager->SetLoopbackOn(MD_DUAL_MIC_ACOUSTIC_LOOPBACK_WITHOUT_DMNR, loopback_output_device);
            break;
        case 24:
            // close dual mic acoustic loopback
            pLoopbackManager->SetLoopbackOff();
            break;
        case 25:
            // open dual mic acoustic loopback (w/ DMNR)
            pLoopbackManager->SetLoopbackOn(MD_DUAL_MIC_ACOUSTIC_LOOPBACK_WITH_DMNR, loopback_output_device);
            break;
        case 30:
            // open AP BT loopback (w/ codec)
            pLoopbackManager->SetLoopbackOn(AP_BT_LOOPBACK, loopback_output_device);
            break;
        case 32:
            // open AP BT loopback (w/o codec)
            pLoopbackManager->SetLoopbackOn(AP_BT_LOOPBACK_NO_CODEC, loopback_output_device);
            break;
        case 33:
            mResourceManager->startOutputDevice(AUDIO_DEVICE_OUT_SPEAKER, 44100);
            break;
        case 34:
            mResourceManager->stopOutputDevice();
            break;
        default: {
            break;
        }
        }
    }

#if defined(MTK_AUDIODSP_SUPPORT)
    if (param.getInt(keyEarFeedbackInDSP, value) == NO_ERROR) {
        param.remove(keyEarFeedbackInDSP);
        AudioALSAHandlerKtv::getInstance()->enableKtvTask((bool)value);
    }
#endif

    //Modem Audio DVT and Debug
    if (param.get(keySpeechDVT_SampleRate, value_str) == NO_ERROR) {
        mSpeechPhoneCallController->setParam(keyValuePairs);
        param.remove(keySpeechDVT_SampleRate);
    }
    if (param.get(keySpeechDVT_MD_IDX, value_str) == NO_ERROR) {
        mSpeechPhoneCallController->setParam(keyValuePairs);
        param.remove(keySpeechDVT_MD_IDX);
    }
    if (param.getInt(keySpeechDVT_MD_LPBK, value) == NO_ERROR) {
        SpeechDriverInterface *pSpeechDriver = NULL;
        for (int modem_index = MODEM_1; modem_index < NUM_MODEM; modem_index++) {
            pSpeechDriver = SpeechDriverFactory::GetInstance()->GetSpeechDriverByIndex((modem_index_t)modem_index);
            if (pSpeechDriver != NULL) { // Might be single talk and some speech driver is NULL
                pSpeechDriver->SetModemLoopbackPoint((int32_t)value);
            }
        }
        param.remove(keySpeechDVT_MD_LPBK);
    }

    //AudioCmdHandlerService usage
    if (param.get(keySetCmd, value_str) == NO_ERROR) {
        param.remove(keySetCmd);
        size_t sz_in = value_str.size();
        size_t sz_out;//output buffer lenth
        size_t sz_dec;//decoded buffer length
        bool bEncode;

        // Decode string to char
        sz_out = Base64_OutputSize(false, sz_in);
        ALOGD("%s(), before decode (%s), sz_in(%zu), sz_out(%zu)", __FUNCTION__, value_str.string(), sz_in, sz_out);

        //allocate output buffer for decode
        unsigned char *buf_dec = new unsigned char[sz_out + 1];
        memset(buf_dec, 0, sz_out + 1);

        sz_dec = Base64_Decode(value_str.string(), buf_dec, sz_in);
        //error check and alert
        if (sz_dec == 0) {
            ALOGE("%s(), Decode Error!!!after decode (%s), sz_in(%zu), sz_out(%zu), sz_dec(%zu)", __FUNCTION__, buf_dec, sz_in, sz_out, sz_dec);
            if (buf_dec != NULL) {
                delete[] buf_dec;
                buf_dec = NULL;
            }
            status = BAD_VALUE;
        } else {
            ALOGD("%s(), after decode (%s), sz_in(%zu), sz_dec(%zu)", __FUNCTION__, buf_dec, sz_in, sz_dec);
            for (size_t indexCount = 0; indexCount < sz_dec; indexCount++) {
                ALOGV("%s(), buf_dec(0x%x)", __FUNCTION__, *(buf_dec + indexCount));
            }
            if (buf_dec != NULL) {
                int type_AudioData = *((int *) buf_dec);
                int par_AudioData = *(((int *) buf_dec) + 1);
                ALOGD("%s(), after decode (%s), type_AudioData(0x%x), par_AudioData(%d)", __FUNCTION__, buf_dec, type_AudioData, par_AudioData);
                SetAudioCommand(type_AudioData, par_AudioData);
                delete[] buf_dec;
                buf_dec = NULL;
            }
            status = NO_ERROR;
        }
        return status;
    }

    if (param.get(keySetBuffer, value_str) == NO_ERROR) {
        param.remove(keySetBuffer);
        size_t sz_in = value_str.size();
        size_t sz_out;//output buffer lenth
        size_t sz_dec;//decoded buffer length
        bool bEncode;
        int type_AudioData = 0, sz_AudioData = 0;

        // Decode string to char
        sz_out = Base64_OutputSize(false, sz_in);
        ALOGD("%s(), before decode (%s), sz_in(%zu), sz_out(%zu)", __FUNCTION__, value_str.string(), sz_in, sz_out);

        //allocate output buffer for decode
        unsigned char *buf_dec = new unsigned char[sz_out + 1];
        memset(buf_dec, 0, sz_out + 1);

        sz_dec = Base64_Decode(value_str.string(), buf_dec, sz_in);
        if (buf_dec != NULL) {
            //error check and alert
            if (sz_dec == 0) {
                ALOGE("%s(), Decode Error!!!after decode (%s), sz_in(%zu), sz_out(%zu), sz_dec(%zu)", __FUNCTION__, buf_dec, sz_in, sz_out, sz_dec);
                status = BAD_VALUE;
            } else {
                ALOGD("%s(), after decode (%s), sz_in(%zu), sz_dec(%zu)", __FUNCTION__, buf_dec, sz_in, sz_dec);
            }

            for (size_t indexCount = 0; indexCount < sz_dec; indexCount++) {
                ALOGV("%s(), buf_dec(0x%x)", __FUNCTION__, *(buf_dec + indexCount));
            }
            type_AudioData = *((int *) buf_dec);
            sz_AudioData = *(((int *) buf_dec) + 1);
            ALOGD("%s(), after decode (%s), type_AudioData(0x%x), sz_AudioData(%d)", __FUNCTION__, buf_dec, type_AudioData, sz_AudioData);
            SetAudioData(type_AudioData, sz_AudioData, buf_dec + 8);
            delete[] buf_dec;
        } else {
            ALOGE("%s(), buf_dec is NULL", __FUNCTION__);
        }
        status = NO_ERROR;
        return status;

    }

    // VT call (true) / Voice call (false)
    if (param.getInt(keySetVTSpeechCall, value) == NO_ERROR) {
        param.remove(keySetVTSpeechCall);
        mStreamManager->setVtNeedOn((bool)value);
    }

    // Set BGS DL Mute (true) / Unmute(false)
    if (param.getInt(keySet_BGS_DL_Mute, value) == NO_ERROR) {
        param.remove(keySet_BGS_DL_Mute);
        ALOGD("%s(), %s", __FUNCTION__, keyValuePairs.string());
        SpeechPcmMixerBGSPlayer::setPcmMixerDlMute((bool)value);
    }

    // Set BGS UL Mute (true) / Unmute(false)
    if (param.getInt(keySet_BGS_UL_Mute, value) == NO_ERROR) {
        param.remove(keySet_BGS_UL_Mute);
        ALOGD("%s(), %s", __FUNCTION__, keyValuePairs.string());
        SpeechPcmMixerBGSPlayer::setPcmMixerUlMute((bool)value);
    }

    // Mute BGS & DL Voice
    if (param.getInt(keyMuteBGS_DLVoice, value) == NO_ERROR) {
        param.remove(keyMuteBGS_DLVoice);
        mSpeechPhoneCallController->muteDlCodecForShutterSound(value != 0);
    }

    // Set SpeechCall DL Mute (true) / Unmute(false)
    if (param.getInt(keySet_SpeechCall_DL_Mute, value) == NO_ERROR) {
        param.remove(keySet_SpeechCall_DL_Mute);
        ALOGD("%s(), %s", __FUNCTION__, keyValuePairs.string());
        mSpeechPhoneCallController->setDlMute((bool)value);
    }

    // Set SpeechCall UL Mute (true) / Unmute(false)
    if (param.getInt(keySet_SpeechCall_UL_Mute, value) == NO_ERROR) {
        param.remove(keySet_SpeechCall_UL_Mute);
        ALOGD("%s(), %s", __FUNCTION__, keyValuePairs.string());
        mSpeechPhoneCallController->setUlMute((bool)value);
    }

    if (param.getInt(String8(AUDIO_PARAMETER_DEVICE_CONNECT), value) == NO_ERROR) {
        param.remove(String8(AUDIO_PARAMETER_DEVICE_CONNECT));
        audio_devices_t device = (audio_devices_t)value;

        mStreamManager->updateDeviceConnectionState(device, true);

        // Headphone Impedance detect when headset/headphone plug-in
        if (device == AUDIO_DEVICE_OUT_WIRED_HEADSET || device == AUDIO_DEVICE_OUT_WIRED_HEADPHONE) {
            AudioALSAHardwareResourceManager::getInstance()->HpImpeDanceDetect();
        }

        if (device & AUDIO_DEVICE_OUT_WIRED_HEADSET) {
            mStreamManager->DeviceNoneUpdate(); //For VOW switch device to Headset
#if defined(MTK_ANC_SUPPORT)
            if (AudioALSAHardwareResourceManager::getInstance()->getNumOfHeadsetPole() == 5) {
                mANCController->setFivePole(true);
            }
#endif
        }

        if (device == AUDIO_DEVICE_OUT_USB_DEVICE ||
            device == AUDIO_DEVICE_OUT_USB_HEADSET) {
#ifdef MTK_USB_PHONECALL
            int usb_card, usb_device;
            if (param.getInt(String8("card"), usb_card) != NO_ERROR ||
                param.getInt(String8("device"), usb_device) != NO_ERROR) {
                usb_card = -1;
                usb_device = -1;
            }
            AudioUSBPhoneCallController::getInstance()->setUSBOutConnectionState(device, true, usb_card, usb_device);
#endif
        } else if (device == AUDIO_DEVICE_IN_USB_DEVICE ||
                   device == AUDIO_DEVICE_IN_USB_HEADSET ||
                   device == AUDIO_DEVICE_IN_BUS) {
#ifdef MTK_USB_PHONECALL
            int usb_card, usb_device;
            if (param.getInt(String8("card"), usb_card) != NO_ERROR ||
                param.getInt(String8("device"), usb_device) != NO_ERROR) {
                usb_card = -1;
                usb_device = -1;
            }
            AudioUSBPhoneCallController::getInstance()->setUSBInConnectionState(device, true, usb_card, usb_device);
#endif
        }
        param.remove(String8("card"));
        param.remove(String8("device"));
    }

    if (param.getInt(String8(AUDIO_PARAMETER_DEVICE_DISCONNECT), value) == NO_ERROR) {
        param.remove(String8(AUDIO_PARAMETER_DEVICE_DISCONNECT));
        audio_devices_t device = (audio_devices_t)value;

        mStreamManager->updateDeviceConnectionState(device, false);

        if (device & AUDIO_DEVICE_OUT_WIRED_HEADSET) {
            mStreamManager->DeviceNoneUpdate(); //For VOW switch device to Main Mic
#if defined(MTK_ANC_SUPPORT)
            if (AudioALSAHardwareResourceManager::getInstance()->getNumOfHeadsetPole() == 5) {
                mANCController->setFivePole(false);
            }
#endif
        }

        if (device == AUDIO_DEVICE_OUT_USB_DEVICE ||
            device == AUDIO_DEVICE_OUT_USB_HEADSET) {
#ifdef MTK_USB_PHONECALL
            AudioUSBPhoneCallController::getInstance()->setUSBOutConnectionState(device, false, -1, -1);
#endif
        } else if (device == AUDIO_DEVICE_IN_USB_DEVICE ||
                   device == AUDIO_DEVICE_IN_USB_HEADSET ||
                   device == AUDIO_DEVICE_IN_BUS) {
#ifdef MTK_USB_PHONECALL
            AudioUSBPhoneCallController::getInstance()->setUSBInConnectionState(device, false, -1, -1);
#endif
        }
        param.remove(String8("card"));
        param.remove(String8("device"));
    }

    // FM enable
    if (param.getInt(keySetFmEnable, value) == NO_ERROR) {
        param.remove(keySetFmEnable);
        if (mUseAudioPatchForFm == false) {
            mStreamManager->setFmEnable((bool)value);
        }
    }

    // Set FM volume
    if (param.getFloat(keySetFmVolume, value_float) == NO_ERROR) {
        param.remove(keySetFmVolume);
        if (mUseAudioPatchForFm == false) {
            mStreamManager->setFmVolume(value_float);
        }
    }

    // Set FM Tx enable
    if (param.getInt(keySetFmTxEnable, value) == NO_ERROR) {
        param.remove(keySetFmTxEnable);
        mFmTxEnable = (bool)value;
    }

    // Force dusable FM Tx due to FM Rx is ready to play
    if (param.getInt(keyFMRXForceDisableFMTX, value) == NO_ERROR) {
        param.remove(keyFMRXForceDisableFMTX);
        if (value == true) {
            mFmTxEnable = false;
        }
    }

#if !defined(MTK_AURISYS_FRAMEWORK_SUPPORT)
    if (param.getInt(keyMusicPlusSet, value) == NO_ERROR) {
        param.remove(keyMusicPlusSet);
        mStreamManager->SetMusicPlusStatus(value ? true : false);
    }

    // ACF/HCF parameters calibration
    if (param.getInt(keyACFHCF_Update, value) == NO_ERROR) {
        param.remove(keyACFHCF_Update);
        mStreamManager->UpdateACFHCF(value);
    }
#endif

    // set the LR channel switch
    if (param.getInt(keyLR_ChannelSwitch, value) == NO_ERROR) {
        ALOGD("keyLR_ChannelSwitch=%d", value);
        bool bIsLRSwitch = value;
        AudioALSAHardwareResourceManager::getInstance()->setMicInverse(bIsLRSwitch);
        param.remove(keyLR_ChannelSwitch);
    }

    // BesRecord Mode setting
    if (param.getInt(keyHDREC_SET_VOICE_MODE, value) == NO_ERROR) {
        ALOGD("HDREC_SET_VOICE_MODE=%d", value); // Normal, Indoor, Outdoor,
        param.remove(keyHDREC_SET_VOICE_MODE);
        //Get and Check Voice/Video Mode Offset
        AUDIO_HD_RECORD_SCENE_TABLE_STRUCT hdRecordSceneTable;
        mAudioCustParamClient->GetHdRecordSceneTableFromNV(&hdRecordSceneTable);
        if (value < hdRecordSceneTable.num_voice_rec_scenes) {
            int32_t BesRecScene = value + 1;//1:cts verifier offset
            mAudioSpeechEnhanceInfoInstance->SetBesRecScene(BesRecScene);
        } else {
            ALOGE("HDREC_SET_VOICE_MODE=%d exceed max value(%d)\n", value, hdRecordSceneTable.num_voice_rec_scenes);
        }
    }

    if (param.getInt(keyHDREC_SET_VIDEO_MODE, value) == NO_ERROR) {
        ALOGD("HDREC_SET_VIDEO_MODE=%d", value); // Normal, Indoor, Outdoor,
        param.remove(keyHDREC_SET_VIDEO_MODE);
        //Get and Check Voice/Video Mode Offset
        AUDIO_HD_RECORD_SCENE_TABLE_STRUCT hdRecordSceneTable;
        mAudioCustParamClient->GetHdRecordSceneTableFromNV(&hdRecordSceneTable);
        if (value < hdRecordSceneTable.num_video_rec_scenes) {
            uint32_t offset = hdRecordSceneTable.num_voice_rec_scenes + 1;//1:cts verifier offset
            int32_t BesRecScene = value + offset;
            mAudioSpeechEnhanceInfoInstance->SetBesRecScene(BesRecScene);
        } else {
            ALOGE("HDREC_SET_VIDEO_MODE=%d exceed max value(%d)\n", value, hdRecordSceneTable.num_video_rec_scenes);
        }
    }

    // BT NREC on/off
    if (param.get(keyBtHeadsetNrec, value_str) == NO_ERROR) {
        if (value_str == "on") {
            mStreamManager->SetBtHeadsetNrec(true);
        } else if (value_str == "off") {
            mStreamManager->SetBtHeadsetNrec(false);
        }
        param.remove(keyBtHeadsetNrec);
    }

    if (param.get(keyBtscoOnOff, value_str) == NO_ERROR) {
        struct mixer *mixer = AudioALSADriverUtility::getInstance()->getMixer();
        mBtscoOn = (value_str == "off") ? false : true;
        int bypass = mBtscoOn ? 0 : 1;

        if (mixer_ctl_set_value(mixer_get_ctl_by_name(mixer, "BTCVSD Bypass Switch"), 0, bypass)) {
            ALOGE("Error: BTCVSD Bypass Switch invalid value");
        }

        param.remove(keyBtscoOnOff);
    }

    if (param.get(keyBtHeadsetName, value_str) == NO_ERROR) {
        mStreamManager->SetBtHeadsetName(value_str.string());
        SpeechDriverFactory::GetInstance()->GetSpeechDriver()->setBtHeadsetName(value_str.string());
        param.remove(keyBtHeadsetName);
    }

    if (param.get(keySetBTMode, value_str) == NO_ERROR) {
        ALOGD("%s(), setBTMode = %s", __FUNCTION__, value_str.string());
        if (value_str == "on") {
            set_uint32_to_mixctrl(PROPERTY_KEY_BTSCO_WB_ON, 1);
            WCNChipController::GetInstance()->SetBTCurrentSamplingRateNumber(16000);
            AudioBTCVSDControl::getInstance()->BT_SCO_SetMode(true);
            mSpeechPhoneCallController->setBTMode(true);
        } else if (value_str == "off") {
            set_uint32_to_mixctrl(PROPERTY_KEY_BTSCO_WB_ON, 0);
            WCNChipController::GetInstance()->SetBTCurrentSamplingRateNumber(8000);
            AudioBTCVSDControl::getInstance()->BT_SCO_SetMode(false);
            mSpeechPhoneCallController->setBTMode(false);
        }
        param.remove(keySetBTMode);
    }

    if (param.get(keySetAudioCustomScene, value_str) == NO_ERROR) {
        ALOGD("%s(), setCustScene = %s", __FUNCTION__, value_str.string());
        mStreamManager->setCustScene(value_str);
        param.remove(keySetAudioCustomScene);
    }

    //<---for audio tool(speech/ACF/HCF/DMNR/HD/Audiotaste calibration)
    // calibrate speech parameters
    if (param.getInt(keySpeechParams_Update, value) == NO_ERROR) {
        ALOGD("setParameters Update Speech Parames");
#if !defined(MTK_AUDIO_HIERARCHICAL_PARAM_SUPPORT)
        mStreamManager->updateSpeechNVRAMParam(value);
#endif
        param.remove(keySpeechParams_Update);
    }

    if (AudioALSAHardwareResourceManager::getInstance()->getNumPhoneMicSupport() >= 2) {
        if (param.getInt(keyDualMicParams_Update, value) == NO_ERROR) {
            param.remove(keyDualMicParams_Update);
            mStreamManager->UpdateDualMicParams();
        }

#if defined(MTK_MAGICONFERENCE_SUPPORT)
        if (param.getInt(keyMagiConParams_Update, value) == NO_ERROR) {
            param.remove(keyMagiConParams_Update);
            mStreamManager->UpdateMagiConParams();
        }
#endif
    }

#if defined(MTK_HAC_SUPPORT)
    if (param.getInt(keyHACParams_Update, value) == NO_ERROR) {
        param.remove(keyHACParams_Update);
        mStreamManager->UpdateHACParams();
    }
#endif

    if (param.get(keyRefreshModem, value_str) == NO_ERROR) {
        mStreamManager->phoneCallRefreshModem(value_str.string());
        param.remove(keyRefreshModem);
    }
    if (param.getInt(keyForegroundPhoneId, value) == NO_ERROR) {
        mStreamManager->phoneCallUpdatePhoneId((phone_id_t)value);
        param.remove(keyForegroundPhoneId);
    }
    if (param.get(keyPhone1Modem, value_str) == NO_ERROR) {
        mSpeechPhoneCallController->setParam(keyValuePairs);
        param.remove(keyPhone1Modem);
    }
    if (param.get(keyPhone2Modem, value_str) == NO_ERROR) {
        mSpeechPhoneCallController->setParam(keyValuePairs);
        param.remove(keyPhone2Modem);
    }

#ifndef MTK_AURISYS_FRAMEWORK_SUPPORT
    if (param.getInt(keyBesRecordParams_Update, value) == NO_ERROR) {
        param.remove(keyBesRecordParams_Update);
        mAudioSpeechEnhanceInfoInstance->UpdateBesRecordParams();
    }
#endif

    // calibrate speech volume
    if (param.getInt(keySpeechVolume_Update, value) == NO_ERROR) {
        ALOGD("setParameters Update Speech volume");
        mStreamManager->UpdateSpeechVolume();
        param.remove(keySpeechVolume_Update);
    }

    if (param.getInt(keyBesLoudnessSet, value) == NO_ERROR) {
        param.remove(keyBesLoudnessSet);
        mStreamManager->SetBesLoudnessStatus(value ? true : false);
#if defined(MTK_AURISYS_FRAMEWORK_SUPPORT)
#if defined(MTK_AUDIODSP_SUPPORT)
        char strdspbuffer [100];
        snprintf(strdspbuffer, sizeof(strdspbuffer),
                 "AURISYS_SET_PARAM,DSP,PLAYBACK,MTKBESSOUND,KEY_VALUE,SetBesLoudnessStatus,%d=SET", value);
        aurisys_set_parameter(strdspbuffer + strlen(aurisys_set_key_prefix) + 1);
#endif
        char strbuffer [100];
        snprintf(strbuffer, sizeof(strbuffer),
                 "AURISYS_SET_PARAM,HAL,ALL,MTKBESSOUND,KEY_VALUE,SetBesLoudnessStatus,%d=SET", value);
        aurisys_set_parameter(strbuffer + strlen(aurisys_set_key_prefix) + 1); // 1: ','
#endif
    }

    // HD recording and DMNR calibration
#if !defined(MTK_AUDIO_HD_REC_SUPPORT)
    if (AudioALSAHardwareResourceManager::getInstance()->getNumPhoneMicSupport() >= 2)
#endif
    {
        if (param.getInt(keyDualMicRecPly, value) == NO_ERROR) {
            unsigned short cmdType = value & 0x000F;
            bool bWB = (value >> 4) & 0x000F;
            status_t ret = NO_ERROR;
            //dmnr tunning at ap side
            switch (cmdType) {
            case DUAL_MIC_REC_PLAY_STOP:
                ret = mAudioALSAParamTunerInstance->enableDMNRAtApSide(false, bWB, OUTPUT_DEVICE_RECEIVER, RECPLAY_MODE);
                mAudioSpeechEnhanceInfoInstance->SetAPDMNRTuningEnable(false);
                break;
            case DUAL_MIC_REC:
                mAudioSpeechEnhanceInfoInstance->SetAPDMNRTuningEnable(true);
                ret = mAudioALSAParamTunerInstance->enableDMNRAtApSide(true, bWB, OUTPUT_DEVICE_RECEIVER, RECONLY_MODE);
                break;
            case DUAL_MIC_REC_PLAY:
                mAudioSpeechEnhanceInfoInstance->SetAPDMNRTuningEnable(true);
                ret = mAudioALSAParamTunerInstance->enableDMNRAtApSide(true, bWB, OUTPUT_DEVICE_RECEIVER, RECPLAY_MODE);
                break;
            case DUAL_MIC_REC_PLAY_HS:
                mAudioSpeechEnhanceInfoInstance->SetAPDMNRTuningEnable(true);
                ret = mAudioALSAParamTunerInstance->enableDMNRAtApSide(true, bWB, OUTPUT_DEVICE_HEADSET, RECPLAY_MODE);
                break;
            case DUAL_MIC_REC_HF:
                mAudioSpeechEnhanceInfoInstance->SetAPDMNRTuningEnable(true);
                ret = mAudioALSAParamTunerInstance->enableDMNRAtApSide(true, bWB, OUTPUT_DEVICE_RECEIVER, RECONLY_HF_MODE);
                break;
            case DUAL_MIC_REC_PLAY_HF:
                mAudioSpeechEnhanceInfoInstance->SetAPDMNRTuningEnable(true);
                ret = mAudioALSAParamTunerInstance->enableDMNRAtApSide(true, bWB, OUTPUT_DEVICE_RECEIVER, RECPLAY_HF_MODE);
                break;
            case DUAL_MIC_REC_PLAY_HF_HS:
                mAudioSpeechEnhanceInfoInstance->SetAPDMNRTuningEnable(true);
                ret = mAudioALSAParamTunerInstance->enableDMNRAtApSide(true, bWB, OUTPUT_DEVICE_HEADSET, RECPLAY_HF_MODE);
                break;
            case FIR_REC:
                ret = mAudioALSAParamTunerInstance->enableFIRRecord(true);
                break;
            case FIR_REC_STOP:
                ret = mAudioALSAParamTunerInstance->enableFIRRecord(false);
                break;
            default:
                ret = BAD_VALUE;
                break;
            }
            if (ret == NO_ERROR) {
                param.remove(keyDualMicRecPly);
            }
        }

        if (param.get(keyDUALMIC_IN_FILE_NAME, value_str) == NO_ERROR) {
            if (mAudioALSAParamTunerInstance->setPlaybackFileName(value_str.string()) == NO_ERROR) {
                param.remove(keyDUALMIC_IN_FILE_NAME);
            }
        }

        if (param.get(keyDUALMIC_OUT_FILE_NAME, value_str) == NO_ERROR) {
            if (mAudioALSAParamTunerInstance->setRecordFileName(value_str.string()) == NO_ERROR) {
#ifndef DMNR_TUNNING_AT_MODEMSIDE
                if (mAudioSpeechEnhanceInfoInstance->SetBesRecVMFileName(value_str.string()) == NO_ERROR)
#endif
                    param.remove(keyDUALMIC_OUT_FILE_NAME);
            }
        }

        if (param.getInt(keyDUALMIC_SET_UL_GAIN, value) == NO_ERROR) {
            if (mAudioALSAParamTunerInstance->setDMNRGain(AUD_MIC_GAIN, value) == NO_ERROR) {
                param.remove(keyDUALMIC_SET_UL_GAIN);
            }
        }

        if (param.getInt(keyDUALMIC_SET_DL_GAIN, value) == NO_ERROR) {
            if (mAudioALSAParamTunerInstance->setDMNRGain(AUD_RECEIVER_GAIN, value) == NO_ERROR) {
                param.remove(keyDUALMIC_SET_DL_GAIN);
            }
        }

        if (param.getInt(keyDUALMIC_SET_HSDL_GAIN, value) == NO_ERROR) {
            if (mAudioALSAParamTunerInstance->setDMNRGain(AUD_HS_GAIN, value) == NO_ERROR) {
                param.remove(keyDUALMIC_SET_HSDL_GAIN);
            }
        }

        if (param.getInt(keyDUALMIC_SET_UL_GAIN_HF, value) == NO_ERROR) {
            if (mAudioALSAParamTunerInstance->setDMNRGain(AUD_MIC_GAIN_HF, value) == NO_ERROR) {
                param.remove(keyDUALMIC_SET_UL_GAIN_HF);
            }
        }
    }

    if (param.getInt(keyHDRecTunningEnable, value) == NO_ERROR) {
        ALOGD("keyHDRecTunningEnable=%d", value);
        bool bEnable = value;
        mAudioSpeechEnhanceInfoInstance->SetBesRecTuningEnable(bEnable);
        param.remove(keyHDRecTunningEnable);
    }

    if (param.get(keyHDRecVMFileName, value_str) == NO_ERROR) {
        ALOGD("keyHDRecVMFileName=%s", value_str.string());
        if (mAudioSpeechEnhanceInfoInstance->SetBesRecVMFileName(value_str.string()) == NO_ERROR) {
            param.remove(keyHDRecVMFileName);
        }
    }
    // --->for audio tool(speech/ACF/HCF/DMNR/HD/Audiotaste calibration)

    if (AudioALSAHardwareResourceManager::getInstance()->getNumPhoneMicSupport() >= 2) {
        // Dual Mic Noise Reduction, DMNR for Receiver
        if (param.getInt(keyEnable_Dual_Mic_Setting, value) == NO_ERROR) {
            param.remove(keyEnable_Dual_Mic_Setting);
            mStreamManager->Enable_DualMicSettng(SPH_ENH_DYNAMIC_MASK_DMNR, (bool) value);
        }

        // Dual Mic Noise Reduction, DMNR for Loud Speaker
        if (param.getInt(keySET_LSPK_DMNR_ENABLE, value) == NO_ERROR) {
            param.remove(keySET_LSPK_DMNR_ENABLE);
            mStreamManager->Set_LSPK_DlMNR_Enable(SPH_ENH_DYNAMIC_MASK_LSPK_DMNR, (bool) value);

        }

        // VoIP Dual Mic Noise Reduction, DMNR for Receiver
        if (param.getInt(keySET_VOIP_RECEIVER_DMNR_ENABLE, value) == NO_ERROR) {
            param.remove(keySET_VOIP_RECEIVER_DMNR_ENABLE);
            mAudioSpeechEnhanceInfoInstance->SetDynamicVoIPSpeechEnhancementMask(VOIP_SPH_ENH_DYNAMIC_MASK_DMNR, (bool)value);
        }

        // VoIP Dual Mic Noise Reduction, DMNR for Loud Speaker
        if (param.getInt(keySET_VOIP_LSPK_DMNR_ENABLE, value) == NO_ERROR) {
            param.remove(keySET_VOIP_LSPK_DMNR_ENABLE);
            mAudioSpeechEnhanceInfoInstance->SetDynamicVoIPSpeechEnhancementMask(VOIP_SPH_ENH_DYNAMIC_MASK_LSPK_DMNR, (bool)value);
        }
    }

    // Voice Clarity Engine, VCE
    if (param.getInt(keySET_VCE_ENABLE, value) == NO_ERROR) {
        param.remove(keySET_VCE_ENABLE);
        mStreamManager->SetVCEEnable((bool) value);
    }

#if defined(MTK_MAGICONFERENCE_SUPPORT)
    if (AudioALSAHardwareResourceManager::getInstance()->getNumPhoneMicSupport() >= 2) {
        // Magic Conference Call
        if (param.getInt(keySET_MAGI_CONFERENCE_ENABLE, value) == NO_ERROR) {
            param.remove(keySET_MAGI_CONFERENCE_ENABLE);
            mStreamManager->SetMagiConCallEnable((bool) value);
        }
    }
#endif

#if defined(MTK_SPEECH_ENCRYPTION_SUPPORT)
    // SET_SPEECH_DATA_ENCRYPT_CONFIG
    if (param.getInt(keySET_SPEECH_DATA_ENCRYPT_CONFIG, value) == NO_ERROR) {
        param.remove(keySET_SPEECH_DATA_ENCRYPT_CONFIG);
        SpeechDataEncrypter::GetInstance()->SetEnableStatus((bool) value);
    }
#endif

    // HAC
    if (param.get(keyHAC_SETTING, value_str) == NO_ERROR) {
        param.remove(keyHAC_SETTING);
#if defined(MTK_HAC_SUPPORT)
        if (value_str == "ON") {
            mStreamManager->SetHACEnable(true);
        } else if (value_str == "OFF") {
            mStreamManager->SetHACEnable(false);
        }
#else
        status = -ENOSYS;
#endif
    }

#if defined(MTK_AUDIO_HIERARCHICAL_PARAM_SUPPORT)
    // VM Log
    if (param.getInt(keySET_VMLOG_CONFIG, value) == NO_ERROR) {
        param.remove(keySET_VMLOG_CONFIG);

        SpeechVMRecorder::getInstance()->configVm((uint8_t)value);
    }

    if (param.getInt(keySET_SUPER_VOLUME_ENABLE, value) == NO_ERROR) {
        param.remove(keySET_SUPER_VOLUME_ENABLE);
        mSpeechPhoneCallController->updateSpeechFeature(SPEECH_FEATURE_SUPERVOLUME, (bool) value);
    }
#endif

#if !defined(MTK_AURISYS_FRAMEWORK_SUPPORT)
    if (IsAudioSupportFeature(AUDIO_SUPPORT_VIBRATION_SPEAKER)) {
        if (param.getInt(keySET_VIBSPK_ENABLE, value) == NO_ERROR) {
            param.remove(keySET_VIBSPK_ENABLE);
            AudioVIBSPKControl::getInstance()->setVibSpkEnable((bool)value);
            ALOGD("setParameters VibSPK!!, %x", value);
        }
        if (param.getInt(keySET_VIBSPK_RAMPDOWN, value) == NO_ERROR) {
            AudioFtmBase *mAudioFtm = AudioFtmBase::createAudioFtmInstance();
            param.remove(keySET_VIBSPK_RAMPDOWN);
            mAudioFtm->SetVibSpkRampControl(value);
            ALOGD("setParameters VibSPK_Rampdown!!, %x", value);
        }
    }
#endif

#ifdef MTK_VOW_SUPPORT
    // Voice Wake Up
    if (param.getInt(keyMTK_VOW_ENABLE, value) == NO_ERROR) {
        param.remove(keyMTK_VOW_ENABLE);
        mStreamManager->setVoiceWakeUpNeedOn((bool)value);
    }
    if (param.getInt(keyMTK_VOW_TRAINING, value) == NO_ERROR) {
        param.remove(keyMTK_VOW_TRAINING);
        AudioALSAVoiceWakeUpController::getInstance()->setBargeInBypass((bool)value);
    }
#endif

    // Set TTY mode
    if (param.get(keySetTtyMode, value_str) == NO_ERROR) {
        param.remove(keySetTtyMode);
#if defined(MTK_TTY_SUPPORT)
        TtyModeType ttyMode;

        if (value_str == "tty_full") {
            ttyMode = AUD_TTY_FULL;
        } else if (value_str == "tty_vco") {
            ttyMode = AUD_TTY_VCO;
        } else if (value_str == "tty_hco") {
            ttyMode = AUD_TTY_HCO;
        } else if (value_str == "tty_off") {
            ttyMode = AUD_TTY_OFF;
        } else {
            ALOGD("setParameters tty_mode error !!");
            ttyMode = AUD_TTY_ERR;
        }
        mSpeechPhoneCallController->setTtyMode(ttyMode);
#else
        status = -ENOSYS;
#endif
    }

#if defined(MTK_RTT_SUPPORT)
    // Set call type for RTT/3GTTY
    if (param.getInt(keySetRttCallType, value) == NO_ERROR) {
        param.remove(keySetRttCallType);
        mSpeechPhoneCallController->setRttCallType(value);
    }
#endif

    // Low latency mode
    if (param.get(keySCREEN_STATE, value_str) == NO_ERROR) {
        param.remove(keySCREEN_STATE);
        //Diable low latency mode for 32bits audio task
        setScreenState(value_str == "on");
    }

    // Loopback use speaker or not
    static bool bForceUseLoudSpeakerInsteadOfReceiver = false;
    if (param.getInt(keySET_LOOPBACK_USE_LOUD_SPEAKER, value) == NO_ERROR) {
        param.remove(keySET_LOOPBACK_USE_LOUD_SPEAKER);
        bForceUseLoudSpeakerInsteadOfReceiver = value & 0x1;
    }

    // Assign delay frame for modem loopback // 1 frame = 20ms
    if (param.getInt(keySET_LOOPBACK_MODEM_DELAY_FRAMES, value) == NO_ERROR) {
        param.remove(keySET_LOOPBACK_MODEM_DELAY_FRAMES);
        SpeechDriverInterface *pSpeechDriver = NULL;
        for (int modem_index = MODEM_1; modem_index < NUM_MODEM; modem_index++) {
            pSpeechDriver = SpeechDriverFactory::GetInstance()->GetSpeechDriverByIndex((modem_index_t)modem_index);
            if (pSpeechDriver != NULL) { // Might be single talk and some speech driver is NULL
                pSpeechDriver->SetAcousticLoopbackDelayFrames((int32_t)value);
            }
        }
    }

    // Loopback
    if (param.get(keySET_LOOPBACK_TYPE, value_str) == NO_ERROR) {
        param.remove(keySET_LOOPBACK_TYPE);

        // parse format like "SET_LOOPBACK_TYPE=1" / "SET_LOOPBACK_TYPE=1+0"
        int type_value = NO_LOOPBACK;
        int device_value = -1;
        sscanf(value_str.string(), "%d,%d", &type_value, &device_value);
        ALOGV("type_value = %d, device_value = %d", type_value, device_value);

        const loopback_t loopback_type = (loopback_t)type_value;
        loopback_output_device_t loopback_output_device;

        if (loopback_type == NO_LOOPBACK) { // close loopback
            LoopbackManager::GetInstance()->SetLoopbackOff();
        } else { // open loopback
            if (device_value == LOOPBACK_OUTPUT_RECEIVER ||
                device_value == LOOPBACK_OUTPUT_EARPHONE ||
                device_value == LOOPBACK_OUTPUT_SPEAKER) { // assign output device
                loopback_output_device = (loopback_output_device_t)device_value;
            } else { // not assign output device
                if (mStreamManager->getDeviceConnectionState(AUDIO_DEVICE_OUT_WIRED_HEADSET) == true ||
                    mStreamManager->getDeviceConnectionState(AUDIO_DEVICE_OUT_WIRED_HEADPHONE) == true) {
                    loopback_output_device = LOOPBACK_OUTPUT_EARPHONE;
                } else if (bForceUseLoudSpeakerInsteadOfReceiver == true) {
                    loopback_output_device = LOOPBACK_OUTPUT_SPEAKER;
                } else {
                    loopback_output_device = LOOPBACK_OUTPUT_RECEIVER;
                }
            }
            LoopbackManager::GetInstance()->SetLoopbackOn(loopback_type, loopback_output_device);
        }
    }

#ifdef MTK_AUDIO_GAIN_TABLE
    // Set ACS volume
    if (param.get(keySET_AT_ACSVolume, value_str) == NO_ERROR) {
        param.remove(keySET_AT_ACSVolume);

        sscanf(value_str.string(), "%d", &value);
        mAudioALSAVolumeController->setAnalogVolume(0, AudioALSAHardwareResourceManager::getInstance()->getOutputDevice(), value, (audio_mode_t)AUDIO_MODE_IN_CALL);
    }

    // TC1 AT%ACS
    if (param.get(keySET_AT_ACS, value_str) == NO_ERROR) {
        param.remove(keySET_AT_ACS);

        int acs_value = -1;
        sscanf(value_str.string(), "%d", &acs_value);
        uint32_t current_device;

        switch (acs_value) {
        case 0:  // turn off loopback
            LoopbackManager::GetInstance()->SetLoopbackOff();
            break;
        case 1:   // turn on loopback by current device status
            current_device = AudioALSAHardwareResourceManager::getInstance()->getOutputDevice();
            if (current_device == AUDIO_DEVICE_OUT_WIRED_HEADSET) {
                LoopbackManager::GetInstance()->SetLoopbackOn(MD_HEADSET_MIC_ACOUSTIC_LOOPBACK, LOOPBACK_OUTPUT_EARPHONE);
            } else if (current_device == AUDIO_DEVICE_OUT_WIRED_HEADPHONE) {
                LoopbackManager::GetInstance()->SetLoopbackOn(MD_MAIN_MIC_ACOUSTIC_LOOPBACK, LOOPBACK_OUTPUT_EARPHONE);
            } else {
                LoopbackManager::GetInstance()->SetLoopbackOn(MD_MAIN_MIC_ACOUSTIC_LOOPBACK, LOOPBACK_OUTPUT_RECEIVER);
            }
            break;
        case 2:
            LoopbackManager::GetInstance()->SetLoopbackOn(MD_HEADSET_MIC_ACOUSTIC_LOOPBACK, LOOPBACK_OUTPUT_EARPHONE);
            break;
        case 3:
            LoopbackManager::GetInstance()->SetLoopbackOn(MD_MAIN_MIC_ACOUSTIC_LOOPBACK, LOOPBACK_OUTPUT_SPEAKER);
            break;
        case 5:
            AudioALSAHardwareResourceManager::getInstance()->changeOutputDevice(AUDIO_DEVICE_OUT_WIRED_HEADSET);
            break;
        case 6:
            AudioALSAHardwareResourceManager::getInstance()->changeOutputDevice(AUDIO_DEVICE_OUT_EARPIECE);
            break;
        case 7:
            AudioALSAHardwareResourceManager::getInstance()->changeOutputDevice(AUDIO_DEVICE_OUT_SPEAKER);
            break;
        }
    }

    if (param.getInt(keyGainTable_Update, value) == NO_ERROR) {
        ALOGD("setParameters Update Gain Table");
        mAudioALSAVolumeController->initVolumeController();
        param.remove(keyGainTable_Update);
    }
    if (param.getInt(keyMicGain_Update, value) == NO_ERROR) {
        ALOGD("setParameters Update Mic Gain");
        //mAudioResourceManager->SetInputDeviceGain();
        mStreamManager->SetCaptureGain();
        param.remove(keyMicGain_Update);
    }

    //BT VGS feature +
    if (param.get(keyBtSupportVolume, value_str) == NO_ERROR) {
        param.remove(keyBtSupportVolume);
        if (value_str == "on") {
            mAudioALSAVolumeController->setBtVolumeCapability(true);
        } else if (value_str == "off") {
            mAudioALSAVolumeController->setBtVolumeCapability(false);
        }
    }
#endif

    if (param.getInt(keySetFlightMode, value) == NO_ERROR) {
        param.remove(keySetFlightMode);
        if (value == 1) {
            ALOGD("flignt mode=1");
        } else if (value == 0) {
            ALOGD("flight mode=0");
        }
    }

    if (param.get(keySetDSM, value_str) == NO_ERROR) {
        //mStreamManager->getStreamOut(0)->setParameters(keyValuePairs);
        mStreamManager->setParametersToStreamOut(keyValuePairs);
        param.remove(keySetDSM);
    }
#ifdef MTK_AUDIO_TUNNELING_SUPPORT
    if (param.get(keySetMP3PCM_DUMP, value_str) == NO_ERROR) {
        mStreamManager->setParametersToStreamOut(keyValuePairs);
        param.remove(keySetMP3PCM_DUMP);
    }
    if (param.get(kKeySampleRate, value_str) == NO_ERROR) {
        ALOGD("%s(), kKeySampleRate", __FUNCTION__);
        mStreamManager->setParametersToStreamOut(keyValuePairs);
        param.remove(kKeySampleRate);
    }
    if (param.get(kKeyBitRate, value_str) == NO_ERROR) {
        ALOGD("%s(), kKeyBitRate", __FUNCTION__);
        mStreamManager->setParametersToStreamOut(keyValuePairs);
        param.remove(kKeyBitRate);
    }
#endif
    if (param.getInt(keyNUM_HEADSET_POLE, value) == NO_ERROR) {
        param.remove(keyNUM_HEADSET_POLE);
        AudioALSAHardwareResourceManager::getInstance()->setNumOfHeadsetPole(value);
    }

#if defined(MTK_HYBRID_NLE_SUPPORT)
    if (param.getInt(keyHYBRID_NLE_BYPASS, value) == NO_ERROR) {
        param.remove(keyHYBRID_NLE_BYPASS);
        AudioALSAHyBridNLEManager::getInstance()->setBypassNLE(value);
    }

    if (param.getInt(keyHYBRID_NLE_DUMP, value) == NO_ERROR) {
        param.remove(keyHYBRID_NLE_DUMP);
        AudioALSAHyBridNLEManager::getInstance()->dump();
    }

    if (param.getInt(keyHYBRID_NLE_ENABLE, value) == NO_ERROR) {
        param.remove(keyHYBRID_NLE_ENABLE);
        if (value != 0) {
            AudioALSAHyBridNLEManager::getInstance()->setBypassNLE(false);
        } else {
            AudioALSAHyBridNLEManager::getInstance()->setBypassNLE(true);
        }
    }

    if (param.getInt(keyHYBRID_NLE_EOP_SET, value) == NO_ERROR) {
        param.remove(keyHYBRID_NLE_EOP_SET);
        AudioALSAHyBridNLEManager::getInstance()->setNleEopDb(value);
    }
#endif

    if (param.getInt(keyLowLatencyDebugEnable, value) == NO_ERROR) {
        param.remove(keyLowLatencyDebugEnable);
        AudioALSAHardwareResourceManager::getInstance()->EnableLowLatencyDebug(value);
    }

#ifdef MTK_LATENCY_DETECT_PULSE
    if (param.getInt(keyDetectPulseEnable, value) == NO_ERROR) {
        param.remove(keyDetectPulseEnable);
        AudioDetectPulse::setDetectPulse(value ? true : false);
    }
#endif

#ifdef MTK_AUDIO_HIERARCHICAL_PARAM_SUPPORT
    if (param.get(keyAPP_SET_PARAM, value_str) == NO_ERROR) {
        setAudioParam(value_str);
        param.remove(keyAPP_SET_PARAM);
    }

    if (param.get(keyAPP_SET_FIELD, value_str) == NO_ERROR) {
        setAudioField(value_str);
        param.remove(keyAPP_SET_FIELD);
    }

    if (param.get(keyAPP_SAVE_XML, value_str) == NO_ERROR) {
        saveAudioParam(value_str);
        param.remove(keyAPP_SAVE_XML);
    }

#endif
    if (param.getInt(keyAEETimeoutEnable, value) == NO_ERROR) {
        param.remove(keyAEETimeoutEnable);
        setNeedAEETimeoutFlg(value);
    }
#if defined(SUPPORT_FM_AUDIO_BY_PROPRIETARY_PARAMETER_CONTROL)
    if (param.getInt(keyFM_AUDIO_SWITCH, value) == NO_ERROR) {
        param.remove(keyFM_AUDIO_SWITCH);
        ALOGD("%s(), handle_fm = %d", __FUNCTION__, value);
        if (value == FM_AUDIO_SWITCH_ON) {
            status = mStreamManager->setFmEnable(true, true, true);
        } else if (value == FM_AUDIO_SWITCH_OFF) {
            status = mStreamManager->setFmEnable(false);
        }
    }
    if (param.getFloat(keyFM_AUDIO_VOLUME, value_float) == NO_ERROR) {
        param.remove(keyFM_AUDIO_VOLUME);
        ALOGD("%s(), fm_volume = %f", __FUNCTION__, value_float);
        if (value_float < 0 || value_float > 1.0) {
            status = INVALID_OPERATION;
            ALOGE("Invalid Operation because %f should between 0 and 1.0", value_float);
        } else {
            status = AudioALSAFMController::getInstance()->setFmVolume(value_float);
        }
    }

    if (param.get(keyFM_Radio_Mode, value_str) == NO_ERROR) {
        param.remove(keyFM_Radio_Mode);
        ALOGD("%s(), fm_mode = %s", __FUNCTION__, value_str.string());
        if (value_str == "on") {
            //status = AudioALSAFMController::getInstance()->setFmMode(true);
            status = mStreamManager->setFmEnable(true, true, true);
        } else if (value_str == "off") {
            //status = AudioALSAFMController::getInstance()->setFmMode(false);
            status = mStreamManager->setFmEnable(false);
        } else if (value_str == "ready") {
            ALOGD("Just a signal, do nothing for ready");
        }
    }
    if (param.getInt(keyFM_Radio_Mute, value) == NO_ERROR) {
        param.remove(keyFM_Radio_Mute);
        ALOGD("%s(), fm_radio_mute = %d", __FUNCTION__, value);
        if (value != 0) {
            status = AudioALSAFMController::getInstance()->setFmMute(true);
        } else {
            status = AudioALSAFMController::getInstance()->setFmMute(false);
        }
    }
    if (param.getFloat(keyFM_Radio_Volume, value_float) == NO_ERROR) {
        param.remove(keyFM_Radio_Volume);
        ALOGD("%s(), fm_radio_volume = %f", __FUNCTION__, value_float);
        status = AudioALSAFMController::getInstance()->setFmVolume(value_float);
    }
#endif

    if (param.get(keyMTK_DEL_AUDIO_DUMP_FILE, value_str) == NO_ERROR) {
        param.remove(keyMTK_DEL_AUDIO_DUMP_FILE);
        delAudioHalDumpFile(value_str.string());
    }

    // Update rotation
    if (param.get(keyRotation, value_str) == NO_ERROR) {
        param.remove(keyRotation);
        status = -ENOSYS;
    }

    // BT SCO HFP client
    if (param.get(keyHfpEnable, value_str) == NO_ERROR) {
        param.remove(keyHfpEnable);
        status = -ENOSYS;
    }

    // BT SCO HFP client
    if (param.get(keyHfpSetSamplingRate, value_str) == NO_ERROR) {
        param.remove(keyHfpSetSamplingRate);
        status = -ENOSYS;
    }

    // BT SCO HFP client
    if (param.get(keyHfpVolume, value_str) == NO_ERROR) {
        param.remove(keyHfpVolume);
        status = -ENOSYS;
    }

    // a2dp suspend
    if (param.get(keyA2dpSuspended, value_str) == NO_ERROR) {
        if (value_str == "true") {
#if defined(MTK_AUDIODSP_SUPPORT)
            int status = mStreamManager->getA2dpSuspendStatus();
            if (status != 4) { // Send SUSPEND request when status is not SUSPEND SUCCESS
                ALOGD("%s() status=%d, setA2dpSuspend true", __FUNCTION__, status);
                AudioALSAStreamManager::getInstance()->setA2dpSuspend(true);
            }
#endif
        } else if (value_str == "false") {
#if defined(MTK_AUDIODSP_SUPPORT)
            int status = mStreamManager->getA2dpSuspendStatus();
            if (status != 0) { // Send START request when status is not START SUCCESS
                ALOGD("%s() status=%d, setA2dpSuspend false", __FUNCTION__, status);
                AudioALSAStreamManager::getInstance()->setA2dpSuspend(false);
            }
#endif
        }
        param.remove(keyA2dpSuspended);
    }

#if defined(MTK_AUDIO_GAIN_TABLE) || defined(MTK_AUDIO_HIERARCHICAL_PARAM_SUPPORT)
    if (param.getInt(keyVolumeStreamType, value) == NO_ERROR) {
        param.remove(keyVolumeStreamType);
        int device = 0;
        int index = 0;
        if (param.getInt(keyVolumeDevice, device) == NO_ERROR) {
            param.remove(keyVolumeDevice);
            if (param.getInt(keyVolumeIndex, index) == NO_ERROR) {
#ifdef MTK_AUDIO_GAIN_TABLE
                mStreamManager->setAnalogVolume(value, device, index, 0);
#endif
#if defined(MTK_AUDIO_HIERARCHICAL_PARAM_SUPPORT)
                mStreamManager->setVolumeIndex(value, device, index);
#endif
            }
        }
        param.remove(keyVolumeIndex);
    }
#endif

    if (param.size()) {
        ALOGW("%s(), still have param.size() = %zu, remain param = \"%s\"",
              __FUNCTION__, param.size(), param.toString().string());
        status = BAD_VALUE;
    }

SET_PARAM_EXIT:
    ALOGD("-%s(): %s ", __FUNCTION__, keyValuePairs.string());
    return status;
}

String8 AudioALSAHardware::getParameters(const String8 &keys) {
    ALOGD("+%s(), key = %s", __FUNCTION__, keys.string());

    String8 value;
    int cmdType = 0, value_int = 0;
    AudioParameter param = AudioParameter(keys);
    AudioParameter returnParam = AudioParameter();

#ifdef MTK_AUDIO_SMARTPASCP_SUPPORT
    ALOGD("+++%s(), key = %s", __FUNCTION__, keys.string());
    if (strncmp(smartpa_aurisys_get_param_prefix, keys.string(),
                strlen(smartpa_aurisys_get_param_prefix)) == 0) {
        /* get  parameters to smartpa class to dsp */
        ALOGD("%s getParameters", __FUNCTION__);
        AudioSmartPaParam *pSmartpaParam =
            static_cast<AudioSmartPaParam *>(AudioSmartPaParam::getInstance());
        char *retchar = pSmartpaParam->getParameter(keys.string());
        returnParam.add(keys, String8(retchar));
        goto GET_PARAM_EXIT;
    }
#endif

#if defined(MTK_AURISYS_FRAMEWORK_SUPPORT)
    if (param.get(aurisys_set_param_result, value) == NO_ERROR) {
        param.remove(aurisys_set_param_result);

        int retval = get_aurisys_set_raram_result();
        returnParam.add(
            aurisys_set_param_result,
            (retval == 1) ? String8("SET_PASS") : String8("SET_FAIL"));
    }
#endif

    if (param.getInt(kayA2dpDelayReport, value_int) == NO_ERROR) {
        param.remove(kayA2dpDelayReport);
        AudioALSAHardwareResourceManager::getInstance()->setA2dpDeviceLatency(value_int);
    }

#if defined(MTK_COMBO_MODEM_SUPPORT)
    if (strncmp(keySpeechParserGetParam,
                keys.string(),
                strlen(keySpeechParserGetParam)) == 0) {

        SpeechDriverInterface *pSpeechDriver = SpeechDriverFactory::GetInstance()->GetSpeechDriver();
        returnParam.add(keys, String8(pSpeechDriver->getParameter(keys.string())));
        goto GET_PARAM_EXIT;
    }
#endif

#ifdef MTK_AURISYS_FRAMEWORK_SUPPORT
    if (strncmp(aurisys_get_key_prefix,
                keys.string(),
                strlen(aurisys_get_key_prefix)) == 0) {
        returnParam.add(keys, String8(
                            aurisys_get_parameter(
                                keys.string() + strlen(aurisys_get_key_prefix) + 1))); // 1: ','
        goto GET_PARAM_EXIT;
    }
#endif

    if (param.get(keyMTK_AUDENH_SUPPORT, value) == NO_ERROR) {
        param.remove(keyMTK_AUDENH_SUPPORT);
#ifdef MTK_AUDENH_SUPPORT
        value = "true";
#else
        value = "false";
#endif
        returnParam.add(keyMTK_AUDENH_SUPPORT, value);
    }
    if (param.get(keyMTK_TTY_SUPPORT, value) == NO_ERROR) {
        param.remove(keyMTK_TTY_SUPPORT);
#ifdef MTK_TTY_SUPPORT
        value = "true";
#else
        value = "false";
#endif
        returnParam.add(keyMTK_TTY_SUPPORT, value);
    }
    if (param.get(keyMTK_WB_SPEECH_SUPPORT, value) == NO_ERROR) {
        param.remove(keyMTK_WB_SPEECH_SUPPORT);
#ifdef MTK_WB_SPEECH_SUPPORT
        value = "true";
#else
        value = "false";
#endif
        returnParam.add(keyMTK_WB_SPEECH_SUPPORT, value);
    }
    if (param.get(keyMTK_DUAL_MIC_SUPPORT, value) == NO_ERROR) {
        param.remove(keyMTK_DUAL_MIC_SUPPORT);
        if (AudioALSAHardwareResourceManager::getInstance()->getNumPhoneMicSupport() >= 2) {
            value = "true";
        } else {
            value = "false";
        }
        returnParam.add(keyMTK_DUAL_MIC_SUPPORT, value);
    }
    if (param.get(keyMTK_AUDIO_HD_REC_SUPPORT, value) == NO_ERROR) {
        param.remove(keyMTK_AUDIO_HD_REC_SUPPORT);
#ifdef MTK_AUDIO_HD_REC_SUPPORT
        value = "true";
#else
        value = "false";
#endif
        returnParam.add(keyMTK_AUDIO_HD_REC_SUPPORT, value);
    }
    if (param.get(keyMTK_BESLOUDNESS_SUPPORT, value) == NO_ERROR) {
        param.remove(keyMTK_BESLOUDNESS_SUPPORT);
#ifdef MTK_BESLOUDNESS_SUPPORT
        value = "true";
#else
        value = "false";
#endif
        returnParam.add(keyMTK_BESLOUDNESS_SUPPORT, value);
    }
    if (param.get(keySmartPAandWithoutDSP, value) == NO_ERROR) {
        param.remove(keySmartPAandWithoutDSP);

        if (AudioSmartPaController::getInstance()->isSwDspSpkProtect(AUDIO_DEVICE_OUT_SPEAKER) &&
            !AudioSmartPaController::getInstance()->isBypassSwDspSpkProtect()) {
            value = "true";
        } else {
            value = "false";
        }

        returnParam.add(keySmartPAandWithoutDSP, value);
    }
    if (param.get(keyMTK_BESLOUDNESS_MUSIC_PARAM, value) == NO_ERROR) {
        param.remove(keyMTK_BESLOUDNESS_MUSIC_PARAM);
        int Type_AudioData = AUDIO_DRC_MUSIC_PARAMETER;
        int Byte_AudioData = sizeof(AUDIO_ACF_CUSTOM_PARAM_STRUCT);
        String8 AudioEncodedString = GetAudioEncodedBuffer(Type_AudioData, Byte_AudioData);
        returnParam.add(keyMTK_BESLOUDNESS_MUSIC_PARAM, AudioEncodedString);
    }
    if (param.get(keyMTK_BESLOUDNESS_RINGTONE_PARAM, value) == NO_ERROR) {
        param.remove(keyMTK_BESLOUDNESS_RINGTONE_PARAM);
        int Type_AudioData = AUDIO_DRC_RINGTONE_PARAMETER;
        int Byte_AudioData = sizeof(AUDIO_ACF_CUSTOM_PARAM_STRUCT);
        String8 AudioEncodedString = GetAudioEncodedBuffer(Type_AudioData, Byte_AudioData);
        returnParam.add(keyMTK_BESLOUDNESS_RINGTONE_PARAM, AudioEncodedString);
    }
    if (param.get(keyMTK_BESLOUDNESS_RUN_WITH_HAL, value) == NO_ERROR) {
        param.remove(keyMTK_BESLOUDNESS_RUN_WITH_HAL);
#ifdef MTK_BESLOUDNESS_RUN_WITH_HAL
        value = "true";
#else
        value = "false";
#endif
        returnParam.add(keyMTK_BESLOUDNESS_RUN_WITH_HAL, value);
    }
    if (param.get(keyMTK_BESSURROUND_SUPPORT, value) == NO_ERROR) {
        param.remove(keyMTK_BESSURROUND_SUPPORT);
#ifdef MTK_BESSURROUND_SUPPORT
        value = "true";
#else
        value = "false";
#endif
        returnParam.add(keyMTK_BESSURROUND_SUPPORT, value);
    }
    if (param.get(keyMTK_HDMI_MULTI_CHANNEL_SUPPORT, value) == NO_ERROR) {
        param.remove(keyMTK_HDMI_MULTI_CHANNEL_SUPPORT);
#ifdef MTK_HDMI_MULTI_CHANNEL_SUPPORT
        value = "true";
#else
        value = "false";
#endif
        returnParam.add(keyMTK_HDMI_MULTI_CHANNEL_SUPPORT, value);
    }

    if (param.get(keyMTK_VOW_SUPPORT, value) == NO_ERROR) {
        param.remove(keyMTK_VOW_SUPPORT);
#ifdef MTK_VOW_SUPPORT
        value = "true";
#else
        value = "false";
#endif
        returnParam.add(keyMTK_VOW_SUPPORT, value);
    }

    if (param.get(keyMTK_VOW_2E2K_SUPPORT, value) == NO_ERROR) {
        param.remove(keyMTK_VOW_2E2K_SUPPORT);
#ifdef MTK_VOW_2E2K_SUPPORT
        value = "true";
#else
        value = "false";
#endif
        returnParam.add(keyMTK_VOW_2E2K_SUPPORT, value);
    }

    if (param.get(keyMTK_VOW_COMMAND, value) == NO_ERROR) {
        param.remove(keyMTK_VOW_COMMAND);
#ifdef MTK_VOW_COMMAND
        String8 vowCommand = String8(MTK_VOW_COMMAND);
        value = vowCommand.string();
#else
        value = "none";
#endif
        ALOGD("%s(), get MTK_VOW_COMMAND = %s", __FUNCTION__, value.string());
        returnParam.add(keyMTK_VOW_COMMAND, value);
    }

    if (param.get(keyMTK_VOW_COMMAND_2ND, value) == NO_ERROR) {
        param.remove(keyMTK_VOW_COMMAND_2ND);
#ifdef MTK_VOW_COMMAND_2ND
        String8 vowCommand2nd = String8(MTK_VOW_COMMAND_2ND);
        value = vowCommand2nd.string();
#else
        value = "none";
#endif
        ALOGD("%s(), get MTK_VOW_COMMAND_2ND = %s", __FUNCTION__, value.string());
        returnParam.add(keyMTK_VOW_COMMAND_2ND, value);
    }

    if (param.get(keyMTK_VOW_COMMAND_3RD, value) == NO_ERROR) {
        param.remove(keyMTK_VOW_COMMAND_3RD);
#ifdef MTK_VOW_COMMAND_3RD
        String8 vowCommand3rd = String8(MTK_VOW_COMMAND_3RD);
        value = vowCommand3rd.string();
#else
        value = "none";
#endif
        ALOGD("%s(), get MTK_VOW_COMMAND_3RD = %s", __FUNCTION__, value.string());
        returnParam.add(keyMTK_VOW_COMMAND_3RD, value);
    }

    if (param.get(keySetDSM, value) == NO_ERROR) {
        float tempf;
        value = mStreamManager->getStreamOut(0)->getParameters(keys);
        param.remove(keySetDSM);
        ALOGD("keySetDSM value = %s", value.string());
        returnParam = AudioParameter(value);
    }
    if (param.get(keyGetAudioCustomScene, value) == NO_ERROR) {
        value = mStreamManager->getCustScene();
        ALOGD("%s(), getCustScene = %s", __FUNCTION__, value.string());
        param.remove(keyGetAudioCustomScene);
        returnParam.add(keyGetAudioCustomScene, value);
    }

    if (param.get(keyGetFmEnable, value) == NO_ERROR) {
        param.remove(keyGetFmEnable);

        value = (mStreamManager->getFmEnable() == true) ? "true" : "false";
        returnParam.add(keyGetFmEnable, value);
    }

    if (AudioALSAHardwareResourceManager::getInstance()->getNumPhoneMicSupport() >= 2) {
        if (strncmp(keyDUALMIC_GET_GAIN,
                    keys.string(),
                    strlen(keyDUALMIC_GET_GAIN)) == 0) {
            char buf[32];
            unsigned short gain = 0;
            value = String8(keys.string() + strlen(keyDUALMIC_GET_GAIN));
            cmdType = atoi(value.string());

            if (mAudioALSAParamTunerInstance->getDMNRGain((unsigned short)cmdType, &gain) == NO_ERROR) {
                sprintf(buf, "%d", gain);
                returnParam.add(keyDUALMIC_GET_GAIN + value, String8(buf));
                param.remove(keys);
            }
        }
    }

#if defined(MTK_ANC_SUPPORT)
    if (param.get(keyANC_UI, value) == NO_ERROR) {
        param.remove(keyANC_UI);
        value = (mANCController->getANCSwitch() == true) ? "on" : "off";
        returnParam.add(keyANC_UI, value);
    }
    if (param.get(keyANC_Enabled, value) == NO_ERROR) {
        param.remove(keyANC_Enabled);
        value = (mANCController->getANCEnable() == true) ? "true" : "false";
        returnParam.add(keyANC_Enabled, value);
    }
#endif

    if (param.get(keyGetFmTxEnable, value) == NO_ERROR) {
        param.remove(keyGetFmTxEnable);

        value = (mFmTxEnable == true) ? "true" : "false";
        returnParam.add(keyGetFmTxEnable, value);
    }
#if !defined(MTK_AURISYS_FRAMEWORK_SUPPORT)
    if (param.get(keyMusicPlusGet, value) == NO_ERROR) {

        bool musicplus_status = mStreamManager->GetMusicPlusStatus();
        value = (musicplus_status) ? "1" : "0";
        param.remove(keyMusicPlusGet);
        returnParam.add(keyMusicPlusGet, value);
    }
#endif
    if (param.get(keyBesLoudnessGet, value) == NO_ERROR) {

        bool besloudness_status = mStreamManager->GetBesLoudnessStatus();
        value = (besloudness_status) ? "1" : "0";
        param.remove(keyBesLoudnessGet);
        returnParam.add(keyBesLoudnessGet, value);
    }

    // Audio Volume version
    if (param.get(keyGET_AUDIO_VOLUME_VER, value) == NO_ERROR) {
        param.remove(keyGET_AUDIO_VOLUME_VER);
        value = "1";
        returnParam.add(keyGET_AUDIO_VOLUME_VER, value);
    }

    // check if the LR channel switched
    if (param.get(keyLR_ChannelSwitch, value) == NO_ERROR) {
        bool bIsLRSwitch = AudioALSAHardwareResourceManager::getInstance()->getMicInverse();
        value = (bIsLRSwitch == true) ? "1" : "0";
        param.remove(keyLR_ChannelSwitch);
        returnParam.add(keyLR_ChannelSwitch, value);
    }

    // get the Capture drop time for voice unlock
    if (param.get(keyGetCaptureDropTime, value) == NO_ERROR) {
        char buf[32];
        sprintf(buf, "%d", CAPTURE_DROP_MS);
        param.remove(keyGetCaptureDropTime);
        returnParam.add(keyGetCaptureDropTime, String8(buf));
    }
#ifdef MTK_VOW_SUPPORT
    if (strncmp(keyMTK_VOW_MIC_TYPE,
                keys.string(),
                strlen(keyMTK_VOW_MIC_TYPE)) == 0) {
        unsigned int mic_type = 0;
        char buf[32];
        mic_type = AudioALSAVoiceWakeUpController::getInstance()->getVOWMicType();
        param.remove(keyMTK_VOW_MIC_TYPE);
        sprintf(buf, "%d", mic_type);
        returnParam.add(keyMTK_VOW_MIC_TYPE, String8(buf));
        ALOGD("%s(), mic_type = %d", keyMTK_VOW_MIC_TYPE.string(), mic_type);
    }
#endif

    if (param.get(keyGET_MAGI_CONFERENCE_SUPPORT, value) == NO_ERROR) {
#if defined(MTK_MAGICONFERENCE_SUPPORT)
        if (AudioALSAHardwareResourceManager::getInstance()->getNumPhoneMicSupport() >= 2) {
            value = "1";
        } else {
            value = "0";
        }
#else
        value = "0";
#endif
        param.remove(keyGET_MAGI_CONFERENCE_SUPPORT);
        returnParam.add(keyGET_MAGI_CONFERENCE_SUPPORT, value);
    }

#if defined(MTK_MAGICONFERENCE_SUPPORT)
    if (AudioALSAHardwareResourceManager::getInstance()->getNumPhoneMicSupport() >= 2) {
        if (param.get(keyGET_MAGI_CONFERENCE_ENABLE, value) == NO_ERROR) {
            bool magiccon_status = mStreamManager->GetMagiConCallEnable();
            value = (magiccon_status) ? "1" : "0";
            param.remove(keyGET_MAGI_CONFERENCE_ENABLE);
            returnParam.add(keyGET_MAGI_CONFERENCE_ENABLE, value);
        }
    }
#endif

    // Determine if AAudio supports speaker
    if (param.get(keyGetSpeakerProtection, value) == NO_ERROR) {
        // phase 1 : AAudio use legacy mode at all kinds of speakers
        value = "1";

        param.remove(keyGetSpeakerProtection);
        returnParam.add(keyGetSpeakerProtection, value);
    }

    if (param.get(keyGET_SPEECH_DATA_ENCRYPT_SUPPORT, value) == NO_ERROR) {
#if defined(MTK_SPEECH_ENCRYPTION_SUPPORT)
        value = "1";
#else
        value = "0";
#endif
        param.remove(keyGET_SPEECH_DATA_ENCRYPT_SUPPORT);
        returnParam.add(keyGET_SPEECH_DATA_ENCRYPT_SUPPORT, value);
    }

#if defined(MTK_SPEECH_ENCRYPTION_SUPPORT)
    if (param.get(keyGET_SPEECH_DATA_ENCRYPT_STATUS, value) == NO_ERROR) {
        bool sph_data_encrypt_status = SpeechDataEncrypter::GetInstance()->GetEnableStatus();
        value = (sph_data_encrypt_status) ? "1" : "0";
        param.remove(keyGET_SPEECH_DATA_ENCRYPT_STATUS);
        returnParam.add(keyGET_SPEECH_DATA_ENCRYPT_STATUS, value);
    }
#endif

    if (param.get(keyGET_HAC_SUPPORT, value) == NO_ERROR) {
#if defined(MTK_HAC_SUPPORT)
        value = "1";
#else
        value = "0";
#endif
        param.remove(keyGET_HAC_SUPPORT);
        returnParam.add(keyGET_HAC_SUPPORT, value);
    }

#if defined(MTK_HAC_SUPPORT)
    // MTK Legacy HAL GET_HAC_ENABLE
    if (param.get(keyGET_HAC_ENABLE, value) == NO_ERROR) {
        bool hac_status = mStreamManager->GetHACEnable();
        value = (hac_status) ? "1" : "0";
        param.remove(keyGET_HAC_ENABLE);
        returnParam.add(keyGET_HAC_ENABLE, value);
    }

    // For HIDL 4.0 HAC
    if (param.get(keyHAC_SETTING, value) == NO_ERROR) {
        bool hac_status = mStreamManager->GetHACEnable();
        value = (hac_status) ? "1" : "0";
        param.remove(keyHAC_SETTING);
        returnParam.add(keyHAC_SETTING, value);
    }
#endif

    //Audio Mode
    if (param.get(keyGetMode, value) == NO_ERROR) {
        param.remove(keyGetMode);
        int result_int = (int) mStreamManager->getMode();
        ALOGD("%s(), %s, result_int=%d", __FUNCTION__, keyGetMode.string(), result_int);
        returnParam.addInt(keyGetMode, result_int);
    }
    //Phone Call status
    if (param.get(keyGetPhoneCallStatus, value) == NO_ERROR) {
        param.remove(keyGetPhoneCallStatus);
        uint32_t result_int = mStreamManager->isPhoneCallOpen();
        ALOGD("%s(), %s, result_int=%d", __FUNCTION__, keyGetPhoneCallStatus.string(), result_int);
        returnParam.addInt(keyGetPhoneCallStatus, result_int);
    }
    if (param.get(keyForegroundPhoneId, value) == NO_ERROR) {
        param.remove(keyForegroundPhoneId);
        uint8_t result_int = mSpeechPhoneCallController->getPhoneId();
        ALOGD("%s(), %s, result_int=%d", __FUNCTION__, keyForegroundPhoneId.string(), result_int);
        returnParam.addInt(keyForegroundPhoneId, result_int);
    }
    if (param.get(keyPhone2Modem, value) == NO_ERROR) {
        param.remove(keyPhone2Modem);
        modem_index_t IdxModem = mSpeechPhoneCallController->getIdxMDByPhoneId(1);
        if (IdxModem == MODEM_EXTERNAL) {
            value = "MD3";
        } else {
            value = "MD1";
        }
        ALOGD("%s(), %s, value=%s", __FUNCTION__, keyPhone2Modem.string(), value.string());
        returnParam.add(keyPhone2Modem, value);
    }
    if (param.get(keyPhone1Modem, value) == NO_ERROR) {
        param.remove(keyPhone1Modem);
        modem_index_t IdxModem = mSpeechPhoneCallController->getIdxMDByPhoneId(0);
        if (IdxModem == MODEM_EXTERNAL) {
            value = "MD3";
        } else {
            value = "MD1";
        }
        ALOGD("%s(), %s, value=%s", __FUNCTION__, keyPhone1Modem.string(), value.string());
        returnParam.add(keyPhone1Modem, value);
    }
    //Modem Audio DVT and Debug
    if (param.get(keySpeechDVT_GetSampleRate, value) == NO_ERROR) {
        param.remove(keySpeechDVT_GetSampleRate);
        uint32_t result_int = mSpeechPhoneCallController->getSpeechDVT_SampleRate();
        ALOGD("%s(), %s, result_int=%d", __FUNCTION__, keySpeechDVT_GetSampleRate.string(), result_int);
        returnParam.addInt(keySpeechDVT_GetSampleRate, result_int);
    }

    if (param.get(keySpeechDVT_GetMD_IDX, value) == NO_ERROR) {
        param.remove(keySpeechDVT_GetMD_IDX);
        uint32_t result_int = mSpeechPhoneCallController->getSpeechDVT_MD_IDX();
        ALOGD("%s(), %s, result_int=%d", __FUNCTION__, keySpeechDVT_GetMD_IDX.string(), result_int);
        returnParam.addInt(keySpeechDVT_GetMD_IDX, result_int);
    }

#if defined(MTK_AUDIO_HIERARCHICAL_PARAM_SUPPORT)
    if (param.get(keyGET_VMLOG_CONFIG, value) == NO_ERROR) {
        SpeechVmType vmConfig = SpeechVMRecorder::getInstance()->getVmConfig();
        switch (vmConfig) {
        case SPEECH_VM_DISABLE:
            value = "0";
            break;
        case SPEECH_VM_SPEECH:
            value = "1";
            break;
        case SPEECH_VM_CTM4WAY:
            value = "2";
            break;
        default:
            value = "0";
            break;
        }
        param.remove(keyGET_VMLOG_CONFIG);
        returnParam.add(keyGET_VMLOG_CONFIG, value);
    }

    if (param.get(keyGET_CUST_XML_ENABLE, value) == NO_ERROR) {
        value = "1";
        param.remove(keyGET_CUST_XML_ENABLE);
        returnParam.add(keyGET_CUST_XML_ENABLE, value);
    }

    if (param.get(keyGET_SUPER_VOLUME_SUPPORT, value) == NO_ERROR) {
        if (SpeechConfig::getInstance()->getSpeechParamSupport(SPEECH_FEATURE_SUPERVOLUME)) {
            value = "1";
        } else {
            value = "0";
        }
        param.remove(keyGET_SUPER_VOLUME_SUPPORT);
        returnParam.add(keyGET_SUPER_VOLUME_SUPPORT, value);
    }

    if (param.get(keyGET_SUPER_VOLUME_ENABLE, value) == NO_ERROR) {
        bool isSuperVolumeEnable = mSpeechPhoneCallController->getSpeechFeatureStatus(SPEECH_FEATURE_SUPERVOLUME);
        value = (isSuperVolumeEnable) ? "1" : "0";
        param.remove(keyGET_SUPER_VOLUME_ENABLE);
        returnParam.add(keyGET_SUPER_VOLUME_ENABLE, value);
    }
#endif

#ifdef MTK_AUDIO_GAIN_TABLE
    if (param.get(keySpeechBand, value) == NO_ERROR) {
        param.remove(keySpeechBand);
        bool nb = mAudioALSAVolumeController->isNbSpeechBand();
        value = nb ? "1" : "0";
        returnParam.add(keySpeechBand, value);
    }
#endif

    if (AudioALSAHardwareResourceManager::getInstance()->getNumPhoneMicSupport() >= 2) {
        if (param.getInt(keyDUALMIC_GET_GAIN, cmdType) == NO_ERROR) {
            unsigned short gain = 0;
            char buf[32];

            if (mAudioALSAParamTunerInstance->getDMNRGain((unsigned short)cmdType, &gain) == NO_ERROR) {
                sprintf(buf, "%d", gain);
                returnParam.add(keyDUALMIC_GET_GAIN, String8(buf));
                param.remove(keyDUALMIC_GET_GAIN);
            }
        }
    }

    // Dual Mic Noise Reduction, DMNR for Receiver
    if (param.get(keyGet_Dual_Mic_Setting, value) == NO_ERROR) { // new name
        param.remove(keyGet_Dual_Mic_Setting);
        value = (SpeechEnhancementController::GetInstance()->GetDynamicMask(SPH_ENH_DYNAMIC_MASK_DMNR) > 0) ? "1" : "0";
        returnParam.add(keyGet_Dual_Mic_Setting, value);
    }

    // Dual Mic Noise Reduction, DMNR for Loud Speaker: feature face out and be replaced by MagiConference
    if (param.get(keyGET_LSPK_DMNR_ENABLE, value) == NO_ERROR) {
        param.remove(keyGET_LSPK_DMNR_ENABLE);
        value = (SpeechEnhancementController::GetInstance()->GetDynamicMask(SPH_ENH_DYNAMIC_MASK_LSPK_DMNR) > 0) ? "1" : "0";
        returnParam.add(keyGET_LSPK_DMNR_ENABLE, value);
    }
    // VoIP Dual Mic Noise Reduction, DMNR for Receiver
    if (param.get(keyGET_VOIP_RECEIVER_DMNR_ENABLE, value) == NO_ERROR) {
        param.remove(keyGET_VOIP_RECEIVER_DMNR_ENABLE);
        //        value = (mAudioSpeechEnhanceInfoInstance->GetDynamicSpeechEnhancementMaskOnOff(VOIP_SPH_ENH_DYNAMIC_MASK_DMNR) > 0) ? "1" : "0";
        value = "0";
        returnParam.add(keyGET_VOIP_RECEIVER_DMNR_ENABLE, value);
    }

    // VoIP Dual Mic Noise Reduction, DMNR for Loud Speaker
    if (param.get(keyGET_VOIP_LSPK_DMNR_ENABLE, value) == NO_ERROR) {
        param.remove(keyGET_VOIP_LSPK_DMNR_ENABLE);
        //        value = (mAudioSpeechEnhanceInfoInstance->GetDynamicSpeechEnhancementMaskOnOff(VOIP_SPH_ENH_DYNAMIC_MASK_LSPK_DMNR) > 0) ? "1" : "0";
        value = "0";
        returnParam.add(keyGET_VOIP_LSPK_DMNR_ENABLE, value);
    }

    // Voice Clarity Engine, VCE
    if (param.get(keyGET_VCE_ENABLE, value) == NO_ERROR) { // new name
        param.remove(keyGET_VCE_ENABLE);
        value = (SpeechEnhancementController::GetInstance()->GetDynamicMask(SPH_ENH_DYNAMIC_MASK_VCE) > 0) ? "1" : "0";
        returnParam.add(keyGET_VCE_ENABLE, value);
    }

    if (param.get(keyGET_VCE_STATUS, value) == NO_ERROR) { // old name
        param.remove(keyGET_VCE_STATUS);
        value = (SpeechEnhancementController::GetInstance()->GetDynamicMask(SPH_ENH_DYNAMIC_MASK_VCE) > 0) ? "1" : "0";
        returnParam.add(keyGET_VCE_STATUS, value);
    }

    if (param.get(keyMTK_AUDIO_HYBRID_NLE_SUPPORT, value) == NO_ERROR) {
        param.remove(keyMTK_AUDIO_HYBRID_NLE_SUPPORT);
#ifdef MTK_HYBRID_NLE_SUPPORT
        value = "1";
#else
        value = "0";
#endif
        returnParam.add(keyMTK_AUDIO_HYBRID_NLE_SUPPORT, value);
    }

#ifdef MTK_HYBRID_NLE_SUPPORT
    if (param.get(keyHYBRID_NLE_ENABLE, value) == NO_ERROR) {
        param.remove(keyHYBRID_NLE_ENABLE);
        if (AudioALSAHyBridNLEManager::getInstance()->getBypassNLE() == true) {
            value = "0";
        } else {
            value = "1";
        }
        returnParam.add(keyHYBRID_NLE_ENABLE, value);
    }

    if (param.get(keyHYBRID_NLE_EOP_SET, value) == NO_ERROR) {
        char buf[32];
        param.remove(keyHYBRID_NLE_EOP_SET);
        sprintf(buf, "%d", AudioALSAHyBridNLEManager::getInstance()->getNleEopDb());
        returnParam.add(keyHYBRID_NLE_EOP_SET, String8(buf));
    }
#endif

#ifdef MTK_NEW_VOL_CONTROL
    if (param.get(keyGetGainTableSceneTable, value) == NO_ERROR) {
        param.remove(keyGetGainTableSceneTable);
        int sceneCount = AudioMTKGainController::getInstance()->getSceneCount();
        int Type_AudioData = GET_AUDIO_SCENE_GAIN_TABLE;
        int Byte_AudioData = sizeof(GainTableForScene) * sceneCount;
        String8 AudioEncodedString = GetAudioEncodedBuffer(Type_AudioData, Byte_AudioData);
        returnParam.add(keyGetGainTableSceneTable, AudioEncodedString);
    }

    if (param.get(keyGetGainTableNonSceneTable, value) == NO_ERROR) {
        param.remove(keyGetGainTableNonSceneTable);
        int Type_AudioData = GET_AUDIO_NON_SCENE_GAIN_TABLE;
        int Byte_AudioData = sizeof(GainTableForNonScene);
        String8 AudioEncodedString = GetAudioEncodedBuffer(Type_AudioData, Byte_AudioData);
        returnParam.add(keyGetGainTableNonSceneTable, AudioEncodedString);
    }

    if (param.get(keyGetGainTableSceneCount, value) == NO_ERROR) {
        param.remove(keyGetGainTableSceneCount);
        int sceneCount = AudioMTKGainController::getInstance()->getSceneCount();
        ALOGD("%s(), %s, sceneCount = %d", __FUNCTION__, keyGetGainTableSceneCount.string(), sceneCount);
        returnParam.addInt(keyGetGainTableSceneCount, sceneCount);
    }

    if (param.get(keyGetGainTableSceneIndex, value) == NO_ERROR) {
        int sceneIndex = AudioMTKGainController::getInstance()->getSceneIndex(value.string());
        param.remove(keyGetGainTableSceneIndex);
        ALOGD("%s(), %s, sceneIndex = %d", __FUNCTION__, keyGetGainTableSceneCount.string(), sceneIndex);
        returnParam.addInt(keyGetGainTableSceneIndex, sceneIndex);
    }

    if (param.get(keyGetGainTableCurrentSceneIndex, value) == NO_ERROR) {
        int currentSceneIndex = AudioMTKGainController::getInstance()->getCurrentSceneIndex();
        param.remove(keyGetGainTableCurrentSceneIndex);
        ALOGD("%s(), %s, currentSceneIndex = %d", __FUNCTION__,
              keyGetGainTableCurrentSceneIndex.string(), currentSceneIndex);
        returnParam.addInt(keyGetGainTableCurrentSceneIndex, currentSceneIndex);
    }
#endif

    if (strncmp(keyAPP_GET_FO.string(),
                keys.string(),
                keyAPP_GET_FO.length()) == 0) {
        value = String8(keys.string() + keyAPP_GET_FO.length() + 1); // +1 for # seperator
        value = getAudioFO(value);
        returnParam.add(keys, value);
        param.remove(keys);
    }

#ifdef MTK_AUDIO_HIERARCHICAL_PARAM_SUPPORT
    if (strncmp(keyAPP_GET_PARAM.string(),
                keys.string(),
                keyAPP_GET_PARAM.length()) == 0) {
        value = String8(keys.string() + keyAPP_GET_PARAM.length() + 1); // +1 for # seperator
        value = getAudioParam(value);
        returnParam.add(keys, value);
        param.remove(keys);
    }

    if (strncmp(keyAPP_GET_FIELD.string(),
                keys.string(),
                keyAPP_GET_FIELD.length()) == 0) {
        value = String8(keys.string() + keyAPP_GET_FIELD.length() + 1); // +1 for # seperator
        value = getAudioField(value);
        returnParam.add(keys, value);
        param.remove(keys);
    }

    if (strncmp(keyAPP_GET_CATEGORY.string(),
                keys.string(),
                keyAPP_GET_CATEGORY.length()) == 0) {
        value = String8(keys.string() + keyAPP_GET_CATEGORY.length() + 1); // +1 for # seperator
        value = getAudioCategory(value);
        returnParam.add(keys, value);
        param.remove(keys);
    }

    if (strncmp(keyAPP_GET_CHECKLIST.string(),
                keys.string(),
                keyAPP_GET_CHECKLIST.length()) == 0) {
        value = String8(keys.string() + keyAPP_GET_CHECKLIST.length() + 1); // +1 for # seperator
        value = getAudioCheckList(value);
        returnParam.add(keys, value);
        param.remove(keys);
    }

#endif

    if (param.get(keyGetAudioGainFromNvRam, value) == NO_ERROR) {
        param.remove(keyGetAudioGainFromNvRam);
        int Type_AudioData = GET_AUDIO_POLICY_VOL_FROM_VER1_DATA;
        int Byte_AudioData = sizeof(AUDIO_CUSTOM_VOLUME_STRUCT);
        String8 AudioEncodedString = GetAudioEncodedBuffer(Type_AudioData, Byte_AudioData);
        returnParam.add(keyGetAudioGainFromNvRam, AudioEncodedString);
    }

#if defined(MTK_AUDIO_GAIN_TABLE) && !defined(MTK_NEW_VOL_CONTROL)
    if (param.get(keyGetATTDisplayInfoTc1, value) == NO_ERROR) {
        param.remove(keyGetATTDisplayInfoTc1);
        int Type_AudioData = GET_TC1_DISP;
        int Byte_AudioData = sizeof(PCDispTotolStru);
        String8 AudioEncodedString = GetAudioEncodedBuffer(Type_AudioData, Byte_AudioData);
        returnParam.add(keyGetATTDisplayInfoTc1, AudioEncodedString);
    }
#endif

#if defined(MTK_AUDIO_GAIN_TABLE) && !defined(MTK_NEW_VOL_CONTROL) && defined(MTK_AUDIO_GAIN_TABLE_BT)
    if (param.get(keyGetATTDisplayInfoTc1BtNrec, value) == NO_ERROR) {
        param.remove(keyGetATTDisplayInfoTc1BtNrec);
        int Type_AudioData = GET_BT_NREC_DISP;
        int Byte_AudioData = sizeof(PCDispItem);
        String8 AudioEncodedString = GetAudioEncodedBuffer(Type_AudioData, Byte_AudioData);
        returnParam.add(keyGetATTDisplayInfoTc1BtNrec, AudioEncodedString);
    }
#endif
    if (param.get(keyGetAudioCustomParamFromNvRam, value) == NO_ERROR) {
        param.remove(keyGetAudioCustomParamFromNvRam);
        int Type_AudioData = AUDIOCUSTOMDATA;
        int Byte_AudioData = sizeof(AUDIO_VOLUME_CUSTOM_STRUCT);
        String8 AudioEncodedString = GetAudioEncodedBuffer(Type_AudioData, Byte_AudioData);
        returnParam.add(keyGetAudioCustomParamFromNvRam, AudioEncodedString);
    }

    if (param.get(keyGetVolumeVer1ParamFromNvRam, value) == NO_ERROR) {
        param.remove(keyGetVolumeVer1ParamFromNvRam);
        int Type_AudioData = AUDIO_VER1_DATA;
        int Byte_AudioData = sizeof(AUDIO_VER1_CUSTOM_VOLUME_STRUCT);
        String8 AudioEncodedString = GetAudioEncodedBuffer(Type_AudioData, Byte_AudioData);
        returnParam.add(keyGetVolumeVer1ParamFromNvRam, AudioEncodedString);
    }

    if (param.get(keyGetNBSpeechParamFromNvRam, value) == NO_ERROR) {
        param.remove(keyGetNBSpeechParamFromNvRam);
        int Type_AudioData = NB_SPEECH_PARAMETER;
        int Byte_AudioData = sizeof(AUDIO_CUSTOM_PARAM_STRUCT);
        String8 AudioEncodedString = GetAudioEncodedBuffer(Type_AudioData, Byte_AudioData);
        returnParam.add(keyGetNBSpeechParamFromNvRam, AudioEncodedString);
    }

#if defined(MTK_WB_SPEECH_SUPPORT)
    if (param.get(keyGetWBSpeechParamFromNvRam, value) == NO_ERROR) {
        param.remove(keyGetWBSpeechParamFromNvRam);
        int Type_AudioData = WB_SPEECH_PARAMETER;
        int Byte_AudioData = sizeof(AUDIO_CUSTOM_WB_PARAM_STRUCT);
        String8 AudioEncodedString = GetAudioEncodedBuffer(Type_AudioData, Byte_AudioData);
        returnParam.add(keyGetWBSpeechParamFromNvRam, AudioEncodedString);
    }
#endif

#if defined(MTK_INCALL_HANDSFREE_DMNR) || defined(MTK_MAGICONFERENCE_SUPPORT)
    if (AudioALSAHardwareResourceManager::getInstance()->getNumPhoneMicSupport() >= 2) {
        if (param.get(keyGetMagiConSpeechParamFromNvRam, value) == NO_ERROR) {
            param.remove(keyGetMagiConSpeechParamFromNvRam);
            int Type_AudioData = SPEECH_MAGICON_PARAMETER;
            int Byte_AudioData = sizeof(AUDIO_CUSTOM_MAGI_CONFERENCE_STRUCT);
            String8 AudioEncodedString = GetAudioEncodedBuffer(Type_AudioData, Byte_AudioData);
            returnParam.add(keyGetMagiConSpeechParamFromNvRam, AudioEncodedString);
        }
    }
#endif

#if defined(MTK_HAC_SUPPORT)
    if (param.get(keyGetHACSpeechParamFromNvRam, value) == NO_ERROR) {
        param.remove(keyGetHACSpeechParamFromNvRam);
        int Type_AudioData = SPEECH_HAC_PARAMETER;
        int Byte_AudioData = sizeof(AUDIO_CUSTOM_HAC_PARAM_STRUCT);
        String8 AudioEncodedString = GetAudioEncodedBuffer(Type_AudioData, Byte_AudioData);
        returnParam.add(keyGetHACSpeechParamFromNvRam, AudioEncodedString);
    }
#endif

    if (param.get(keyGetSpeechOutFirIdxFromNvRam, value) == NO_ERROR) {
        param.remove(keyGetSpeechOutFirIdxFromNvRam);
        AUDIO_PARAM_MED_STRUCT pMedPara;
        mAudioCustParamClient->GetMedParamFromNV(&pMedPara);
        int idxSpeechOutFir = pMedPara.select_FIR_output_index[Normal_Coef_Index];
        returnParam.addInt(keyGetSpeechOutFirIdxFromNvRam, idxSpeechOutFir);
    }
    if (param.get(keyGetAudioCustomDataSize, value) == NO_ERROR) {
        param.remove(keyGetAudioCustomDataSize);
        int AudioCustomDataSize = sizeof(AUDIO_VOLUME_CUSTOM_STRUCT);
        ALOGD("%s(), AudioCustomDataSize=%d", __FUNCTION__, AudioCustomDataSize);
        returnParam.addInt(keyGetAudioCustomDataSize, AudioCustomDataSize);
    }
    if (param.get(keyGetSpeechNormalOutFirIdxFromNvRam, value) == NO_ERROR) {
        param.remove(keyGetSpeechNormalOutFirIdxFromNvRam);
        AUDIO_PARAM_MED_STRUCT pMedPara;
        mAudioCustParamClient->GetMedParamFromNV(&pMedPara);
        int idxSpeechOutFir = pMedPara.select_FIR_output_index[Normal_Coef_Index];
        returnParam.addInt(keyGetSpeechNormalOutFirIdxFromNvRam, idxSpeechOutFir);
    }
    if (param.get(keyGetSpeechHeadsetOutFirIdxFromNvRam, value) == NO_ERROR) {
        param.remove(keyGetSpeechHeadsetOutFirIdxFromNvRam);
        AUDIO_PARAM_MED_STRUCT pMedPara;
        mAudioCustParamClient->GetMedParamFromNV(&pMedPara);
        int idxSpeechOutFir = pMedPara.select_FIR_output_index[Headset_Coef_Index];
        returnParam.addInt(keyGetSpeechHeadsetOutFirIdxFromNvRam, idxSpeechOutFir);
    }
    if (param.get(keyGetSpeechHandfreeOutFirIdxFromNvRam, value) == NO_ERROR) {
        param.remove(keyGetSpeechHandfreeOutFirIdxFromNvRam);
        AUDIO_PARAM_MED_STRUCT pMedPara;
        mAudioCustParamClient->GetMedParamFromNV(&pMedPara);
        int idxSpeechOutFir = pMedPara.select_FIR_output_index[Handfree_Coef_Index];
        returnParam.addInt(keyGetSpeechHandfreeOutFirIdxFromNvRam, idxSpeechOutFir);
    }
    if (param.get(keyQueryFeatureSupportInfo, value) == NO_ERROR) {
        param.remove(keyQueryFeatureSupportInfo);
        uint32_t featureSupportInfo = mAudioCustParamClient->QueryFeatureSupportInfo();
        returnParam.addInt(keyQueryFeatureSupportInfo, featureSupportInfo);
    }
    if (param.get(keyGetDualMicParamFromNvRam, value) == NO_ERROR) {
        param.remove(keyGetDualMicParamFromNvRam);
        int Type_AudioData = DUAL_MIC_PARAMETER;
        int Byte_AudioData = sizeof(AUDIO_CUSTOM_EXTRA_PARAM_STRUCT);
        String8 AudioEncodedString = GetAudioEncodedBuffer(Type_AudioData, Byte_AudioData);
        returnParam.add(keyGetDualMicParamFromNvRam, AudioEncodedString);
    }
    if (param.get(keyGetMedParamFromNvRam, value) == NO_ERROR) {
        param.remove(keyGetMedParamFromNvRam);
        int Type_AudioData = MEDDATA;
        int Byte_AudioData = sizeof(AUDIO_PARAM_MED_STRUCT);
        String8 AudioEncodedString = GetAudioEncodedBuffer(Type_AudioData, Byte_AudioData);
        returnParam.add(keyGetMedParamFromNvRam, AudioEncodedString);
    }
    if (param.get(keyGetNBSpeechLpbkParamFromNvRam, value) == NO_ERROR) {
        param.remove(keyGetNBSpeechLpbkParamFromNvRam);
        int Type_AudioData = NB_SPEECH_LPBK_PARAMETER;
        int Byte_AudioData = sizeof(AUDIO_CUSTOM_SPEECH_LPBK_PARAM_STRUCT);
        String8 AudioEncodedString = GetAudioEncodedBuffer(Type_AudioData, Byte_AudioData);
        returnParam.add(keyGetNBSpeechLpbkParamFromNvRam, AudioEncodedString);
    }
    if (param.get(keyGetAudioVoIPParamFromNvRam, value) == NO_ERROR) {
        param.remove(keyGetAudioVoIPParamFromNvRam);
        int Type_AudioData = VOIP_PARAMETER;
        int Byte_AudioData = sizeof(AUDIO_VOIP_PARAM_STRUCT);
        String8 AudioEncodedString = GetAudioEncodedBuffer(Type_AudioData, Byte_AudioData);
        returnParam.add(keyGetAudioVoIPParamFromNvRam, AudioEncodedString);
    }
    if (param.get(keyGetAudioHFPParamFromNvRam, value) == NO_ERROR) {
        param.remove(keyGetAudioHFPParamFromNvRam);
        int Type_AudioData = HFP_PARAMETER;
        int Byte_AudioData = sizeof(AUDIO_HFP_PARAM_STRUCT);
        String8 AudioEncodedString = GetAudioEncodedBuffer(Type_AudioData, Byte_AudioData);
        returnParam.add(keyGetAudioHFPParamFromNvRam, AudioEncodedString);
    }
    if (param.get(keyGetHdRecordParamFromNvRam, value) == NO_ERROR) {
        param.remove(keyGetHdRecordParamFromNvRam);
        int Type_AudioData = HD_RECORD_PARAMETER;
        int Byte_AudioData = sizeof(AUDIO_HD_RECORD_PARAM_STRUCT);
        String8 AudioEncodedString = GetAudioEncodedBuffer(Type_AudioData, Byte_AudioData);
        returnParam.add(keyGetHdRecordParamFromNvRam, AudioEncodedString);
    }
    if (param.get(keyGetHdRecordSceneTableFromNvRam, value) == NO_ERROR) {
        param.remove(keyGetHdRecordSceneTableFromNvRam);
        int Type_AudioData = HD_RECORD_SCENE_TABLE;
        int Byte_AudioData = sizeof(AUDIO_HD_RECORD_SCENE_TABLE_STRUCT);
        String8 AudioEncodedString = GetAudioEncodedBuffer(Type_AudioData, Byte_AudioData);
        returnParam.add(keyGetHdRecordSceneTableFromNvRam, AudioEncodedString);
    }
    if (param.get(keyGetDumpAEECheck, value) == NO_ERROR) {
        param.remove(keyGetDumpAEECheck);
        char valueDump[PROPERTY_VALUE_MAX];
        property_get("vendor.streamout.aee.dump", valueDump, "0");
        int isDumpEnable = atoi(valueDump);
        ALOGD("%s(), DUMP_AUDIO_AEE_CHECK=%d", __FUNCTION__, isDumpEnable);
        returnParam.addInt(keyGetDumpAEECheck, isDumpEnable);
    }
    if (param.get(keyGetDumpAudioStreamOut, value) == NO_ERROR) {
        param.remove(keyGetDumpAudioStreamOut);
        char valueDump[PROPERTY_VALUE_MAX];
        property_get("vendor.streamout.pcm.dump", valueDump, "0");
        int isDumpEnable = atoi(valueDump);
        ALOGD("%s(), DUMP_AUDIO_STREAM_OUT=%d", __FUNCTION__, isDumpEnable);
        returnParam.addInt(keyGetDumpAudioStreamOut, isDumpEnable);
    }
    if (param.get(keyGetDumpAudioMixerBuf, value) == NO_ERROR) {
        param.remove(keyGetDumpAudioMixerBuf);
        char valueDump[PROPERTY_VALUE_MAX];
        property_get("vendor.af.mixer.pcm", valueDump, "0");
        int isDumpEnable = atoi(valueDump);
        ALOGD("%s(), DUMP_AUDIO_MIXER_BUF=%d", __FUNCTION__, isDumpEnable);
        returnParam.addInt(keyGetDumpAudioMixerBuf, isDumpEnable);
    }

    if (param.get(keyGetDumpAudioTrackBuf, value) == NO_ERROR) {
        param.remove(keyGetDumpAudioTrackBuf);
        char valueDump[PROPERTY_VALUE_MAX];
        property_get("vendor.af.track.pcm", valueDump, "0");
        int isDumpEnable = atoi(valueDump);
        ALOGD("%s(), DUMP_AUDIO_TRACK_BUF=%d", __FUNCTION__, isDumpEnable);
        returnParam.addInt(keyGetDumpAudioTrackBuf, isDumpEnable);
    }
    if (param.get(keyGetDumpA2DPStreamOut, value) == NO_ERROR) {
        param.remove(keyGetDumpA2DPStreamOut);
        char valueDump[PROPERTY_VALUE_MAX];
        property_get("vendor.a2dp.streamout.pcm", valueDump, "0");
        int isDumpEnable = atoi(valueDump);
        ALOGD("%s(), DUMP_A2DP_STREAM_OUT=%d", __FUNCTION__, isDumpEnable);
        returnParam.addInt(keyGetDumpA2DPStreamOut, isDumpEnable);
    }
    if (param.get(keyGetDumpAudioStreamIn, value) == NO_ERROR) {
        param.remove(keyGetDumpAudioStreamIn);
        char valueDump[PROPERTY_VALUE_MAX];
        property_get("vendor.streamin.pcm.dump", valueDump, "0");
        int isDumpEnable = atoi(valueDump);
        ALOGD("%s(), DUMP_AUDIO_STREAM_IN=%d", __FUNCTION__, isDumpEnable);
        returnParam.addInt(keyGetDumpAudioStreamIn, isDumpEnable);
    }
    if (param.get(keyGetDumpIdleVM, value) == NO_ERROR) {
        int isDumpEnable = 0;
        param.remove(keyGetDumpIdleVM);
#if defined(MTK_AUDIO_HD_REC_SUPPORT)
        char valueDump[PROPERTY_VALUE_MAX];
        property_get("vendor.streamin.vm.dump", valueDump, "0");
        isDumpEnable = atoi(valueDump);
#endif
        ALOGD("%s(), DUMP_IDLE_VM_RECORD=%d", __FUNCTION__, isDumpEnable);
        returnParam.addInt(keyGetDumpIdleVM, isDumpEnable);
    }
    if (param.get(keyGetDumpApSpeechEPL, value) == NO_ERROR) {
        param.remove(keyGetDumpApSpeechEPL);
        char valueDump[PROPERTY_VALUE_MAX];
        property_get("vendor.streamin.epl.dump", valueDump, "0");
        int isDumpEnable = atoi(valueDump);

        char valueDump2[PROPERTY_VALUE_MAX];
        property_get("vendor.streamin.epl.dump", valueDump2, "0");
        int isDumpCheckAgain = atoi(valueDump2);

        if (isDumpCheckAgain == 1) {
            isDumpEnable = 1;
        }
        ALOGD("%s(), DUMP_AP_SPEECH_EPL=%d", __FUNCTION__, isDumpEnable);
        returnParam.addInt(keyGetDumpApSpeechEPL, isDumpEnable);
    }
    if (param.get(keyGetMagiASRTestEnable, value) == NO_ERROR) {
        param.remove(keyGetMagiASRTestEnable);
        //get the MagiASR verify mode status
        int StatusMagiASR = mAudioSpeechEnhanceInfoInstance->GetForceMagiASRState();
        ALOGD("%s(), MagiASR_TEST_ENABLE=%d", __FUNCTION__, StatusMagiASR);
        returnParam.addInt(keyGetMagiASRTestEnable, StatusMagiASR);
    }
    if (param.get(keyGetAECRecTestEnable, value) == NO_ERROR) {
        param.remove(keyGetAECRecTestEnable);
        int StatusAECRec = 0;
        //get the AECRec verify mode status
        if (mAudioSpeechEnhanceInfoInstance->GetForceAECRecState()) {
            StatusAECRec = 1;
        }
        ALOGD("%s(), AECREC_TEST_ENABLE=%d", __FUNCTION__, StatusAECRec);
        returnParam.addInt(keyGetAECRecTestEnable, StatusAECRec);
    }
    if (param.get(keyGetDumpAudioTrackBuf, value) == NO_ERROR) {
        param.remove(keyGetDumpAudioTrackBuf);
        char valueDump[PROPERTY_VALUE_MAX];
        property_get("vendor.af.track.pcm", valueDump, "0");
        int isDumpEnable = atoi(valueDump);
        ALOGD("%s(), DUMP_AUDIO_TRACK_BUF=%d", __FUNCTION__, isDumpEnable);
        returnParam.addInt(keyGetDumpAudioTrackBuf, isDumpEnable);
    }

#ifdef  MTK_HIFIAUDIO_SUPPORT
    if (param.get(String8("hifi_dac"), value) == NO_ERROR) {
        param.remove(String8("hifi_dac"));
        int ret_value = 0;
#ifdef MTK_AUDIO_HIERARCHICAL_PARAM_SUPPORT
        // get from xml: SoundEnhancement_AudioParam.xml
        ret_value = getHiFiStateFromXML();
        ALOGD("%s(), get HiFi state = %d", __FUNCTION__, ret_value);
#endif
        returnParam.add(String8("hifi_dac"), (ret_value) ? String8("on") : String8("off"));
    }

    if (param.get(keyHiFiState, value) == NO_ERROR) {
        param.remove(keyHiFiState);
        int ret_value = 0;
#ifdef MTK_AUDIO_HIERARCHICAL_PARAM_SUPPORT
        // get from xml: SoundEnhancement_AudioParam.xml
        ret_value = getHiFiStateFromXML();
        ALOGD("%s(), get HiFi state = %d", __FUNCTION__, ret_value);
#endif
        returnParam.addInt(keyHiFiState, ret_value);
    }
#endif


#if defined(SUPPORT_FM_AUDIO_BY_PROPRIETARY_PARAMETER_CONTROL)
    if (param.get(keyFM_AUDIO_SWITCH, value) == NO_ERROR) {
        param.remove(keyFM_AUDIO_SWITCH);
        if (mStreamManager->getFmEnable() == true) {
            returnParam.addInt(keyFM_AUDIO_SWITCH, FM_AUDIO_SWITCH_ON);
        } else {
            returnParam.addInt(keyFM_AUDIO_SWITCH, FM_AUDIO_SWITCH_OFF);
        }
    }
    if (param.get(keyFM_AUDIO_VOLUME, value) == NO_ERROR) {
        param.remove(keyFM_AUDIO_VOLUME);
        returnParam.addFloat(keyFM_AUDIO_VOLUME, AudioALSAFMController::getInstance()->getFmVolume());
    }
#endif

#if defined(MTK_AUDIO_HIERARCHICAL_PARAM_SUPPORT)||defined(MTK_AUDIO_GAIN_TABLE)
    if (param.get(keyVolumeIndex, value) == NO_ERROR) {
        param.remove(keyVolumeIndex);
        int volumeIndex = mStreamManager->getVolumeIndex();
        returnParam.addInt(keyVolumeIndex, volumeIndex);
    }
#endif

    // a2dp offload
    if (param.get(keyA2dpCodec, value) == NO_ERROR) {
        // refer to hardware\interfaces\bluetooth\a2dp\1.0\Types.hal
        uint32_t type = mStreamManager->getBluetoothAudioCodecType();
        // if type unknown, check it again due to possible timing delay of setBluetoothAudioOffloadParam
        if (type == 0) {
            usleep(50 * 1000);
            type = mStreamManager->getBluetoothAudioCodecType();
        }
        ALOGD("a2dp codec type 0x%x", type);
        switch (type) {
        case 0x01:
            value = "sbc";
            break;
        case 0x02:
            value = "aac";
            break;
        case 0x04:
            value = "aptx";
            break;
        case 0x08:
            value = "aptx_hd";
            break;
        case 0x10:
            value = "ldac";
            break;
        default:
            value = "unknown";
            break;
        }
        param.remove(keyA2dpCodec);
        returnParam.add(keyA2dpCodec, value);
    }

    if (param.get(keyMTK_GET_AUDIO_DUMP_FILE_LIST, value) == NO_ERROR) {
        param.remove(keyMTK_GET_AUDIO_DUMP_FILE_LIST);
        returnParam.add(keyMTK_GET_AUDIO_DUMP_FILE_LIST, getAudioHalDumpFileList());
    }

    if (strncmp(keyMTK_GET_AUDIO_DUMP_FILE_CONTENT.string(), keys.string(),
                keyMTK_GET_AUDIO_DUMP_FILE_CONTENT.length()) == 0) {

        char *restOfStr = NULL;
        String8 encStr = String8("");

        // read file content size
        char *fromIdxStr = NULL;
        char *readSizeStr = NULL;
        char *fileName = NULL;
        size_t fromIdx = 0, readSize = 0;

        char *str = strdup(keys.string());
        char *command = strtok_r(str, "#", &restOfStr);

        if (command) {
            fileName = strtok_r(NULL, "#", &restOfStr);
        }

        if (fileName) {
            fromIdxStr = strtok_r(NULL, "#", &restOfStr);
            if (fromIdxStr) {
                readSizeStr = strtok_r(NULL, "#", &restOfStr);
            }
        }

        if (fromIdxStr) {
            if (sscanf(fromIdxStr, "%zu", &fromIdx) == 0) {
                fromIdx = 0;
                ALOGW("%s(), wrong fromIdxStr info... (%s)\n", __FUNCTION__, fromIdxStr);
            }
        }

        if (readSizeStr) {
            if (sscanf(readSizeStr, "%zu", &readSize) == 0) {
                readSize = 0;
                ALOGW("%s(), wrong sizeStr info... (%s)\n", __FUNCTION__, readSizeStr);
            }
        }
        ALOGV("%s(), fileName= %s, fromIdxStr = %zu, readSizeStr = %zu)\n", __FUNCTION__, fileName, fromIdx, readSize);

        // Read content to encBuffer
        if (fileName && readSize != 0) {
            unsigned char *encBuffer = NULL;
            size_t ret = readAudioHalDumpFileContent(fileName, &encBuffer, fromIdx, readSize);
            if (ret) {
                encStr = String8((char *)encBuffer);
            }
            if (encBuffer) {
                delete[] encBuffer;
            }
        }

        free(str);
        param.remove(keys);
        returnParam.add(keys, encStr);
    }

#if defined(MTK_AUDIODSP_SUPPORT)
    if (param.get(keyEarFeedbackInDSP, value) == NO_ERROR) {
        param.remove(keyEarFeedbackInDSP);
        value = (AudioALSAHandlerKtv::getInstance()->getTaskKtvStatus()) ? "1" : "0";
        returnParam.add(keyEarFeedbackInDSP, value);
    }
#endif

GET_PARAM_EXIT:

    const String8 keyValuePairs = returnParam.toString();
    ALOGD("-%s(), return \"%s\"", __FUNCTION__, keyValuePairs.string());
    return keyValuePairs;
}

size_t AudioALSAHardware::getInputBufferSize(uint32_t sampleRate, int format, int channelCount) {
    // TODO(Harvey): check it
    return mStreamManager->getInputBufferSize(sampleRate, static_cast<audio_format_t>(format), channelCount); //return 1024;
}

AudioMTKStreamOutInterface *AudioALSAHardware::openOutputStream(
    uint32_t devices,
    int *format,
    uint32_t *channels,
    uint32_t *sampleRate,
    status_t *status) {
    return mStreamManager->openOutputStream(devices, format, channels, sampleRate, status);
}

void AudioALSAHardware::closeOutputStream(AudioMTKStreamOutInterface *out) {
    mStreamManager->closeOutputStream(out);
}

AudioMTKStreamInInterface *AudioALSAHardware::openInputStream(
    uint32_t devices,
    int *format,
    uint32_t *channels,
    uint32_t *sampleRate,
    status_t *status,
    audio_in_acoustics_t acoustics) {
    return mStreamManager->openInputStream(devices, format, channels, sampleRate, status, acoustics);
}

AudioMTKStreamInInterface *AudioALSAHardware::openInputStreamWithFlags(
    uint32_t devices,
    int *format,
    uint32_t *channels,
    uint32_t *sampleRate,
    status_t *status,
    audio_in_acoustics_t acoustics,
    audio_input_flags_t flags) {
    return mStreamManager->openInputStream(devices, format, channels, sampleRate, status, acoustics, flags);
}

void AudioALSAHardware::closeInputStream(AudioMTKStreamInInterface *in) {
    mStreamManager->closeInputStream(in);
}


status_t AudioALSAHardware::dumpState(int fd __unused, const Vector<String16> &args __unused) {
    ALOGV("%s()", __FUNCTION__);
    return NO_ERROR;
}

status_t AudioALSAHardware::dump(int fd __unused, const Vector<String16> &args __unused) {
    ALOGV("%s()", __FUNCTION__);
    return NO_ERROR;
}

int AudioALSAHardware::getMicrophones(struct audio_microphone_characteristic_t *micArray, size_t *micCount) {
    if (micCount == NULL) {
        ALOGE("%s(), micCount is NULL", __FUNCTION__);
        return -EINVAL;
    }

    if (micArray == NULL) {
        ALOGE("%s(), micArray is NULL", __FUNCTION__);
        return -EINVAL;
    }

    /* Get all mic info */
    getMicInfo(micArray, micCount);
    ALOGD("%s(), micArray = %p, micCount = %zu", __FUNCTION__, micArray, *micCount);

    //dumpMicInfo(micArray, *micCount);

    return 0;
}

#if 1

static status_t TDMrecordInit(uint32_t sampleRate) {
    ALOGD("+%s(), sampleRate = %d", __FUNCTION__, sampleRate);

    if (gAudioHardware == NULL) {
        gAudioHardware = createMTKAudioHardware();
    }
    gAudioStreamIn = NULL;

    if (gAudioStreamIn == NULL) {
        android::AudioParameter paramVoiceMode = android::AudioParameter();
        paramVoiceMode.addInt(android::String8("HDREC_SET_VOICE_MODE"), 0);
        gAudioHardware->setParameters(paramVoiceMode.toString());

        uint32_t device = AUDIO_DEVICE_IN_HDMI;
        //        uint32_t device = AUDIO_DEVICE_IN_BUILTIN_MIC;

        int format = AUDIO_FORMAT_PCM_16_BIT;
        uint32_t channel = AUDIO_CHANNEL_IN_STEREO;
        status_t status = 0;
        android::AudioParameter param = android::AudioParameter();

        gAudioStreamIn = gAudioHardware->openInputStream(device, &format, &channel, &sampleRate, &status, (audio_in_acoustics_t)0);
        if (gAudioStreamIn == NULL) {
            ALOGD("Reopen openInputStream with format=%d, channel=%d, sampleRate=%d \n", format, channel, sampleRate);
            gAudioStreamIn = gAudioHardware->openInputStream(device, &format, &channel, &sampleRate, &status, (audio_in_acoustics_t)0);
        }

        //        param.addInt(android::String8(android::AudioParameter::keyInputSource), android_audio_legacy::AUDIO_SOURCE_TDM);
        ALOGD("recordInit samplerate=%d\n", sampleRate);

        /*if (gAudioStreamIn != NULL)
        {
            gAudioStreamIn->setParameters(param.toString());
        }*/
    }
    android::AudioParameter param = android::AudioParameter();
    param.addInt(android::String8(android::AudioParameter::keyRouting), AUDIO_DEVICE_IN_HDMI);
    //    param.addInt(android::String8(android::AudioParameter::keyRouting), AUDIO_DEVICE_IN_BUILTIN_MIC);

    /*if (gAudioStreamIn != NULL)
    {
        gAudioStreamIn->setParameters(param.toString());
    }*/

    return NO_ERROR;
}


status_t AudioALSAHardware::setTDMRecord(int samplerate) {
    ALOGD("+%s(), samplerate = %d", __FUNCTION__, samplerate);

    if (samplerate > 0) {
        SetTDMrecordEnable(samplerate);
    } else {
        SetTDMrecordDisable();
    }
    return NO_ERROR;
}

static void *TDM_Record_Thread(void *mPtr __unused) {

    short pbuffer[8192] = {0};
#if 0
    char filenameL[] = "/data/vendor/audiohal/record_tdm.pcm";
    FILE *fpL = fopen(filenameL, "wb+");
#endif
    int readSize = 0;

    TDMrecordInit(gTDMsetSampleRate);
    while (1) {

        memset(pbuffer, 0, sizeof(pbuffer));

        readSize = gAudioStreamIn->read(pbuffer, 8192 * 2);

#if 0
        if (fpL != NULL) {
            fwrite(pbuffer, readSize, 1, fpL);
        }
#endif
        if (gTDMsetExit) {
            break;
        }
    }
#if 0
    fclose(fpL);
#endif
    ALOGD("%s: Stop", __FUNCTION__);
    pthread_exit(NULL); // thread exit
    return NULL;

}

status_t AudioALSAHardware::SetTDMrecordEnable(uint32_t sampleRate) {
    pthread_t TDMRecordThread;

    pthread_create(&TDMRecordThread, NULL, TDM_Record_Thread, NULL);

    gTDMsetSampleRate = sampleRate;

    gTDMsetExit = false;

    return NO_ERROR;

}

status_t AudioALSAHardware::SetTDMrecordDisable(void) {
    ALOGD("+%s()", __FUNCTION__);

    gTDMsetExit = true;


    if (gAudioStreamIn != NULL) {
        gAudioStreamIn->standby();
        gAudioHardware->closeInputStream(gAudioStreamIn);
        gAudioStreamIn = NULL;
    }

    return NO_ERROR;
}

status_t AudioALSAHardware::SetEMParameter(void *ptr, int len) {
    ALOGD("%s() len [%d] sizeof [%zu]", __FUNCTION__, len, sizeof(AUDIO_CUSTOM_PARAM_STRUCT));

    if (len == sizeof(AUDIO_CUSTOM_PARAM_STRUCT)) {
        AUDIO_CUSTOM_PARAM_STRUCT *pSphParamNB = (AUDIO_CUSTOM_PARAM_STRUCT *)ptr;
        mStreamManager->SetEMParameter(pSphParamNB);
        return NO_ERROR;
    } else {
        ALOGE("len [%d] != Sizeof(AUDIO_CUSTOM_PARAM_STRUCT) [%zu] ", len, sizeof(AUDIO_CUSTOM_PARAM_STRUCT));
        return UNKNOWN_ERROR;
    }
}

status_t AudioALSAHardware::GetEMParameter(void *ptr, int len) {
    ALOGD("%s() len [%d] sizeof [%zu]", __FUNCTION__, len, sizeof(AUDIO_CUSTOM_PARAM_STRUCT));

    if (len == sizeof(AUDIO_CUSTOM_PARAM_STRUCT)) {
        mAudioCustParamClient->GetNBSpeechParamFromNVRam((AUDIO_CUSTOM_PARAM_STRUCT *)ptr);
        return NO_ERROR;
    } else {
        ALOGE("len [%d] != Sizeof(AUDIO_CUSTOM_PARAM_STRUCT) [%zu] ", len, sizeof(AUDIO_CUSTOM_PARAM_STRUCT));
        return UNKNOWN_ERROR;
    }
}

status_t AudioALSAHardware::SetAudioCommand(int par1, int par2) {
    ALOGD("%s(), par1 = 0x%x, par2 = %d", __FUNCTION__, par1, par2);
    switch (par1) {
    case OUTPUTFIRINDEX: {
        UpdateOutputFIR(Normal_Coef_Index, par2);
        break;
    }
    case NORMALOUTPUTFIRINDEX: {
        UpdateOutputFIR(Normal_Coef_Index, par2);
        break;
    }
    case HEADSETOUTPUTFIRINDEX: {
        UpdateOutputFIR(Headset_Coef_Index, par2);
        break;
    }
    case SPEAKEROUTPUTFIRINDEX: {
        UpdateOutputFIR(Handfree_Coef_Index, par2);
        break;
    }
    case LOAD_VOLUME_SETTING: {
        mAudioALSAVolumeController->initVolumeController();
        setMasterVolume(mAudioALSAVolumeController->getMasterVolume());
        break;
    }
    case SPEECH_VM_ENABLE: {
        ALOGD("+%s(), SPEECH_VM_ENABLE(%d)", __FUNCTION__, par2);
        bool isEpl = (par2 > 0) ? true : false;
        SpeechVMRecorder::getInstance()->configVmEpl(isEpl);
        ALOGD("-%s(), SPEECH_VM_ENABLE(%d)", __FUNCTION__, par2);
        break;
    }
    case DUMP_AUDIO_DEBUG_INFO: {
        ALOGD("%s(), DUMP_AUDIO_DEBUG_INFO(%d)", __FUNCTION__, par2);
        AudioALSAHardwareResourceManager::getInstance()->setAudioDebug(true);
        break;
    }
    case DUMP_AUDIO_AEE_CHECK: {
        ALOGD("%s(), DUMP_AUDIO_AEE_CHECK(%d)", __FUNCTION__, par2);
        if (par2 == 0) {
            property_set("vendor.streamout.aee.dump", "0");
        } else {
            property_set("vendor.streamout.aee.dump", "1");
        }
        break;
    }
    case DUMP_AUDIO_STREAM_OUT: {
        ALOGD("%s(), DUMP_AUDIO_STREAM_OUT(%d)", __FUNCTION__, par2);
        if (par2 == 0) {
            property_set("vendor.streamout.pcm.dump", "0");
            property_set("vendor.streamout.dsp.dump", "0");
            //::ioctl(mFd, AUDDRV_AEE_IOCTL, 0);
        } else {
            property_set("vendor.streamout.pcm.dump", "1");
            property_set("vendor.streamout.dsp.dump", "1");
            //::ioctl(mFd, AUDDRV_AEE_IOCTL, 1);
        }
        break;
    }
    case DUMP_AUDIO_MIXER_BUF: {
        ALOGD("%s(), DUMP_AUDIO_MIXER_BUF(%d)", __FUNCTION__, par2);
        if (par2 == 0) {
            property_set("vendor.af.mixer.pcm", "0");
        } else {
            property_set("vendor.af.mixer.pcm", "1");
        }
        break;
    }
    case DUMP_AUDIO_TRACK_BUF: {
        ALOGD("%s(), DUMP_AUDIO_TRACK_BUF(%d)", __FUNCTION__, par2);
        if (par2 == 0) {
            property_set("vendor.af.track.pcm", "0");
        } else {
            property_set("vendor.af.track.pcm", "1");
        }
        break;
    }
    case DUMP_A2DP_STREAM_OUT: {
        ALOGD("%s(), DUMP_A2DP_STREAM_OUT(%d)", __FUNCTION__, par2);
        if (par2 == 0) {
            property_set("vendor.a2dp.streamout.pcm", "0");
        } else {
            property_set("vendor.a2dp.streamout.pcm", "1");
        }
        break;
    }
    case DUMP_AUDIO_STREAM_IN: {
        ALOGD("%s(), DUMP_AUDIO_STREAM_IN(%d)", __FUNCTION__, par2);
        if (par2 == 0) {
            property_set("vendor.streamin.pcm.dump", "0");
            property_set("vendor.streamin.dsp.dump", "0");
        } else {
            property_set("vendor.streamin.pcm.dump", "1");
            property_set("vendor.streamin.dsp.dump", "1");
        }
        break;
    }
    case DUMP_IDLE_VM_RECORD: {
        ALOGD("%s(), DUMP_IDLE_VM_RECORD(%d)", __FUNCTION__, par2);
#if defined(MTK_AUDIO_HD_REC_SUPPORT)
        if (par2 == 0) {
            property_set("vendor.streamin.vm.dump", "0");
        } else {
            property_set("vendor.streamin.vm.dump", "1");
        }
#endif
        break;
    }
    case DUMP_AP_SPEECH_EPL: {
        ALOGD("%s(), DUMP_AP_SPEECH_EPL(%d)", __FUNCTION__, par2);
        if (par2 == 0) {
            property_set("vendor.streamin.epl.dump", "0");
        } else {
            property_set("vendor.streamin.epl.dump", "1");
        }

#ifdef MTK_AURISYS_FRAMEWORK_SUPPORT
        if (par2 == 0) {
            setParameters(keyAurisysLibDumpDisable);
        } else {
            setParameters(keyAurisysLibDumpEnable);
        }
#endif
        break;
    }
    case MagiASR_TEST_ENABLE: {
        ALOGD("%s(), MagiASR_TEST_ENABLE(%d)", __FUNCTION__, par2);
        if (par2 == 0) {
            //disable MagiASR verify mode
            mAudioSpeechEnhanceInfoInstance->SetForceMagiASR(false);
        } else {
            //enable MagiASR verify mode
            mAudioSpeechEnhanceInfoInstance->SetForceMagiASR(true);
        }
        break;
    }
    case AECREC_TEST_ENABLE: {
        ALOGD("%s(), AECREC_TEST_ENABLE(%d)", __FUNCTION__, par2);
        if (par2 == 0) {
            //disable AECRec verify mode
            mAudioSpeechEnhanceInfoInstance->SetForceAECRec(false);
        } else {
            //enable AECRec verify mode
            mAudioSpeechEnhanceInfoInstance->SetForceAECRec(true);
        }
        break;
    }
    case CURRENT_SENSOR_ENABLE: {
        ALOGD("%s(), CUREENT_SENSOR_ENABLE(%d)", __FUNCTION__, par2);
        AudioALSAHardwareResourceManager::getInstance()->setSPKCurrentSensor((bool)par2);
        break;
    }
    case CURRENT_SENSOR_RESET: {
        ALOGD("%s(), CURRENT_SENSOR_RESET(%d)", __FUNCTION__, par2);
        AudioALSAHardwareResourceManager::getInstance()->setSPKCurrentSensorPeakDetectorReset((bool)par2);
        break;
    }
    case TEST_AUDIODATA: {
        valAudioCmd = par2;
        ALOGD("%s(), TEST_AUDIODATA(0x%x), valAudioCmd = %d", __FUNCTION__, par1, valAudioCmd);
        break;
    }
    default:
        ALOGW("-%s(), Unknown command par1 = 0x%x, par2 = %d", __FUNCTION__, par1, par2);
        break;
    }
    return NO_ERROR;
}

status_t AudioALSAHardware::GetAudioCommand(int par1) {
    ALOGD("%s(), par1 = 0x%x", __FUNCTION__, par1);
    int result = 0 ;
    char value[PROPERTY_VALUE_MAX];

    switch (par1) {
    case OUTPUTFIRINDEX: {
        AUDIO_PARAM_MED_STRUCT pMedPara;
        mAudioCustParamClient->GetMedParamFromNV(&pMedPara);
        result = pMedPara.select_FIR_output_index[Normal_Coef_Index];
        break;
    }
    case GETAUDIOCUSTOMDATASIZE: {
        int AudioCustomDataSize = sizeof(AUDIO_VOLUME_CUSTOM_STRUCT);
        ALOGD("GETAUDIOCUSTOMDATASIZE  AudioCustomDataSize = %d", AudioCustomDataSize);
        return AudioCustomDataSize;
    }
    case NORMALOUTPUTFIRINDEX: {
        AUDIO_PARAM_MED_STRUCT pMedPara;
        mAudioCustParamClient->GetMedParamFromNV(&pMedPara);
        result = pMedPara.select_FIR_output_index[Normal_Coef_Index];
        break;
    }
    case HEADSETOUTPUTFIRINDEX: {
        AUDIO_PARAM_MED_STRUCT pMedPara;
        mAudioCustParamClient->GetMedParamFromNV(&pMedPara);
        result = pMedPara.select_FIR_output_index[Headset_Coef_Index];
        break;
    }
    case SPEAKEROUTPUTFIRINDEX: {
        AUDIO_PARAM_MED_STRUCT pMedPara;
        mAudioCustParamClient->GetMedParamFromNV(&pMedPara);
        result = pMedPara.select_FIR_output_index[Handfree_Coef_Index];
        break;
    }
    case DUMP_AUDIO_AEE_CHECK: {
        property_get("vendor.streamout.aee.dump", value, "0");
        result = atoi(value);
        ALOGD("%s(), DUMP_AUDIO_AEE_CHECK=0x%x", __FUNCTION__, result);
        break;
    }
    case DUMP_AUDIO_STREAM_OUT: {
        property_get("vendor.streamout.pcm.dump", value, "0");
        result = atoi(value);
        ALOGD("%s(), DUMP_AUDIO_STREAM_OUT=0x%x", __FUNCTION__, result);
        break;
    }
    case DUMP_AUDIO_MIXER_BUF: {
        property_get("vendor.af.mixer.pcm", value, "0");
        result = atoi(value);
        ALOGD("%s(), DUMP_AUDIO_MIXER_BUF=0x%x", __FUNCTION__, result);
        break;
    }
    case DUMP_AUDIO_TRACK_BUF: {
        property_get("vendor.af.track.pcm", value, "0");
        result = atoi(value);
        ALOGD("%s(), DUMP_AUDIO_TRACK_BUF=0x%x", __FUNCTION__, result);
        break;
    }
    case DUMP_A2DP_STREAM_OUT: {
        property_get("vendor.a2dp.streamout.pcm", value, "0");
        result = atoi(value);
        ALOGD("%s(), DUMP_A2DP_STREAM_OUT=0x%x", __FUNCTION__, result);
        break;
    }
    case DUMP_AUDIO_STREAM_IN: {
        property_get("vendor.streamin.pcm.dump", value, "0");
        result = atoi(value);
        ALOGD("%s(), DUMP_AUDIO_STREAM_IN=0x%x", __FUNCTION__, result);
        break;
    }
    case DUMP_IDLE_VM_RECORD: {
#if defined(MTK_AUDIO_HD_REC_SUPPORT)
        property_get("vendor.streamin.vm.dump", value, "0");
        result = atoi(value);
#else
        result = 0;
#endif
        ALOGD("%s(), DUMP_IDLE_VM_RECORD=0x%x", __FUNCTION__, result);
        break;
    }
    case DUMP_AP_SPEECH_EPL: {
        property_get("vendor.streamin.epl.dump", value, "0");
        result = atoi(value);

        int result1 = 0 ;
        char value1[PROPERTY_VALUE_MAX];
        property_get("vendor.streamin.epl.dump", value1, "0");
        result1 = atoi(value1);

        if (result1 == 1) {
            result = 1;
        }
        ALOGD("%s(), DUMP_AP_SPEECH_EPL=0x%x", __FUNCTION__, result);
        break;
    }
    case MagiASR_TEST_ENABLE: {
        //get the MagiASR verify mode status
        result = mAudioSpeechEnhanceInfoInstance->GetForceMagiASRState();
        ALOGD("%s(), MagiASR_TEST_ENABLE=0x%x", __FUNCTION__, result);
        break;
    }
    case AECREC_TEST_ENABLE: {
        //get the AECRec verify mode status
        result = 0;
        if (mAudioSpeechEnhanceInfoInstance->GetForceAECRecState()) {
            result = 1;
        }
        ALOGD("%s(), AECREC_TEST_ENABLE=0x%x", __FUNCTION__, result);
        break;
    }
    case TEST_AUDIODATA: {
        result = valAudioCmd;
        ALOGD("%s(), TEST_AUDIODATA(0x%x), valAudioCmd = %d", __FUNCTION__, par1, valAudioCmd);
        break;
    }

    default:
        ALOGW("-%s(), Unknown command par1=0x%x", __FUNCTION__, par1);
        break;
    }

    // call fucntion want to get status adn return it.
    return result;
}

status_t AudioALSAHardware::SetAudioCommonData(int par1, size_t len, void *ptr) {
    ALOGD("%s(), par1 = 0x%x, len = %zu", __FUNCTION__, par1, len);
    switch (par1) {
    case MEDDATA: {
        ASSERT(len == sizeof(AUDIO_PARAM_MED_STRUCT));
        mAudioCustParamClient->SetMedParamToNV((AUDIO_PARAM_MED_STRUCT *)ptr);
        break;
    }
    case AUDIOCUSTOMDATA: {
        ASSERT(len == sizeof(AUDIO_VOLUME_CUSTOM_STRUCT));
        mAudioCustParamClient->SetAudioCustomParamToNV((AUDIO_VOLUME_CUSTOM_STRUCT *)ptr);
        mAudioALSAVolumeController->initVolumeController();
        setMasterVolume(mAudioALSAVolumeController->getMasterVolume());
        break;
    }
    case DUAL_MIC_PARAMETER: {
        if (AudioALSAHardwareResourceManager::getInstance()->getNumPhoneMicSupport() >= 2) {
            ASSERT(len == sizeof(AUDIO_CUSTOM_EXTRA_PARAM_STRUCT));
            mAudioCustParamClient->SetDualMicSpeechParamToNVRam((AUDIO_CUSTOM_EXTRA_PARAM_STRUCT *)ptr);
            mAudioALSAVolumeController->initVolumeController();
            SpeechEnhancementController::GetInstance()->SetDualMicSpeechParametersToAllModem((AUDIO_CUSTOM_EXTRA_PARAM_STRUCT *)ptr);
        } else {
            ALOGW("-%s(), Unknown command par1=0x%x, len=%zu", __FUNCTION__, par1, len);
        }
        break;
    }
#if defined(MTK_MAGICONFERENCE_SUPPORT)
    case SPEECH_MAGICON_PARAMETER: {
        if (AudioALSAHardwareResourceManager::getInstance()->getNumPhoneMicSupport() >= 2) {
            ASSERT(len == sizeof(AUDIO_CUSTOM_MAGI_CONFERENCE_STRUCT));
            mAudioCustParamClient->SetMagiConSpeechParamToNVRam((AUDIO_CUSTOM_MAGI_CONFERENCE_STRUCT *)ptr);
            mAudioALSAVolumeController->initVolumeController();
            SpeechEnhancementController::GetInstance()->SetMagiConSpeechParametersToAllModem((AUDIO_CUSTOM_MAGI_CONFERENCE_STRUCT *)ptr);
        } else {
            ALOGW("-%s(), Unknown command par1=0x%x, len=%zu", __FUNCTION__, par1, len);
        }
        break;
    }
#endif
#if defined(MTK_HAC_SUPPORT)
    case SPEECH_HAC_PARAMETER: {
        ASSERT(len == sizeof(AUDIO_CUSTOM_HAC_PARAM_STRUCT));
        mAudioCustParamClient->SetHACSpeechParamToNVRam((AUDIO_CUSTOM_HAC_PARAM_STRUCT *)ptr);
        mAudioALSAVolumeController->initVolumeController();
        SpeechEnhancementController::GetInstance()->SetHACSpeechParametersToAllModem((AUDIO_CUSTOM_HAC_PARAM_STRUCT *)ptr);
        break;
    }
#endif
    case NB_SPEECH_PARAMETER: {
        ALOGD("%s() len [%zu] sizeof [%zu]", __FUNCTION__, len, sizeof(AUDIO_CUSTOM_PARAM_STRUCT));

        if (len == sizeof(AUDIO_CUSTOM_PARAM_STRUCT)) {
            AUDIO_CUSTOM_PARAM_STRUCT *pSphParamNB = (AUDIO_CUSTOM_PARAM_STRUCT *)ptr;
            mStreamManager->SetEMParameter(pSphParamNB);
        } else {
            ALOGE("len [%zu] != Sizeof(AUDIO_CUSTOM_PARAM_STRUCT) [%zu] ", len, sizeof(AUDIO_CUSTOM_PARAM_STRUCT));
        }
        break;
    }

#if defined(MTK_WB_SPEECH_SUPPORT)
    case WB_SPEECH_PARAMETER: {
        ALOGD("%s(), len [%zu] sizeof [%zu]", __FUNCTION__, len, sizeof(AUDIO_CUSTOM_WB_PARAM_STRUCT));
        ASSERT(len == sizeof(AUDIO_CUSTOM_WB_PARAM_STRUCT));
        mAudioCustParamClient->SetWBSpeechParamToNVRam((AUDIO_CUSTOM_WB_PARAM_STRUCT *)ptr);
        SpeechEnhancementController::GetInstance()->SetWBSpeechParametersToAllModem((AUDIO_CUSTOM_WB_PARAM_STRUCT *)ptr);
        mAudioALSAVolumeController->initVolumeController(); // for DRC2.0 need volume to get speech mode
        break;
    }
#endif
    case AUDIO_VER1_DATA: {
        ASSERT(len == sizeof(AUDIO_VER1_CUSTOM_VOLUME_STRUCT));

        mAudioCustParamClient->SetVolumeVer1ParamToNV((AUDIO_VER1_CUSTOM_VOLUME_STRUCT *)ptr);
        mAudioALSAVolumeController->initVolumeController();
        setMasterVolume(mAudioALSAVolumeController->getMasterVolume());
        break;
    }
    // for Audio Taste Tuning
    case AUD_TASTE_TUNING: {
        status_t ret = NO_ERROR;
        AudioTasteTuningStruct audioTasteTuningParam;
        memcpy((void *)&audioTasteTuningParam, ptr, sizeof(AudioTasteTuningStruct));

        switch (audioTasteTuningParam.cmd_type) {
        case AUD_TASTE_STOP: {
            mAudioALSAParamTunerInstance->enableModemPlaybackVIASPHPROC(false);
            audioTasteTuningParam.wb_mode = mAudioALSAParamTunerInstance->m_bWBMode;
            mAudioALSAParamTunerInstance->updataOutputFIRCoffes(&audioTasteTuningParam);

            break;
        }
        case AUD_TASTE_START: {

            mAudioALSAParamTunerInstance->setMode(audioTasteTuningParam.phone_mode);
            ret = mAudioALSAParamTunerInstance->setPlaybackFileName(audioTasteTuningParam.input_file);
            if (ret != NO_ERROR) {
                return ret;
            }
            ret = mAudioALSAParamTunerInstance->setDLPGA((uint32) audioTasteTuningParam.dlPGA);
            if (ret != NO_ERROR) {
                return ret;
            }
            mAudioALSAParamTunerInstance->updataOutputFIRCoffes(&audioTasteTuningParam);
            ret = mAudioALSAParamTunerInstance->enableModemPlaybackVIASPHPROC(true, audioTasteTuningParam.wb_mode);
            if (ret != NO_ERROR) {
                return ret;
            }

            break;
        }
        case AUD_TASTE_DLDG_SETTING:
        case AUD_TASTE_INDEX_SETTING: {
            mAudioALSAParamTunerInstance->updataOutputFIRCoffes(&audioTasteTuningParam);
            break;
        }
        case AUD_TASTE_DLPGA_SETTING: {
            mAudioALSAParamTunerInstance->setMode(audioTasteTuningParam.phone_mode);
            ret = mAudioALSAParamTunerInstance->setDLPGA((uint32) audioTasteTuningParam.dlPGA);
            if (ret != NO_ERROR) {
                return ret;
            }

            break;
        }
        default:
            break;
        }

        break;
    }
    case HOOK_FM_DEVICE_CALLBACK: {
        AudioALSAFMController::getInstance()->setFmDeviceCallback((AUDIO_DEVICE_CHANGE_CALLBACK_STRUCT *)ptr);
        break;
    }
    case UNHOOK_FM_DEVICE_CALLBACK: {
        AudioALSAFMController::getInstance()->setFmDeviceCallback(NULL);
        break;
    }
    case HOOK_BESLOUDNESS_CONTROL_CALLBACK: {
        mStreamManager->SetBesLoudnessControlCallback((BESLOUDNESS_CONTROL_CALLBACK_STRUCT *)ptr);
        break;
    }
    case UNHOOK_BESLOUDNESS_CONTROL_CALLBACK: {
        mStreamManager->SetBesLoudnessControlCallback(NULL);
        break;
    }
    case NB_SPEECH_LPBK_PARAMETER: {
        ASSERT(len == sizeof(AUDIO_CUSTOM_SPEECH_LPBK_PARAM_STRUCT));
        mAudioCustParamClient->SetNBSpeechLpbkParamToNVRam((AUDIO_CUSTOM_SPEECH_LPBK_PARAM_STRUCT *)ptr);
        break;
    }
    case VOIP_PARAMETER: {
        ASSERT(len == sizeof(AUDIO_VOIP_PARAM_STRUCT));
        mAudioCustParamClient->SetAudioVoIPParamToNV((AUDIO_VOIP_PARAM_STRUCT *)ptr);
        break;
    }
    case HFP_PARAMETER: {
        ASSERT(len == sizeof(AUDIO_HFP_PARAM_STRUCT));
        mAudioCustParamClient->SetAudioHFPParamToNV((AUDIO_HFP_PARAM_STRUCT *)ptr);
        break;
    }
    case HD_RECORD_PARAMETER: {
        ASSERT(len == sizeof(AUDIO_HD_RECORD_PARAM_STRUCT));
        mAudioCustParamClient->SetHdRecordParamToNV((AUDIO_HD_RECORD_PARAM_STRUCT *)ptr);
        break;
    }
    case HD_RECORD_SCENE_TABLE: {
        ASSERT(len == sizeof(AUDIO_HD_RECORD_SCENE_TABLE_STRUCT));
        mAudioCustParamClient->SetHdRecordSceneTableToNV((AUDIO_HD_RECORD_SCENE_TABLE_STRUCT *)ptr);
        break;
    }
    case TEST_AUDIODATA: {
        ALOGD("%s(), TEST_AUDIODATA(0x%x), len [%zu]", __FUNCTION__, par1, len);
        memcpy(pAudioBuffer, (char *) ptr,  len);
        ALOGD("%s(), TEST_AUDIODATA(0x%x), after write=0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x", __FUNCTION__, par1, pAudioBuffer[0], pAudioBuffer[1], pAudioBuffer[2], pAudioBuffer[3], pAudioBuffer[4], pAudioBuffer[5], pAudioBuffer[6], pAudioBuffer[7], pAudioBuffer[8], pAudioBuffer[9]);
        break;
    }
    default:
        ALOGW("-%s(), Unknown command par1=0x%x, len=%zu", __FUNCTION__, par1, len);
        break;
    }
    return NO_ERROR;
}

status_t AudioALSAHardware::GetAudioCommonData(int par1, size_t len, void *ptr) {
    ALOGD("%s(), par1=0x%x, len=%zu", __FUNCTION__, par1, len);
    switch (par1) {
    case MEDDATA: {
        ASSERT(len == sizeof(AUDIO_PARAM_MED_STRUCT));
        mAudioCustParamClient->GetMedParamFromNV((AUDIO_PARAM_MED_STRUCT *)ptr);
        break;
    }
    case AUDIOCUSTOMDATA: {
        ASSERT(len == sizeof(AUDIO_VOLUME_CUSTOM_STRUCT));
        mAudioCustParamClient->GetAudioCustomParamFromNV((AUDIO_VOLUME_CUSTOM_STRUCT *)ptr);
        break;
    }
    case DUAL_MIC_PARAMETER: {
        if (AudioALSAHardwareResourceManager::getInstance()->getNumPhoneMicSupport() >= 2) {
            ASSERT(len == sizeof(AUDIO_CUSTOM_EXTRA_PARAM_STRUCT));
            mAudioCustParamClient->GetDualMicSpeechParamFromNVRam((AUDIO_CUSTOM_EXTRA_PARAM_STRUCT *)ptr);
        } else {
            ALOGW("-%s(), Unknown command par1=0x%x, len=%zu", __FUNCTION__, par1, len);
        }
        break;
    }
    case NB_SPEECH_PARAMETER: {
        ALOGD("%s(), sizeof(AUDIO_CUSTOM_PARAM_STRUCT) =%zu", __FUNCTION__, sizeof(AUDIO_CUSTOM_PARAM_STRUCT));
        ASSERT(len == sizeof(AUDIO_CUSTOM_PARAM_STRUCT));
        mAudioCustParamClient->GetNBSpeechParamFromNVRam((AUDIO_CUSTOM_PARAM_STRUCT *)ptr);
        AUDIO_CUSTOM_PARAM_STRUCT *pSphParamNB = (AUDIO_CUSTOM_PARAM_STRUCT *)ptr;
        ALOGD("%s(), uAutoVM = 0x%x, uMicbiasVolt=0x%x, debug_info[0] = %u, speech_common_para[0] = %u", __FUNCTION__,
              pSphParamNB->uAutoVM, pSphParamNB->uMicbiasVolt, pSphParamNB->debug_info[0], pSphParamNB->speech_common_para[0]);
        break;
    }
#if defined(MTK_WB_SPEECH_SUPPORT)
    case WB_SPEECH_PARAMETER: {
        ASSERT(len == sizeof(AUDIO_CUSTOM_WB_PARAM_STRUCT));
        mAudioCustParamClient->GetWBSpeechParamFromNVRam((AUDIO_CUSTOM_WB_PARAM_STRUCT *) ptr);
        break;
    }
#endif
#if defined(MTK_MAGICONFERENCE_SUPPORT)
    case SPEECH_MAGICON_PARAMETER: {
        if (AudioALSAHardwareResourceManager::getInstance()->getNumPhoneMicSupport() >= 2) {
            ASSERT(len == sizeof(AUDIO_CUSTOM_MAGI_CONFERENCE_STRUCT));
            mAudioCustParamClient->GetMagiConSpeechParamFromNVRam((AUDIO_CUSTOM_MAGI_CONFERENCE_STRUCT *)ptr);
        } else {
            ALOGW("-%s(), Unknown command par1=0x%x, len=%zu", __FUNCTION__, par1, len);
        }
        break;
    }
#endif
#if defined(MTK_HAC_SUPPORT)
    case SPEECH_HAC_PARAMETER: {
        ASSERT(len == sizeof(AUDIO_CUSTOM_HAC_PARAM_STRUCT));
        mAudioCustParamClient->GetHACSpeechParamFromNVRam((AUDIO_CUSTOM_HAC_PARAM_STRUCT *)ptr);
        break;
    }
#endif
    case NB_SPEECH_LPBK_PARAMETER: {
        ASSERT(len == sizeof(AUDIO_CUSTOM_SPEECH_LPBK_PARAM_STRUCT));
        mAudioCustParamClient->GetNBSpeechLpbkParamFromNVRam((AUDIO_CUSTOM_SPEECH_LPBK_PARAM_STRUCT *)ptr);
        break;
    }
    case VOIP_PARAMETER: {
        ASSERT(len == sizeof(AUDIO_VOIP_PARAM_STRUCT));
        mAudioCustParamClient->GetAudioVoIPParamFromNV((AUDIO_VOIP_PARAM_STRUCT *)ptr);
        break;
    }
    case HFP_PARAMETER: {
        ASSERT(len == sizeof(AUDIO_HFP_PARAM_STRUCT));
        mAudioCustParamClient->GetAudioHFPParamFromNV((AUDIO_HFP_PARAM_STRUCT *)ptr);
        break;
    }
    case HD_RECORD_PARAMETER: {
        ASSERT(len == sizeof(AUDIO_HD_RECORD_PARAM_STRUCT));
        mAudioCustParamClient->GetHdRecordParamFromNV((AUDIO_HD_RECORD_PARAM_STRUCT *)ptr);
        break;
    }
    case HD_RECORD_SCENE_TABLE: {
        ASSERT(len == sizeof(AUDIO_HD_RECORD_SCENE_TABLE_STRUCT));
        mAudioCustParamClient->GetHdRecordSceneTableFromNV((AUDIO_HD_RECORD_SCENE_TABLE_STRUCT *)ptr);
        break;
    }
    case AUDIO_VER1_DATA: {
        mAudioCustParamClient->GetVolumeVer1ParamFromNV((AUDIO_VER1_CUSTOM_VOLUME_STRUCT *) ptr);
        break;
    }
    case GET_AUDIO_POLICY_VOL_FROM_VER1_DATA: {
        AUDIO_CUSTOM_VOLUME_STRUCT *pTarget = (AUDIO_CUSTOM_VOLUME_STRUCT *) ptr;

        if (len == sizeof(AUDIO_CUSTOM_VOLUME_STRUCT)) {
            AUDIO_VER1_CUSTOM_VOLUME_STRUCT Source;
            mAudioCustParamClient->GetVolumeVer1ParamFromNV(&Source);
            memcpy((void *)pTarget->audiovolume_steamtype[CUSTOM_VOL_TYPE_VOICE_CALL], (void *)Source.audiovolume_sph, sizeof(unsigned char)*NUM_OF_VOL_MODE * AUDIO_MAX_VOLUME_STEP);
#if defined(MTK_HAC_SUPPORT)
            bool mHACon = SpeechEnhancementController::GetInstance()->GetHACOn();
            if (mHACon) {
                AUDIO_CUSTOM_HAC_PARAM_STRUCT mHacParam;
                mAudioCustParamClient->GetHACSpeechParamFromNVRam(&mHacParam);
                memcpy((void *)pTarget->audiovolume_steamtype[CUSTOM_VOL_TYPE_VOICE_CALL], (void *)mHacParam.audiovolume_sph_hac, sizeof(unsigned char)* AUDIO_MAX_VOLUME_STEP);
                ALOGD("%s(): mHACon=%d, replace Receiver gain by Hac", __FUNCTION__, mHACon);
            }
#endif
            memcpy((void *)pTarget->audiovolume_steamtype[CUSTOM_VOL_TYPE_SIP], (void *)Source.audiovolume_sip, sizeof(unsigned char)*NUM_OF_VOL_MODE * AUDIO_MAX_VOLUME_STEP);

            memcpy((void *)Source.audiovolume_ring[VOLUME_NORMAL_MODE], (void *)Source.audiovolume_ring[VOLUME_HEADSET_SPEAKER_MODE], sizeof(unsigned char)*AUDIO_MAX_VOLUME_STEP);
            memcpy((void *)Source.audiovolume_ring[VOLUME_HEADSET_MODE], (void *)Source.audiovolume_ring[VOLUME_HEADSET_SPEAKER_MODE], sizeof(unsigned char)*AUDIO_MAX_VOLUME_STEP);

            memcpy((void *)pTarget->audiovolume_steamtype[CUSTOM_VOL_TYPE_RING], (void *)Source.audiovolume_ring, sizeof(unsigned char)*NUM_OF_VOL_MODE * AUDIO_MAX_VOLUME_STEP);
            memcpy((void *)pTarget->audiovolume_steamtype[CUSTOM_VOL_TYPE_ALARM], (void *)Source.audiovolume_ring, sizeof(unsigned char)*NUM_OF_VOL_MODE * AUDIO_MAX_VOLUME_STEP);
            memcpy((void *)pTarget->audiovolume_steamtype[CUSTOM_VOL_TYPE_NOTIFICATION], (void *)Source.audiovolume_ring, sizeof(unsigned char)*NUM_OF_VOL_MODE * AUDIO_MAX_VOLUME_STEP);

            memcpy((void *)pTarget->audiovolume_steamtype[CUSTOM_VOL_TYPE_MUSIC], (void *)Source.audiovolume_media, sizeof(unsigned char)*NUM_OF_VOL_MODE * AUDIO_MAX_VOLUME_STEP);
            memcpy((void *)pTarget->audiovolume_steamtype[CUSTOM_VOL_TYPE_ENFORCED_AUDIBLE], (void *)Source.audiovolume_media, sizeof(unsigned char)*NUM_OF_VOL_MODE * AUDIO_MAX_VOLUME_STEP);

            memcpy((void *)pTarget->audiovolume_steamtype[CUSTOM_VOL_TYPE_TTS], (void *)audiovolume_tts_nonspeaker, sizeof(unsigned char)*NUM_OF_VOL_MODE * AUDIO_MAX_VOLUME_STEP);
            memcpy((void *)pTarget->audiovolume_steamtype[CUSTOM_VOL_TYPE_TTS][CUSTOM_VOLUME_SPEAKER_MODE], (void *)Source.audiovolume_media[VOLUME_SPEAKER_MODE], sizeof(unsigned char)* AUDIO_MAX_VOLUME_STEP);

            memcpy((void *)pTarget->audiovolume_steamtype[CUSTOM_VOL_TYPE_BLUETOOTH_SCO], (void *)Source.audiovolume_media, sizeof(unsigned char)*NUM_OF_VOL_MODE * AUDIO_MAX_VOLUME_STEP);

            memcpy((void *)pTarget->audiovolume_steamtype[CUSTOM_VOL_TYPE_SYSTEM], (void *)audiovolume_system, sizeof(unsigned char)*NUM_OF_VOL_MODE * AUDIO_MAX_VOLUME_STEP);
            memcpy((void *)pTarget->audiovolume_steamtype[CUSTOM_VOL_TYPE_DTMF], (void *)audiovolume_dtmf, sizeof(unsigned char)*NUM_OF_VOL_MODE * AUDIO_MAX_VOLUME_STEP);

            memcpy((void *)pTarget->audiovolume_steamtype[CUSTOM_VOL_TYPE_ACCESSIBILITY], (void *)Source.audiovolume_media, sizeof(unsigned char)*NUM_OF_VOL_MODE * AUDIO_MAX_VOLUME_STEP);
            memcpy((void *)pTarget->audiovolume_steamtype[CUSTOM_VOL_TYPE_REROUTING], (void *)audiovolume_rerouting, sizeof(unsigned char)*NUM_OF_VOL_MODE * AUDIO_MAX_VOLUME_STEP);
            memcpy((void *)pTarget->audiovolume_steamtype[CUSTOM_VOL_TYPE_PATCH], (void *)audiovolume_patch, sizeof(unsigned char)*NUM_OF_VOL_MODE * AUDIO_MAX_VOLUME_STEP);

            pTarget->audiovolume_level[CUSTOM_VOL_TYPE_VOICE_CALL] = Source.audiovolume_level[VER1_VOL_TYPE_SPH];
            pTarget->audiovolume_level[CUSTOM_VOL_TYPE_SYSTEM] = Source.audiovolume_level[VER1_VOL_TYPE_MEDIA];
            pTarget->audiovolume_level[CUSTOM_VOL_TYPE_RING] = Source.audiovolume_level[VER1_VOL_TYPE_RING];
            pTarget->audiovolume_level[CUSTOM_VOL_TYPE_MUSIC] = Source.audiovolume_level[VER1_VOL_TYPE_MEDIA];
            pTarget->audiovolume_level[CUSTOM_VOL_TYPE_ALARM] = Source.audiovolume_level[VER1_VOL_TYPE_RING];
            pTarget->audiovolume_level[CUSTOM_VOL_TYPE_NOTIFICATION] = Source.audiovolume_level[VER1_VOL_TYPE_RING];
            pTarget->audiovolume_level[CUSTOM_VOL_TYPE_BLUETOOTH_SCO] = Source.audiovolume_level[VER1_VOL_TYPE_MEDIA];
            pTarget->audiovolume_level[CUSTOM_VOL_TYPE_ENFORCED_AUDIBLE] = Source.audiovolume_level[VER1_VOL_TYPE_MEDIA];
            pTarget->audiovolume_level[CUSTOM_VOL_TYPE_DTMF] = Source.audiovolume_level[VER1_VOL_TYPE_MEDIA];
            pTarget->audiovolume_level[CUSTOM_VOL_TYPE_TTS] = Source.audiovolume_level[VER1_VOL_TYPE_MEDIA];
            pTarget->audiovolume_level[CUSTOM_VOL_TYPE_SIP] = Source.audiovolume_level[VER1_VOL_TYPE_SIP];
            pTarget->audiovolume_level[CUSTOM_VOL_TYPE_ACCESSIBILITY] = pTarget->audiovolume_level[CUSTOM_VOL_TYPE_REROUTING] = pTarget->audiovolume_level[CUSTOM_VOL_TYPE_PATCH] = Source.audiovolume_level[VER1_VOL_TYPE_MEDIA];
            pTarget->bReady = 1;
            mUseTuningVolume = true;
            memcpy((void *)&VolCache, (void *)pTarget, sizeof(AUDIO_CUSTOM_VOLUME_STRUCT));
            ALOGD("Get PolicyCustomVolume");
        }
        break;
    }
    case VOICE_RECOG_FIR_COEF: {
        AUDIO_HD_RECORD_PARAM_STRUCT custHDRECParam;
        mAudioCustParamClient->GetHdRecordParamFromNV(&custHDRECParam);
        ASSERT(len == sizeof(custHDRECParam.hd_rec_fir));
        memcpy(ptr, (void *)custHDRECParam.hd_rec_fir, len);
        break;
    }
    case VOICE_RECOG_GAIN: {
        AUDIO_VER1_CUSTOM_VOLUME_STRUCT custGainParam;
        mAudioCustParamClient->GetVolumeVer1ParamFromNV(&custGainParam);
        uint16_t *pGain = (uint16_t *)ptr;
#if 0
        *pGain = 0;
        *(pGain + 1) = 0;
#else
        *pGain = mAudioALSAVolumeController->MappingToDigitalGain(custGainParam.audiovolume_mic[VOLUME_NORMAL_MODE][7]);
        *(pGain + 1) = mAudioALSAVolumeController->MappingToDigitalGain(custGainParam.audiovolume_mic[VOLUME_HEADSET_MODE][7]);
#endif
        break;
    }
#if defined(MTK_AUDIO_GAIN_TABLE) && !defined(MTK_NEW_VOL_CONTROL)
    case GET_TC1_DISP: {
        ASSERT(len == sizeof(PCDispTotolStru));
        if (NULL != mAudioALSAParamTunerInstance) {
            mAudioALSAParamTunerInstance->getGainInfoForDisp(ptr);
        }
        break;
    }
#if defined(MTK_AUDIO_GAIN_TABLE_BT)
    case GET_BT_NREC_DISP: {
        ASSERT(len == sizeof(PCDispItem));
        if (NULL != mAudioALSAParamTunerInstance) {
            mAudioALSAParamTunerInstance->getBtNrecInfoForDisp(ptr);
        }
        break;
    }
#endif
#endif
#ifdef MTK_NEW_VOL_CONTROL
    case GET_AUDIO_SCENE_GAIN_TABLE: {
        int sceneCount = AudioMTKGainController::getInstance()->getSceneCount();
        ASSERT(len == (sizeof(GainTableForScene) * sceneCount));
        return AudioMTKGainController::getInstance()->getSceneGainTableParameter((GainTableForScene *)ptr);
        break;
    }
    case GET_AUDIO_NON_SCENE_GAIN_TABLE: {
        ASSERT(len == sizeof(GainTableForNonScene));
        return AudioMTKGainController::getInstance()->getNonSceneGainTableParameter((GainTableForNonScene *)ptr);
        break;
    }
#endif
    case TEST_AUDIODATA: {
        ALOGD("%s(), TEST_AUDIODATA(0x%x), len [%zu]", __FUNCTION__, par1, len);
        memcpy((char *)ptr, pAudioBuffer, len);
        ALOGD("%s(), TEST_AUDIODATA(0x%x), read print=0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x", __FUNCTION__, par1, pAudioBuffer[0], pAudioBuffer[1], pAudioBuffer[2], pAudioBuffer[3], pAudioBuffer[4], pAudioBuffer[5], pAudioBuffer[6], pAudioBuffer[7], pAudioBuffer[8], pAudioBuffer[9]);
        break;
    }
    case AUDIO_DRC_MUSIC_PARAMETER: {
        String8 scene = mStreamManager->getCustScene();
        ALOGD("%s(), AUDIO_DRC_MUSIC_PARAMETER(0x%x), len [%zu] scene %s", __FUNCTION__, par1, len, scene.string());
        int resultLen = getAudioCompFltCustParam(AUDIO_COMP_FLT_DRC_FOR_MUSIC, (AUDIO_ACF_CUSTOM_PARAM_STRUCT *)ptr, scene.string());
        if ((int)len != resultLen) {
            ALOGE("%s(), AUDIO_DRC_MUSIC_PARAMETER(0x%x), len [%zu] != resultLen[%d] scene %s", __FUNCTION__, par1, len, resultLen, scene.string());
            return UNKNOWN_ERROR;
        }
        break;
    }
    case AUDIO_DRC_RINGTONE_PARAMETER: {
        String8 scene = mStreamManager->getCustScene();
        ALOGD("%s(), AUDIO_DRC_RINGTONE_PARAMETER(0x%x), len [%zu] scene %s", __FUNCTION__, par1, len, scene.string());
        int resultLen = getAudioCompFltCustParam(AUDIO_COMP_FLT_DRC_FOR_RINGTONE, (AUDIO_ACF_CUSTOM_PARAM_STRUCT *)ptr, scene.string());
        if ((int)len != resultLen) {
            ALOGE("%s(), AUDIO_DRC_RINGTONE_PARAMETER(0x%x), len [%zu] != resultLen[%d] scene %s", __FUNCTION__, par1, len, resultLen, scene.string());
            return UNKNOWN_ERROR;
        }
        break;
    }
    default:
        ALOGW("-%s(), Unknown command par1=0x%x, len=%zu", __FUNCTION__, par1, len);
        break;
    }
    return NO_ERROR;
}

status_t AudioALSAHardware::SetAudioData(int par1, size_t len, void *ptr) {
    ALOGV("%s(), par1 = 0x%x, len = %zu", __FUNCTION__, par1, len);
    return SetAudioCommonData(par1, len, ptr);
}

status_t AudioALSAHardware::GetAudioData(int par1, size_t len, void *ptr) {
    ALOGV("%s(), par1 = 0x%x, len = %zu", __FUNCTION__, par1, len);
    return GetAudioCommonData(par1, len, ptr);
}

String8 AudioALSAHardware::GetAudioEncodedBuffer(int TypeAudioData, size_t ByteAudioData) {
    unsigned char *pAudioData = new unsigned char[ByteAudioData + 5];  //add one status_t(4 byte)
    memset(pAudioData, 0, ByteAudioData + 5);
    //get audio data
    status_t rtnVal = GetAudioData(TypeAudioData, (size_t)ByteAudioData, pAudioData + 4);

    if (pAudioData != NULL) {
        *pAudioData = (int) rtnVal;
    }
    //encode to string
    size_t sz_in = ByteAudioData + 4;

    // Encode string to char
    size_t sz_out = Base64_OutputSize(true, sz_in);
    ALOGV("%s(), before encode (%s), sz_in(%zu), sz_out(%zu)", __FUNCTION__, pAudioData, sz_in, sz_out);

    //allocate output buffer for encode
    char *buf_enc = new char[sz_out + 1];
    memset(buf_enc, 0, sz_out + 1);
    size_t sz_enc = Base64_Encode(pAudioData, buf_enc, sz_in);
    //error check and alert
    if (sz_enc == 0) {
        ALOGE("%s(), Encode Error!!!after encode (%s), sz_in(%zu), sz_out(%zu), sz_enc(%zu)", __FUNCTION__, buf_enc, sz_in, sz_out, sz_enc);
    } else {
        ALOGD("%s(), after encode (%s), sz_in(%zu), sz_enc(%zu)", __FUNCTION__, buf_enc, sz_in, sz_enc);
    }
    if (pAudioData != NULL) {
        delete[] pAudioData;
    }

    //char to string8
    String8 valstr = String8(buf_enc, sz_enc);

    if (buf_enc != NULL) {
        delete[] buf_enc;
    }
    return valstr;
}

// ACF Preview parameter
status_t AudioALSAHardware::SetACFPreviewParameter(void *ptr, int len) {
    ALOGD("%s()", __FUNCTION__);
#if !defined(MTK_AURISYS_FRAMEWORK_SUPPORT)
    mStreamManager->SetACFPreviewParameter(ptr, len);
#else
    (void) len;
    (void *) ptr;
#endif
    return NO_ERROR;
}

status_t AudioALSAHardware::SetHCFPreviewParameter(void *ptr, int len) {
    ALOGD("%s()", __FUNCTION__);
#if !defined(MTK_AURISYS_FRAMEWORK_SUPPORT)
    mStreamManager->SetHCFPreviewParameter(ptr, len);
#else
    (void) len;
    (void *) ptr;
#endif
    return NO_ERROR;
}

//for PCMxWay Interface API
int AudioALSAHardware::xWayPlay_Start(int sample_rate __unused) {
    ALOGD("%s()", __FUNCTION__);
    return Play2Way::GetInstance()->Start();
}

int AudioALSAHardware::xWayPlay_Stop(void) {
    ALOGD("%s()", __FUNCTION__);
    return Play2Way::GetInstance()->Stop();
}

int AudioALSAHardware::xWayPlay_Write(void *buffer, int size_bytes) {
    ALOGD("%s()", __FUNCTION__);
    return Play2Way::GetInstance()->Write(buffer, size_bytes);
}

int AudioALSAHardware::xWayPlay_GetFreeBufferCount(void) {
    ALOGD("%s()", __FUNCTION__);
    return Play2Way::GetInstance()->GetFreeBufferCount();
}

int AudioALSAHardware::xWayRec_Start(int sample_rate __unused) {
    ALOGD("%s()", __FUNCTION__);
    return Record2Way::GetInstance()->Start();
}

int AudioALSAHardware::xWayRec_Stop(void) {
    ALOGD("%s()", __FUNCTION__);
    return Record2Way::GetInstance()->Stop();
}

int AudioALSAHardware::xWayRec_Read(void *buffer, int size_bytes) {
    ALOGD("%s()", __FUNCTION__);
    return Record2Way::GetInstance()->Read(buffer, size_bytes);
}

int AudioALSAHardware::setupParametersCallback(device_parameters_callback_t callback, void *cookie) {
    ALOGD("%s()", __FUNCTION__);
    int ret = NO_ERROR;
    mHwParametersCbk = callback;
    mHwParametersCbkCookie = cookie;
    if (mHwParametersCbk == NULL) {
        ALOGE("%s() mHwParametersCbk = NULL", __FUNCTION__);
        ret = BAD_VALUE;
    }
    if (mHwParametersCbkCookie == NULL) {
        ALOGE("%s() mHwParametersCbkCookie = NULL", __FUNCTION__);
        ret = BAD_VALUE;
    }
    return ret;
}

int AudioALSAHardware::setAudioParameterChangedCallback(device_audio_parameter_changed_callback_t callback, void *cookie) {
    AL_AUTOLOCK(mAudioParameterChangedHidlCallbackListLock);

    bool dupItemFound = false;
    size_t oldCallbackSize = mAudioParameterChangedHidlCallbackList.size();
    for (Vector<AudioParameterChangedHidlCallback *>::iterator iter = mAudioParameterChangedHidlCallbackList.begin(); iter != mAudioParameterChangedHidlCallbackList.end(); iter++) {
        if ((*iter)->mHidlCookie == cookie && (*iter)->mHidlCallback == callback) {
            ALOGD("%s(), duplicated item found, don't add callback again! (cookie = %p, callback = %p)", __FUNCTION__, cookie, callback);
            dupItemFound = true;
        }
    }

    if (dupItemFound == false) {
        mAudioParameterChangedHidlCallbackList.push_back(new AudioParameterChangedHidlCallback(callback, cookie));
    }

    ALOGD("%s(), add callback = 0x%p, cookie = 0x%p, callback size = %zu->%zu", __FUNCTION__, callback, cookie, oldCallbackSize, mAudioParameterChangedHidlCallbackList.size());

    return NO_ERROR;
}

int AudioALSAHardware::clearAudioParameterChangedCallback(void *cookie) {
    AL_AUTOLOCK(mAudioParameterChangedHidlCallbackListLock);

    size_t oldCallbackSize = mAudioParameterChangedHidlCallbackList.size();
    for (Vector<AudioParameterChangedHidlCallback *>::iterator iter = mAudioParameterChangedHidlCallbackList.begin(); iter != mAudioParameterChangedHidlCallbackList.end();) {
        if ((*iter)->mHidlCookie == cookie) {
            ALOGD("Find matched cookie(%p) callback(%p), remove it!, cur size = %zu", (*iter)->mHidlCookie, (*iter)->mHidlCallback, mAudioParameterChangedHidlCallbackList.size());
            delete (*iter);
            iter = mAudioParameterChangedHidlCallbackList.erase(iter);
        } else {
            iter++;
        }
    }
    ALOGD("%s(), callback size = %zu->%zu", __FUNCTION__, oldCallbackSize, mAudioParameterChangedHidlCallbackList.size());

    return NO_ERROR;
}

#ifdef MTK_AUDIO_HIERARCHICAL_PARAM_SUPPORT
void AudioALSAHardware::onXmlChangedCallback(AppHandle *appHandle, const char *audioType) {
    ALOGD("%s(), Got AudioParamParser xml changed callback from AudioParamParser. (%s)", __FUNCTION__, audioType);
#if defined(MTK_AURISYS_FRAMEWORK_SUPPORT)
    int xml_value = -1;
    int ret = 0;
    if (strcmp(audioType, "PlaybackACF") == 0) {
        xml_value = 0;
    } else if (strcmp(audioType, "PlaybackDRC") == 0) {
        xml_value = 5;
    } else if (strcmp(audioType, "PlaybackHCF") == 0) {
        xml_value = 1;
    }
    if (xml_value >= 0) {
        String8 key_value_pair = String8::format("HAL,%s,%s,KEY_VALUE,UpdateACFHCFParameters,%d=SET",
                                                 "AURISYS_SCENARIO_ALL",
                                                 "MTKBESSOUND",
                                                 xml_value);
        ret = aurisys_set_parameter(key_value_pair.string());

        if (ret == 0) {
            key_value_pair = String8::format("HAL,%s,%s,APPLY_PARAM,0=SET", "PLAYBACK_NORMAL", "MTKBESSOUND");
            aurisys_set_parameter(key_value_pair.string());
            key_value_pair = String8::format("HAL,%s,%s,APPLY_PARAM,0=SET", "PLAYBACK_LOW_LATENCY", "MTKBESSOUND");
            aurisys_set_parameter(key_value_pair.string());
        }
    }
#endif
    /* reload XML file */
    AppOps *appOps = appOpsGetInstance();
    if (appOps == NULL || appOps->appHandleReloadAudioType(appHandle, audioType) == APP_ERROR) {
        ALOGW("Reload xml fail! (appOps = 0x%p, audioType = %s)\n", appOps, audioType);
    } else {
        ALOGV("Reload XML success! (audioType = %s)\n", audioType);
    }

    /* Invoke HIDL callback */
    GetInstance()->onAudioParameterChangedCallback(audioType);
}
#endif

/* Triger all hidl callback function */
void AudioALSAHardware::onAudioParameterChangedCallback(const char *audioType) {
    AL_AUTOLOCK(mAudioParameterChangedHidlCallbackListLock);
    for (size_t i = 0; i < mAudioParameterChangedHidlCallbackList.size(); i++) {
        auto ret = mAudioParameterChangedHidlCallbackList[i]->mHidlCallback(audioType, mAudioParameterChangedHidlCallbackList[i]->mHidlCookie);
        ALOGD("%s(),  Invoke hidle callback[%zu/%zu]: %s return %d", __FUNCTION__, i + 1, mAudioParameterChangedHidlCallbackList.size(), audioType, ret);
    }
}

int AudioALSAHardware::setParametersByCallback(const String8 &keyValuePairs) {  // Static for easy use
    if (mHwParametersCbk == NULL || mHwParametersCbkCookie == NULL) {
        ALOGE("%s() mHwParametersCbk = NULL or mHwParametersCbkCookie = NULL", __FUNCTION__);
        return INVALID_OPERATION;
    } else {
        int stringlen = strlen(keyValuePairs.string());
        if (stringlen + 1 > 1024) {
            ALOGE("%s() Don't support string len > 1024, the len is %d", __FUNCTION__, stringlen);
            return BAD_VALUE;
        }
        audio_hw_device_set_parameters_callback_t data;
        memcpy(data.paramchar, keyValuePairs.string(), stringlen);
        data.paramchar[stringlen] = '\0';
        data.paramchar_len = stringlen;
        ALOGD("Start AudioSystem::setParameters by Callback: %s", keyValuePairs.string());
        mHwParametersCbk(DEVICE_CBK_EVENT_SETPARAMETERS, &data, mHwParametersCbkCookie);
        ALOGD("End AudioSystem::setParameters by Callback: %s", keyValuePairs.string());
        return NO_ERROR;
    }
}

void AudioALSAHardware::setScreenState(bool mode) {
    mStreamManager->setScreenState(mode);
}

using android::hardware::bluetooth::a2dp::V1_0::IBluetoothAudioHost;
using android::hardware::bluetooth::a2dp::V1_0::CodecConfiguration;

void AudioALSAHardware::setBluetoothAudioOffloadParam(const sp<IBluetoothAudioHost> &hostIf,
                                                      const CodecConfiguration &codecConfig,
                                                      bool on) {
    static AudioLock mSetOffloadParamLock;
    AL_AUTOLOCK(mSetOffloadParamLock);

    ALOGD("+%s() on=%d", __FUNCTION__, on);
    if (on == true) {
        ALOGD("new codecConfig is set");
        memcpy(&mBluetoothAudioOffloadCodecConfig, &codecConfig, sizeof(codecConfig));
    }
    mStreamManager->setBluetoothAudioOffloadParam(hostIf, codecConfig, on);
    AudioALSAHardwareResourceManager::getInstance()->resetA2dpFwLatency();
    AudioALSAHardwareResourceManager::getInstance()->resetA2dpDeviceLatency();
    ALOGD("-%s()", __FUNCTION__);
}

void AudioALSAHardware::setA2dpSuspendStatus(int status) {
    ALOGD("+%s() status=%d", __FUNCTION__, status);
    mStreamManager->setA2dpSuspendStatus(status);
    ALOGD("-%s()", __FUNCTION__);
}

#endif

bool AudioALSAHardware::UpdateOutputFIR(int mode, int index) {
    ALOGD("%s(),  mode = %d, index = %d", __FUNCTION__, mode, index);

    // save index to MED with different mode.
    AUDIO_PARAM_MED_STRUCT eMedPara;
    memset((void *)&eMedPara, 0, sizeof(AUDIO_PARAM_MED_STRUCT));
    mAudioCustParamClient->GetMedParamFromNV(&eMedPara);
    eMedPara.select_FIR_output_index[mode] = index;

    // copy med data into audio_custom param
    AUDIO_CUSTOM_PARAM_STRUCT eSphParamNB;
    memset((void *)&eSphParamNB, 0, sizeof(AUDIO_CUSTOM_PARAM_STRUCT));
    mAudioCustParamClient->GetNBSpeechParamFromNVRam(&eSphParamNB);

    for (int i = 0; i < NB_FIR_NUM;  i++) {
        ALOGD("eSphParamNB.sph_out_fir[%d][%d] = %d <= eMedPara.speech_output_FIR_coeffs[%d][%d][%d] = %d",
              mode, i, eSphParamNB.sph_out_fir[mode][i],
              mode, index, i, eMedPara.speech_output_FIR_coeffs[mode][index][i]);
    }

    memcpy((void *)eSphParamNB.sph_out_fir[mode],
           (void *)eMedPara.speech_output_FIR_coeffs[mode][index],
           sizeof(eSphParamNB.sph_out_fir[mode]));

    // set to nvram
    mAudioCustParamClient->SetNBSpeechParamToNVRam(&eSphParamNB);
    mAudioCustParamClient->SetMedParamToNV(&eMedPara);

    // set to modem side
    SpeechEnhancementController::GetInstance()->SetNBSpeechParametersToAllModem(&eSphParamNB);

    return true;
}

status_t AudioALSAHardware::setMasterMute(bool muted __unused) {
    return INVALID_OPERATION;
}
#ifdef MTK_SMARTPA_DUMMY_LIB
#define FM_DEVICE_TO_DEVICE_SUPPORT_OUTPUT_DEVICES (AUDIO_DEVICE_OUT_WIRED_HEADSET | AUDIO_DEVICE_OUT_WIRED_HEADPHONE)
#else
#define FM_DEVICE_TO_DEVICE_SUPPORT_OUTPUT_DEVICES (AUDIO_DEVICE_OUT_SPEAKER | AUDIO_DEVICE_OUT_WIRED_HEADSET | AUDIO_DEVICE_OUT_WIRED_HEADPHONE)
#endif
const char *strAudioPatchRole[] = {"AUDIO_PORT_ROLE_NONE", "AUDIO_PORT_ROLE_SOURCE", "AUDIO_PORT_ROLE_SINK"};
const char *strAudioPatchType[] = {"AUDIO_PORT_TYPE_NONE", "AUDIO_PORT_TYPE_DEVICE", "AUDIO_PORT_TYPE_MIX", "AUDIO_PORT_TYPE_SESSION"};

int AudioALSAHardware::createAudioPatch(unsigned int num_sources,
                                        const struct audio_port_config *sources,
                                        unsigned int num_sinks,
                                        const struct audio_port_config *sinks,
                                        audio_patch_handle_t *handle) {
    int status = NO_ERROR;
    ALOGV("+%s num_sources [%d] , num_sinks [%d]", __FUNCTION__, num_sources, num_sinks);
#if 1 //Debug
    if (handle == NULL || sources == NULL || sinks == NULL) {
        ALOGW("Ptr is null");
        return BAD_VALUE;
    }
    ALOGV("handlecheck %s handle [0x%x] current size %zu", __FUNCTION__, *handle, mAudioHalPatchVector.size());
    unsigned int i = 0, j = 0;

    for (i = 0; i < num_sources ; i++) {
        ALOGV("== source [%d]/[%d] ==", i, num_sources);
        ALOGV("id 0x%x", sources[i].id);
        ALOGV("role 0x%x %s", sources[i].role, strAudioPatchRole[sources[i].role]);
        ALOGV("type 0x%x %s", sources[i].type, strAudioPatchType[sources[i].type]);
        ALOGV("config_mask 0x%x", sources[i].config_mask);
        ALOGV("sample_rate 0x%x", sources[i].sample_rate);
        ALOGV("channel_mask 0x%x", sources[i].channel_mask);
        ALOGV("gain.index 0x%x", sources[i].gain.index);
        ALOGV("gain.mode 0x%x", sources[i].gain.mode);
        ALOGV("gain.channel_mask 0x%x", sources[i].gain.channel_mask);
        ALOGV("gain.ramp_duration_ms 0x%x", sources[i].gain.ramp_duration_ms);
#if 0 //When gain check , enable
        for (j = 0; j < sizeof(audio_channel_mask_t) * 8; j++) {
            ALOGD("gain.values[%d] 0x%x", j, sources[i].gain.values[j]);
        }
#endif
        if (sources[i].type == AUDIO_PORT_TYPE_DEVICE) {
            ALOGV("device.hw_module 0x%x", sources[i].ext.device.hw_module);
            ALOGV("device.type 0x%x", sources[i].ext.device.type);
            ALOGV("device.address %s", sources[i].ext.device.address);
        } else if (sources[i].type == AUDIO_PORT_TYPE_MIX) {
            ALOGV("mix.hw_module 0x%x", sources[i].ext.mix.hw_module);
            ALOGV("mix.handle 0x%x", sources[i].ext.mix.handle);
            ALOGV("mix.usecase.stream 0x%x", sources[i].ext.mix.usecase.stream);
            ALOGV("mix.usecase.source 0x%x", sources[i].ext.mix.usecase.source);
        } else if (sources[i].type == AUDIO_PORT_TYPE_SESSION) {

        }

    }

    for (i = 0; i < num_sinks ; i++) {
        ALOGV("== sinks [%d]/[%d] ==", i, num_sinks);
        ALOGV("id 0x%x", sinks[i].id);
        ALOGV("role 0x%x %s", sinks[i].role, strAudioPatchRole[sinks[i].role]);
        ALOGV("type 0x%x %s", sinks[i].type, strAudioPatchType[sinks[i].type]);
        ALOGV("config_mask 0x%x", sinks[i].config_mask);
        ALOGV("sample_rate 0x%x", sinks[i].sample_rate);
        ALOGV("channel_mask 0x%x", sinks[i].channel_mask);
        ALOGV("gain.index 0x%x", sinks[i].gain.index);
        ALOGV("gain.mode 0x%x", sinks[i].gain.mode);
        ALOGV("gain.channel_mask 0x%x", sinks[i].gain.channel_mask);
        ALOGV("gain.ramp_duration_ms 0x%x", sinks[i].gain.ramp_duration_ms);
#if 0 //When gain check , enable
        for (j = 0; j < sizeof(audio_channel_mask_t) * 8; j++) {
            ALOGD("gain.values[%d] 0x%x", j, sinks[i].gain.values[j]);
        }
#endif
        if (sinks[i].type == AUDIO_PORT_TYPE_DEVICE) {
            ALOGV("device.hw_module 0x%x", sinks[i].ext.device.hw_module);
            ALOGV("device.type 0x%x", sinks[i].ext.device.type);
            ALOGV("device.address %s", sinks[i].ext.device.address);
        } else if (sinks[i].type == AUDIO_PORT_TYPE_MIX) {
            ALOGV("mix.hw_module 0x%x", sinks[i].ext.mix.hw_module);
            ALOGV("mix.handle 0x%x", sinks[i].ext.mix.handle);
            ALOGV("mix.usecase.stream 0x%x", sinks[i].ext.mix.usecase.stream);
            ALOGV("mix.usecase.source 0x%x", sinks[i].ext.mix.usecase.source);
        } else if (sinks[i].type == AUDIO_PORT_TYPE_SESSION) {

        }
    }


#endif
#if 1
    //    ALOGD("+%s num_sources [%d] , num_sinks [%d]", __FUNCTION__, num_sources, num_sinks);
    audio_devices_t eOutDeviceList = AUDIO_DEVICE_NONE;
    audio_devices_t eInputDeviceList = AUDIO_DEVICE_NONE;
    audio_source_t eInputSource = AUDIO_SOURCE_DEFAULT;
    do {
        if (handle == NULL || sources == NULL || sinks == NULL) {
            ALOGW("Ptr is null");
            status = BAD_VALUE;
            break;
        }
        // We can support legacy routing with setting single source or single sink
        if ((!num_sources && !num_sinks) || (num_sources > 1) || (num_sinks > AUDIO_PATCH_PORTS_MAX)) {
            ALOGW("num is invalid");
            status = BAD_VALUE;
            break;
        }

        if (sources[0].type == AUDIO_PORT_TYPE_MIX) {

            if (sinks[0].type != AUDIO_PORT_TYPE_DEVICE) {
                ALOGW("sinks[0].type != AUDIO_PORT_TYPE_DEVICE");
                status = BAD_VALUE;
                break;
            }

            unsigned int dDeviceIndex;

            for (dDeviceIndex = 0; dDeviceIndex < num_sinks; dDeviceIndex++) {
                eOutDeviceList |= sinks[dDeviceIndex].ext.device.type;
            }
            if (eOutDeviceList == AUDIO_DEVICE_NONE) {
                ALOGW("Mixer->Device Routing to AUDIO_DEVICE_NONE");
                status = BAD_VALUE;
                break;
            }

#ifdef MTK_USB_PHONECALL
            // avoid using USB_PRIMARY in HAL
            if (eOutDeviceList == AUDIO_DEVICE_OUT_BUS) {
                eOutDeviceList = AUDIO_DEVICE_OUT_USB_DEVICE;
            }
#endif

            ALOGD("+routing createAudioPatch Mixer->%x", eOutDeviceList);
            AudioParameter param;
            param.addInt(String8(AudioParameter::keyRouting), (int)eOutDeviceList);
            status = mStreamManager->setParameters(param.toString(), sources[0].ext.mix.handle);
            //Gain Apply to do
            if (status == NO_ERROR) {
                ssize_t index;
                ssize_t total = mAudioHalPatchVector.size();
                for (index = total - 1; index >= 0; index--) {

                    if (mAudioHalPatchVector[index]->sources[0].type == AUDIO_PORT_TYPE_MIX &&
                        mAudioHalPatchVector[index]->sinks[0].type == AUDIO_PORT_TYPE_DEVICE &&
                        sources[0].ext.mix.handle == mAudioHalPatchVector[index]->sources[0].ext.mix.handle) {
                        AudioHalPatch *patch;
                        patch = mAudioHalPatchVector[index];
                        ALOGD_IF(mLogEnable, "handlecheck createAudioPatch() removing patch handle %d index %zd DL",
                                 mAudioHalPatchVector[index]->mHalHandle, index);
                        mAudioHalPatchVector.removeAt(index);
                        delete (patch);
                        break;
                    }
                }
            } else {
                ALOGE("Err %s %d", __FUNCTION__, __LINE__);
            }

        } else if (sources[0].type == AUDIO_PORT_TYPE_DEVICE) {

            if (sinks[0].type == AUDIO_PORT_TYPE_MIX) {

                eInputDeviceList = sources[0].ext.device.type;
                eInputSource = sinks[0].ext.mix.usecase.source;
                ALOGD("+routing createAudioPatch %x->Mixer Src %x", eInputDeviceList, eInputSource);
                AudioParameter param;
                param.addInt(String8(AudioParameter::keyRouting), (int)eInputDeviceList);
                param.addInt(String8(AudioParameter::keyInputSource),
                             (int)eInputSource);

                status = mStreamManager->setParameters(param.toString(), sinks[0].ext.mix.handle);
#if 0
                if (status != NO_ERROR) {
                    ALOGW("[%s] [%d]", __FUNCTION__, __LINE__);
                    break;
                } else if (eInputDeviceList == AUDIO_DEVICE_IN_FM_TUNER) {
                    if (mUseAudioPatchForFm == true) {
                        // FM enable (indirect mode)
                        mStreamManager->setFmVolume(1.0); // For be sure mFmVolume doesn't equal to -1.0
                        status = mStreamManager->setFmEnable(true, true, false);
                    }
                }

                //Gain Apply to do
#else
                if (status == NO_ERROR) {
                    ssize_t index;
                    ssize_t total = mAudioHalPatchVector.size();
                    for (index = total - 1; index >= 0; index--) {

                        if (mAudioHalPatchVector[index]->sources[0].type == AUDIO_PORT_TYPE_DEVICE &&
                            mAudioHalPatchVector[index]->sinks[0].type == AUDIO_PORT_TYPE_MIX &&
                            sinks[0].ext.mix.handle == mAudioHalPatchVector[index]->sinks[0].ext.mix.handle) {
                            AudioHalPatch *patch;
                            patch = mAudioHalPatchVector[index];
                            ALOGD_IF(mLogEnable, "handlecheck createAudioPatch() removing patch handle %d index %zd UL",
                                     mAudioHalPatchVector[index]->mHalHandle, index);
                            mAudioHalPatchVector.removeAt(index);
                            delete (patch);
                            break;
                        }
                    }

                    if (eInputDeviceList == AUDIO_DEVICE_IN_FM_TUNER) {
                        if (mUseAudioPatchForFm == true) {
                            status = mStreamManager->setFmEnable(true, true, false);
                        }
                    }
                } else {
                    ALOGE("Err %s %d", __FUNCTION__, __LINE__);
                }
#endif
            } else if (sinks[0].type == AUDIO_PORT_TYPE_DEVICE) {
                ALOGW("sinks[0].type == AUDIO_PORT_TYPE_DEVICE");
                // DO Device to Device
                eInputDeviceList = sources[0].ext.device.type;
                unsigned int dDeviceIndex;

                for (dDeviceIndex = 0; dDeviceIndex < num_sinks; dDeviceIndex++) {
                    eOutDeviceList |= sinks[dDeviceIndex].ext.device.type; //should be only one device , limited by frameworks
                }

                if (eInputDeviceList != AUDIO_DEVICE_IN_FM_TUNER || !(eOutDeviceList & FM_DEVICE_TO_DEVICE_SUPPORT_OUTPUT_DEVICES)) {
                    ALOGW("[%s] [%d]", __FUNCTION__, __LINE__);
                    status = INVALID_OPERATION;
                    break;
                } else if (eInputDeviceList == AUDIO_DEVICE_IN_FM_TUNER) {
                    if (AudioALSAFMController::getInstance()->checkFmNeedUseDirectConnectionMode() == false) {
                        ALOGW("[%s] [%d] InDirectConnectionMode", __FUNCTION__, __LINE__);
                        status = INVALID_OPERATION;
                        break;
                    }

                    if (mUseAudioPatchForFm == false) {
                        ALOGW("[%s] [%d]", __FUNCTION__, __LINE__);
                        status = INVALID_OPERATION;
                        break;
                    }
#if 1
#if 1
                    AudioParameter param;
                    param.addInt(String8(AudioParameter::keyRouting), (int)eOutDeviceList);
                    //The sources[0].ext.mix.handle doesn't record IOport
                    status = mStreamManager->setParametersToStreamOut(param.toString());

                    if (status != NO_ERROR) {
                        ALOGW("[%s] [%d]", __FUNCTION__, __LINE__);
                        break;
                    }
#endif
                    ALOGD("+routing createAudioPatch %x->%x", eInputDeviceList, eOutDeviceList);
                    mStreamManager->setFmVolume(0);//initial value is -1 , should change it first

                    // for bybpass audio hw
                    if (mAudioHWBypass) {
                        break;
                    }

                    // FM enable
                    mStreamManager->setFmEnable(false);// make sure FM disable first (FM App non-sync issue)
                    status = mStreamManager->setFmEnable(true, true, true);
#else
                    /*
                                        else if (eOutDeviceList != AUDIO_DEVICE_OUT_WIRED_HEADSET && eOutDeviceList != AUDIO_DEVICE_OUT_WIRED_HEADPHONE)
                                        {
                                            ALOGW("[%s] [%d]", __FUNCTION__, __LINE__);
                                            status = INVALID_OPERATION;
                                            break;
                                        }
                    */
                    else { // FM earphone
                        int dGainDB = 256;
                        unsigned int ramp_duration_ms = 0;

                        if (sinks[0].config_mask & AUDIO_PORT_CONFIG_GAIN) {
                            if (sinks[0].gain.mode & (AUDIO_GAIN_MODE_JOINT | AUDIO_GAIN_MODE_CHANNELS)) { //Hw support joint only
                                dGainDB = sinks[0].gain.values[0] / 100;
                                if (dGainDB >= 0) { //Support degrade only
                                    dGainDB = 0;
                                } else {
                                    dGainDB = (-1) * dGainDB;
                                }

                                dGainDB = dGainDB << 2;
                                if (dGainDB > 256) {
                                    dGainDB = 256;
                                }
                                dGainDB = 256 - dGainDB;
                            }

                            if (sinks[0].gain.mode & AUDIO_GAIN_MODE_RAMP) {
                                ramp_duration_ms = sinks[0].gain.ramp_duration_ms;
                            }
                        }

                        // routing
                        for (dDeviceIndex = 0; dDeviceIndex < num_sinks; dDeviceIndex++) {
                            eOutDeviceList |= sinks[dDeviceIndex].ext.device.type;
                        }

                        AudioParameter param;
                        param.addInt(String8(AudioParameter::keyRouting), (int)eOutDeviceList);
                        //status = mStreamManager->setParameters(param.toString(), sources[0].ext.mix.handle);

                        status = mStreamManager->setParametersToStreamOut(param.toString());//The sources[0].ext.mix.handle doesn't record IOport

                        if (status != NO_ERROR) {
                            ALOGW("[%s] [%d]", __FUNCTION__, __LINE__);
                            break;
                        } else {
#ifndef MTK_AUDIO_GAIN_TABLE
                            float fFMVolume = AudioALSAVolumeController::linearToLog(dGainDB);
                            ALOGD("fFMVolume %f", fFMVolume);

                            if (fFMVolume < 0) {
                                fFMVolume = 0;
                            } else if (fFMVolume > 1.0) {
                                fFMVolume = 1.0;
                            }
                            ALOGD("+routing createAudioPatch %x->%x", eInputDeviceList, eOutDeviceList);
                            mStreamManager->setFmVolume(fFMVolume); // initial value is -1 , should change it first
#endif
                            // FM enable
                            status = mStreamManager->setFmEnable(true, true, true);
                            //FMTODO : Gain setting

                        }
                    }
#endif
                    break;
                }
            }

        }



    } while (0);

    if (status == NO_ERROR) {
#if 0
        for (size_t index = 0; *handle != AUDIO_PATCH_HANDLE_NONE && index < mAudioHalPatchVector.size(); index++) {
            if (*handle == mAudioHalPatchVector[index]->mHalHandle) {
                ALOGV("createAudioPatch() removing patch handle %d", *handle);
                mAudioHalPatchVector.removeAt(index);
                break;
            }
        }
#endif
        *handle = static_cast<audio_patch_handle_t>(android_atomic_inc(&mNextUniqueId));
        AudioHalPatch *newPatch = new AudioHalPatch(*handle);
        newPatch->num_sources = num_sources;
        newPatch->num_sinks = num_sinks;
        for (unsigned int index = 0; index < num_sources ; index++) {
            memcpy((void *)&newPatch->sources[index], (void *)&sources[index], sizeof(struct audio_port_config));
        }
        for (unsigned int index = 0; index < num_sinks ; index++) {
            memcpy((void *)&newPatch->sinks[index], (void *)&sinks[index], sizeof(struct audio_port_config));
        }
        mAudioHalPatchVector.add(newPatch);

        ALOGV("handlecheck %s sucess new *handle 0x%x", __FUNCTION__, (int)(*handle));
    } else {
        ALOGE("Fail status %d", (int)(status));
    }
    ALOGV("-%s num_sources [%d] , num_sinks [%d]", __FUNCTION__, num_sources, num_sinks);
#endif
    return status;
}

int AudioALSAHardware::releaseAudioPatch(audio_patch_handle_t handle) {
    //TODO
    int status = NO_ERROR;
    ssize_t index;
    bool bReturnFlag = false;
    AudioHalPatch *patch;
    ALOGD_IF(mLogEnable, "%s() handle [0x%x]", __FUNCTION__, handle);
    do {

        if (handle == AUDIO_PATCH_HANDLE_NONE) {
            ALOGW("[%s] [%d]", __FUNCTION__, __LINE__);
            status = BAD_VALUE;
            return status;
        }

        for (index = 0; index < (ssize_t)mAudioHalPatchVector.size(); index++) {
            if (handle == mAudioHalPatchVector[index]->mHalHandle) {
                break;
            }
        }
        if (index == (ssize_t)mAudioHalPatchVector.size()) {
            ALOGW("[%s] [%d]", __FUNCTION__, __LINE__);
            status = INVALID_OPERATION;
            return status;
        }

        patch = mAudioHalPatchVector[index];
        mAudioHalPatchVector.removeAt(index);

        if (patch->sources[0].type == AUDIO_PORT_TYPE_MIX) {

            if (patch->sinks[0].type != AUDIO_PORT_TYPE_DEVICE) {
                ALOGW("sinks[0].type != AUDIO_PORT_TYPE_DEVICE");
                status = BAD_VALUE;
                break;
            }
            ALOGD("+routing releaseAudioPatch Mixer->%x", patch->sinks[0].ext.device.type);
#if 1
            for (index = (ssize_t)mAudioHalPatchVector.size() - 1; index >= 0; index--) {
                for (unsigned int sink_i = 0; sink_i < mAudioHalPatchVector[index]->num_sinks; sink_i++) {
                    if ((mAudioHalPatchVector[index]->sinks[sink_i].type == AUDIO_PORT_TYPE_DEVICE) &&
                        (mAudioHalPatchVector[index]->sinks[sink_i].ext.device.type != AUDIO_DEVICE_NONE)) {
                        ALOGD("Still have AudioPatches routing to outputDevice, Don't routing null Size %zu", mAudioHalPatchVector.size());
                        status = NO_ERROR;
                        bReturnFlag = true;
                        break;
                    }
                }
            }
            if (bReturnFlag) {
                break;
            }
#else
            for (index = 0; index < mAudioHalPatchVector.size(); index++) {
                for (int sink_i = 0; sink_i < mAudioHalPatchVector[index]->num_sinks; sink_i++) {
                    if (mAudioHalPatchVector[index]->sinks[sink_i].type == AUDIO_PORT_TYPE_DEVICE && mAudioHalPatchVector[index]->sinks[sink_i].ext.device.type != AUDIO_DEVICE_NONE) {
                        ALOGD("Still have AudioPatches routing to outputDevice, Don't routing null Size %d", mAudioHalPatchVector.size());
                        status = NO_ERROR;
                        bReturnFlag = true;
                        break;
                    }
                }
            }
            if (bReturnFlag) {
                break;
            }
#endif
#if 0   //Policy doesn't change to none , it will non-sync
            AudioParameter param;
            param.addInt(String8(AudioParameter::keyRouting), (int)AUDIO_DEVICE_NONE);
            status = mStreamManager->setParameters(param.toString(), patch->sources[0].ext.mix.handle);
#endif
        } else if (patch->sources[0].type == AUDIO_PORT_TYPE_DEVICE) {

            if (patch->sinks[0].type == AUDIO_PORT_TYPE_MIX) {
                // close FM if need (indirect)
                ALOGD("+routing releaseAudioPatch %x->Mixer", patch->sources[0].ext.device.type);
                if (mUseAudioPatchForFm == true) {
                    if (patch->sources[0].ext.device.type == AUDIO_DEVICE_IN_FM_TUNER) {
                        for (index = (ssize_t)mAudioHalPatchVector.size() - 1; index >= 0; index--) {
                            for (unsigned int source_i = 0; source_i < mAudioHalPatchVector[index]->num_sources; source_i++) {
                                if ((mAudioHalPatchVector[index]->sources[source_i].type == AUDIO_PORT_TYPE_DEVICE) &&
                                    (mAudioHalPatchVector[index]->sources[source_i].ext.device.type == AUDIO_DEVICE_IN_FM_TUNER)) {
                                    ALOGD("Still have AudioPatches need  AUDIO_DEVICE_IN_FM_TUNER, Don't Disable FM [%zu]",
                                          mAudioHalPatchVector.size());
                                    status = NO_ERROR;
                                    bReturnFlag = true;
                                    break;
                                }
                            }
                        }

                        if (!bReturnFlag) {
                            status = mStreamManager->setFmEnable(false);
                        }
                    }
                }

                audio_devices_t eInDeviceList = AUDIO_DEVICE_NONE;
                for (index = (ssize_t)mAudioHalPatchVector.size() - 1; index >= 0; index--) {
                    for (unsigned int source_i = 0; source_i < mAudioHalPatchVector[index]->num_sources; source_i++) {
                        if ((mAudioHalPatchVector[index]->sources[source_i].type == AUDIO_PORT_TYPE_DEVICE) &&
                            (mAudioHalPatchVector[index]->sources[source_i].ext.device.type != AUDIO_DEVICE_NONE)) {
                            eInDeviceList = mAudioHalPatchVector[index]->sources[source_i].ext.device.type;
                            ALOGD("Still have AudioPatches need  routing input device, Don't change routing [%zu]",
                                  mAudioHalPatchVector.size());
                            status = NO_ERROR;
                            break;
                        }
                    }
                }

                AudioParameter param;
                param.addInt(String8(AudioParameter::keyRouting), (int)eInDeviceList);
                status = mStreamManager->setParameters(param.toString(), patch->sinks[0].ext.mix.handle);
            } else if (patch->sinks[0].type == AUDIO_PORT_TYPE_DEVICE) {
                ALOGD_IF(mLogEnable, "+routing releaseAudioPatch %x->%x", patch->sources[0].ext.device.type, patch->sinks[0].ext.device.type);
                ALOGW("sinks[0].type == AUDIO_PORT_TYPE_DEVICE");
                if ((patch->sources[0].ext.device.type == AUDIO_DEVICE_IN_FM_TUNER) && (patch->sinks[0].ext.device.type & FM_DEVICE_TO_DEVICE_SUPPORT_OUTPUT_DEVICES)) {
                    // close FM if need (direct)
                    if (mUseAudioPatchForFm == false) {
                        ALOGW("[%s] [%d]", __FUNCTION__, __LINE__);
                        status = INVALID_OPERATION;
                    } else {
#if 0
                        for (index = mAudioHalPatchVector.size() - 1; index >= 0; index--) {
                            for (int source_i = 0; source_i < mAudioHalPatchVector[index]->num_sources; source_i++) {
                                if ((mAudioHalPatchVector[index]->sources[source_i].type == AUDIO_PORT_TYPE_DEVICE) &&
                                    (mAudioHalPatchVector[index]->sources[source_i].ext.device.type == AUDIO_DEVICE_IN_FM_TUNER)) {
                                    ALOGD("Still have AudioPatches need  AUDIO_DEVICE_IN_FM_TUNER, Don't Disable FM [%d]",
                                          mAudioHalPatchVector.size());
                                    status = NO_ERROR;
                                    bReturnFlag = true;
                                    break;
                                }
                            }
                        }
                        if (!bReturnFlag) {
                            mStreamManager->setFmVolume(0);
                            status = mStreamManager->setFmEnable(false);
                        }
#else
                        //always disable fm for direct/indirect setting pass
                        mStreamManager->setFmVolume(0);
                        status = mStreamManager->setFmEnable(false);
#endif

                    }
                    break;
                } else {
                    ALOGW("[%s] [%d]", __FUNCTION__, __LINE__);
                    status = INVALID_OPERATION;//TODO
                    break;
                }

            }
        }
    } while (0);

    if (status == NO_ERROR) {
        ALOGD("handlecheck %s remove handle [%x] OK", __FUNCTION__, handle);
        delete (patch);
    } else {
        ALOGD("handlecheck %s remove handle [%x] NG", __FUNCTION__, handle);
        mAudioHalPatchVector.add(patch);
    }
    ALOGD_IF(mLogEnable, "-%s handle [0x%x] status [%d]", __FUNCTION__, handle, status);
    return status;
}

int AudioALSAHardware::getAudioPort(struct audio_port *port __unused) {
    //TODO , I think the implementation is designed in aps.
    ALOGW("-%s Unsupport", __FUNCTION__);
    return INVALID_OPERATION;
}

// We limit valid for existing Audio port of AudioPatch
int AudioALSAHardware::setAudioPortConfig(const struct audio_port_config *config) {
    int status = NO_ERROR;

    do {

        if (config == NULL) {
            ALOGW("[%s] [%d]", __FUNCTION__, __LINE__);
            status = BAD_VALUE;
            break;
        }

        if ((config->config_mask & AUDIO_PORT_CONFIG_GAIN) == 0) {
            ALOGW("[%s] [%d]", __FUNCTION__, __LINE__);
            status = INVALID_OPERATION;
            break;
        }

        ALOGV("%s", __FUNCTION__);
        ALOGD("%s(), config->type [0x%x]", __FUNCTION__, config->type);
        ALOGD("%s(), config->role [0x%x]", __FUNCTION__, config->role);
        ALOGV("%s(), config->gain.mode [0x%x]", __FUNCTION__, config->gain.mode);
        ALOGV("%s(), config->gain.values[0] [0x%x]", __FUNCTION__, config->gain.values[0]);
        ALOGV("%s(), config->gain.ramp_duration_ms [0x%x]", __FUNCTION__, config->gain.ramp_duration_ms);

        if (config->type == AUDIO_PORT_TYPE_MIX) {
            if (config->role == AUDIO_PORT_ROLE_SOURCE) {
                //Apply Gain to MEMIF , don't support it so far
                ALOGW("[%s] [%d]", __FUNCTION__, __LINE__);
                status = INVALID_OPERATION;
                break;
            } else if (config->role == AUDIO_PORT_ROLE_SINK) {
                ALOGW("[%s] [%d]", __FUNCTION__, __LINE__);
                status = INVALID_OPERATION;
                break;
            } else {
                ALOGW("[%s] [%d]", __FUNCTION__, __LINE__);
                status = BAD_VALUE;
                break;
            }
        } else if (config->type == AUDIO_PORT_TYPE_DEVICE) {
            if (mUseAudioPatchForFm == false) {
                ALOGW("[%s] [%d]", __FUNCTION__, __LINE__);
                status = INVALID_OPERATION;
                break;
            }
            if (config->role == AUDIO_PORT_ROLE_SINK) {
                //Support specific device eg. headphone/speaker
                size_t indexOfPatch;
                size_t indexOfSink;
                audio_port_config *pstCurConfig = NULL;
                bool bhit = false;
                for (indexOfPatch = 0; indexOfPatch < mAudioHalPatchVector.size() && !bhit; indexOfPatch++) {
                    for (indexOfSink = 0; indexOfSink < mAudioHalPatchVector[indexOfPatch]->num_sinks; indexOfSink++) {
                        if ((config->ext.device.type == mAudioHalPatchVector[indexOfPatch]->sinks[indexOfSink].ext.device.type)
                            && (mAudioHalPatchVector[indexOfPatch]->sources[0].ext.device.type == AUDIO_DEVICE_IN_FM_TUNER)
                            && (mAudioHalPatchVector[indexOfPatch]->sinks[indexOfSink].ext.device.type & FM_DEVICE_TO_DEVICE_SUPPORT_OUTPUT_DEVICES)) {
                            bhit = true;
                            pstCurConfig = &(mAudioHalPatchVector[indexOfPatch]->sinks[indexOfSink]);
                            break;
                        }
                    }
                }

                if (!bhit || pstCurConfig == NULL) {
                    ALOGW("[%s] [%d]", __FUNCTION__, __LINE__);
                    status = INVALID_OPERATION;
                    break;
                }

                if (!config->gain.mode) {
                    ALOGW("[%s] [%d]", __FUNCTION__, __LINE__);
                    status = INVALID_OPERATION;
                    break;
                }
#if 0
                int dGainDB = 0;
                unsigned int ramp_duration_ms = 0;
                if (config->gain.mode & (AUDIO_GAIN_MODE_JOINT | AUDIO_GAIN_MODE_CHANNELS)) { //Hw support joint only
                    dGainDB = config->gain.values[0] / 100;
                    if (dGainDB >= 0) { //Support degrade only
                        dGainDB = 0;
                    } else {
                        dGainDB = (-1) * dGainDB;
                    }

                    dGainDB = dGainDB << 2;
                    if (dGainDB > 256) {
                        dGainDB = 256;
                    }
                    dGainDB = 256 - dGainDB;
                }

                if (config->gain.mode & AUDIO_GAIN_MODE_RAMP) {
                    ramp_duration_ms = config->gain.ramp_duration_ms;
                }

#ifndef MTK_AUDIO_GAIN_TABLE
                //FMTODO : Gain setting
                float fFMVolume = AudioALSAVolumeController::linearToLog(dGainDB);
                ALOGD("fFMVolume %f", fFMVolume);
                if (fFMVolume >= 0 && fFMVolume <= 1.0) {
                    mStreamManager->setFmVolume(fFMVolume);
                } else {
                    ALOGW("[%s] [%d]", __FUNCTION__, __LINE__);
                    status = BAD_VALUE;
                    break;
                }
#endif
#else
                int dGainDB = 0;
                unsigned int ramp_duration_ms = 0;
                float fFMVolume;
#ifdef MTK_NEW_VOL_CONTROL
                fFMVolume = AudioMTKGainController::getInstance()->GetDigitalLogGain(config->gain.values[0],
                                                                                     pstCurConfig->ext.device.type,
                                                                                     AUDIO_STREAM_MUSIC);
#else
                if (config->gain.mode & (AUDIO_GAIN_MODE_JOINT | AUDIO_GAIN_MODE_CHANNELS)) { //Hw support joint only
                    fFMVolume = MappingFMVolofOutputDev(config->gain.values[0], pstCurConfig->ext.device.type);
                } else {
#ifndef MTK_AUDIO_GAIN_TABLE
                    fFMVolume = AudioALSAVolumeController::linearToLog(dGainDB);
#else
                    fFMVolume = AudioMTKGainController::linearToLog(dGainDB);
#endif
                }

                if (config->gain.mode & AUDIO_GAIN_MODE_RAMP) {
                    ramp_duration_ms = config->gain.ramp_duration_ms;
                }
#endif
                if (fFMVolume >= 0 && fFMVolume <= 1.0) {
                    mStreamManager->setFmVolume(fFMVolume);
                } else {
                    ALOGW("[%s] [%d]", __FUNCTION__, __LINE__);
                    status = BAD_VALUE;
                    break;
                }
#endif

            } else {
                ALOGW("[%s] [%d]", __FUNCTION__, __LINE__);
                status = BAD_VALUE;
                break;
            }
        }


    } while (0);

    //TODO
    return status;
}

float AudioALSAHardware::MappingFMVolofOutputDev(int Gain, audio_devices_t eOutput) {
    float fFMVolume;
    if ((eOutput & FM_DEVICE_TO_DEVICE_SUPPORT_OUTPUT_DEVICES) == 0) {
        ALOGE("Error FM createAudioPatch direct mode fail device [0x%x]", eOutput);
        return 1.0;
    }

    if (mUseTuningVolume == false) {
        int dGainDB = 0;
        dGainDB = Gain / 100;
        if (dGainDB >= 0) { //Support degrade only
            dGainDB = 0;
        } else {
            dGainDB = (-1) * dGainDB;
        }
        dGainDB = dGainDB << 2;
        if (dGainDB > 256) {
            dGainDB = 256;
        }
        dGainDB = 256 - dGainDB;

#ifndef MTK_AUDIO_GAIN_TABLE
        fFMVolume = AudioALSAVolumeController::linearToLog(dGainDB);
#else
        fFMVolume = AudioMTKGainController::linearToLog(dGainDB);
#endif

        ALOGD("default f fFMVolume %f", fFMVolume);
        if (fFMVolume < 0) {
            fFMVolume = 0;
        } else if (fFMVolume > 1.0) {
            fFMVolume = 1.0;
        }
    } else {
        const float fCUSTOM_VOLUME_MAPPING_STEP = 256.0f;
        unsigned char *array;

        if (eOutput & AUDIO_DEVICE_OUT_SPEAKER) {
            array = VolCache.audiovolume_steamtype[CUSTOM_VOL_TYPE_MUSIC][CUSTOM_VOLUME_SPEAKER_MODE];
        } else {
            array = VolCache.audiovolume_steamtype[CUSTOM_VOL_TYPE_MUSIC][CUSTOM_VOLUME_HEADSET_MODE];
        }

        int dIndex = 15 - (((-1) * Gain) / 300);
        int dMaxIndex = VolCache.audiovolume_level[CUSTOM_VOL_TYPE_MUSIC];

        if (dIndex > 15) {
            dIndex = 15;
        } else if (dIndex < 0) {
            dIndex = 0;
        }
        float vol = (fCUSTOM_VOLUME_MAPPING_STEP * dIndex) / dMaxIndex;
        float volume = 0.0;
        if (vol == 0) {
            volume = vol;
        } else {    // map volume value to custom volume
            float unitstep = fCUSTOM_VOLUME_MAPPING_STEP / dMaxIndex;
            if (vol < fCUSTOM_VOLUME_MAPPING_STEP / dMaxIndex) {
                volume = array[0];
            } else {
                int Index = (vol + 0.5) / unitstep;
                vol -= (Index * unitstep);
                float Remind = (1.0 - (float)vol / unitstep);
                if (Index != 0) {
                    volume = ((array[Index]  - (array[Index] - array[Index - 1]) * Remind) + 0.5);
                } else {
                    volume = 0;
                }
            }
            // -----clamp for volume
            if (volume > 253.0) {
                volume = fCUSTOM_VOLUME_MAPPING_STEP;
            } else if (volume <= array[0]) {
                volume = array[0];
            }
        }

#ifndef MTK_AUDIO_GAIN_TABLE
        fFMVolume = AudioALSAVolumeController::linearToLog(volume);
#else
        fFMVolume = AudioMTKGainController::linearToLog(volume);
#endif

    }
    ALOGD("%s(), Final fFMVolume %f", __FUNCTION__, fFMVolume);
    return fFMVolume;
}

AudioALSAHardware *AudioALSAHardware::mAudioALSAHardware = 0;

AudioALSAHardware *AudioALSAHardware::GetInstance() {
    static AudioLock mGetInstanceLock;
    AL_AUTOLOCK(mGetInstanceLock);

    if (mAudioALSAHardware == 0) {
        ALOGD("+mAudioALSAHardware");
        mAudioALSAHardware = new AudioALSAHardware();
        ALOGD("-mAudioALSAHardware");
    }
    ASSERT(mAudioALSAHardware != NULL);
    return mAudioALSAHardware;
}

AudioMTKStreamOutInterface *AudioALSAHardware::openOutputStreamWithFlags(uint32_t devices,
                                                                         audio_output_flags_t flags,
                                                                         int *format,
                                                                         uint32_t *channels,
                                                                         uint32_t *sampleRate,
                                                                         status_t *status) {
    return mStreamManager->openOutputStream(devices, format, channels, sampleRate, status, flags);
}


};

AudioMTKStreamOutInterface::~AudioMTKStreamOutInterface() {}
AudioMTKStreamInInterface::~AudioMTKStreamInInterface() {}


AudioMTKHardwareInterface *AudioMTKHardwareInterface::create() {
    /*
     * FIXME: This code needs to instantiate the correct audio device
     * interface. For now - we use compile-time switches.
     */
    AudioMTKHardwareInterface *hw = 0;
    char value[PROPERTY_VALUE_MAX];

    ALOGV("Creating MTK AudioHardware");
    //hw = new android::AudioALSAHardware();
    hw = android::AudioALSAHardware::GetInstance();

    return hw;

}

extern "C" AudioMTKHardwareInterface *createMTKAudioHardware() {
    /*
     * FIXME: This code needs to instantiate the correct audio device
     * interface. For now - we use compile-time switches.
     */
    return AudioMTKHardwareInterface::create();

}




