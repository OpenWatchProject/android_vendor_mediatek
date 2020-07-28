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

#ifndef __AUDIO_AMP_CONTROL_INTERFACE_H__
#define __AUDIO_AMP_CONTROL_INTERFACE_H__

namespace android {

enum AUDIO_AMP_CONTROL_COMMAND {
    AUD_AMP_GET_CTRP_NUM,
    AUD_AMP_GET_CTRP_BITS,
    AUD_AMP_GET_CTRP_TABLE,
    AUD_AMP_GET_REGISTER,
    AUD_AMP_SET_REGISTER,
    AUD_AMP_SET_AMPGAIN,  // gain is use for low 24bits as external amp , device should base on control point set to AMPLL_CON0_REG
    AUD_AMP_GET_AMPGAIN,
    AUD_AMP_SET_MODE,
    NUM_AUD_AMP_CONTROL_COMMAND
};

typedef struct {
    unsigned long int   command;
    unsigned long int   param1;
    unsigned long int   param2;
} AMP_Control;

class AudioAMPControlInterface {
public:
    AudioAMPControlInterface() {}
    virtual ~AudioAMPControlInterface() {}
    //open/close device
    virtual bool setSpeaker(bool on) = 0;
    virtual bool setHeadphone(bool on) = 0;
    virtual bool setReceiver(bool on) = 0;
    virtual status_t setVolume(void *points, int num, int device) = 0;
    //set mode to amp
    virtual void setMode(audio_mode_t mode) = 0;
    // for set and get parameters , for command1 and command 2 , use to command
    // and data is used for comamnd need to carry a lots of command.
    virtual status_t setParameters(int command1, int command2, unsigned int data) = 0;
    virtual status_t getParameters(int command1, int command2, void *data) = 0;
};

class AudioDeviceManger {
public:
    static AudioAMPControlInterface *createInstance();
private:
    static AudioAMPControlInterface *mInstance;
};

}

#endif

