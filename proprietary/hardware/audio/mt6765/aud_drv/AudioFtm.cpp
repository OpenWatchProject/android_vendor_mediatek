/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

/*******************************************************************************
 *
 * Filename:
 * ---------
 *   AudioFtm.cpp
 *
 * Project:
 * --------
 *   Android Audio Driver
 *
 * Description:
 * ------------
 *   Factory mode Control
 *
 * Author:
 * -------
 *   Chipeng Chang (mtk02308)
 *
 *------------------------------------------------------------------------------
 * $Revision: #5 $
 * $Modtime:$
 * $Log:$
 *
 * 04 10 2012 weiguo.li
 * [ALPS00266592] [Need Patch] [Volunteer Patch] ICS_MP patchback to ALPS.ICS of Audio
 * .
 *
 * 01 11 2012 donglei.ji
 * [ALPS00106007] [Need Patch] [Volunteer Patch]DMNR acoustic loopback feature
 * .
 *
 *
 *******************************************************************************/

/*****************************************************************************
*                     C O M P I L E R   F L A G S
******************************************************************************
*/


/*****************************************************************************
*                E X T E R N A L   R E F E R E N C E S
******************************************************************************
*/

#include <string.h>
#include <stdint.h>
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sched.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <utils/String8.h>
#include <utils/Log.h>
#include <linux/hdmitx.h>
#include <linux/mtkfb_info.h>

#include "AudioALSAFMController.h"
#include "AudioFtm.h"
#include "AudioUtility.h"
#include "AudioDef.h"
#include "AudioType.h"

#include <AudioLock.h>

#include "AudioALSAStreamOut.h"
#include "AudioSmartPaController.h"
#include "AudioALSAHardwareResourceManager.h"

#include "AudioALSAStreamManager.h"

#include "LoopbackManager.h"

#include "audio_custom_exp.h"


#include <tinyalsa/asoundlib.h> // TODO(Harvey): remove later
#include "AudioALSADriverUtility.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "AudioFtm"

/*****************************************************************************
*                          C O N S T A N T S
******************************************************************************
*/


/*****************************************************************************
*                          D A T A   T Y P E S
******************************************************************************
*/

enum SPEAKER_CHANNEL {
    Channel_None = 0,
    Channel_Right,
    Channel_Left,
    Channel_Stereo
};


/*****************************************************************************
*                        F U N C T I O N   D E F I N I T I O N
******************************************************************************
*/

static struct mixer *mMixer; // TODO(Harvey): move it to AudioALSAHardwareResourceManager later

