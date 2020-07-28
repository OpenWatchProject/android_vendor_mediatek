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

#ifndef __RFX_CAT_CONTROLLER_H__
#define __RFX_CAT_CONTROLLER_H__

/*****************************************************************************
 * Include
 *****************************************************************************/
#include "call/RpAudioControlHandler.h"
#include "RfxController.h"
#include "RfxTimer.h"
#include "RfxNwServiceState.h"
#include <cutils/properties.h>


/*****************************************************************************
 * Enum
 *****************************************************************************/
// Proactive Command Type
typedef enum {
    CMD_REFRESH = 0x01,
    CMD_MORE_TIME = 0x02,
    CMD_POLL_INTERVAL = 0x03,
    CMD_POLLING_OFF = 0x04,
    CMD_SETUP_EVENT_LIST = 0x05,
    CMD_SETUP_CALL = 0x10,
    CMD_SEND_SS = 0x11,
    CMD_SEND_USSD = 0x12,
    CMD_SEND_SMS = 0x13,
    CMD_DTMF = 0x14,
    CMD_LAUNCH_BROWSER = 0x15,
    CMD_PLAY_TONE = 0x20,
    CMD_DSPL_TXT = 0x21,
    CMD_GET_INKEY = 0x22,
    CMD_GET_INPUT = 0x23,
    CMD_SELECT_ITEM = 0x24,
    CMD_SETUP_MENU = 0x25,
    CMD_PROVIDE_LOCAL_INFO = 0x26,
    CMD_TIMER_MANAGER = 0x27,
    CMD_IDLE_MODEL_TXT = 0x28,
    CMD_PERFORM_CARD_APDU = 0x30,
    CMD_POWER_ON_CARD = 0x31,
    CMD_POWER_OFF_CARD = 0x32,
    CMD_GET_READER_STATUS = 0x33,
    CMD_RUN_AT = 0x34,
    CMD_LANGUAGE_NOTIFY = 0x35,
    CMD_OPEN_CHAN = 0x40,
    CMD_CLOSE_CHAN = 0x41,
    CMD_RECEIVE_DATA = 0x42,
    CMD_SEND_DATA = 0x43,
    CMD_GET_CHAN_STATUS = 0x44,
    CMD_RFU = 0x60,
    CMD_END_PROACTIVE_SESSION = 0x81,
    CMD_DETAIL = 0xFF
} CatProactiveCmdEnum;

// / M: OTASP {
#define OTASP_PROGRAMMING_STARTED               1
#define OTASP_PROGRAMMING_SUCCESSFULLY          8
#define OTASP_PROGRAMMING_UNSUCCESSFULLY        9
// / M: OTASP }
/*****************************************************************************
 * Define
 *****************************************************************************/
// Invalid value
#define INVALID_VALUE -1

// Envelpoe Command Type
#define MENU_SELECTION_TAG  0xD3
#define EVENT_DOWNLOAD_TAG  0xD6

// Event Type
#define EVENT_DATA_AVAILABLE  0x09
#define EVENT_CHANNEL_STATUS  0x0A


/*****************************************************************************
 * typedef
 *****************************************************************************/

typedef struct {
    int cmdType;
    int source;
} ProactiveCmdRecord;

/*****************************************************************************
 * Class RpCatController
 *****************************************************************************/

class RpCatController : public RfxController {
    RFX_DECLARE_CLASS(RpCatController);  // Required: declare this class

public:
    RpCatController();

    virtual ~RpCatController();

// Override
protected:
    virtual bool onHandleRequest(const sp<RfxMessage>& message);

    virtual bool onHandleUrc(const sp<RfxMessage>& message);

    virtual bool onHandleResponse(const sp<RfxMessage>& message);

    virtual void onInit();

    virtual void onDeinit();

private:
    void onCardTypeChanged(RfxStatusKeyEnum key,
                           RfxVariant oldValue,
                           RfxVariant newValue);

    char* strdupReadString(Parcel* p);

    void writeStringToParcel(Parcel *p, const char *s);

    int getStkCommandType(char *cmd);

    void parseStkCmdType(const sp<RfxMessage>& message, int* cmdType);

    void parseStkEnvCmdType(const sp<RfxMessage>& message,
                            int* envCmdType,
                            int* eventType);

    const char* requestToString(int reqId);

    const char* urcToString(int reqId);

    void onModeChanged();

    void setSTKUTKMode(int mode);

    uint8_t toByte(char c);

    bool findComprehensionTlvTag(char* cmd, int start_pos, uint8_t tagSearch,
            int* pTagStart, int* pTagTotal);

    void configUtkMode(int mode);
    // / M: OTASP {
    void onRatSwitchDone(RfxStatusKeyEnum key,
                         RfxVariant oldValue,
                         RfxVariant newValue);

    int getNwsMode();

    void onCsimStateChanged(RfxStatusKeyEnum key, RfxVariant oldValue, RfxVariant newValue);

    void onVoiceCallCountChanged(RfxStatusKeyEnum key, RfxVariant oldValue, RfxVariant newValue);

    void onOtaspActivationStatusChanged(RfxStatusKeyEnum key, RfxVariant oldValue,
            RfxVariant newValue);

    void onServiceStateChanged(int slotId, RfxStatusKeyEnum key, RfxVariant oldValue,
            RfxVariant newValue);

    bool checkIfNeedToTriggerOtasp();

    void handleOtaspRequest(const sp<RfxMessage>& message);

    void sendOpenChannelTRForOtasp(const sp<RfxMessage>& message);

    void sendTriggerOtaspUrc(const sp<RfxMessage>& message);

    void RetryOtaspIfNeeded();

    void sendOtaspRetryRequestToModem3();

    bool isVzWSupport();
    // / M: OTASP }
private:
    int mCardType;
    int mcdmaCardType;
    int mSetupMenuSource;
    int mSetupEventSource;
    int mOpenChannelSource;
    ProactiveCmdRecord mProCmdRec;
    // / M: OTASP {
    int mCsimStatus;
    int mOtaspActivationStatus;
    int mDataRadioTech;
    int mFailureCount;
    bool mStartRetry;
    bool mIsOpenChannelCommandQueued;
    sp<RfxMessage> mPendingOpenChannelRequest;
    int mVoiceCallCount;
    int mSupportBipOtaspActivation;
    // / M: OTASP }
};

#endif /* __RFX_CAT_CONTROLLER_H__ */

