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

#define LOG_TAG "AudioAMPControl"
#define MTK_LOG_ENABLE 1
#include <log/log.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/ioctl.h>
//#include "AudioIoctl.h" //ccc mark for build pass
#include <utils/threads.h>
#include"AudioAMPControlInterface.h"

namespace {

using  namespace android;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// AMP Control

class AudioAMPControl : public AudioAMPControlInterface {
public:
    AudioAMPControl(int fb);
    ~AudioAMPControl();
    bool setSpeaker(bool on);
    bool setHeadphone(bool on);
    bool setReceiver(bool on);
    status_t setVolume(void *points, int num, int device);
    void setMode(audio_mode_t mode);
    int setParameters(int command1, int command2, unsigned int data);
    int getParameters(int command1, int command2, void *data);
private:
    int selectPathMask(audio_mode_t phoneState, audio_devices_t device);
private:
    int            mFd;
    bool           mSpeakerOn;
    bool           mReceiverOn;
    bool           mHeadphoneOn;
    audio_mode_t   mMode;
    Mutex          mLock;
};


static int shiftAMPGain(uint8_t *points, int num, int mask)
//points   0: in1 index,  1: SPK index,  2: in 2 index, 3: HP index
//mask    IN1   IN2   SPK   HP
{
    //  Analog,              INPUT1,  INPUT2 , SPEAKER ,      HP  ,       mask
    //xxxxxxxxxxxxxx,     xx,         xx,       xxxxx,       xxxxx,       xxxx   (32 bits)
    uint8_t *p = points;
    int amp = 0;
    SLOGD("p[0] is 0x%x, p[1] is 0x%x, p[2] is 0x%x, p[3] is 0x%x", p[0], p[1], p[2], p[3]);

    amp = mask & 0xF;                   //mask
    amp = ((p[3] & 0x1F) << 5  | amp);   //hpr
    amp = ((p[3] & 0x1F) << 10 | amp);  //hpl
    amp = ((p[1] & 0x1F) << 15 | amp);  //speaker
    amp = ((p[2] & 0x03) << 20 | amp);  //IN2
    amp = ((p[0] & 0x03) << 22 | amp);  //IN1

    return amp;
}

AudioAMPControl::AudioAMPControl(int fd)
    : mFd(fd),
      mSpeakerOn(false),
      mReceiverOn(false),
      mHeadphoneOn(false),
      mMode(AUDIO_MODE_INVALID) {
}

AudioAMPControl::~AudioAMPControl() {

}

bool AudioAMPControl::setSpeaker(bool on) {
    SLOGD("setSpeaker::mode=%d,prestate=%d,state=%d", mMode, mSpeakerOn, on);
    Mutex::Autolock autoLock(mLock);
    int ret = NO_ERROR;
    if (mSpeakerOn == on) {
        return true;
    }
    if (on) {
        //ret = ::ioctl(mFd,SET_SPEAKER_ON,Channel_Stereo);// enable speaker //ccc mark for build pass
    } else {
        //ret = ::ioctl(mFd,SET_SPEAKER_OFF,Channel_Stereo);// disable speaker  //ccc mark for build pass
    }
    if (ret < 0) {
        SLOGW(" setSpeaker(%d) error %s (%d)", on, ::strerror(errno), errno);
        return false;
    }
    mSpeakerOn = on;
    return true;
}

bool AudioAMPControl::setHeadphone(bool on) {
    SLOGD("setHeadphone::mode=%d,prestate=%d,state=%d", mMode, mHeadphoneOn, on);
    Mutex::Autolock autoLock(mLock);
    int ret = NO_ERROR;
    if (mHeadphoneOn == on) {
        return true;
    }
    if (on) {
        //ret = ::ioctl(mFd,SET_HEADPHONE_ON,Channel_Stereo);// enable headphone, //ccc mark for build pass
    } else {
        //ret = ::ioctl(mFd,SET_HEADPHONE_OFF,Channel_Stereo);// disable headphone, //ccc mark for build pass
    }
    if (ret < 0) {
        SLOGW(" setHeadphone(%d) error %s (%d)", on, ::strerror(errno), errno);
        return false;
    }
    mHeadphoneOn = on;
    return true;
}

bool AudioAMPControl::setReceiver(bool on) {
    SLOGD("setReceiver::mode=%d,prestate=%d,state=%d", mMode, mReceiverOn, on);
    Mutex::Autolock autoLock(mLock);
    int ret = NO_ERROR;
    if (mReceiverOn == on) {
        return true;
    }
    if (on) {
        //ret = ::ioctl(mFd,SET_EARPIECE_ON,Channel_Stereo);// enable earpiece, //ccc mark for build pass
    } else {
        //ret = ::ioctl(mFd,SET_EARPIECE_OFF,Channel_Stereo);// diable earpiece //ccc mark for build pass
    }
    if (ret < 0) {
        SLOGW(" setreceiver(%d) error %s (%d)", on, ::strerror(errno), errno);
        return false;
    }
    mReceiverOn = on;
    return true;
}

status_t AudioAMPControl::setVolume(void *points,  int num, int device) {
    Mutex::Autolock autoLock(mLock);
    int mask = selectPathMask(mMode, (audio_devices_t)device);
    SLOGD("setVolume mask 0x%x", mask);
    int amp  = shiftAMPGain((uint8_t *)points, num, mask);
    SLOGD("setVolume amp 0x%x", amp);
    return setParameters(AUD_AMP_SET_AMPGAIN, 0, amp);
}

void AudioAMPControl::setMode(audio_mode_t mode) {
    SLOGD("setMode mode=%d", mode);
    // do not use lock here ,because setparameter will try to get lock.
    setParameters(AUD_AMP_SET_MODE, 0, mode);
    return;
}

status_t AudioAMPControl::setParameters(int command1, int command2, unsigned int data) {
    SLOGD("setparameters commad1 = %d command2 = %d,data=%u", command1, command2, data);
    status_t ret = NO_ERROR;
    AMP_Control command;
    memset(&command, 0, sizeof(AMP_Control));
    switch (command1) {
    case AUD_AMP_SET_REGISTER: {
        command.command = command1;
        command.param1  = command2;
        command.param2  = data;
        //ret =::ioctl(mFd,SET_EAMP_PARAMETER,&command);//ccc mark for build pass
        break;
    }
    case AUD_AMP_SET_AMPGAIN: {
        command.command = command1;
        command.param1  = data;
        //ret =::ioctl(mFd,SET_EAMP_PARAMETER,&command);//ccc mark for build pass
        break;
    }
    case AUD_AMP_SET_MODE: {
        command.command = command1;
        command.param1  = data;
        mMode = (audio_mode_t)data;
        //ret =::ioctl(mFd,SET_EAMP_PARAMETER,&command);//ccc mark for build pass
        break;
    }
    default:
        break;
    }
    if (ret < 0) {
        SLOGW(" setParameters(%d) error %s (%d)", command1, ::strerror(errno), errno);
    }
    return ret;
}

int AudioAMPControl::getParameters(int command1, int command2, void *data) {
    SLOGD("getparameters command = %d command2 = %d,data=%p", command1, command2, data);
    Mutex::Autolock autoLock(mLock);
    int ret = NO_ERROR;
    AMP_Control command;
    memset(&command, 0, sizeof(AMP_Control));
    switch (command1) {
    case AUD_AMP_GET_CTRP_NUM:
    case AUD_AMP_GET_CTRP_BITS:
    case AUD_AMP_GET_REGISTER:
    case AUD_AMP_GET_AMPGAIN: {
        command.command = command1;
        command.param1  = command2;
        //ret =::ioctl(mFd,GET_EAMP_PARAMETER,&command);//ccc mark for build pass
        if (data) {
            int *p = (int *)data;
            *p = ret;
        }
        break;
    }
    case AUD_AMP_GET_CTRP_TABLE: {
        command.command = command1;
        command.param1  = command2;
        command.param2  = (unsigned long int)data;
        //ret =::ioctl(mFd,GET_EAMP_PARAMETER,&command);//ccc mark for build pass
        break;
    }

    default:
        break;

    }
    if (ret < 0) {
        SLOGW(" getParameters(%d) error %s (%d)", command1, ::strerror(errno), errno);
    }
    return ret;
}

int AudioAMPControl::selectPathMask(audio_mode_t phoneState, audio_devices_t device) {
    ALOGV("getAmpValidMask phoneState %d, device 0x%x", phoneState, device);
    //use 5bits for valid amp gain
    //IN1(CP0), IN2(CP2),  SPK(CP1), HP(CP3):xxxx(4bits)
    //normal mode/call mode disable USING_EXTAMP_ALL_VOICE_BUFFER:headphone:cp0,cp3;speaker:cp1,cp2
    //call mode enable USING_EXTAMP_ALL_VOICE_BUFFER:headphone:cp2,cp3;speaker:cp1,cp2
    int mask = 0; //00000 (not use)
    if (device & AUDIO_DEVICE_OUT_SPEAKER || device == 0) {
        mask = 0x6;//0110

    }
    if (device & AUDIO_DEVICE_OUT_WIRED_HEADSET || device & AUDIO_DEVICE_OUT_WIRED_HEADPHONE) {
#ifdef USING_EXTAMP_ALL_VOICE_BUFFER
        if (phoneState == AUDIO_MODE_IN_CALL) {
            mask = mask | 0x5; //0101
        } else
#endif
        {
            mask = mask | 0x9; //1001
        }

    }
    if (device & AUDIO_DEVICE_OUT_EARPIECE) {
        mask = 0;  //0000  not use
    }
    return mask;
}

//AMP Control end
//---------------------------------------------------------------------------------
}

namespace android {

AudioAMPControlInterface *AudioDeviceManger::mInstance = NULL;

AudioAMPControlInterface *AudioDeviceManger::createInstance() {
    if (! mInstance) {
#if 0//ccc mark for build pass
        int fd = -1;
        // here open audio hardware for register setting
        fd = ::open(kAudioDeviceName, O_RDWR);
        if (fd < 0) {
            ALOGE("AudioDeviceManger open fd fail");
            return NULL;
        }
        mInstance = new AudioAMPControl(fd);
#endif
    }
    return mInstance;
}


}

