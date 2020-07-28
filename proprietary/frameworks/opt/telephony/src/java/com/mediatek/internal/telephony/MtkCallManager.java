/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2017. All rights reserved.
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

package com.mediatek.internal.telephony;

import com.android.internal.telephony.Call;
import com.android.internal.telephony.CallManager;
import com.android.internal.telephony.Connection;
import com.android.internal.telephony.Phone;
import android.os.AsyncResult;
import android.os.Message;
import android.telephony.PhoneNumberUtils;
import android.telephony.Rlog;
import android.telephony.TelephonyManager;

/**
 * MTK add-on class for CallManager extensions.
 */
public class MtkCallManager extends CallManager {

    public class MtkCallManagerHandler extends CallManagerHandler {
        @Override
        public void handleMessage(Message msg) {

            switch (msg.what) {
                case EVENT_NEW_RINGING_CONNECTION:
                    if (VDBG) Rlog.d(LOG_TAG, " handleMessage (EVENT_NEW_RINGING_CONNECTION)");
                    Connection c = (Connection) ((AsyncResult) msg.obj).result;
                    int subId = c.getCall().getPhone().getSubId();
                    if (getActiveFgCallState(subId).isDialing() || hasMoreThanOneRingingCall()) {
                        /// M: CC: Remove handling for MO/MT conflict, not hangup MT
                        if (VDBG) Rlog.d(LOG_TAG, "MO/MT conflict! MO should be hangup by MD");
                    } else {
                        mNewRingingConnectionRegistrants.notifyRegistrants((AsyncResult) msg.obj);
                    }
                    break;
                default:
                    super.handleMessage(msg);
            }
        }
    }

    protected void registerForPhoneStates(Phone phone) {
        // We need to keep a mapping of handler to Phone for proper unregistration.
        // TODO: Clean up this solution as it is just a work around for each Phone instance
        // using the same Handler to register with the RIL. When time permits, we should consider
        // moving the handler (or the reference ot the handler) into the Phone object.
        // See b/17414427.
        CallManagerHandler handler = mHandlerMap.get(phone);
        if (handler != null) {
            Rlog.d(LOG_TAG, "This phone has already been registered.");
            return;
        }

        // New registration, create a new handler instance and register the phone.
        handler = new MtkCallManagerHandler();
        mHandlerMap.put(phone, handler);

        // for common events supported by all phones
        // The mRegistrantIdentifier passed here, is to identify in the Phone
        // that the registrants are coming from the CallManager.
        phone.registerForPreciseCallStateChanged(handler, EVENT_PRECISE_CALL_STATE_CHANGED,
                mRegistrantidentifier);
        phone.registerForDisconnect(handler, EVENT_DISCONNECT,
                mRegistrantidentifier);
        phone.registerForNewRingingConnection(handler, EVENT_NEW_RINGING_CONNECTION,
                mRegistrantidentifier);
        phone.registerForUnknownConnection(handler, EVENT_UNKNOWN_CONNECTION,
                mRegistrantidentifier);
        phone.registerForIncomingRing(handler, EVENT_INCOMING_RING,
                mRegistrantidentifier);
        phone.registerForRingbackTone(handler, EVENT_RINGBACK_TONE,
                mRegistrantidentifier);
        phone.registerForInCallVoicePrivacyOn(handler, EVENT_IN_CALL_VOICE_PRIVACY_ON,
                mRegistrantidentifier);
        phone.registerForInCallVoicePrivacyOff(handler, EVENT_IN_CALL_VOICE_PRIVACY_OFF,
                mRegistrantidentifier);
        phone.registerForDisplayInfo(handler, EVENT_DISPLAY_INFO,
                mRegistrantidentifier);
        phone.registerForSignalInfo(handler, EVENT_SIGNAL_INFO,
                mRegistrantidentifier);
        phone.registerForResendIncallMute(handler, EVENT_RESEND_INCALL_MUTE,
                mRegistrantidentifier);
        phone.registerForMmiInitiate(handler, EVENT_MMI_INITIATE,
                mRegistrantidentifier);
        phone.registerForMmiComplete(handler, EVENT_MMI_COMPLETE,
                mRegistrantidentifier);
        phone.registerForSuppServiceFailed(handler, EVENT_SUPP_SERVICE_FAILED,
                mRegistrantidentifier);
        phone.registerForServiceStateChanged(handler, EVENT_SERVICE_STATE_CHANGED,
                mRegistrantidentifier);

        // FIXME Taken from klp-sprout-dev but setAudioMode was removed in L.
        //phone.registerForRadioOffOrNotAvailable(handler, EVENT_RADIO_OFF_OR_NOT_AVAILABLE, null);

        // for events supported only by GSM, CDMA and IMS phone
        phone.setOnPostDialCharacter(handler, EVENT_POST_DIAL_CHARACTER, null);

        // for events supported only by CDMA phone
        phone.registerForCdmaOtaStatusChange(handler, EVENT_CDMA_OTA_STATUS_CHANGE, null);
        phone.registerForSubscriptionInfoReady(handler, EVENT_SUBSCRIPTION_INFO_READY, null);
        phone.registerForCallWaiting(handler, EVENT_CALL_WAITING, null);
        phone.registerForEcmTimerReset(handler, EVENT_ECM_TIMER_RESET, null);

        // for events supported only by IMS phone
        phone.registerForOnHoldTone(handler, EVENT_ONHOLD_TONE, null);
        phone.registerForSuppServiceFailed(handler, EVENT_SUPP_SERVICE_FAILED, null);
        phone.registerForTtyModeReceived(handler, EVENT_TTY_MODE_RECEIVED, null);
    }
}
