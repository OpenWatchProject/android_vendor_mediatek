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

#ifndef __RFX_CAPABILITY_SWITCH_H__
#define __RFX_CAPABILITY_SWITCH_H__

/*****************************************************************************
 * Include
 *****************************************************************************/

#include "RfxController.h"
#include "RfxTimer.h"

/*****************************************************************************
 * Class RfxController
 *****************************************************************************/

const char PROPERTY_NET_CDMA_MDMSTAT[25] = "vendor.net.cdma.mdmstat";

class RpCapabilitySwitchController : public RfxController {
    RFX_DECLARE_CLASS(RpCapabilitySwitchController); // Required: declare this class

public:

    RpCapabilitySwitchController();

    virtual ~RpCapabilitySwitchController();

// Override
protected:
    virtual bool onHandleRequest(const sp<RfxMessage>& message);
    virtual bool onHandleResponse(const sp<RfxMessage>& message);
    virtual void onInit();
    // For pending the sequential by check the mode switch status
    virtual bool onPreviewMessage(const sp<RfxMessage>& message);
    virtual bool onCheckIfResumeMessage(const sp<RfxMessage>& message);

private:
    // Used to check some preconditions befor handling a message
    bool isReadyForMessage(const sp<RfxMessage>& message, bool log);
    // Handle the response of RIL_REQUEST_SET_RADIO_CAPABILITY
    void handleSetRadioCapabilityResponse(const sp<RfxMessage>& message);
    // Handle and send request to rild after power off radio
    void onRequestRadioOffDone(int slotId, const sp<RfxMessage> message);
    int checkPhase(const sp<RfxMessage>& message);

    bool checkPhaseIfFinish(const sp<RfxMessage>& message);

    const char* requestToString(int reqId);

    const char* urcToString(int reqId);

    bool isSimReady();

    bool isSvlteSupport();

    int getSession(const sp<RfxMessage>& message);
    // Get the radio capability by parsing the message
    void getRadioCapability(const sp<RfxMessage>& message, RIL_RadioCapability& rc);

private:
    const int kModeSwitching;
    // Counter used when closing radio, -1 means no close, 0 means starting to close, 1 means 1 slot is closed, ...
    int mCloseRadioCount;
    // Used to check if c2k channel is locked for pending and sending request to gsm rild
    bool mC2kChannelLocked;
};

#endif /* __RFX_HELLO_CONTROLLER_H__ */

