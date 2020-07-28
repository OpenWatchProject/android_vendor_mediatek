#ifndef ANDROID_AUDIO_MTK_DEVICE_STRING_H
#define ANDROID_AUDIO_MTK_DEVICE_STRING_H

#include <stdint.h>
#include <sys/types.h>

#include <utils/Mutex.h>
#include <utils/String8.h>
#include "AudioType.h"
#include <utils/KeyedVector.h>

namespace android {
static String8 keyCardName = String8("mtsndcard");

static String8 keypcmDl1Meida = String8("MultiMedia1_PLayback");
static String8 keypcmUl1Capture = String8("MultiMedia1_Capture");
static String8 keypcmPcm2voice = String8("PCM2_PLayback");
static String8 keypcmHDMI = String8("HMDI_PLayback");
static String8 keypcmUlDlLoopback = String8("ULDL_Loopback");
static String8 keypcmI2Splayback = String8("I2S0_PLayback");
static String8 keypcmMRGrxPlayback = String8("MRGRX_PLayback");
static String8 keypcmMRGrxCapture = String8("MRGRX_CAPTURE");
static String8 keypcmFMI2SPlayback = String8("FM_I2S_Playback");
static String8 keypcmFMI2SCapture = String8("FM_I2S_Capture");
static String8 keypcmI2S0Dl1Playback = String8("I2S0DL1_PLayback");
static String8 keypcmDl1SpkPlayback = String8("DL1SCPSPK_PLayback");
static String8 keypcmScpVoicePlayback = String8("SCPVoice_PLayback");
static String8 keypcmCS43130 = String8("CS43130_Stream");
static String8 keypcmCS35L35 = String8("CS35L35_Stream");
static String8 keypcmDl1AwbCapture = String8("DL1_AWB_Record");
static String8 keypcmVoiceCallBT = String8("Voice_Call_BT_Playback");
static String8 keypcmVOIPCallBTPlayback = String8("VOIP_Call_BT_Playback");
static String8 keypcmVOIPCallBTCapture = String8("VOIP_Call_BT_Capture");
static String8 keypcmTDMLoopback = String8("TDM_Debug_Record");
static String8 keypcmMRGTxPlayback = String8("FM_MRGTX_Playback");
static String8 keypcmUl2Capture = String8("MultiMediaData2_Capture");
static String8 keypcmI2SAwbCapture = String8("I2S0AWB_Capture");
static String8 keypcmMODADCI2S = String8("ANC_Debug_Record_MOD");
static String8 keypcmADC2AWB = String8("ANC_Debug_Record_ADC2");
static String8 keypcmIO2DAI = String8("ANC_Debug_Record_IO2");
static String8 keypcmHpimpedancePlayback = String8("HP_IMPEDANCE_Playback");
static String8 keypcmModomDaiCapture = String8("Moddai_Capture");
static String8 keypcmOffloadGdmaPlayback = String8("OFFLOAD_GDMA_Playback");
static String8 keypcmDl2Meida = String8("MultiMedia2_PLayback");    //DL2 playback
static String8 keypcmDl3Meida = String8("MultiMedia3_PLayback");        //DL3 playback
static String8 keypcmBTCVSDCapture = String8("BTCVSD");
static String8 keypcmBTCVSDPlayback = String8("BTCVSD");
static String8 keypcmExtSpkMeida = String8("Speaker_PLayback");
static String8 keypcmVoiceMD1 = String8("Voice_MD1_PLayback");
static String8 keypcmVoiceMD2 = String8("Voice_MD2_PLayback");
static String8 keypcmVoiceMD1BT = String8("Voice_MD1_BT_Playback");
static String8 keypcmVoiceMD2BT = String8("Voice_MD2_BT_Playback");
static String8 keypcmVoiceUltra = String8("Voice_ULTRA_PLayback");
static String8 keypcmVoiceUSB = String8("Voice_USB_PLayback");
static String8 keypcmVoiceUSBEchoRef = String8("Voice_USB_EchoRef");
static String8 keypcmI2S2ADCCapture = String8("I2S2ADC2_Capture");
static String8 keypcmVoiceDaiCapture = String8("Voice_Dai_Capture");
static String8 keypcmOffloadPlayback = String8("Offload_Playback");
static String8 keypcmExtHpMedia = String8("Headphone_PLayback");
static String8 keypcmDL1DATA2PLayback = String8("Deep_Buffer_PLayback");

#if defined(MTK_AUDIO_KS)
static String8 keypcmPlayback1 = String8("Playback_1");
static String8 keypcmPlayback2 = String8("Playback_2");
static String8 keypcmPlayback3 = String8("Playback_3");
static String8 keypcmPlayback4 = String8("Playback_4");
static String8 keypcmPlayback5 = String8("Playback_5");
static String8 keypcmPlayback6 = String8("Playback_6");
static String8 keypcmPlayback8 = String8("Playback_8");
static String8 keypcmPlayback12 = String8("Playback_12");

static String8 keypcmCapture1 = String8("Capture_1");
static String8 keypcmCapture2 = String8("Capture_2");
static String8 keypcmCapture3 = String8("Capture_3");
static String8 keypcmCapture4 = String8("Capture_4");
static String8 keypcmCapture6 = String8("Capture_6");
static String8 keypcmCapture7 = String8("Capture_7");
static String8 keypcmCapture8 = String8("Capture_8");

static String8 keypcmCaptureMono1 = String8("Capture_Mono_1");

static String8 keypcmHostlessFm = String8("Hostless_FM");
static String8 keypcmHostlessLpbk = String8("Hostless_LPBK");
static String8 keypcmHostlessSpeech = String8("Hostless_Speech");
static String8 keypcmHostlessSphEchoRef = String8("Hostless_Sph_Echo_Ref");
static String8 keypcmHostlessSpkInit = String8("Hostless_Spk_Init");
static String8 keypcmHostlessADDADLI2SOut = String8("Hostless_ADDA_DL_I2S_OUT");
static String8 keypcmHostlessSRCBargein = String8("Hostless_SRC_Bargein");

static String8 keypcmDeepBuffer = String8(PCM_DEEP_BUFFER);

#if defined(MTK_AUDIODSP_SUPPORT)
static String8 keypcmPlaybackDspprimary = String8("DSP_Playback_Primary");
static String8 keypcmPlaybackDspVoip    = String8("DSP_Playback_Voip");
static String8 keypcmPlaybackDspDeepbuf = String8("DSP_Playback_DeepBuf");
static String8 keypcmPlaybackDsp        = String8("DSP_Playback_Playback");
static String8 keypcmPlaybackDspMixer1  = String8("DSP_Playback_Swmixer1");
static String8 keypcmPlaybackDspMixer2  = String8("DSP_Playback_Swmixer2");
static String8 keypcmCaptureDspUl1      = String8("DSP_Capture_Ul1");
static String8 keypcmPlaybackDspA2DP    = String8("DSP_Playback_A2DP");
static String8 keypcmPlaybackDspDataProvider    = String8("DSP_Playback_DataProvider");
static String8 keypcmCallfinalDsp       = String8("DSP_Call_Final");
static String8 keypcmPlaybackDspKtv    = String8("DSP_Playback_Ktv");
#endif
#if defined(MTK_VOW_SUPPORT)
static String8 keypcmVOWCapture = String8("VOW_Capture");
#endif

#if defined(MTK_AUDIO_SMARTPASCP_SUPPORT)
static String8 keypcmScpSpkPlayback = String8("SCP_SPK_Playback");
#endif

#endif
static String8 keypcmVOWBargeInCapture = String8("VOW_Barge_In_Capture");
}

#endif
