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

#ifndef __RP_AUDIOCONTROL_HANDLER_H__
#define __RP_AUDIOCONTROL_HANDLER_H__

/*****************************************************************************
 * Include
 *****************************************************************************/
#include <stdio.h>
#include <stdlib.h>

#include <binder/Parcel.h>
#include <cutils/properties.h>

#include "RfxController.h"
#include "RfxStatusDefs.h"
#include "RfxTimer.h"
#include "nw/RpNwRatController.h"

/*****************************************************************************
 * MACRO
 *****************************************************************************/


#define RP_AUDIOCONTROL_TAG "RpAudioControl"

#define AUDIOCONTROL_ASSERT(_c)       assert(_c)
#define AUDIOCONTROL_DBG_ASSERT(_c)   assert(_c)

typedef struct audioModeParameterStructDef {
    int slotId;
    char mdString[PROPERTY_VALUE_MAX];
    sp<RfxAction> action;

    pthread_mutex_t *mutexObject;
} audioModeParameterStruct;

/*****************************************************************************
 * Class RpAudioControlHandler
 *****************************************************************************/
class RpAudioControlHandler : public RfxController {
    RFX_DECLARE_CLASS(RpAudioControlHandler);  // Required: declare this class

public:
    RpAudioControlHandler();

    virtual ~RpAudioControlHandler();
    bool updateAudioPathSync(int csPhone, const sp<RfxAction>& action);
    bool updateAudioPathAsync(int csPhone);
    void updateAudioModem(int csPhone);

    static String8 modemGsm;
    static String8 modemC2k;

    static String8 audioPhone1Md;
    static String8 audioPhone2Md;

// Override
protected:
    virtual void onInit();


private:
    // Overwrite
    void launchThreadToRefreshAudioMode();
    bool needToRefreshAudioModem(int csPhone);
    static void *refreshAudioMode(void *arg);
    static void setRefreshModem(bool isSvlte, const String8& activeModem, audioModeParameterStruct* parameter);

private:
    audioModeParameterStruct mAudioModeParameter;
    sp<RfxAction> mAction;
    pthread_mutex_t mAudioSwitchMutex;

    static String8 refreshMd;
};

#endif /* __RP_AUDIOCONTROL_HANDLER_H__ */