namespace android {

AudioFtm *AudioFtm::mAudioFtm = 0;
AudioFtm *AudioFtm::getInstance() {
    static AudioLock mGetInstanceLock;
    AL_AUTOLOCK(mGetInstanceLock);

    if (mAudioFtm == 0) {
        mAudioFtm = new AudioFtm();
    }
    ASSERT(mAudioFtm != NULL);
    return mAudioFtm;
}

AudioFtm::AudioFtm() :
    mStreamManager(AudioALSAStreamManager::getInstance()),
    mStreamOut(NULL),
    mStreamIn(NULL),
    mLoopbackManager(LoopbackManager::GetInstance()),
    mHardwareResourceManager(AudioALSAHardwareResourceManager::getInstance()) {
    ALOGD("%s()", __FUNCTION__);

    // TODO(Harvey): tmp, remove it later
    mMixer = AudioALSADriverUtility::getInstance()->getMixer();
    memset((void *)&m_WaveThread, 0, sizeof(m_WaveThread));
    memset((void *)&mAudioSinWave_thread, 0, sizeof(mAudioSinWave_thread));
    memset((void *)&IdxAudioPattern, 0, sizeof(IdxAudioPattern));
    memset((void *)&SizeAudioPattern, 0, sizeof(SizeAudioPattern));
    memset((void *)&mDataTable, 0, sizeof(mDataTable));
    memset((void *)&mDataBuffer, 0, sizeof(mDataBuffer));

    ALOGD("mMixer = %p", mMixer);
    ASSERT(mMixer != NULL);
}

AudioFtm::~AudioFtm() {
    ALOGD("%s()", __FUNCTION__);
}

int AudioFtm::SineGenTest(char sinegen_test) {
    ALOGD("%s(), sinegen_test = %d", __FUNCTION__, sinegen_test);

    if (sinegen_test) { // enable
        mHardwareResourceManager->setSgenMode(SGEN_MODE_O03_O04);
        mHardwareResourceManager->setSgenSampleRate(SGEN_MODE_SAMPLERATE_32000HZ);
    } else { // disable
        mHardwareResourceManager->setSgenMode(SGEN_MODE_DISABLE);
    }

    return true;
}

/// Output device test
int AudioFtm::RecieverTest(char receiver_test) {
    ALOGD("%s(), receiver_test = %d", __FUNCTION__, receiver_test);

    if (receiver_test) { // enable
        mHardwareResourceManager->openAddaOutput(32000);
        mHardwareResourceManager->startOutputDevice(AUDIO_DEVICE_OUT_EARPIECE, 32000);
        mHardwareResourceManager->setSgenMode(SGEN_MODE_O03_O04);
        mHardwareResourceManager->setSgenSampleRate(SGEN_MODE_SAMPLERATE_32000HZ);
    } else { // disable
        mHardwareResourceManager->setSgenMode(SGEN_MODE_DISABLE);
        mHardwareResourceManager->stopOutputDevice();
        mHardwareResourceManager->closeAddaOutput();
    }

    return true;
}

int AudioFtm::LouderSPKTest(char left_channel, char right_channel) {
    ALOGD("%s(), left_channel = %d, right_channel = %d", __FUNCTION__, left_channel, right_channel);

    if (left_channel != false || right_channel != false) { // enable
        mHardwareResourceManager->openAddaOutput(32000);
        mHardwareResourceManager->startOutputDevice(AUDIO_DEVICE_OUT_SPEAKER, 32000);
        mHardwareResourceManager->setSgenMode(SGEN_MODE_I05_I06);
        mHardwareResourceManager->setSgenSampleRate(SGEN_MODE_SAMPLERATE_32000HZ);
    } else { // disable
        mHardwareResourceManager->setSgenMode(SGEN_MODE_DISABLE);
        mHardwareResourceManager->stopOutputDevice();
        mHardwareResourceManager->closeAddaOutput();
    }

    return true;
}

int AudioFtm::EarphoneTest(char bEnable) {
    ALOGD("%s(), bEnable = %d", __FUNCTION__, bEnable);

    if (bEnable) { // enable
        mHardwareResourceManager->openAddaOutput(32000);
        mHardwareResourceManager->startOutputDevice(AUDIO_DEVICE_OUT_WIRED_HEADPHONE, 32000);
        mHardwareResourceManager->setSgenMode(SGEN_MODE_O03_O04);
        mHardwareResourceManager->setSgenSampleRate(SGEN_MODE_SAMPLERATE_32000HZ);
    } else { // disable
        mHardwareResourceManager->setSgenMode(SGEN_MODE_DISABLE);
        mHardwareResourceManager->stopOutputDevice();
        mHardwareResourceManager->closeAddaOutput();
    }
    return true;
}

int AudioFtm::EarphoneTestLR(char bLR) {
    if (bLR) { // Right channel
        mHardwareResourceManager->setSgenMode(SGEN_MODE_O04);
    } else { // Left channel
        mHardwareResourceManager->setSgenMode(SGEN_MODE_O03);
    }
    return true;
}

/// Input device test
int AudioFtm::SpecificBuildInMicTest(char type) {
    ALOGD("%s() micType = %c", __FUNCTION__, type);
    switch (type) {
    case SELECT_MIC1:
        mHardwareResourceManager->setBuiltInMicSpecificType(BUILTIN_MIC_MIC1_ONLY);
        break;
    case SELECT_MIC2:
        mHardwareResourceManager->setBuiltInMicSpecificType(BUILTIN_MIC_MIC2_ONLY);
        break;
    case SELECT_MIC3:
        mHardwareResourceManager->setBuiltInMicSpecificType(BUILTIN_MIC_MIC3_ONLY);
        break;
    case SELECT_MIC4:
        mHardwareResourceManager->setBuiltInMicSpecificType(BUILTIN_MIC_MIC4_ONLY);
        break;
    case SELECT_MIC5:
        mHardwareResourceManager->setBuiltInMicSpecificType(BUILTIN_MIC_MIC5_ONLY);
        break;
    default:
        mHardwareResourceManager->setBuiltInMicSpecificType(BUILTIN_MIC_DEFAULT);
        break;
    }
    return true;
}

/// Speaker over current test
int AudioFtm::Audio_READ_SPK_OC_STA(void) {
    ALOGD("%s()", __FUNCTION__);
    struct mixer_ctl *ctl;
    int retval, dValue;
    ctl = mixer_get_ctl_by_name(mMixer, "Audio_Speaker_OC_Falg");
    if (NULL == ctl) {
        ALOGD("[%s] [%d]", __FUNCTION__, __LINE__);
        return true; //true means SPK OC fail
    }
    dValue = mixer_ctl_get_value(ctl, 0);
    ALOGD("-%s() value [0x%x]", __FUNCTION__, dValue);
    return dValue;
}
int AudioFtm::LouderSPKOCTest(char left_channel, char right_channel) {
    ALOGD("%s()", __FUNCTION__);

    LouderSPKTest(left_channel, right_channel);

    return true;
}


/// Loopback
int AudioFtm::PhoneMic_Receiver_Loopback(char echoflag) {
    ALOGD("%s(), echoflag = %d", __FUNCTION__, echoflag);

    if (echoflag == MIC1_ON) { // enable
        mLoopbackManager->SetLoopbackOn(AP_MAIN_MIC_AFE_LOOPBACK, LOOPBACK_OUTPUT_RECEIVER);
    } else if (echoflag == MIC2_ON) { // enable
        mLoopbackManager->SetLoopbackOn(AP_REF_MIC_AFE_LOOPBACK, LOOPBACK_OUTPUT_RECEIVER);
    } else { // disable
        mLoopbackManager->SetLoopbackOff();
    }

    return true;
}

int AudioFtm::PhoneMic_EarphoneLR_Loopback(char echoflag) {
    ALOGD("%s(), echoflag = %d", __FUNCTION__, echoflag);

    if (echoflag == MIC1_ON) { // enable
        mLoopbackManager->SetLoopbackOn(AP_MAIN_MIC_AFE_LOOPBACK, LOOPBACK_OUTPUT_EARPHONE);
    } else if (echoflag == MIC2_ON) { // enable
        mLoopbackManager->SetLoopbackOn(AP_REF_MIC_AFE_LOOPBACK, LOOPBACK_OUTPUT_EARPHONE);
    } else { // disable
        mLoopbackManager->SetLoopbackOff();
    }

    return true;
}

int AudioFtm::PhoneMic_SpkLR_Loopback(char echoflag) {
    ALOGD("%s(), echoflag = %d", __FUNCTION__, echoflag);

    if (echoflag == MIC1_ON) { // enable
        mLoopbackManager->SetLoopbackOn(AP_MAIN_MIC_AFE_LOOPBACK, LOOPBACK_OUTPUT_SPEAKER);
    } else if (echoflag == MIC2_ON) { // enable
        mLoopbackManager->SetLoopbackOn(AP_REF_MIC_AFE_LOOPBACK, LOOPBACK_OUTPUT_SPEAKER);
    } else { // disable
        mLoopbackManager->SetLoopbackOff();
    }

    return true;
}

int AudioFtm::HeadsetMic_EarphoneLR_Loopback(char bEnable, char bHeadsetMic __unused) {
    ALOGD("%s(), bEnable = %d", __FUNCTION__, bEnable);

    if (bEnable) { // enable
        mLoopbackManager->SetLoopbackOn(AP_HEADSET_MIC_AFE_LOOPBACK, LOOPBACK_OUTPUT_EARPHONE);
    } else { // disable
        mLoopbackManager->SetLoopbackOff();
    }

    return true;
}

int AudioFtm::HeadsetMic_SpkLR_Loopback(char echoflag) {
    ALOGD("%s(), enable = %d", __FUNCTION__, echoflag);

    if (echoflag) { // enable
        mLoopbackManager->SetLoopbackOn(AP_HEADSET_MIC_AFE_LOOPBACK, LOOPBACK_OUTPUT_SPEAKER);
    } else { // disable
        mLoopbackManager->SetLoopbackOff();
    }

    return true;
}

int AudioFtm::HeadsetMic_Receiver_Loopback(char bEnable, char bHeadsetMic __unused) {
    ALOGD("%s(), bEnable = %d", __FUNCTION__, bEnable);

    if (bEnable) { // enable
        mLoopbackManager->SetLoopbackOn(AP_HEADSET_MIC_AFE_LOOPBACK, LOOPBACK_OUTPUT_RECEIVER);
    } else { // disable
        mLoopbackManager->SetLoopbackOff();
    }

    return true;
}

int AudioFtm::PhoneMic_Receiver_Acoustic_Loopback(int Acoustic_Type, int *Acoustic_Status_Flag, int bHeadset_Output) {
    ALOGD("+%s(), Acoustic_Type = %d, headset_available = %d", __FUNCTION__, Acoustic_Type, bHeadset_Output);
    /*  Acoustic loopback
    *   0: Dual mic (w/o DMNR)acoustic loopback off
    *   1: Dual mic (w/o DMNR)acoustic loopback
    *   2: Dual mic (w/  DMNR)acoustic loopback off
    *   3: Dual mic (w/  DMNR)acoustic loopback
    */

    static LoopbackManager *pLoopbackManager = LoopbackManager::GetInstance();
    loopback_output_device_t loopback_output_device;
    if (bHeadset_Output == true) {
        loopback_output_device = LOOPBACK_OUTPUT_EARPHONE;
    } else {
        loopback_output_device = LOOPBACK_OUTPUT_RECEIVER; // default use receiver here
    }



    bool retval = true;
    static int acoustic_status = 0;
    switch (Acoustic_Type) {
    case ACOUSTIC_STATUS:
        *Acoustic_Status_Flag = acoustic_status;
        break;
    case DUAL_MIC_WITHOUT_DMNR_ACS_OFF:
        // close single mic acoustic loopback
        pLoopbackManager->SetLoopbackOff();
        acoustic_status = DUAL_MIC_WITHOUT_DMNR_ACS_OFF;
        break;
    case DUAL_MIC_WITHOUT_DMNR_ACS_ON:
        // open dual mic acoustic loopback (w/o DMNR)
        pLoopbackManager->SetLoopbackOn(MD_DUAL_MIC_ACOUSTIC_LOOPBACK_WITHOUT_DMNR, loopback_output_device);
        acoustic_status = DUAL_MIC_WITHOUT_DMNR_ACS_ON;
        break;
    case DUAL_MIC_WITH_DMNR_ACS_OFF:
        // close dual mic acoustic loopback
        pLoopbackManager->SetLoopbackOff();
        acoustic_status = DUAL_MIC_WITH_DMNR_ACS_OFF;
        break;
    case DUAL_MIC_WITH_DMNR_ACS_ON:
        // open dual mic acoustic loopback (w/ DMNR)
        pLoopbackManager->SetLoopbackOn(MD_DUAL_MIC_ACOUSTIC_LOOPBACK_WITH_DMNR, loopback_output_device);
        acoustic_status = DUAL_MIC_WITH_DMNR_ACS_ON;
        break;
    default:
        break;
    }

    ALOGD("-%s(), out -", __FUNCTION__);
    return retval;
}


/// FM / mATV
int AudioFtm::FMLoopbackTest(char bEnable __unused) {
    ALOGD("%s()", __FUNCTION__);
    return true;
}

int AudioFtm::Audio_FM_I2S_Play(char bEnable) {
    ALOGD("%s()", __FUNCTION__);

    const float kMaxFmVolume = 1.0;

    if (mStreamOut == NULL) {
        if (mStreamManager->getStreamOutVectorSize() == 0) { // Factory mode
            uint32_t devices = 0x2;
            int format = 0x1;
            uint32_t channels = 0x3;
            uint32_t sampleRate = 44100;
            status_t status = 0;

            mStreamManager->openOutputStream(devices, &format, &channels, &sampleRate, &status);
        }

        mStreamOut = mStreamManager->getStreamOut(0);
    }

    if (bEnable == true) {
        // force assigned earphone
        AudioParameter paramRouting = AudioParameter();
        paramRouting.addInt(String8(AudioParameter::keyRouting), AUDIO_DEVICE_OUT_WIRED_HEADPHONE);
        mStreamOut->setParameters(paramRouting.toString());

        // enable
#if defined(SUPPORT_FM_AUDIO_BY_PROPRIETARY_PARAMETER_CONTROL)
        AudioALSAFMController::getInstance()->setFmMute(true);
        AudioALSAFMController::getInstance()->setFmMode(true);
        mStreamManager->setFmEnable(true, true, true, AUDIO_DEVICE_OUT_WIRED_HEADPHONE);
        mStreamManager->setFmVolume(kMaxFmVolume);
        AudioALSAFMController::getInstance()->setFmMute(false);
#else
        mStreamManager->setFmVolume(0);
        mStreamManager->setFmEnable(true, true , true, AUDIO_DEVICE_OUT_WIRED_HEADPHONE);
        mStreamManager->setFmVolume(kMaxFmVolume);
#endif
    } else {
        // disable
#if defined(SUPPORT_FM_AUDIO_BY_PROPRIETARY_PARAMETER_CONTROL)
        AudioALSAFMController::getInstance()->setFmMute(true);
        AudioALSAFMController::getInstance()->setFmMode(false);
        mStreamManager->setFmEnable(false);
        AudioALSAFMController::getInstance()->setFmMute(false);
#else
        mStreamManager->setFmVolume(0);
        mStreamManager->setFmEnable(false);
#endif
    }

    return true;
}

int AudioFtm::Audio_MATV_I2S_Play(int enable_flag __unused) {
    ALOGD("%s()", __FUNCTION__);
    return true;
}
int AudioFtm::Audio_FMTX_Play(bool Enable __unused, unsigned int Freq __unused) {
    ALOGD("%s()", __FUNCTION__);
    return true;
}

int AudioFtm::ATV_AudPlay_On(void) {
    ALOGD("%s()", __FUNCTION__);
    return true;
}
int AudioFtm::ATV_AudPlay_Off(void) {
    ALOGD("%s()", __FUNCTION__);
    return true;
}
unsigned int AudioFtm::ATV_AudioWrite(void *buffer __unused, unsigned int bytes __unused) {
    ALOGD("%s()", __FUNCTION__);
    return true;
}


/// HDMI
int AudioFtm::HDMI_SineGenPlayback(bool Enable, int Freq) {
    ALOGD("%s(), enable = %d Freq = %d", __FUNCTION__, Enable, Freq);

    if (Enable) { // enable
        mStreamManager->setHdmiEnable(true);

        mHardwareResourceManager->setSgenMode(SGEN_MODE_O03_O04);
        // 6763 connect HDMI to O3O4
        // mHardwareResourceManager->setSgenMode(SGEN_MODE_O00_O01);
        mHardwareResourceManager->setSgenSampleRate(SGEN_MODE_SAMPLERATE_44100HZ);
    } else { // disable
        mHardwareResourceManager->setSgenMode(SGEN_MODE_DISABLE);

        mStreamManager->setHdmiEnable(false);
    }
    return 0;
}


/// Vibration Speaker
int AudioFtm::SetVibSpkCalibrationParam(void *cali_param) {
    ALOGD("%s()", __FUNCTION__);
    return setAudioCompFltCustParam(AUDIO_COMP_FLT_VIBSPK, (AUDIO_ACF_CUSTOM_PARAM_STRUCT *)cali_param);
}

uint32_t AudioFtm::GetVibSpkCalibrationStatus() {
    ALOGD("%s()", __FUNCTION__);
    AUDIO_ACF_CUSTOM_PARAM_STRUCT audioParam;
    getAudioCompFltCustParam(AUDIO_COMP_FLT_VIBSPK, &audioParam);
    if (audioParam.bes_loudness_WS_Gain_Max != VIBSPK_CALIBRATION_DONE && audioParam.bes_loudness_WS_Gain_Max != VIBSPK_SETDEFAULT_VALUE) {
        return 0;
    } else {
        return audioParam.bes_loudness_WS_Gain_Min;
    }
}

void AudioFtm::SetVibSpkEnable(bool enable, uint32_t freq) {
    ALOGD("%s() enable:[%d] freq:[%d]", __FUNCTION__, enable, freq);
    if (enable) {
        AudioVIBSPKControl::getInstance()->setParameters(44100, freq, 0, 0);
    }
    AudioVIBSPKControl::getInstance()->setVibSpkEnable(enable);
}

void AudioFtm::SetVibSpkRampControl(uint8_t rampcontrol) {
    ALOGD("%s()", __FUNCTION__);
    AudioVIBSPKControl::getInstance()->VibSpkRampControl(rampcontrol);
}

bool AudioFtm::ReadAuxadcData(int channel, int *value) {
    struct mixer_ctl *ctl;
    int retval, dValue;
    channel = 0x001B; //AUX_SPK_THR_I_AP include mt_pmic.h
    ALOGD("+%s() channel [0x%x] (force to replace)", __FUNCTION__, channel);
    ctl = mixer_get_ctl_by_name(mMixer, "Audio AUXADC Data");
    if (NULL == ctl || NULL == value) {
        ALOGD("[%s] [%d]", __FUNCTION__, __LINE__);
        return false;
    }
    retval = mixer_ctl_set_value(ctl, 0, channel);
    usleep(1000);
    dValue = mixer_ctl_get_value(ctl, 0);
    *value = dValue;
    ALOGD("-%s() value [0x%x]", __FUNCTION__, dValue);
    return false;
}


status_t AudioFtm::setMicEnable(const audio_mic_mask_t audio_mic_mask, const bool enable) {
    ALOGD("%s(), audio_mic_mask = 0x%x, enable = %d", __FUNCTION__, audio_mic_mask, enable);

    if (enable == true) {
        switch (audio_mic_mask) {
        case AUDIO_MIC_MASK_MIC1: {
            mHardwareResourceManager->setMIC1Mode(false);
            if (mixer_ctl_set_enum_by_string(mixer_get_ctl_by_name(mMixer, "Audio_MicSource1_Setting"), "ADC1")) { //K2 add
                ALOGE("Error:  Audio_MicSource1_Setting invalid value");
            }
            if (mixer_ctl_set_enum_by_string(mixer_get_ctl_by_name(mMixer, "Audio_ADC_1_Switch"), "On")) {
                ALOGE("Error:  Audio_ADC_1_Switch invalid value");
            }
            if (mixer_ctl_set_enum_by_string(mixer_get_ctl_by_name(mMixer, "Audio_Preamp1_Switch"), "IN_ADC1")) { //preamp1, "IN_ADC1": AIN0, "IN_ADC2": AIN1
                ALOGE("Error:  Audio_Preamp1_Switch invalid value");
            }
            break;
        }
        case AUDIO_MIC_MASK_MIC2: {
            mHardwareResourceManager->setMIC2Mode(false);
            if (mixer_ctl_set_enum_by_string(mixer_get_ctl_by_name(mMixer, "Audio_MicSource1_Setting"), "ADC1")) { //ADC1: main/ref mic, ADC2: headset mic
                ALOGE("Error:  Audio_MicSource1_Setting invalid value");
            }
            if (mixer_ctl_set_enum_by_string(mixer_get_ctl_by_name(mMixer, "Audio_ADC_1_Switch"), "On")) {
                ALOGE("Error:  Audio_ADC_1_Switch invalid value");
            }
            if (mixer_ctl_set_enum_by_string(mixer_get_ctl_by_name(mMixer, "Audio_Preamp1_Switch"), "IN_ADC3")) { //preamp1, "IN_ADC1": AIN0, "IN_ADC2": AIN1, "IN_ADC3": AIN2
                ALOGE("Error:  Audio_Preamp1_Switch invalid value");
            }
            break;
        }
        case AUDIO_MIC_MASK_MIC3: {
            break;
        }
        case AUDIO_MIC_MASK_HEADSET_MIC: {
            mHardwareResourceManager->setMIC1Mode(true);
            if (mixer_ctl_set_enum_by_string(mixer_get_ctl_by_name(mMixer, "Audio_MicSource1_Setting"), "ADC2")) { //K2 add
                ALOGE("Error:  Audio_MicSource1_Setting invalid value");
            }
            if (mixer_ctl_set_enum_by_string(mixer_get_ctl_by_name(mMixer, "Audio_ADC_1_Switch"), "On")) {
                ALOGE("Error:  Audio_ADC_1_Switch invalid value");
            }
            if (mixer_ctl_set_enum_by_string(mixer_get_ctl_by_name(mMixer, "Audio_Preamp1_Switch"), "IN_ADC2")) {
                ALOGE("Error:  Audio_Preamp1_Switch invalid value");
            }
            break;
        }
        default: {
            ALOGW("%s(), not support audio_mic_mask = 0x%x", __FUNCTION__, audio_mic_mask);
            break;
        }
        }
    } else {
        switch (audio_mic_mask) {
        case AUDIO_MIC_MASK_MIC1: {
            mHardwareResourceManager->setMIC1Mode(false);
            if (mixer_ctl_set_enum_by_string(mixer_get_ctl_by_name(mMixer, "Audio_Preamp1_Switch"), "OPEN")) {
                ALOGE("Error:  Audio_Preamp1_Switch invalid value");
            }
            if (mixer_ctl_set_enum_by_string(mixer_get_ctl_by_name(mMixer, "Audio_ADC_1_Switch"), "Off")) {
                ALOGE("Error:  Audio_ADC_1_Switch invalid value");
            }
            break;
        }
        case AUDIO_MIC_MASK_MIC2: {
            mHardwareResourceManager->setMIC2Mode(false);
            if (mixer_ctl_set_enum_by_string(mixer_get_ctl_by_name(mMixer, "Audio_Preamp1_Switch"), "OPEN")) {
                ALOGE("Error:  Audio_Preamp1_Switch invalid value");
            }
            if (mixer_ctl_set_enum_by_string(mixer_get_ctl_by_name(mMixer, "Audio_ADC_1_Switch"), "Off")) {
                ALOGE("Error:  Audio_ADC_1_Switch invalid value");
            }
            break;
        }
        case AUDIO_MIC_MASK_MIC3: {
            break;
        }
        case AUDIO_MIC_MASK_HEADSET_MIC: {
            mHardwareResourceManager->setMIC1Mode(true);
            if (mixer_ctl_set_enum_by_string(mixer_get_ctl_by_name(mMixer, "Audio_Preamp1_Switch"), "OPEN")) {
                ALOGE("Error:  Audio_Preamp1_Switch invalid value");
            }
            if (mixer_ctl_set_enum_by_string(mixer_get_ctl_by_name(mMixer, "Audio_ADC_1_Switch"), "Off")) {
                ALOGE("Error:  Audio_ADC_1_Switch invalid value");
            }
            break;
        }
        default: {
            ALOGW("%s(), not support audio_mic_mask = 0x%x", __FUNCTION__, audio_mic_mask);
            break;
        }
        }
    }

    return NO_ERROR;
}

void AudioFtm::SetStreamOutPostProcessBypass(bool flag) {
    ALOGD("%s() %d", __FUNCTION__, flag);
    mStreamManager->setBypassDLProcess(flag);
    return ;
}

bool AudioFtm::SpeakerCalibration(int calibStage) {
    ALOGD("%s(), calibStage: %d\n",
          __FUNCTION__, calibStage);

    int result = 0;
    int spkTye = AudioSmartPaController::getInstance()->getSpkProtectType();

    if (spkTye == SPK_ONBOARD_DSP) {
        mHardwareResourceManager->startOutputDevice(AUDIO_DEVICE_OUT_SPEAKER, 32000);
    }

    result =
        AudioSmartPaController::getInstance()->setSmartPaCalibration(calibStage);

    if (spkTye == SPK_ONBOARD_DSP) {
        mHardwareResourceManager->stopOutputDevice();
    }

    ALOGD("%s(), ret = %d\n", __FUNCTION__, result);
    return result < 0? false: true;
}

}
